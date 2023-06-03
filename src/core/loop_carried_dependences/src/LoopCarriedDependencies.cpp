/*
 * Copyright 2016 - 2021  Angelo Matni, Simone Campanoni, Brian Homerding
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
#include "noelle/core/LoopCarriedDependencies.hpp"
#include "noelle/core/MayPointToAnalysis.hpp"
#include "noelle/core/MayPointToAnalysisUtils.hpp"

namespace llvm::noelle {

void LoopCarriedDependencies::setLoopCarriedDependencies(
    LoopTree *loopNode,
    const DominatorSummary &DS,
    PDG &dgForLoops) {
  for (auto edge : dgForLoops.getEdges()) {
    assert(!edge->isLoopCarriedDependence() && "Flag was already set");
  }

  for (auto edge : dgForLoops.getEdges()) {
    if (!LoopCarriedDependencies::isALoopCarriedDependence(loopNode,
                                                           DS,
                                                           edge)) {
      continue;
    }

    auto loop =
        LoopCarriedDependencies::isALoopCarriedDependence(loopNode, DS, edge);
    if (!loop) {
      continue;
    }
    edge->setLoopCarried(true);
  }

  auto loopFunction = loopNode->getLoop()->getFunction();
  auto mayPointToAnalysis = new MayPointToAnalysis(loopFunction);
  auto ptSum = mayPointToAnalysis->getPointToSummary();

  std::unordered_set<DGEdge<Value> *> spuriousLoopCarriedEdges;
  for (auto edge : dgForLoops.getEdges()) {
    if (canLoopCarriedDependenceRemovedByMemoryOverwriting(loopNode,
                                                           DS,
                                                           edge,
                                                           ptSum,
                                                           dgForLoops)) {
      spuriousLoopCarriedEdges.insert(edge);
    }
  }

  errs()
      << "LoopCarriedDependences: Removing spurious loop-carried edges for loop"
      << *loopNode->getLoop()->getEntryInstruction() << "\n";
  for (auto edge : spuriousLoopCarriedEdges) {
    auto producer = edge->getOutgoingT();
    auto consumer = edge->getIncomingT();
    errs() << "LoopCarriedDependences: Removing spurious loop-carried edge: "
           << *producer << " ---> " << *consumer << "\n";
    edge->setLoopCarried(false);
  }

  return;
}

bool LoopCarriedDependencies::isALoopCarriedDependence(
    LoopTree *loopNode,
    const DominatorSummary &DS,
    DGEdge<Value> *edge) {

  /*
   * Fetch the loop.
   */
  auto topLoop = loopNode->getLoop();
  assert(topLoop != nullptr);
  auto topLoopHeader = topLoop->getHeader();
  auto topLoopHeaderBranch = topLoopHeader->getTerminator();

  /*
   * Fetch the instructions involved in the dependence.
   */
  auto producer = edge->getOutgoingT();
  auto consumer = edge->getIncomingT();

  /*
   * Only dependences between instructions can be loop-carried.
   */
  if (!isa<Instruction>(producer)) {
    return false;
  }
  if (!isa<Instruction>(consumer)) {
    return false;
  }

  /*
   * Fetch the instructions involved in the dependence.
   */
  auto producerI = dyn_cast<Instruction>(producer);
  auto consumerI = dyn_cast<Instruction>(consumer);

  /*
   * Fetch the innermost loops that contain the two instructions.
   */
  auto producerLoop = loopNode->getInnermostLoopThatContains(producerI);
  auto consumerLoop = loopNode->getInnermostLoopThatContains(consumerI);

  /*
   * If either of the instruction does not belong to a loop, then the dependence
   * cannot be loop-carried.
   */
  if (!producerLoop || !consumerLoop) {
    return false;
  }

  /*
   * If the dependence is a control one and the two instructions belong to a
   * subloop, then this cannot be a loop-carried one for the target loop.
   */
  if (edge->isControlDependence() && (producerLoop != loopNode->getLoop())
      && (consumerLoop != loopNode->getLoop())) {
    return false;
  }

  /*
   * Check if both instructions will access the same element (variable, memory
   * location) in the same iteration
   */
  auto doTheyTouchTheSameElementInTheSameIteration = true;
  if (edge->isMemoryDependence()) {

    /*
     * Fetch the pointer of the location accessed by the producer.
     */
    Value *producerPointer = nullptr;
    if (auto load = dyn_cast<LoadInst>(producerI)) {
      producerPointer = load->getPointerOperand();
    } else if (auto store = dyn_cast<StoreInst>(producerI)) {
      producerPointer = store->getPointerOperand();
    }

    /*
     * Fetch the pointer of the location accessed by the consumer.
     */
    Value *consumerPointer = nullptr;
    if (auto load = dyn_cast<LoadInst>(consumerI)) {
      consumerPointer = load->getPointerOperand();
    } else if (auto store = dyn_cast<StoreInst>(consumerI)) {
      consumerPointer = store->getPointerOperand();
    }

    /*
     * If we cannot identify the single pointer for each instruction, then we
     * cannot use dominance to determine whether the dependence is loop-carried
     * or not.
     */
    if ((producerPointer == nullptr) || (consumerPointer == nullptr)) {
      doTheyTouchTheSameElementInTheSameIteration = false;

    } else {

      /*
       * Each instruction can only access a single memory location per
       * iteration.
       *
       * Check whether they access the same location per iteration or not.
       */
      if (producerPointer != consumerPointer) {
        doTheyTouchTheSameElementInTheSameIteration = false;

      } else {
        assert(producerPointer == consumerPointer);
        auto pointerAsInst = dyn_cast<Instruction>(producerPointer);
        if (pointerAsInst == nullptr) {
          doTheyTouchTheSameElementInTheSameIteration = false;
        } else {
          if (topLoop->isIncluded(pointerAsInst)) {
            doTheyTouchTheSameElementInTheSameIteration = false;
          }
        }
      }
    }
  }
  if (!doTheyTouchTheSameElementInTheSameIteration) {
    return true;
  }

  if (producerI == consumerI || !DS.DT.dominates(producerI, consumerI)) {
    auto producerLevel = producerLoop->getNestingLevel();
    auto consumerLevel = consumerLoop->getNestingLevel();
    auto isMemoryDependenceThusCanCrossLoops = edge->isMemoryDependence();
    auto isControlDependence = edge->isControlDependence();

    /*
     * Check if the dependence is data and via variable.
     */
    if (!edge->isMemoryDependence() && edge->isDataDependence()) {

      /*
       * The data dependence is variable based
       *
       * If the producer cannot reach the header of the loop without reaching
       * the consumer, then the dependence cannot be loop-carried.
       */
      auto producerB = producerI->getParent();
      auto consumerB = consumerI->getParent();
      auto mustProducerReachConsumerBeforeHeader =
          !canBasicBlockReachHeaderBeforeOther(*consumerLoop,
                                               producerB,
                                               consumerB);
      if (mustProducerReachConsumerBeforeHeader) {
        return false;
      }

      /*
       * The data dependence is variable based.
       * The producer can reach the header before reaching the consumer.
       *
       * Check if the consumer will take the value from someone else when the
       * execution comes from the header rather than the producer of the
       * previous iteration
       */
      if (DS.DT.dominates(consumerI, producerI)
          && DS.DT.dominates(topLoopHeaderBranch, consumerI)) {
        if (auto phiConsumer = dyn_cast<PHINode>(consumerI)) {
          return false;
        }
      }
    }

    return true;
  }

  return false;
}

std::set<DGEdge<Value> *> LoopCarriedDependencies::
    getLoopCarriedDependenciesForLoop(const LoopStructure &LS,
                                      LoopTree *loopNode,
                                      PDG &LoopDG) {

  std::set<DGEdge<Value> *> LCEdges;
  for (auto edge : LoopDG.getEdges()) {
    if (!edge->isLoopCarriedDependence()) {
      continue;
    }

    auto consumer = edge->getIncomingT();
    auto consumerI = cast<Instruction>(consumer);
    auto consumerLoop = loopNode->getInnermostLoopThatContains(consumerI);
    if (consumerLoop != &LS) {
      continue;
    }

    LCEdges.insert(edge);
  }

  return LCEdges;
}

std::set<DGEdge<Value> *> LoopCarriedDependencies::
    getLoopCarriedDependenciesForLoop(const LoopStructure &LS,
                                      LoopTree *loopNode,
                                      SCCDAG &sccdag) {

  std::set<DGEdge<Value> *> LCEdges;

  for (auto sccNode : sccdag.getNodes()) {
    auto scc = sccNode->getT();
    for (auto edge : scc->getEdges()) {
      if (!edge->isLoopCarriedDependence()) {
        continue;
      }

      auto consumer = edge->getIncomingT();
      auto consumerI = cast<Instruction>(consumer);
      auto consumerLoop = loopNode->getInnermostLoopThatContains(consumerI);
      if (consumerLoop != &LS) {
        continue;
      }

      auto producer = edge->getOutgoingT();
      auto producerI = dyn_cast<Instruction>(producer);
      if (producerI == NULL) {
        continue;
      }

      auto producerLoop = loopNode->getInnermostLoopThatContains(producerI);
      if (!producerLoop) {
        continue;
      }
      LCEdges.insert(edge);
    }
  }
  return LCEdges;
}

bool LoopCarriedDependencies::canBasicBlockReachHeaderBeforeOther(
    const LoopStructure &LS,
    BasicBlock *I,
    BasicBlock *J) {

  assert(LS.isIncluded(I) && LS.isIncluded(J));

  // I->print(errs() << "Source:\n");
  // J->print(errs() << "Destination:\n");

  /*
   * If the source is the destination, the loop must be at a later iteration
   */
  if (I == J) {
    return true;
  }

  auto header = LS.getHeader();
  auto exitsVector = LS.getLoopExitBasicBlocks();
  std::set<BasicBlock *> exits(exitsVector.begin(), exitsVector.end());
  std::queue<BasicBlock *> queue;
  std::unordered_set<BasicBlock *> enqueued;
  queue.push(I);
  enqueued.insert(I);
  bool isJReached = false;

  while (!queue.empty()) {
    auto B = queue.front();
    queue.pop();

    /*
     * Check if the successor is the header block
     * Check if the successor is an exit block; if so, do not traverse further
     * Check if the destination is reached; if so, do not traverse further
     */
    if (B == header)
      return true;
    if (exits.find(B) != exits.end())
      continue;
    if (B == J) {
      isJReached = true;
      continue;
    }

    for (auto succIter = succ_begin(B); succIter != succ_end(B); ++succIter) {
      auto succ = *succIter;

      /*
       * Do not re-traverse enqueued blocks
       */
      if (enqueued.find(succ) != enqueued.end())
        continue;
      queue.push(succ);
      enqueued.insert(succ);
    }
  }

  /*
   * The header was never reached
   */
  assert(isJReached);
  return false;
}

bool LoopCarriedDependencies::
    canLoopCarriedDependenceRemovedByMemoryOverwriting(
        LoopTree *loopNode,
        const DominatorSummary &DS,
        DGEdge<Value> *edge,
        PointToSummary *ptSum,
        PDG &dgForLoops) {

  /*
   * Focus on Loop-carried RAW Memory Dependence
   */
  if (!(edge->isLoopCarriedDependence() && edge->isMemoryDependence()
        && edge->isRAWDependence())) {
    return false;
  }

  /*
   * If we could not identify the producer and the consumer,
   * then we choose not to remove loop-carried property.
   */
  auto producerI = cast<Instruction>(edge->getOutgoingT());
  auto consumerI = cast<Instruction>(edge->getIncomingT());
  if (!producerI || !consumerI) {
    return false;
  }
  auto producerLoop = loopNode->getInnermostLoopThatContains(producerI);
  auto consumerLoop = loopNode->getInnermostLoopThatContains(consumerI);
  if (!producerLoop || !consumerLoop) {
    return false;
  }

  /*
   * Use May-point-to-analysis to ensure that the producer and the consumer
   * access exactly the same memory location.
   */
  auto getAccessedMemoryObjects = [&](Instruction *i) -> MemoryObjects {
    if (auto loadInst = dyn_cast<LoadInst>(i)) {
      auto pointToGraph = ptSum->instIN.at(loadInst);
      return ptSum->pointees(pointToGraph, loadInst->getPointerOperand());
    } else if (auto storeInst = dyn_cast<StoreInst>(i)) {
      auto pointToGraph = ptSum->instIN.at(storeInst);
      return ptSum->pointees(pointToGraph, storeInst->getPointerOperand());
    } else {
      return MemoryObjects();
    }
  };

  auto producerMemoryObjects = getAccessedMemoryObjects(producerI);
  auto consumerMemoryObjects = getAccessedMemoryObjects(consumerI);
  auto memoObjsMustAccessedByBoth =
      intersect(producerMemoryObjects, consumerMemoryObjects);
  auto memoObjsMayAccessedByBoth =
      unite(producerMemoryObjects, consumerMemoryObjects);
  if (!(memoObjsMustAccessedByBoth.size() == 1
        && memoObjsMayAccessedByBoth.size() == 1)) {
    return false;
  }

  auto allocaInst =
      dyn_cast<AllocaInst>((*memoObjsMustAccessedByBoth.begin())->source);
  if (!allocaInst) {
    return false;
  }

  /*
   * For now, we just focus on how to detect the memory overwriting for array
   */
  if (allocaInst->getAllocatedType()->isArrayTy()) {
    LLVMContext &C = allocaInst->getContext();
    auto arrayType = dyn_cast<ArrayType>(allocaInst->getAllocatedType());
    auto arraySize =
        ConstantInt::get(Type::getInt32Ty(C), arrayType->getNumElements());

    for (auto child : loopNode->getChildren()) {
      /*
       * To fully overwrite an array in each iteration,
       * the overwriting happens in a sub-loop, and each entry of the array
       * should be overwritten. Therefore we have some requirements about (1).
       * how we could access each entry (2). whether the data written to each
       * entry is "good" (3). whether the overwriting has impact
       */
      auto subLoop = child->getLoop();
      auto subLoopHeader = subLoop->getHeader();

      /*
       * (1). To access each entry, the sub-loop should have one Induction
       * Variable with 4 properties: I. The IV of the sub-loop starts from 0
       */
      ConstantInt *zero = ConstantInt::get(Type::getInt32Ty(C), 0);
      PHINode *arrayIndex = [&]() {
        for (auto &phi : subLoopHeader->phis()) {
          return &phi;
        }
      }();
      if (!arrayIndex || arrayIndex->getNumIncomingValues() != 2
          || !isa<ConstantInt>(arrayIndex->getIncomingValue(0))
          || dyn_cast<ConstantInt>(arrayIndex->getIncomingValue(0))
                     ->getZExtValue()
                 != 0) {
        continue;
      }

      /*
       * II. The IV of the sub-loop is incremented by 1 in each iteration
       */
      AddOperator *addOneEachStep =
          dyn_cast<AddOperator>(arrayIndex->getIncomingValue(1));
      if (!addOneEachStep || (addOneEachStep->getOperand(0) != arrayIndex)
          || !isa<ConstantInt>(addOneEachStep->getOperand(1))
          || dyn_cast<ConstantInt>(addOneEachStep->getOperand(1))
                     ->getZExtValue()
                 != 1) {
        continue;
      }

      /*
       * III. The IV of the sub-loop ends at the size of the array
       */
      ICmpInst *arraySizeEnd;
      if (auto cbr = dyn_cast<BranchInst>(subLoopHeader->getTerminator())) {
        if (auto condition = dyn_cast<ICmpInst>(cbr->getCondition())) {
          if (condition->getOperand(0) == arrayIndex
              && condition->getOperand(1) == arraySize
              && condition->getPredicate() == ICmpInst::ICMP_EQ) {
            arraySizeEnd = condition;
          }
        }
      }
      if (!arraySizeEnd) {
        continue;
      }

      /*
       * IV. Each entry should be written in one iteration of the sub-loop
       */
      std::unordered_set<StoreInst *> overwrites;
      for (auto inst : subLoop->getInstructions()) {
        if (auto storeInst = dyn_cast<StoreInst>(inst)) {
          overwrites.insert(storeInst);
        }
      }
      if (overwrites.size() != 1) {
        continue;
      }
      auto overwrite = *overwrites.begin();
      if (!isa<GetElementPtrInst>(overwrite->getPointerOperand())) {
        continue;
      }
      auto gep = dyn_cast<GetElementPtrInst>(overwrite->getPointerOperand());
      if (gep->getNumOperands() != 3 || gep->getOperand(0) != allocaInst
          || gep->getOperand(2) != arrayIndex) {
        continue;
      }

      /*
       * (2). To ensure the data written to each entry is "good",
       * we need to ensure that the data has no loop-carried dependence.
       * which means after the overwriting, each entry of the array has no
       * loop-carried dependence. therefore the array itself has no loop-carried
       * dependence.
       */
      auto overwriteValueHasNoLoopCarriedDependence = true;
      for (auto edge : dgForLoops.getEdges()) {
        auto producer = edge->getOutgoingT();
        auto consumer = edge->getIncomingT();
        auto overwriteValue = overwrite->getValueOperand();
        if (edge->isLoopCarriedDependence() && consumer == overwriteValue) {
          overwriteValueHasNoLoopCarriedDependence = false;
          break;
        }
      }
      if (!overwriteValueHasNoLoopCarriedDependence) {
        continue;
      }

      /*
       * (3). To ensure the overwriting has impact,
       * the overwriting must dominate the consumer (in this case, the reader).
       * otherwise, we cannot ensure the reader will read the data written by
       * the overwriting, and the loop-carried RAW memory dependence stils holds
       * conservatively.
       */
      if (DS.DT.dominates(subLoopHeader->getTerminator(), consumerI)) {
        return true;
      }
    }
  }

  return false;
}

} // namespace llvm::noelle
