install(
  FILES
    noelle-deadcode
    noelle-fixedpoint
    noelle-loopsize
    noelle-loop-stats
    noelle-pdg-stats
    noelle-privatizer
    noelle-repl
    noelle-rm-function
  DESTINATION
    bin
  PERMISSIONS
    OWNER_EXECUTE OWNER_WRITE OWNER_READ
    GROUP_EXECUTE GROUP_READ
    WORLD_EXECUTE WORLD_READ
)
