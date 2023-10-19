# Project
cmake_minimum_required(VERSION 3.13)
project(HELIX)

# Dependences
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/DependencesCMake.txt)

# Pass
add_subdirectory(src)

# Install
install(
  FILES
  include/noelle/tools/HELIX.hpp 
  include/noelle/tools/HELIXTask.hpp 
  include/noelle/tools/SequentialSegment.hpp 
  include/noelle/tools/SpilledLoopCarriedDependence.hpp
  DESTINATION 
  include/noelle/tools
  )
