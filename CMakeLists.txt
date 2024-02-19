cmake_minimum_required(VERSION 3.4.3)

project(noelle)

get_filename_component(
  CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} REALPATH
)
get_filename_component(
  CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR} REALPATH
)

set(NOELLE_VERSION 9.14.0.wip)
set(NOELLE_EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)

set(NOELLE_CXX_FLAGS 
  -O0
  -g
  -fPIC
  -std=c++17
  -Wall
)

configure_file(enable.in enable @ONLY)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/enable
  DESTINATION ${CMAKE_SOURCE_DIR}
)

install(
  FILES ${CMAKE_SOURCE_DIR}/.githooks/pre-commit
  DESTINATION ${CMAKE_SOURCE_DIR}/.git/hooks
  PERMISSIONS
    OWNER_EXECUTE OWNER_WRITE OWNER_READ
    GROUP_EXECUTE GROUP_READ
    WORLD_EXECUTE WORLD_READ
)

execute_process(
  COMMAND git log -1 --format=%H
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE NOELLE_GIT_COMMIT
  OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET
)

execute_process(
  COMMAND git remote get-url --all origin
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE NOELLE_GIT_ORIGIN
  OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET
)

enable_language(C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# See `cmake --help-policy CMP0077`
# `OLD` specifies that the effects of this policy are whatever LLVM 9
# assumed at that time.
cmake_policy(SET CMP0077 OLD)

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
