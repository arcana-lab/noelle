target_sources(
  Noelle # component name
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/AnalysisPass.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/IntegrationWithSVF.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Pass.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PDGGenerator_callGraph.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PDGGenerator_compare.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PDGGenerator_controlDependences.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PDGGenerator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PDGGenerator_library.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PDGGenerator_memory.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PDGGenerator_metadata.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PDGGenerator_metadata_embedder.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PDGGenerator_metadata_scc_embedder.cpp
)
