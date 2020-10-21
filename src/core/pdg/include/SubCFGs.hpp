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

using namespace std;
using namespace llvm;
using namespace llvm::noelle;

namespace llvm::noelle {

  /*
   * Execution Graph.
   */
  class SubCFGs : public DG<BasicBlock> {
    public:

      /*
       * Add all basic blocks included in the module M as nodes.
       */
      SubCFGs (Module &M) ;

      /*
       * Add all basic blocks included in the function F as nodes.
       */
      SubCFGs (Function &F) ;

      /*
       * Add all basic blocks included in the loop only.
       */
      SubCFGs (Loop *loop) ;

      /*
       * Add all basic blocks given
       */
      SubCFGs (std::set<BasicBlock *> &bbs) ;

    private:
      void connectBasicBlockNodes() ;
  };

}
