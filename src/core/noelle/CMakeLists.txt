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
  include/Noelle.hpp
  include/FunctionsManager.hpp
  include/TypesManager.hpp
  include/CompilationOptionsManager.hpp
  DESTINATION 
  include
  )
