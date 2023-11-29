# Project
cmake_minimum_required(VERSION 3.13)
project(Parallelization)

# Dependences
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/DependencesCMake.txt)

# Pass
add_subdirectory(src)

# Install
# install(
#   FILES
#   include/noelle/core/SystemHeaders.hpp 
#   include/noelle/core/DominatorSummary.hpp
#   include/noelle/core/Queue.hpp
#   include/noelle/core/ScalarEvolutionReferencer.hpp
#   include/noelle/core/ScalarEvolutionDelinearization.hpp
#   include/noelle/core/BitMatrix.hpp
#   include/noelle/core/Utils.hpp
#   DESTINATION 
#   include/noelle/core
#   )
