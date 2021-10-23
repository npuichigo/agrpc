agrpc_add_all_subdirs()

agrpc_cc_library(
  NAME
    runtime
  DEPS
    agrpc::base::logging
    gflags
    unifex
  PUBLIC
)
