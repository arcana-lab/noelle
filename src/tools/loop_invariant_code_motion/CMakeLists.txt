noelle_component_declare(loop_invariant_code_motion)
noelle_component_add_sources(
  loop_invariant_code_motion # component
  FILES
  src/LastLiveOutPeeler.cpp
  src/LastLiveOutStoreHoister.cpp
  src/LoopInvariantCodeMotion.cpp
  src/LoopInvariantHoister.cpp
  src/Mem2RegNonAlloca.cpp
)
noelle_component_commit(
  loop_invariant_code_motion # component name
  LICM # shared library name
)
