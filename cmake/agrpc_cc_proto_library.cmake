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

  set(_OUTS "")
  foreach(_SRC ${_RULE_SRCS})
    get_filename_component(_SRC_FILENAME ${_SRC} NAME_WE)
    set(_PROTOBUF_PROTOC_HDR "${_SRC_FILENAME}.pb.h")
    set(_PROTOBUF_PROTOC_SRC "${_SRC_FILENAME}.pb.cc")
    list(APPEND _OUTS _PROTOBUF_PROTOC_HDR _PROTOBUF_PROTOC_SRC)
  endforeach()
  list(TRANSFORM _OUTS PREPEND "${CMAKE_CURRENT_BINARY_DIR}/")

  agrpc_get_executable_path(_PROTOC_BIN protoc)
  add_custom_command(
    OUTPUT ${_OUTS}
    COMMAND "${_PROTOC_BIN}" --cpp_out "${CMAKE_CURRENT_BINARY_DIR}" -I
            "${CMAKE_CURRENT_SOURCE_DIR}" ${_RULE_PROTOC_ARGS} "${_RULE_SRCS}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    MAIN_DEPENDENCY ${_RULE_SRCS}
    DEPENDS ${_RULE_SRCS}
    COMMAND_EXPAND_LISTS)

  set(_GEN_TARGET "${_NAME}_gen")
  add_custom_target(${_GEN_TARGET} DEPENDS ${_OUTS})

  add_library(${_NAME} STATIC "")
  add_dependencies(${_NAME} ${_GEN_TARGET})
  set_source_files_properties(
    ${_PROTOBUF_PROTOC_HDR} ${_PROTOBUF_PROTOC_SRC} PROPERTIES GENERATED TRUE)
  target_sources(${_NAME} PRIVATE ${_PROTOBUF_PROTOC_SRC} ${_PROTOBUF_PROTOC_HDR})
  target_include_directories(
    ${_NAME} SYSTEM PUBLIC "$<BUILD_INTERFACE:${AGRPC_SOURCE_DIR}>"
                           "$<BUILD_INTERFACE:${AGRPC_BINARY_DIR}>"
                           ${CMAKE_CURRENT_BINARY_DIR})
  target_link_options(${_NAME} PRIVATE ${AGRPC_DEFAULT_LINKOPTS})
  target_link_libraries(${_NAME} PUBLIC protobuf)

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
