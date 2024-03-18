add_llvm_library(
  LoopInvariantCodeMotion # pass name
  MODULE
  src/LastLiveOutPeeler.cpp
  src/LastLiveOutStoreHoister.cpp
  src/LoopInvariantCodeMotion.cpp
  src/LoopInvariantHoister.cpp
  src/Mem2RegNonAlloca.cpp
)
