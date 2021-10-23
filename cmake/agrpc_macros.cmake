include(CMakeParseArguments)

# -------------------------------------------------------------------------------
# Missing CMake Variables
# -------------------------------------------------------------------------------

if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
  set(AGRPC_HOST_SCRIPT_EXT "bat")
  # https://gitlab.kitware.com/cmake/cmake/-/issues/17553
  set(AGRPC_HOST_EXECUTABLE_SUFFIX ".exe")
else()
  set(AGRPC_HOST_SCRIPT_EXT "sh")
  set(AGRPC_HOST_EXECUTABLE_SUFFIX "")
endif()

# -------------------------------------------------------------------------------
# General utilities
# -------------------------------------------------------------------------------

# agrpc_to_bool
#
# Sets `variable` to `ON` if `value` is true and `OFF` otherwise.
function(agrpc_to_bool VARIABLE VALUE)
  if(VALUE)
    set(${VARIABLE}
        "ON"
        PARENT_SCOPE)
  else()
    set(${VARIABLE}
        "OFF"
        PARENT_SCOPE)
  endif()
endfunction()

# agrpc_append_list_to_string
#
# Joins ${ARGN} together as a string separated by " " and appends it to
# ${VARIABLE}.
function(agrpc_append_list_to_string VARIABLE)
  if(NOT "${ARGN}" STREQUAL "")
    string(JOIN " " _ARGN_STR ${ARGN})
    set(${VARIABLE}
        "${${VARIABLE}} ${_ARGN_STR}"
        PARENT_SCOPE)
  endif()
endfunction()

# -------------------------------------------------------------------------------
# Packages and Paths
# -------------------------------------------------------------------------------

# Sets ${PACKAGE_NS} to the AGRPC-root relative package name in C++ namespace
# format (::).
#
# Example when called from agrpc/base/CMakeLists.txt: agrpc::base
function(agrpc_package_ns PACKAGE_NS)
  string(REPLACE ${AGRPC_ROOT_DIR} "" _PACKAGE ${CMAKE_CURRENT_LIST_DIR})
  string(SUBSTRING ${_PACKAGE} 1 -1 _PACKAGE)
  string(REPLACE "/" "::" _PACKAGE_NS ${_PACKAGE})
  set(${PACKAGE_NS}
      ${_PACKAGE_NS}
      PARENT_SCOPE)
endfunction()

# Sets ${PACKAGE_NAME} to the AGRPC-root relative package name.
#
# Example when called from agrpc/base/CMakeLists.txt: agrpc_base
function(agrpc_package_name PACKAGE_NAME)
  agrpc_package_ns(_PACKAGE_NS)
  string(REPLACE "::" "_" _PACKAGE_NAME ${_PACKAGE_NS})
  set(${PACKAGE_NAME}
      ${_PACKAGE_NAME}
      PARENT_SCOPE)
endfunction()

# Sets ${PACKAGE_PATH} to the AGRPC-root relative package path.
#
# Example when called from agrpc/base/CMakeLists.txt: agrpc/base
function(agrpc_package_path PACKAGE_PATH)
  agrpc_package_ns(_PACKAGE_NS)
  string(REPLACE "::" "/" _PACKAGE_PATH ${_PACKAGE_NS})
  set(${PACKAGE_PATH}
      ${_PACKAGE_PATH}
      PARENT_SCOPE)
endfunction()

# Sets ${PACKAGE_DIR} to the directory name of the current package.
#
# Example when called from agrpc/base/CMakeLists.txt: base
function(agrpc_package_dir PACKAGE_DIR)
  agrpc_package_ns(_PACKAGE_NS)
  string(FIND ${_PACKAGE_NS} "::" _END_OFFSET REVERSE)
  math(EXPR _END_OFFSET "${_END_OFFSET} + 2")
  string(SUBSTRING ${_PACKAGE_NS} ${_END_OFFSET} -1 _PACKAGE_DIR)
  set(${PACKAGE_DIR}
      ${_PACKAGE_DIR}
      PARENT_SCOPE)
endfunction()

# agrpc_get_executable_path
#
# Gets the path to an executable in a cross-compilation-aware way. This should
# be used when accessing binaries that are used as part of the build, such as
# for generating files used for later build steps.
#
# Parameters: - OUTPUT_PATH_VAR: variable name for receiving the path to the
# built target. - EXECUTABLE: the executable to get its path. Note that this
# needs to be the name of the executable target when not cross compiling and the
# basename of the binary when importing a binary from a host build. Thus this
# should be the global unqualified name of the binary, not the fully-specified
# name.
function(agrpc_get_executable_path OUTPUT_PATH_VAR EXECUTABLE)
  if(NOT DEFINED AGRPC_HOST_BINARY_ROOT OR TARGET "${EXECUTABLE}")
    # We can either expect the target to be defined as part of this CMake
    # invocation (if not cross compiling) or the target is defined already.
    set(${OUTPUT_PATH_VAR}
        "$<TARGET_FILE:${EXECUTABLE}>"
        PARENT_SCOPE)
  else()
    # The target won't be directly defined by this CMake invocation so check for
    # an already built executable at AGRPC_HOST_BINARY_ROOT. If we find it, add
    # it as an imported target so it gets picked up on later invocations.
    set(_EXECUTABLE_PATH
        "${AGRPC_HOST_BINARY_ROOT}/bin/${EXECUTABLE}${AGRPC_HOST_EXECUTABLE_SUFFIX}"
    )
    if(EXISTS ${_EXECUTABLE_PATH})
      add_executable("${EXECUTABLE}" IMPORTED GLOBAL)
      set_property(TARGET "${EXECUTABLE}" PROPERTY IMPORTED_LOCATION
                                                   "${_EXECUTABLE_PATH}")
      set(${OUTPUT_PATH_VAR}
          "$<TARGET_FILE:${EXECUTABLE}>"
          PARENT_SCOPE)
    else()
      message(
        FATAL_ERROR
          "Could not find '${EXECUTABLE}' at '${_EXECUTABLE_PATH}'. "
          "Ensure that AGRPC_HOST_BINARY_ROOT points to installed binaries.")
    endif()
  endif()
endfunction()

# -------------------------------------------------------------------------------
# select()-like Evaluation
# -------------------------------------------------------------------------------

# Appends ${OPTS} with a list of values based on the current compiler.
#
# Example: agrpc_select_compiler_opts(COPTS CLANG "-Wno-foo" "-Wno-bar" CLANG_CL
# "/W3" GCC "-Wsome-old-flag" MSVC "/W3" )
#
# Note that variables are allowed, making it possible to share options between
# different compiler targets.
function(agrpc_select_compiler_opts OPTS)
  cmake_parse_arguments(
    PARSE_ARGV 1 _AGRPC_SELECTS "" ""
    "ALL;CLANG;CLANG_CL;MSVC;GCC;CLANG_OR_GCC;MSVC_OR_CLANG_CL")
  set(_OPTS)
  list(APPEND _OPTS "${_AGRPC_SELECTS_ALL}")
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    list(APPEND _OPTS "${_AGRPC_SELECTS_GCC}")
    list(APPEND _OPTS "${_AGRPC_SELECTS_CLANG_OR_GCC}")
  elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    if(MSVC)
      list(APPEND _OPTS ${_AGRPC_SELECTS_CLANG_CL})
      list(APPEND _OPTS ${_AGRPC_SELECTS_MSVC_OR_CLANG_CL})
    else()
      list(APPEND _OPTS ${_AGRPC_SELECTS_CLANG})
      list(APPEND _OPTS ${_AGRPC_SELECTS_CLANG_OR_GCC})
    endif()
  elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    list(APPEND _OPTS ${_AGRPC_SELECTS_MSVC})
    list(APPEND _OPTS ${_AGRPC_SELECTS_MSVC_OR_CLANG_CL})
  else()
    message(ERROR "Unknown compiler: ${CMAKE_CXX_COMPILER}")
    list(APPEND _OPTS "")
  endif()
  set(${OPTS}
      ${_OPTS}
      PARENT_SCOPE)
endfunction()

# -------------------------------------------------------------------------------
# Data dependencies
# -------------------------------------------------------------------------------

# Adds 'data' dependencies to a target.
#
# Parameters: NAME: name of the target to add data dependencies to DATA: List of
# targets and/or files in the source tree. Files should use the same format as
# targets (i.e. agrpc::package::subpackage::file.txt)
function(agrpc_add_data_dependencies)
  cmake_parse_arguments(_RULE "" "NAME" "DATA" ${ARGN})

  if(NOT _RULE_DATA)
    return()
  endif()

  foreach(_DATA_LABEL ${_RULE_DATA})
    if(TARGET ${_DATA_LABEL})
      add_dependencies(${_RULE_NAME} ${_DATA_LABEL})
    else()
      # Not a target, assume to be a file instead.
      string(REPLACE "::" "/" _FILE_PATH ${_DATA_LABEL})

      # Create a target which copies the data file into the build directory. If
      # this file is included in multiple rules, only create the target once.
      string(REPLACE "::" "_" _DATA_TARGET ${_DATA_LABEL})
      if(NOT TARGET ${_DATA_TARGET})
        set(_INPUT_PATH "${CMAKE_SOURCE_DIR}/${_FILE_PATH}")
        set(_OUTPUT_PATH "${CMAKE_BINARY_DIR}/${_FILE_PATH}")
        add_custom_target(
          ${_DATA_TARGET} COMMAND ${CMAKE_COMMAND} -E copy ${_INPUT_PATH}
                                  ${_OUTPUT_PATH})
      endif()

      add_dependencies(${_RULE_NAME} ${_DATA_TARGET})
    endif()
  endforeach()
endfunction()

# -------------------------------------------------------------------------------
# Tool symlinks
# -------------------------------------------------------------------------------

# agrpc_symlink_tool
#
# Adds a command to TARGET which symlinks a tool from elsewhere
# (FROM_TOOL_TARGET_NAME) to a local file name (TO_EXE_NAME) in the current
# binary directory.
#
# Parameters: TARGET: Local target to which to add the symlink command (i.e. an
# agrpc_py_library, etc). FROM_TOOL_TARGET: Target of the tool executable that
# is the source of the link. TO_EXE_NAME: The executable name to output in the
# current binary dir.
function(agrpc_symlink_tool)
  cmake_parse_arguments(ARG "" "TARGET;FROM_TOOL_TARGET;TO_EXE_NAME" "" ${ARGN})

  # Transform TARGET
  agrpc_package_ns(_PACKAGE_NS)
  agrpc_package_name(_PACKAGE_NAME)
  set(_TARGET "${_PACKAGE_NAME}_${ARG_TARGET}")
  set(_FROM_TOOL_TARGET ${ARG_FROM_TOOL_TARGET})
  set(_TO_TOOL_PATH
      "${CMAKE_CURRENT_BINARY_DIR}/${ARG_TO_EXE_NAME}${CMAKE_EXECUTABLE_SUFFIX}"
  )
  get_filename_component(_TO_TOOL_DIR "${_TO_TOOL_PATH}" DIRECTORY)

  add_custom_command(
    TARGET "${_TARGET}"
    BYPRODUCTS
      "${CMAKE_CURRENT_BINARY_DIR}/${ARG_TO_EXE_NAME}${CMAKE_EXECUTABLE_SUFFIX}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${_TO_TOOL_DIR}"
    COMMAND ${CMAKE_COMMAND} -E create_symlink
            "$<TARGET_FILE:${_FROM_TOOL_TARGET}>" "${_TO_TOOL_PATH}")
endfunction()
