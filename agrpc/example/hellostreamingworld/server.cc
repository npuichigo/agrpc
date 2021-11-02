#include <fmt/core.h>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <unifex/inplace_stop_token.hpp>
#include <unifex/sync_wait.hpp>
#include <unifex/task.hpp>
#include <unifex/when_all.hpp>

#include "agrpc/base/logging.h"
#include "agrpc/context/grpc_context.h"
#include "agrpc/example/proto/hellostreamingworld.grpc.pb.h"

DEFINE_int32(port, 50051, "Grpc port to listen on");

template <typename Scheduler>
unifex::task<hellostreamingworld::HelloReply> HandleRequest(
    Scheduler scheduler,
    const hellostreamingworld::HelloRequest& request,
    grpc::ServerAsyncWriter<hellostreamingworld::HelloReply>& writer,
    const grpc::ServerContext& context) {
  hellostreamingworld::HelloReply response;
  response.set_message("Hello " + request.name());
  co_await agrpc::AsyncWrite(scheduler, writer, response);
  response.set_message("Hello " + request.name() + " again");
  co_await agrpc::AsyncWrite(scheduler, writer, response);
}

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, false);
  google::InitGoogleLogging(argv[0]);

  const std::string server_address = fmt::format("0.0.0.0:{}", FLAGS_port);

  grpc::ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register the service instance through which we'll communicate with clients.
  hellostreamingworld::MultiGreeter::AsyncService service;
  builder.RegisterService(&service);
  // Create the grpc context with a completion queue used for the asynchronous
  // communication with the gRPC runtime.
  agrpc::GrpcContext grpc_context{builder.AddCompletionQueue()};
  auto server = builder.BuildAndStart();
  AGRPC_LOG_INFO("Server listening on {}", server_address);

  unifex::inplace_stop_source stop_source{};
  unifex::sync_wait(unifex::when_all(
      [&]() -> unifex::task<void> {
        while (true) {
          grpc::ServerContext server_context;
          hellostreamingworld::HelloRequest request;
          grpc::ServerAsyncWriter<hellostreamingworld::HelloReply> writer{
              &server_context};
          bool request_ok = co_await agrpc::AsyncRequest(
              grpc_context.get_scheduler(),
              &hellostreamingworld::MultiGreeter::AsyncService::RequestSayHello,
              service, server_context, request, writer);
          if (!request_ok) {
            co_return;
          }
          co_await HandleRequest(grpc_context.get_scheduler(), request, writer,
                                 server_context);
          co_await agrpc::AsyncFinish(grpc_context.get_scheduler(), writer,
                                      grpc::Status::OK);
        }
      }(),
      [&]() -> unifex::task<void> {
        grpc_context.Run(stop_source.get_token());
        co_return;
      }()));

  return 0;
}
