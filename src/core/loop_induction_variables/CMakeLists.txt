# Project
cmake_minimum_required(VERSION 3.13)
project(InductionVariables)

# Dependences
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/DependencesCMake.txt)

# Pass
add_subdirectory(src)

# Install
install(
  FILES
  include/noelle/core/InductionVariable.hpp
  include/noelle/core/LoopGoverningInductionVariable.hpp
  include/noelle/core/InductionVariables.hpp
  include/noelle/core/IVStepperUtility.hpp
  DESTINATION 
  include/noelle/core
  )
