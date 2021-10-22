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
  include/CallGraph.hpp
  include/CallGraphNode.hpp
  include/CallGraphEdge.hpp
  include/CallGraphTraits.hpp
  include/SCCCAG.hpp
  DESTINATION 
  include
  )
