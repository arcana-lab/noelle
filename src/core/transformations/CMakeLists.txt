# Project
cmake_minimum_required(VERSION 3.13)
project(Transformations)

# Dependences
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/DependencesCMake.txt)

# Install
install(
  FILES
  include/noelle/core/Transformations.hpp
  DESTINATION 
  include/noelle/core
  )
