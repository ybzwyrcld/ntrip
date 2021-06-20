cmake_minimum_required(VERSION 2.8)

project(ntrip)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)

option(NTRIP_BUILD_EXAMPLES "Build ntrip examples" OFF)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message("-- Using default build type: Debug.")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wfatal-errors")

set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O2 -Wall -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

set(CMAKE_LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_INCLUDE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/include)

set(ntrip_include_dirs ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(SYSTEM ${ntrip_include_dirs})

aux_source_directory(src src_MAIN)

add_library(${PROJECT_NAME}
  ${src_MAIN}
)
set_target_properties(${PROJECT_NAME} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${ntrip_include_dirs})

if (NTRIP_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif (NTRIP_BUILD_EXAMPLES)
