cmake_minimum_required(VERSION 2.8)

project(ntrip)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)

option(NTRIP_BUILD_EXAMPLES "Build ntrip examples" OFF)

add_definitions("-std=c++11 -pthread")

set(CMAKE_LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_INCLUDE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/include)

set(ntrip_include_dirs ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(SYSTEM ${ntrip_include_dirs})

macro (add_sources)
  file (RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
  foreach (_src ${ARGN})
    if (_relPath)
      list (APPEND SRCS "${_relPath}/${_src}")
    else()
      list (APPEND SRCS "${_src}")
    endif ()
  endforeach ()
  if (_relPath)
    # propagate SRCS to parent directory
    set (SRCS ${SRCS} PARENT_SCOPE)
  endif ()
endmacro ()

aux_source_directory(src src_MAIN)

add_library(${PROJECT_NAME}
  ${src_MAIN}
)
set_target_properties(${PROJECT_NAME} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${ntrip_include_dirs})

if (NTRIP_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif (NTRIP_BUILD_EXAMPLES)

