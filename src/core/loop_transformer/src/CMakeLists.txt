# Sources
set(Srcs
  LoopTransformer.cpp
  Pass.cpp
)

# Compilation flags
set_source_files_properties(${Srcs} PROPERTIES COMPILE_FLAGS " -std=c++17 -fPIC")

# Name of the LLVM pass
set(PassName "LoopTransformer")

# configure LLVM
find_package(LLVM 9 REQUIRED CONFIG)

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
  ../../loop_structure/include 
  ../../loop_environment/include 
  ../../loop_forest/include
  ../../dg/include
  ../../pdg/include
  ../../sccdag/include
  ../../loop_invariants/include
  ../../loop_induction_variables/include
  ../../loop_content/include
  ../../scheduler/include
  ../../loop_whilifier/include
  ../../loop_unroll/include
  ../../loop_distribution/include
  ../../loop_carried_dependences/include
  ../../loop_scc_attributes/include
  ../../loop_sccdag_attributes/include
  ../../memory_cloning_analysis/include
  ../../alias_analysis_engine/include
  ../../compilation_options_manager/include
  ../../ldg_analysis/include
	../include
)

# Declare the LLVM pass to compile
add_llvm_library(${PassName} MODULE ${Srcs})
