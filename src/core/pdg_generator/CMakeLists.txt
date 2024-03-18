add_llvm_library(
  PDGGenerator # pass name
  MODULE
  src/AnalysisPass.cpp
  src/IntegrationWithSVF.cpp
  src/Pass.cpp
  src/PDGGenerator_callGraph.cpp
  src/PDGGenerator_compare.cpp
  src/PDGGenerator_controlDependences.cpp
  src/PDGGenerator.cpp
  src/PDGGenerator_library.cpp
  src/PDGGenerator_memory.cpp
  src/PDGGenerator_metadata.cpp
  src/PDGGenerator_metadata_embedder.cpp
  src/PDGGenerator_metadata_scc_embedder.cpp
)
