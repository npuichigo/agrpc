agrpc_cc_proto_library(
  NAME
    helloworld
  SRCS
    "helloworld.proto"
  PUBLIC
  WITH_GRPC
)

agrpc_cc_proto_library(
  NAME
    hellostreamingworld
  SRCS
    "hellostreamingworld.proto"
  PUBLIC
  WITH_GRPC
)
