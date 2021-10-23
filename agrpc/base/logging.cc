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

#include <string>
#include <vector>

namespace agrpc::internal::logging {

namespace {

std::vector<PrefixAppender>* GetProviders() {
  static std::vector<PrefixAppender> providers;
  return &providers;
}

}  // namespace

void InstallPrefixProvider(PrefixAppender cb) {
  GetProviders()->push_back(cb);
}

void WritePrefixTo(std::string* to) {
  for (auto&& e : *GetProviders()) {
    auto was = to->size();
    e(to);
    if (to->size() != was) {
      to->push_back(' ');
    }
  }
}

std::string FormatLog([[maybe_unused]] const char* file,
                      [[maybe_unused]] int line) {
  std::string result;
  WritePrefixTo(&result);
  return result;
}

}  // namespace agrpc::internal::logging
