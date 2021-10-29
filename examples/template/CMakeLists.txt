# Project
cmake_minimum_required(VERSION 3.13)
project(CAT)

# Dependences
include(${CMAKE_CURRENT_SOURCE_DIR}/../../../src/scripts/DependencesCMake.txt)

# Pass
add_subdirectory(src)
