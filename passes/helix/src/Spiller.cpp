/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "HELIX.hpp"
#include "HELIXTask.hpp"
#include <set>

using namespace llvm ;

void HELIX::spillLoopCarriedDataDependencies (LoopDependenceInfo *LDI) {
  auto helixTask = static_cast<HELIXTask *>(this->tasks[0]);

  /*
   * Collect all PHIs in the loop header; they are local variables
   * with loop carried data dependencies and need to be spilled
   */
  std::vector<PHINode *> originalLoopPHIs;
  for (auto &phi : LDI->header->phis()) {
    originalLoopPHIs.push_back(&phi);
    auto clonePHI = (PHINode *)(helixTask->instructionClones[&phi]);
    this->loopCarriedPHIs.push_back(clonePHI);
  }
  assert(this->loopCarriedPHIs.size() > 0
    && "There should be loop carried data dependencies for a HELIX loop");

  /*
   * Register each PHI as part of the loop carried environment
   */
  std::vector<Type *> phiTypes;
  std::set<int> nonReducablePHIs;
  std::set<int> cannotReduceLoopCarriedPHIs;
  for (auto i = 0; i < this->loopCarriedPHIs.size(); ++i) {
    phiTypes.push_back(this->loopCarriedPHIs[i]->getType());
    nonReducablePHIs.insert(i);
  }

  /*
   * Instantiate a builder to the task's entry, track the terminator,
   * and later hoist the terminator back to the end of the entry block.
   */
  auto entryBlockTerminator = helixTask->entryBlock->getTerminator();
  IRBuilder<> entryBuilder(helixTask->entryBlock);

  /*
   * Register a new environment builder and the single HELIX task
   */
  this->loopCarriedEnvBuilder = new EnvBuilder(module.getContext());
  this->loopCarriedEnvBuilder->createEnvVariables(phiTypes, nonReducablePHIs, cannotReduceLoopCarriedPHIs, 1);
  this->loopCarriedEnvBuilder->createEnvUsers(1);
  auto envUser = loopCarriedEnvBuilder->getUser(0);

  envUser->setEnvArray(entryBuilder.CreateBitCast(
    helixTask->loopCarriedArrayArg,
    PointerType::getUnqual(loopCarriedEnvBuilder->getEnvArrayTy())
  ));

  /*
   * Allocate the environment array (64 byte aligned)
   * Load incoming values from the preheader
   */
  IRBuilder<> loopFunctionBuilder(&*LDI->function->begin()->begin());
  loopCarriedEnvBuilder->generateEnvArray(loopFunctionBuilder);
  loopCarriedEnvBuilder->generateEnvVariables(loopFunctionBuilder);

  IRBuilder<> builder(this->entryPointOfParallelizedLoop);
  for (auto envIndex = 0; envIndex < originalLoopPHIs.size(); ++envIndex) {
    auto phi = originalLoopPHIs[envIndex];
    auto preHeaderIndex = phi->getBasicBlockIndex(LDI->preHeader);
    auto preHeaderV = phi->getIncomingValue(preHeaderIndex);
    builder.CreateStore(preHeaderV, loopCarriedEnvBuilder->getEnvVar(envIndex));
  }

  /*
   * Generate code to store each incoming loop carried PHI value,
   * load the incoming value, and replace PHI uses with load uses
   * For the pre header edge case, store this initial value at time of
   * allocation of the environment
   */
  auto preHeaderClone = helixTask->basicBlockClones[LDI->preHeader];
  auto firstNonPHI = helixTask->instructionClones[LDI->header->getFirstNonPHI()];
  IRBuilder<> headerBuilder(firstNonPHI);
  for (auto phiI = 0; phiI < loopCarriedPHIs.size(); phiI++) {
    auto phi = loopCarriedPHIs[phiI];

    /*
     * Create GEP access of the environment variable at index i
     */
    envUser->createEnvPtr(entryBuilder, phiI, phiTypes[phiI]);
    auto envPtr = envUser->getEnvPtr(phiI);

    /*
     * Store loop carried values of the PHI into the environment
     */
    for (auto inInd = 0; inInd < phi->getNumIncomingValues(); ++inInd) {
      auto incomingBB = phi->getIncomingBlock(inInd);
      if (incomingBB == preHeaderClone) continue;

      /*
       * Determine the position of the incoming value's producer
       * If it isn't an instruction, insert at the incoming block's entry
       */
      auto incomingV = phi->getIncomingValue(inInd);
      Instruction *insertPoint = &*incomingBB->begin();
      if (auto incomingI = dyn_cast<Instruction>(incomingV)) {
        insertPoint = incomingI->getNextNode();
      }

      IRBuilder<> builder(insertPoint);
      builder.CreateStore(incomingV, envPtr);
    }

    /*
     * Replace uses of PHI with environment load
     */
    auto envLoad = headerBuilder.CreateLoad(envPtr);
    std::set<User *> phiUsers(phi->user_begin(), phi->user_end());
    for (auto user : phiUsers) {
      user->replaceUsesOfWith(phi, envLoad);
    }
    phi->eraseFromParent();
  }

  entryBlockTerminator->removeFromParent();
  entryBuilder.Insert(entryBlockTerminator);
}
