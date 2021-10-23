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

#include <unifex/detail/atomic_intrusive_queue.hpp>
#include <unifex/detail/intrusive_queue.hpp>

namespace agrpc {

class GrpcContext {
 public:
  class scheduler;

  GrpcContext();

  ~GrpcContext();

  template <typename StopToken>
  void run(StopToken stopToken);

  scheduler get_scheduler() noexcept;

 private:
  struct operation_base {
    operation_base() noexcept {}
    operation_base* next_;
    void (*execute_)(operation_base*) noexcept;
  };

  using LocalWorkQueue =
      unifex::intrusive_queue<operation_base, &operation_base::next_>;
  using RemoteWorkQueue =
      unifex::atomic_intrusive_queue<operation_base, &operation_base::next_>;

  LocalWorkQueue local_queue_;
  RemoteWorkQueue remote_queue_;
};

}  // namespace agrpc

#endif  // AGRPC_CONTEXT_GRPC_CONTEXT_H_
