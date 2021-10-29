# Project
cmake_minimum_required(VERSION 3.13)
project(DataFlow)

# Dependences
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/DependencesCMake.txt)

# Pass
add_subdirectory(src)

# Install
install(
  FILES
  include/noelle/core/DataFlow.hpp 
  include/noelle/core/DataFlowAnalysis.hpp 
  include/noelle/core/DataFlowEngine.hpp 
  include/noelle/core/DataFlowResult.hpp 
  DESTINATION 
  include/noelle/core
  )
