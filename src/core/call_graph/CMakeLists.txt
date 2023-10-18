# Project
cmake_minimum_required(VERSION 3.13)
project(CallGraph)

# Dependences
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/DependencesCMake.txt)

# Pass
add_subdirectory(src)

# Install
install(
  FILES
  include/noelle/core/CallGraph.hpp
  include/noelle/core/CallGraphNode.hpp
  include/noelle/core/CallGraphEdge.hpp
  include/noelle/core/CallGraphTraits.hpp
  include/noelle/core/SCCCAG.hpp
  DESTINATION 
  include/noelle/core
  )
