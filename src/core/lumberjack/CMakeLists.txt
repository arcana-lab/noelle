configure_file(src/Config.hpp.in Config.hpp @ONLY)

target_sources(
  Noelle # component name
  PRIVATE
  src/Guard.cpp
  src/Logger.cpp
  src/Lumberjack.cpp
  src/Sections.cpp
)
target_include_directories(Noelle PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

install(
  FILES
  Lumberjack.json
  DESTINATION
  share/noelle
)
