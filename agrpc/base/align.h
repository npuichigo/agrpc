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

#ifndef AGRPC_BASE_ALIGN_H_
#define AGRPC_BASE_ALIGN_H_

#include <cstddef>

namespace agrpc {

constexpr std::size_t max_align_v = alignof(max_align_t);

// At the time of writing, GCC 11 has not implemented these constants.
#if defined(__x86_64__)

// On Sandy Bridge, accessing adjacent cache lines also see destructive
// interference.
//
// @see: https://github.com/facebook/folly/blob/master/folly/lang/Align.h
//
// Update at 20201124: Well, AMD's Zen 3 does the same.
constexpr std::size_t hardware_destructive_interference_size = 128;
constexpr std::size_t hardware_constructive_interference_size = 64;

#elif defined(__aarch64__)

// AArch64 is ... weird, to say the least. Some vender (notably Samsung) uses a
// non-consistent cacheline size across BIG / little cores..
//
// Let's ignore those CPUs for now.
//
// @see: https://www.mono-project.com/news/2016/09/12/arm64-icache/
constexpr std::size_t hardware_destructive_interference_size = 64;
constexpr std::size_t hardware_constructive_interference_size = 64;

#elif defined(__powerpc64__)

// These values are read from
// `/sys/devices/system/cpu/cpu0/cache/index*/coherency_line_size`
constexpr std::size_t hardware_destructive_interference_size = 128;
constexpr std::size_t hardware_constructive_interference_size = 128;

#else

#error Unsupported architecture.

#endif

}  // namespace agrpc

#endif  // AGRPC_BASE_ALIGN_H_
