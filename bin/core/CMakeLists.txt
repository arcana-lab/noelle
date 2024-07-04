string(REPLACE ";" " \\\n  " noelle_load_SCAF_LIBS "${noelle_load_SCAF_LIBS}")
string(REPLACE ";" " \\\n  " noelle_load_SCAF_ANALYSES "${noelle_load_SCAF_ANALYSES}")
string(REPLACE ";" " \\\n  " noelle_load_SVF_LIBS "${noelle_load_SVF_LIBS}")
string(REPLACE ";" " \\\n  " noelle_load_SVF_ANALYSES "${noelle_load_SVF_ANALYSES}")
string(REPLACE ";" " \\\n  " noelle_load_CORE_LIBS "${noelle_load_CORE_LIBS}")
string(REPLACE ";" " \\\n  " noelle_load_LLVM_ANALYSES "${noelle_load_LLVM_ANALYSES}")
string(REPLACE ";" " \\\n  " noelle_norm_OPT_ARGS "${noelle_norm_OPT_ARGS}")
string(REPLACE ";" " \\\n  " noelle_simplification_SVF_LIBS "${noelle_simplification_SVF_LIBS}")
string(REPLACE ";" " \\\n  " noelle_simplification_SVF_ANALYSES "${noelle_simplification_SVF_ANALYSES}")

configure_file(noelle-load.in noelle-load @ONLY)
configure_file(noelle-norm.in noelle-norm @ONLY)
configure_file(noelle-simplification.in noelle-simplification @ONLY)

install(
  PROGRAMS
    noelle-meta-loop-embed
    noelle-meta-prof-embed
    noelle-pdg
    noelle-prof-coverage
    ${CMAKE_CURRENT_BINARY_DIR}/noelle-load
    ${CMAKE_CURRENT_BINARY_DIR}/noelle-norm
    ${CMAKE_CURRENT_BINARY_DIR}/noelle-simplification
  DESTINATION
    bin
)
