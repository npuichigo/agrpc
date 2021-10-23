// Copyright 2021 The AGRPC Authors
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

#include "agrpc/base/logging.h"

#include <chrono>
#include <thread>
#include <vector>

#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"

#include "agrpc/base/chrono.h"

namespace agrpc {
namespace {

struct AwesomeLogSink : public google::LogSink {
  void send(google::LogSeverity severity, const char* full_filename,
            const char* base_filename, int line, const struct ::tm* tm_time,
            const char* message, size_t message_len) override {
    msgs.emplace_back(message, message_len);
    ++count;
  }
  std::vector<std::string> msgs;
  std::atomic<int> count{};
};
std::string my_prefix, my_prefix2;

void WriteLoggingPrefix(std::string* s) { *s += my_prefix; }
void WriteLoggingPrefix2(std::string* s) { *s += my_prefix2; }

TEST(Logging, Prefix) {
  AwesomeLogSink sink;
  google::AddLogSink(&sink);

  AGRPC_LOG_INFO("something");

  my_prefix = "[prefix]";
  AGRPC_LOG_INFO("something");

  my_prefix = "[prefix1]";
  AGRPC_LOG_INFO("something");

  my_prefix2 = "[prefix2]";
  AGRPC_LOG_INFO("something");

  ASSERT_THAT(sink.msgs,
              ::testing::ElementsAre("something", "[prefix] something",
                                     "[prefix1] something",
                                     "[prefix1] [prefix2] something"));
  google::RemoveLogSink(&sink);
}

AGRPC_INTERNAL_LOGGING_REGISTER_PREFIX_PROVIDER(0, WriteLoggingPrefix)
AGRPC_INTERNAL_LOGGING_REGISTER_PREFIX_PROVIDER(1, WriteLoggingPrefix2)

TEST(Logging, LogEverySecond) {
  AwesomeLogSink sink;
  google::AddLogSink(&sink);

  std::vector<std::thread> ts(100);
  for (auto&& t : ts) {
    t = std::thread([] {
      auto start = std::chrono::steady_clock::now();
      while (start + std::chrono::seconds(10) >
             std::chrono::steady_clock::now()) {
        AGRPC_LOG_WARNING_EVERY_SECOND("Some warning.");
      }
    });
  }
  for (auto&& t : ts) {
    t.join();
  }
  google::RemoveLogSink(&sink);
  ASSERT_NEAR(11 /* Plus the initial one. */, sink.count, 1);
}

}  // namespace
}  // namespace agrpc
