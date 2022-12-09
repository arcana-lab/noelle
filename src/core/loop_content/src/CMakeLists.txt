# Sources
set(Srcs 
  ControlFlowEquivalence.cpp
  LoopAwareMemDepAnalysis.cpp
  LoopIterationDomainSpaceAnalysis.cpp
  SCCDAGAttrs.cpp
  SCCDAGNormalizer.cpp
  LoopDependenceInfo.cpp
  SCCDAGPartition.cpp
  LoopTransformationsOptions.cpp
)

# Compilation flags
set_source_files_properties(${Srcs} PROPERTIES COMPILE_FLAGS " -std=c++17 -fPIC")

# Name of the LLVM pass
set(PassName "LoopContent")

# configure LLVM 
find_package(LLVM REQUIRED CONFIG)

set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/)
set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(HandleLLVMOptions)
include(AddLLVM)

message(STATUS "LLVM_DIR IS ${LLVM_CMAKE_DIR}.")

include_directories(${LLVM_INCLUDE_DIRS} 
  ../../basic_utilities/include 
  ../../dominators/include 
  ../../transformations/include
  ../../callgraph/include
  ../../dataflow/include
  ../../architecture/include 
  ../../alloc_aa/include 
  ../../pdg/include 
  ../../hotprofiler/include 
  ../../loop_structure/include 
  ../../loop_environment/include 
  ../../loop_forest/include
  ../../talkdown/include
  ../../invariants/include
  ../../induction_variables/include
  ../../loop_scc_attributes/include
  ../include
  ./
  ${CMAKE_INSTALL_PREFIX}/include
  )

# Declare the LLVM pass to compile
add_llvm_library(${PassName} MODULE ${Srcs})
