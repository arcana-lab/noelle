target_sources(
  Noelle # component name
  PRIVATE
  src/PragmaEraserPass.cpp
  src/PragmaForest.cpp
  src/PragmaTree.cpp
)

add_library(pragma SHARED
  src/SharedLib.cpp
)

install(
  FILES
    include/arcana/noelle/core/PragmaDecl.h
    include/arcana/noelle/core/PragmaDef.h
    include/arcana/noelle/core/Pragma.h
  DESTINATION
    include/arcana/noelle/core
)

install(
  TARGETS pragma
  LIBRARY DESTINATION lib
)
