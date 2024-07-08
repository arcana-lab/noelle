
# Programming languages to use
enable_language(C CXX)

# Find and link with LLVM
find_package(LLVM 18.1.6 REQUIRED CONFIG)

add_definitions(${LLVM_DEFINITIONS})
add_definitions(
  -D__STDC_LIMIT_MACROS
  -D__STDC_CONSTANT_MACROS
)

include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

# Prepare the pass to be included in the source tree
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)
