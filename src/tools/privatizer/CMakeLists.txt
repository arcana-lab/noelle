noelle_component_declare(privatizer)
noelle_component_add_sources(
  privatizer # component
  FILES
  src/GlobalToStack.cpp
  src/HeapToStack.cpp
  src/Pass.cpp
  src/Privatizer.cpp
  src/Utils.cpp
)
noelle_component_commit(
  privatizer # component name
  Privatizer # shared library name
)
