#include <fmt/core.h>
#include <gflags/gflags.h>
#include <glog/logging.h>

#include "agrpc/base/logging.h"
#include "agrpc/proto/helloworld.pb.h"

using namespace agrpc;

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, false);
  google::InitGoogleLogging(argv[0]);

  helloworld::HelloReply reply;
  reply.set_message("hello");
  AGRPC_LOG_INFO("{}", reply.message());

  return 0;
}
