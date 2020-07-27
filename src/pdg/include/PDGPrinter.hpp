/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/CallGraph.h"

#include "DGBase.hpp"
#include "DGGraphTraits.hpp"
#include "PDG.hpp"
#include "SCCDAG.hpp"

#include "llvm/ADT/GraphTraits.h"
#include "llvm/Analysis/DOTGraphTraitsPass.h"
#include "llvm/Analysis/DomPrinter.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Support/DOTGraphTraits.h"

#include <set>
#include <queue>
#include <unordered_map>
#include <iostream>
#include <fstream>

using namespace llvm;

namespace llvm {

  class DGPrinter {
    public:

      template <class GT, class T>
      static bool writeGraph(const std::string& filename, GT *graph) {
        errs() << "Writing '" << filename << "'...\n";

        DGGraphWrapper<GT, T> graphWrapper(graph);

        std::error_code EC;
        raw_fd_ostream File(filename, EC, sys::fs::F_Text);
        std::string Title = filename; // TODO: DOTGraphTraits<GT *>::getGraphName(graphWrapper);

        if (!EC) {
          WriteGraph(File, &graphWrapper, false, Title);
					File.close();
          return true;
        }

        errs() << "  error opening file for writing!\n";
        return false;
      }

      template <class GT, class T>
      static bool writeClusteredGraph(const std::string& filename, GT *graph) {
        const std::string unclusteredFilename = "_unclustered_" + filename;
        if (writeGraph<GT, T>(unclusteredFilename, graph)) {
          addClusteringToDotFile(unclusteredFilename, filename);
          return true;
        }

        return false;
      }

    private:

      static void addClusteringToDotFile(std::string inputFileName, std::string outputFileName);
      static void groupNodesByCluster(unordered_map<std::string, std::set<std::string>> &clusterNodes, int &numLines, ifstream &ifile);
      static void writeClusterToFile(const unordered_map<std::string, std::set<std::string>> &clusterNodes, ofstream &cfile);
  };

  class PDGPrinter {
    public:
      PDGPrinter();

      void printPDG (
        Module &module, 
        CallGraph &callGraph, 
        PDG *graph,
        std::function<LoopInfo& (Function *f)> getLoopInfo
        );

      void printGraphsForFunction(Function &F, PDG *graph, LoopInfo &LI);

    private:

      void collectAllFunctionsInCallGraph (
        Module &M, 
        CallGraph &callGraph,
        std::set<Function *> &funcSet
        );
  };

}
