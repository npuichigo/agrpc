# include third_party according to flags
find_package(fmt REQUIRED)
find_package(gflags REQUIRED)
find_package(glog REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

if(AGRPC_BUILD_TESTS)
  enable_testing()
  find_package(GTest REQUIRED)
  find_package(benchmark REQUIRED)
endif()
