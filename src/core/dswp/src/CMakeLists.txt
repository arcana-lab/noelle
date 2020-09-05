# Sources
set(Srcs 
  DSWP.cpp
  CFG.cpp
  Environment.cpp
  Inliner.cpp
  Partition.cpp
  Pipeline.cpp
  Printer.cpp
  Queue.cpp
  DSWPTask.cpp
)

# Compilation flags
set_source_files_properties(${Srcs} PROPERTIES COMPILE_FLAGS " -std=c++17 -fPIC")

# Name of the LLVM pass
set(PassName "DSWP")

# configure LLVM 
find_package(LLVM REQUIRED CONFIG)

set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/)
set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(HandleLLVMOptions)
include(AddLLVM)

message(STATUS "LLVM_DIR IS ${LLVM_CMAKE_DIR}.")

include_directories(${LLVM_INCLUDE_DIRS} 
  ${CMAKE_INSTALL_PREFIX}/include
  ${CMAKE_INSTALL_PREFIX}/include/svf
  ../../basic_utilities/include 
  ../../transformations/include
  ../../task/include 
  ../../alloc_aa/include 
  ../../pdg/include 
  ../../threadpool/include 
  ../../loop_structure/include
  ../../loops/include 
  ../../heuristics/include 
  ../../parallelization_technique/include 
  ../../hotprofiler/include 
  ../../talkdown/include
  ../../noelle/include
  ../../dataflow/include
  ../../callgraph/include
  ../../scheduler/include
  ../include 
  ./ 
  )

# Declare the LLVM pass to compile
add_llvm_library(${PassName} MODULE ${Srcs})
