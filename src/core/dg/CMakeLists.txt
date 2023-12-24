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
  include/noelle/core/DGBase.hpp
  include/noelle/core/DGNode.hpp
  include/noelle/core/DGEdge.hpp
  include/noelle/core/ControlDependence.hpp
  include/noelle/core/DataDependence.hpp
  include/noelle/core/VariableDependence.hpp
  include/noelle/core/MemoryDependence.hpp
  include/noelle/core/UndefinedDependence.hpp
  include/noelle/core/DGGraphTraits.hpp
  include/noelle/core/SubCFGs.hpp
  DESTINATION 
  include/noelle/core
  )
