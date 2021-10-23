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

#include "benchmark/benchmark.h"

// The benchmark result makes little sense, as L1i / iTLB effect is negligible
// for such small method, and branch predictor should always predict right.
//
// Run on (40 X 3200 MHz CPU s)
// CPU Caches:
//  L1 Data 32 KiB (x20)
//  L1 Instruction 32 KiB (x20)
//  L2 Unified 1024 KiB (x20)
//  L3 Unified 14080 KiB (x2)
// Load Average: 1.25, 1.33, 1.33
//----------------------------------------------------------------
// Benchmark                      Time             CPU   Iterations
//----------------------------------------------------------------
// Benchmark_AgrpcCheck         1.02 ns         1.02 ns    682731668
// Benchmark_Check             1.37 ns         1.37 ns    475655532
// Benchmark_AgrpcCheckOp       1.32 ns         1.32 ns    529860761
// Benchmark_CheckOp           1.65 ns         1.65 ns    428222440

namespace agrpc {

volatile int x, y;

void Benchmark_AgrpcCheck(benchmark::State& state) {
  while (state.KeepRunning()) {
    AGRPC_CHECK(!x);
    AGRPC_CHECK(!x, "Some sophisticated log [{}].", x);
  }
}

BENCHMARK(Benchmark_AgrpcCheck);

void Benchmark_Check(benchmark::State& state) {
  while (state.KeepRunning()) {
    CHECK(!x);
    CHECK(!x) << "Some sophisticated log [" << x << "].";
  }
}

BENCHMARK(Benchmark_Check);

void Benchmark_AgrpcCheckOp(benchmark::State& state) {
  while (state.KeepRunning()) {
    AGRPC_CHECK_LE(x, y);
    AGRPC_CHECK_LE(x, y, "Some sophisticated log [{}, {}].", x, y);
  }
}

BENCHMARK(Benchmark_AgrpcCheckOp);

void Benchmark_CheckOp(benchmark::State& state) {
  while (state.KeepRunning()) {
    CHECK_LE(x, y);
    CHECK_LE(x, y) << "Some sophisticated log [" << x << ", " << y << "].";
  }
}

BENCHMARK(Benchmark_CheckOp);

}  // namespace agrpc
