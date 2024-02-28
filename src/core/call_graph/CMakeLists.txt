target_sources(
  Noelle # component name
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/CallGraph.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/CallGraphEdge.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/CallGraphNode.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/CallGraphTraits.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/SCCCAG.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/SCCCAGNode.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/SCCCAGNode_Function.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/SCCCAGNode_SCC.cpp
)
