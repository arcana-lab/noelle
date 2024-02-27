noelle_component_declare(dead_function_eliminator)
noelle_component_add_sources(
  dead_function_eliminator # component
  FILES
  src/DeadFunctionEliminator.cpp
  src/Pass.cpp
)
noelle_component_commit(
  dead_function_eliminator # component name
  DeadFunctionEliminator # shared library name
)
