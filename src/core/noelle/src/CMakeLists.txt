# Sources
set(Srcs 
  Pass.cpp
  Noelle.cpp
  Noelle_dependences.cpp
  Noelle_function.cpp
  Noelle_loops.cpp
  Noelle_transformations.cpp
  FunctionsManager.cpp
  CompilationOptionsManager.cpp
)

# Compilation flags
set_source_files_properties(${Srcs} PROPERTIES COMPILE_FLAGS " -std=c++17 -fPIC")

# Name of the LLVM pass
set(PassName "Noelle")

# configure LLVM 
find_package(LLVM REQUIRED CONFIG)

set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/)
set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(HandleLLVMOptions)
include(AddLLVM)

message(STATUS "LLVM_DIR IS ${LLVM_CMAKE_DIR}.")

include_directories(${LLVM_INCLUDE_DIRS} 
  ../include 
  ../../dominators/include
  ../../transformations/include
  ../../architecture/include 
  ../../alloc_aa/include 
  ../../hotprofiler/include 
  ../../talkdown/include
  ../../loop_structure/include
  ../../loop_content/include
  ../../loop_forest/include
  ../../loop_nesting_graph/include
  ../../callgraph/include
  ../../pdg/include
  ../../pdg_analysis/include
  ../../scheduler/include
  ../../cfg_analysis/include
  ../../cfg_transformer/include
  ../../loop_transformer/include
  ../../types_manager/include
  ../../linker/include
  ../../alias_analysis_engine/include
  ${CMAKE_INSTALL_PREFIX}/include
  )

# Declare the LLVM pass to compile
add_llvm_library(${PassName} MODULE ${Srcs})
