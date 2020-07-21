/*
 * Copyright 2019 - 2020  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "LastLiveOutPeeler.hpp"

using namespace llvm;

/*
 * Restricted to loops where:
 * The only loop exit is from the loop entry block
 * The loop (and all sub-loops?) are governed by an IV
 */

/*
 TODO:

  Clone every basic block in the loop
  Exit from the original loop entry to the cloned loop entry
    If no loop body iteration ever executed, then route to the loop exit
    Otherwise, route to the cloned loop body
    The cloned latches route to a 2nd cloned entry that unconditionally branches to the loop exit

  Clone IV sccs, branches/conditions on IVs in dependent SCCs, last live out SCCs with computation and their dependent SCCs
  Step loop governing IV back one iteration
  Wire instructions together as follows:
    Any instructions from original loop governing IV to cloned, stepped-back IV
    Any instructions from other IVs to cloned IVs
    Any instructions from original loop entry to trailing/latch PHI pairs
      A trailing PHI at the loop entry consumes the PHI's previous iteration value at each latch
    Any instructions from original loop body to PHIs on the cloned loop body values
      The cloned body values only need PHIs since they do not dominate the last iteration's execution

*/

#include "LastLiveOutPeeler.hpp"

using namespace llvm;

LastLiveOutPeeler::LastLiveOutPeeler (LoopDependenceInfo const &LDI, Noelle &noelle)
  : LDI{LDI}, noelle{noelle} {
}

// bool LastLiveOutPeeler::peelLastLiveOutComputation () {

//   /*
//    * Ensure the loop entry is the only block to exit the loop
//    */
//   auto loopStructure = LDI.getLoopStructure();
//   auto loopHeader = loopStructure->getHeader(); 
//   auto exitBlocks = loopStructure->getLoopExitBasicBlocks();
//   if (exitBlocks.size() != 1) return false;

//   auto singleExitBlock = exitBlocks[0];
//   bool onlyExitsFromHeader = true;
//   bool exitsFromHeader = false;
//   for (auto exitPred : predecessors(singleExitBlock)) {
//     exitsFromHeader |= exitPred == loopHeader;
//     onlyExitsFromHeader = !loopStructure->isIncluded(exitPred) || exitPred == loopHeader;
//   }
//   if (!exitsFromHeader || !onlyExitsFromHeader) return false;

//   /*
//    * Ensure the loop is governed by an IV
//    */
//   auto loopGoverningIVAttribution = LDI.getLoopGoverningIVAttribution();
//   if (!loopGoverningIVAttribution) return false;

//   /*
//    * Determine if there is any live out computation that can be peeled
//    */
//   fetchSCCsOfLastLiveOuts();
//   if (this->sccsOfLastLiveOuts.size() == 0) return false;

//   /*
//    * Ensure that the control flow of the loop is governed by IVs and fully understood
//    */
//   auto controlFlowGovernedByIVs = fetchNormalizedSCCsGoverningControlFlowOfLoop();
//   if (!controlFlowGovernedByIVs) return false;

//   /*
//    * Identify induction variable SCCs in all sub-loops
//    */
//   auto ivManager = LDI.getInductionVariableManager();
//   std::unordered_set<InductionVariable *> allIVsInLoop{};
//   auto loops = loopStructure->getDescendants();
//   for (auto loop : loops) {
//     auto ivs = ivManager->getInductionVariables(*loop);
//     allIVsInLoop.insert(ivs.begin(), ivs.end());
//   }
//   auto loopGoverningIV = loopGoverningIVAttribution->getInductionVariable();
//   // TODO: Clone this later:
//   loopGoverningIV.getComputationOfStepValue();

//   // TODO: Everything

//   return true;
// }

// bool LastLiveOutPeeler::fetchNormalizedSCCsGoverningControlFlowOfLoop (void) {

//   auto loopStructure = LDI.getLoopStructure();
//   auto normalizedSCCDAG = LDI.sccdagAttrs.getSCCDAG();
//   auto ivManager = LDI.getInductionVariableManager();

//   for (auto loopBlock : loopStructure->getBasicBlocks()) {
//     auto terminator = loopBlock->getTerminator();
//     assert(terminator != nullptr
//       && "LastLiveOutPeeler: Loop is not well formed, having an un-terminated basic block");

//     /*
//      * Currently, we only support un-conditional or conditional branches w/conditions that are
//      * loop invariants OR instructions using IVs and loop invariants only
//      */
//     if (!isa<BranchInst>(terminator)) return false;
//     auto brInst = cast<BranchInst>(terminator);
//     if (brInst->isUnconditional()) continue;

//     auto sccOfTerminator = normalizedSCCDAG->sccOfValue(terminator);
//     auto sccInfoOfTerminator = LDI.sccdagAttrs.getSCCAttrs(sccOfTerminator);
//     if (sccInfoOfTerminator->isInductionVariableSCC()) {
//       normalizedSCCsOfGoverningIVs.insert(sccOfTerminator);
//       continue;
//     }

//     /*
//      * The condition must be loop invariant or an instruction using IVs and loop invariants only
//      */
//     auto condition = brInst->getCondition();
//     if (!isa<Instruction>(condition)) {
//       if (!loopStructure->isLoopInvariant(condition)) return false;
//       continue;
//     }

//     auto conditionInst = cast<Instruction>(condition);
//     auto sccOfCondition = normalizedSCCDAG->sccOfValue(condition);
//     normalizedSCCsOfConditionsAndBranchesDependentOnIVSCCs.insert(sccOfTerminator);
//     normalizedSCCsOfConditionsAndBranchesDependentOnIVSCCs.insert(sccOfCondition);

//     for (auto &op : conditionInst->operands()) {
//       auto value = op.get();
//       if (loopStructure->isLoopInvariant(value)) continue;

//       if (!isa<Instruction>(value)) return false;
//       auto inst = cast<Instruction>(value);
//       auto loopOfValue = LDI.getNestedMostLoopStructure(inst);
//       auto ivOfValue = ivManager->getInductionVariable(*loopOfValue, inst);
//       if (ivOfValue) continue;

//       return false;
//     }
//   }

//   return true;
// }

// /*
//  * We are interested in any last live outs with meaningful computation contained in the chain (excludes PHIs, casts)
//  */
// void LastLiveOutPeeler::fetchSCCsOfLastLiveOuts (void) {

//   auto loopStructure = LDI.getLoopStructure();
//   auto loopHeader = loopStructure->getHeader();
//   auto loopSCCDAG = LDI.getLoopSCCDAG();
//   auto normalizedSCCDAG = LDI.sccdagAttrs.getSCCDAG();

//   auto loopCarriedDependencies = LDI.getLoopCarriedDependencies();
//   auto outermostLoopCarriedDependencies = loopCarriedDependencies->getLoopCarriedDependenciesForLoop(*loopStructure);
//   std::unordered_set<Value *> loopCarriedConsumers{};
//   for (auto dependency : outermostLoopCarriedDependencies) {
//     auto consumer = dependency->getIncomingT();
//     this->loopCarriedConsumers.insert(consumer);
//   }

//   /*
//    * Last live outs can only result in leaf nodes
//    * Their computation CAN span a chain of SCCs though, all of which must only produce last live out loop carried dependencies
//    * 
//    * To be sure the parent SCCs/instructions up that chain we collect ONLY contain last live outs,
//    * we use the strict SCCDAG, not the normalized SCCDAG
//    */
//   for (auto leafSCCNode : loopSCCDAG->getLeafNodes()) {
//     auto leafSCC = leafSCCNode->getT();

//     /*
//      * The leaf SCC must be a single loop carried PHI
//      */
//     if (leafSCC->numInternalNodes() > 1) continue;
//     auto singleValue = leafSCC->internalNodePairs().begin()->first;
//     if (!isa<PHINode>(singleValue)) continue;
//     auto singlePHI = cast<PHINode>(singleValue);
//     if (singlePHI->getParent() != loopHeader) continue;

//     auto chainOfSCCs = fetchChainOfSCCsForLastLiveOutLeafSCC(leafSCCNode);
//     this->sccsOfLastLiveOuts.insert(chainOfSCCs.begin(), chainOfSCCs.end());
//   }

//   return;
// }

// std::unordered_set<SCC *> LastLiveOutPeeler::fetchChainOfSCCsForLastLiveOutLeafSCC (DGNode<SCC> *sccNode) {

//   /*
//    * Traverse up the graph, collecting as many SCC nodes that ONLY contribute loop carried
//    * dependencies to last live out values. Keep track if any of those SCCs contain meaningful computation
//    */
//   bool hasMeaningfulComputation = false;
//   std::unordered_set<SCC *> computationOfLastLiveOut{};
//   std::queue<DGNode<SCC> *> queueOfLastLiveOutComputation{};
//   queueOfLastLiveOutComputation.push(sccNode);

//   /*
//    * For the sake of efficiency, even if the SCCDAG is acyclic, don't re-process SCCs
//    */
//   std::unordered_set<DGNode<SCC> *> visited{};
//   visited.insert(sccNode);

//   while (!queueOfLastLiveOutComputation.empty()) {
//     auto sccNode = queueOfLastLiveOutComputation.front();
//     queueOfLastLiveOutComputation.pop();

//     auto scc = sccNode->getT();
//     bool isLoopCarried = false;
//     bool hasComputation = false;
//     for (auto nodePair : scc->internalNodePairs()) {
//       auto value = nodePair.first;
//       if (!isa<Instruction>(value)) continue;
//       auto inst = cast<Instruction>(value);

//       if (!isa<PHINode>(inst) && !isa<CastInst>(inst)) {
//         hasComputation = true;
//       }

//       if (loopCarriedConsumers.find(inst) != loopCarriedConsumers.end()) {
//         isLoopCarried = true;
//         break;
//       }
//     }

//     /*
//      * Do not include SCCs with loop carried values
//      */
//     if (isLoopCarried) continue;

//     hasMeaningfulComputation |= hasComputation;
//     computationOfLastLiveOut.insert(scc);

//     for (auto edge : sccNode->getIncomingEdges()) {
//       auto producerSCCNode = edge->getOutgoingNode();
//       if (visited.find(producerSCCNode) != visited.end()) continue;
//       queueOfLastLiveOutComputation.push(producerSCCNode);
//       visited.insert(producerSCCNode);
//     }
//   }

//   /*
//    * Only return a non-empty set if those SCCs are worth peeling
//    */
//   if (!hasMeaningfulComputation) computationOfLastLiveOut.clear();
//   return computationOfLastLiveOut;
// }