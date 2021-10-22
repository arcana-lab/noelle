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
  include/DataFlow.hpp 
  include/DataFlowAnalysis.hpp 
  include/DataFlowEngine.hpp 
  include/DataFlowResult.hpp 
  DESTINATION 
  include
  )
