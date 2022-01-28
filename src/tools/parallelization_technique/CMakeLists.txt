# Project
cmake_minimum_required(VERSION 3.13)
project(ParallelizationTechnique)

# Dependences
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/DependencesCMake.txt)

# Pass
add_subdirectory(src)

# Install
install(
  FILES
  include/noelle/tools/ParallelizationTechnique.hpp 
  include/noelle/tools/ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences.hpp 
  DESTINATION 
  include/noelle/tools
  )
