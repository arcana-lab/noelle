# Project
cmake_minimum_required(VERSION 3.13)
project(AliasAnalysisEngine)

# Dependences
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/DependencesCMake.txt)

# Pass
add_subdirectory(src)

# Install
install(
  FILES
  include/noelle/core/AliasAnalysisEngine.hpp
  include/noelle/core/LoopAliasAnalysisEngine.hpp
  include/noelle/core/ProgramAliasAnalysisEngine.hpp
  DESTINATION 
  include/noelle/core
  )
