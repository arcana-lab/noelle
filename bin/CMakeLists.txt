if(EXISTS ${NOELLE_CMAKE_ROOT}/.git/HEAD)
  execute_process(
    COMMAND git remote get-url --all origin
    WORKING_DIRECTORY ${NOELLE_CMAKE_ROOT}
    OUTPUT_VARIABLE NOELLE_GIT_ORIGIN
    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET
  )
  execute_process(
    COMMAND git log -1 --format=%H
    WORKING_DIRECTORY ${NOELLE_CMAKE_ROOT}
    OUTPUT_VARIABLE NOELLE_GIT_COMMIT
    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET
  )
else()
  set(NOELLE_GIT_ORIGIN "")
  set(NOELLE_GIT_COMMIT "")
endif()

string(REPLACE ";" " " NOELLE_CXX_FLAGS "${NOELLE_CXX_FLAGS}")

configure_file(noelle-config.in noelle-config @ONLY)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/noelle-config DESTINATION bin)

add_subdirectory(core)
add_subdirectory(tools)
