include(CMakeParseArguments)
include(agrpc_installed_test)

# agrpc_cc_test()
#
# CMake function to imitate Bazel's cc_test rule.
#
# Parameters:
# NAME: name of target. This name is used for the generated executable and
# SRCS: List of source files for the binary
# DATA: List of other targets and files required for this binary
# DEPS: List of other libraries to be linked in to the binary targets
# COPTS: List of private compile options
# DEFINES: List of public defines
# LINKOPTS: List of link options
# LABELS: Additional labels to apply to the test. The package path is added automatically.
#
# Note: agrpc_cc_test will create a binary called ${PACKAGE_NAME}_${NAME}, e.g.
# agrpc_base_foo_test.
#
# Usage:
# agrpc_cc_library(
#   NAME
#     awesome
#   HDRS
#     "a.h"
#   SRCS
#     "a.cc"
#   PUBLIC
# )
#
# agrpc_cc_test(
#   NAME
#     awesome_test
#   SRCS
#     "awesome_test.cc"
#   DEPS
#     gtest_main
#     agrpc::awesome
# )
function(agrpc_cc_test)
  if(NOT AGRPC_BUILD_TESTS)
    return()
  endif()

  cmake_parse_arguments(_RULE "" "NAME"
                        "SRCS;COPTS;DEFINES;LINKOPTS;DATA;DEPS;LABELS" ${ARGN})

  # Prefix the library with the package name, so we get: agrpc_package_name
  agrpc_package_name(_PACKAGE_NAME)
  agrpc_package_ns(_PACKAGE_NS)
  set(_NAME "${_PACKAGE_NAME}_${_RULE_NAME}")

  add_executable(${_NAME} "")
  # Alias the agrpc_package_name test binary to agrpc::package::name. This lets
  # us more clearly map to Bazel and makes it possible to disambiguate the
  # underscores in paths vs. the separators.
  add_executable(${_PACKAGE_NS}::${_RULE_NAME} ALIAS ${_NAME})

  # If the test binary name matches the package then treat it as a default. For
  # example, foo/bar/ library 'bar' would end up as 'foo::bar'. This isn't
  # likely to be common for tests, but is consistent with the behavior for
  # libraries.
  agrpc_package_dir(_PACKAGE_DIR)
  if(${_RULE_NAME} STREQUAL ${_PACKAGE_DIR})
    add_executable(${_PACKAGE_NS} ALIAS ${_NAME})
  endif()

  set_target_properties(${_NAME} PROPERTIES OUTPUT_NAME "${_RULE_NAME}")
  target_sources(${_NAME} PRIVATE ${_RULE_SRCS})
  target_include_directories(
    ${_NAME} SYSTEM PUBLIC "$<BUILD_INTERFACE:${AGRPC_SOURCE_DIR}>"
                           "$<BUILD_INTERFACE:${AGRPC_BINARY_DIR}>")
  target_compile_definitions(${_NAME} PUBLIC ${_RULE_DEFINES})
  target_compile_options(${_NAME} PRIVATE ${AGRPC_DEFAULT_COPTS} ${_RULE_COPTS})
  target_link_options(${_NAME} PRIVATE ${AGRPC_DEFAULT_LINKOPTS}
                      ${_RULE_LINKOPTS})

  # Replace dependencies passed by ::name with agrpc::package::name

  list(TRANSFORM _RULE_DEPS REPLACE "^::" "${_PACKAGE_NS}::")

  target_link_libraries(${_NAME} PUBLIC ${_RULE_DEPS})
  agrpc_add_data_dependencies(NAME ${_NAME} DATA ${_RULE_DATA})

  # Add all AGRPC targets to a folder in the IDE for organization.
  set_property(TARGET ${_NAME} PROPERTY FOLDER ${AGRPC_IDE_FOLDER}/test)

  set_property(TARGET ${_NAME} PROPERTY CXX_STANDARD ${AGRPC_CXX_STANDARD})
  set_property(TARGET ${_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

  string(REPLACE "::" "/" _PACKAGE_PATH ${_PACKAGE_NS})
  set(_TEST_NAME "${_PACKAGE_PATH}/${_RULE_NAME}")

  list(APPEND _RULE_LABELS "${_PACKAGE_PATH}")

  agrpc_add_installed_test(
    TEST_NAME
    "${_TEST_NAME}"
    LABELS
    "${_RULE_LABELS}"
    COMMAND
    # We run all our tests through a custom test runner to allow temp directory
    # cleanup upon test completion.
    "${CMAKE_SOURCE_DIR}/build_tools/cmake/run_test.${AGRPC_HOST_SCRIPT_EXT}"
    "$<TARGET_FILE:${_NAME}>"
    INSTALLED_COMMAND
    # Must match install destination below.
    "${_PACKAGE_PATH}/$<TARGET_FILE_NAME:${_NAME}>")

  install(
    TARGETS ${_NAME}
    DESTINATION "tests/${_PACKAGE_PATH}"
    COMPONENT Tests)
endfunction()
