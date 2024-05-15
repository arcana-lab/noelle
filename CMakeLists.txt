cmake_minimum_required(VERSION 3.13)

project(noelle)

include(FetchContent)

include(config.default.cmake)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config.cmake)
  include(config.cmake)
endif()

if(DEFINED NOELLE_INSTALL_DIR)
  set(CMAKE_INSTALL_PREFIX ${NOELLE_INSTALL_DIR})
endif()

set(NOELLE_CXX_FLAGS 
  -O0
  -g
  -fPIC
  -std=c++17
  -Wall
)

find_package(LLVM 14 REQUIRED CONFIG)
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
  GIT_TAG         noelle-14
)
FetchContent_Declare(
  scaf
  GIT_REPOSITORY  "https://github.com/PrincetonUniversity/SCAF.git"
  GIT_TAG         llvm14
)

set(NOELLE_CMAKE_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

get_filename_component(
  CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} REALPATH
)
get_filename_component(
  NOELLE_CMAKE_ROOT ${NOELLE_CMAKE_ROOT} REALPATH
)

string(ASCII 27 Esc)
set(Purple "${Esc}[35m")
set(Green "${Esc}[32m")
set(Red "${Esc}[31m")
set(ColorReset "${Esc}[m")

function(noelle_acquire_option OPT)
  if(${OPT} MATCHES "ON|On|on|1")
    set(${OPT} ON PARENT_SCOPE)
    message(STATUS "${Purple}Option${ColorReset} ${OPT} ${Green}ON${ColorReset}")
  elseif(${OPT} MATCHES "OFF|Off|off|0")
    set(${OPT} OFF PARENT_SCOPE)
    message(STATUS "${Purple}Option${ColorReset} ${OPT} ${Red}OFF${ColorReset}")
  elseif(${OPT} MATCHES "OFF|Off|off|0")
  else()
    message(FATAL_ERROR "${OPT} must be either ON or OFF")
  endif()
endfunction()

message(STATUS "${Purple}Install directory${ColorReset} is ${CMAKE_INSTALL_PREFIX}")

noelle_acquire_option(NOELLE_SVF)
noelle_acquire_option(NOELLE_SCAF)
noelle_acquire_option(NOELLE_AUTOTUNER)
noelle_acquire_option(NOELLE_REPL)
noelle_acquire_option(NOELLE_TOOLS)

set(LLVM_ENABLE_UNWIND_TABLES ON)

if(NOELLE_SVF STREQUAL ON)
  set(NOELLE_SVF ON)
  list(APPEND NOELLE_CXX_FLAGS "-DNOELLE_ENABLE_SVF")
  FetchContent_Populate(svf)
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E env LLVM_DIR=${LLVM_DIR} bash build.sh ${CMAKE_INSTALL_PREFIX}
    RESULT_VARIABLE CMD_ERROR
    WORKING_DIRECTORY ${svf_SOURCE_DIR}
  )
  include_directories(${CMAKE_INSTALL_PREFIX}/include/svf)
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

add_subdirectory(src)
add_subdirectory(bin)
