# Sources
set(Srcs 
  InvocationLatency.cpp
  PartitionCostAnalysis.cpp
  MinMaxSizePartitionAnalysis.cpp
  SmallestSizePartitionAnalysis.cpp
  Heuristics.cpp
  HeuristicsPass.cpp
)

# Compilation flags
set_source_files_properties(${Srcs} PROPERTIES COMPILE_FLAGS " -std=c++17 -fPIC")

# Name of the LLVM pass
set(PassName "Heuristics")

# configure LLVM 
find_package(LLVM REQUIRED CONFIG)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(HandleLLVMOptions)
include(AddLLVM)

message(STATUS "LLVM_DIR IS ${LLVM_CMAKE_DIR}.")

include_directories(${LLVM_INCLUDE_DIRS} 
  ../../basic_utilities/include 
  ../../transformations/include
  ../../alloc_aa/include 
  ../../pdg/include 
  ../../threadpool/include 
  ../../loop_structure/include
  ../../loops/include 
  ../../talkdown/include
  ../../hotprofiler/include
  ../../noelle/include
  ../../dataflow/include
  ../../callgraph/include
  ../../scheduler/include
  ../include
  ${CMAKE_INSTALL_PREFIX}/include
  )

# Declare the LLVM pass to compile
add_llvm_library(${PassName} MODULE ${Srcs})
