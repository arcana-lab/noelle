add_llvm_library(
  LoopInductionVariables # pass name
  MODULE
  src/InductionVariable.cpp
  src/InductionVariables.cpp
  src/IVStepperUtility.cpp
  src/LoopGoverningInductionVariable.cpp
  src/LoopGoverningIVUtility.cpp
)
