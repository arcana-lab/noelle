target_sources(
  Noelle # component name
  PRIVATE
  src/CallGraph.cpp
  src/CallGraphEdge.cpp
  src/CallGraphNode.cpp
  src/CallGraphTraits.cpp
  src/SCCCAG.cpp
  src/SCCCAGNode.cpp
  src/SCCCAGNode_Function.cpp
  src/SCCCAGNode_SCC.cpp
)
