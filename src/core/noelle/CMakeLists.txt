# Project
cmake_minimum_required(VERSION 3.13)
project(Noelle)

# Dependences
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/DependencesCMake.txt)

# Pass
add_subdirectory(src)

# Install
install(
  FILES
  include/noelle/core/Noelle.hpp
  include/noelle/core/FunctionsManager.hpp
  include/noelle/core/TypesManager.hpp
  include/noelle/core/ConstantsManager.hpp
  include/noelle/core/CompilationOptionsManager.hpp
  DESTINATION 
  include/noelle/core
  )
