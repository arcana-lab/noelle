install(
  PROGRAMS
    noelle-codesize
    noelle-deadcode
    noelle-fixedpoint
    noelle-ldg-dot
    noelle-loop-size
    noelle-loop-stats
    noelle-meta-loop-clean
    noelle-meta-pdg-clean
    noelle-meta-scc-clean
    noelle-meta-prof-clean
    noelle-meta-clean
    noelle-pdg-stats
    noelle-privatizer
    noelle-rm-function
    noelle-scc-print
    noelle-meta-pdg-embed
    noelle-meta-scc-embed
  DESTINATION
    bin
)

if(NOELLE_REPL STREQUAL ON)
  install(PROGRAMS noelle-repl DESTINATION bin)
endif()
