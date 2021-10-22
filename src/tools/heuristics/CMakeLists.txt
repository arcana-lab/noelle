# Project
cmake_minimum_required(VERSION 3.13)
project(Heuristics)

# Dependences
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/DependencesCMake.txt)

# Pass
add_subdirectory(src)

# Install
install(
  FILES
  include/HeuristicsPass.hpp 
  include/Heuristics.hpp
  include/InvocationLatency.hpp
  include/PartitionCostAnalysis.hpp
  include/SmallestSizePartitionAnalysis.hpp
  include/MinMaxSizePartitionAnalysis.hpp
  DESTINATION 
  include/noelle/tools
  )
