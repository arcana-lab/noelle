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

find_package(LLVM 9 REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
link_directories(${LLVM_LIBRARY_DIRS})

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

function(noelle_acquire_option OPT)
  if(${OPT} MATCHES "ON|On|on|1")
    set(${OPT} ON PARENT_SCOPE)
  elseif(${OPT} MATCHES "OFF|Off|off|0")
    set(${OPT} OFF PARENT_SCOPE)
  else()
    message(FATAL_ERROR "${OPT} must be either ON or OFF")
  endif()
  message(STATUS "Option ${OPT} is ${${OPT}}")
endfunction()

option(NOELLE_SVF "SVF analysis modules" ON)
option(NOELLE_SCAF "SCAF analysis modules" ON)
option(NOELLE_AUTOTUNER "NOELLE autotuner module" ON)
option(NOELLE_REPL "NOELLE REPL module" OFF)
option(NOELLE_TOOLS "Tools built on top of NOELLE" ON)

noelle_acquire_option(NOELLE_SVF)
noelle_acquire_option(NOELLE_SCAF)
noelle_acquire_option(NOELLE_AUTOTUNER)
noelle_acquire_option(NOELLE_REPL)
noelle_acquire_option(NOELLE_TOOLS)

set(LLVM_ENABLE_UNWIND_TABLES ON)

if(NOELLE_SVF STREQUAL ON)
  set(NOELLE_SVF ON)
  list(APPEND NOELLE_CXX_FLAGS "-DNOELLE_ENABLE_SVF")
  FetchContent_MakeAvailable(svf)
  FetchContent_GetProperties(svf)
  include_directories(${svf_SOURCE_DIR}/include)
endif()

if(NOELLE_SCAF STREQUAL ON)
  list(APPEND NOELLE_CXX_FLAGS "-DNOELLE_ENABLE_SCAF")
  option(ENABLE_SPECULATION "SCAF speculation" OFF)
  FetchContent_MakeAvailable(scaf)
  FetchContent_GetProperties(scaf)
  include_directories(${scaf_SOURCE_DIR}/include)
endif()

file(READ ${NOELLE_CMAKE_ROOT}/VERSION NOELLE_VERSION)
string(STRIP ${NOELLE_VERSION} NOELLE_VERSION)
set(NOELLE_EXTERNAL_DIR ${NOELLE_CMAKE_ROOT}/external)

configure_file(enable.in enable @ONLY)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/enable
  DESTINATION ${NOELLE_CMAKE_ROOT}
)
install(
  PROGRAMS ${NOELLE_CMAKE_ROOT}/.githooks/pre-commit
  DESTINATION ${NOELLE_CMAKE_ROOT}/.git/hooks
)

enable_language(C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

add_custom_target(noelle_libraries)
add_custom_target(noelle_tool_libraries)
set_target_properties(noelle_libraries PROPERTIES NAMES "")
set_target_properties(noelle_tool_libraries PROPERTIES NAMES "")

function(noelle_component_declare name)
  add_llvm_library(${name} MODULE)
  get_target_property(names noelle_libraries NAMES)
  list(APPEND names ${name})
  set_target_properties(noelle_libraries PROPERTIES NAMES "${names}")
endfunction()

function(noelle_tool_declare name)
  add_llvm_library(${name} MODULE)
  get_target_property(names noelle_tool_libraries NAMES)
  list(APPEND names ${name})
  set_target_properties(noelle_tool_libraries PROPERTIES NAMES "${names}")
endfunction()

if(NOELLE_AUTOTUNER STREQUAL ON)
  add_subdirectory(autotuner)
endif()
add_subdirectory(src)
add_subdirectory(bin)
