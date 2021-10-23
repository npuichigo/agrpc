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

#ifndef AGRPC_BASE_CHRONO_H_
#define AGRPC_BASE_CHRONO_H_

#include <atomic>
#include <chrono>
#include <thread>

#include "agrpc/base/align.h"

namespace agrpc {

namespace detail::chrono {

// The two timestamps are always updated at almost the same time, there's no
// point in putting them into separate cache lines, so we group them together
// here.
struct alignas(hardware_destructive_interference_size)
    AsynchronouslyUpdatedTimestamps {
  std::atomic<std::chrono::nanoseconds> steady_clock_time_since_epoch;
  std::atomic<std::chrono::nanoseconds> system_clock_time_since_epoch;
};

// Not so accurate.
inline AsynchronouslyUpdatedTimestamps async_updated_timestamps;

inline struct CoarseClockInitializer {
 public:
  CoarseClockInitializer();
  ~CoarseClockInitializer();

 private:
  std::thread worker_;
  std::atomic<bool> exiting_{false};
} coarse_clock_initializer;

}  // namespace detail::chrono

// This method is faster than `std::chrono::steady_clock::now()`, but it only
// provides a precision in (several) milliseconds (deviates less than 10ms.).
inline std::chrono::steady_clock::time_point ReadCoarseSteadyClock() {
  return std::chrono::steady_clock::time_point(
      detail::chrono::async_updated_timestamps.steady_clock_time_since_epoch
          .load(std::memory_order_relaxed));
}

// Same as `ReadCoarseSteadyClock` except it's for `std::system_clock`.
inline std::chrono::system_clock::time_point ReadCoarseSystemClock() {
  return std::chrono::system_clock::time_point(
      detail::chrono::async_updated_timestamps.system_clock_time_since_epoch
          .load(std::memory_order_relaxed));
}

}  // namespace agrpc

#endif  // AGRPC_BASE_CHRONO_H_
