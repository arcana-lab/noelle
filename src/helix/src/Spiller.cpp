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

  /*
   * Fetch the task
   */
  auto helixTask = static_cast<HELIXTask *>(this->tasks[0]);

  /*
   * Fetch the header.
   */
  auto loopSummary = LDI->getLoopSummary();
  auto loopHeader = loopSummary->getHeader();
  auto loopPreHeader = loopSummary->getPreHeader();

  /*
   * Collect all PHIs in the loop header; they are local variables
   * with loop carried data dependencies and need to be spilled
   */
  std::vector<PHINode *> originalLoopCarriedPHIs;
  std::vector<PHINode *> clonedLoopCarriedPHIs;
  for (auto &phi : loopHeader->phis()) {
    auto phiSCC = LDI->sccdagAttrs.getSCCDAG()->sccOfValue(cast<Value>(&phi));
    if (LDI->sccdagAttrs.getSCCAttrs(phiSCC)->canExecuteReducibly()) continue;
    originalLoopCarriedPHIs.push_back(&phi);
    auto clonePHI = (PHINode *)(helixTask->instructionClones[&phi]);
    clonedLoopCarriedPHIs.push_back(clonePHI);
  }
  assert(clonedLoopCarriedPHIs.size() > 0
    && "There should be loop carried data dependencies for a HELIX loop");

  /*
   * Register each PHI as part of the loop carried environment
   */
  std::vector<Type *> phiTypes;
  std::set<int> nonReducablePHIs;
  std::set<int> cannotReduceLoopCarriedPHIs;
  for (auto i = 0; i < clonedLoopCarriedPHIs.size(); ++i) {
    auto phiType = clonedLoopCarriedPHIs[i]->getType();
    phiTypes.push_back(phiType);
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

  /*
   * Fetch the unique user of the environment builder dedicated to spilled variables.
   */
  auto envUser = this->loopCarriedEnvBuilder->getUser(0);

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
  for (auto envIndex = 0; envIndex < originalLoopCarriedPHIs.size(); ++envIndex) {
    auto phi = originalLoopCarriedPHIs[envIndex];
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
  auto firstNonPHI = helixTask->instructionClones[loopHeader->getFirstNonPHI()];
  IRBuilder<> headerBuilder(firstNonPHI);
  for (auto phiI = 0; phiI < clonedLoopCarriedPHIs.size(); phiI++) {
    auto phi = clonedLoopCarriedPHIs[phiI];
    auto spilled = new SpilledLoopCarriedDependency();
    this->spills.insert(spilled);
    spilled->loopCarriedPHI = phi;

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
      Instruction *insertPoint = incomingBB->getFirstNonPHIOrDbgOrLifetime();
      if (auto incomingI = dyn_cast<Instruction>(incomingV)) {
        if (!isa<PHINode>(incomingI) && !isa<DbgInfoIntrinsic>(incomingI) && !incomingI->isLifetimeStartOrEnd()) {
          insertPoint = incomingI->getNextNode();
        }
      }

      IRBuilder<> builder(insertPoint);
      spilled->environmentStores.insert(builder.CreateStore(incomingV, envPtr));
    }

    /*
     * Replace uses of PHI with environment load
     */
    spilled->environmentLoad = headerBuilder.CreateLoad(envPtr);
    std::set<User *> phiUsers(phi->user_begin(), phi->user_end());
    for (auto user : phiUsers) {
      user->replaceUsesOfWith(phi, spilled->environmentLoad);
    }
    phi->eraseFromParent();
  }

  /*
   * Erase record of spilled PHIs
   */
  for (auto phi : originalLoopCarriedPHIs) {
    helixTask->instructionClones.erase(phi);
  }

  entryBlockTerminator->removeFromParent();
  entryBuilder.Insert(entryBlockTerminator);

  return ;
}
