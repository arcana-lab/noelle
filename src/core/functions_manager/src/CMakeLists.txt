# Sources
set(Srcs 
  FunctionsManager.cpp
)

# Compilation flags
set_source_files_properties(${Srcs} PROPERTIES COMPILE_FLAGS " -std=c++17 -fPIC")

# Name of the LLVM pass
set(PassName "FunctionsManager")

set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/)
set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(HandleLLVMOptions)
include(AddLLVM)

message(STATUS "LLVM_DIR IS ${LLVM_CMAKE_DIR}.")

include_directories(${LLVM_INCLUDE_DIRS} 
  ../include 
  ../../basic_utilities/include
  ../../call_graph/include
  ../../hotprofiler/include
  ../../pdg/include
  ../../dg/include
  ../../pdg_analysis/include
  ../../dependence_analysis/include
  ../../call_graph_analysis/include
  ../../pdg_printer/include
  ../../sccdag/include
  ../../alloc_aa/include
  ../../talkdown/include
  ../../alias_analysis_engine/include
  ../../loop_structure/include
  ../../dataflow/include
  ../../types_manager/include
  ../../may_points_to_analysis/include
  ${CMAKE_INSTALL_PREFIX}/include
  )

# Declare the LLVM pass to compile
add_llvm_library(${PassName} MODULE ${Srcs})
