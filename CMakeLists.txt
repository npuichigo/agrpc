cmake_minimum_required(VERSION 3.15)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(
  agrpc
  LANGUAGES C CXX
  VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(AGRPC_IDE_FOLDER AGRPC)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(DEFAULT_CMAKE_BUILD_TYPE "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type selected, default to ${DEFAULT_CMAKE_BUILD_TYPE}")
  set(CMAKE_BUILD_TYPE "${DEFAULT_CMAKE_BUILD_TYPE}" CACHE STRING "Build type (default ${DEFAULT_CMAKE_BUILD_TYPE})" FORCE)
endif()

#-------------------------------------------------------------------------------
# AGRPC utility definitions
#-------------------------------------------------------------------------------

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_BINARY_DIR})

include(agrpc_macros)
include(agrpc_copts)
include(agrpc_cc_binary)
include(agrpc_cc_library)
include(agrpc_cc_test)
include(agrpc_cc_proto_library)
include(agrpc_add_all_subdirs)
include(third_party)

set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

# Third-party directories that we include using the simpler add_subdirectory
# method
add_subdirectory(third_party/libunifex)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR})

add_subdirectory(agrpc)
