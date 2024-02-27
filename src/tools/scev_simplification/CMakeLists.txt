noelle_component_declare(scev_simplification)
noelle_component_add_sources(
  scev_simplification # component
  FILES
  src/SCEVSimplification.cpp
)
noelle_component_commit(
  scev_simplification # component name
  SCEVSimplification # shared library name
)
