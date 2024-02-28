configure_file(noelle-norm.in noelle-norm @ONLY)
configure_file(noelle-simplification.in noelle-simplification @ONLY)

install(
  PROGRAMS
    noelle-codesize
    noelle-load
    noelle-meta-clean
    noelle-meta-loop-clean
    noelle-meta-loop-embed
    noelle-meta-pdg-clean
    noelle-meta-pdg-embed
    noelle-meta-prof-clean
    noelle-meta-prof-embed
    noelle-meta-scc-clean
    noelle-meta-scc-embed
    noelle-pdg
    noelle-prof-coverage
    ${CMAKE_CURRENT_BINARY_DIR}/noelle-norm
    ${CMAKE_CURRENT_BINARY_DIR}/noelle-simplification
  DESTINATION
    bin
)
