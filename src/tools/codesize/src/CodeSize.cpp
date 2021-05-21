/*
 * Copyright 2021 Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "CodeSize.hpp"
#include "Noelle.hpp"

namespace llvm::noelle {

CodeSize::CodeSize() 
  : ModulePass{ID}
{
}

bool CodeSize::runOnModule (Module &M) {

  /*
   * Fetch NOELLE
   */
  auto& noelle = getAnalysis<Noelle>();

  /*
   * Compute the code size.
   */
  uint64_t s = 0;
  for (auto &F : M){
    for (auto &bb : F){
      for (auto &I : bb){

        /*
         * Check the current instruction
         */
        if (isa<PHINode>(&I)){
          continue ;
        }
        if (auto brInst = dyn_cast<BranchInst>(&I)){
          if (brInst->isUnconditional()){
            continue ;
          }
        }

        s++;
      }
    }
  }
  outs() << s << "\n";

  return false; 
}

}
