/*
 * Copyright 2019 - 2020  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "Mem2RegNonAlloca.hpp"

using namespace llvm;
using namespace llvm::noelle;

Mem2RegNonAlloca::Mem2RegNonAlloca (LoopDependenceInfo const &LDI, Noelle &noelle)
  : LDI{LDI}, invariants{*LDI.getInvariantManager()}, noelle{noelle} {
}

bool Mem2RegNonAlloca::promoteMemoryToRegister () {

  auto loopStructure = LDI.getLoopStructure();
  if (noelle.getVerbosity() >= Verbosity::Maximal) {
    auto terminator = loopStructure->getHeader()->getTerminator();
    if (!terminator) return false;
    terminator->print(errs() << "Mem2Reg: Checking loop: "); errs() << "\n";
  }

  /*
   * Ensure the function does not return from within the loop
   */
  for (auto B : loopStructure->getBasicBlocks()) {
    auto terminator = B->getTerminator();
    if (!terminator) {
      return false;
    }
    if (isa<ReturnInst>(terminator)) {
      return false;
    }
    if (isa<InvokeInst>(terminator)){
      return false;
    }
  }

  auto singleMemoryLocationsBySCC = findSCCsWithSingleMemoryLocations();
  for (auto memoryAndSCCPair : singleMemoryLocationsBySCC) {
    auto memoryInst = memoryAndSCCPair.first;
    auto memorySCC = memoryAndSCCPair.second;

    if (noelle.getVerbosity() >= Verbosity::Maximal) {
      memoryInst->print(errs() << "Mem2Reg:  Loop invariant memory location: "); errs() << "\n";
      memorySCC->printMinimal(errs() << "Mem2Reg:  SCC:\n"); errs() << "\n";
    }

    // if (hoistMemoryInstructionsRelyingOnExistingRegisterValues(memorySCC, memoryInst)) {
    //   modified = true;
    //   continue;
    // }

    bool promoted = promoteMemoryToRegisterForSCC(memorySCC, memoryInst);
    if (noelle.getVerbosity() >= Verbosity::Maximal) {
      memoryInst->print(errs() << "Mem2Reg:  Loop invariant memory location loads/stores promoted: " << promoted << " ");
      errs() << "\n";
    }
    if (promoted) return true;
  }

  return false;
}

std::unordered_map<Value *, SCC *> Mem2RegNonAlloca::findSCCsWithSingleMemoryLocations (void) {

  /*
   * Identify SCC containing only loads/stores on a single memory location
   * along with any computation that does NOT alias the loads/stores
   */
  auto loopStructure = LDI.getLoopStructure();
  auto sccdag = LDI.sccdagAttrs.getSCCDAG();
  std::unordered_map<Value *, SCC *> singleMemoryLocationsBySCC{};
  for (auto sccNode : sccdag->getNodes()) {
    auto scc = sccNode->getT();
    auto sccInfo = LDI.sccdagAttrs.getSCCAttrs(scc);

    // scc->printMinimal(errs() << "SCC: \n"); errs() << "\n";
    // for (auto edge : scc->getEdges()) {
    //   auto value = edge->getOutgoingT();
    //   // if (isa<GetElementPtrInst>(value) || isa<StoreInst>(value) || isa<LoadInst>(value)) {
    //   //   edge->print(errs() << "Edge:\n"); errs() << "\n";
    //   // }
    // }

    bool isSingleMemoryLocation = false;
    Value *memoryLocation = nullptr;
    for (auto nodePair : scc->internalNodePairs()) {
      auto value = nodePair.first;

      /*
       * TODO: Expand understanding of instructions that won't interfere
       */
      if (isa<BinaryOperator>(value)
        || isa<CmpInst>(value)
        || isa<BranchInst>(value)
        || isa<SelectInst>(value)
        || isa<CastInst>(value)
        || isa<PHINode>(value)) continue;

      Value *loadOrStoreLocation = nullptr;
      if (auto load = dyn_cast<LoadInst>(value)) {
        loadOrStoreLocation = load->getPointerOperand();
      } else if (auto store = dyn_cast<StoreInst>(value)) {
        loadOrStoreLocation = store->getPointerOperand();
      }

      if (loadOrStoreLocation) {
        if (!memoryLocation || loadOrStoreLocation == memoryLocation) {
          isSingleMemoryLocation = true;
          memoryLocation = loadOrStoreLocation;
          continue;
        }
      }

      isSingleMemoryLocation = false;
      break;
    }

    /*
     * Memory location access must be the same across the loads/stores, and loop invariant
     */
    if (!isSingleMemoryLocation) continue;

    /*
     * Ensure no memory aliases with any SCC externals
     */
    bool hasExternalMemoryDependence = false;
    for (auto nodePair : scc->internalNodePairs()) {
      auto node = nodePair.second;

      for (auto edge : node->getAllConnectedEdges()) {
        auto producer = edge->getOutgoingT();
        auto consumer = edge->getIncomingT();
        if (scc->isInternal(consumer) && scc->isInternal(producer)) continue;
        if (!edge->isMemoryDependence()) continue;

        hasExternalMemoryDependence = true;
        break;
      }

      if (hasExternalMemoryDependence) break;
    }
    if (hasExternalMemoryDependence) continue;

    if (noelle.getVerbosity() >= Verbosity::Maximal) {
      memoryLocation->print(errs() << "Mem2Reg:  Possible loop invariant memory location: "); errs() << "\n";
    }

    if (auto memoryInst = dyn_cast<Instruction>(memoryLocation)) {
      if (loopStructure->isIncluded(memoryInst)
        && !invariants.isLoopInvariant(memoryInst)) continue;
    }

    singleMemoryLocationsBySCC.insert(std::make_pair(memoryLocation, scc));
  }

  return singleMemoryLocationsBySCC;
}

bool Mem2RegNonAlloca::promoteMemoryToRegisterForSCC (SCC *scc, Value *memoryLocation) {

  auto orderedMemoryInstsByBlock = collectOrderedMemoryInstsByBlock(scc);

  /*
   * Traverse loop blocks, creating PHIs to track the latest value to-be-stored
   * and replacing uses of the loads with the latest value at that point
   */
  std::queue<BasicBlock *> queue;
  std::unordered_set<BasicBlock *> visited;
  std::unordered_map<BasicBlock *, Value *> lastRegisterValueByBlock;

  /*
   * Initialize traversal
   */
  auto loopStructure = LDI.getLoopStructure();
  auto loopHeader = loopStructure->getHeader();
  auto loopPreHeader = loopStructure->getPreHeader();
  queue.push(loopHeader);
  visited.insert(loopHeader);

  /*
   * Register load in pre-header
   */
  IRBuilder<> preHeaderBuilder(loopPreHeader->getTerminator());
  auto initialLoad = preHeaderBuilder.CreateLoad(memoryLocation);
  lastRegisterValueByBlock.insert(std::make_pair(loopPreHeader, initialLoad));

  /*
   * This list will track placeholder PHIs at all entries where predecessors weren't visited first
   * Their incoming values will be determined after the end of the entire traversal
   * Also track every phi for pruning afterwards
   */ 
  std::unordered_set<PHINode *> placeholderPHIs{};
  std::unordered_set<PHINode *> allPHIs{};

  if (noelle.getVerbosity() >= Verbosity::Maximal) {
    errs() << "Mem2Reg: Iterating basic blocks to determine last stored values\n";
  }

  while (!queue.empty()) {
    auto B = queue.front();
    queue.pop();

    /*
     * If this block as well as 1+ predecessors do not have a last register value,
     * add a placeholder PHI. It's incoming values will be determined after the traversal
     */
    if (lastRegisterValueByBlock.find(B) == lastRegisterValueByBlock.end()) {

      bool pushOffExecutionUntilPredecessorsAreTraversed = false;
      for (auto predBlock : predecessors(B)) {
        if (lastRegisterValueByBlock.find(predBlock) != lastRegisterValueByBlock.end()) continue;

        if (noelle.getVerbosity() >= Verbosity::Maximal) {
          B->printAsOperand(errs() << "Mem2Reg: placeholder PHI required: "); errs() << "\n";
        }

        IRBuilder<> builder(B->getFirstNonPHIOrDbgOrLifetime());
        int32_t numPreds = pred_size(B);
        auto phi = builder.CreatePHI(initialLoad->getType(), numPreds);
        lastRegisterValueByBlock.insert(std::make_pair(B, phi));
        placeholderPHIs.insert(phi);
        allPHIs.insert(phi);

        break;
      }
    }

    if (noelle.getVerbosity() >= Verbosity::Maximal) {
      B->printAsOperand(errs() << "Mem2Reg:  checking for last value entering block: "); errs() << "\n";
    }

    /*
     * Fetch the current register value that would be in the memory location
     * If the last register value for the block is ALREADY set, it will be the loop/sub-loop entry
     * 
     * For all other blocks, their already-traversed predecessors will have last register values
     * If there is more than 1 predecessor, create a PHI to collect those predecessor's last values
     */
    Value *lastValue = nullptr;
    if (lastRegisterValueByBlock.find(B) != lastRegisterValueByBlock.end()) {
      lastValue = lastRegisterValueByBlock.at(B);
    } else {

      auto singlePredBlock = B->getSinglePredecessor();
      if (singlePredBlock) {
        assertAndDumpLogsOnFailure(
          [&]() -> bool { return lastRegisterValueByBlock.find(singlePredBlock) != lastRegisterValueByBlock.end(); },
          "Mem2Reg: can't identify last value of the single predecessor to the block");
        lastValue = lastRegisterValueByBlock.at(singlePredBlock);
      } else {

        IRBuilder<> builder(B->getFirstNonPHIOrDbgOrLifetime());
        auto numPreds = pred_size(B);
        auto phi = builder.CreatePHI(initialLoad->getType(), numPreds);
        allPHIs.insert(phi);
        lastValue = phi;

        for (auto predBlock : predecessors(B)) {
          assertAndDumpLogsOnFailure(
            [&]() -> bool { return lastRegisterValueByBlock.find(predBlock) != lastRegisterValueByBlock.end(); },
            "Mem2Reg: can't identify last value of one of the predecessors to the block");
          auto predV = lastRegisterValueByBlock.at(predBlock);
          phi->addIncoming(predV, predBlock);
        }
      }

      lastRegisterValueByBlock.insert_or_assign(B, lastValue);
    }

    if (noelle.getVerbosity() >= Verbosity::Maximal) {
      B->printAsOperand(errs() << "Mem2Reg:  Last value entering block: "); errs() << "\t";
      lastValue->print(errs()); errs() << "\n";
    }

    /*
     * Traverse to successors of current block that are within the loop
     */
    for (auto succBlock : successors(B)) {
      if (!loopStructure->isIncluded(succBlock)) continue;
      if (visited.find(succBlock) != visited.end()) continue;
      queue.push(succBlock);
      visited.insert(succBlock);
    }

    /*
     * For each load in the block, replace it with the current register value
     * For each store in the block, update the current register value
     */
    if (orderedMemoryInstsByBlock.find(B) == orderedMemoryInstsByBlock.end()) continue;
    for (auto memInst : orderedMemoryInstsByBlock.at(B)) {
      if (auto loadInst = dyn_cast<LoadInst>(memInst)) {

        /*
         * NOTE: We cannot replace users as we traverse the users list, so we cache users of the load
         */
        std::unordered_set<User *> usersOfLoad{loadInst->user_begin(), loadInst->user_end()};
        for (auto user : usersOfLoad) {
          user->replaceUsesOfWith(loadInst, lastValue);
        }

      } else if (auto storeInst = dyn_cast<StoreInst>(memInst)) {
        lastValue = storeInst->getValueOperand();

        if (noelle.getVerbosity() >= Verbosity::Maximal) {
          lastValue->print(errs() << "Mem2Reg:  Value updated: "); errs() << "\n";
        }

      } else assert(false && "Mem2Reg: corrupt internal memory instruction map data structure");
    }
    lastRegisterValueByBlock.insert_or_assign(B, lastValue);

  }

  /*
   * For each placeholder, wire up the PHI with the last register values from all predecessors
   */ 
  for (auto phi : placeholderPHIs) {
    auto phiBlock = phi->getParent();
    for (auto predBlock : predecessors(phiBlock)) {
      assertAndDumpLogsOnFailure(
        [&]() -> bool { return lastRegisterValueByBlock.find(predBlock) != lastRegisterValueByBlock.end(); },
        "Mem2Reg: can't identify last value of predecessor to placeholder PHI block");
      auto prevValue = lastRegisterValueByBlock.at(predBlock);

      /*
       * To prevent a PHI from referencing itself, add an intermediate in the predecessor block
       * that references the PHI, and then use that intermediate
       * 
       * TODO: Determine if we need to handle the case where the block is its own predecessor differently
       */
      if (prevValue == phi) {
        auto numPreds = pred_size(predBlock);
        IRBuilder<> builder(predBlock->getFirstNonPHIOrDbgOrLifetime());
        auto intermediatePHI = builder.CreatePHI(phi->getType(), numPreds);
        for (auto latchPredBlock : predecessors(predBlock)) {
          intermediatePHI->addIncoming(phi, latchPredBlock);
        }
        prevValue = intermediatePHI;
        
        allPHIs.insert(intermediatePHI);
        lastRegisterValueByBlock.insert_or_assign(predBlock, intermediatePHI);
      }

      phi->addIncoming(prevValue, predBlock);
    }
  }

  /*
   * Store the last register value for the memory location at each loop exit
   * This may require creating a PHI at the loop exit
   */
  for (auto exitBlock : loopStructure->getLoopExitBasicBlocks()) {
    IRBuilder<> exitBuilder(exitBlock->getFirstNonPHIOrDbgOrLifetime());

    Value *lastValue = nullptr;
    auto singlePredBlock = exitBlock->getSinglePredecessor();
    if (singlePredBlock) {
      assertAndDumpLogsOnFailure(
        [&]() -> bool { return lastRegisterValueByBlock.find(singlePredBlock) != lastRegisterValueByBlock.end(); },
        "Mem2Reg: can't identify last value of predecessor to loop exit block");
      lastValue = lastRegisterValueByBlock.at(singlePredBlock);
    } else {

      auto numPreds = pred_size(exitBlock);
      auto exitPHI = exitBuilder.CreatePHI(initialLoad->getType(), numPreds);
      for (auto exitPredBlock : predecessors(exitBlock)) {
        assertAndDumpLogsOnFailure(
          [&]() -> bool { return lastRegisterValueByBlock.find(exitPredBlock) != lastRegisterValueByBlock.end(); },
          "Mem2Reg: can't identify last value of predecessor to loop exit block");
        auto exitPredValue = lastRegisterValueByBlock.at(exitPredBlock);
        exitPHI->addIncoming(exitPredValue, exitPredBlock);
      }
      lastValue = exitPHI;
    }

    exitBuilder.CreateStore(lastValue, memoryLocation);
  }

  /*
   * Delete stores and loads
   */
  for (auto blockAndInsts : orderedMemoryInstsByBlock) {
    auto insts = blockAndInsts.second;
    for (auto I : insts) {

      if (noelle.getVerbosity() >= Verbosity::Maximal) {
        I->print(errs() << "Mem2Reg:  Removing\n"); errs() << "\n";
      }

      assert(I->user_empty() && "Mem2Reg: Removing instruction but failed to replace all its uses");
      I->eraseFromParent();
    }
  }

  /*
   * Primary goal: prevent any extra PHI loop carried dependencies that already exist from being re-stated
   */
  removeRedundantPHIs(allPHIs, lastRegisterValueByBlock);

  return true;
}

bool Mem2RegNonAlloca::hoistMemoryInstructionsRelyingOnExistingRegisterValues (SCC *scc, Value *memoryLocation) {

  auto orderedMemoryInstsByBlock = collectOrderedMemoryInstsByBlock(scc);

  auto loopStructure = LDI.getLoopStructure();
  auto loopHeader = loopStructure->getHeader();

  // Build a list of basic blocks that collect 2+ unique stored values (store merging blocks)
  std::unordered_map<BasicBlock *, StoreInst *> blockToLastStoreMap;
  std::queue<BasicBlock *> blocksToTraverse;
  std::unordered_set<BasicBlock *> blocksMergingStores;

  for (auto memoryInstsByBlock : orderedMemoryInstsByBlock) {
    auto block = memoryInstsByBlock.first;
    auto memoryInsts = memoryInstsByBlock.second;

    StoreInst *lastStore = nullptr;
    for (auto instIter = memoryInsts.rbegin(); instIter != memoryInsts.rend(); --instIter) {
      auto inst = *instIter;
      if (auto store = dyn_cast<StoreInst>(inst)) {
        lastStore = store;
        break;
      }
    }

    if (!lastStore) continue;
    blockToLastStoreMap.insert(std::make_pair(block, lastStore));
    blocksToTraverse.push(block);
  }

  while (!blocksToTraverse.empty()) {
    auto block = blocksToTraverse.front();
    blocksToTraverse.pop();

    for (auto succBlock : successors(block)) {
      if (pred_size(succBlock) > 1) {
        blocksMergingStores.insert(succBlock);
        continue;
      }

      /*
       * This check is needed, even if the only way a block points to itself in our traversal
       * is if our traversal started in that block
       */
      if (succBlock == block) continue;
      blocksToTraverse.push(succBlock);
    }
  }

  // Locate candidate SCCs that have single header PHIs and consume stored values
  // TODO:

  // Filter candidates:
  // The header PHI's pre-header incoming value must be the initial value at the memory location
  // PHIs must exist at all store merging blocks and propagate all last-stored values
  // Only and all last-stored values and store merging PHIs must propagate to the header

  return false;
}

void Mem2RegNonAlloca::removeRedundantPHIs (
  std::unordered_set<PHINode *> phis,
  std::unordered_map<BasicBlock *, Value *> lastRegisterValueByBlock
) {

  /*
   * For each PHI, determine if all incoming values are the same.
   * If so, replace this PHI's uses with that incoming value
   */
  // TODO:

}

std::unordered_map<BasicBlock *, std::vector<Instruction *>> Mem2RegNonAlloca::collectOrderedMemoryInstsByBlock (SCC *scc) {

  if (noelle.getVerbosity() >= Verbosity::Maximal) {
    errs() << "Mem2Reg:  Collecting and ordering memory loads/stores by basic block\n";
  }

  /*
   * Index memory values by their basic block
   */
  std::unordered_map<BasicBlock *, std::unordered_set<Instruction *>> memoryInstsByBlock;
  for (auto nodePair : scc->internalNodePairs()) {
    auto value = nodePair.first;
    Instruction *memoryInst = nullptr;
    if (isa<LoadInst>(value) || isa<StoreInst>(value)) {
      memoryInst = cast<Instruction>(value);
    }
    if (!memoryInst) continue;

    auto B = memoryInst->getParent();
    if (memoryInstsByBlock.find(B) == memoryInstsByBlock.end()) {
      std::unordered_set<Instruction *> memoryInsts = { memoryInst };
      memoryInstsByBlock.insert(std::make_pair(B, memoryInsts));
    } else {
      auto &memoryInsts = memoryInstsByBlock.at(B);
      memoryInsts.insert(memoryInst);
    }
  }

  /*
   * Sort memory values in execution order for each basic block
   */
  std::unordered_map<BasicBlock *, std::vector<Instruction *>> orderedMemoryInstsByBlock;
  for (auto blockAndInstsPair : memoryInstsByBlock) {
    auto B = blockAndInstsPair.first;
    auto memoryInsts = blockAndInstsPair.second;
    std::vector<Instruction *> orderedInstsInit{};
    auto result = orderedMemoryInstsByBlock.insert(std::make_pair(B, orderedInstsInit));
    auto &orderedInsts = (*result.first).second;

    for (auto &I : *B) {
      if (memoryInsts.find(&I) == memoryInsts.end()) continue;
      orderedInsts.push_back(&I);
    }
  }

  return orderedMemoryInstsByBlock;
}

void Mem2RegNonAlloca::assertAndDumpLogsOnFailure (std::function<bool(void)> assertion, std::string errorString) {
  if (!assertion()) {
    errs() << errorString << "\n";
    if (noelle.getVerbosity() >= Verbosity::Maximal) {
      dumpLogs();
    }
    abort();
  }
}

void Mem2RegNonAlloca::dumpLogs (void) {
  auto loop = LDI.getLoopStructure();
  auto basicBlocks = loop->getBasicBlocks();

  /*
   * Identify loop
   */
  std::string loopId{std::to_string(loop->getID())};

  // DGPrinter::writeGraph<SCCDAG, SCC>("mem2reg-sccdag-loop-" + loopId + ".dot", LDI.sccdagAttrs.getSCCDAG());
  // std::set<BasicBlock *> basicBlocksSet(basicBlocks.begin(), basicBlocks.end());
  // DGPrinter::writeGraph<SubCFGs, BasicBlock>("mem2reg-current-loop-" + loopId + ".dot", new SubCFGs(basicBlocksSet));
}
