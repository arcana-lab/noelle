add_llvm_library(
  UniqueIRMarker # pass name
  MODULE
  src/IDToValueMapper.cpp
  src/UniqueIRConstants.cpp
  src/UniqueIRMarker.cpp
  src/UniqueIRMarkerPass.cpp
  src/UniqueIRMarkerReader.cpp
  src/UniqueIRVerifier.cpp
)
