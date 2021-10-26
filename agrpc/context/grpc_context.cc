// Copyright 2021 The CRPC Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "agrpc/context/grpc_context.h"

#include <utility>

#include <unifex/scope_guard.hpp>

#include "agrpc/base/logging.h"
#include "agrpc/base/likely.h"

namespace agrpc {

static thread_local GrpcContext* current_thread_context;

static constexpr void* remote_queue_event_user_data = nullptr;

GrpcContext::GrpcContext(
    std::unique_ptr<grpc::ServerCompletionQueue> completion_queue)
    : completion_queue_(std::move(completion_queue)) {}

GrpcContext::~GrpcContext() {}

bool GrpcContext::IsRunningOnThisThread() const noexcept {
  return this == current_thread_context;
}

void GrpcContext::RunImpl(const bool& should_stop) {
  AGRPC_DLOG_INFO("Run loop started");

  auto* old_context = std::exchange(current_thread_context, this);
  unifex::scope_guard g = [=]() noexcept {
    std::exchange(current_thread_context, old_context);
    AGRPC_DLOG_INFO("Run loop exited");
  };

  while (true) {
    // Dequeue and process local queue items (ready to run)
    ExecutePendingLocal();

    if (AGRPC_UNLIKELY(should_stop)) {
      break;
    }

    // Check for remotely-queued items.
    if (!remote_queue_read_submitted_) {
      remote_queue_read_submitted_ = TryScheduleRemoteQueuedItems();
    }

    // Check for any new completion-queue items. Exit for shutdown event.
    if (!AcquireCompletionQueueItems()) {
      break;
    }
  }
}

void GrpcContext::ScheduleImpl(OperationBase* op) {
  AGRPC_CHECK(op != nullptr);
  if (IsRunningOnThisThread()) {
    ScheduleLocal(op);
  } else {
    ScheduleRemote(op);
  }
}

void GrpcContext::ScheduleLocal(OperationBase* op) noexcept {
  local_queue_.push_back(op);
}

void GrpcContext::ScheduleLocal(OperationQueue ops) noexcept {
  local_queue_.append(std::move(ops));
}

void GrpcContext::ScheduleRemote(OperationBase* op) noexcept {
  bool processing_thread_was_inactive = remote_queue_.enqueue(op);
  if (processing_thread_was_inactive) {
    // We were the first to queue an item and the processing thread
    // is not going to check the queue until we signal it that new
    // items have been enqueued remotely by adding a new task to the
    // end of the completion_queue_.
    SignalRemoteQueue();
  }
}

void GrpcContext::ExecutePendingLocal() noexcept {
  if (local_queue_.empty()) {
    AGRPC_DLOG_INFO("local queue is empty");
    return;
  }

  AGRPC_DLOG_INFO("Processing local queue items");

  size_t count = 0;
  auto pending = std::move(local_queue_);
  while (!pending.empty()) {
    auto* item = pending.pop_front();
    item->execute_(item);
    ++count;
  }

  AGRPC_DLOG_INFO("Processed {} local queue items", count);
}

bool GrpcContext::AcquireCompletionQueueItems() noexcept {
  bool got_event = completion_queue_->Next(&event_.tag, &event_.ok);
  if (AGRPC_UNLIKELY(!got_event))
    return false;
  if (event_.tag == remote_queue_event_user_data) {
    // Skip processing this item and let the loop check
    // for the remote-queued items next time around.
    remote_queue_read_submitted_ = false;
  } else {
    auto& completion_state = *reinterpret_cast<OperationBase*>(event_.tag);
    ScheduleLocal(&completion_state);
  }
  return true;
}

bool GrpcContext::TryScheduleRemoteQueuedItems() noexcept {
  auto queued_items = remote_queue_.try_mark_inactive_or_dequeue_all();
  if (!queued_items.empty()) {
    AGRPC_DLOG_INFO("Schedule remote queued items");
    ScheduleLocal(std::move(queued_items));
    return false;
  } else {
    AGRPC_DLOG_INFO("Remote queue is empty");
  }
  return true;
}

void GrpcContext::SignalRemoteQueue() {
  work_alarm_.Set(completion_queue_.get(),
                  gpr_now(gpr_clock_type::GPR_CLOCK_REALTIME),
                  remote_queue_event_user_data);
}

}  // namespace agrpc
