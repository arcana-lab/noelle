target_sources(
  Noelle # component name
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/BitMatrix.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ScalarEvolutionDelinearization.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ScalarEvolutionReferencer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ScalarEvolutionReferenceTreeExpander.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Utils.cpp
)
