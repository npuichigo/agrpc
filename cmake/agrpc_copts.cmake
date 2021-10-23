set(AGRPC_CXX_STANDARD ${CMAKE_CXX_STANDARD})

set(AGRPC_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(AGRPC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(AGRPC_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

if(${AGRPC_ENABLE_THREADING})
  find_package(Threads REQUIRED)
  agrpc_select_compiler_opts(_AGRPC_PTHREADS_LINKOPTS CLANG_OR_GCC "-pthread")
else()
  # Don't link pthreads
endif()

agrpc_select_compiler_opts(
  AGRPC_DEFAULT_LINKOPTS CLANG_OR_GCC
  # Required by all modern software, effectively:
  "-lm" ${_AGRPC_PTHREADS_LINKOPTS})
