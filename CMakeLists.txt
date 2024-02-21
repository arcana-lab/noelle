cmake_minimum_required(VERSION 3.4.3)

project(noelle)

get_filename_component(
  CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} REALPATH
)
get_filename_component(
  CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR} REALPATH
)

file(READ ${CMAKE_SOURCE_DIR}/VERSION NOELLE_VERSION)
string(STRIP ${NOELLE_VERSION} NOELLE_VERSION)
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

add_custom_target(
  CompileCommands ALL
  COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_BINARY_DIR}/compile_commands.json
    ${CMAKE_SOURCE_DIR}/compile_commands.json
)

add_subdirectory(src)
