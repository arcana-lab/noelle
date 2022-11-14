# Project
cmake_minimum_required(VERSION 3.13)
project(Dominators)

# Dependences
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/DependencesCMake.txt)

# Pass
add_subdirectory(src)

# Install
install(
  FILES
  include/noelle/core/Dominators.hpp
  include/noelle/core/DominatorNode.hpp
  DESTINATION 
  include/noelle/core
  )
