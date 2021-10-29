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

#include <fmt/core.h>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <unifex/inplace_stop_token.hpp>
#include <unifex/sync_wait.hpp>
#include <unifex/task.hpp>
#include <unifex/when_all.hpp>

#include "agrpc/base/logging.h"
#include "agrpc/context/grpc_context.h"
#include "agrpc/proto/helloworld.grpc.pb.h"

DEFINE_int32(port, 50051, "Grpc port to connect");

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, false);
  google::InitGoogleLogging(argv[0]);

  const std::string server_address = fmt::format("0.0.0.0:{}", FLAGS_port);

  auto stub = helloworld::Greeter::NewStub(
      grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()));
  agrpc::GrpcContext grpc_context{std::make_unique<grpc::CompletionQueue>()};

  unifex::inplace_stop_source stop_source{};
  unifex::sync_wait(unifex::when_all(
      [&]() -> unifex::task<void> {
        grpc::ClientContext client_context;
        helloworld::HelloRequest request;
        request.set_name("world");
        auto reader = stub->AsyncSayHello(&client_context, request,
                                          grpc_context.get_completion_queue());
        helloworld::HelloReply response;
        grpc::Status status;
        bool ok = co_await agrpc::AsyncFinish(grpc_context.get_scheduler(),
                                              *reader, response, status);
        AGRPC_CHECK(ok);
        AGRPC_LOG_INFO("Received: {}", response.message());
        grpc_context.ShutDown();
      }(),
      [&]() -> unifex::task<void> {
        grpc_context.Run(stop_source.get_token());
        co_return;
      }()));

  return 0;
}
