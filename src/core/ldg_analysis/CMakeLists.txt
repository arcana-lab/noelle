# Project
cmake_minimum_required(VERSION 3.13)
project(LDGAnalysis)

# Dependences
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/DependencesCMake.txt)

# Pass
add_subdirectory(src)

# Install
install(
  FILES
  include/noelle/core/LoopIterationSpaceAnalysis.hpp
  include/noelle/core/LDGAnalysis.hpp
  DESTINATION 
  include/noelle/core
  )
