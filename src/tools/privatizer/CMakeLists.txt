add_llvm_library(
  Privatizer # pass name
  MODULE
  src/GlobalToStack.cpp
  src/HeapToStack.cpp
  src/Pass.cpp
  src/Privatizer.cpp
  src/Utils.cpp
)
