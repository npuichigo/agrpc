include(ExternalProject)

set(GFLAGS_PREFIX_DIR ${THIRD_PARTY_PATH}/gflags)
set(GFLAGS_SOURCE_DIR ${THIRD_PARTY_PATH}/gflags/src/extern_gflags)
set(GFLAGS_INSTALL_DIR ${THIRD_PARTY_PATH}/install/gflags)
set(GFLAGS_INCLUDE_DIR
    "${GFLAGS_INSTALL_DIR}/include"
    CACHE PATH "gflags include directory." FORCE)
set(GFLAGS_REPOSITORY ${GIT_URL}/gflags/gflags.git)
set(GFLAGS_TAG v2.2.2)
if(WIN32)
  set(GFLAGS_LIBRARIES
      "${GFLAGS_INSTALL_DIR}/lib/gflags_static.lib"
      CACHE FILEPATH "GFLAGS_LIBRARIES" FORCE)
else()
  set(GFLAGS_LIBRARIES
      "${GFLAGS_INSTALL_DIR}/lib/libgflags.a"
      CACHE FILEPATH "GFLAGS_LIBRARIES" FORCE)
endif()

include_directories(${GFLAGS_INCLUDE_DIR})

cache_third_party(
  extern_gflags
  REPOSITORY ${GFLAGS_REPOSITORY}
  TAG ${GFLAGS_TAG}
  DIR GFLAGS_SOURCE_DIR)

ExternalProject_Add(
  extern_gflags
  ${SHALLOW_CLONE} ${EXTERNAL_PROJECT_LOG_ARGS} ${GFLAGS_DOWNLOAD_CMD}
  PREFIX ${GFLAGS_PREFIX_DIR}
  UPDATE_COMMAND ""
  SOURCE_DIR ${GFLAGS_SOURCE_DIR}
  CMAKE_ARGS -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
             -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
             -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
             -DCMAKE_CXX_FLAGS_RELEASE=${CMAKE_CXX_FLAGS_RELEASE}
             -DCMAKE_CXX_FLAGS_DEBUG=${CMAKE_CXX_FLAGS_DEBUG}
             -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
             -DCMAKE_C_FLAGS_DEBUG=${CMAKE_C_FLAGS_DEBUG}
             -DCMAKE_C_FLAGS_RELEASE=${CMAKE_C_FLAGS_RELEASE}
             -DBUILD_STATIC_LIBS=ON
             -DCMAKE_INSTALL_PREFIX=${GFLAGS_INSTALL_DIR}
             -DCMAKE_POSITION_INDEPENDENT_CODE=ON
             -DBUILD_TESTING=OFF
             -DCMAKE_BUILD_TYPE=${THIRD_PARTY_BUILD_TYPE}
             ${EXTERNAL_OPTIONAL_ARGS}
  CMAKE_CACHE_ARGS
    -DCMAKE_INSTALL_PREFIX:PATH=${GFLAGS_INSTALL_DIR}
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
    -DCMAKE_BUILD_TYPE:STRING=${THIRD_PARTY_BUILD_TYPE}
  BUILD_BYPRODUCTS ${GFLAGS_LIBRARIES})

add_library(gflags STATIC IMPORTED GLOBAL)
set_property(TARGET gflags PROPERTY IMPORTED_LOCATION ${GFLAGS_LIBRARIES})
add_dependencies(gflags extern_gflags)

# On Windows (including MinGW), the Shlwapi library is used by gflags if
# available.
if(WIN32)
  include(CheckIncludeFileCXX)
  check_include_file_cxx("shlwapi.h" HAVE_SHLWAPI)
  if(HAVE_SHLWAPI)
    set_property(GLOBAL PROPERTY OS_DEPENDENCY_MODULES shlwapi.lib)
  endif()
endif()
