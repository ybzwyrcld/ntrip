if (0)
cmake_minimum_required(VERSION 2.8)

project(ntrip)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)

option(NTRIP_BUILD_EXAMPLES "Build ntrip examples" OFF)

add_definitions("-std=c++11 -pthread")

set(CMAKE_LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_INCLUDE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/include)

# set(ntrip_include_dirs ${CMAKE_CURRENT_SOURCE_DIR}/include)
# include_directories(SYSTEM ${ntrip_include_dirs})

add_library(${PROJECT_NAME}
  ntrip.cc
)

if (NTRIP_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif (NTRIP_BUILD_EXAMPLES)
endif ()

add_sources(
  ntrip.cc
  ntrip.h
)
