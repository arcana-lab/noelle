add_llvm_library(
  Noelle # pass name
  MODULE
  src/Noelle.cpp
  src/Noelle_dependences.cpp
  src/Noelle_function.cpp
  src/Noelle_loops.cpp
  src/Noelle_transformations.cpp
  src/Pass.cpp
)
