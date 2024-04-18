install(
  PROGRAMS
    noelle-codesize
    noelle-deadcode
    noelle-fixedpoint
    noelle-loop-size
    noelle-loop-stats
    noelle-pdg-stats
    noelle-privatizer
    noelle-rm-function
  DESTINATION
    bin
)

if(NOELLE_REPL STREQUAL ON)
  install(PROGRAMS noelle-repl DESTINATION bin)
endif()
