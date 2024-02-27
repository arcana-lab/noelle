noelle_component_declare(loop_stats)
noelle_component_add_sources(
  loop_stats # component
  FILES
  src/LoopStats.cpp
  src/LoopStats_Dependences.cpp
  src/LoopStats_InductionVariables.cpp
  src/LoopStats_Invariants.cpp
  src/LoopStats_Loops.cpp
  src/LoopStats_Printer.cpp
  src/Pass.cpp
)
noelle_component_commit(
  loop_stats # component name
  LoopStats # shared library name
)
