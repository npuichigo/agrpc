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

#ifndef AGRPC_BASE_INTERNAL_MACROS_H_
#define AGRPC_BASE_INTERNAL_MACROS_H_

// Concatenate token
#define AGRPC_CONCATENATE_IMPL(s1, s2) s1##s2
#define AGRPC_CONCATENATE(s1, s2) AGRPC_CONCATENATE_IMPL(s1, s2)

#define AGRPC_MACRO_EXPAND(args) args

// Stringize token
#define AGRPC_STRINGIZE_IMPL(x) #x
#define AGRPC_STRINGIZE(x) AGRPC_STRINGIZE_IMPL(x)

// AGRPC_ANONYMOUS_VARIABLE(str) introduces an identifier starting with
// str and ending with a number that varies with the line.
#ifdef __COUNTER__
#define AGRPC_ANONYMOUS_VARIABLE(str) AGRPC_CONCATENATE(str, __COUNTER__)
#else
#define AGRPC_ANONYMOUS_VARIABLE(str) AGRPC_CONCATENATE(str, __LINE__)
#endif

#endif  // AGRPC_BASE_INTERNAL_MACROS_H_
