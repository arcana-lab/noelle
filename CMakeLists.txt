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

if(NOT DEFINED NOELLE_BUILD_TYPE)
  set(NOELLE_BUILD_TYPE "Debug")
endif()

if(NOT DEFINED SVF_BUILD_TYPE)
  set(SVF_BUILD_TYPE "Release")
endif()

if(NOT DEFINED SCAF_BUILD_TYPE)
  set(SCAF_BUILD_TYPE "Release")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(LLVM 18.1.8 REQUIRED CONFIG)

FetchContent_Declare(
  svf
  GIT_REPOSITORY  "https://github.com/arcana-lab/SVF.git"
  GIT_TAG         noelle-18
)
FetchContent_Declare(
  scaf
  GIT_REPOSITORY  "https://github.com/PrincetonUniversity/SCAF.git"
  GIT_TAG         llvm18
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
message(STATUS "${Purple}NOELLE build type${ColorReset} is ${NOELLE_BUILD_TYPE}")
message(STATUS "${Purple}SVF build type${ColorReset} is ${SVF_BUILD_TYPE}")
message(STATUS "${Purple}SCAF build type${ColorReset} is ${SCAF_BUILD_TYPE}")

noelle_acquire_option(NOELLE_SVF)
noelle_acquire_option(NOELLE_SCAF)
noelle_acquire_option(NOELLE_AUTOTUNER)
noelle_acquire_option(NOELLE_REPL)
noelle_acquire_option(NOELLE_TOOLS)

if(NOELLE_SVF STREQUAL ON)
  set(NOELLE_SVF ON)
  list(APPEND NOELLE_CXX_FLAGS "-DNOELLE_ENABLE_SVF")
  set(CMAKE_BUILD_TYPE ${SVF_BUILD_TYPE})
  FetchContent_MakeAvailable(svf)
  FetchContent_GetProperties(svf)
  include_directories(${svf_BINARY_DIR}/include)
  include_directories(${svf_SOURCE_DIR}/svf/include)
  include_directories(${svf_SOURCE_DIR}/svf-llvm/include)
endif()

if(NOELLE_SCAF STREQUAL ON)
  list(APPEND NOELLE_CXX_FLAGS "-DNOELLE_ENABLE_SCAF")
  option(ENABLE_SPECULATION "SCAF speculation" OFF)
  set(CMAKE_BUILD_TYPE ${SCAF_BUILD_TYPE})
  FetchContent_MakeAvailable(scaf)
  FetchContent_GetProperties(scaf)
  include_directories(${scaf_SOURCE_DIR}/include)
endif()

set(CMAKE_BUILD_TYPE ${NOELLE_BUILD_TYPE})

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
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_INSTALL_PREFIX}")

# prepare the pass to be included in the source tree
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")

include_directories(
  ${LLVM_INCLUDE_DIRS}
)

add_compile_options(${NOELLE_CXX_FLAGS})

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
