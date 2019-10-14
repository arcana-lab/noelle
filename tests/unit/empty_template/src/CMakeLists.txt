# Sources
set(Srcs 
  MyTestSuite.cpp
)

# Compilation flags
set_source_files_properties(${Srcs} PROPERTIES COMPILE_FLAGS " -std=c++14 -fPIC")

# Name of the LLVM pass
set(PassName "UnitTester")

# configure LLVM 
find_package(LLVM REQUIRED CONFIG)

set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/)
set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(HandleLLVMOptions)
include(AddLLVM)

message(STATUS "LLVM_DIR IS ${LLVM_CMAKE_DIR}.")

set(PassesPath ../../../../src)
set(UtilDep ${PassesPath}/basic_utilities/include)
set(DGDep ${PassesPath}/alloc_aa/include ${PassesPath}/pdg/include)
set(ParDep ${PassesPath}/parallelization_utilities/include)
set(InlinerDep ${PassesPath}/dgsimplify/include)
set(ParTechniqueDep ${PassesPath}/parallelization_technique/include)
set(DOALLDep ${PassesPath}/doall/include)
set(DSWPDep ${PassesPath}/dswp/include)
set(HELIXep ${PassesPath}/helix/include)
include_directories(${LLVM_INCLUDE_DIRS} ${DGDep} ${UtilDep} ../../helpers/include ../include ./)

# Declare the LLVM pass to compile
add_llvm_loadable_module(${PassName} ${Srcs})

