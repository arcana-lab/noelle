add_custom_command(
  OUTPUT GitInfo.hpp
  COMMAND eval "echo \"#define\" NOELLE_GIT_ORIGIN \"\\\"$(git remote get-url --all origin)\\\"\"" > GitInfo.hpp
  COMMAND eval "echo \"#define\" NOELLE_GIT_COMMIT \"\\\"$(git rev-parse HEAD)\\\"\"" >> GitInfo.hpp
  DEPENDS ${CMAKE_SOURCE_DIR}/.git/HEAD
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  VERBATIM
)
add_custom_target(GitInfo DEPENDS GitInfo.hpp)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
configure_file(src/config.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/config.hpp @ONLY)
add_executable(noelle-config src/Noelle_config.cpp  ${CMAKE_CURRENT_BINARY_DIR}/config.hpp)
add_dependencies(noelle-config GitInfo)

install(
  TARGETS noelle-config
  DESTINATION bin
)
