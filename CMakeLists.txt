cmake_minimum_required(VERSION 3.4.3)

project(noelle)

include(FetchContent)

# See `cmake --help-policy CMP0077`
# `OLD` specifies that the effects of this policy are whatever LLVM 9
# assumed at that time.
cmake_policy(SET CMP0077 OLD)

set(NOELLE_CXX_FLAGS 
  -O0
  -g
  -fPIC
  -std=c++17
  -Wall
)

execute_process(
  COMMAND llvm-config --obj-root
  OUTPUT_VARIABLE LLVM_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_STRIP_TRAILING_WHITESPACE
)

FetchContent_Declare(
  svf
  GIT_REPOSITORY  "https://github.com/arcana-lab/SVF.git"
  GIT_TAG         noelle-9
)
FetchContent_Declare(
  scaf
  GIT_REPOSITORY  "https://github.com/PrincetonUniversity/SCAF.git"
  GIT_TAG         v9.1.8
)

set(NOELLE_CMAKE_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

get_filename_component(
  CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} REALPATH
)
get_filename_component(
  NOELLE_CMAKE_ROOT ${NOELLE_CMAKE_ROOT} REALPATH
)

option(ENABLE_SPECULATION "SCAF speculation" OFF)
set(LLVM_ENABLE_UNWIND_TABLES ON)

function(noelle_add_svf)
  list(APPEND NOELLE_CXX_FLAGS "-DNOELLE_ENABLE_SVF")
  set(NOELLE_CXX_FLAGS "${NOELLE_CXX_FLAGS}" PARENT_SCOPE)
  set(NOELLE_SVF ON PARENT_SCOPE)
  FetchContent_MakeAvailable(svf)
endfunction()

function(noelle_add_scaf)
  list(APPEND NOELLE_CXX_FLAGS "-DNOELLE_ENABLE_SCAF")
  set(NOELLE_CXX_FLAGS "${NOELLE_CXX_FLAGS}" PARENT_SCOPE)
  set(NOELLE_SCAF ON PARENT_SCOPE)
  FetchContent_MakeAvailable(scaf)
endfunction()

function(noelle_add_autotuner)
  set(NOELLE_AUTOTUNER ON PARENT_SCOPE)
  add_subdirectory(autotuner)
endfunction()

if(DEFINED NOELLE_SVF)
  string(TOUPPER ${NOELLE_SVF} NOELLE_SVF)
  if((NOELLE_SVF STREQUAL "ON") OR (NOELLE_SVF STREQUAL "1"))
    noelle_add_svf()
  elseif((NOELLE_SVF STREQUAL "OFF") OR (NOELLE_SVF STREQUAL "0"))
    set(NOELLE_SVF OFF)
  else()
    message(FATAL_ERROR "'NOELLE_SVF' must be ON, OFF, 0 or 1")
  endif()
else()
  # default behavior
  noelle_add_svf()
endif()

if(DEFINED NOELLE_SCAF)
  string(TOUPPER ${NOELLE_SCAF} NOELLE_SCAF)
  if((NOELLE_SCAF STREQUAL "ON") OR (NOELLE_SCAF STREQUAL "1"))
    noelle_add_scaf()
  elseif((NOELLE_SCAF STREQUAL "OFF") OR (NOELLE_SCAF STREQUAL "0"))
    set(NOELLE_SCAF OFF)
  else()
    message(FATAL_ERROR "'NOELLE_SCAF' must be ON, OFF, 0 or 1")
  endif()
else()
  # default behavior
  noelle_add_scaf()
endif()

if(DEFINED NOELLE_AUTOTUNER)
  string(TOUPPER ${NOELLE_AUTOTUNER} NOELLE_SVF)
  if((NOELLE_AUTOTUNER STREQUAL "ON") OR (NOELLE_AUTOTUNER STREQUAL "1"))
  elseif((NOELLE_AUTOTUNER STREQUAL "OFF") OR (NOELLE_AUTOTUNER STREQUAL "0"))
    noelle_add_autotuner()
  else()
    message(FATAL_ERROR "'NOELLE_AUTOTUNER' must be ON, OFF, 0 or 1")
  endif()
else()
  # default behavior
  noelle_add_autotuner()
endif()

FetchContent_GetProperties(svf)
FetchContent_GetProperties(scaf)

message(STATUS "NOELLE: SCAF analysis is ${NOELLE_SCAF}")
message(STATUS "NOELLE: SVF analysis is ${NOELLE_SVF}")
message(STATUS "NOELLE: AUTOTUNER is ${NOELLE_AUTOTUNER}")

file(READ ${NOELLE_CMAKE_ROOT}/VERSION NOELLE_VERSION)
string(STRIP ${NOELLE_VERSION} NOELLE_VERSION)
set(NOELLE_EXTERNAL_DIR ${NOELLE_CMAKE_ROOT}/external)

configure_file(enable.in enable @ONLY)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/enable
  DESTINATION ${NOELLE_CMAKE_ROOT}
)

install(
  FILES ${NOELLE_CMAKE_ROOT}/.githooks/pre-commit
  DESTINATION ${NOELLE_CMAKE_ROOT}/.git/hooks
  PERMISSIONS
    OWNER_EXECUTE OWNER_WRITE OWNER_READ
    GROUP_EXECUTE GROUP_READ
    WORLD_EXECUTE WORLD_READ
)

enable_language(C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(LLVM 14 REQUIRED CONFIG)
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
  ${svf_SOURCE_DIR}/include
  ${scaf_SOURCE_DIR}/include
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
    ${NOELLE_CMAKE_ROOT}/compile_commands.json
)

add_subdirectory(src)
