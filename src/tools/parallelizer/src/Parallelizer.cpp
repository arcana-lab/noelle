/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "Parallelizer.hpp"

namespace llvm::noelle {

    BasicBlock* Parallelizer::CreateSynchronization (Function *f, IRBuilder<> builder,
      BasicBlock* bbBeforeSync, BasicBlock* originalBBAfterSync, bool eraseTarget, Instruction* isSyncedAlloca, Instruction *numCoresAlloca, Instruction *memoryIdxAlloca) {

      //create a before sync BB
      auto beforeSyncBB = BasicBlock::Create(f->getContext(), "beforeSyncBB", f);

      auto bbTerminator = bbBeforeSync->getTerminator();
      if(!eraseTarget){
        if(BranchInst *br = dyn_cast<BranchInst>(bbTerminator)){
          if(!br->isConditional()) builder.CreateBr(beforeSyncBB);
          else{
            auto cond = br->getCondition();
            auto succ0 = br->getSuccessor(0);
            auto succ1 = br->getSuccessor(1);
            Instruction* newBr = nullptr;
            if(succ0 == originalBBAfterSync)
              newBr = builder.CreateCondBr(cond, beforeSyncBB, succ1);
            else if(succ1 == originalBBAfterSync)
              newBr = builder.CreateCondBr(cond, succ0, beforeSyncBB);

            assert(newBr && "synchronization not linked properly\n");
          }
        }
        else if(SwitchInst *sw = dyn_cast<SwitchInst>(bbTerminator)){
          for (SwitchInst::CaseIt i = sw->case_begin(), e = sw->case_end(); i != e; ++i){
            ConstantInt *CaseVal = i->getCaseValue();
            BasicBlock *succ = i->getCaseSuccessor();
            if(succ == originalBBAfterSync){
              sw->removeCase(i);
              sw->addCase(CaseVal, beforeSyncBB);
            }
          }
        }
     } else builder.CreateBr(beforeSyncBB);


     if (bbTerminator != nullptr && isa<BranchInst>(bbTerminator)){
      bbTerminator->eraseFromParent();
     }

     //create check for whether synced or not yet
     IRBuilder<> beforeSyncBuilder{beforeSyncBB};
     auto int1Ty = IntegerType::get(beforeSyncBuilder.getContext(), 1);
     auto constantOne = ConstantInt::get(int1Ty, 1);
     auto loadedSyncBit = beforeSyncBuilder.CreateLoad(int1Ty, isSyncedAlloca);
     auto cmpSync = beforeSyncBuilder.CreateICmpEQ(loadedSyncBit, constantOne);

     //create a sync BB
     auto syncBB = BasicBlock::Create(f->getContext(), "SyncBB", f);

     //create a BB after syncBB
     auto afterSyncBB = BasicBlock::Create(f->getContext(), "afterSyncBB", f, syncBB);

     //create branch based on whether synced or not
     beforeSyncBuilder.CreateCondBr(cmpSync, afterSyncBB, syncBB);

     //syncBB: call SyncFunction in syncBB
     IRBuilder<> syncBBBuilder{syncBB};
     auto int32Ty = IntegerType::get(syncBBBuilder.getContext(), 32);
     auto int64Ty = IntegerType::get(syncBBBuilder.getContext(), 64);
     auto numThreadsUsed = syncBBBuilder.CreateLoad(int32Ty, numCoresAlloca);
     auto memoryIndex = syncBBBuilder.CreateLoad(int64Ty, memoryIdxAlloca);
     syncBBBuilder.CreateCall(SyncFunction, ArrayRef<Value *>({numThreadsUsed, memoryIndex}));

     //syncBB: store 1 to isSyncedSignal
     int1Ty = IntegerType::get(syncBBBuilder.getContext(), 1);
     syncBBBuilder.CreateStore(ConstantInt::get(int1Ty,1), isSyncedAlloca);

     //link syncBB to afterSyncBB
     syncBBBuilder.CreateBr(afterSyncBB);

     return afterSyncBB;
  }

  void Parallelizer::InsertSyncFunctionBefore(BasicBlock* currBB, ParallelizationTechnique *usedTechnique, Function* f, std::set<std::pair<BasicBlock*, BasicBlock*>> &addedSyncEdges){
        std::set<BasicBlock*> predBB2Remove;
        for (pred_iterator PI = pred_begin(currBB), E = pred_end(currBB); PI != E; ++PI)
        {
          BasicBlock *predBB = *PI;
          if(predBB->getParent() != f) continue;
          bool alreadyInserted = false;
          for(auto edge : addedSyncEdges)
            if(edge.first == predBB && edge.second == currBB){
              alreadyInserted = true;
              break;
            }
          if(alreadyInserted) continue;
          addedSyncEdges.insert(std::make_pair(currBB,predBB));
          auto builder = new IRBuilder<>(predBB);
          auto afterSyncBB = CreateSynchronization(f, *builder, predBB, currBB, 0, isSyncedAlloca[usedTechnique], numCoresAlloca[usedTechnique], memoryIdxAlloca[usedTechnique]);
          delete builder;
          //link afterSyncBB to dispatcherBB
          IRBuilder <>afterSyncBuilder(afterSyncBB);
          afterSyncBuilder.CreateBr(currBB);

          //adjust phis
          for(auto &I : *currBB){
            PHINode* phi = dyn_cast<PHINode>(&I);
            if(!phi) break;
            phi->replaceIncomingBlockWith(predBB, afterSyncBB);
          }
        }


  }

  bool Parallelizer::parallelizeLoop (
      LoopDependenceInfo *LDI,
      Noelle &par,
      Heuristics *h
      ){
    auto prefix = "Parallelizer: parallelizerLoop: " ;

    /*
     * Assertions.
     */
    assert(LDI != nullptr);
    assert(h != nullptr);

    /*
     * Allocate the parallelization techniques.
     */
    DSWP dswp{
      par,
      this->forceParallelization,
      !this->forceNoSCCPartition
    };
    auto doall = new DOALL(par);
    //DOALL doall{
    //  par
    //};
    HELIX helix{
      par,
      this->forceParallelization
    };

    /*
     * Fetch the verbosity level.
     */
    auto verbose = par.getVerbosity();

    /*
     * Fetch the loop headers.
     */
    auto loopStructure = LDI->getLoopStructure();
    auto loopHeader = loopStructure->getHeader();
    auto loopPreHeader = loopStructure->getPreHeader();

    /*
     * Fetch the loop function.
     */
    auto loopFunction = loopStructure->getFunction();
    assert(par.verifyCode());

    /*
     * Print
     */
    if (verbose != Verbosity::Disabled) {
      errs() << prefix << "Start\n";
      errs() << prefix << "  Function = \"" << loopFunction->getName() << "\"\n";
      errs() << prefix << "  Loop " << LDI->getID() << " = \"" << *loopHeader->getFirstNonPHI() << "\"\n";
      errs() << prefix << "  Nesting level = " << loopStructure->getNestingLevel() << "\n";
      errs() << prefix << "  Number of threads to extract = " << LDI->getMaximumNumberOfCores() << "\n";
    }

    /*
     * Parallelize the loop.
     */
    auto codeModified = false;
    ParallelizationTechnique *usedTechnique = nullptr;
    if (  true
        && par.isTransformationEnabled(DOALL_ID)
        && LDI->isTransformationEnabled(DOALL_ID)
        && doall->canBeAppliedToLoop(LDI, h)
       ){

      /*
       * Apply DOALL.
       */
      codeModified = doall->apply(LDI, h);
      usedTechnique = doall;

    } else if ( true
        && par.isTransformationEnabled(HELIX_ID)
        && LDI->isTransformationEnabled(HELIX_ID)
        && helix.canBeAppliedToLoop(LDI, h)
        ){

      /*
       * Apply HELIX
       */
      codeModified = helix.apply(LDI, h);

      auto function = helix.getTaskFunction();
      auto &LI = getAnalysis<LoopInfoWrapperPass>(*function).getLoopInfo();
      auto& PDT = getAnalysis<PostDominatorTreeWrapperPass>(*function).getPostDomTree();
      auto& SE = getAnalysis<ScalarEvolutionWrapperPass>(*function).getSE();

      if (par.getVerbosity() >= Verbosity::Maximal) {
        errs() << "HELIX:  Constructing task dependence graph\n";
      }

      auto taskFunctionDG = helix.constructTaskInternalDependenceGraphFromOriginalLoopDG(LDI, PDT);

      if (par.getVerbosity() >= Verbosity::Maximal) {
        errs() << "HELIX:  Constructing task loop dependence info\n";
      }

      auto DS = par.getDominators(function);
      auto l = LI.getLoopsInPreorder()[0];
      auto newLDI = new LoopDependenceInfo(taskFunctionDG, l, *DS, SE, par.getCompilationOptionsManager()->getMaximumNumberOfCores(), par.canFloatsBeConsideredRealNumbers());
      newLDI->copyParallelizationOptionsFrom(LDI);

      codeModified = helix.apply(newLDI, h);
      usedTechnique = &helix;

    } else if ( true
        && par.isTransformationEnabled(DSWP_ID)
        && LDI->isTransformationEnabled(DSWP_ID)
        && dswp.canBeAppliedToLoop(LDI, h)
        ) {

      /*
       * Apply DSWP.
       */
      codeModified = dswp.apply(LDI, h);
      usedTechnique = &dswp;
    }

    /*
     * Check if the loop has been parallelized.
     */
    if (!codeModified){
      errs() << prefix << "  The loop has not been parallelized\n";
      errs() << prefix << "Exit\n";
      return false;
    }

    /*
     * Fetch the environment array where the exit block ID has been stored.
     */
    assert(usedTechnique != nullptr);
    auto envArray = usedTechnique->getEnvArray();
    assert(envArray != nullptr);

    /*
     * Fetch entry and exit point executed by the parallelized loop.
     */
    auto entryPoint = usedTechnique->getParLoopEntryPoint();
    auto exitPoint = usedTechnique->getParLoopExitPoint();
    assert(entryPoint != nullptr && exitPoint != nullptr);

    /*
     * The loop has been parallelized.
     *
     * Link the parallelized loop within the original function that includes the sequential loop.
     */
    if (verbose != Verbosity::Disabled) {
      errs() << prefix << "  Link the parallelize loop\n";
    }
    auto exitIndex = ConstantInt::get(par.int64, LDI->environment->indexOfExitBlockTaken());
    auto loopExitBlocks = loopStructure->getLoopExitBasicBlocks();
    par.linkTransformedLoopToOriginalFunction(
        loopFunction->getParent(),
        loopPreHeader,
        entryPoint,
        exitPoint,
        envArray,
        exitIndex,
        loopExitBlocks
        );
    assert(par.verifyCode());
    // if (verbose >= Verbosity::Maximal) {
    //   loopFunction->print(errs() << "Final printout:\n"); errs() << "\n";
    // }



    //NOTE:> not tested in performance tests
    if(usedTechnique == doall){
      techniques.insert(usedTechnique);
      std::set<BasicBlock*> insertedBlocks;
      Value* threadsUsed = usedTechnique->getNumOfThreads();
      Value* memoryIndex = usedTechnique->getMemoryIndex();
      auto dispatcherInst = usedTechnique->getDispatcherInst();
      auto dispatcherBB = dispatcherInst->getParent();
      Function *f = dispatcherBB->getParent();

      /*
       * Synchronization: Insert sync function before live-out
       */
      for(auto liveoutUse : usedTechnique->getLiveOutUses()){
        Instruction* liveoutInst = dyn_cast<Instruction>(liveoutUse);
        if(!liveoutInst) continue;
        auto liveoutBB = liveoutInst->getParent();
        BasicBlock *newInsertPt = liveoutBB;
        if(insertedBlocks.find(newInsertPt) != insertedBlocks.end())
          continue;
        insertedBlocks.insert(newInsertPt);
        errs() << "SUSAN: inserting at live-out Deps: " << *newInsertPt << "\n";
        insertingPts.push_back(std::make_pair(newInsertPt, usedTechnique));
      }

      /*
       * Synchronization: add sync function before mem dependences.
       * If a bb has multiple inserting points, insert at the earliest one
       */
      std::set<Value*> externalDeps = LDI->environment->getExternalDeps();
      for(auto insertPt : externalDeps){
        Instruction *depInst = dyn_cast<Instruction>(insertPt);
        errs() << "SUSAN: mem dep: " << *depInst << "\n";
        BasicBlock *newInsertPt = depInst->getParent();
        if(insertedBlocks.find(newInsertPt) != insertedBlocks.end())
          continue;
        errs() << "SUSAN: inserting at mem Deps: " << *newInsertPt << "\n";
        insertedBlocks.insert(newInsertPt);
        insertingPts.push_back(std::make_pair(newInsertPt, usedTechnique));
      }

      /*
       * Synchronization: add sync function before dispatcher
       */
      errs() << "SUSAN: inserting at dispatch: " << *dispatcherBB << "\n";
      if(insertedBlocks.find(dispatcherBB) == insertedBlocks.end()){
        insertedBlocks.insert(dispatcherBB);
        insertingPts.push_back(std::make_pair(dispatcherBB, usedTechnique));
      }

      /*
       * Synchronization: add sync function before exiting functions only for last
       * parallel region
       */
      //for(auto ldi : treesToParallelize.back()){
      //  if(LDI->getLoopStructure() == ldi->getLoopStructure())
          for(auto &BB : *f)
            for(auto &I : BB)
              if(isa<ReturnInst>(&I)){
                 BasicBlock *newInsertPt = &BB;
                 if(insertedBlocks.find(newInsertPt) != insertedBlocks.end())
                  continue;
                 insertedBlocks.insert(newInsertPt);
                 errs() << "SUSAN: inserting at exit: " << *newInsertPt << "\n";
                 insertingPts.push_back(std::make_pair(newInsertPt, usedTechnique));
              }
    //  }
    } //end of adding sync function for doall

    if (verbose != Verbosity::Disabled) {
      errs() << prefix << "  The loop has been parallelized\n";
      errs() << prefix << "Exit\n";
    }

    return true;
  }
}
