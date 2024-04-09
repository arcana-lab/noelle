add_llvm_library(
  CallGraph # pass name
  MODULE
  src/CallGraph.cpp
  src/CallGraphEdge.cpp
  src/CallGraphNode.cpp
  src/CallGraphTraits.cpp
  src/SCCCAG.cpp
  src/SCCCAGNode.cpp
  src/SCCCAGEdge.cpp
  src/SCCCAGNode_Function.cpp
  src/SCCCAGNode_SCC.cpp
)
