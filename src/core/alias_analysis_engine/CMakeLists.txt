add_llvm_library(
  AliasAnalysisEngine # pass name
  MODULE
  src/AliasAnalysisEngine.cpp
  src/LoopAliasAnalysisEngine.cpp
  src/ProgramAliasAnalysisEngine.cpp
)
