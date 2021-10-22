# Project
cmake_minimum_required(VERSION 3.13)
project(LoopDistribution)

# Dependences
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/DependencesCMake.txt)

# Pass
add_subdirectory(src)

# Install
install(
  FILES
  include/LoopDistribution.hpp
  DESTINATION 
  include
  )
