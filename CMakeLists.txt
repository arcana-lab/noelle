cmake_minimum_required(VERSION 3.4.3)

project(noelle)

set(NOELLE_VERSION 9.14.0.wip)
set(NOELLE_EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)

set(NOELLE_CXX_FLAGS 
  -O0
  -g
  -fPIC
  -std=c++17
  -Wall
)

configure_file(enable.in ${CMAKE_SOURCE_DIR}/enable @ONLY)

enable_language(C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(LLVM 9 REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
link_directories(${LLVM_LIBRARY_DIRS})

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# prepare the pass to be included in the source tree
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)
include(HandleLLVMOptions)

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${NOELLE_EXTERNAL_DIR}/svf/include
  ${NOELLE_EXTERNAL_DIR}/scaf/include
)

set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/)
set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/)

add_compile_options(${NOELLE_CXX_FLAGS})

add_definitions(
  -D__STDC_LIMIT_MACROS
  -D__STDC_CONSTANT_MACROS
)

add_subdirectory(src)
