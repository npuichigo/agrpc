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

#ifndef AGRPC_CONTEXT_RPCS_H_
#define AGRPC_CONTEXT_RPCS_H_

#include <grpcpp/completion_queue.h>
#include <grpcpp/server_context.h>
#include <unifex/type_traits.hpp>

namespace agrpc {

using namespace unifex;

namespace detail {

template <class RPC, class Request, class Responder>
using ServerMultiArgRequest = void (RPC::*)(grpc::ServerContext*, Request*,
                                            Responder*, grpc::CompletionQueue*,
                                            grpc::ServerCompletionQueue*,
                                            void*);
}  // namespace detail

inline const struct AsyncRequestCPO {
  template <typename Executor, typename RPC, typename Service, typename Request,
      typename Responder>
  auto operator()(Executor&& executor,
                  detail::ServerMultiArgRequest<RPC, Request, Responder> rpc,
                  Service& service, grpc::ServerContext& server_context,
                  Request& request, Responder& responder) const
      noexcept(is_nothrow_tag_invocable_v<
               AsyncRequestCPO,
               Executor,
               detail::ServerMultiArgRequest<RPC, Request, Responder>,
               Service&, grpc::ServerContext&, Request&, Responder&>)
          -> tag_invoke_result_t<
              AsyncRequestCPO,
              Executor,
              detail::ServerMultiArgRequest<RPC, Request, Responder>,
              Service&, grpc::ServerContext&, Request&, Responder&> {
    return unifex::tag_invoke(*this, (Executor &&) executor, rpc, service,
                              server_context, request, responder);
  }
} AsyncRequest{};


}  // namespace agrpc

#endif  // AGRPC_CONTEXT_RPCS_H_
