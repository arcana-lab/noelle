/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/Analysis/DOTGraphTraitsPass.h"
#include "llvm/Analysis/DomPrinter.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Support/DOTGraphTraits.h"

#include <set>
#include <queue>

#include "PDGPrinter.hpp"
#include "PDGAnalysis.hpp"

using namespace llvm;

namespace llvm::noelle {

  PDGPrinter::PDGPrinter () {
    return ;
  }

  void
  PDGPrinter::printPDG (
    Module &module, 
    llvm::CallGraph &callGraph, 
    PDG *graph,
    std::function<LoopInfo& (Function *f)> getLoopInfo
    ){

    /*
    * Collect functions through call graph starting at function "main"
    */
    std::set<Function *> funcToGraph;
    collectAllFunctionsInCallGraph(module, callGraph, funcToGraph);

    /*
    * Print the PDG
    */
    DGPrinter::writeClusteredGraph<PDG, Value>("pdg-full.dot", graph);
    for (auto F : funcToGraph) {
      auto& LI = getLoopInfo(F);
      printGraphsForFunction(*F, graph, LI);
    }

    return ;
  }

  void
  PDGPrinter::collectAllFunctionsInCallGraph (
    Module &M,
    llvm::CallGraph &callGraph,
    std::set<Function *> &funcSet
    ) {
    std::queue<Function *> funcToTraverse;
    funcToTraverse.push(M.getFunction("main"));
    while (!funcToTraverse.empty())
    {
      auto func = funcToTraverse.front();
      funcToTraverse.pop();
      if (funcSet.find(func) != funcSet.end()) continue;
      funcSet.insert(func);

      auto funcCGNode = callGraph[func];
      for (auto &callRecord : make_range(funcCGNode->begin(), funcCGNode->end()))
      {
        auto F = callRecord.second->getFunction();
        if (!F) {
          continue ;
        }
        if (F->empty()) {
          continue;
        }
        funcToTraverse.push(F);
      }
    }
  }

  void PDGPrinter::printGraphsForFunction(Function &F, PDG *graph, LoopInfo &LI) {

    /*
    * Print the DG of the function.
    */
    std::string filename;
    raw_string_ostream ros(filename);
    ros << "pdg-function-" << F.getName() << ".dot";
    auto subgraph = graph->createFunctionSubgraph(F);
    DGPrinter::writeClusteredGraph<PDG, Value>(ros.str(), subgraph);
    delete subgraph;

    /*
    * Check if the function has loops.
    */
    if (LI.empty()) return ;

    /*
    * Print the DG of each loop.
    */
    auto loopCount = 0;
    for (auto currentLoop : LI.getLoopsInPreorder()){

      /*
      * Print the loop DG.
      */
      filename.clear();
      ros << "pdg-function-" << F.getName() << "-loop" << loopCount << ".dot";
      subgraph = graph->createLoopsSubgraph(currentLoop);
      DGPrinter::writeClusteredGraph<PDG, Value>(ros.str(), subgraph);

      /*
      * Print the SCCDAG of the loop.
      */
      filename.clear();
      ros << "pdg-function-" << F.getName() << "-loop" << loopCount << "-SCCDAG.dot";
      auto sccSubgraph = new SCCDAG(subgraph);
      DGPrinter::writeClusteredGraph<SCCDAG, SCC>(ros.str(), sccSubgraph);

      /*
      * Print each SCC within the loop SCCDAG.
      */
      auto sccCount = 0;
      for (auto scc : sccSubgraph->getSCCs()){
        filename.clear();
        ros << "pdg-function-" << F.getName() << "-loop" << loopCount << "-SCCDAG-SCC" << sccCount << ".dot";
        DGPrinter::writeClusteredGraph<SCC, Value>(ros.str(), scc);
        sccCount++;
      }
      
      /*
      * Free the memory
      */
      delete sccSubgraph;
      delete subgraph;

      /*
      * Increase the loop count ID
      */
      loopCount++;
    }

    return ;
  }

  void DGPrinter::addClusteringToDotFile (std::string inputFileName, std::string outputFileName) {
    ifstream ifile(inputFileName);
    unordered_map<std::string, std::set<std::string>> clusterNodes;

    if (!ifile.is_open()) {
      errs() << "ERROR: Couldn't open dot file: " << inputFileName << "\n";
      return;
    }

    int numLines = 0;
    groupNodesByCluster(clusterNodes, numLines, ifile);

    if (clusterNodes.size() == 0) {
      errs() << "ERROR: No clusters found\n";
      ifile.close();
      return;
    }

    ifile.clear();
    ifile.seekg(0, ios::beg);

    ofstream cfile;
    cfile.open(outputFileName);
    if (!cfile.is_open()) {
      errs() << "ERROR: Couldn't open dot files: " << inputFileName << "," << outputFileName << "\n";
      ifile.close();
      return;
    }

    string line;
    for (int i = 0; i < numLines - 1; ++i) {
      getline(ifile, line);
      cfile << line << "\n";
    }

    writeClusterToFile(clusterNodes, cfile);

    getline(ifile, line);
    cfile << line;
    cfile.close();
    ifile.close();
  }

  void DGPrinter::writeClusterToFile (const unordered_map<std::string, std::set<std::string>> &clusterNodes, ofstream &cfile) {
    for (auto clusterNodesPair : clusterNodes) {
      std::string indent = "    ";
      cfile << "\n";
      cfile << indent << "subgraph cluster_" << clusterNodesPair.first << " {\n";
      cfile << indent << indent << "label=\"" << clusterNodesPair.first << "\";\n";
      for (auto node : clusterNodesPair.second) {
        cfile << indent << indent << node << ";\n";
      }
      cfile << indent << "}\n";
    }
  }

  void DGPrinter::groupNodesByCluster (unordered_map<std::string, std::set<std::string>> &clusterNodes, int &numLines, ifstream &ifile) {
    std::string CLUSTER_KEY = "cluster=";
    std::string NODE_NAME = "Node";
    std::string line;
    while (getline(ifile, line)) {
      ++numLines;
      // errs() << "Found line: " << line << "\n";
      int nodeIndex = line.find(NODE_NAME);
      if (nodeIndex == std::string::npos) continue;

      if (line.find(NODE_NAME, nodeIndex + NODE_NAME.length()) != std::string::npos) continue;
      // errs() << "Is node line\n";

      std::size_t nodeEndIndex = line.find("[", nodeIndex);
      if (nodeEndIndex == std::string::npos) continue;
      std::string nodeName = line.substr(nodeIndex, nodeEndIndex - nodeIndex);

      std::size_t clusterIndex = line.find(CLUSTER_KEY);
      std::size_t clusterEndIndex = line.find(",", clusterIndex);
      if (clusterIndex == std::string::npos || clusterEndIndex == std::string::npos) continue;

      auto clusterNameSize = clusterEndIndex - clusterIndex - CLUSTER_KEY.size();
      std::string clusterName = line.substr(clusterIndex + CLUSTER_KEY.size(), clusterNameSize);
      // errs() << "Has cluster name: " << clusterName << "\n";
      clusterNodes[clusterName].insert(nodeName);
    }
  }
}
