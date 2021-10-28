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

#ifndef AGRPC_CONTEXT_GRPC_CONTEXT_H_
#define AGRPC_CONTEXT_GRPC_CONTEXT_H_

#include <atomic>
#include <memory>
#include <functional>

#include <grpcpp/alarm.h>
#include <grpcpp/completion_queue.h>

#include <unifex/detail/atomic_intrusive_queue.hpp>
#include <unifex/detail/intrusive_queue.hpp>
#include <unifex/receiver_concepts.hpp>
#include <unifex/type_traits.hpp>

#include "agrpc/base/logging.h"
#include "agrpc/context/rpcs.h"

namespace agrpc {

namespace detail {

struct GrpcCompletionQueueEvent {
  void* tag{nullptr};
  bool ok{false};
};

}  // namespace detail

class GrpcContext {
 public:
  class Scheduler;

  class AsyncRPCSender;

  GrpcContext(std::unique_ptr<grpc::ServerCompletionQueue> completion_queue);

  ~GrpcContext();

  template <typename StopToken>
  void Run(StopToken stopToken);

  Scheduler get_scheduler() noexcept;

  grpc::ServerCompletionQueue* get_completion_queue() noexcept;

 private:
  struct OperationBase {
    OperationBase() noexcept {}
    OperationBase* next_;
    void (*execute_)(OperationBase*) noexcept;
  };

  struct StopOperation : OperationBase {
    StopOperation() noexcept {
      this->execute_ = [](OperationBase* op) noexcept {
        static_cast<StopOperation*>(op)->should_stop_ = true;
      };
    }
    bool should_stop_ = false;
  };

  using OperationQueue =
      unifex::intrusive_queue<OperationBase, &OperationBase::next_>;
  using AtomicOperationQueue =
      unifex::atomic_intrusive_queue<OperationBase, &OperationBase::next_>;

  bool IsRunningOnThisThread() const noexcept;
  void RunImpl(const bool& should_stop);

  void ScheduleImpl(OperationBase* op);
  void ScheduleLocal(OperationBase* op) noexcept;
  void ScheduleLocal(OperationQueue ops) noexcept;
  void ScheduleRemote(OperationBase* op) noexcept;

  // Execute all ready-to-run items on the local queue.
  // Will not run other items that were enqueued during the execution of the
  // items that were already enqueued.
  // This bounds the amount of work to a finite amount.
  void ExecutePendingLocal() noexcept;

  // Check if any completion queue items are available and if so add them
  // to the local queue.
  //
  // Returns true if successful.
  //
  // Returns false if the completion queue is fully drained and shutdown.
  bool AcquireCompletionQueueItems() noexcept;

  // Collect the contents of the remote queue and pass them to ScheduleLocal
  //
  // Returns true if successful.
  //
  // Returns false if some other thread concurrently enqueued work to the remote
  // queue.
  bool TryScheduleRemoteQueuedItems() noexcept;

  // Wakeup the processing thread to acquire remotely-queued items.
  //
  // This should only be called after trying to enqueue() work
  // to the remote_queue_ and being told that the processing thread
  // is inactive.
  void SignalRemoteQueue();

  grpc::Alarm work_alarm_;
  detail::GrpcCompletionQueueEvent event_;
  std::unique_ptr<grpc::ServerCompletionQueue> completion_queue_;

  bool remote_queue_read_submitted_{false};

  OperationQueue local_queue_;
  AtomicOperationQueue remote_queue_{false};
};

template <typename StopToken>
void GrpcContext::Run(StopToken stop_token) {
  StopOperation stop_op;
  auto on_stop_requested = [&] { this->ScheduleImpl(&stop_op); };
  typename StopToken::template callback_type<decltype(on_stop_requested)>
      stop_callback{std::move(stop_token), std::move(on_stop_requested)};
  RunImpl(stop_op.should_stop_);
}

class GrpcContext::AsyncRPCSender {
  using AsyncRPC = std::function<void(GrpcContext&, void*)>;

  template <typename Receiver>
  class Operation : private OperationBase {
    friend GrpcContext;

   public:
    template <typename Receiver2>
    explicit Operation(const AsyncRPCSender& sender, Receiver2&& r)
        : context_(sender.context_),
          rpc_(sender.rpc_),
          receiver_((Receiver2 &&) r) {}

    void start() noexcept {
      if (!context_.IsRunningOnThisThread()) {
        static_cast<OperationBase*>(this)->execute_ =
            &Operation::OnScheduleComplete;
        context_.ScheduleRemote(static_cast<OperationBase*>(this));
      } else {
        StartAsyncRPC();
      }
    }

   private:
    static void OnScheduleComplete(OperationBase* op) noexcept {
      static_cast<Operation*>(op)->StartAsyncRPC();
    }

    void StartAsyncRPC() {
      AGRPC_CHECK(context_.IsRunningOnThisThread());
      static_cast<OperationBase*>(this)->execute_ =
          &Operation::OnRequestComplete;
      rpc_(context_, this);
    }

    static void OnRequestComplete(OperationBase* op) noexcept {
      auto& self = *static_cast<Operation*>(op);
      auto result = self.context_.event_.ok;
      if constexpr (noexcept(
                        unifex::set_value(std::move(self.receiver_), result))) {
        unifex::set_value(std::move(self.receiver_), result);
      } else {
        UNIFEX_TRY { unifex::set_value(std::move(self.receiver_), result); }
        UNIFEX_CATCH(...) {
          unifex::set_error(std::move(self.receiver_),
                            std::current_exception());
        }
      }
    }

    GrpcContext& context_;
    AsyncRPC rpc_;
    Receiver receiver_;
  };

 public:
  // Produces number of bytes read.
  template <template <typename...> class Variant,
            template <typename...> class Tuple>
  using value_types = Variant<Tuple<bool>>;

  // Note: Only case it might complete with exception_ptr is if the
  // receiver's set_value() exits with an exception.
  template <template <typename...> class Variant>
  using error_types = Variant<std::error_code, std::exception_ptr>;

  static constexpr bool sends_done = true;

  explicit AsyncRPCSender(GrpcContext& context, AsyncRPC rpc) noexcept
      : context_(context), rpc_(rpc) {}

  template <typename Receiver>
  Operation<unifex::remove_cvref_t<Receiver>> connect(Receiver&& r) && {
    return Operation<unifex::remove_cvref_t<Receiver>>{*this, (Receiver &&) r};
  }

 private:
  GrpcContext& context_;
  AsyncRPC rpc_;
};

class GrpcContext::Scheduler {
 public:
  Scheduler(const Scheduler&) noexcept = default;
  Scheduler& operator=(const Scheduler&) = default;
  ~Scheduler() = default;

 private:
  friend GrpcContext;

  template <typename RPC, typename Service, typename Request,
            typename Responder>
  friend GrpcContext::AsyncRPCSender tag_invoke(
      tag_t<AsyncRequest>, Scheduler s,
      detail::ServerMultiArgRequest<RPC, Request, Responder> rpc,
      Service& service, grpc::ServerContext& server_context, Request& request,
      Responder& responder);

  template <typename Response>
  friend GrpcContext::AsyncRPCSender tag_invoke(
      tag_t<AsyncFinish>, Scheduler s,
      grpc::ServerAsyncResponseWriter<Response>& writer,
      const Response& response, const grpc::Status& status);

  explicit Scheduler(GrpcContext& context) noexcept : context_(&context) {}

  GrpcContext* context_;
};

template <typename RPC, typename Service, typename Request, typename Responder>
GrpcContext::AsyncRPCSender tag_invoke(
    tag_t<AsyncRequest>, GrpcContext::Scheduler s,
    detail::ServerMultiArgRequest<RPC, Request, Responder> rpc,
    Service& service, grpc::ServerContext& server_context, Request& request,
    Responder& responder) {
  return GrpcContext::AsyncRPCSender(
      *s.context_, [&, rpc](GrpcContext& context, void* tag) {
        auto* cq = context.get_completion_queue();
        (service.*rpc)(&server_context, &request, &responder, cq, cq, tag);
      });
}

template <typename Response>
GrpcContext::AsyncRPCSender tag_invoke(
    tag_t<AsyncFinish>, GrpcContext::Scheduler s,
    grpc::ServerAsyncResponseWriter<Response>& writer,
    const Response& response, const grpc::Status& status) {
  return GrpcContext::AsyncRPCSender(
      *s.context_, [&](GrpcContext&, void* tag) {
        writer.Finish(response, status, tag);
      });
}

inline GrpcContext::Scheduler GrpcContext::get_scheduler() noexcept {
  return Scheduler{*this};
}

inline grpc::ServerCompletionQueue*
GrpcContext::get_completion_queue() noexcept {
  return completion_queue_.get();
}

}  // namespace agrpc

#endif  // AGRPC_CONTEXT_GRPC_CONTEXT_H_
