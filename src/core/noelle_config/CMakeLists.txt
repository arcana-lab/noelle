# Project
cmake_minimum_required(VERSION 3.13)
project(NoelleConfig)
set(NOELLE_VERSION 9.13.1)
set(NOELLE_DESTINATION ${CMAKE_INSTALL_PREFIX})

# Dependences
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/DependencesCMake.txt)

# Pass
add_subdirectory(src)

# Install
install(PROGRAMS build/src/noelle-config DESTINATION bin)
