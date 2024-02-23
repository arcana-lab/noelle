add_llvm_library(
  BasicUtilities # pass name
  MODULE
  src/BitMatrix.cpp
  src/ScalarEvolutionDelinearization.cpp
  src/ScalarEvolutionReferencer.cpp
  src/ScalarEvolutionReferenceTreeExpander.cpp
  src/Utils.cpp
)
