include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(noelle-config src/Noelle_config.cpp)

configure_file(src/config.hpp.in config.hpp @ONLY)

install(
  TARGETS noelle-config
  DESTINATION bin
)
