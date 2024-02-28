target_sources(
  Noelle # component name
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/AliasAnalysisEngine.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/LoopAliasAnalysisEngine.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ProgramAliasAnalysisEngine.cpp
)
