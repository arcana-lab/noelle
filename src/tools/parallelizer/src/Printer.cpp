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
#include "llvm/Support/raw_ostream.h"

#include "SCCDAG.hpp"
#include "Parallelizer.hpp"

namespace llvm::noelle {
  void Parallelizer::printLoop (Loop *loop)
  {
    errs() << "Parallelizing the following loop\n" ;
    auto header = loop->getHeader();
    errs() << "Number of bbs: " << std::distance(loop->block_begin(), loop->block_end()) << "\n";
    for (auto bbi = loop->block_begin(); bbi != loop->block_end(); ++bbi){
      auto bb = *bbi;
      if (header == bb) {
        errs() << "Header:\n";
      } else if (loop->isLoopLatch(bb)) {
        errs() << "Loop latch:\n";
      } else if (loop->isLoopExiting(bb)) {
        errs() << "Loop exiting:\n";
      } else {
        errs() << "Loop body:\n";
      }
      for (auto &I : *bb) {
        I.print(errs());
        errs() << "\n";
      }
    }
  }
}