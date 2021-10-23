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

#ifndef AGRPC_BASE_LIKELY_H_
#define AGRPC_BASE_LIKELY_H_

// We define likely and unlikely here for special cases where c++20
// [[likely]] and [[unlikely]] attributes cannot be applied.
#if __GNUC__
#define AGRPC_LIKELY(expr) (__builtin_expect(!!(expr), 1))
#define AGRPC_UNLIKELY(expr) (__builtin_expect(!!(expr), 0))
#else
#define AGRPC_LIKELY(expr) (expr)
#define AGRPC_UNLIKELY(expr) (expr)
#endif

#endif  // AGRPC_BASE_LIKELY_H_
