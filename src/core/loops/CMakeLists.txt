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
  include/noelle/core/MemoryCloningAnalysis.hpp
  include/noelle/core/Variable.hpp
  include/noelle/core/ControlFlowEquivalence.hpp
  include/noelle/core/LoopDependenceInfo.hpp
  include/noelle/core/LoopIterationDomainSpaceAnalysis.hpp
  include/noelle/core/SCCDAGAttrs.hpp
  include/noelle/core/SCCDAGNormalizer.hpp
  include/noelle/core/SCCDAGPartition.hpp
  include/noelle/core/SCCAttrs.hpp
  include/noelle/core/AccumulatorOpInfo.hpp
  include/noelle/core/LoopTransformationsOptions.hpp
  DESTINATION 
  include/noelle/core
  )
