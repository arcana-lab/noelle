target_sources(
  Noelle # component name
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/InductionVariable.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/InductionVariables.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/IVStepperUtility.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/LoopGoverningInductionVariable.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/LoopGoverningIVUtility.cpp
)
