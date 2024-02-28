target_sources(
  Noelle # component name
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/LoopCarriedCycle.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/LoopCarriedDependencies.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/LoopCarriedVariable.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Variable.cpp
)
