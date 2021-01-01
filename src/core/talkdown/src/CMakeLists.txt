# Sources
set(Srcs 
  TalkDown.cpp
  Node.cpp
  Tree.cpp
  Annotation.cpp
  AnnotationParser.cpp
)

# Compilation flags
set_source_files_properties(${Srcs} PROPERTIES COMPILE_FLAGS " -std=c++17 -fPIC")

# Name of the LLVM pass
set(PassName "TalkDown")

# configure LLVM 
find_package(LLVM REQUIRED CONFIG)

set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/)
set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(HandleLLVMOptions)
include(AddLLVM)

message(STATUS "LLVM_DIR IS ${LLVM_CMAKE_DIR}.")

include_directories(
  ../../basic_utilities/include
  ../../loop_structure/include
  ../../pdg/include
  ../../alloca_aa/include
  ../../dataflow/include
  ../../callgraph/include
  ../include
  ./
  ${CMAKE_INSTALL_PREFIX}/include
  ${CMAKE_INSTALL_PREFIX}/include/svf
  ${CMAKE_INSTALL_PREFIX}/include/scaf
  ${LLVM_INCLUDE_DIRS}
  )

# Declare the LLVM pass to compile
add_llvm_library(${PassName} MODULE ${Srcs})
