# Project
cmake_minimum_required(VERSION 3.13)
project(LoopContent)

# Dependences
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/DependencesCMake.txt)

# Pass
add_subdirectory(src)

# Install
install(
  FILES
  include/noelle/core/ControlFlowEquivalence.hpp
  include/noelle/core/LoopDependenceInfo.hpp
  include/noelle/core/LoopTransformationsOptions.hpp
  DESTINATION 
  include/noelle/core
  )
