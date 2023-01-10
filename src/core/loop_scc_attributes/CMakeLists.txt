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
  include/noelle/core/SCCAttrs.hpp
  include/noelle/core/LoopCarriedSCC.hpp
  include/noelle/core/LoopIterationSCC.hpp
  include/noelle/core/ReductionSCC.hpp
  include/noelle/core/BinaryReductionSCC.hpp
  include/noelle/core/RecomputableSCC.hpp
  include/noelle/core/InductionVariableSCC.hpp
  include/noelle/core/LinearInductionVariableSCC.hpp
  include/noelle/core/MemoryClonableSCC.hpp
  include/noelle/core/StackObjectClonableSCC.hpp
  include/noelle/core/LoopCarriedUnknownSCC.hpp
  DESTINATION 
  include/noelle/core
  )
