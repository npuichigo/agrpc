if(NOT AGRPC_BUILD_EXAMPLES)
  return()
endif()

agrpc_cc_binary(
  NAME
    server
  SRCS
    "server.cc"
  DEPS
    agrpc::base::logging
    agrpc::context::grpc_context
    agrpc::proto::hello_world
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
    agrpc::proto::hello_world
    gflags
    gRPC::grpc++
    unifex
)
