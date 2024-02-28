target_sources(
  Noelle # component name
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ControlFlowEquivalence.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/LoopAwareMemDepAnalysis.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/LoopContent.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/LoopTransformationsOptions.cpp
)
