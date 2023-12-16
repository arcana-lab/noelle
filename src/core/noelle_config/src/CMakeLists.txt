# Sources
set(Srcs 
  Noelle_config.cpp
)

# Compilation flags
set_source_files_properties(${Srcs} PROPERTIES COMPILE_FLAGS " -std=c++17 -fPIC")

# Name of the LLVM pass
set(PassName "noelle-config")

# configure LLVM 
find_package(LLVM 9 REQUIRED CONFIG)

set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/)
set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(HandleLLVMOptions)
include(AddLLVM)

message(STATUS "LLVM_DIR IS ${LLVM_CMAKE_DIR}.")

# Configuration
get_source_file_property(CXX_FLAGS Noelle_config.cpp COMPILE_FLAGS)
configure_file(config.in config.hpp @ONLY)

# Declare the LLVM pass to compile
add_executable(${PassName} ${Srcs})
