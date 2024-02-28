target_sources(
  Noelle # component name
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Noelle.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Noelle_dependences.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Noelle_function.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Noelle_loops.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Noelle_transformations.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Pass.cpp
)
