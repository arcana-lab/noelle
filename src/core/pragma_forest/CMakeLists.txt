target_sources(
  Noelle # component name
  PRIVATE
  src/PragmaEraserPass.cpp
  src/PragmaForest.cpp
  src/PragmaTree.cpp
)

install(
  FILES include/arcana/noelle/core/Pragma.h
  DESTINATION include/arcana/noelle/core/Pragma.h
)
