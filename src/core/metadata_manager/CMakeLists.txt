target_sources(
  Noelle # component name
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/MetadataEntry.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/MetadataManager_annotations.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/MetadataManager.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/MetadataManager_Instruction.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/MetadataManager_Module.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/MetadataManager_PDG.cpp
)
