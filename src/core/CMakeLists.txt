include_directories(transformations/include)

set(noelle_modules
  alias_analysis_engine
  alloc_aa
  architecture
  basic_utilities
  call_graph
  call_graph_analysis
  cfg_analysis
  cfg_transformer
  codesize
  compilation_options_manager
  constants_manager
  dataflow
  dependence_analysis
  dg
  dominators
  functions_manager
  globals_manager
  hotprofiler
  ldg_generator
  linker
  loop_carried_dependences
  loop_content
  loop_distribution
  loop_environment
  loop_forest
  loop_induction_variables
  loop_invariants
  loop_iteration_space_analysis
  loop_metadata
  loop_nesting_graph
  loop_scc_attributes
  loop_sccdag_attributes
  loop_sccdag_normalizer
  loop_structure
  loop_transformer
  loop_unroll
  loop_whilifier
  may_points_to_analysis
  memory_cloning_analysis
  metadata_cleaner
  metadata_manager
  noelle
  outliner
  pdg
  pdg_generator
  pdg_printer
  sccdag
  scheduler
  talkdown
  task
  types_manager
  unique_ir_marker
)

foreach(module IN LISTS noelle_modules)
  include_directories(${module}/include)
endforeach()

foreach(module IN LISTS noelle_modules)
  add_subdirectory(${module})
endforeach()

foreach(module IN LISTS noelle_modules)
  install(
    DIRECTORY ${module}/include
    DESTINATION ${CMAKE_INSTALL_PREFIX}
    FILES_MATCHING PATTERN "*.hpp"
  )
endforeach()

add_subdirectory(noelle_config)
