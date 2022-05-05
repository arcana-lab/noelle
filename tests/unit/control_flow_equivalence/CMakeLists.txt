# Project
cmake_minimum_required(VERSION 3.13)
project(Parallelization)

# Programming languages to use
enable_language(C CXX)

# Find and link with LLVM
find_package(LLVM REQUIRED CONFIG)

add_definitions(${LLVM_DEFINITIONS})
add_definitions(
-D__STDC_LIMIT_MACROS
-D__STDC_CONSTANT_MACROS
)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(CUSTOM_COMPILE_FLAGS "-fexceptions")
SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${CUSTOM_COMPILE_FLAGS}" )
SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${CUSTOM_COMPILE_FLAGS}" )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Prepare the pass to be included in the source tree
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

# Pass
add_subdirectory(src)

# Install
install(PROGRAMS include/CFETestSuite.hpp DESTINATION include)
