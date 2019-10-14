# Project
cmake_minimum_required(VERSION 3.4.3)
project(Parallelization)

set(LibraryName UnitTestHelpers)
set(Srcs
  src/Comparators.cpp
  src/TestSuite.cpp
)

# Programming languages to use
enable_language(C CXX)

# Find and link with LLVM
find_package(LLVM REQUIRED CONFIG)
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)
include_directories(./include)

# Compilation flags
set_source_files_properties(${Srcs} PROPERTIES COMPILE_FLAGS " -std=c++14 -fPIC")
add_library(${LibraryName} SHARED ${Srcs})
set_target_properties(${LibraryName} PROPERTIES PREFIX "")

# Install
install(TARGETS ${LibraryName} DESTINATION include)
