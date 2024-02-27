noelle_component_declare(remove_function)
noelle_component_add_sources(
  remove_function # component
  FILES
  src/Pass.cpp
  src/RemoveFunction.cpp
)
noelle_component_commit(
  remove_function # component name
  RemoveFunction # shared library name
)

