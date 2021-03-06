include(CMakeParseArguments)

# agrpc_cc_library()
#
# CMake function to imitate Bazel's cc_library rule.
#
# Parameters:
# NAME: name of target (see Note)
# HDRS: List of public header files for the library
# TEXTUAL_HDRS: List of public header files that cannot be compiled on their own
# SRCS: List of source files for the library
# DATA: List of other targets and files required for this binary
# DEPS: List of other libraries to be linked in to the binary targets
# COPTS: List of private compile options
# DEFINES: List of public defines
# INCLUDES: Include directories to add to dependencies
# LINKOPTS: List of link options
# PUBLIC: Add this so that this library will be exported under agrpc::
# Also in IDE, target will appear in AGRPC folder while non PUBLIC will be in AGRPC/internal.
# TESTONLY: When added, this target will only be built if user passes -DAGRPC_BUILD_TESTS=ON to CMake.
# SHARED: If set, will compile to a shared object.
#
# Note: By default, agrpc_cc_library will always create a library named
# agrpc_${NAME}, and alias target agrpc::${NAME}. The agrpc:: form should always
# be used. This is to reduce namespace pollution.
#
# agrpc_cc_library(
#   NAME
#     awesome
#   HDRS
#     "a.h"
#   SRCS
#     "a.cc"
# )
#
# agrpc_cc_library(
#   NAME
#     fantastic_lib
#   SRCS
#     "b.cc"
#   DEPS
#     agrpc::package::awesome # not "awesome" !
#   PUBLIC
# )
#
# agrpc_cc_library(
#   NAME
#     main_lib
#   DEPS
#     agrpc::package::fantastic_lib
# )
function(agrpc_cc_library)
  cmake_parse_arguments(
    _RULE "PUBLIC;TESTONLY;SHARED" "NAME"
    "HDRS;TEXTUAL_HDRS;SRCS;COPTS;DEFINES;LINKOPTS;DATA;DEPS;INCLUDES" ${ARGN})

  if(_RULE_TESTONLY AND NOT AGRPC_BUILD_TESTS)
    return()
  endif()

  # Replace dependencies passed by ::name with agrpc::package::name
  agrpc_package_ns(_PACKAGE_NS)
  list(TRANSFORM _RULE_DEPS REPLACE "^::" "${_PACKAGE_NS}::")

  # Prefix the library with the package name, so we get: agrpc_package_name.
  agrpc_package_name(_PACKAGE_NAME)
  set(_NAME "${_PACKAGE_NAME}_${_RULE_NAME}")

  # Check if this is a header-only library. Note that as of February 2019, many
  # popular OS's (for example, Ubuntu 16.04 LTS) only come with cmake 3.5 by
  # default.  For this reason, we can't use list(FILTER...)
  set(_CC_SRCS "${_RULE_SRCS}")
  foreach(src_file IN LISTS _CC_SRCS)
    if(${src_file} MATCHES ".*\\.(h|inc)")
      list(REMOVE_ITEM _CC_SRCS "${src_file}")
    endif()
  endforeach()
  if("${_CC_SRCS}" STREQUAL "")
    set(_RULE_IS_INTERFACE 1)
  else()
    set(_RULE_IS_INTERFACE 0)
  endif()

  if(NOT _RULE_IS_INTERFACE)
    if(_RULE_SHARED)
      add_library(${_NAME} SHARED "")
    else()
      add_library(${_NAME} STATIC "")
    endif()

    target_sources(${_NAME} PRIVATE ${_RULE_SRCS} ${_RULE_TEXTUAL_HDRS}
                                    ${_RULE_HDRS})
    target_include_directories(
      ${_NAME} SYSTEM PUBLIC "$<BUILD_INTERFACE:${AGRPC_SOURCE_DIR}>"
                             "$<BUILD_INTERFACE:${AGRPC_BINARY_DIR}>")
    target_include_directories(${_NAME}
                               PUBLIC "$<BUILD_INTERFACE:${_RULE_INCLUDES}>")
    target_compile_options(${_NAME} PRIVATE ${AGRPC_DEFAULT_COPTS}
                                            ${_RULE_COPTS})
    target_link_options(${_NAME} PRIVATE ${AGRPC_DEFAULT_LINKOPTS}
                        ${_RULE_LINKOPTS})
    target_link_libraries(${_NAME} PUBLIC ${_RULE_DEPS})

    agrpc_add_data_dependencies(NAME ${_NAME} DATA ${_RULE_DATA})
    target_compile_definitions(${_NAME} PUBLIC ${_RULE_DEFINES})

    # Add all AGRPC targets to a folder in the IDE for organization.
    if(_RULE_PUBLIC)
      set_property(TARGET ${_NAME} PROPERTY FOLDER ${AGRPC_IDE_FOLDER})
    elseif(_RULE_TESTONLY)
      set_property(TARGET ${_NAME} PROPERTY FOLDER ${AGRPC_IDE_FOLDER}/test)
    else()
      set_property(TARGET ${_NAME} PROPERTY FOLDER ${AGRPC_IDE_FOLDER}/internal)
    endif()

    # INTERFACE libraries can't have the CXX_STANDARD property set.
    set_property(TARGET ${_NAME} PROPERTY CXX_STANDARD ${AGRPC_CXX_STANDARD})
    set_property(TARGET ${_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
  else()
    # Generating header-only library.
    add_library(${_NAME} INTERFACE)
    target_include_directories(
      ${_NAME} SYSTEM INTERFACE "$<BUILD_INTERFACE:${AGRPC_SOURCE_DIR}>"
                                "$<BUILD_INTERFACE:${AGRPC_BINARY_DIR}>")
    target_link_options(${_NAME} INTERFACE ${AGRPC_DEFAULT_LINKOPTS}
                        ${_RULE_LINKOPTS})
    target_link_libraries(${_NAME} INTERFACE ${_RULE_DEPS})
    agrpc_add_data_dependencies(NAME ${_NAME} DATA ${_RULE_DATA})
    target_compile_definitions(${_NAME} INTERFACE ${_RULE_DEFINES})
  endif()

  # Alias the agrpc_package_name library to agrpc::package::name. This lets us
  # more clearly map to Bazel and makes it possible to disambiguate the
  # underscores in paths vs. the separators.
  add_library(${_PACKAGE_NS}::${_RULE_NAME} ALIAS ${_NAME})

  # If the library name matches the final component of the package then treat it
  # as a default. For example, foo/bar/ library 'bar' would end up as
  # 'foo::bar'.
  agrpc_package_dir(_PACKAGE_DIR)
  if(${_RULE_NAME} STREQUAL ${_PACKAGE_DIR})
    add_library(${_PACKAGE_NS} ALIAS ${_NAME})
  endif()
endfunction()
