add_llvm_library(
  MetadataManager # pass name
  MODULE
  src/MetadataEntry.cpp
  src/MetadataManager_annotations.cpp
  src/MetadataManager.cpp
  src/MetadataManager_Instruction.cpp
  src/MetadataManager_Module.cpp
  src/MetadataManager_PDG.cpp
)
