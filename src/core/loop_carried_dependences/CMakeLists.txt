# Project
cmake_minimum_required(VERSION 3.13)
project(LoopCarriedDependencies)

# Dependences
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/DependencesCMake.txt)

# Pass
add_subdirectory(src)

# Install
install(
  FILES
  include/noelle/core/Variable.hpp
  include/noelle/core/LoopCarriedDependencies.hpp
  DESTINATION 
  include/noelle/core
  )
