# Project
cmake_minimum_required(VERSION 3.4.3)
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
install(PROGRAMS include/Techniques.hpp include/EnvBuilder.hpp include/LoopSummary.hpp include/LoopInfoSummary.hpp include/LoopEnvironment.hpp include/LoopDependenceInfo.hpp include/Parallelization.hpp include/SCCDAGAttrs.hpp include/SCCDAGPartition.hpp include/Queue.hpp include/SystemHeaders.hpp include/Architecture.hpp DESTINATION include)
