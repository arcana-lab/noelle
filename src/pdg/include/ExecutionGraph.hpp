/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/CFG.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/ADT/iterator_range.h"
#include <set>

#include "DGBase.hpp"
#include "DGGraphTraits.hpp"

using namespace std;
using namespace llvm;

namespace llvm {

  /*
   * Execution Graph.
   */
  class ExecutionGraph : public DG<BasicBlock> {
    public:

      /*
       * Add all basic blocks included in the module M as nodes.
       */
      ExecutionGraph (Module &M) ;

      /*
       * Add all basic blocks included in the function F as nodes.
       */
      ExecutionGraph (Function &F) ;

      /*
       * Add all basic blocks included in the loop only.
       */
      ExecutionGraph (Loop *loop) ;

    private:
      void connectBasicBlockNodes() ;
  };

  template<>
  struct DOTGraphTraits<ExecutionGraph *> : DGDOTGraphTraits<ExecutionGraph, BasicBlock> {
    DOTGraphTraits (bool isSimple=false) : DGDOTGraphTraits<ExecutionGraph, BasicBlock>(isSimple) {}

    static std::string getGraphName(ExecutionGraph *dg) {
      return "Execution Graph";
    }
  };

  template<> struct GraphTraits<ExecutionGraph *> : DGGraphTraits<ExecutionGraph, BasicBlock> {};
}
