# Find and link with LLVM
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -Weverything")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Weverything")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Link with LLVM
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
#link_directories(${LLVM_LIBRARY_DIRS})

# Prepare the pass to be included in the source tree
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

# All include files for CPF
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}/include
  ${CMAKE_INSTALL_PREFIX}/include
  ${CMAKE_INSTALL_PREFIX}/include/svf)

# List of NOELLE core modules
# NOTE: If you add a module, you MUST add it here
set(core_dirs
  alloc_aa
  architecture
  basic_utilities
  callgraph
  clean_metadata
  dataflow
  hotprofiler
  loop_distribution
  loops
  loop_structure
  loop_unroll
  loop_whilifier
  noelle
  outliner
  pdg
  scheduler
  talkdown
  task
  transformations
  unique_ir_marker
  )


# Add all include directories
foreach(core_dir ${core_dirs})
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${core_dir}/include)
endforeach()

# Add each Pass
foreach(core_dir ${core_dirs})
  add_subdirectory(${core_dir})
endforeach()
