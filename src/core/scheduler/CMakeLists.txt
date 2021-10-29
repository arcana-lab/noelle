# Project
cmake_minimum_required(VERSION 3.13)
project(Scheduler)

# Dependences
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/DependencesCMake.txt)

# Pass
add_subdirectory(src)

# Install
install(
  FILES
  include/noelle/core/Scheduler.hpp
  include/noelle/core/SCCPartitionScheduler.hpp
  DESTINATION 
  include/noelle/core
  )
