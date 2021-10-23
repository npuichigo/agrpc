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

#include "agrpc/base/chrono.h"

#include <chrono>
#include <thread>

#include "gtest/gtest.h"

using namespace std::literals;

namespace agrpc {
namespace {

TEST(CoarseSystemClock, Compare) {
  auto diff =
      (ReadCoarseSystemClock() - std::chrono::system_clock::now()) / 1ms;
  ASSERT_NEAR(diff, 0, 10);
}

TEST(CoarseSteadyClock, Compare) {
  auto diff =
      (ReadCoarseSteadyClock() - std::chrono::steady_clock::now()) / 1ms;
  ASSERT_NEAR(diff, 0, 10);
}

}  // namespace
}  // namespace agrpc