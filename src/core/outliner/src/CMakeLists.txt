# Sources
set(Srcs 
  Pass.cpp
  Outliner.cpp
)

# Compilation flags
set_source_files_properties(${Srcs} PROPERTIES COMPILE_FLAGS " -std=c++17 -fPIC")

# Name of the LLVM pass
set(PassName "OutlinerPass")

# configure LLVM 
find_package(LLVM REQUIRED CONFIG)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(HandleLLVMOptions)
include(AddLLVM)

message(STATUS "LLVM_DIR IS ${LLVM_CMAKE_DIR}.")

include_directories(${LLVM_INCLUDE_DIRS} 
  ../include 
  ../../basic_utilities/include 
  ../../transformations/include
  ../../architecture/include 
  ../../alloc_aa/include 
  ../../pdg/include 
  ../../hotprofiler/include 
  ../../loop_structure/include 
  ../../talkdown/include
  ../../loops/include
  ../../dataflow/include
  ../../callgraph/include
  ../../scheduler/include
  ${CMAKE_INSTALL_PREFIX}/include
  )

# Declare the LLVM pass to compile
add_llvm_library(${PassName} MODULE ${Srcs})
