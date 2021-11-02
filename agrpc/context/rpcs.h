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

template <class RPC, class Responder>
using ServerSingleArgRequest = void (RPC::*)(grpc::ServerContext*, Responder*,
                                             grpc::CompletionQueue*,
                                             grpc::ServerCompletionQueue*,
                                             void*);

template <class RPC, class Request, class Reader>
using ClientUnaryRequest = Reader (RPC::*)(grpc::ClientContext*, const Request&,
                                           grpc::CompletionQueue*);

template <class RPC, class Request, class Reader>
using ClientServerStreamingRequest = Reader (RPC::*)(grpc::ClientContext*,
                                                     const Request&,
                                                     grpc::CompletionQueue*,
                                                     void*);

template <class RPC, class Writer, class Response>
using ClientSideStreamingRequest = Writer (RPC::*)(grpc::ClientContext*,
                                                   Response*,
                                                   grpc::CompletionQueue*,
                                                   void*);

template <class RPC, class ReaderWriter>
using ClientBidirectionalStreamingRequest =
    ReaderWriter (RPC::*)(grpc::ClientContext*, grpc::CompletionQueue*, void*);

}  // namespace detail

inline const struct AsyncRequestCPO {
  template <typename Executor, typename RPC, typename Service, typename Request,
            typename Responder>
  auto operator()(Executor&& executor,
                  detail::ServerMultiArgRequest<RPC, Request, Responder> rpc,
                  Service& service, grpc::ServerContext& server_context,
                  Request& request, Responder& responder) const
      noexcept(is_nothrow_tag_invocable_v<
               AsyncRequestCPO, Executor,
               detail::ServerMultiArgRequest<RPC, Request, Responder>, Service&,
               grpc::ServerContext&, Request&, Responder&>)
          -> tag_invoke_result_t<
              AsyncRequestCPO, Executor,
              detail::ServerMultiArgRequest<RPC, Request, Responder>, Service&,
              grpc::ServerContext&, Request&, Responder&> {
    return unifex::tag_invoke(*this, (Executor &&) executor, rpc, service,
                              server_context, request, responder);
  }

  template <typename Executor, typename RPC, typename Service,
            typename Responder>
  auto operator()(Executor&& executor,
                  detail::ServerSingleArgRequest<RPC, Responder> rpc,
                  Service& service, grpc::ServerContext& server_context,
                  Responder& responder) const
      noexcept(is_nothrow_tag_invocable_v<
               AsyncRequestCPO, Executor,
               detail::ServerSingleArgRequest<RPC, Responder>, Service&,
               grpc::ServerContext&, Responder&>)
          -> tag_invoke_result_t<AsyncRequestCPO, Executor,
                                 detail::ServerSingleArgRequest<RPC, Responder>,
                                 Service&, grpc::ServerContext&, Responder&> {
    return unifex::tag_invoke(*this, (Executor &&) executor, rpc, service,
                              server_context, responder);
  }
} AsyncRequest{};

inline const struct AsyncReadCPO {
  template <typename Executor, typename Response, typename Request>
  auto operator()(Executor&& executor,
                  grpc::ServerAsyncReader<Response, Request>& reader,
                  Request& request) const
      noexcept(is_nothrow_tag_invocable_v<
               AsyncReadCPO, Executor,
               grpc::ServerAsyncReader<Response, Request>&, Request&>)
          -> tag_invoke_result_t<AsyncReadCPO, Executor,
                                 grpc::ServerAsyncReader<Response, Request>&,
                                 Request&> {
    return unifex::tag_invoke(*this, (Executor &&) executor, reader, request);
  }

  template <typename Executor, typename Response, typename Request>
  auto operator()(
      Executor&& executor,
      grpc::ServerAsyncReaderWriter<Response, Request>& reader_writer,
      Request& request) const
      noexcept(is_nothrow_tag_invocable_v<
               AsyncReadCPO, Executor,
               grpc::ServerAsyncReaderWriter<Response, Request>&, Request&>)
          -> tag_invoke_result_t<
              AsyncReadCPO, Executor,
              grpc::ServerAsyncReaderWriter<Response, Request>&, Request&> {
    return unifex::tag_invoke(*this, (Executor &&) executor, reader_writer,
                              request);
  }
} AsyncRead{};

inline const struct AsyncWriteCPO {
  template <typename Executor, typename Response>
  auto operator()(Executor&& executor,
                  grpc::ServerAsyncWriter<Response>& writer,
                  const Response& response) const
      noexcept(is_nothrow_tag_invocable_v<AsyncWriteCPO, Executor,
                                          grpc::ServerAsyncWriter<Response>&,
                                          const Response&>)
          -> tag_invoke_result_t<AsyncWriteCPO, Executor,
                                 grpc::ServerAsyncWriter<Response>&,
                                 const Response&> {
    return unifex::tag_invoke(*this, (Executor &&) executor, writer, response);
  }

  template <typename Executor, typename Response, typename Request>
  auto operator()(
      Executor&& executor,
      grpc::ServerAsyncReaderWriter<Response, Request>& reader_writer,
      const Response& response) const
      noexcept(is_nothrow_tag_invocable_v<
               AsyncWriteCPO, Executor,
               grpc::ServerAsyncReaderWriter<Response, Request>&,
               const Response&>)
          -> tag_invoke_result_t<
              AsyncWriteCPO, Executor,
              grpc::ServerAsyncReaderWriter<Response, Request>&,
              const Response&> {
    return unifex::tag_invoke(*this, (Executor &&) executor, reader_writer,
                              response);
  }
} AsyncWrite{};

inline const struct AsyncFinishCPO {
  // Server
  template <typename Executor, typename Response>
  auto operator()(Executor&& executor,
                  grpc::ServerAsyncResponseWriter<Response>& writer,
                  const Response& response, const grpc::Status& status) const
      noexcept(
          is_nothrow_tag_invocable_v<AsyncFinishCPO, Executor,
                                     grpc::ServerAsyncResponseWriter<Response>&,
                                     const Response&, const grpc::Status&>)
          -> tag_invoke_result_t<AsyncFinishCPO, Executor,
                                 grpc::ServerAsyncResponseWriter<Response>&,
                                 const Response&, const grpc::Status&> {
    return unifex::tag_invoke(*this, (Executor &&) executor, writer, response,
                              status);
  }

  template <typename Executor, typename Response>
  auto operator()(Executor&& executor,
                  grpc::ServerAsyncWriter<Response>& writer,
                  const grpc::Status& status) const
      noexcept(is_nothrow_tag_invocable_v<AsyncFinishCPO, Executor,
                                          grpc::ServerAsyncWriter<Response>&,
                                          const grpc::Status&>)
          -> tag_invoke_result_t<AsyncFinishCPO, Executor,
                                 grpc::ServerAsyncWriter<Response>&,
                                 const grpc::Status&> {
    return unifex::tag_invoke(*this, (Executor &&) executor, writer, status);
  }

  template <typename Executor, typename Response, typename Request>
  auto operator()(Executor&& executor,
                  grpc::ServerAsyncReader<Response, Request>& reader,
                  const Response& response, const grpc::Status& status) const
      noexcept(is_nothrow_tag_invocable_v<
               AsyncFinishCPO, Executor,
               grpc::ServerAsyncReader<Response, Request>&, const Response&,
               const grpc::Status&>)
          -> tag_invoke_result_t<AsyncFinishCPO, Executor,
                                 grpc::ServerAsyncReader<Response, Request>&,
                                 const Response&, const grpc::Status&> {
    return unifex::tag_invoke(*this, (Executor &&) executor, reader, response,
                              status);
  }

  template <typename Executor, typename Response, typename Request>
  auto operator()(
      Executor&& executor,
      grpc::ServerAsyncReaderWriter<Response, Request>& reader_writer,
      const grpc::Status& status) const
      noexcept(is_nothrow_tag_invocable_v<
               AsyncFinishCPO, Executor,
               grpc::ServerAsyncReaderWriter<Response, Request>&,
               const grpc::Status&>)
          -> tag_invoke_result_t<
              AsyncFinishCPO, Executor,
              grpc::ServerAsyncReaderWriter<Response, Request>&,
              const grpc::Status&> {
    return unifex::tag_invoke(*this, (Executor &&) executor, reader_writer,
                              status);
  }

  // Client
  template <typename Executor, typename Response>
  auto operator()(Executor&& executor,
                  grpc::ClientAsyncResponseReader<Response>& reader,
                  Response& response, grpc::Status& status) const
      noexcept(
          is_nothrow_tag_invocable_v<AsyncFinishCPO, Executor,
                                     grpc::ClientAsyncResponseReader<Response>&,
                                     Response&, const grpc::Status&>)
          -> tag_invoke_result_t<AsyncFinishCPO, Executor,
                                 grpc::ClientAsyncResponseReader<Response>&,
                                 Response&, grpc::Status&> {
    return unifex::tag_invoke(*this, (Executor &&) executor, reader, response,
                              status);
  }
} AsyncFinish{};

inline const struct AsyncWriteAndFinishCPO {
  template <typename Executor, typename Response>
  auto operator()(Executor&& executor,
                  grpc::ServerAsyncWriter<Response>& writer,
                  const Response& response, grpc::WriteOptions options,
                  const grpc::Status& status) const
      noexcept(is_nothrow_tag_invocable_v<AsyncWriteAndFinishCPO, Executor,
                                          grpc::ServerAsyncWriter<Response>&,
                                          const Response&, grpc::WriteOptions,
                                          const grpc::Status&>)
          -> tag_invoke_result_t<AsyncWriteAndFinishCPO, Executor,
                                 grpc::ServerAsyncWriter<Response>&,
                                 const Response&, grpc::WriteOptions,
                                 const grpc::Status&> {
    return unifex::tag_invoke(*this, (Executor &&) executor, writer, response,
                              options, status);
  }

  template <typename Executor, typename Response, typename Request>
  auto operator()(
      Executor&& executor,
      grpc::ServerAsyncReaderWriter<Response, Request>& reader_writer,
      const Response& response, grpc::WriteOptions options,
      const grpc::Status& status) const
      noexcept(is_nothrow_tag_invocable_v<
               AsyncWriteCPO, Executor,
               grpc::ServerAsyncReaderWriter<Response, Request>&,
               const Response&, grpc::WriteOptions, const grpc::Status&>)
          -> tag_invoke_result_t<
              AsyncWriteAndFinishCPO, Executor,
              grpc::ServerAsyncReaderWriter<Response, Request>&,
              const Response&, grpc::WriteOptions, const grpc::Status&> {
    return unifex::tag_invoke(*this, (Executor &&) executor, reader_writer,
                              response, options, status);
  }
} AsyncWriteAndFinish{};

inline const struct AsyncFinishWithErrorCPO {
  template <typename Executor, typename Response, typename Request>
  auto operator()(Executor&& executor,
                  grpc::ServerAsyncReader<Response, Request>& reader,
                  const grpc::Status& status) const
      noexcept(
          is_nothrow_tag_invocable_v<
              AsyncFinishCPO, Executor,
              grpc::ServerAsyncReader<Response, Request>&, const grpc::Status&>)
          -> tag_invoke_result_t<AsyncFinishCPO, Executor,
                                 grpc::ServerAsyncReader<Response, Request>&,
                                 const grpc::Status&> {
    return unifex::tag_invoke(*this, (Executor &&) executor, reader, status);
  }

  template <typename Executor, typename Response>
  auto operator()(Executor&& executor,
                  grpc::ServerAsyncResponseWriter<Response> writer,
                  const grpc::Status& status) const
      noexcept(is_nothrow_tag_invocable_v<
               AsyncFinishWithErrorCPO, Executor,
               grpc::ServerAsyncResponseWriter<Response>&, const grpc::Status&>)
          -> tag_invoke_result_t<AsyncFinishWithErrorCPO, Executor,
                                 grpc::ServerAsyncResponseWriter<Response>&,
                                 const grpc::Status&> {
    return unifex::tag_invoke(*this, (Executor &&) executor, writer, status);
  }
} AsyncFinishWithError{};

inline const struct AsyncSendInitialMetadataCPO {
  template <typename Executor, typename Responder>
  auto operator()(Executor&& executor, Responder& responder) const noexcept(
      is_nothrow_tag_invocable_v<AsyncFinishWithErrorCPO, Executor, Responder&>)
      -> tag_invoke_result_t<AsyncFinishWithErrorCPO, Executor, Responder&> {
    return unifex::tag_invoke(*this, (Executor &&) executor, responder);
  }
} AsyncSendInitialMetadata;

}  // namespace agrpc

#endif  // AGRPC_CONTEXT_RPCS_H_
