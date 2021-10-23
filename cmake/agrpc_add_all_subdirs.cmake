# agrpc_add_all_subidrs
#
# CMake function to add all subdirectories of the current directory that contain
# a CMakeLists.txt file
#
# Takes no arguments.
function(agrpc_add_all_subdirs)
  file(
    GLOB _CHILDREN
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/*)
  set(_DIRLIST "")
  foreach(_CHILD ${_CHILDREN})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${_CHILD}
       AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${_CHILD}/CMakeLists.txt)
      list(APPEND _DIRLIST ${_CHILD})
    endif()
  endforeach()

  foreach(subdir ${_DIRLIST})
    add_subdirectory(${subdir})
  endforeach()
endfunction()
