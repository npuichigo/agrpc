agrpc_cc_binary(
  NAME
    server
  SRCS
    "server.cc"
  DEPS
    agrpc::base::logging
    agrpc::context::grpc_context
    agrpc::example::proto::helloworld
    gflags
    gRPC::grpc++
    unifex
)

agrpc_cc_binary(
  NAME
    client
  SRCS
    "client.cc"
  DEPS
    agrpc::base::logging
    agrpc::context::grpc_context
    agrpc::example::proto::helloworld
    gflags
    gRPC::grpc++
    unifex
)
