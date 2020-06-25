/*
 * Copyright 2016 - 2020  Angelo Matni, Yian Su, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "SystemHeaders.hpp"

#include "Util/SVFModule.h"
#include "WPA/Andersen.h"
#include "TalkDown.hpp"
#include "PDGPrinter.hpp"
#include "PDGAnalysis.hpp"

using namespace llvm;

void PDGAnalysis::constructEdgesFromControl (PDG *pdg, Module &M){
  assert(pdg != nullptr);

  for (auto &F : M) {

    /*
     * Fetch the next function with a body.
     */
    if (F.empty()) {
      continue ;
    }

    /*
     * Fetch the post-dominator tree of the function.
     */
    auto &postDomTree = getAnalysis<PostDominatorTreeWrapperPass>(F).getPostDomTree();

    /*
     * Compute the control dependences of the function based on its post-dominator tree.
     */
    this->constructEdgesFromControlForFunction(pdg, F, postDomTree);
  }

  return ;
}

void PDGAnalysis::constructEdgesFromControlForFunction (PDG *pdg, Function &F, PostDominatorTree &postDomTree) {
  assert(pdg != nullptr);

  for (auto &B : F) {

    /*
     * Fetch the basic blocks post-dominated by the current one.
     */
    SmallVector<BasicBlock *, 10> dominatedBBs;
    postDomTree.getDescendants(&B, dominatedBBs);

    /*
     * For each basic block that B post dominates, check if B doesn't stricly post dominate its predecessor
     * If it does not, then there is a control dependency from the predecessor to B 
     */
    for (auto dominatedBB : dominatedBBs) {
      for (auto predBB : make_range(pred_begin(dominatedBB), pred_end(dominatedBB))) {

        /*
         * Fetch the terminator of the predecessor.
         */
        auto controlTerminator = predBB->getTerminator();

        /*
         * Check if the predecessor terminator is a conditional branch.
         * This is necessary to avoid adding incorrect control dependences between basic blocks of a loop that has no exit blocks.
         * For example:
         *
         * predBB:
         *  branch B
         *
         * B: 
         *  i
         *  branch %B
         *
         * In this case, if we don't check that the terminator of predBB is a conditional branch, we would add a control dependence from branch %B to i
         */
        if (controlTerminator->getNumSuccessors() == 1){
          continue ;
        }

        /*
         * Check if B strictly post-dominates predBB.
         */
        if (postDomTree.properlyDominates(&B, predBB)) {
          continue;
        }

        /*
         * Add the control dependences.
         */
        for (auto &I : B) {
          auto edge = pdg->addEdge((Value*)controlTerminator, (Value*)&I);
          edge->setControl(true);
        }
      }
    }
  }

  return ;
}
