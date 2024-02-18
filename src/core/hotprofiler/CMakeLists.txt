add_llvm_library(
  HotProfiler # pass name
  MODULE
  src/Hot_BasicBlock.cpp
  src/Hot.cpp
  src/Hot_Function.cpp
  src/Hot_Instruction.cpp
  src/Hot_Loop.cpp
  src/Hot_Module.cpp
  src/HotProfiler.cpp
  src/Hot_SCC.cpp
  src/Pass.cpp
)
