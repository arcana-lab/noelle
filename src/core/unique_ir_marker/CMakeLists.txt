target_sources(
  Noelle # component name
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/IDToValueMapper.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/UniqueIRConstants.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/UniqueIRMarker.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/UniqueIRMarkerPass.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/UniqueIRMarkerReader.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/UniqueIRVerifier.cpp
)
