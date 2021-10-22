# Project
cmake_minimum_required(VERSION 3.13)
project(Parallelization)

# Dependences
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/DependencesCMake.txt)

# Pass
add_subdirectory(src)

# Install
install(  
  FILES
  include/SystemHeaders.hpp 
  include/DominatorSummary.hpp
  include/Queue.hpp
  include/ScalarEvolutionReferencer.hpp
  include/ScalarEvolutionDelinearization.hpp
  include/BitMatrix.hpp
  include/Utils.hpp
  DESTINATION 
  include
  )
