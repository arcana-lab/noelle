# Project
cmake_minimum_required(VERSION 3.13)
project(LoopEnvironment)

# Dependences
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/DependencesCMake.txt)

# Pass
add_subdirectory(src)

# Install
install(
  FILES
  include/noelle/core/LoopEnvironment.hpp
  include/noelle/core/LoopEnvironmentUser.hpp
  include/noelle/core/LoopEnvironmentBuilder.hpp
  DESTINATION 
  include/noelle/core
  )
