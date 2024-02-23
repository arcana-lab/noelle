add_llvm_library(
  LoopSCCDAGAttributes # pass name
  MODULE
  src/SCCDAGAttrs.cpp
  src/SCCDAGPartition.cpp
)
