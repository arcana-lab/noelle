# Project
cmake_minimum_required(VERSION 3.4.3)
project(CAT)

# Programming languages to use
enable_language(C CXX)

# Find and link with LLVM
if(${APPLE})
  set(LLVM_DIR "/usr/local/Cellar/llvm38/3.8.0/lib/llvm-3.8/share/llvm/cmake")
  if( NOT EXISTS ${LLVM_DIR} )
    message(FATAL_ERROR "Please install LLVM 3.8 in your system (i.e., brew install homebrew/versions/llvm38 )")
  endif()
endif()
find_package(LLVM REQUIRED CONFIG)

if(!${APPLE})
  add_definitions(${LLVM_DEFINITIONS})
endif()
add_definitions(
-D__STDC_LIMIT_MACROS
-D__STDC_CONSTANT_MACROS
)

include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Prepare the pass to be included in the source tree
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

# Pass
add_subdirectory(catpass)

# Install
install(PROGRAMS bin/cat-c DESTINATION bin)
