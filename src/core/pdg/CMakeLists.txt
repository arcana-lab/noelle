# Project
cmake_minimum_required(VERSION 3.13)
project(PDG)

# Dependences
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/DependencesCMake.txt)

# Pass
add_subdirectory(src)

# Install
install(
  FILES
  include/Assumptions.h
  include/DGBase.hpp
  include/DGGraphTraits.hpp
  include/SubCFGs.hpp
  include/PDG.hpp
  include/PDGAnalysis.hpp
  include/SCC.hpp
  include/SCCDAG.hpp
  include/PDGPrinter.hpp
  DESTINATION 
  include
  )
