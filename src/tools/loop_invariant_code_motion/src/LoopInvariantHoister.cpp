/*
 * Copyright 2019 - 2022  Simone Campanoni
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
#include "arcana/noelle/tools/LoopInvariantCodeMotion.hpp"

namespace arcana::noelle {

bool LoopInvariantCodeMotion::hoistInvariantValues(LoopContent const &LC) {
  auto modified = false;
  errs() << "LICM: Start\n";

  /*
   * Fetch the information.
   */
  auto invariantManager = LC.getInvariantManager();
  auto loopStructure = LC.getLoopStructure();
  auto header = loopStructure->getHeader();
  auto preHeader = loopStructure->getPreHeader();
  auto loopFunction = header->getParent();
  errs() << "LICM:   Loop \"" << *header->getFirstNonPHI() << "\"\n";

  /*
   * Compute the dominators.
   */
  auto DS = this->noelle.getDominators(loopFunction);

  /*
   * Identify the instructions to hoist outside the loop.
   */
  std::vector<Instruction *> instructionsToHoistToPreheader{};
  std::map<Instruction *, std::set<Instruction *>> conditionalHoisting{};
  std::unordered_set<PHINode *> phisToRemove{};
  for (auto B : loopStructure->getBasicBlocks()) {
    for (auto &I : *B) {

      /*
       * Check if the current instruction is a loop invariant.
       */
      if (!invariantManager->isLoopInvariant(&I)) {
        continue;
      }
      errs() << "LICM:     Invariant = \"" << I << "\n";

      /*
       * Check if the instruction can generate unwanted side-effects if there is
       * no guarantee it will execute at least once per loop invocation.
       *
       * Call instructions don't need to be checked because if they are
       * invariants, it means they must have no memory data dependences between
       * theirself. In other words, they cannot write to memory that can be
       * loaded outside the loop.
       */
      auto mayWriteToMemory = false;
      if (isa<StoreInst>(&I)) {
        mayWriteToMemory = true;
      }
      if (auto callInst = dyn_cast<CallBase>(&I)) {

        /*
         * Check if the callee is known
         */
        auto calleeFunction = callInst->getCalledFunction();
        if (calleeFunction == nullptr) {

          /*
           * The callee is unknown.
           * Hence, we must be conservative and assume that callee will write to
           * memory.
           * TODO: this can be improved by using the call graph of NOELLE to
           * check all possible callees.
           */
          mayWriteToMemory = true;

        } else {

          /*
           * The callee is known and there is only one possible callee.
           *
           * Check if the callee is a library function.
           */
          if (!calleeFunction->empty()) {

            /*
             * The callee is not a library function
             */
            mayWriteToMemory = true;

          } else {

            /*
             * The callee is a library function
             */
            if (!PDGGenerator::isTheLibraryFunctionPure(calleeFunction)) {
              mayWriteToMemory = true;
            }
          }
        }
      }
      if (mayWriteToMemory) {
        errs()
            << "LICM:       The instruction might generate unwanted side-effects if it does not execute at least once per loop invocation\n";
        continue;
      }

      /*
       * The current instruction is a loop invariant.
       *
       * Check all instructions that directly-or-indirectly have a data
       * dependence to @I. In other words, check all sources of all data
       * dependence that have @I as destination. All of these instructions must
       * be invariant for @I to be hoisted.
       */
      auto dependenceInstructions =
          this->getSourceDependenceInstructionsFrom(LC, I);
      auto isSafe = true;
      errs() << "LICM:       Checking dependences\n";
      std::set<Instruction *> dependentInvariantsInLoop{};
      for (auto depI : dependenceInstructions) {
        errs() << "LICM:         Dependent instruction = \"" << *depI << "\n";

        /*
         * We can skip instructions that are outside the target loop.
         */
        if (!loopStructure->isIncluded(depI)) {
          continue;
        }

        /*
         * If @depI isn't invariant, than we cannot hoist @I.
         */
        if (!invariantManager->isLoopInvariant(depI)) {
          isSafe = false;
          break;
        }

        /*
         * Keep track of the other invariants that have to be hoisted.
         */
        dependentInvariantsInLoop.insert(depI);
      }
      if (!isSafe) {
        continue;
      }
      if (dependentInvariantsInLoop.size() == 0) {
        errs() << "LICM:       The instruction can be hoisted\n";

      } else {
        errs()
            << "LICM:       The instruction is conditionally hoisted if the next invariants are hoisted as well:\n";
        for (auto depI : dependentInvariantsInLoop) {
          errs() << "LICM:       " << *depI << "\n";
        }
      }

      /*
       * The instruction @I is invariant and it is safe to hoist it.
       *
       * Handle non-phi instructions separately.
       */
      auto phi = dyn_cast<PHINode>(&I);
      if (!phi) {
        if (std::find(instructionsToHoistToPreheader.begin(),
                      instructionsToHoistToPreheader.end(),
                      &I)
            == instructionsToHoistToPreheader.end()) {
          if (dependentInvariantsInLoop.size() == 0) {
            instructionsToHoistToPreheader.push_back(&I);
          } else {
            conditionalHoisting[&I] = dependentInvariantsInLoop;
          }
        }
        continue;
      }

      /*
       * All PHI invariants are equivalent, but to ensure dominance of the
       * replacing value, choose the first incoming value that dominates the
       * PHI. If none exist, do not hoist the PHI
       */
      Value *valueToReplacePHI = nullptr;
      for (auto i = 0u; i < phi->getNumIncomingValues(); ++i) {
        auto incomingBlock = phi->getIncomingBlock(i);
        if (!DS->DT.dominates(incomingBlock, B)) {
          continue;
        }
        valueToReplacePHI = phi->getIncomingValue(i);
        break;
      }
      if (!valueToReplacePHI) {
        continue;
      }

      /*
       * Note, the users are modified, so we must cache them first
       */
      std::unordered_set<User *> users(phi->user_begin(), phi->user_end());
      for (auto user : users) {
        user->replaceUsesOfWith(phi, valueToReplacePHI);
        modified = true;
      }
      phisToRemove.insert(phi);

      /*
       * If the replacement is an Instruction and in the loop, it needs to be
       * hoisted
       */
      if (auto instToReplacePHI = dyn_cast<Instruction>(valueToReplacePHI)) {
        if (loopStructure->isIncluded(instToReplacePHI)) {
          if (std::find(instructionsToHoistToPreheader.begin(),
                        instructionsToHoistToPreheader.end(),
                        instToReplacePHI)
              == instructionsToHoistToPreheader.end()) {
            if (dependentInvariantsInLoop.size() == 0) {
              instructionsToHoistToPreheader.push_back(instToReplacePHI);
            } else {
              conditionalHoisting[instToReplacePHI] = dependentInvariantsInLoop;
            }
          }
        }
      }
    }
  }

  /*
   * Evaluate the invariants that can only conditionally be hoisted.
   */
  errs() << "LICM:     Check invariants that can be hoisted conditionally\n";
  auto conditionsAreModified = true;
  while (conditionsAreModified) {
    conditionsAreModified = false;

    /*
     * Check the invariants that can only conditionally be hoisted.
     */
    errs() << "LICM:       Check the invariants\n";
    for (auto p : conditionalHoisting) {

      /*
       * Fetch the invariant.
       */
      auto instToHoist = p.first;
      assert(loopStructure->isIncluded(instToHoist));

      /*
       * Check if the invariant is already tagged as safe to be hoisted.
       */
      if (std::find(instructionsToHoistToPreheader.begin(),
                    instructionsToHoistToPreheader.end(),
                    instToHoist)
          != instructionsToHoistToPreheader.end()) {
        continue;
      }

      /*
       * Check conditions
       */
      errs() << "LICM:         Invariant " << *instToHoist << "\n";
      auto areConditionsMet = true;
      auto &conditions = p.second;
      for (auto cond : conditions) {
        assert(loopStructure->isIncluded(cond));

        /*
         * Check if the current condition is matched.
         */
        if (std::find(instructionsToHoistToPreheader.begin(),
                      instructionsToHoistToPreheader.end(),
                      cond)
            != instructionsToHoistToPreheader.end()) {

          /*
           * The instruction that represents a condition to hoist @instToHoist
           * will be hoisted.
           */
          continue;
        }

        /*
         * Check if the current condition to hoist @instToHoist depends on
         * another condition.
         */
        if (conditionalHoisting.find(cond) == conditionalHoisting.end()) {

          /*
           * There is at least one other invariant that depends on the current
           * one that cannot be hoisted. Hence, the current one cannot be
           * hoisted.
           */
          areConditionsMet = false;
          break;
        }
      }

      if (!areConditionsMet) {

        /*
         * The conditions are not met.
         * We cannot hoist the current invariant.
         */
        errs() << "LICM:           It cannot be hoisted\n";

        /*
         * Remove the current invariant from the list of instructions that could
         * be hoisted.
         */
        assert(std::find(instructionsToHoistToPreheader.begin(),
                         instructionsToHoistToPreheader.end(),
                         instToHoist)
               == instructionsToHoistToPreheader.end());
        conditionalHoisting.erase(instToHoist);
        conditionsAreModified = true;

        /*
         * Remove all other invariants that their hoisting was conditioned to
         * this one.
         */
        std::set<Instruction *> toDelete{};
        for (auto p2 : conditionalHoisting) {
          auto removed = false;
          for (auto cond2 : p2.second) {
            if (cond2 != instToHoist) {
              continue;
            }
            removed = true;
            auto it = std::find(instructionsToHoistToPreheader.begin(),
                                instructionsToHoistToPreheader.end(),
                                p2.first);
            if (it != instructionsToHoistToPreheader.end()) {
              errs() << "LICM:             Remove " << *cond2
                     << " as well as it was conditioned to this one\n";
              instructionsToHoistToPreheader.erase(it);
            }
            break;
          }
          if (removed) {
            toDelete.insert(p2.first);
          }
        }
        for (auto inst : toDelete) {
          conditionalHoisting.erase(inst);
        }

        break;
      }

      /*
       * The conditions are met.
       * We can hoist the current invariant.
       */
      errs() << "LICM:           It can be hoisted\n";
      instructionsToHoistToPreheader.push_back(instToHoist);
      modified = true;
    }
  }

  /*
   * Remove the dead PHIs.
   */
  for (auto phi : phisToRemove) {
    phi->eraseFromParent();
    modified = true;
  }

  /*
   * Re-compute the dominators.
   */
  auto newDS = this->noelle.getDominators(loopFunction);

  /*
   * Sort invariants to hoist in order of dominance to preserve execution order
   */
  if (instructionsToHoistToPreheader.size() > 0) {
    auto converged = false;
    do {
      converged = true;
      for (auto i = 0u; i < (instructionsToHoistToPreheader.size() - 1); i++) {
        for (auto j = i + 1u; j < instructionsToHoistToPreheader.size(); j++) {
          auto I = instructionsToHoistToPreheader[i];
          auto J = instructionsToHoistToPreheader[j];
          assert(I != J);
          if (newDS->DT.dominates(J, I)) {
            instructionsToHoistToPreheader[i] = J;
            instructionsToHoistToPreheader[j] = I;
            converged = false;
          }
        }
      }
    } while (!converged);
  }

  /*
   * Hoist each instruction into the preheader
   */
  IRBuilder<> preHeaderBuilder(preHeader->getTerminator());
  for (auto I : instructionsToHoistToPreheader) {
    I->removeFromParent();
    preHeaderBuilder.Insert(I);
    modified = true;
  }
  if (modified) {
    errs() << "LICM:   The loop has been modified\n";
  } else {
    errs() << "LICM:   The loop has not been modified\n";
  }

  /*
   * Free the memory.
   */
  delete DS;
  delete newDS;

  errs() << "LICM: Exit\n";
  return modified;
}

std::vector<Instruction *> LoopInvariantCodeMotion::
    getSourceDependenceInstructionsFrom(LoopContent const &LC, Instruction &I) {
  std::vector<Instruction *> s;

  /*
   * Fetch the loop structure.
   */
  auto ls = LC.getLoopStructure();

  /*
   * Fetch the Loop DG.
   */
  auto ldg = LC.getLoopDG();

  /*
   * Code to collect dependences.
   */
  auto collectF = [ls, &s](Value *f, DGEdge<Value, Value> *d) -> bool {
    auto fI = dyn_cast<Instruction>(f);
    if (fI == nullptr) {
      return false;
    }
    if (!ls->isIncluded(fI)) {
      return false;
    }
    s.push_back(fI);
    return false;
  };
  ldg->iterateOverDependencesTo(&I, false, true, true, collectF);

  return s;
}

} // namespace arcana::noelle
