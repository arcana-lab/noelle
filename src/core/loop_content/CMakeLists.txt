add_llvm_library(
  LoopContent # pass name
  MODULE
  src/ControlFlowEquivalence.cpp
  src/LoopAwareMemDepAnalysis.cpp
  src/LoopContent.cpp
  src/LoopTransformationsOptions.cpp
)
