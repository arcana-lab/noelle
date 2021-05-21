# Project
cmake_minimum_required(VERSION 3.4.3)
project(NOELLE-Core)

# Programming languages to use
enable_language(C CXX)

# Find and link with LLVM
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -Weverything")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Weverything")
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

# Link with LLVM
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
#link_directories(${LLVM_LIBRARY_DIRS})

# Prepare the pass to be included in the source tree
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

# All include files for CPF
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include
                    ${CMAKE_CURRENT_BINARY_DIR}/include
                    ${CMAKE_INSTALL_PREFIX}/include
                    ${CMAKE_INSTALL_PREFIX}/include/svf)

# Pass
add_subdirectory(alloc_aa)
add_subdirectory(architecture)
add_subdirectory(basic_utilities)
add_subdirectory(callgraph)
add_subdirectory(clean_metadata)
add_subdirectory(dataflow)
add_subdirectory(hotprofiler)
add_subdirectory(loop_distribution)
add_subdirectory(loops)
add_subdirectory(loop_structure)
add_subdirectory(loop_unroll)
add_subdirectory(loop_whilifier)
add_subdirectory(noelle)
add_subdirectory(outliner)
add_subdirectory(pdg)
add_subdirectory(scheduler)
add_subdirectory(talkdown)
add_subdirectory(task)
add_subdirectory(transformations)
add_subdirectory(unique_ir_marker)
