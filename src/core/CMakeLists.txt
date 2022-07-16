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
  cfg_analysis
  dataflow
  forest
  hotprofiler
  induction_variables
  invariants
  loop_distribution
  loop_environment
  loop_nesting_graph
  loop_structure
  loop_transformer
  loop_unroll
  loop_whilifier
  loops
  metadata_manager
  noelle
  outliner
  pdg
  scheduler
  talkdown
  task
  transformations
  unique_ir_marker
  )



# Building a list of all NOELLE include dirs to reference
set(noelle_include_dirs "")
foreach(core_dir ${core_dirs})
  set(noelle_include_dirs
    ${noelle_include_dirs}
    ${CMAKE_CURRENT_SOURCE_DIR}/${core_dir}/include)
endforeach()
set_property(GLOBAL PROPERTY NOELLE_INCLUDE_DIRS "${noelle_include_dirs}")

# Add all include directories
get_property(noelle_include_dirs GLOBAL PROPERTY NOELLE_INCLUDE_DIRS)
include_directories(${noelle_include_dirs})

# Add each Pass
foreach(core_dir ${core_dirs})
  add_subdirectory(${core_dir})
endforeach()
