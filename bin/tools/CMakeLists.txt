install(
  PROGRAMS
    noelle-codesize
    noelle-deadcode
    noelle-fixedpoint
    noelle-ldg-dot
    noelle-loop-size
    noelle-loop-stats
    noelle-meta-clean
    noelle-meta-loop-clean
    noelle-meta-pdg-clean
    noelle-meta-pdg-embed
    noelle-meta-prof-clean
    noelle-meta-scc-clean
    noelle-meta-scc-embed
    noelle-pdg-stats
    noelle-privatizer
    noelle-profiler
    noelle-rm-function
    noelle-scc-print
  DESTINATION
    bin
)

if(NOELLE_REPL STREQUAL ON)
  install(PROGRAMS noelle-repl DESTINATION bin)
endif()
