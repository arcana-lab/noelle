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
  include/InductionVariables.hpp
  include/LoopGoverningIVAttribution.hpp
  include/IVStepperUtility.hpp
  DESTINATION 
  include
  )
