noelle_component_declare(pdg_stats)
noelle_component_add_sources(
  pdg_stats # component
  FILES
  src/Pass.cpp
  src/PDGStats.cpp
)
noelle_component_commit(
  pdg_stats # component name
  PDGStats # shared library name
)
