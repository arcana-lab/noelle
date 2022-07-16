# Project
cmake_minimum_required(VERSION 3.4.3)
project(NOELLE-Tools)

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
                  
# List of NOELLE tools
# NOTE: If you add a tool, you MUST add it here
set(tool_dirs
  deadfunctioneliminator
  doall
  dswp
  enablers
  helix
  heuristics
  inliner
  loop_invariant_code_motion
  loop_metadata
  loop_stats
  parallelization_technique
  parallelizer
  pdg_stats
  scev_simplification
  codesize
  )

                  
# Add all include directories
foreach(tool_dir ${tool_dirs})
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${tool_dir}/include)
endforeach()

# Add each Tool
foreach(tool_dir ${tool_dirs})
  add_subdirectory(${tool_dir})
endforeach()
                  
