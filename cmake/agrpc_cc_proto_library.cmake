include(CMakeParseArguments)

# agrpc_cc_proto_library()
#
# CMake function to invoke the protoc compiler.
#
# Parameters:
# NAME: name of target (see Note)
# SRCS: List of source files for the library
# PROTOC_ARGS: List of protobuf arguments.
# PUBLIC: Add this so that this library will be exported under agrpc::
# Also in IDE, target will appear in AGRPC folder while non PUBLIC will be in AGRPC/internal.
# TESTONLY: When added, this target will only be built if user passes -DAGRPC_BUILD_TESTS=ON to CMake.
#
# Note: By default, agrpc_cc_proto_library will always create a library named
# ${NAME}, and alias target agrpc::${NAME}. The agrpc:: form should always be
# used. This is to reduce namespace pollution.
#
# agrpc_cc_proto_library(
#   NAME
#     some_def
#   SRCS
#     "some_def.proto"
#   PUBLIC
# )
#
# agrpc_cc_binary(
#   NAME
#     main_lib
#   ...
#   DEPS
#     agrpc::schemas::some_def )
function(agrpc_cc_proto_library)
  cmake_parse_arguments(_RULE "PUBLIC;TESTONLY" "NAME" "SRCS;PROTOC_ARGS"
                        ${ARGN})

  if(_RULE_TESTONLY AND NOT AGRPC_BUILD_TESTS)
    return()
  endif()

  # Prefix the library with the package name, so we get: agrpc_package_name
  agrpc_package_name(_PACKAGE_NAME)
  set(_NAME "${_PACKAGE_NAME}_${_RULE_NAME}")

  protobuf_generate(PROTOS ${_RULE_SRCS} LANGUAGE cpp OUT_VAR _OUTS)

  add_library(${_NAME} STATIC "")
  set_source_files_properties(${_OUTS} PROPERTIES GENERATED TRUE)
  target_sources(${_NAME} PRIVATE ${_OUTS})
  target_include_directories(
    ${_NAME} SYSTEM PUBLIC "$<BUILD_INTERFACE:${AGRPC_SOURCE_DIR}>"
                           "$<BUILD_INTERFACE:${AGRPC_BINARY_DIR}>"
                           ${CMAKE_CURRENT_BINARY_DIR})
  target_link_options(${_NAME} PRIVATE ${AGRPC_DEFAULT_LINKOPTS})
  target_link_libraries(${_NAME} PUBLIC protobuf::libprotobuf)

  # Add all AGRPC targets to a folder in the IDE for organization.
  if(_RULE_PUBLIC)
    set_property(TARGET ${_NAME} PROPERTY FOLDER ${AGRPC_IDE_FOLDER})
  elseif(_RULE_TESTONLY)
    set_property(TARGET ${_NAME} PROPERTY FOLDER ${AGRPC_IDE_FOLDER}/test)
  else()
    set_property(TARGET ${_NAME} PROPERTY FOLDER ${AGRPC_IDE_FOLDER}/internal)
  endif()

  set_property(TARGET ${_NAME} PROPERTY CXX_STANDARD ${AGRPC_CXX_STANDARD})
  set_property(TARGET ${_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

  agrpc_package_ns(_PACKAGE_NS)
  add_library(${_PACKAGE_NS}::${_RULE_NAME} ALIAS ${_NAME})
endfunction()
