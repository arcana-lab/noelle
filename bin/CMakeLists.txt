# noelle-load

set(noelle_load_SVF_LIBS "")
set(noelle_load_SVF_ANALYSES "")
set(noelle_load_SVF_PASS "")
set(noelle_load_SCAF_LIBS "")
set(noelle_load_SCAF_ANALYSES "")
set(noelle_load_SCAF_PASS "")
set(noelle_load_CORE_LIBS "")

get_target_property(NOELLE_LIBRARIES noelle_libraries NAMES)
set(noelle_load_CORE_LIBS "")
foreach(lib IN LISTS NOELLE_LIBRARIES)
  list(APPEND noelle_load_CORE_LIBS
    "-load ${CMAKE_INSTALL_PREFIX}/lib/${lib}.so"
  )
endforeach()

if(NOELLE_SCAF STREQUAL "ON")
  list(APPEND noelle_load_SCAF_LIBS
    "-load ${CMAKE_INSTALL_PREFIX}/lib/libSCAFUtilities.so"
    "-load ${CMAKE_INSTALL_PREFIX}/lib/libMemoryAnalysisModules.so"
  )
  list(APPEND noelle_load_SCAF_ANALYSES
    -basic-loop-aa
    -scev-loop-aa
    -auto-restrict-aa
    -intrinsic-aa
    -global-malloc-aa
    -pure-fun-aa
    -semi-local-fun-aa

    -phi-maze-aa
    -no-capture-global-aa
    -no-capture-src-aa
    -type-aa
    -no-escape-fields-aa
    -acyclic-aa

    -disjoint-fields-aa
    -field-malloc-aa
    -loop-variant-allocation-aa
    -std-in-out-err-aa
    -array-of-structures-aa

    -kill-flow-aa
    -callsite-depth-combinator-aa
    -unique-access-paths-aa

    -llvm-aa-results 
  )
  list(APPEND noelle_load_SCAF_PASS -noelle-scaf)
endif()

if(NOELLE_SVF STREQUAL "ON")
  list(APPEND noelle_load_SVF_LIBS
    "-load ${CMAKE_INSTALL_PREFIX}/lib/libSvf.so"
    "-load ${CMAKE_INSTALL_PREFIX}/lib/libCudd.so"
  )
  list(APPEND noelle_load_SVF_ANALYSES -stat=false)
  list(APPEND noelle_load_SVF_PASS -noelle-svf)
endif()

set(noelle_load_LLVM_ANALYSES
  -disable-basicaa
  -globals-aa
  -cfl-steens-aa
  -tbaa
  -scev-aa
  -cfl-anders-aa
  --objc-arc-aa
)

# noelle-norm

set(noelle_norm_SVF_LIBS ${noelle_load_SVF_LIBS})
if(NOELLE_SVF STREQUAL "ON")
  set(noelle_norm_SVF_ANALYSES
    -stat=false
    -break-constgeps
  )
endif()

set(args
  -basicaa
  ${noelle_norm_SVF_LIBS}
  -mem2reg
  -simplifycfg-sink-common=false
  ${noelle_norm_SVF_ANALYSES}
  -lowerswitch
  -mergereturn
  -break-crit-edges
  -loop-simplify
  -lcssa
  -indvars 
  -functionattrs 
  -rpo-functionattrs
)

# generating `opt` arguments for noelle-norm
set(noelle_norm_OPT_ARGS "")
foreach(arg IN LISTS args)
  if (NOT arg STREQUAL "")
    list(APPEND noelle_norm_OPT_ARGS "${arg}")
  endif()
endforeach()

# noelle-simplification

set(noelle_simplification_SVF_LIBS ${noelle_load_SVF_LIBS})
if(NOELLE_SVF STREQUAL "ON")
  set(noelle_simplification_SVF_ANALYSES
    -stat=false
    -break-constgeps
  )
endif()

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
string(REPLACE ";" " " NOELLE_CONFIG_CORE_LIBS "${noelle_load_CORE_LIBS}")
string(REPLACE ";" " " NOELLE_CONFIG_SVF_LIBS "${noelle_load_SVF_LIBS}")
string(REPLACE ";" " " NOELLE_CONFIG_SCAF_LIBS "${noelle_load_SCAF_LIBS}")
string(REPLACE ";" " " NOELLE_CONFIG_SVF_ANALYSES "${noelle_load_SVF_ANALYSES}")
string(REPLACE ";" " " NOELLE_CONFIG_SCAF_ANALYSES "${noelle_load_SCAF_ANALYSES}")
string(REPLACE ";" " " NOELLE_CONFIG_LLVM_ANALYSES "${noelle_load_LLVM_ANALYSES}")

get_target_property(NOELLE_TOOL_LIBRARIES noelle_tool_libraries NAMES)
set(NOELLE_CONFIG_TOOL_LIBS "")
foreach(lib IN LISTS NOELLE_TOOL_LIBRARIES)
  list(APPEND NOELLE_CONFIG_TOOL_LIBS
    "-load ${CMAKE_INSTALL_PREFIX}/lib/${lib}.so"
  )
endforeach()
string(REPLACE ";" " " NOELLE_CONFIG_TOOL_LIBS "${NOELLE_CONFIG_TOOL_LIBS}")

configure_file(noelle-config.in noelle-config @ONLY)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/noelle-config DESTINATION bin)
install(PROGRAMS noelle-eval DESTINATION bin)

add_subdirectory(core)

if(NOELLE_TOOLS STREQUAL ON)
  add_subdirectory(tools)
endif()
