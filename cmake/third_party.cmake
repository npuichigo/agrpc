include(ExternalProject)

find_package(Git REQUIRED)

set(THIRD_PARTY_PATH
    "${CMAKE_BINARY_DIR}/third_party"
    CACHE STRING
          "A path setting third party libraries download & build directories.")
set(THIRD_PARTY_CACHE_PATH
    "${CMAKE_SOURCE_DIR}"
    CACHE STRING
          "A path cache third party source code to avoid repeated download.")

set(THIRD_PARTY_BUILD_TYPE Release)
set(third_party_deps)

# cache function to avoid repeat download code of third_party. This function has
# 4 parameters, URL / REPOSITORY / TAG / DIR: 1. URL:           specify download
# url of 3rd party 2. REPOSITORY:    specify git REPOSITORY of 3rd party 3. TAG:
# specify git tag/branch/commitID of 3rd party 4. DIR:           overwrite the
# original SOURCE_DIR when cache directory
#
# The function Return 1 PARENT_SCOPE variables: - ${TARGET}_DOWNLOAD_CMD: Simply
# place "${TARGET}_DOWNLOAD_CMD" in ExternalProject_Add, and you no longer need
# to set any download steps in ExternalProject_Add. For example:
# Cache_third_party(${TARGET} REPOSITORY ${TARGET_REPOSITORY} TAG ${TARGET_TAG}
# DIR        ${TARGET_SOURCE_DIR})
function(cache_third_party TARGET)
  set(options "")
  set(oneValueArgs URL REPOSITORY TAG DIR)
  set(multiValueArgs "")
  cmake_parse_arguments(cache_third_party "${options}" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN})

  string(REPLACE "extern_" "" TARGET_NAME ${TARGET})
  string(REGEX REPLACE "[0-9]+" "" TARGET_NAME ${TARGET_NAME})
  string(TOUPPER ${TARGET_NAME} TARGET_NAME)
  if(cache_third_party_REPOSITORY)
    set(${TARGET_NAME}_DOWNLOAD_CMD GIT_REPOSITORY
                                    ${cache_third_party_REPOSITORY})
    if(cache_third_party_TAG)
      list(APPEND ${TARGET_NAME}_DOWNLOAD_CMD GIT_TAG ${cache_third_party_TAG})
    endif()
  elseif()
    set(${TARGET_NAME}_DOWNLOAD_CMD URL ${cache_third_party_URL})
  else()
    message(
      FATAL_ERROR "Download link (Git repo or URL) must be specified for cache!"
    )
  endif()

  # Pass ${TARGET_NAME}_DOWNLOAD_CMD to parent scope, the double quotation marks
  # can't be removed
  set(${TARGET_NAME}_DOWNLOAD_CMD
      "${${TARGET_NAME}_DOWNLOAD_CMD}"
      PARENT_SCOPE)
endfunction()

macro(UNSET_VAR VAR_NAME)
  unset(${VAR_NAME} CACHE)
  unset(${VAR_NAME})
endmacro()

# Function to Download the dependencies during compilation This function has 2
# parameters, URL / DIRNAME: 1. URL: The download url of 3rd dependencies 2.
# NAME: The name of file, that determine the dirname
#
function(file_download_and_uncompress URL NAME)
  set(options "")
  set(oneValueArgs MD5)
  set(multiValueArgs "")
  cmake_parse_arguments(URL "${options}" "${oneValueArgs}" "${multiValueArgs}"
                        ${ARGN})
  message(STATUS "Download dependence[${NAME}] from ${URL}, MD5: ${URL_MD5}")
  set(${NAME}_INCLUDE_DIR
      ${THIRD_PARTY_PATH}/${NAME}/data
      PARENT_SCOPE)
  ExternalProject_Add(
    download_${NAME}
    ${EXTERNAL_PROJECT_LOG_ARGS}
    PREFIX ${THIRD_PARTY_PATH}/${NAME}
    URL ${URL}
    URL_MD5 ${URL_MD5}
    TIMEOUT 120
    DOWNLOAD_DIR ${THIRD_PARTY_PATH}/${NAME}/data/
    SOURCE_DIR ${THIRD_PARTY_PATH}/${NAME}/data/
    DOWNLOAD_NO_PROGRESS 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    UPDATE_COMMAND ""
    INSTALL_COMMAND "")
  set(third_party_deps
      ${third_party_deps} download_${NAME}
      PARENT_SCOPE)
endfunction()

if(${CMAKE_VERSION} VERSION_GREATER "3.5.2")
  set(SHALLOW_CLONE GIT_SHALLOW TRUE) # adds --depth=1 arg to git clone of
                                      # External_Projects
endif()

# include third_party according to flags
include(external/fmt) # download, build, install fmt
include(external/gflags) # download, build, install gflags
include(external/glog) # download, build, install glog
include(external/zlib) # download, build, install zlib
include(external/protobuf) # download, build, install protobuf

list(APPEND third_party_deps extern_fmt extern_gflags extern_glog)

if(AGRPC_BUILD_TESTS)
  include(external/gtest) # download, build, install gtest
  include(external/benchmark) # download, build, install benchmark
  list(APPEND third_party_deps extern_gtest extern_benchmark)
endif()

# Creat a target named "third_party", which can compile external dependencies on
# all platform(windows/linux/mac)
add_custom_target(third_party ALL DEPENDS ${third_party_deps})
