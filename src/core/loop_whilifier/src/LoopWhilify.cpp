/*
 * Copyright 2019 - 2021  Souradip Ghosh, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/core/LoopWhilify.hpp"

namespace llvm::noelle{

  LoopWhilifier::LoopWhilifier () :
    outputPrefix{"Whilifier: "}
  {
    return ;
  }


  bool LoopWhilifier::whilifyLoop (
    LoopDependenceInfo &LDI,
    Scheduler &scheduler,
    DominatorSummary *DS,
    PDG *FDG
  ) {
    
    /*
     * Execute on target loop from @LDI
     */ 
    auto AnyTransformed = false;
    errs() << outputPrefix << "Start\n";
    errs() << outputPrefix << " Try to whilify the target loop\n";

    auto LS = LDI.getLoopStructure();
    AnyTransformed |= whilifyLoopDriver(LS, scheduler, DS, FDG);

    errs() << outputPrefix << " Transformed = " << AnyTransformed << "\n";
    errs() << outputPrefix << "Exit\n";

    return AnyTransformed;
  }


  bool LoopWhilifier::whilifyLoopDriver(
    LoopStructure * const LS,
    Scheduler &scheduler,
    DominatorSummary *DS,
    PDG *FDG
  ) {
    auto Transformed = false;

    /*
     * Fetch the function that contains the loop.
     */
    auto ParentFunc = LS->getFunction();

    /*
     * Get necessary info to invoke scheduler
     */ 
    auto firstInst = LS->getEntryInstruction();
    errs() << outputPrefix << "   Loop: " << *firstInst << "\n";


    /*
     * Scheduler invocation --- try to shrink the loop prologue before whilifying
     *
     * Check if we need to shrink the prologue at all.
     */
    errs() << outputPrefix << "     Try to shrink the loop prologue " << *firstInst << "\n";


    /*
     * Set up the loop scheduler
     */ 
    auto LSched = scheduler.getNewLoopScheduler(
      LS,
      DS,
      FDG
    );


    /*
     * Shrink the loop prologue (with debugging), return true immediately
     */ 
    Transformed |= LSched.shrinkLoopPrologue();
    if (Transformed) {
      errs() << outputPrefix << "       The prologue has shrunk\n";
      return Transformed;
    }


    /*
     * Check if the loop can be whilified
     */ 
    errs() << outputPrefix << "     Try to whilify " << *firstInst << "\n";
    auto WC = WhilifierContext(LS);
    if (!(canWhilify(WC))) { 
      return Transformed; 
    }


    /*
     * If the loop is a single block, perform necessary transforms
     * for whilifying the loop --- use collected data structures
     */ 
    if (WC.OriginalHeader == WC.OriginalLatch) { 
      errs() << outputPrefix << "       This is a single-block loop\n";
      this->transformSingleBlockLoop(WC);
      WC.IsSingleBlockLoop |= true;
    }


    /*
     * Split into anchors and new preheader, name anchors and new 
     * preheader, update preheader pointer
     */
    buildAnchors(WC);


    /*
     * Clone loop blocks and remap instructions
     */ 
    cloneLoopBlocksForWhilifying(WC);
    llvm::remapInstructionsInBlocks(
      WC.NewBlocks, 
      WC.BodyToPeelMap
    );


    /*
     * Fix block placement --- set the peeled iteration before  
     * the loop body itself --- also optional
     */  
    WC.TopAnchor = WC.BottomAnchor; /* For block placement */
    Function *F = WC.F;
    F->getBasicBlockList().splice(
      WC.TopAnchor->getIterator(),
      F->getBasicBlockList(),
      (WC.NewBlocks)[0]->getIterator(), 
      F->end()
    );

    errs() << outputPrefix << "     Whilified\n";


    /*
     * *** NOTE *** --- the "peeled" latch (the mapping from the
     * original latch to the "peeled" iteration) == NEW HEADER
     */ 
    BasicBlock *PeeledLatch = cast<BasicBlock>((WC.BodyToPeelMap)[WC.OriginalLatch]),
               *NewHeader = PeeledLatch;


    /*
     * Resolve all dependencies for exit edges --- all exit blocks'
     * PHINodes must reflect incoming values that are handled from
     * the new header
     */ 
    resolveExitEdgeDependencies(
      WC,
      NewHeader
    );


    /*
     * Find dependencies in the original latch that are defined
     * elsewhere in the loop --- necessary to build PHINodes for
     * the new header, and fix old header's incoming values
     */ 
    resolveNewHeaderDependencies(
      WC, 
      NewHeader
    );


    /*
     * Resolve old header PHINodes --- remove references to the
     * old latch, update any incoming values with new header
     * PHINodes whenever possible
     */ 
    resolveOriginalHeaderPHIs(WC);


    /*
     * Fix branches in loop body --- go to the new header instead of 
     * the old latch, target predecessors of old latch
     */ 
    rerouteLoopBranches(
      WC,
      NewHeader
    );


    /*
     * Erase old latch
     */ 
    (WC.OriginalLatch)->eraseFromParent();
    WC.ResolvedLatch |= true;

    Transformed |= true;
    return Transformed;

  }


  bool LoopWhilifier::containsInOriginalLoop(
    WhilifierContext const &WC,
    BasicBlock * const BB 
  ) {

    /*
     * TOP --- Perform a simple find through the
     * blocks to perform "contains" --- can't rely
     * on the loop structure, as that info is invalid
     * during whilification
     */

    return llvm::find(
        (WC.LoopBlocks),
        BB) != (WC.LoopBlocks).end();

  }


  void LoopWhilifier::compressStructuralLatch(
    WhilifierContext &WC,
    BasicBlock *&SemanticLatch
  ) {

    /*
     * Get the original (structural) latch from the context
     */ 
    BasicBlock *StructuralLatch = WC.OriginalLatch;


    /*
     * Resolve the branch in the semantic latch to 
     * jump to the successor of the structural latch
     */
    BasicBlock *StructuralLatchSucc = StructuralLatch->getSingleSuccessor();
    BranchInst *SemanticLatchTerm = dyn_cast<BranchInst>(SemanticLatch->getTerminator());
    for (
      uint32_t SuccNo = 0; 
      SuccNo < SemanticLatchTerm->getNumSuccessors();
      ++SuccNo
    ) {

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
     * Update context --- remove the structural latch from the loop blocks
     */ 
    (WC.LoopBlocks).erase(llvm::find((WC.LoopBlocks), StructuralLatch));


    /*
     * Erase the structural latch
     */ 
    StructuralLatch->eraseFromParent();


    /*
     * Update context --- set the latch to be the semantic latch
     */ 
    WC.OriginalLatch = SemanticLatch;


    return;

  }


  bool LoopWhilifier::isSemanticLatch(
    WhilifierContext const &WC,
    BasicBlock *&LatchPred
  ) {

    /*
     * NOTE --- This method denotes the latch from the parameters
     * as the semantic latch unless all checks fail --- at
     * which point, the structural latch needs to be combined
     * with the semantic latch and then passed to isDoWhile
     */ 

    BasicBlock *CurrentLatch = WC.OriginalLatch;
    bool KeepLatch = true;


    /*
     * Check if the latch is empty (apart from the terminator)
     */ 
    if (!(CurrentLatch->getInstList().size() == 1)) {
      return KeepLatch;
    }


    /*
     * Check if the latch has a single predecessor
     */
    LatchPred = CurrentLatch->getSinglePredecessor();
    if (!LatchPred) {
      return KeepLatch;
    }


    /*
     * Check if the latch terminator is an unconditional branch
     */  
    BranchInst *LatchTerm = dyn_cast<BranchInst>(CurrentLatch->getTerminator());
    if (false 
        || (!LatchTerm)
        || (LatchTerm->isConditional())) {
      return KeepLatch;
    }


    /*
     * Structural latch is not the semantic latch --- return
     * the result
     */ 
    return (KeepLatch & false);

  }


  bool LoopWhilifier::isAppropriateToWhilify(
    WhilifierContext &WC,
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
          || !(containsInOriginalLoop(WC, PredBB))) {
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
        if (!(containsInOriginalLoop(WC, SuccBB))) {
          IsAppropriate &= false;;
        }
      }
    }


    /*
     * Update context
     */
    WC.IsAppropriateToWhilify = IsAppropriate;


    return IsAppropriate;

  }


  bool LoopWhilifier::isDoWhile(
    WhilifierContext &WC
  ) {

    auto IsDoWhile = false;

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
    BasicBlock *CurrentLatch = WC.OriginalLatch;
    BasicBlock *CurrentLatchPred = nullptr;
    bool NeedToChangeLatch = !(isSemanticLatch(WC, CurrentLatchPred));
    BasicBlock *SemanticLatch = (NeedToChangeLatch) ?
      (CurrentLatchPred) :
      (CurrentLatch);


    /*
     * Next, check the do-while condition --- is the latch loop exiting?
     */ 
    for (auto *SuccBB : successors(SemanticLatch)) {
      if (!(containsInOriginalLoop(WC, SuccBB))) {
        IsDoWhile |= true;
      }
    }


    /*
     * Check if loop is appropriate to whilify
     */ 
    bool IsAppropriateToWhilify = isAppropriateToWhilify(WC, SemanticLatch);


    /* 
     * Is a do-while loop and appropriate to whilify --- transform
     * the structural latch if necessary
     */ 
    if (true
        && NeedToChangeLatch 
        && IsDoWhile
        && IsAppropriateToWhilify) {
      compressStructuralLatch(WC, SemanticLatch);
      WC.ConsolidatedOriginalLatch |= true;
    }


    return IsDoWhile & IsAppropriateToWhilify;

  }


  bool LoopWhilifier::canWhilify (
    WhilifierContext &WC
  ) {

    auto canWhilify = true;

    /*
     * TOP --- Require valid header, preheader, and single latch, 
     * then check if the loop is in do-while form
     */ 


    /*
     * Acquire header
     */ 
    canWhilify &= !!(WC.OriginalHeader);


    /*
     * Acquire latch
     */ 
    canWhilify &= (WC.NumLatches == 1);


    /*
     * Acquire preheader
     */ 
    canWhilify &= !!(WC.OriginalPreHeader);


    /*
     * Acquire exits
     */ 
    canWhilify &= ((WC.ExitEdges).size() > 0);


    /*
     * Check if loop is in do-while form
     */ 
    if (canWhilify) {
      canWhilify &= isDoWhile(WC);
    }


    /*
     * Set context
     */ 
    WC.IsDoWhile |= canWhilify;


    return canWhilify;

  }


  void LoopWhilifier::transformSingleBlockLoop(
    WhilifierContext &WC
  ) {

    /*
     * Split the header at the terminator --- new block will be the new latch
     */ 
    BasicBlock *Header = WC.OriginalHeader;
    Instruction *SplitPoint = Header->getTerminator();
    BasicBlock *NewLatch = SplitBlock(Header, SplitPoint);
    NewLatch->setName(".new.latch");


    /*
     * Update latch, loop blocks, and exit blocks
     */ 
    WC.OriginalLatch = NewLatch;
    (WC.LoopBlocks).push_back(NewLatch);

    std::vector<std::pair<BasicBlock *, BasicBlock *>> NewExitEdges;
    for (auto Edge : WC.ExitEdges) {
      if (Edge.first == Header) {
        NewExitEdges.push_back({ NewLatch, Edge.second });
      } else {
        NewExitEdges.push_back(Edge);
      }
    }

    WC.ExitEdges = NewExitEdges; /* FIX --- Copy */


    return;

  }


  void LoopWhilifier::buildAnchors(
    WhilifierContext &WC
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
    BasicBlock *Header = WC.OriginalHeader,
               *PreHeader = WC.OriginalPreHeader,
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
    WC.TopAnchor = InsertTop;
    WC.BottomAnchor = InsertBot;
    WC.OriginalPreHeader = NewPreHeader;


    return;

  }


  /*
   * Based on LoopUnrollPeel.cpp : cloneLoopBlocks
   */
  void LoopWhilifier::cloneLoopBlocksForWhilifying(
    WhilifierContext &WC
  ) {

    BasicBlock *InsertTop = WC.TopAnchor,
               *InsertBot = WC.BottomAnchor,
               *OriginalHeader = WC.OriginalHeader,
               *OriginalLatch = WC.OriginalLatch,
               *OriginalPreHeader = WC.OriginalPreHeader;

    Function *F = WC.F;


    /*
     * For each block in the original loop, create a new copy,
     * and update the value map with the newly created values.
     */
    for (auto OrigBB : WC.LoopBlocks) {

      BasicBlock *PeelBB = CloneBasicBlock(
        OrigBB, 
        (WC.BodyToPeelMap), 
        ".whilify", 
        F
      );

      (WC.NewBlocks).push_back(PeelBB);
      (WC.BodyToPeelMap)[OrigBB] = PeelBB;

    }


    /*
     * Fix control flow --- The top anchor must branch unconditionally
     * to the "peeled header" --- pulled from the ValueToValueMap
     */ 
    InsertTop->getTerminator()->setSuccessor(
      0, cast<BasicBlock>((WC.BodyToPeelMap)[OriginalHeader])
    );


    /*
     * Fix control flow --- The bottom anchor must receive the "peeled" 
     * latch, which currently goes straight to the original header and 
     * to the exit block --- this must be rerouted to the bottom 
     * anchor instead
     */ 
    BasicBlock *PeelLatch = cast<BasicBlock>((WC.BodyToPeelMap)[OriginalLatch]);
    BranchInst *PeelLatchTerm = cast<BranchInst>(PeelLatch->getTerminator());
    for (
      uint32_t SuccNo = 0; 
      SuccNo < PeelLatchTerm->getNumSuccessors(); 
      ++SuccNo
    ) {
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
      PHINode *PeelPHI = cast<PHINode>((WC.BodyToPeelMap)[&PHI]);
      (WC.BodyToPeelMap)[&PHI] = PeelPHI->getIncomingValueForBlock(OriginalPreHeader);
      cast<BasicBlock>((WC.BodyToPeelMap)[OriginalHeader])->getInstList().erase(PeelPHI);
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
     */ 
    for (auto Edge : WC.ExitEdges) {

      for (PHINode &PHI : Edge.second->phis()) {

        /*
         * If the exit edge source is the OriginalLatch, the incoming
         * value must be removed. 
         */
        bool NeedToRemoveIncoming = (Edge.first == OriginalLatch);


        /*
         * Analyze the incoming value to the exit basic block PHI
         */
        Value *Incoming = PHI.getIncomingValueForBlock(Edge.first);
        Value *Propagating = Incoming;
        Instruction *IncomingInst = dyn_cast<Instruction>(Incoming);

        if (true
            && IncomingInst
            && (containsInOriginalLoop(WC, IncomingInst->getParent()))) {
          
          /*
           * Fetch the corresponding clone to the incoming value
           * to propagate to the exit edge destination
           */
          Propagating = (WC.BodyToPeelMap)[Incoming];


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
          if (true
              && NeedToRemoveIncoming /* i.e. incoming block is OriginalLatch */
              && (IncomingInst->getParent() != OriginalLatch)) {
            (WC.ExitDependencies)[&PHI] = Incoming;
          }

        }


        /*
         * Add the propagating value and remove the incoming 
         * value if necessary
         */
        PHI.addIncoming(
          Propagating, 
          cast<BasicBlock>((WC.BodyToPeelMap)[Edge.first])
        );

        if (NeedToRemoveIncoming) {
          PHI.removeIncomingValue(Edge.first);
        }

      }
  

    }


    return;

  }


  PHINode * LoopWhilifier::buildNewHeaderDependencyPHI(
    WhilifierContext &WC,
    Value *Dependency
  ) {

    /*
     * Get necessary blocks
     */  
    BasicBlock *Latch = WC.OriginalLatch,
               *NewHeader = cast<BasicBlock>((WC.BodyToPeelMap)[Latch]);


    /*
     * Build the new PHINode
     */ 
    IRBuilder<> NewHeaderBuilder{NewHeader->getFirstNonPHI()};
    PHINode *DependencyPHI = NewHeaderBuilder.CreatePHI(Dependency->getType(), 0);


    /*
     * Populate PHINode --- add incoming values based on the
     * predecessors of both the original and peeled latch
     */ 
    Value *MappedDependency = cast<Value>((WC.BodyToPeelMap)[Dependency]);

    for (auto *PredBB : predecessors(NewHeader)) {
      DependencyPHI->addIncoming(MappedDependency, PredBB);
    }

    for (auto *PredBB : predecessors(Latch)) {
      DependencyPHI->addIncoming(Dependency, PredBB);
    }


    return DependencyPHI;

  }


  void LoopWhilifier::resolveExitEdgeDependencies(
    WhilifierContext &WC,
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

    for (auto const &[PHI, Incoming] : WC.ExitDependencies) {
      PHINode *ExitDependencyPHI = buildNewHeaderDependencyPHI(WC, Incoming);
      PHI->setIncomingValueForBlock(NewHeader, ExitDependencyPHI);
    }


    return;

  }


  void LoopWhilifier::resolveNewHeaderPHIDependencies(
    WhilifierContext &WC
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

    BasicBlock *Latch = WC.OriginalLatch;
    for (PHINode &OriginalPHI : Latch->phis()) {

      PHINode *PeeledPHI = cast<PHINode>((WC.BodyToPeelMap)[&OriginalPHI]);
      for (
        uint32_t PHINo = 0; 
        PHINo < OriginalPHI.getNumIncomingValues(); 
        ++PHINo
      ) {

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
    WhilifierContext &WC
  ) {

    /*
     * TOP --- find all instruction-based dependencies in
     * the original latch block that are NOT defined in the 
     * original latch but ARE defined elsewhere in the 
     * original loop body
     */ 
    BasicBlock *Latch = WC.OriginalLatch;
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
      for (
        uint32_t OpNo = 0; 
        OpNo != I.getNumOperands(); 
          ++OpNo
      ) {

        Value *OP = I.getOperand(OpNo);
        Instruction *Dependence = dyn_cast<Instruction>(OP);
        if (!Dependence) {
          continue;
        }

        BasicBlock *DependenceParent = Dependence->getParent();
        if (true
            && (containsInOriginalLoop(WC, DependenceParent))
            && (DependenceParent != Latch)) {
          ((WC.OriginalLatchDependencies)[Dependence])[&I] = OpNo;
        }

      }

    }


    return;

  }


  void LoopWhilifier::resolveNewHeaderNonPHIDependencies(
    WhilifierContext &WC,
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
    for (auto const &[D, Uses] : WC.OriginalLatchDependencies) {

      /*
       * Build the new PHINode
       */ 
      PHINode *DependencyPHI = buildNewHeaderDependencyPHI(WC, D);


      /*
       * Get corresponding value for the dependency in "peeled" iteration
       */ 
      Value *PeeledDependency = cast<Value>((WC.BodyToPeelMap)[D]);


      /*
       * Replacing uses in instructions that carry the dependencies,
       * corresponding to instructions in the peeled latch 
       */ 
      for (auto const &[I, OpNo] : Uses) {
        Instruction *CorrespondingI = cast<Instruction>((WC.BodyToPeelMap)[I]);
        CorrespondingI->setOperand(OpNo, DependencyPHI);
      }


      /*
       * Save the mapping between D, PeeledDependency and the 
       * DependencyPHI instruction
       */ 
      (WC.ResolvedDependencyMapping)[D] = 
        (WC.ResolvedDependencyMapping)[PeeledDependency] = 
        DependencyPHI;

    }


    return;

  }


  void LoopWhilifier::resolveNewHeaderDependencies(
    WhilifierContext &WC,
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
    resolveNewHeaderPHIDependencies(WC);


    /*
     * Find dependencies in the original latch that are defined
     * elsewhere in the loop --- necessary to build PHINodes for
     * the new header, and fix old header's incoming values
     */  
    findNonPHIOriginalLatchDependencies(WC);


    /*
     * Now build PHINodes for all other dependencies in the new header
     */ 
    resolveNewHeaderNonPHIDependencies(
      WC,
      NewHeader
    );


    return;

  }


  void LoopWhilifier::resolveOriginalHeaderPHIs(
    WhilifierContext &WC
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

    BasicBlock *Header = WC.OriginalHeader,
               *PreHeader = WC.OriginalPreHeader,
               *Latch = WC.OriginalLatch;

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
        if ((WC.ResolvedDependencyMapping).find(Incoming) !=
            (WC.ResolvedDependencyMapping).end()) {
          Incoming = (WC.ResolvedDependencyMapping)[Incoming];
        } 


        /*
         * If the incoming value is defined in the loop body but outside
         * the latch, a new dependency PHINode must be generated to 
         * handle "peeled" incoming values and incoming values from the 
         * rest of the loop body --- set incoming accordingly
         */ 
        else if (
          true
          && (IncomingParent != Latch)
          && (containsInOriginalLoop(WC, IncomingParent))
        ) {
          PHINode *DependencyPHI = buildNewHeaderDependencyPHI(WC, Incoming);
          Incoming = DependencyPHI;
        } 


        /*
         * Otherwise, set incoming to be the corresponding value from the
         * ValueToValueMap --- prevent using the original latch value, since 
         * the original latch will be erased
         */ 
        else {
          Incoming = (WC.BodyToPeelMap)[IncomingInst];
        }

      }

      OriginalPHI.setIncomingValueForBlock(PreHeader, Incoming);
      OriginalPHI.removeIncomingValue(Latch);

    }


    return;

  }


  void LoopWhilifier::rerouteLoopBranches(
    WhilifierContext &WC,
    BasicBlock *NewHeader
  ) {

    /*
     * TOP --- Reroute branches from original latch to the new 
     * header, loop through all predecessors to reroute
     * 
     * Necessary because the original latch will be erased
     */ 

    /*
     * Setup
     */
    BasicBlock *Latch = WC.OriginalLatch;
    SmallVector<BasicBlock *, 32> Preds;
    for (auto *PredBB : predecessors(Latch)) {
      Preds.push_back(PredBB);
    }


    /*
     * Patch [original latch --> new header] for each 
     * predecessor of the original latch, iterate
     */
    for (auto *PredBB : Preds) {

      /*
       * Fetch the branch to modify 
       */
      BranchInst *PredTerm = dyn_cast<BranchInst>(PredBB->getTerminator());
      if (!PredTerm) {
        continue;
      }


      /*
       * Patch each incoming basic block if it's the original latch
       */
      for (
        uint32_t SuccNo = 0;
        SuccNo < PredTerm->getNumSuccessors();
        ++SuccNo
      ) {
        if (PredTerm->getSuccessor(SuccNo) == Latch) {
          PredTerm->setSuccessor(SuccNo, NewHeader);
        }
      }

    }


    return;

  }


  WhilifierContext::WhilifierContext(
    LoopStructure * const LS
  ) :
    OriginalHeader{LS->getHeader()},
    OriginalPreHeader{LS->getPreHeader()},
    F{LS->getFunction()} {

    /*
     * Record latch info
     */
    auto Latches = LS->getLatches();
    OriginalLatch = *(Latches.begin());
    NumLatches = Latches.size();


    /*
     * Record exit edges
     */
    auto AllExitEdges = LS->getLoopExitEdges();
    for (auto EE : AllExitEdges) {
      ExitEdges.push_back(EE);
    }


    /*
     * Record loop blocks
     */
    for (auto NextBB : LS->getBasicBlocks()){
      LoopBlocks.push_back(NextBB);
    }

  }


  void WhilifierContext::Dump()
  {

    errs() << "WhilifierContext: Current Context\n";

    /*
     * Loop body info
     */ 
    if (OriginalHeader) {
      errs() << "WhilifierContext:   OriginalHeader " << *(OriginalHeader) << "\n";
    }

    if (OriginalLatch && !(ResolvedLatch)) {
      errs() << "WhilifierContext:   OriginalLatch " << *(OriginalLatch) << "\n";
    }

    if (OriginalPreHeader) {
      errs() << "WhilifierContext:   OriginalPreHeader " << *(OriginalPreHeader) << "\n";
    }

    errs() << "WhilifierContext:   ExitEdges:\n";
    for (auto EE : ExitEdges) {
      errs() << "---\nWhilifierContext:     From: " << *(EE.first) << "\n"
        << "WhilifierContext:     To: " << *(EE.second) << "\n---\n";
    }

    errs() << "---\nWhilifierContext:   LoopBlocks:\n";
    for (auto BB : LoopBlocks) {
      errs() << *BB << "\n";
    }
    errs() << "---\n";

    errs() << "WhilifierContext:   Current Function:\n" << *F << "\n";


    /*
     * Whilification info
     */ 
    errs() << "WhilifierContext:   IsDoWhile: " << std::to_string(IsDoWhile) << "\n"
           << "WhilifierContext:   IsAppropriateToWhilify: " << std::to_string(IsAppropriateToWhilify) << "\n"
           << "WhilifierContext:   IsSingleBlockLoop: " << std::to_string(IsSingleBlockLoop) << "\n"
           << "WhilifierContext:   ConsolidatedOriginalLatch: " << std::to_string(ConsolidatedOriginalLatch) << "\n";


    return;


  }

}
