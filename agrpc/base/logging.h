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

#ifndef AGRPC_BASE_LOGGING_H_
#define AGRPC_BASE_LOGGING_H_

// Macros here are more performant than their counterparts in glog. Consider
// use macro here unless you have a reason not to do so.

// You should use the same argument for `std::format` for `...` part in the
// declaration

#include <atomic>
#include <functional>
#include <string>

#include "fmt/format.h"
#include "fmt/ostream.h"
#include "glog/logging.h"

#include "agrpc/base/chrono.h"
#include "agrpc/base/internal/macros.h"
#include "agrpc/base/likely.h"

// glog's CHECK_OP does not optimize well by GCC 8.2. `google::CheckOpString`
// tries to hint the compiler, but the compiler does not seem to recognize that.
// In the meantime, `CHECK_OP` produces really large amount of code which can
// hurt performance. Our own implementation avoid these limitations.
#define AGRPC_CHECK(expr, ...) \
  AGRPC_INTERNAL_DETAIL_LOGGING_CHECK(expr, ##__VA_ARGS__)
#define AGRPC_CHECK_EQ(val1, val2, ...)   \
  AGRPC_INTERNAL_DETAIL_LOGGING_CHECK_OP( \
      _EQ, ==, val1, val2, ##__VA_ARGS__)  // `##` is GNU extension.
#define AGRPC_CHECK_NE(val1, val2, ...) \
  AGRPC_INTERNAL_DETAIL_LOGGING_CHECK_OP(_NE, !=, val1, val2, ##__VA_ARGS__)
#define AGRPC_CHECK_LE(val1, val2, ...) \
  AGRPC_INTERNAL_DETAIL_LOGGING_CHECK_OP(_LE, <=, val1, val2, ##__VA_ARGS__)
#define AGRPC_CHECK_LT(val1, val2, ...) \
  AGRPC_INTERNAL_DETAIL_LOGGING_CHECK_OP(_LT, <, val1, val2, ##__VA_ARGS__)
#define AGRPC_CHECK_GE(val1, val2, ...) \
  AGRPC_INTERNAL_DETAIL_LOGGING_CHECK_OP(_GE, >=, val1, val2, ##__VA_ARGS__)
#define AGRPC_CHECK_GT(val1, val2, ...) \
  AGRPC_INTERNAL_DETAIL_LOGGING_CHECK_OP(_GT, >, val1, val2, ##__VA_ARGS__)
#define AGRPC_CHECK_NEAR(val1, val2, margin, ...)             \
  do {                                                       \
    AGRPC_CHECK_LE((val1), (val2) + (margin), ##__VA_ARGS__); \
    AGRPC_CHECK_GE((val1), (val2) - (margin), ##__VA_ARGS__); \
  } while (0)

// Do NOT use `DCHECK`s from glog, they're not `constexpr`-friendly.
#ifndef NDEBUG
#define AGRPC_DCHECK(expr, ...) AGRPC_CHECK(expr, ##__VA_ARGS__)
#define AGRPC_DCHECK_EQ(expr1, expr2, ...) \
  AGRPC_CHECK_EQ(expr1, expr2, ##__VA_ARGS__)
#define AGRPC_DCHECK_NE(expr1, expr2, ...) \
  AGRPC_CHECK_NE(expr1, expr2, ##__VA_ARGS__)
#define AGRPC_DCHECK_LE(expr1, expr2, ...) \
  AGRPC_CHECK_LE(expr1, expr2, ##__VA_ARGS__)
#define AGRPC_DCHECK_GE(expr1, expr2, ...) \
  AGRPC_CHECK_GE(expr1, expr2, ##__VA_ARGS__)
#define AGRPC_DCHECK_LT(expr1, expr2, ...) \
  AGRPC_CHECK_LT(expr1, expr2, ##__VA_ARGS__)
#define AGRPC_DCHECK_GT(expr1, expr2, ...) \
  AGRPC_CHECK_GT(expr1, expr2, ##__VA_ARGS__)
#define AGRPC_DCHECK_NEAR(expr1, expr2, margin, ...) \
  AGRPC_CHECK_NEAR(expr1, expr2, ##__VA_ARGS__)
#else
#define AGRPC_DCHECK(expr, ...) \
  while (0) AGRPC_CHECK(expr, ##__VA_ARGS__)
#define AGRPC_DCHECK_EQ(expr1, expr2, ...) \
  while (0) AGRPC_CHECK_EQ(expr1, expr2, ##__VA_ARGS__)
#define AGRPC_DCHECK_NE(expr1, expr2, ...) \
  while (0) AGRPC_CHECK_NE(expr1, expr2, ##__VA_ARGS__)
#define AGRPC_DCHECK_LE(expr1, expr2, ...) \
  while (0) AGRPC_CHECK_LE(expr1, expr2, ##__VA_ARGS__)
#define AGRPC_DCHECK_GE(expr1, expr2, ...) \
  while (0) AGRPC_CHECK_GE(expr1, expr2, ##__VA_ARGS__)
#define AGRPC_DCHECK_LT(expr1, expr2, ...) \
  while (0) AGRPC_CHECK_LT(expr1, expr2, ##__VA_ARGS__)
#define AGRPC_DCHECK_GT(expr1, expr2, ...) \
  while (0) AGRPC_CHECK_GT(expr1, expr2, ##__VA_ARGS__)
#define AGRPC_DCHECK_NEAR(expr1, expr2, margin, ...) \
  while (0) AGRPC_CHECK_NEAR(expr1, expr2, ##__VA_ARGS__)
#endif

#define AGRPC_PCHECK(expr, ...) \
  AGRPC_INTERNAL_DETAIL_LOGGING_PCHECK(expr, ##__VA_ARGS__)

#define AGRPC_VLOG(n, ...)                    \
  LOG_IF(INFO, AGRPC_UNLIKELY(VLOG_IS_ON(n))) \
      << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__, __VA_ARGS__)

#define AGRPC_LOG_INFO(...)                                              \
  LOG(INFO) << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__, \
                                                    __VA_ARGS__)
#define AGRPC_LOG_WARNING(...)                                              \
  LOG(WARNING) << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__, \
                                                       __VA_ARGS__)
#define AGRPC_LOG_ERROR(...)                                              \
  LOG(ERROR) << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__, \
                                                     __VA_ARGS__)
#define AGRPC_LOG_FATAL(...)                                                \
  do {                                                                     \
    LOG(FATAL) << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__, \
                                                       __VA_ARGS__);       \
  } while (0);                                                             \
  AGRPC_UNREACHABLE()

#define AGRPC_LOG_INFO_IF(expr, ...)                           \
  LOG_IF(INFO, expr) << ::agrpc::internal::logging::FormatLog( \
      __FILE__, __LINE__, __VA_ARGS__)
#define AGRPC_LOG_WARNING_IF(expr, ...) \
  LOG_IF(WARNING, AGRPC_UNLIKELY(expr)) \
      << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__, __VA_ARGS__)
#define AGRPC_LOG_ERROR_IF(expr, ...) \
  LOG_IF(ERROR, AGRPC_UNLIKELY(expr)) \
      << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__, __VA_ARGS__)
#define AGRPC_LOG_FATAL_IF(expr, ...) \
  LOG_IF(FATAL, AGRPC_UNLIKELY(expr)) \
      << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__, __VA_ARGS__)

#define AGRPC_LOG_INFO_EVERY_N(N, ...) \
  AGRPC_INTERNAL_DETAIL_LOG_EVERY_N(INFO, N, __VA_ARGS__)
#define AGRPC_LOG_WARNING_EVERY_N(N, ...) \
  AGRPC_INTERNAL_DETAIL_LOG_EVERY_N(WARNING, N, __VA_ARGS__)
#define AGRPC_LOG_ERROR_EVERY_N(N, ...) \
  AGRPC_INTERNAL_DETAIL_LOG_EVERY_N(ERROR, N, __VA_ARGS__)
#define AGRPC_LOG_FATAL_EVERY_N(N, ...) \
  AGRPC_INTERNAL_DETAIL_LOG_EVERY_N(FATAL, N, __VA_ARGS__)

#define AGRPC_LOG_INFO_IF_EVERY_N(expr, N, ...) \
  AGRPC_INTERNAL_DETAIL_LOG_IF_EVERY_N(expr, INFO, N, __VA_ARGS__)
#define AGRPC_LOG_WARNING_IF_EVERY_N(expr, N, ...) \
  AGRPC_INTERNAL_DETAIL_LOG_IF_EVERY_N(expr, WARNING, N, __VA_ARGS__)
#define AGRPC_LOG_ERROR_IF_EVERY_N(expr, N, ...) \
  AGRPC_INTERNAL_DETAIL_LOG_IF_EVERY_N(expr, ERROR, N, __VA_ARGS__)
#define AGRPC_LOG_FATAL_IF_EVERY_N(expr, N, ...) \
  AGRPC_INTERNAL_DETAIL_LOG_IF_EVERY_N(expr, FATAL, N, __VA_ARGS__)

#define AGRPC_LOG_INFO_ONCE(...) AGRPC_INTERNAL_DETAIL_LOG_ONCE(INFO, __VA_ARGS__)
#define AGRPC_LOG_WARNING_ONCE(...) \
  AGRPC_INTERNAL_DETAIL_LOG_ONCE(WARNING, __VA_ARGS__)
#define AGRPC_LOG_ERROR_ONCE(...) \
  AGRPC_INTERNAL_DETAIL_LOG_ONCE(ERROR, __VA_ARGS__)
#define AGRPC_LOG_FATAL_ONCE(...)                        \
  /* You're unlikely to have a second chance anyway. */ \
  AGRPC_INTERNAL_DETAIL_LOG_ONCE(FATAL, __VA_ARGS__)

#define AGRPC_LOG_INFO_IF_ONCE(expr, ...)                \
  do {                                                  \
    if (expr) {                                         \
      AGRPC_INTERNAL_DETAIL_LOG_ONCE(INFO, __VA_ARGS__); \
    }                                                   \
  } while (0)
#define AGRPC_LOG_WARNING_IF_ONCE(expr, ...)                \
  do {                                                     \
    if (expr) [[unlikely]] {                               \
      AGRPC_INTERNAL_DETAIL_LOG_ONCE(WARNING, __VA_ARGS__); \
    }                                                      \
  } while (0)
#define AGRPC_LOG_ERROR_IF_ONCE(expr, ...)                \
  do {                                                   \
    if (expr) [[unlikely]] {                             \
      AGRPC_INTERNAL_DETAIL_LOG_ONCE(ERROR, __VA_ARGS__); \
    }                                                    \
  } while (0)
#define AGRPC_LOG_FATAL_IF_ONCE(expr, ...)                \
  do {                                                   \
    if (expr) [[unlikely]] {                             \
      AGRPC_INTERNAL_DETAIL_LOG_ONCE(FATAL, __VA_ARGS__); \
    }                                                    \
  } while (0)

#define AGRPC_DLOG_INFO(...)                                              \
  DLOG(INFO) << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__, \
                                                     __VA_ARGS__)
#define AGRPC_DLOG_WARNING(...)                                              \
  DLOG(WARNING) << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__, \
                                                        __VA_ARGS__)
#define AGRPC_DLOG_ERROR(...)                                              \
  DLOG(ERROR) << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__, \
                                                      __VA_ARGS__)
#define AGRPC_DLOG_FATAL(...)                                              \
  DLOG(FATAL) << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__, \
                                                      __VA_ARGS__)

#define AGRPC_DLOG_INFO_IF(expr, ...)                           \
  DLOG_IF(INFO, expr) << ::agrpc::internal::logging::FormatLog( \
      __FILE__, __LINE__, __VA_ARGS__)
#define AGRPC_DLOG_WARNING_IF(expr, ...) \
  DLOG_IF(WARNING, AGRPC_UNLIKELY(expr)) \
      << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__, __VA_ARGS__)
#define AGRPC_DLOG_ERROR_IF(expr, ...) \
  DLOG_IF(ERROR, AGRPC_UNLIKELY(expr)) \
      << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__, __VA_ARGS__)
#define AGRPC_DLOG_FATAL_IF(expr, ...) \
  DLOG_IF(FATAL, AGRPC_UNLIKELY(expr)) \
      << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__, __VA_ARGS__)

#define AGRPC_DLOG_INFO_EVERY_N(N, ...)                           \
  DLOG_EVERY_N(INFO, N) << ::agrpc::internal::logging::FormatLog( \
      __FILE__, __LINE__, __VA_ARGS__)
#define AGRPC_DLOG_WARNING_EVERY_N(N, ...)                           \
  DLOG_EVERY_N(WARNING, N) << ::agrpc::internal::logging::FormatLog( \
      __FILE__, __LINE__, __VA_ARGS__)
#define AGRPC_DLOG_ERROR_EVERY_N(N, ...)                           \
  DLOG_EVERY_N(ERROR, N) << ::agrpc::internal::logging::FormatLog( \
      __FILE__, __LINE__, __VA_ARGS__)
#define AGRPC_DLOG_FATAL_EVERY_N(N, ...)                           \
  DLOG_EVERY_N(FATAL, N) << ::agrpc::internal::logging::FormatLog( \
      __FILE__, __LINE__, __VA_ARGS__)

#ifndef NDEBUG
#define AGRPC_DLOG_INFO_ONCE(...) \
  AGRPC_INTERNAL_DETAIL_LOG_ONCE(INFO, __VA_ARGS__)
#define AGRPC_DLOG_WARNING_ONCE(...) \
  AGRPC_INTERNAL_DETAIL_LOG_ONCE(WARNING, __VA_ARGS__)
#define AGRPC_DLOG_ERROR_ONCE(...) \
  AGRPC_INTERNAL_DETAIL_LOG_ONCE(ERROR, __VA_ARGS__)
#define AGRPC_DLOG_FATAL_ONCE(...) \
  AGRPC_INTERNAL_DETAIL_LOG_ONCE(FATAL, __VA_ARGS__)
#else
// The expansion below is NOT a bug.
//
// AGRPC_DLOG_XXX is expanded to "no-op" expression if `NDEBUG` is defined.
// Therefore, although `AGRPC_DLOG_INFO_ONCE` doesn't behave in the same way as
// `AGRPC_DLOG_INFO`, the end result it the same (nothing is ever evaluated).
//
// The reason why we expands to `AGRPC_DLOG_XXX` instead of simply `(void)0` is
// that `AGRPC_DLOG_XXX` does some trick (done by glog, to be precise) to avoid
// "unused variable" in a best-effort fashion, and we want to benefit from that
// here.
#define AGRPC_DLOG_INFO_ONCE(...) AGRPC_DLOG_INFO(__VA_ARGS__);
#define AGRPC_DLOG_WARNING_ONCE(...) AGRPC_DLOG_WARNING(__VA_ARGS__);
#define AGRPC_DLOG_ERROR_ONCE(...) AGRPC_DLOG_ERROR(__VA_ARGS__);
#define AGRPC_DLOG_FATAL_ONCE(...) AGRPC_DLOG_FATAL(__VA_ARGS__);
#endif

#define AGRPC_PLOG_INFO(...)                                              \
  PLOG(INFO) << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__, \
                                                     __VA_ARGS__)
#define AGRPC_PLOG_WARNING(...)                                              \
  PLOG(WARNING) << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__, \
                                                        __VA_ARGS__)
#define AGRPC_PLOG_ERROR(...)                                              \
  PLOG(ERROR) << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__, \
                                                      __VA_ARGS__)
#define AGRPC_PLOG_FATAL(...)                                              \
  PLOG(FATAL) << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__, \
                                                      __VA_ARGS__)

#define AGRPC_PLOG_INFO_IF(expr, ...)                           \
  PLOG_IF(INFO, expr) << ::agrpc::internal::logging::FormatLog( \
      __FILE__, __LINE__, __VA_ARGS__)
#define AGRPC_PLOG_WARNING_IF(expr, ...) \
  PLOG_IF(WARNING, AGRPC_UNLIKELY(expr)) \
      << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__, __VA_ARGS__)
#define AGRPC_PLOG_ERROR_IF(expr, ...) \
  PLOG_IF(ERROR, AGRPC_UNLIKELY(expr)) \
      << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__, __VA_ARGS__)
#define AGRPC_PLOG_FATAL_IF(expr, ...) \
  PLOG_IF(FATAL, AGRPC_UNLIKELY(expr)) \
      << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__, __VA_ARGS__)

#define AGRPC_LOG_INFO_EVERY_SECOND(...) \
  AGRPC_DETAIL_LOGGING_LOG_EVERY_N_SECOND_IMPL(1, INFO, __VA_ARGS__)
#define AGRPC_LOG_WARNING_EVERY_SECOND(...) \
  AGRPC_DETAIL_LOGGING_LOG_EVERY_N_SECOND_IMPL(1, WARNING, __VA_ARGS__)
#define AGRPC_LOG_ERROR_EVERY_SECOND(...) \
  AGRPC_DETAIL_LOGGING_LOG_EVERY_N_SECOND_IMPL(1, ERROR, __VA_ARGS__)
#define AGRPC_LOG_FATAL_EVERY_SECOND(...) \
  AGRPC_DETAIL_LOGGING_LOG_EVERY_N_SECOND_IMPL(1, FATAL, __VA_ARGS__)

#define AGRPC_LOG_INFO_IF_EVERY_SECOND(expr, ...) \
  AGRPC_DETAIL_LOGGING_LOG_EVERY_N_SECOND_IF_IMPL(expr, 1, INFO, __VA_ARGS__)
#define AGRPC_LOG_WARNING_IF_EVERY_SECOND(expr, ...)                      \
  AGRPC_DETAIL_LOGGING_LOG_EVERY_N_SECOND_IF_IMPL(AGRPC_UNLIKELY(expr), 1, \
                                                 WARNING, __VA_ARGS__)
#define AGRPC_LOG_ERROR_IF_EVERY_SECOND(expr, ...)                        \
  AGRPC_DETAIL_LOGGING_LOG_EVERY_N_SECOND_IF_IMPL(AGRPC_UNLIKELY(expr), 1, \
                                                 ERROR, __VA_ARGS__)
#define AGRPC_LOG_FATAL_IF_EVERY_SECOND(expr, ...)                        \
  AGRPC_DETAIL_LOGGING_LOG_EVERY_N_SECOND_IF_IMPL(AGRPC_UNLIKELY(expr), 1, \
                                                 FATAL, __VA_ARGS__)

namespace agrpc::internal::logging {

// Prefix writer.
//
// Note that the implementation MAY NOT touch whatever has been in `to`. The
// implementation may only append its own prefix to `to`.
using PrefixAppender = std::function<void(std::string* to)>;

// Install a new logging prefix provider.
void InstallPrefixProvider(PrefixAppender writer);

// Call logging prefix providers to generate the prefix for log.
void WritePrefixTo(std::string* to);

// FOR INTERNAL USE ONLY.
//
// Logging prefix providers must be registered before `main` is entered to avoid
// potential race conditions. We provide this macro to accomplish this.
#define AGRPC_INTERNAL_LOGGING_REGISTER_PREFIX_PROVIDER(priority, cb)        \
  [[gnu::constructor(priority + 101)]] static void AGRPC_ANONYMOUS_VARIABLE( \
      agrpc_reserved_logging_prefix_provider_installer_)() {                 \
    ::agrpc::internal::logging::InstallPrefixProvider(cb);                   \
  }

// Marked as noexcept. Throwing in formatting log is likely a programming
// error.
template <class... Args>
std::string FormatLog([[maybe_unused]] const char* file,
                      [[maybe_unused]] int line,
                      fmt::format_string<Args...> fmt,
                      Args&&... args) noexcept {
  std::string result;

  WritePrefixTo(&result);
  try {
    result += fmt::format(fmt, std::forward<Args>(args)...);
  } catch (const std::exception& ex) {
    // Presumably a wrong format string was provided?
    LOG(FATAL) << "Failed to format log at [" << file << ":" << line
               << "]: " << ex.what();
  }
  return result;
}

std::string FormatLog([[maybe_unused]] const char* file,
                      [[maybe_unused]] int line);

}  // namespace agrpc::internal::logging

// Clang 10 has not implemented P1381R1 yet, therefore the "cold lambda" trick
// won't work quite right if structured binding identifiers are accessed during
// evaluating log message.
#if defined(__clang__) && __clang__ <= 10 || defined(_MSC_VER)
#define AGRPC_INTERNAL_DETAIL_LOGGING_ATTRIBUTE_NOINLINE_COLD
#define AGRPC_INTERNAL_DETAIL_LOGGING_ATTRIBUTE_NORETURN_NOINLINE_COLD
#else
// C++ attribute won't work here.
//
// @see: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=89640,
#define AGRPC_INTERNAL_DETAIL_LOGGING_ATTRIBUTE_NOINLINE_COLD \
  __attribute__((noinline, cold))
#define AGRPC_INTERNAL_DETAIL_LOGGING_ATTRIBUTE_NORETURN_NOINLINE_COLD \
  __attribute__((noreturn, noinline, cold))
#endif

#ifdef _MSC_VER
#define AGRPC_UNREACHABLE(...)                                              \
  do {                                                                     \
    LOG(FATAL) << "UNREACHABLE. "                                          \
               << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__, \
                                                       ##__VA_ARGS__);     \
  } while (0)
#define AGRPC_NOT_IMPLEMENTED(...)                                          \
  do {                                                                     \
    LOG(FATAL) << "Not implemented. "                                      \
               << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__, \
                                                       ##__VA_ARGS__);     \
  } while (0)
#define AGRPC_UNEXPECTED(...)                                               \
  do {                                                                     \
    LOG(FATAL) << "UNEXPECTED. "                                           \
               << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__, \
                                                       ##__VA_ARGS__);     \
  } while (0)
#else
#define AGRPC_UNREACHABLE(...)                                                \
  do {                                                                       \
    [&]() AGRPC_INTERNAL_DETAIL_LOGGING_ATTRIBUTE_NORETURN_NOINLINE_COLD {    \
      LOG(FATAL) << "UNREACHABLE. "                                          \
                 << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__, \
                                                         ##__VA_ARGS__);     \
      __builtin_unreachable();                                               \
    }();                                                                     \
  } while (0)
#define AGRPC_NOT_IMPLEMENTED(...)                                            \
  do {                                                                       \
    [&]() AGRPC_INTERNAL_DETAIL_LOGGING_ATTRIBUTE_NORETURN_NOINLINE_COLD {    \
      LOG(FATAL) << "Not implemented. "                                      \
                 << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__, \
                                                         ##__VA_ARGS__);     \
      __builtin_unreachable();                                               \
    }();                                                                     \
  } while (0)
#define AGRPC_UNEXPECTED(...)                                                 \
  do {                                                                       \
    [&]() AGRPC_INTERNAL_DETAIL_LOGGING_ATTRIBUTE_NORETURN_NOINLINE_COLD {    \
      LOG(FATAL) << "UNEXPECTED. "                                           \
                 << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__, \
                                                         ##__VA_ARGS__);     \
      __builtin_unreachable();                                               \
    }();                                                                     \
  } while (0)
#endif

#define AGRPC_INTERNAL_DETAIL_LOGGING_CHECK(expr, ...)                        \
  do {                                                                       \
    if (!(expr)) [[unlikely]] {                                              \
      /* Attribute `noreturn` is not applicable to lambda, unfortunately. */ \
      [&]() AGRPC_INTERNAL_DETAIL_LOGGING_ATTRIBUTE_NORETURN_NOINLINE_COLD {  \
        ::google::LogMessage(__FILE__, __LINE__, ::google::GLOG_FATAL)       \
                .stream()                                                    \
            << "Check failed: " #expr " "                                    \
            << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__,      \
                                                    ##__VA_ARGS__);          \
        AGRPC_UNREACHABLE();                                                  \
      }();                                                                   \
    }                                                                        \
  } while (0)

#define AGRPC_INTERNAL_DETAIL_LOGGING_CHECK_OP(name, op, val1, val2, ...)       \
  do {                                                                         \
    auto&& agrpc_anonymous_x = (val1);                                          \
    auto&& agrpc_anonymous_y = (val2);                                          \
    if (!(agrpc_anonymous_x op agrpc_anonymous_y)) [[unlikely]] {                \
      [&]() AGRPC_INTERNAL_DETAIL_LOGGING_ATTRIBUTE_NORETURN_NOINLINE_COLD {    \
        ::google::LogMessageFatal(                                             \
            __FILE__, __LINE__,                                                \
            ::google::CheckOpString(::google::MakeCheckOpString(               \
                agrpc_anonymous_x, agrpc_anonymous_y, #val1 " " #op " " #val2))) \
                .stream()                                                      \
            << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__,        \
                                                    ##__VA_ARGS__);            \
        AGRPC_UNREACHABLE();                                                    \
      }();                                                                     \
    }                                                                          \
  } while (0)

#define AGRPC_INTERNAL_DETAIL_LOGGING_PCHECK(expr, ...)                         \
  do {                                                                         \
    if (!(expr)) [[unlikely]] {                                                \
      [&]() AGRPC_INTERNAL_DETAIL_LOGGING_ATTRIBUTE_NORETURN_NOINLINE_COLD {    \
        ::google::ErrnoLogMessage(__FILE__, __LINE__, ::google::GLOG_FATAL, 0, \
                                  &::google::LogMessage::SendToLog)            \
                .stream()                                                      \
            << "Check failed: " #expr " "                                      \
            << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__,        \
                                                    ##__VA_ARGS__);            \
        AGRPC_UNREACHABLE();                                                    \
      }();                                                                     \
    }                                                                          \
  } while (0)

#define AGRPC_INTERNAL_DETAIL_LOG_ONCE(Level, ...)                              \
  do {                                                                         \
    static std::atomic<bool> agrpc_anonymous_logged{false};                     \
    if (!agrpc_anonymous_logged.load(std::memory_order_relaxed)) [[unlikely]] { \
      [&]() AGRPC_INTERNAL_DETAIL_LOGGING_ATTRIBUTE_NOINLINE_COLD {             \
        if (!agrpc_anonymous_logged.exchange(true)) {                           \
          LOG(Level) << ::agrpc::internal::logging::FormatLog(                  \
              __FILE__, __LINE__, __VA_ARGS__);                                \
        }                                                                      \
      }();                                                                     \
    }                                                                          \
  } while (0)

#define AGRPC_INTERNAL_DETAIL_LOG_EVERY_N(Level, N, ...)                   \
  do {                                                                    \
    static int agrpc_anonymous_logged_counter_mod_n = 0;                   \
    if (++agrpc_anonymous_logged_counter_mod_n > N) [[unlikely]] {         \
      [&]() AGRPC_INTERNAL_DETAIL_LOGGING_ATTRIBUTE_NOINLINE_COLD {        \
        agrpc_anonymous_logged_counter_mod_n -= N;                         \
        if (agrpc_anonymous_logged_counter_mod_n == 1)                     \
          google::LogMessage(__FILE__, __LINE__, google::GLOG_##Level, 0, \
                             &google::LogMessage::SendToLog)              \
                  .stream()                                               \
              << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__, \
                                                      __VA_ARGS__);       \
      }();                                                                \
    }                                                                     \
  } while (0)

#define AGRPC_INTERNAL_DETAIL_LOG_IF_EVERY_N(expr, Level, N, ...) \
  do {                                                           \
    if (expr) {                                                  \
      AGRPC_INTERNAL_DETAIL_LOG_EVERY_N(Level, N, __VA_ARGS__);   \
    }                                                            \
  } while (0)

#define AGRPC_DETAIL_LOGGING_LOG_EVERY_N_SECOND_IMPL(N, severity, ...)          \
  do {                                                                         \
    /* Read / write (but not RMW) comes with no cost in `std::atomic<...>`, */ \
    /* so we make it safe. */                                                  \
    static ::std::atomic<::std::chrono::nanoseconds>                           \
        agrpc_anonymous_log_last_occurs{};                                      \
    /* You're not expected to see the log too often if you use this macro, */  \
    /* aren't you? */                                                          \
    if (auto agrpc_anonymous_now =                                              \
            ::agrpc::ReadCoarseSteadyClock().time_since_epoch();                \
        agrpc_anonymous_log_last_occurs.load(::std::memory_order_relaxed) +     \
            ::std::chrono::seconds(N) <                                        \
        agrpc_anonymous_now) [[unlikely]] {                                     \
      /* Executed every second. I think it's "cold" enough to be moved out. */ \
      [&]() AGRPC_INTERNAL_DETAIL_LOGGING_ATTRIBUTE_NOINLINE_COLD {             \
        static ::std::atomic<bool> agrpc_anonymous_logging{false};              \
        /* To ensure only a single thread really writes. */                    \
        /* Full barrier here to enforce order between atomics. */              \
        if (!agrpc_anonymous_logging.exchange(true,                             \
                                             ::std::memory_order_seq_cst)) {   \
          auto agrpc_anonymous_really_write =                                   \
              agrpc_anonymous_log_last_occurs.load(                             \
                  ::std::memory_order_relaxed) +                               \
                  ::std::chrono::seconds(N) <                                  \
              agrpc_anonymous_now;                                              \
          if (agrpc_anonymous_really_write) {                                   \
            agrpc_anonymous_log_last_occurs.store(agrpc_anonymous_now,           \
                                                 ::std::memory_order_relaxed); \
          }                                                                    \
          /* Now that we've determined whether we should write and updated */  \
          /* the timestamp, free the lock ASAP. */                             \
          agrpc_anonymous_logging.store(false, ::std::memory_order_seq_cst);    \
          if (agrpc_anonymous_really_write) {                                   \
            /* The log may be written now. */                                  \
            ::google::LogMessage(                                              \
                __FILE__, __LINE__, ::google::GLOG_##severity,                 \
                0, /* Number of occurrence. Not supplied for perf. reasons. */ \
                &::google::LogMessage::SendToLog)                              \
                    .stream()                                                  \
                << ::agrpc::internal::logging::FormatLog(__FILE__, __LINE__,    \
                                                        __VA_ARGS__);          \
          }                                                                    \
        }                                                                      \
      }();                                                                     \
    }                                                                          \
  } while (0)

#define AGRPC_DETAIL_LOGGING_LOG_EVERY_N_SECOND_IF_IMPL(expr, N, severity, ...) \
  do {                                                                         \
    if (expr) {                                                                \
      AGRPC_DETAIL_LOGGING_LOG_EVERY_N_SECOND_IMPL(N, severity, __VA_ARGS__);   \
    }                                                                          \
  } while (0)

#endif  // AGRPC_BASE_LOGGING_H_
