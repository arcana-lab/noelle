# Project
cmake_minimum_required(VERSION 3.13)
project(MayPointToAnalysis)

# Dependences
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/DependencesCMake.txt)

# Pass
add_subdirectory(src)

# Install
install(
  FILES
  include/noelle/core/MayPointToAnalysis.hpp
  include/noelle/core/MayPointToAnalysisUtils.hpp
  DESTINATION 
  include/noelle/core
  )
