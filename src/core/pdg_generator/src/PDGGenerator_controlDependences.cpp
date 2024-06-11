/*
 * Copyright 2016 - 2020  Angelo Matni, Yian Su, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "arcana/noelle/core/SystemHeaders.hpp"
#include "arcana/noelle/core/TalkDown.hpp"
#include "arcana/noelle/core/PDGPrinter.hpp"
#include "arcana/noelle/core/PDGGenerator.hpp"
#include "llvm/Analysis/PostDominators.h"

namespace arcana::noelle {

void PDGGenerator::constructEdgesFromControl(PDG *pdg, Module &M) {
  assert(pdg != nullptr);

  for (auto &F : M) {

    /*
     * Fetch the next function with a body.
     */
    if (F.empty()) {
      continue;
    }

    /*
     * Compute the control dependences of the function based on its
     * post-dominator tree.
     */
    this->constructEdgesFromControlForFunction(pdg, F);
  }

  return;
}

void PDGGenerator::constructEdgesFromControlForFunction(PDG *pdg, Function &F) {
  assert(pdg != nullptr);

  /*
   * There is a control dependence from a basic block A to a basic block B iff
   * 1) there is E such that E is a successor of A, and
   * 2) B post-dominates E, and
   * 3) B doesn't strictly post-dominate A
   */

  /*
   * Fetch the post-dominator tree of the function.
   */
  auto &postDomTree =
      getAnalysis<PostDominatorTreeWrapperPass>(F).getPostDomTree();

  for (auto &B : F) {

    /*
     * Fetch the basic blocks post-dominated by the current one.
     */
    SmallVector<BasicBlock *, 10> dominatedBBs;
    postDomTree.getDescendants(&B, dominatedBBs);

    /*
     * For each basic block that B post dominates, check if B doesn't stricly
     * post dominate its predecessor If it does not, then there is a control
     * dependency from the predecessor to B
     */
    for (auto dominatedBB : dominatedBBs) {
      for (auto predBB :
           make_range(pred_begin(dominatedBB), pred_end(dominatedBB))) {

        /*
         * Fetch the terminator of the predecessor.
         */
        auto controlTerminator = predBB->getTerminator();

        /*
         * Check if the predecessor terminator is a conditional branch.
         * This is necessary to avoid adding incorrect control dependences
         * between basic blocks of a loop that has no exit blocks. For example:
         *
         * predBB:
         *  branch B
         *
         * B:
         *  i
         *  branch %B
         *
         * In this case, if we don't check that the terminator of predBB is a
         * conditional branch, we would add a control dependence from branch %B
         * to i
         */
        if (controlTerminator->getNumSuccessors() == 1) {
          continue;
        }

        /*
         * Check if B strictly post-dominates predBB.
         */
        if (postDomTree.properlyDominates(&B, predBB)) {

          /*
           * B strictly post-dominates predBB.
           * Therefore, there is no control dependence from predBB to B
           */
          continue;
        }

        /*
         * There is a control dependence from predBB to B
         *
         * Add the control dependences.
         */
        for (auto &I : B) {
          auto srcNode = pdg->fetchNode(controlTerminator);
          auto dstNode = pdg->fetchNode(&I);
          assert(srcNode != nullptr);
          assert(dstNode != nullptr);
          ControlDependence<Value, Value> edge{ srcNode, dstNode };
          pdg->copyAddEdge(edge);
        }
      }
    }
  }

  auto getControlProducers = [&](Value *V) -> std::unordered_set<Value *> {
    std::unordered_set<Value *> controlProducers;
    auto node = pdg->fetchNode(V);
    for (auto edge : node->getIncomingEdges()) {
      if (!isa<ControlDependence<Value, Value>>(edge))
        continue;
      auto controlProducer = edge->getSrc();
      controlProducers.insert(controlProducer);
    }
    return controlProducers;
  };

  /*
   * For PHI nodes with incoming values that do not reside in their respective
   * incoming block, add control edges on the incoming block's terminator to the
   * PHI
   */
  for (auto &B : F) {
    for (auto &phi : B.phis()) {

      /*
       * Locate control producers of incoming blocks to PHIs
       * where the incoming value doesn't reside in incoming block
       */
      std::unordered_set<Value *> controlProducers;
      for (auto i = 0u; i < phi.getNumIncomingValues(); ++i) {
        auto incomingValue = phi.getIncomingValue(i);
        if (!incomingValue)
          continue;

        auto incomingInst = dyn_cast<Instruction>(incomingValue);
        auto incomingBlock = phi.getIncomingBlock(i);
        if (incomingInst && incomingInst->getParent() == incomingBlock)
          continue;

        auto terminator = incomingBlock->getTerminator();
        auto terminatorControlProducers = getControlProducers(terminator);
        controlProducers.insert(terminatorControlProducers.begin(),
                                terminatorControlProducers.end());
      }
      if (controlProducers.size() == 0)
        continue;

      /*
       * Determine which of these control producers do NOT have a control edge
       * to the PHI already Add a control edge from those producers to the PHI
       */
      std::unordered_set<Value *> currentControlProducersOnPHI =
          getControlProducers(&phi);
      for (auto producer : controlProducers) {
        if (currentControlProducersOnPHI.find(producer)
            != currentControlProducersOnPHI.end()) {
          continue;
        }

        auto srcNode = pdg->fetchNode(producer);
        auto dstNode = pdg->fetchNode(&phi);
        assert(srcNode != nullptr);
        assert(dstNode != nullptr);
        ControlDependence<Value, Value> edge{ srcNode, dstNode };
        pdg->copyAddEdge(edge);
      }
    }
  }

  return;
}

} // namespace arcana::noelle
