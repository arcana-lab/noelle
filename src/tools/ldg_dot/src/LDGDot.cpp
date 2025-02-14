#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <unordered_set>
#include <utility>

#include "llvm/IR/Value.h"
#include "arcana/noelle/core/DataDependence.hpp"
#include "arcana/noelle/core/LoopCarriedSCC.hpp"
#include "arcana/noelle/core/LoopCarriedUnknownSCC.hpp"
#include "arcana/noelle/core/MemoryDependence.hpp"
#include "arcana/noelle/core/Noelle.hpp"
#include "arcana/noelle/core/SCCDAGAttrs.hpp"
#include "LeptoInstVisitor.hpp"

#include "LDGDot.hpp"

using namespace std;
using namespace llvm;

namespace arcana::noelle {

string patchTemplate(const string &templateStr,
                     const map<string, string> &patches) {
  string result = templateStr;
  for (const auto &[oldStr, newStr] : patches) {
    size_t pos = 0;
    while ((pos = result.find(oldStr, pos)) != string::npos) {
      result.replace(pos, oldStr.length(), newStr);
      pos += newStr.length();
    }
  }
  return result;
}

void emitDotFile(const string &dotContent, string outputFile) {
  ofstream outputStream(outputFile);
  assert(outputStream.is_open());
  outputStream << dotContent;
  outputStream.close();
}

string pointerToString(const Value *V) {
  ostringstream oss;
  oss << std::setw(8) << std::setfill('0') << std::hex
      << reinterpret_cast<uintptr_t>(V);
  return oss.str();
}

string SCCKindToString(GenericSCC::SCCKind type) {
  switch (type) {
    case GenericSCC::LOOP_CARRIED:
      return "Loop Carried";
    case GenericSCC::REDUCTION:
      return "Reduction";
    case GenericSCC::BINARY_REDUCTION:
      return "Binary Reduction";
    case GenericSCC::RECOMPUTABLE:
      return "Recomputable";
    case GenericSCC::SINGLE_ACCUMULATOR_RECOMPUTABLE:
      return "Single Accumulator Recomputable";
    case GenericSCC::INDUCTION_VARIABLE:
      return "IV";
    case GenericSCC::LINEAR_INDUCTION_VARIABLE:
      return "Linear IV";
    case GenericSCC::PERIODIC_VARIABLE:
      return "Periodic Variable";
    case GenericSCC::UNKNOWN_CLOSED_FORM:
      return "Unknown Closed Form";
    case GenericSCC::MEMORY_CLONABLE:
      return "Memory Clonable";
    case GenericSCC::STACK_OBJECT_CLONABLE:
      return "Stack Object Clonable";
    case GenericSCC::LOOP_ITERATION:
      return "Loop Iteration";
    case GenericSCC::LOOP_CARRIED_UNKNOWN:
      return "";
    default:
      return to_string(type);
  }
}

string fixEscapes(const string &str) {
  string escaped;
  for (char c : str) {
    if (c == '"') {
      escaped += "\\\""; // Add escaped double quote
    } else if (c == '\\') {
      escaped += "\\\\"; // Add escaped double quote
    } else {
      escaped += c;
    }
  }
  return escaped;
}

void exportToDotGraph(LoopContent *LC,
                      string outputFile,
                      DotOptions options,
                      DependenceAnalysis *DA) {

  auto LS = LC->getLoopStructure();
  auto SCCManager = LC->getSCCManager();
  auto SCCDAG = SCCManager->getSCCDAG();

  LeptoInstVisitor lepto;

  string graphTemplate =
      "digraph G {\n"
      "graph [style=\"filled,rounded\", fillcolor=\"white\", layout=\"fdp\"]\n"
      "node [color=\"transparent\", fontname=\"Verdana\"]\n"
      "@SUBGRAPHS@\n"
      "@EDGES@\n"
      "}\n";
  string subgraphTemplate = "subgraph cluster_scc@ID@ {\n"
                            "\tlabel=\"@LABEL@\"\n"
                            "\tcolor=\"@COLOR@\"\n"
                            "@NODES@"
                            "}\n";
  string nodeTemplate = "\t@ID@ [label=\"@LABEL@\"]\n";
  string edgeTemplate =
      "\t@SRC@ -> @DST@ [color=\"@COLOR@\", style=\"@STYLE@\", arrowhead=\"@ARROWHEAD@\"]\n";

  map<string, string> graph;
  set<pair<Value *, Value *>> addedEdges;
  unordered_set<Value *> addedNodes;
  set<tuple<Value *, Value *, DataDependenceType>> a;

  graph["@EDGES@"] = "";
  int subgraphId = 0;

  for (auto SCCNode : SCCDAG->getSCCs()) {
    auto genericSCC = SCCManager->getSCCAttrs(SCCNode);
    map<string, string> subgraph;
    subgraph["@ID@"] = to_string(subgraphId);
    subgraph["@LABEL@"] = SCCKindToString(genericSCC->getKind());
    if (isa<LoopCarriedUnknownSCC>(genericSCC)) {
      // unknown
      subgraph["@COLOR@"] = "red";
    } else {
      // known
      if (options & HIDE_KNOWN_SCCS) {
        continue;
      }
      subgraph["@COLOR@"] = "green";
    }
    for (auto I : SCCNode->getInstructions()) {
      map<string, string> node;
      auto srcId = "i" + pointerToString(I);
      node["@ID@"] = srcId;
      node["@LABEL@"] = fixEscapes(lepto(*I));
      subgraph["@NODES@"] += patchTemplate(nodeTemplate, node);
      addedNodes.insert(I);
    }
    auto deps = SCCNode->getEdges();
    for (auto dep : deps) {
      map<string, string> node;
      map<string, string> edge;
      auto src = dep->getSrc();
      auto dst = dep->getDst();
      auto srcId = "i" + pointerToString(src);
      auto dstId = "i" + pointerToString(dst);

      edge["@SRC@"] = srcId;
      edge["@DST@"] = dstId;

      if (isa<ControlDependence<Value, Value>>(dep)) {
        edge["@STYLE@"] = "dashed";
        edge["@ARROWHEAD@"] = "normal";
        edge["@COLOR@"] = "lightskyblue";
      } else {
        edge["@STYLE@"] = "solid";
        auto DD = cast<DataDependence<Value, Value>>(dep);
        if (options & COLLAPSE_EDGES) {
          edge["@ARROWHEAD@"] = "none";
        } else {
          if (DD->isRAWDependence()) {
            if (isa<MemoryDependence<Value, Value>>(dep)) {
              edge["@ARROWHEAD@"] = "normal";
            } else if (isa<VariableDependence<Value, Value>>(dep)) {
              edge["@ARROWHEAD@"] = "empty";
            }
          } else if (DD->isWARDependence()) {
            if (isa<MemoryDependence<Value, Value>>(dep)) {
              edge["@ARROWHEAD@"] = "inv";
            } else if (isa<VariableDependence<Value, Value>>(dep)) {
              edge["@ARROWHEAD@"] = "invempty";
            }
          } else if (DD->isWAWDependence()) {
            if (isa<MemoryDependence<Value, Value>>(dep)) {
              edge["@ARROWHEAD@"] = "dot";
            } else if (isa<VariableDependence<Value, Value>>(dep)) {
              edge["@ARROWHEAD@"] = "odot";
            }
          }
        }
        if (dep->isLoopCarriedDependence()) {
          if (isa<LoopCarriedUnknownSCC>(genericSCC)) {
            if (DA && !DA->canThisDependenceBeLoopCarried(dep, *LS)) {
              // terminable
              edge["@COLOR@"] = "orange";
            } else {
              // non-terminable
              edge["@COLOR@"] = "red";
            }
          } else {
            edge["@COLOR@"] = "green";
          }
        } else {
          edge["@COLOR@"] = "lightgrey";
        }
      }

      bool add = true;
      if (options & COLLAPSE_EDGES) {
        if (addedEdges.find({ src, dst }) != addedEdges.end()
            || addedEdges.find({ dst, src }) != addedEdges.end()) {
          add = false;
        }
      }
      if (options & ONLY_LC_EDGES) {
        if (!dep->isLoopCarriedDependence()) {
          add = false;
        }
      }
      if (!(options & SHOW_CONTROL_DEPS)) {
        if (isa<ControlDependence<Value, Value>>(dep)) {
          add = false;
        }
      }
      if (add) {
        graph["@EDGES@"] += patchTemplate(edgeTemplate, edge);
        addedEdges.insert({ src, dst });
      }
    }

    graph["@SUBGRAPHS@"] += patchTemplate(subgraphTemplate, subgraph);
    subgraphId++;
  }

  string dotContent = patchTemplate(graphTemplate, graph);
  emitDotFile(dotContent, outputFile);
}

} // namespace arcana::noelle
