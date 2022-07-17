# Sources
set(Srcs 
  DOALL.cpp
  DOALLTask.cpp
  DOALL_analysis.cpp
  DOALL_lastIteration.cpp
  Builder.cpp
)

# Compilation flags
set_source_files_properties(${Srcs} PROPERTIES COMPILE_FLAGS " -std=c++17 -fPIC")

# Name of the LLVM pass
set(PassName "DOALL")

# configure LLVM 
find_package(LLVM REQUIRED CONFIG)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(HandleLLVMOptions)
include(AddLLVM)

message(STATUS "LLVM_DIR IS ${LLVM_CMAKE_DIR}.")

include_directories(${LLVM_INCLUDE_DIRS} 
  ../../heuristics/include 
  ../../parallelization_technique/include 
  ../include 
  ./ 
  ${CMAKE_INSTALL_PREFIX}/include
  )

# Declare the LLVM pass to compile
add_llvm_library(${PassName} MODULE ${Srcs})
