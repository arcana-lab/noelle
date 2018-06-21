# Sources
set(Srcs 
  Printer.cpp
  LoopSelector.cpp
  Pipeline.cpp
  Inliner.cpp
  Queue.cpp
  Partition.cpp
  CFG.cpp
  Environment.cpp
  InductionVariable.cpp
  Helper.cpp
  ControlDependence.cpp
  ParallelSCC.cpp
  DSWP.cpp
  DSWPPass.cpp
  DSWPLoopDependenceInfo.cpp
)

# Compilation flags
set_source_files_properties(${Srcs} PROPERTIES COMPILE_FLAGS " -std=c++14 -fPIC")

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

include_directories(${LLVM_INCLUDE_DIRS} ../../analysis/include ../../threadpool/include ../../parallelization/include)

# Declare the LLVM pass to compile
add_llvm_loadable_module(${PassName} ${Srcs})
