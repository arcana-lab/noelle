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
  include/LoopStructure.hpp 
  include/LoopsSummary.hpp 
  include/LoopEnvironment.hpp
  include/EnvBuilder.hpp
  include/StayConnectedNestedLoopForest.hpp
  DESTINATION 
  include
  )
