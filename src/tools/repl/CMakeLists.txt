noelle_component_declare(repl)
noelle_component_add_sources(
  repl # component
  FILES
  src/Repl.cpp
)
noelle_component_commit(
  repl # component name
  Repl # shared library name
)

