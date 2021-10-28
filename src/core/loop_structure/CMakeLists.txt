# Project
cmake_minimum_required(VERSION 3.13)
project(LoopStructure)

# Dependences
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/DependencesCMake.txt)

# Pass
add_subdirectory(src)

# Install
install(
  FILES
  include/noelle/core/LoopStructure.hpp 
  include/noelle/core/LoopsSummary.hpp 
  include/noelle/core/LoopEnvironment.hpp
  include/noelle/core/EnvBuilder.hpp
  include/noelle/core/StayConnectedNestedLoopForest.hpp
  DESTINATION 
  include/noelle/core
  )
