# Project
cmake_minimum_required(VERSION 3.13)
project(DG)

# Dependences
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/DependencesCMake.txt)

# Pass
add_subdirectory(src)

# Install
install(
  FILES
  include/noelle/core/Assumptions.hpp
  include/noelle/core/DGBase.hpp
  include/noelle/core/DGNode.hpp
  include/noelle/core/DGEdge.hpp
  include/noelle/core/DGGraphTraits.hpp
  include/noelle/core/SubCFGs.hpp
  DESTINATION 
  include/noelle/core
  )
