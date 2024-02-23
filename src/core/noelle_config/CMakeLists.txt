if(EXISTS ${CMAKE_SOURCE_DIR}/.git/HEAD)
  execute_process(
    COMMAND git remote get-url --all origin
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE NOELLE_GIT_ORIGIN
    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET
  )
  execute_process(
    COMMAND git log -1 --format=%H
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE NOELLE_GIT_COMMIT
    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET
  )
else()
  set(NOELLE_GIT_ORIGIN "")
  set(NOELLE_GIT_COMMIT "")
endif()

configure_file(src/config.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/config.hpp @ONLY)
string(REPLACE ";" " " NOELLE_CXX_FLAGS "${NOELLE_CXX_FLAGS}")
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(noelle-config src/Noelle_config.cpp)

install(
  TARGETS noelle-config
  DESTINATION bin
)
