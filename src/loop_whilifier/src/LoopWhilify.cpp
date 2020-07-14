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
 
LoopWhilifier::LoopWhilifier(Noelle &noelle)
  : noelle{noelle}
  {
  return ;
}

bool LoopWhilifier::whilifyLoop (
  LoopDependenceInfo const &LDI
) {

  auto Transformed = false;

  /*
   * Check if the loop can be whilified
   */ 
  BasicBlock *Header = nullptr,
             *Latch = nullptr,
             *PreHeader = nullptr;

  std::vector<std::pair<BasicBlock *, BasicBlock *>> ExitEdges;
  
  LoopStructure *LS = LDI.getLoopStructure();

  if (!(this->canWhilify(
                LS, 
                Header, 
                Latch, 
                PreHeader, 
                ExitEdges
              ))) { 
    return Transformed; 
  }


  /*
   * Acquire all other necessary info to whilify
   */ 
  std::vector<BasicBlock *> LoopBlocks;
  for (auto NextBB : LS->orderedBBs) {
    LoopBlocks.push_back(NextBB);
  }

  Function *F = Header->getParent();


  /*
   * If the loop is a single block, perform necessary transforms
   * for whilifying the loop --- use collected data structures
   */ 
  if (Header == Latch) { 
    this->transformSingleBlockLoop(
      Header, 
      Latch, 
      ExitEdges, 
      LoopBlocks
    ); 
  }


  /*
   * Split into anchors and new preheader, name anchors and new 
   * preheader, update preheader pointer
   */
  BasicBlock *InsertTop = nullptr,
             *InsertBot = nullptr;
  
  this->buildAnchors(
    Header, 
    PreHeader, 
    InsertTop, 
    InsertBot
  );


  /*
   * Clone loop blocks and remap instructions
   */ 
  SmallVector<BasicBlock *, 16> NewBlocks;
  ValueToValueMapTy BodyToPeelMap;

  this->cloneLoopBlocksForWhilifying(
    InsertTop, 
    InsertBot, 
    Header, 
    Latch, 
    PreHeader,
    F,
    LoopBlocks, 
    ExitEdges, 
    NewBlocks, 
    BodyToPeelMap
  );
  
  llvm::remapInstructionsInBlocks(
    NewBlocks, 
    BodyToPeelMap
  );


  /*
   * Introduce another block to anchor the "peeled" 
   * iteration --- optional here
   */ 
  InsertTop = InsertBot;

#if EXTRA_ANCHOR
  InsertBot = SplitBlock(InsertBot, InsertBot->getTerminator());
  InsertBot->setName(Header->getName() + ".whilify.next");
#endif


  /*
   * Fix block placement --- set the peeled iteration before  
   * the loop body itself --- also optional
   */  
#if FIX_BLOCK_PLACEMENT
  F->getBasicBlockList().splice(InsertTop->getIterator(),
                                F->getBasicBlockList(),
                                NewBlocks[0]->getIterator(), F->end());
#endif


  /*
   * Find dependencies in the original latch that are defined
   * elsewhere in the loop --- necessary to build PHINodes for
   * the new header, and fix old header's incoming values
   */  
  DenseMap<Value *, Value *> ResolvedDependencyMapping;

  /*
   * *** NOTE *** --- the "peeled" latch (the mapping from the
   * original latch to the "peeled" iteration) == NEW HEADER
   */ 
  BasicBlock *PeeledLatch = cast<BasicBlock>(BodyToPeelMap[Latch]); // show
             *NewHeader = PeeledLatch;

  this->resolveNewHeaderDependencies(
    Latch, 
    NewHeader, 
    LoopBlocks,
    BodyToPeelMap,
    ResolvedDependencyMapping
  );
    

  /*
   * Resolve old header PHINodes --- remove references to the
   * old latch, update any incoming values with new header
   * PHINodes whenever possible
   */ 
  this->resolveOriginalHeaderPHIs(
    Header,
    PreHeader,
    Latch,
    BodyToPeelMap,
    ResolvedDependencyMapping
  );


  /*
   * Fix branches in loop body --- go to the new header instead of 
   * the old latch, target predecessors of old latch
   */ 
  this->rerouteLoopBranches(
    Latch,
    NewHeader
  );


  /*
   * Erase old latch
   */ 
  Latch->eraseFromParent();
  Transformed |= true;


  return Transformed;

}


bool LoopWhilifier::isDoWhile(
  LoopStructure * const LS,
  BasicBlock * const Latch
) {

  bool isDoWhile = false;

  /*
   * TOP --- If any of the successors of the latch are not part 
   * of the loop --- the latch must be an intra-loop edge to an 
   * exit block
   * 
   * The loop is a do-while loop as long as the latch is loop-
   * exiting
   */  

  for (auto *SuccBB : successors(Latch)) {
    
    if (!(LS->isIncluded(SuccBB))) {
      isDoWhile |= true;
      return isDoWhile;
    }

  }

  return isDoWhile;

}


bool LoopWhilifier::canWhilify (
  LoopStructure * const LS,
  BasicBlock *&Header,
  BasicBlock *&Latch,
  BasicBlock *&PreHeader,
  std::vector<std::pair<BasicBlock *, BasicBlock *>> &ExitEdges
) {

  bool canWhilify = true;

  /*
   * TOP --- Require valid header, preheader, and single latch, 
   * then check if the loop is in do-while form
   */ 


  /*
   * Acquire header
   */ 
  Header = LS->getHeader();
  canWhilify &= !!Header;


  /*
   * Acquire latch
   */ 
  auto Latches = LS->getLatches();
  Latch = *(Latches.begin());
  canWhilify &= (Latches.size() == 1);


  /*
   * Acquire preheader
   */ 
  PreHeader = LS->getPreHeader();
  canWhilify &= !!PreHeader;


  /*
   * Acquire exits
   */ 
  auto AllExitEdges = LS->getLoopExitEdges();
  for (auto EE : AllExitEdges) {
    ExitEdges.push_back(EE);
  }
  canWhilify &= (AllExitEdges.size() > 0);


  /*
   * Check if loop is in do-while form
   */ 
  if (canWhilify) {
    canWhilify &= this->isDoWhile(LS, Latch);
  }


  return canWhilify;

}


void LoopWhilifier::transformSingleBlockLoop(
  BasicBlock *&Header,
  BasicBlock *&Latch,
  std::vector<std::pair<BasicBlock *, BasicBlock *>> &ExitEdges,
  SmallVector<BasicBlock *, 16> &LoopBlocks
) {

  /*
   * Split the header at the terminator --- new block will
   * be the new latch
   */ 
  Instruction *SplitPoint = Header->getTerminator();
  BasicBlock *NewLatch = SplitBlock(Header, SplitPoint);
  NewLatch->setName(".new.latch");


  /*
   * Update latch, loop blocks, and exit blocks
   */ 
  Latch = NewLatch;

  LoopBlocks.push_back(Latch);

  for (auto Edge : ExitEdges) {

    if (Edge.first == Header) {
      Edge.first = Latch;
    }

  }

  return;

}


void LoopWhilifier::buildAnchors(
  BasicBlock *Header,
  BasicBlock *&PreHeader,
  BasicBlock *&InsertTop,
  BasicBlock *&InsertBot
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

  InsertTop = SplitEdge(PreHeader, Header);
  InsertBot = SplitBlock(InsertTop, InsertTop->getTerminator());
  BasicBlock *NewPreHeader = SplitBlock(InsertBot, InsertBot->getTerminator());


  /*
   * Naming
   */ 
  InsertTop->setName(Header->getName() + ".whilify.top.anchor");
  InsertBot->setName(Header->getName() + ".whilify.bottom.anchor");
  NewPreHeader->setName(PreHeader->getName() + ".whilify.ph.old.loop");


  /*
   * Update old loop's preheader
   */ 
  PreHeader = NewPreHeader;


  return;

}


bool containsInOriginalLoop(
  BasicBlock * const BB, 
  std::vector<BasicBlock *> const &OriginalLoopBlocks
) {

  /*
   * TOP --- Perform a simple find through the
   * blocks to perform "contains" --- can't rely
   * on the loop structure, as that info is invalid
   * during whilification
   */

  return std::find(
    OriginalLoopBlocks.begin(), 
    OriginalLoopBlocks.end(),
    BB) != OriginalLoopBlocks.end();

}


/*
 * Based on LoopUnrollPeel.cpp : cloneLoopBlocks
 */
void LoopWhilifier::cloneLoopBlocksForWhilifying(
  BasicBlock *InsertTop, 
  BasicBlock *InsertBot,
  BasicBlock *OriginalHeader,
  BasicBlock *OriginalLatch,
  BasicBlock *OriginalPreHeader,
  Function *F,
  std::vector<BasicBlock *> &LoopBlocks,
  std::vector<std::pair<BasicBlock *, BasicBlock *>> &ExitEdges,
  SmallVectorImpl<BasicBlock *> &NewBlocks, 
  ValueToValueMapTy &BodyToPeelMap
) {

  /*
   * For each block in the original loop, create a new copy,
   * and update the value map with the newly created values.
   */
  for (auto OrigBB : LoopBlocks) {
  
    BasicBlock *PeelBB = CloneBasicBlock(
      OrigBB, 
      BodyToPeelMap, 
      ".whilify", 
      F
    );

    NewBlocks.push_back(PeelBB);
    BodyToPeelMap[OrigBB] = PeelBB;

  }


  /*
   * Fix control flow --- The top anchor must branch unconditionally
   * to the "peeled header" --- pulled from the ValueToValueMap
   */ 
  InsertTop->getTerminator()->setSuccessor(
    0, cast<BasicBlock>(BodyToPeelMap[OriginalHeader])
  );


  /*
   * Fix control flow --- The bottom anchor must receive the "peeled" 
   * latch, which currently goes straight to the original header and 
   * to the exit block --- this must be rerouted to the bottom 
   * anchor instead
   */ 
  BasicBlock *PeelLatch = cast<BasicBlock>(BodyToPeelMap[OriginalLatch]);
  BranchInst *PeelLatchBR = cast<BranchInst>(PeelLatch->getTerminator());

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

    PHINode *PeelPHI = cast<PHINode>(BodyToPeelMap[&PHI]);
    BodyToPeelMap[&PHI] = PeelPHI->getIncomingValueForBlock(OriginalPreHeader);
    cast<BasicBlock>(BodyToPeelMap[OriginalHeader])->getInstList().erase(PeelPHI);
  
  }


  /*
   * Resolve exit edges --- the outgoing values must be fixed so
   * the "peeled" latch will replace the original latch as an
   * incoming block to the destination in the edge
   * 
   * The incoming value depends on if the value was defined
   * in the loop body --- if so, we must propagate the corresponding
   * value from the "peeled" block
   */ 
  for (auto Edge : ExitEdges) {

    for (PHINode &PHI : Edge.second->phis()) {

      Value *LatchVal = PHI.getIncomingValueForBlock(Edge.first);
      Instruction *LatchInst = dyn_cast<Instruction>(LatchVal);

      if (LatchInst 
          && (this->containsInOriginalLoop(
                LatchInst->getParent(), 
                LoopBlocks
              ))) {
        LatchVal = BodyToPeelMap[LatchVal];
      }
      
      /* 
       * Add incoming for the "peeled latch" --- will become new header
       */ 
      PHI.addIncoming(LatchVal, cast<BasicBlock>(BodyToPeelMap[Edge.first]));

      /* 
       * Remove incoming for old latch
       */ 
      PHI.removeIncomingValue(Edge.first);

    }

  }


  return;
}



void LoopWhilifier::resolveNewHeaderPHIDependencies(
  BasicBlock * const Latch, 
  ValueToValueMap &BodyToPeelMap
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

  for (PHINode &OriginalPHI : Latch->phis()) {

    PHINode *PeeledPHI = cast<PHINode>(BodyToPeelMap[&OriginalPHI]);

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
  BasicBlock *Latch,
  std::vector<BasicBlock *> &LoopBlocks,
  DenseMap<Instruction *, 
           DenseMap<Instruction *, 
           uint32_t>> &DependenciesInLoop
) {

  /*
   * TOP --- find all instruction-based dependencies in
   * the original latch block that are NOT defined in the 
   * original latch but ARE defined elsewhere in the 
   * original loop body
   */ 
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

      if ((this->containsInOriginalLoop((DependenceParent, LoopBlocks)))
          && (DependenceParent != Latch)) {
        (DependenciesInLoop[Dependence])[&I] = OpNo;
      }

    }
  }

  return;
  
}

void LoopWhilifier::resolveNewHeaderNonPHIDependencies(
  BasicBlock *Latch,
  BasicBlock *NewHeader,
  ValueToValueMap &BodyToPeelMap,
  DenseMap<Value *, Value *> &ResolvedDependencyMapping,
  DenseMap<Instruction *, 
           DenseMap<Instruction *, 
                    uint32_t>> &OriginalLatchDependencies
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

  IRBuilder<> PHIBuilder{NewHeader->getFirstNonPHI()};

  for (auto const &[D, Uses] : OriginalLatchDependencies) {
    
    /*
     * Build the new PHINode
     */ 
    PHINode *DependencyPHI = PHIBuilder.CreatePHI(D->getType(), 0);


    /*
     * Populate PHINode --- add incoming values based on the
     * predecessors of both the original and peeled latch
     */ 
    Instruction *PeeledDependency = cast<Instruction>(BodyToPeelMap[D]);
    for (auto *PredBB : predecessors(NewHeader)) {
      DependencyPHI->addIncoming(PeeledDependency, PredBB);
    }

    for (auto *PredBB : predecessors(Latch)) {
      DependencyPHI->addIncoming(D, PredBB);
    }


    /*
     * Replacing uses in instructions that carry the dependencies,
     * corresponding to instructions in the peeled latch 
     */ 
    for (auto const &[I, OpNo] : Uses) {
      Instruction *CorrespondingI = cast<Instruction>(BodyToPeelMap[I]);
      CorrespondingI->setOperand(OpNo, DependencyPHI);
    }

    
    /*
     * Save the mapping between D, PeeledDependency and the 
     * DependencyPHI instruction
     */ 
    ResolvedDependencyMapping[D] = 
      ResolvedDependencyMapping[PeeledDependency] = 
        DependencyPHI;

  }

  return;

}

void LoopWhilifier::resolveNewHeaderDependencies(
  BasicBlock *Latch,
  BasicBlock *NewHeader,
  std::vector<BasicBlock *> &LoopBlocks,
  ValueToValueMap &BodyToPeelMap,
  DenseMap<Value *, Value *> &ResolvedDependencyMapping
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
  this->resolveNewHeaderPHIDependencies(
    Latch, 
    BodyToPeelMap
  );


  /*
   * Find dependencies in the original latch that are defined
   * elsewhere in the loop --- necessary to build PHINodes for
   * the new header, and fix old header's incoming values
   */  
  DenseMap<Instruction *, 
           DenseMap<Instruction *, 
                    uint32_t>> OriginalLatchDependencies;
  
  this->findNonPHIOriginalLatchDependencies( 
    Latch, 
    LoopBlocks,
    OriginalLatchDependencies
  );


  /*
   * Now build PHINodes for all other dependencies in the new header
   */ 
  this->resolveNewHeaderNonPHIDependencies(
    Latch, 
    NewHeader, 
    BodyToPeelMap,
    ResolvedDependencyMapping, 
    OriginalLatchDependencies
  );


  return;

}


void LoopWhilifier::resolveOriginalHeaderPHIs(
  BasicBlock *Header,
  BasicBlock *PreHeader,
  BasicBlock *Latch,
  ValueToValueMap &BodyToPeelMap,
  DenseMap<Value *, Value *> &ResolvedDependencyMapping
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

  for (PHINode &OriginalPHI : Header->phis()) {

    Value *Incoming = OriginalPHI.getIncomingValueForBlock(Latch);
    Instruction *IncomingInst = dyn_cast<Instruction>(Incoming);

    if (IncomingInst) {
      Value *Resolved = ResolvedDependencyMapping[Incoming];
      Incoming = (Resolved) ? 
                 (Resolved) : 
                 cast<Value>(BodyToPeelMap[IncomingInst]);
    }

    OriginalPHI.setIncomingValueForBlock(PreHeader, Incoming);
    OriginalPHI.removeIncomingValue(Latch);

  }

  return;

}

void LoopWhilifier::rerouteLoopBranches(
  BasicBlock *Latch,
  BasicBlock *NewHeader
) {

  /*
   * TOP --- Reroute branches from original latch to the new 
   * header, loop through all predecessors to reroute
   * 
   * Necessary because the original latch will be erased
   */ 

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



