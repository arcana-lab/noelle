add_llvm_library(
  DeadFunctionEliminator # pass name
  MODULE
  src/DeadFunctionEliminator.cpp
  src/Pass.cpp
)
