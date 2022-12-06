# Project
cmake_minimum_required(VERSION 3.13)
project(LoopSCCAttributes)

# Dependences
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/DependencesCMake.txt)

# Pass
add_subdirectory(src)

# Install
install(
  FILES
  include/noelle/core/ClonableMemoryObject.hpp
  include/noelle/core/MemoryCloningAnalysis.hpp
  include/noelle/core/SCCAttrs.hpp
  include/noelle/core/Variable.hpp
  include/noelle/core/LoopCarriedDependencies.hpp
  include/noelle/core/Reduction.hpp
  include/noelle/core/BinaryReduction.hpp
  DESTINATION 
  include/noelle/core
  )
