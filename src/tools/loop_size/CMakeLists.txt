noelle_component_declare(loop_size)
noelle_component_add_sources(
  loop_size # component
  FILES
  src/LoopSize.cpp
  src/Pass.cpp
)
noelle_component_commit(
  loop_size # component name
  LoopSize # shared library name
)
