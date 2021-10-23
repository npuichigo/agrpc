agrpc_add_all_subdirs()

agrpc_cc_library(
  NAME
    align
  HDRS
    "align.h"
  PUBLIC
)

agrpc_cc_library(
  NAME
    chrono
  HDRS
    "chrono.h"
  SRCS
    "chrono.cc"
  DEPS
    ::align
  PUBLIC
)

agrpc_cc_test(
  NAME
    chrono_test
  SRCS
    "chrono_test.cc"
  DEPS
    gtest
    gtest_main
    ::chrono
)

agrpc_cc_library(
  NAME
    likely
  HDRS
    "likely.h"
  PUBLIC
)

agrpc_cc_library(
  NAME
    logging
  HDRS
    "logging.h"
  SRCS
    "logging.cc"
  DEPS
    ::internal::macros
    ::chrono
    ::likely
    fmt
    glog
  PUBLIC
)

agrpc_cc_test(
  NAME
    logging_test
  SRCS
    "logging_test.cc"
  DEPS
    ::logging
    gtest
    gtest_main
)

agrpc_cc_test(
  NAME
    logging_benchmark
  SRCS
    "logging_benchmark.cc"
  DEPS
    ::logging
    benchmark
    benchmark_main
)
