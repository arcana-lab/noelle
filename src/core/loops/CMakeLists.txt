# Project
cmake_minimum_required(VERSION 3.13)
project(Loops)

# Dependences
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/DependencesCMake.txt)

# Pass
add_subdirectory(src)

# Install
install(
  FILES
  include/MemoryCloningAnalysis.hpp
  include/Variable.hpp
  include/ControlFlowEquivalence.hpp
  include/LoopDependenceInfo.hpp
  include/LoopIterationDomainSpaceAnalysis.hpp
  include/SCCDAGAttrs.hpp
  include/SCCDAGNormalizer.hpp
  include/SCCDAGPartition.hpp
  include/SCCAttrs.hpp
  include/AccumulatorOpInfo.hpp
  DESTINATION 
  include
  )
