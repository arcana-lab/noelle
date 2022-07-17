# Sources
set(Srcs 
  Pass.cpp
  Inliner.cpp
  Inliner_loopCarried.cpp
  Printer.cpp
)

# Compilation flags
set_source_files_properties(${Srcs} PROPERTIES COMPILE_FLAGS " -std=c++17 -fPIC")

# Name of the LLVM pass
set(PassName "Inliner")

# configure LLVM 
find_package(LLVM REQUIRED CONFIG)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(HandleLLVMOptions)
include(AddLLVM)

message(STATUS "LLVM_DIR IS ${LLVM_CMAKE_DIR}.")

include_directories(${LLVM_INCLUDE_DIRS} 
  ../../doall/include
  ../../parallelization_technique/include
  ../../heuristics/include
  ../include
  ./
  ${CMAKE_INSTALL_PREFIX}/include
  )


# Declare the LLVM pass to compile
add_llvm_library(${PassName} MODULE ${Srcs})
