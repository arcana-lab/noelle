# Sources
set(Srcs 
  CFETestSuite.cpp
)

# Compilation flags
set_source_files_properties(${Srcs} PROPERTIES COMPILE_FLAGS " -std=c++17 -fPIC")

# Name of the LLVM pass
set(PassName "control_flow_equivalence")

# configure LLVM 
find_package(LLVM REQUIRED CONFIG)

set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/)
set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(HandleLLVMOptions)
include(AddLLVM)

message(STATUS "LLVM_DIR IS ${LLVM_CMAKE_DIR}.")

set(RootPath ../../../..)
set(SVFDep ${RootPath}/external/svf/include)
include_directories(${LLVM_INCLUDE_DIRS} ${RootPath}/install/include ${SVFDep} ../../helpers/include ../include ./)

# Declare the LLVM pass to compile
add_llvm_library(${PassName} MODULE ${Srcs})
