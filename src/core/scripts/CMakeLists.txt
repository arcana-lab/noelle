install(
  FILES
    loopaa
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
    noelle-norm
    noelle-pdg
    noelle-prof-coverage
    noelle-simplification
  DESTINATION
    bin
  PERMISSIONS
    OWNER_EXECUTE OWNER_WRITE OWNER_READ
    GROUP_EXECUTE GROUP_READ
    WORLD_EXECUTE WORLD_READ
)
