# Project
cmake_minimum_required(VERSION 3.13)
project(DSWP)

# Dependences
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/DependencesCMake.txt)

# Pass
add_subdirectory(src)

# Install
install(
  FILES
  include/noelle/tools/DSWP.hpp 
  include/noelle/tools/DSWPTask.hpp 
  DESTINATION 
  include/noelle/tools
  )
