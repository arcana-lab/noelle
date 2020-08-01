# Sources
set(Srcs 
  Pass.cpp
  Enablers.cpp
  EnablersManager.cpp
)

# Compilation flags
set_source_files_properties(${Srcs} PROPERTIES COMPILE_FLAGS " -std=c++17 -fPIC")

# Name of the LLVM pass
set(PassName "Enablers")

# configure LLVM 
find_package(LLVM REQUIRED CONFIG)

set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/)
set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(HandleLLVMOptions)
include(AddLLVM)

message(STATUS "LLVM_DIR IS ${LLVM_CMAKE_DIR}.")

include_directories(${LLVM_INCLUDE_DIRS} 
  ../../../external/svf/include
  ../../../external/scaf/include
  ../../basic_utilities/include 
  ../../transformations/include
  ../../alloc_aa/include 
  ../../pdg/include 
  ../../threadpool/include 
  ../../loop_structure/include
  ../../loops/include 
  ../../heuristics/include 
  ../../parallelization_technique/include 
  ../../dswp/include 
  ../../doall/include 
  ../../helix/include 
  ../../hotprofiler/include
  ../../loop_distribution/include
  ../../loop_unroll/include
  ../../loop_whilifier/include
  ../../loop_invariant_code_motion/include
  ../../scev_simplification/include
  ../../talkdown/include
  ../../noelle/include
  ../../dataflow/include
  ../../callgraph/include
  ../../scheduler/include
  ./
  )

# Declare the LLVM pass to compile
add_llvm_library(${PassName} MODULE ${Srcs})
