/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "InductionVariables.hpp"
#include "LoopGoverningIVAttribution.hpp"

using namespace llvm;

InductionVariables::InductionVariables (LoopsSummary &LIS, ScalarEvolution &SE, SCCDAG &sccdag, LoopEnvironment &loopEnv)
  : loopToIVsMap{}, loopToGoverningIVMap{} {

  Function &F = *LIS.getLoopNestingTreeRoot()->getHeader()->getParent();
  ScalarEvolutionReferentialExpander referentialExpander(SE, F);

  for (auto &loop : LIS.loops) {
    loopToIVsMap[loop.get()] = std::set<InductionVariable *>();

    /*
     * Fetch the loop header.
     */
    auto header = loop->getHeader();

    /*
     * Iterate over all phis within the loop header.
     */
    for (auto &phi : header->phis()) {
      // phi.print(errs() << "Checking PHI: "); errs() << "\n";
      auto scev = SE.getSCEV(&phi);
      if (!scev || scev->getSCEVType() != SCEVTypes::scAddRecExpr) continue;

      auto sccContainingIV = sccdag.sccOfValue(&phi);
      auto IV = new InductionVariable(loop.get(), SE, &phi, *sccContainingIV, loopEnv, referentialExpander); 
      if (!IV->getSimpleValueOfStepSize() && !IV->getComposableStepSize()) {
        delete IV;
        continue;
      }

      loopToIVsMap[loop.get()].insert(IV);
      auto exitBlocks = LIS.getLoop(phi)->getLoopExitBasicBlocks();
      LoopGoverningIVAttribution attribution(*IV, *sccContainingIV, exitBlocks);
      if (attribution.isSCCContainingIVWellFormed()) {
        loopToGoverningIVMap[loop.get()] = IV;
      }
    }
  }
}

InductionVariables::~InductionVariables () {
  for (auto loopIVs : loopToIVsMap) {
    for (auto IV : loopIVs.second) {
      delete IV;
    }
  }
  loopToIVsMap.clear();
  loopToGoverningIVMap.clear();
}

std::set<InductionVariable *> &InductionVariables::getInductionVariables(LoopSummary &LS) {
  return loopToIVsMap.at(&LS);
}

InductionVariable *InductionVariables::getLoopGoverningInductionVariable (LoopSummary &LS) {
  if (loopToGoverningIVMap.find(&LS) == loopToGoverningIVMap.end()) return nullptr;
  return loopToGoverningIVMap.at(&LS);
}

InductionVariable::InductionVariable  (
  LoopSummary *LS,
  ScalarEvolution &SE,
  PHINode *headerPHI,
  SCC &scc,
  LoopEnvironment &loopEnv,
  ScalarEvolutionReferentialExpander &referentialExpander
) : scc{scc}, headerPHI{headerPHI}, startValue{nullptr},
    stepSize{nullptr}, compositeStepSize{nullptr}, expansionOfCompositeStepSize{},
    isStepLoopInvariant{false} {

  /*
   * Collect intermediate values of the IV within the loop (by traversing its strongly connected component)
   * Traverse data dependencies the header PHI has.
   */
  std::queue<DGNode<Value> *> ivIntermediateValues;
  std::set<Value *> valuesVisited;
  ivIntermediateValues.push(scc.fetchNode(headerPHI));
  while (!ivIntermediateValues.empty()) {
    auto node = ivIntermediateValues.front();
    auto value = node->getT();
    ivIntermediateValues.pop();

    if (valuesVisited.find(value) != valuesVisited.end()) continue;
    valuesVisited.insert(value);

    if (auto phi = dyn_cast<PHINode>(value)) {
      this->PHIs.insert(phi);
      this->allInstructions.insert(cast<Instruction>(phi));
    } else if (auto I = dyn_cast<Instruction>(value)) {
      this->accumulators.insert(I);
      this->allInstructions.insert(I);
    }

    for (auto edge : node->getIncomingEdges()) {
      if (!edge->isDataDependence()) continue;
      auto otherNode = edge->getOutgoingNode();
      if (!scc.isInternal(otherNode->getT())) continue;
      ivIntermediateValues.push(otherNode);
    }
  }

  /*
   * Include any casts on intermediate values
   * TODO: Determine what other instructions could still represent the induction variable
   * but not necessarily appear in the SCC for that induction variable
   */
  std::set<CastInst *> castsToAdd{};
  for (auto intermediateValue : this->allInstructions) {
    for (auto user : intermediateValue->users()) {
      if (!isa<CastInst>(user)) continue;
      castsToAdd.insert(cast<CastInst>(user));
    }
  }
  this->allInstructions.insert(castsToAdd.begin(), castsToAdd.end());

  /*
   * Fetch initial value of induction variable
   */
  auto bbs = LS->getBasicBlocks();
  for (auto i = 0; i < headerPHI->getNumIncomingValues(); ++i) {
    auto incomingBB = headerPHI->getIncomingBlock(i);
    if (bbs.find(incomingBB) == bbs.end()) {
      this->startValue = headerPHI->getIncomingValue(i);
      break;
    }
  }

  auto headerSCEV = SE.getSCEV(headerPHI);
  assert(headerSCEV->getSCEVType() == SCEVTypes::scAddRecExpr);
  auto stepSCEV = cast<SCEVAddRecExpr>(headerSCEV)->getStepRecurrence(SE);

  // auto M = headerPHI->getFunction()->getParent();
  // DataLayout DL(M);
  // const char name = 'a';
  // SCEVExpander *expander = new SCEVExpander(SE, DL, &name);

  std::set<Value *> valuesInScope{}; 
  std::set<Value *> valuesToReferenceAndNotExpand{};
  std::set<Value *> valuesInternalToLoop{};
  for (auto internalNodePair : scc.internalNodePairs()) {
    auto value = internalNodePair.first;
    valuesInScope.insert(value);
    valuesInternalToLoop.insert(value);
  }
  for (auto externalPair : scc.externalNodePairs()) {
    auto value = externalPair.first;
    valuesInScope.insert(value);
    if (!isa<Instruction>(value) || bbs.find(cast<Instruction>(value)->getParent()) == bbs.end()) {
      valuesToReferenceAndNotExpand.insert(value);
    } else {
      valuesInternalToLoop.insert(value);
    }
  }
  for (auto liveIn : loopEnv.getProducers()) {
    valuesInScope.insert(liveIn);
    valuesToReferenceAndNotExpand.insert(liveIn);
  }

  switch (stepSCEV->getSCEVType()) {
    case SCEVTypes::scConstant:
      this->stepSize = cast<SCEVConstant>(stepSCEV)->getValue();
      this->isStepLoopInvariant = true;
      break;
    case SCEVTypes::scUnknown:
      this->stepSize = cast<SCEVUnknown>(stepSCEV)->getValue();
      this->isStepLoopInvariant = true;
      if (valuesToReferenceAndNotExpand.find(this->stepSize) == valuesToReferenceAndNotExpand.end()) {
        this->stepSize = nullptr;
        this->isStepLoopInvariant = false;
      }
      break;
    default:

      // stepSCEV->print(errs() << "Referencing: "); errs() << "\n";
      auto stepSizeReferenceTree = referentialExpander.createReferenceTree(stepSCEV, valuesInScope);
      if (stepSizeReferenceTree) {
        // stepSizeReferenceTree->getSCEV()->print(errs() << "Expanding: "); errs() << "\n";
        auto tempBlock = BasicBlock::Create(headerPHI->getContext());
        IRBuilder<> tempBuilder(tempBlock);
        auto finalValue = referentialExpander.expandUsingReferenceValues(
          stepSizeReferenceTree,
          valuesToReferenceAndNotExpand,
          tempBuilder
        );
        if (finalValue) {
          this->isStepLoopInvariant = true;
          auto references = stepSizeReferenceTree->collectAllReferences();
          for (auto reference : references) {
            if (isa<SCEVAddRecExpr>(reference->getSCEV())) {
              if (!reference->getValue() || valuesInternalToLoop.find(reference->getValue()) != valuesInternalToLoop.end()) {
                this->isStepLoopInvariant = false;
                break;
              }
            }
          }

          this->compositeStepSize = stepSCEV;
          finalValue->print(errs() << "Expanded final value: "); errs() << "\n";
          // TODO: Handle case where no expansion is necessary, say if the finalValue is an SCEV already external to the loop
          for (auto &I : *tempBlock) {
            expansionOfCompositeStepSize.push_back(&I);
          }
        }
      }
      break;
  }

  // TODO: Determine why this seg faults on the destructor of a ValueHandleBase
  // delete expander;
}

bool InductionVariable::isStepSizeLoopInvariant (void) const {
  return isStepLoopInvariant;
}

InductionVariable::~InductionVariable () {
  BasicBlock *tempBlock = nullptr;
  // for (auto expandedInst : expansionOfCompositeStepSize) {
  //   tempBlock = expandedInst->getParent();
  //   expandedInst->eraseFromParent();
  // }
  if (tempBlock) {
    tempBlock->deleteValue();
  }
}

SCC *InductionVariable::getSCC (void) const {
  return &scc;
}

PHINode * InductionVariable::getHeaderPHI (void) const {
  return headerPHI;
}

std::set<PHINode *> & InductionVariable::getPHIs (void) {
  return PHIs;
}

std::set<Instruction *> &InductionVariable::getAccumulators (void) {
  return accumulators;
}

std::set<Instruction *> &InductionVariable::getAllInstructions(void) {
  return allInstructions;
}

Value *InductionVariable::getStartAtHeader (void) const {
  return startValue;
}

Value *InductionVariable::getSimpleValueOfStepSize (void) const {
  return stepSize;
}

const SCEV *InductionVariable::getComposableStepSize (void) const {
  return compositeStepSize;
}

std::vector<Instruction *> InductionVariable::getExpansionOfCompositeStepSize(void) const {
  return expansionOfCompositeStepSize;
}
