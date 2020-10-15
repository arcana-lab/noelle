/*
 * Copyright 2019 - 2020  Souradip Ghosh, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "LoopWhilify.hpp"

using namespace llvm;
using namespace llvm::noelle;


LoopWhilifier::LoopWhilifier(Noelle &noelle)
  : noelle{noelle}
  {
  return ;
}

#define NOELLE_TEST_WHILIFIER 1
#define MAX_HANDLED_WHILIFIER 0

bool LoopWhilifier::whilifyLoop (
  LoopDependenceInfo &LDI
) {

  bool AnyTransformed = false;

  errs() << "LoopWhilifier: Starting ... \n";


#if 0
  if(const char *env_p = std::getenv("noelletest"))
  {   
    int test = std::atoi(env_p);
    errs() << "Numerically: " << test << '\n';

#if 1
    if (test > NOELLE_TEST_WHILIFIER) { /* 6+ */
      errs() << "LoopWhilifier: Cutting ...\n";
      return AnyTransformed;
    }
#endif

#if 1
    if (test == NOELLE_TEST_WHILIFIER) { /* 5. */
      errs() << "LoopWhilifier: Need to restrict NumHandled: currently ---"
             << this->NumHandled << "\n";

      if (this->NumHandled >= MAX_HANDLED_WHILIFIER) {
        errs() << "LoopWhilifier: NumHandled greater than MAX_HANDLED_WHILIFIER --- cutting...\n"; 
        return AnyTransformed;
      }
      
    }

#endif

  }   
  else errs() << "noelletest is not found" << '\n';
#endif


  /*
   * Handle subloops --- return if there is any
   * change to a subloop
   */ 
  LoopStructure *LS = LDI.getLoopStructure();

  auto SubLoops = LS->getChildren();
  for (auto SL : SubLoops) {

    /*
     * Invoke the driver on any loop structure
     */
    AnyTransformed |= this->whilifyLoopDriver(SL);

    if (AnyTransformed) {
      break;
    }

  }


  /*
   * Execute on parent loop
   */ 
  if (!AnyTransformed) {
    AnyTransformed |= this->whilifyLoopDriver(LS);
  }


  return AnyTransformed;

}


bool LoopWhilifier::whilifyLoopDriver(
  LoopStructure * const LS
) {

  auto Transformed = false;

  /*
   * Scheduler invocation --- try to shrink the 
   * loop prologue before whilifying --- FIX
   */ 
#if 1

  errs() << "THE SCHEDULER\n";

  /*
   * Get necessary info to invoke scheduler
   */ 
  auto Func = LS->getFunction();
  auto Scheduler = noelle.getScheduler();
  auto DS = noelle.getDominators(Func);
  

  /*
   * Set up the loop scheduler
   */ 
  auto LSched = Scheduler.getNewLoopScheduler(
    LS,
    DS,
    noelle.getFunctionDependenceGraph(Func)
  );


  /*
   * Shrink the loop prologue (with debugging), return 
   * true immediately
   */ 
  LSched.Dump();

  Transformed |= LSched.shrinkLoopPrologue();
  
  if (Transformed) {

    errs() << "SCHEDULED\n";
    LSched.Dump();

    return Transformed;

  }

#endif


  /*
   * Check if the loop can be whilified
   */ 
  WhilifierContext *WC = new WhilifierContext(LS);

  if (!(this->canWhilify(WC))) { 
    
    // errs() << "LoopWhilifier: Can't whilify\n";
    return Transformed; 

  }

  errs() << "BEFORE\n";
  WC->Dump();


  /*
   * If the loop is a single block, perform necessary transforms
   * for whilifying the loop --- use collected data structures
   */ 
  if (WC->OriginalHeader == WC->OriginalLatch) { 

    // errs() << "LoopWhilifier: Transforming single block loop\n";

    this->transformSingleBlockLoop(WC);
    WC->IsSingleBlockLoop |= true;

  }


  /*
   * Split into anchors and new preheader, name anchors and new 
   * preheader, update preheader pointer
   */
  this->buildAnchors(WC);


  /*
   * Clone loop blocks and remap instructions
   */ 
  this->cloneLoopBlocksForWhilifying(WC);

  llvm::remapInstructionsInBlocks(
    WC->NewBlocks, 
    WC->BodyToPeelMap
  );


  /*
   * Fix block placement --- set the peeled iteration before  
   * the loop body itself --- also optional
   */  
  WC->TopAnchor = WC->BottomAnchor; // For block placement

#if FIX_BLOCK_PLACEMENT
  Function *F = WC->F;
  F->getBasicBlockList().splice(WC->TopAnchor->getIterator(),
                                F->getBasicBlockList(),
                                (WC->NewBlocks)[0]->getIterator(), F->end());
#endif

  errs() << "LoopWhilifier: Built anchors, cloned blocks, fixed block placement:\n";
  // WC->Dump();

  /*
   * *** NOTE *** --- the "peeled" latch (the mapping from the
   * original latch to the "peeled" iteration) == NEW HEADER
   */ 
  BasicBlock *PeeledLatch = cast<BasicBlock>((WC->BodyToPeelMap)[WC->OriginalLatch]), // show
             *NewHeader = PeeledLatch;


  /*
   * Resolve all dependencies for exit edges --- all exit blocks'
   * PHINodes must reflect incoming values that are handled from
   * the new header
   */ 
  this->resolveExitEdgeDependencies(
    WC,
    NewHeader
  );


  /*
   * Find dependencies in the original latch that are defined
   * elsewhere in the loop --- necessary to build PHINodes for
   * the new header, and fix old header's incoming values
   */ 
  this->resolveNewHeaderDependencies(
    WC, 
    NewHeader
  );
    
  errs() << "LoopWhilifier: Resolved new header and exit edge dependencies\n";
  // WC->Dump(); 


  /*
   * Resolve old header PHINodes --- remove references to the
   * old latch, update any incoming values with new header
   * PHINodes whenever possible
   */ 
  this->resolveOriginalHeaderPHIs(WC);


  /*
   * Fix branches in loop body --- go to the new header instead of 
   * the old latch, target predecessors of old latch
   */ 
  this->rerouteLoopBranches(
    WC,
    NewHeader
  );

  errs() << "LoopWhilifier: Resolved original header PHIs, rerouted branches\n";
  // WC->Dump(); 


  /*
   * Erase old latch
   */ 
  (WC->OriginalLatch)->eraseFromParent();
  WC->ResolvedLatch |= true;

  errs() << "AFTER\n" << *(WC->F) << "\n";

  Transformed |= true;
  // errs() << "LoopWhilifier: Whilified\n";

  return Transformed;

}


bool LoopWhilifier::containsInOriginalLoop(
  WhilifierContext * const WC,
  BasicBlock * const BB 
) {

  /*
   * TOP --- Perform a simple find through the
   * blocks to perform "contains" --- can't rely
   * on the loop structure, as that info is invalid
   * during whilification
   */

  return llvm::find(
    (WC->LoopBlocks),
    BB) != (WC->LoopBlocks).end();

}


void LoopWhilifier::compressStructuralLatch(
  WhilifierContext *WC,
  BasicBlock *&SemanticLatch
) {

  /*
   * Get the original (structural) latch from the context
   */ 
  BasicBlock *StructuralLatch = WC->OriginalLatch;


  /*
   * Resolve the branch in the semantic latch to 
   * jump to the successor of the structural latch
   */
  BasicBlock *StructuralLatchSucc = StructuralLatch->getSingleSuccessor();

  BranchInst *SemanticLatchTerm = dyn_cast<BranchInst>(SemanticLatch->getTerminator());
  
  for (uint32_t SuccNo = 0; 
       SuccNo < SemanticLatchTerm->getNumSuccessors();
       ++SuccNo) {
    
    BasicBlock *SuccBB = SemanticLatchTerm->getSuccessor(SuccNo);
    if (SuccBB == StructuralLatch) {
      SemanticLatchTerm->setSuccessor(SuccNo, StructuralLatchSucc);
      break;
    }

  }


  /*
   * Resolve PHINodes in successors to use semantic latch
   */ 
  StructuralLatch->replaceSuccessorsPhiUsesWith(SemanticLatch);


  /*
   * Update context --- remove the structural latch from 
   * the loop blocks
   */ 
  (WC->LoopBlocks).erase(llvm::find((WC->LoopBlocks), StructuralLatch));


  /*
   * Erase the structural latch
   */ 
  StructuralLatch->eraseFromParent();


  /*
   * Update context --- set the latch to be the semantic latch
   */ 
  WC->OriginalLatch = SemanticLatch;


  // errs() << "LoopWhilifier: compressStructuralLatch, New latch is: " 
  //        << *(WC->OriginalLatch) << "\n";


  return;

}


bool LoopWhilifier::isSemanticLatch(
  WhilifierContext * const WC,
  BasicBlock *&LatchPred
) {

  /*
   * NOTE --- This method denotes the latch from the parameters
   * as the semantic latch unless all checks fail --- at
   * which point, the structural latch needs to be combined
   * with the semantic latch and then passed to isDoWhile
   */ 

  BasicBlock *CurrentLatch = WC->OriginalLatch;

  // errs() << "LoopWhilifier: Current latch:\n" 
  //        << *CurrentLatch << "\n";

  bool KeepLatch = true;


  /*
   * Check if the latch is empty (apart from the terminator)
   */ 
  if (!(CurrentLatch->getInstList().size() == 1)) {

    // errs() << "LoopWhilifier: Keeping latch --- latch not empty\n";
    return KeepLatch;

  }


  /*
   * Check if the latch has a single predecessor
   */
  LatchPred = CurrentLatch->getSinglePredecessor();
  if (!LatchPred) {

    // errs() << "LoopWhilifier: Keeping latch --- has multiple predecessors\n";
    return KeepLatch;

  }
  

  /*
   * Check if the latch terminator is an unconditional branch
   */  
  BranchInst *LatchTerm = dyn_cast<BranchInst>(CurrentLatch->getTerminator());
  if (false 
      || (!LatchTerm)
      || (LatchTerm->isConditional())) {
    
    // errs() << "LoopWhilifier: Keeping latch --- terminator not unconditional branch\n";
    return KeepLatch;

  }


  /*
   * Structural latch is not the semantic latch --- return
   * the result
   */ 
  return (KeepLatch & false);

}


bool LoopWhilifier::isAppropriateToWhilify(
  WhilifierContext *WC,
  BasicBlock * const SemanticLatch
) {

  /*
   * TOP --- To determine if it is ***appropriate*** to whilify,
   * two conditions must be met:
   * 
   * 1. The new latch(es) must not be loop exiting. 
   * 
   *    Otherwise, the loop will be whilified again in a future
   *    invocation of the EnablerManager, and there is no gain 
   *    from the whilifier.
   * 
   * 2. The loop prologue must shrink. 
   * 
   *    In a implementation built on completeness, the loop prologue
   *    must be computed from the post-dominator tree (or the CDG from 
   *    the PDG), and the "whilification" should compare to the current
   *    prologue to determine if the new prologue is smaller.
   * 
   * However, both of these conditions can be possibly be met by 
   * computing if the predecessors of the semantic latch are loop
   * exiting. 
   * 
   * If they are not, the first condition is immediately met. 
   * 
   * Additionally, this must mean that new prologue will shrink.
   * Because the semantic latch will become the new header, the
   * dominance relation between the semantic latch (to-be new header)
   * and its predecessors will be reversed in the whilified loop. 
   * Since the predecessors are not loop-exiting, no instruction
   * in the whilified loop will be control-dependent on those blocks.
   * As a result, the blocks will be removed from the prologue in 
   * the whilified loop --- generating at least *some* shrinkage.
   * 
   * If the predecessors were loop-exiting, the prologue cannot
   * shrink because the predecessors would become the new latch(es)
   * of the whilified loop.
   * 
   * It is worth noting that if all instructions in the loop are 
   * control-dependent on the semantic latch terminator, then the
   * enabling mechanism from the whilifier is maximized for 
   * parallelization.
   */ 


  /*
   * Get necessary predecessors of the semantic latch
   * 
   * Ignore blocks that are:
   * a. Not part of the current loop
   * b. The semantic latch itself
   */
  SmallVector<BasicBlock *, 16> SemanticLatchPreds;

  for (auto *PredBB : predecessors(SemanticLatch)) {

    if (false
        || PredBB == SemanticLatch
        || !(this->containsInOriginalLoop(WC, PredBB))) {
      continue;
    }

    SemanticLatchPreds.push_back(PredBB);

  }


  /*
   * Compute if they are loop exiting --- NEEDS OPTIMIZATION
   */ 
  bool IsAppropriate = true;

  for (auto *PredBB : SemanticLatchPreds) {

    for (auto *SuccBB : successors(PredBB)) {

      if (!(this->containsInOriginalLoop(WC, SuccBB))) {
        IsAppropriate &= false;;
      }
      
    }

  }


  /*
   * Update context
   */
  WC->IsAppropriateToWhilify = IsAppropriate;


  return IsAppropriate;

}


bool LoopWhilifier::isDoWhile(
  WhilifierContext *WC
) {

  bool IsDoWhile = false;

  /*
   * TOP --- If any of the successors of the latch are not part 
   * of the loop --- the latch must be an intra-loop edge to an 
   * exit block
   * 
   * The loop is a do-while loop as long as the latch is loop-
   * exiting
   * 
   * Often times the latch selected is not the semantic latch, so
   * this must be identified first
   */  


  /*
   * First, check if the semantic latch is the structural latch,
   * need to use the predecessor basic block in the do-while 
   * check otherwise
   */ 
  BasicBlock *CurrentLatch = WC->OriginalLatch,
             *CurrentLatchPred = nullptr;

  bool NeedToChangeLatch = !(this->isSemanticLatch(WC, CurrentLatchPred));

  BasicBlock *SemanticLatch = (NeedToChangeLatch) ?
                              (CurrentLatchPred) :
                              (CurrentLatch);

  // errs() << "LoopWhilifier: NeedToChangeLatch " 
        //  << std::to_string(NeedToChangeLatch) << "\n"
        //  << "LoopWhilifier: SemanticLatch " << *SemanticLatch << "\n";


  /*
   * Next, check the do-while condition --- is the latch loop
   * exiting?
   */ 
  for (auto *SuccBB : successors(SemanticLatch)) {
    
    if (!(this->containsInOriginalLoop(WC, SuccBB))) {
      IsDoWhile |= true;
    }

  }

  // errs() << "LoopWhilifier: IsDoWhile (Latch --- loop exiting): "
        //  << std::to_string(IsDoWhile) << "\n";


  /*
   * Check if loop is appropriate to whilify
   */ 
  bool IsAppropriateToWhilify = this->isAppropriateToWhilify(WC, SemanticLatch);

  // errs() << "LoopWhilifier: IsDoWhile (Appropriate to whilify): "
  //        << std::to_string(IsAppropriateToWhilify) + "\n";


  /* 
   * Is a do-while loop and appropriate to whilify --- transform
   * the structural latch if necessary
   */ 
  if (NeedToChangeLatch 
      && IsDoWhile
      && IsAppropriateToWhilify) {

    this->compressStructuralLatch(WC, SemanticLatch);
    WC->ConsolidatedOriginalLatch |= true;

  }

  // WC->Dump();

  return IsDoWhile & IsAppropriateToWhilify;

}


bool LoopWhilifier::canWhilify (
  WhilifierContext *WC
) {

  bool canWhilify = true;

  /*
   * TOP --- Require valid header, preheader, and single latch, 
   * then check if the loop is in do-while form
   */ 


  /*
   * Acquire header
   */ 
  canWhilify &= !!(WC->OriginalHeader);


  /*
   * Acquire latch
   */ 
  canWhilify &= (WC->NumLatches == 1);


  /*
   * Acquire preheader
   */ 
  canWhilify &= !!(WC->OriginalPreHeader);


  /*
   * Acquire exits
   */ 
  canWhilify &= ((WC->ExitEdges).size() > 0);


  /*
   * Check if loop is in do-while form
   */ 
  if (canWhilify) {
    canWhilify &= this->isDoWhile(WC);
  }


  /*
   * Set context
   */ 
  WC->IsDoWhile |= canWhilify;


  return canWhilify;

}


void LoopWhilifier::transformSingleBlockLoop(
  WhilifierContext *WC
) {

  /*
   * Split the header at the terminator --- new block will
   * be the new latch
   */ 
  BasicBlock *Header = WC->OriginalHeader;

  Instruction *SplitPoint = Header->getTerminator();
  BasicBlock *NewLatch = SplitBlock(Header, SplitPoint);
  NewLatch->setName(".new.latch");


  /*
   * Update latch, loop blocks, and exit blocks
   */ 
  WC->OriginalLatch = NewLatch;

  (WC->LoopBlocks).push_back(NewLatch);

  std::vector<std::pair<BasicBlock *, BasicBlock *>> NewExitEdges;
  for (auto Edge : WC->ExitEdges) {

    if (Edge.first == Header) {
      NewExitEdges.push_back({ NewLatch, Edge.second });
    } else {
      NewExitEdges.push_back(Edge);
    }

  }

  WC->ExitEdges = NewExitEdges; /* FIX --- Copy */


  return;

}


void LoopWhilifier::buildAnchors(
  WhilifierContext *WC
) {

  /*
   * TOP --- Split the edge between the original preheader and  
   * the original header --- create a top anchor for the future
   * cloned loop blocks to reside; bottom anchor for a similar
   * reason;
   * 
   * The new preheader created for the current (soon to be 
   * old) loop is customary --- remnant of llvm::peelLoop
   */ 

  // errs() << "LoopWhilifier: Building anchors ...\n";

  BasicBlock *Header = WC->OriginalHeader,
             *PreHeader = WC->OriginalPreHeader,

             *InsertTop = SplitEdge(PreHeader, Header),
             *InsertBot = SplitBlock(InsertTop, InsertTop->getTerminator()),
             *NewPreHeader = SplitBlock(InsertBot, InsertBot->getTerminator());


  /*
   * Naming
   */ 
  InsertTop->setName("whilify.top.anchor." + Header->getName());
  InsertBot->setName("whilify.bottom.anchor." + Header->getName());
  NewPreHeader->setName("whilify.ph.old.loop." + PreHeader->getName());


  /*
   * Update context
   */ 
  WC->TopAnchor = InsertTop;
  WC->BottomAnchor = InsertBot;
  WC->OriginalPreHeader = NewPreHeader;


  // errs() << "LoopWhilifier: TopAnchor: " << *InsertTop << "\n"
  //        << "LoopWhilifier: BottomAnchor: " << *InsertBot << "\n"
  //        << "LoopWhilifier: WhilifiedPreheader: " << *NewPreHeader << "\n"
  //        << "LoopWhilifier: Done building anchors\n";


  return;

}


/*
 * Based on LoopUnrollPeel.cpp : cloneLoopBlocks
 */
void LoopWhilifier::cloneLoopBlocksForWhilifying(
  WhilifierContext *WC
) {

  BasicBlock *InsertTop = WC->TopAnchor,
             *InsertBot = WC->BottomAnchor,
             *OriginalHeader = WC->OriginalHeader,
             *OriginalLatch = WC->OriginalLatch,
             *OriginalPreHeader = WC->OriginalPreHeader;

  Function *F = WC->F;


  /*
   * For each block in the original loop, create a new copy,
   * and update the value map with the newly created values.
   */
  for (auto OrigBB : WC->LoopBlocks) {
  
    BasicBlock *PeelBB = CloneBasicBlock(
      OrigBB, 
      (WC->BodyToPeelMap), 
      ".whilify", 
      F
    );

    (WC->NewBlocks).push_back(PeelBB);
    (WC->BodyToPeelMap)[OrigBB] = PeelBB;

  }


  /*
   * Fix control flow --- The top anchor must branch unconditionally
   * to the "peeled header" --- pulled from the ValueToValueMap
   */ 
  InsertTop->getTerminator()->setSuccessor(
    0, cast<BasicBlock>((WC->BodyToPeelMap)[OriginalHeader])
  );


  /*
   * Fix control flow --- The bottom anchor must receive the "peeled" 
   * latch, which currently goes straight to the original header and 
   * to the exit block --- this must be rerouted to the bottom 
   * anchor instead
   */ 
  BasicBlock *PeelLatch = cast<BasicBlock>((WC->BodyToPeelMap)[OriginalLatch]);
  BranchInst *PeelLatchTerm = cast<BranchInst>(PeelLatch->getTerminator());

  for (uint32_t SuccNo = 0; 
       SuccNo < PeelLatchTerm->getNumSuccessors(); 
       ++SuccNo) {

    if (PeelLatchTerm->getSuccessor(SuccNo) == OriginalHeader) {
        PeelLatchTerm->setSuccessor(SuccNo, InsertBot);
        break;
    }

  }


  /*
   * Remove unnecessary PHINodes from the "peeled" header --- these
   * are necessary for the backedge for the original header, but the
   * corresponding "peeled" block does not require these PHINodes 
   * 
   * Update the ValueToValueMap to map the original PHINodes to the 
   * static incoming values from the preheader
   */
  for (PHINode &PHI : OriginalHeader->phis()) {

    PHINode *PeelPHI = cast<PHINode>((WC->BodyToPeelMap)[&PHI]);
    (WC->BodyToPeelMap)[&PHI] = PeelPHI->getIncomingValueForBlock(OriginalPreHeader);
    cast<BasicBlock>((WC->BodyToPeelMap)[OriginalHeader])->getInstList().erase(PeelPHI);
  
  }


  /*
   * Resolve exit edges --- the outgoing values must be fixed so
   * the "peeled" latch will replace the original latch as an
   * incoming block to the destination in the edge
   * 
   * The incoming value depends on if the value was defined
   * in the loop body --- if so, we must propagate the corresponding
   * value from the "peeled" block
   * 
   * Resolve all latch-exit dependencies here  
   * 
   * NEEDS ENGINEERING FIX --- CODE REPETITION --- TODO
   * 
   */ 
  // F->print(errs());
  for (auto Edge : WC->ExitEdges) {

    for (PHINode &PHI : Edge.second->phis()) {

      /*
       * Need to determine if the incoming value
       * must be removed 
       */ 
      auto NeedToRemoveIncoming = false;

      Value *Incoming = PHI.getIncomingValueForBlock(Edge.first),
            *Propagating = Incoming;

      Instruction *IncomingInst = dyn_cast<Instruction>(Incoming);

      if (IncomingInst 
          && (this->containsInOriginalLoop(WC, IncomingInst->getParent()))) {

        Propagating = (WC->BodyToPeelMap)[Incoming];

        /*
         * If the exit edge source is the OriginalLatch, the incoming
         * value must be removed
         */ 
        if (Edge.first == OriginalLatch) {

          /*
           * If the incoming value itself is not defined in the 
           * original latch --- it needs a dependency PHINode
           * to be propagated to the exit block
           * 
           * For now --- mark this in the ExitDependencies map, 
           * and add the Propagating value to the exit block PHI
           * 
           * This must be resolved post remapInstructionsInBlocks
           */ 
          if (IncomingInst->getParent() != OriginalLatch) {
            (WC->ExitDependencies)[&PHI] = Incoming;
          }

          NeedToRemoveIncoming |= true;

        }

      }

      PHI.addIncoming(Propagating, cast<BasicBlock>((WC->BodyToPeelMap)[Edge.first]));

      if (NeedToRemoveIncoming) {
        PHI.removeIncomingValue(Edge.first);
      }

    }

  }


  return;

}


PHINode * LoopWhilifier::buildNewHeaderDependencyPHI(
  WhilifierContext *WC,
  Value *Dependency
) {

  /*
   * Get necessary blocks
   */  
  BasicBlock *Latch = WC->OriginalLatch,
             *NewHeader = cast<BasicBlock>((WC->BodyToPeelMap)[Latch]);

  /*
   * Build the new PHINode
   */ 
  IRBuilder<> NewHeaderBuilder{NewHeader->getFirstNonPHI()};
  PHINode *DependencyPHI = NewHeaderBuilder.CreatePHI(Dependency->getType(), 0);


  /*
   * Populate PHINode --- add incoming values based on the
   * predecessors of both the original and peeled latch
   */ 
  Value *MappedDependency = cast<Value>((WC->BodyToPeelMap)[Dependency]);

  for (auto *PredBB : predecessors(NewHeader)) {
    DependencyPHI->addIncoming(MappedDependency, PredBB);
  }

  for (auto *PredBB : predecessors(Latch)) {
    DependencyPHI->addIncoming(Dependency, PredBB);
  }


  return DependencyPHI;

}


void LoopWhilifier::resolveExitEdgeDependencies(
  WhilifierContext *WC,
  BasicBlock *NewHeader
) {

  /*
   * For each exit dependency --- which consists of the 
   * exit block PHINode and the incoming value that needs to
   * be handled --- build a new PHINode in the header to 
   * handle values from the "peeled" iteration and the original
   * loop body
   * 
   * Propagate this new PHINode as the incoming value in the
   * Phi for the exit block
   */ 

  for (auto const &[PHI, Incoming] : WC->ExitDependencies) {

    PHINode *ExitDependencyPHI = this->buildNewHeaderDependencyPHI(
      WC,
      Incoming
    );

    PHI->setIncomingValueForBlock(NewHeader, ExitDependencyPHI);

  }

  return;

}


void LoopWhilifier::resolveNewHeaderPHIDependencies(
  WhilifierContext *WC
) {

  /*
   * TOP --- Get each PHINode in the latch and its corresponding
   * PHINode in the "peeled" latch / new header from the 
   * ValueToValueMap
   * 
   * For each of these corresponding PHINodes, the incoming values
   * that already exist are from the "peeled" loop iteration. Since
   * this block is also the new header, it must take all values from
   * the loop body as well
   */

  BasicBlock *Latch = WC->OriginalLatch;

  for (PHINode &OriginalPHI : Latch->phis()) {

    PHINode *PeeledPHI = cast<PHINode>((WC->BodyToPeelMap)[&OriginalPHI]);

    for (uint32_t PHINo = 0; 
         PHINo < OriginalPHI.getNumIncomingValues(); 
        ++PHINo) {

      /*
       * Pull from the original PHINode --- that original
       * is from the old latch --- which will be erased --- so
       * the incoming values must be preserved
       */ 
      PeeledPHI->addIncoming(
        OriginalPHI.getIncomingValue(PHINo),
        OriginalPHI.getIncomingBlock(PHINo) 
      );

    }

  }

  return;
}


void LoopWhilifier::findNonPHIOriginalLatchDependencies(
  WhilifierContext *WC
) {

  /*
   * TOP --- find all instruction-based dependencies in
   * the original latch block that are NOT defined in the 
   * original latch but ARE defined elsewhere in the 
   * original loop body
   */ 
  BasicBlock *Latch = WC->OriginalLatch;

  for (auto &I : *Latch) {

    /*
     * PHIs handled separately/already
     */ 
    if (isa<PHINode>(&I)) {
      continue;
    }
    
    /*
     * Loop through operands of each instruction
     * to find dependencies, fill in the map
     */ 
    for (uint32_t OpNo = 0; 
         OpNo != I.getNumOperands(); 
         ++OpNo) {

      Value *OP = I.getOperand(OpNo);

      Instruction *Dependence = dyn_cast<Instruction>(OP);
      if (!Dependence) {
        continue;
      }

      BasicBlock *DependenceParent = Dependence->getParent();

      if ((this->containsInOriginalLoop(WC, DependenceParent))
          && (DependenceParent != Latch)) {
        ((WC->OriginalLatchDependencies)[Dependence])[&I] = OpNo;
      }

    }
  }

  return;
  
}


void LoopWhilifier::resolveNewHeaderNonPHIDependencies(
  WhilifierContext *WC,
  BasicBlock *NewHeader
) {

  /*
   * TOP --- For each dependency (non PHINode) found in the
   * original latch, a new PHINode must be inserted into the 
   * new header to handle incoming values from the actual
   * loop body AND from the "peeled" iteration
   * 
   * Replace uses in appropriate positions
   * 
   * Record the resolved dependencies in a map
   */ 
  for (auto const &[D, Uses] : WC->OriginalLatchDependencies) {

    /*
     * Build the new PHINode
     */ 
    PHINode *DependencyPHI = this->buildNewHeaderDependencyPHI(WC, D);


    /*
     * Get corresponding value for the dependency in "peeled" iteration
     */ 
    Value *PeeledDependency = cast<Value>((WC->BodyToPeelMap)[D]);


    /*
     * Replacing uses in instructions that carry the dependencies,
     * corresponding to instructions in the peeled latch 
     */ 
    for (auto const &[I, OpNo] : Uses) {
      Instruction *CorrespondingI = cast<Instruction>((WC->BodyToPeelMap)[I]);
      CorrespondingI->setOperand(OpNo, DependencyPHI);
    }

    
    /*
     * Save the mapping between D, PeeledDependency and the 
     * DependencyPHI instruction
     */ 
    (WC->ResolvedDependencyMapping)[D] = 
      (WC->ResolvedDependencyMapping)[PeeledDependency] = 
        DependencyPHI;

  }

  return;

}


void LoopWhilifier::resolveNewHeaderDependencies(
  WhilifierContext *WC,
  BasicBlock *NewHeader
) {

  /*
   * TOP --- The new header must handle all correct incoming
   * values --- this means all existing PHINodes in the block
   * must be updated to handle incoming values from the original
   * loop body AND "peeled" iteration, all other dependencies
   * must have proper PHINodes to also account for incoming values
   */ 


  /*
   * Start with PHINodes of the latch
   */ 
  this->resolveNewHeaderPHIDependencies(WC);


  /*
   * Find dependencies in the original latch that are defined
   * elsewhere in the loop --- necessary to build PHINodes for
   * the new header, and fix old header's incoming values
   */  
  this->findNonPHIOriginalLatchDependencies(WC);


  /*
   * Now build PHINodes for all other dependencies in the new header
   */ 
  this->resolveNewHeaderNonPHIDependencies(
    WC,
    NewHeader
  );


  return;

}


void LoopWhilifier::resolveOriginalHeaderPHIs(
  WhilifierContext *WC
) {

  /*
   * TOP --- Take all PHINodes for the original header and
   * update all incoming values to route from the correct
   * predecessor instead of the old latch
   * 
   * In the current basic block layout, the "new preheader" 
   * anchor will be the predecessor of the old header --- so
   * all incoming values must route from there, and all values
   * incoming from the old latch must be removed
   * 
   * Finally --- if there is a dependency that was resolved
   * for the NEW header, it must be reflected in the PHINodes
   * of the old header --- pulled from the ResolvedDependencyMap
   */ 

  BasicBlock *Header = WC->OriginalHeader,
             *PreHeader = WC->OriginalPreHeader,
             *Latch = WC->OriginalLatch;

  for (PHINode &OriginalPHI : Header->phis()) {

    Value *Incoming = OriginalPHI.getIncomingValueForBlock(Latch);
    Instruction *IncomingInst = dyn_cast<Instruction>(Incoming);

    if (IncomingInst) {

      BasicBlock *IncomingParent = IncomingInst->getParent();

      /*
       * If the incoming value is found in the ResolvedDependencyMapping,
       * then the value (PHINode) to propagate already exists --- and
       * incoming can be set properly
       */ 
      if ((WC->ResolvedDependencyMapping).find(Incoming) !=
          (WC->ResolvedDependencyMapping).end()) {
        Incoming = (WC->ResolvedDependencyMapping)[Incoming];
      } 

      /*
       * If the incoming value is defined in the loop body but outside
       * the latch, a new dependency PHINode must be generated to 
       * handle "peeled" incoming values and incoming values from the 
       * rest of the loop body --- set incoming accordingly
       */ 
      else if ((IncomingParent != Latch)
                && (this->containsInOriginalLoop(WC, IncomingParent))) {
        
        PHINode *DependencyPHI = this->buildNewHeaderDependencyPHI(
          WC,
          Incoming
        );

        Incoming = DependencyPHI;

      } 

      /*
       * Otherwise, set incoming to be the corresponding value from the
       * ValueToValueMap --- prevent using the original latch value, since 
       * the original latch will be erased
       */ 
      else {
        Incoming = (WC->BodyToPeelMap)[IncomingInst];
      }

    }

    OriginalPHI.setIncomingValueForBlock(PreHeader, Incoming);
    OriginalPHI.removeIncomingValue(Latch);

  }

  return;

}


void LoopWhilifier::rerouteLoopBranches(
  WhilifierContext *WC,
  BasicBlock *NewHeader
) {

  /*
   * TOP --- Reroute branches from original latch to the new 
   * header, loop through all predecessors to reroute
   * 
   * Necessary because the original latch will be erased
   */ 

  BasicBlock *Latch = WC->OriginalLatch;

  SmallVector<BasicBlock *, 32> Preds;
  for (auto *PredBB : predecessors(Latch)) {
    Preds.push_back(PredBB);
  }

  for (auto *PredBB : Preds) {

    BranchInst *PredTerm = dyn_cast<BranchInst>(PredBB->getTerminator());
    if (!PredTerm) {
      continue;
    }

    for (uint32_t SuccNo = 0;
         SuccNo < PredTerm->getNumSuccessors();
         ++SuccNo) {

      if (PredTerm->getSuccessor(SuccNo) == Latch) {
        PredTerm->setSuccessor(SuccNo, NewHeader);
      }

    }

  }

  return;

}


WhilifierContext::WhilifierContext(
  LoopStructure * const LS
) {

  /*
   * Only set loop body context
   */ 
  this->OriginalHeader = LS->getHeader();

  this->OriginalPreHeader = LS->getPreHeader();

  auto Latches = LS->getLatches();
  this->OriginalLatch = *(Latches.begin());
  this->NumLatches = Latches.size();

  auto AllExitEdges = LS->getLoopExitEdges();
  for (auto EE : AllExitEdges) {
    this->ExitEdges.push_back(EE);
  }

  for (auto NextBB : LS->orderedBBs) {
    this->LoopBlocks.push_back(NextBB);
  }

  this->F = LS->getFunction();


  /*
   * Set analysis results to false
   */ 
  this->IsDoWhile = 
    this->IsAppropriateToWhilify =
      this->IsSingleBlockLoop = 
        this->ConsolidatedOriginalLatch = 
          this->ResolvedLatch = false;


  return;

}


void WhilifierContext::Dump()
{

  errs() << "LoopWhilifier: Current Context\n";

  /*
   * Loop body info
   */ 
  if (this->OriginalHeader) {
    errs() << "LoopWhilifier:   OriginalHeader " << *(this->OriginalHeader) << "\n";
  }

  if (this->OriginalLatch && !(this->ResolvedLatch)) {
    errs() << "LoopWhilifier:   OriginalLatch " << *(this->OriginalLatch) << "\n";
  }

  if (this->OriginalPreHeader) {
    errs() << "LoopWhilifier:   OriginalPreHeader " << *(this->OriginalPreHeader) << "\n";
  }

  errs() << "LoopWhilifier:   ExitEdges:\n";
  for (auto EE : this->ExitEdges) {
    errs() << "---\nLoopWhilifier:     From: " << *(EE.first) << "\n"
           << "LoopWhilifier:     To: " << *(EE.second) << "\n---\n";
  }

  errs() << "---\nLoopWhilifier:   LoopBlocks:\n";
  for (auto BB : LoopBlocks) {
    errs() << *BB << "\n";
  }
  errs() << "---\n";

  errs() << "LoopWhilifier:   Current Function:\n" << *F << "\n";


  /*
   * Whilification info
   */ 
  errs() << "LoopWhilifier:   IsDoWhile: " << std::to_string(this->IsDoWhile) << "\n"
         << "LoopWhilifier:   IsAppropriateToWhilify: " << std::to_string(this->IsAppropriateToWhilify) << "\n"
         << "LoopWhilifier:   IsSingleBlockLoop: " << std::to_string(this->IsSingleBlockLoop) << "\n"
         << "LoopWhilifier:   ConsolidatedOriginalLatch: " << std::to_string(this->ConsolidatedOriginalLatch) << "\n";


  return;


}

