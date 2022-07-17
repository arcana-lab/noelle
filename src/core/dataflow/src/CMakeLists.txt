# Sources
set(Srcs 
  DataFlowResult.cpp
  DataFlowEngine.cpp
  DataFlowAnalysis.cpp
)

# Compilation flags
set_source_files_properties(${Srcs} PROPERTIES COMPILE_FLAGS " -std=c++17 -fPIC")

# Name of the LLVM pass
set(PassName "DataFlow")

# configure LLVM 
find_package(LLVM REQUIRED CONFIG)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(HandleLLVMOptions)
include(AddLLVM)

message(STATUS "LLVM_DIR IS ${LLVM_CMAKE_DIR}.")

include_directories(${LLVM_INCLUDE_DIRS} ../../basic_utilities/include ../../loops/include ../include ./)

# Declare the LLVM pass to compile
add_llvm_library(${PassName} MODULE ${Srcs})
