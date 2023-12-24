/*
 * Copyright 2016 - 2023  Angelo Matni, Yian Su, Simone Campanoni
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
#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/TalkDown.hpp"
#include "noelle/core/PDGPrinter.hpp"
#include "noelle/core/PDGAnalysis.hpp"
#include "noelle/core/Utils.hpp"

namespace arcana::noelle {

PDGAnalysis::PDGAnalysis()
  : ModulePass{ ID },
    M{ nullptr },
    programDependenceGraph{ nullptr },
    mpa{},
    dfa{},
    embedPDG{ false },
    dumpPDG{ false },
    performThePDGComparison{ false },
    disableSVF{ false },
    disableAllocAA{ false },
    disableRA{ false },
    printer{},
    noelleCG{ nullptr } {

  return;
}

void PDGAnalysis::releaseMemory() {
  if (this->programDependenceGraph)
    delete this->programDependenceGraph;
  this->programDependenceGraph = nullptr;

  return;
}

void PDGAnalysis::printFunctionReachabilityResult() {

  /*
   * Print internal and unhandled external functions.
   */
  errs() << "Internal Functions:\n";
  for (auto &internal : this->internalFuncs) {
    errs() << "\t" << internal->getName() << "\n";
  }
  errs() << "Unhandled External Functions:\n";
  for (auto &external : this->unhandledExternalFuncs) {
    errs() << "\t" << external->getName() << "\n";
  }

  /*
   * Print reachability results.
   */
  for (auto &pair : this->reachableUnhandledExternalFuncs) {
    errs()
        << "Reachable external functions of " << pair.first->getName() << "\n";
    for (auto &external : pair.second) {
      errs() << "\t" << external->getName() << "\n";
    }
  }

  return;
}

PDG *PDGAnalysis::getPDG(void) {

  /*
   * Check if we have already built the PDG.
   */
  if (this->programDependenceGraph) {
    return this->programDependenceGraph;
  }

  /*
   * Construct the PDG
   *
   * Check if we have already done it and the PDG has been embedded in the IR.
   */
  if (this->hasPDGAsMetadata(*this->M)) {

    /*
     * The PDG has been embedded in the IR.
     *
     * Load the embedded PDG.
     */
    this->programDependenceGraph = constructPDGFromMetadata(*this->M);
    if (this->performThePDGComparison) {
      auto PDGFromAnalysis = this->constructPDGFromAnalysis(*this->M);
      auto arePDGsEquivalent =
          this->comparePDGs(PDGFromAnalysis, this->programDependenceGraph);
      if (!arePDGsEquivalent) {
        errs() << "PDGAnalysis: Error = PDGs constructed are not the same\n";
        abort();
      }
      delete PDGFromAnalysis;
    }

  } else {

    /*
     * There is no PDG in the IR.
     *
     * Compute the PDG using the dependence analyses.
     */
    this->programDependenceGraph = constructPDGFromAnalysis(*this->M);

    /*
     * Check if we should embed the PDG.
     */
    if (this->embedPDG) {
      embedPDGAsMetadata(this->programDependenceGraph);
      if (this->performThePDGComparison) {
        auto PDGFromMetadata = this->constructPDGFromMetadata(*this->M);
        auto arePDGsEquivalen =
            this->comparePDGs(this->programDependenceGraph, PDGFromMetadata);
        if (!arePDGsEquivalen) {
          errs() << "PDGAnalysis: Error = PDGs constructed are not the same";
          abort();
        }
        delete PDGFromMetadata;
      }
    }
  }

  /*
   * Print the PDG
   */

  if (this->dumpPDG) {
    llvm::CallGraph llvmCG = llvm::CallGraph(*(this->M));
    this->printer.printPDG(
        *(this->M),
        llvmCG,
        this->programDependenceGraph,
        [this](llvm::Function *F) -> llvm::LoopInfo & {
          return llvm::Pass::getAnalysis<LoopInfoWrapperPass>(*F).getLoopInfo();
        });
  }

  return this->programDependenceGraph;
}

PDG *PDGAnalysis::constructPDGFromAnalysis(Module &M) {
  if (verbose >= PDGVerbosity::Maximal) {
    errs() << "PDGAnalysis: Construct PDG from Analysis\n";
  }

  auto pdg = new PDG(M);

  constructEdgesFromUseDefs(pdg);
  constructEdgesFromAliases(pdg, M);
  constructEdgesFromControl(pdg, M);

  trimDGUsingCustomAliasAnalysis(pdg);

  return pdg;
}

void PDGAnalysis::trimDGUsingCustomAliasAnalysis(PDG *pdg) {

  /*
   * Fetch AllocAA
   */
  this->allocAA = &getAnalysis<AllocAA>();
  if (this->disableAllocAA) {
    return;
  }

  /*
   * Invoke AllocAA
   * Fetch and invoke MayPointsToAnalysis
   */
  this->mpa = MayPointsToAnalysis{};
  removeEdgesNotUsedByParSchemes(pdg);

  /*
   * Invoke the TalkDown
   */
  auto &talkDown = getAnalysis<TalkDown>();
  // TODO

  return;
}

void PDGAnalysis::constructEdgesFromUseDefs(PDG *pdg) {

  /*
   * Add the dependences due to variables.
   */
  for (auto node : make_range(pdg->begin_nodes(), pdg->end_nodes())) {

    /*
     * Check the current definition has uses.
     * If it doesn't, then there is no variable dependence.
     */
    auto pdgValue = node->getT();
    if (pdgValue->getNumUses() == 0) {
      continue;
    }

    /*
     * The current definition has uses.
     * Add the uses.
     */
    for (auto &U : pdgValue->uses()) {
      auto user = U.getUser();

      if (isa<Instruction>(user) || isa<Argument>(user)) {
        auto edge = pdg->addVariableDataDependenceEdge(pdgValue, user);
        edge->setMemMustType(true, DG_DATA_RAW);
      }
    }
  }

  return;
}

void PDGAnalysis::constructEdgesFromAliases(PDG *pdg, Module &M) {

  /*
   * Use alias analysis on stores, loads, and function calls to construct PDG
   * edges
   */
  for (auto &F : M) {

    /*
     * Check if the function has a body.
     */
    if (F.empty())
      continue;

    /*
     * Add the edges to the PDG.
     */
    constructEdgesFromAliasesForFunction(pdg, F);
  }

  return;
}

void PDGAnalysis::constructEdgesFromAliasesForFunction(PDG *pdg, Function &F) {

  /*
   * Fetch the alias analysis.
   */
  auto &AA = getAnalysis<AAResultsWrapperPass>(F).getAAResults();

  /*
   * Run the reachable analysis.
   */
  auto onlyMemoryInstructionFilter = [](Instruction *i) -> bool {
    if (isa<LoadInst>(i)) {
      return true;
    }
    if (isa<StoreInst>(i)) {
      return true;
    }
    if (isa<CallBase>(i)) {
      return true;
    }
    return false;
  };
  auto dfr =
      this->disableRA
          ? this->dfa.getFullSets(&F)
          : this->dfa.runReachableAnalysis(&F, onlyMemoryInstructionFilter);

  for (auto &B : F) {
    for (auto &I : B) {

      /*
       * Check if the instruction can access memory.
       */
      if (!PDGAnalysis::canAccessMemory(&I)) {
        continue;
      }

      /*
       * Check the memory dependences that start from @I
       */
      if (auto store = dyn_cast<StoreInst>(&I)) {
        iterateInstForStore(pdg, F, AA, dfr, store);
      } else if (auto load = dyn_cast<LoadInst>(&I)) {
        iterateInstForLoad(pdg, F, AA, dfr, load);
      } else if (auto call = dyn_cast<CallBase>(&I)) {
        iterateInstForCall(pdg, F, AA, dfr, call);
      }
    }
  }

  /*
   * Free the memory.
   */
  delete dfr;
}

void PDGAnalysis::removeEdgesNotUsedByParSchemes(PDG *pdg) {
  std::set<DGEdge<Value, Value> *> removeEdges;

  /*
   * Collect the edges in the PDG that can be safely removed.
   */
  for (auto edge : pdg->getEdges()) {

    /*
     * Fetch the source of the dependence.
     */
    auto source = edge->getSrc();
    if (!isa<Instruction>(source)) {
      continue;
    }

    /*
     * Check if the dependence can be removed because the instructions accessing
     * separate memory regions.
     */
    if (isa<MemoryDependence<Value, Value>>(edge)
        && this->canMemoryEdgeBeRemoved(pdg, edge)) {
      removeEdges.insert(edge);
      continue;
    }

    /*
     * Check if the function of the dependence destination cannot be reached
     * from main.
     */
    if (edgeIsNotLoopCarriedMemoryDependency(edge)
        || edgeIsAlongNonMemoryWritingFunctions(edge)) {
      removeEdges.insert(edge);
    }
  }

  /*
   * Remove the tagged edges.
   */
  for (auto edge : removeEdges) {
    pdg->removeEdge(edge);
  }

  return;
}

bool PDGAnalysis::canMemoryEdgeBeRemoved(PDG *pdg, DGEdge<Value, Value> *edge) {
  assert(pdg != nullptr);
  assert(edge != nullptr);

  auto isCallToPrint = [](Value *i) -> bool {
    if (!isa<CallBase>(i)) {
      return false;
    }
    auto calleeFunc = dyn_cast<CallBase>(i)->getCalledFunction();
    if (!calleeFunc) {
      return false;
    }

    auto fname = calleeFunc->getName();
    std::unordered_set<std::string> printFuncs = { "printf",
                                                   "fprintf",
                                                   "puts",
                                                   "putc",
                                                   "putchar" };
    for (auto printFunc : printFuncs) {
      if (fname == printFunc || fname == (printFunc + "_unlocked")) {
        return true;
      }
    }
    return false;
  };

  auto getPointer = [](Value *i) -> Value * {
    if (isa<LoadInst>(i)) {
      return dyn_cast<LoadInst>(i)->getPointerOperand();
    } else if (isa<StoreInst>(i)) {
      return dyn_cast<StoreInst>(i)->getPointerOperand();
    } else {
      return nullptr;
    }
  };

  /*
   * Fetch the instructions
   */
  auto i0 = edge->getSrc();
  auto i1 = edge->getDst();

  /*
   * Handle the case where the two instructions are not calls.
   */
  if ((!isa<CallBase>(i0)) && (!isa<CallBase>(i1))) {
    auto p0 = getPointer(i0);
    auto p1 = getPointer(i1);
    if (p0 && p1 && !mpa.mayAlias(p0, p1)) {
      return true;
    }
    return false;
  }

  /*
   * One of the instruction is a call.
   *
   * Fetch the call
   */
  auto callInst = dyn_cast<CallBase>(i0);
  auto otherInst = i1;
  if (!callInst) {
    callInst = dyn_cast<CallBase>(i1);
    otherInst = i0;
  }
  assert(callInst != nullptr);

  /*
   * We don't handle self dependences.
   */
  if (otherInst == callInst) {
    return false;
  }

  /*
   * Check if it invokes a library function.
   */
  auto callee = callInst->getCalledFunction();
  if (callee == nullptr) {
    return false;
  }
  if (!callee->empty()) {
    return false;
  }

  /*
   * The call invokes a library function.
   *
   * Check it invokes a known library function.
   */
  if (!isCallToPrint(callInst)) {
    return false;
  }

  /*
   * There is no memory dependence between print and loadInst.
   * Both of them only read.
   */
  if (isa<LoadInst>(otherInst)) {
    return true;
  }

  /*
   * Exploit are knowledge of library calls to identify the set of pointers that
   * are used to read memory by the library call.
   */
  std::unordered_set<Value *> objects;
  for (auto argID = 0u; argID < callInst->getNumArgOperands(); argID++) {
    auto arg = callInst->getArgOperand(argID);
    auto argType = arg->getType();
    if (argType->isPointerTy()) {
      objects.insert(arg);
    }
  }

  /*
   * Get the object accessed by the other instruction.
   */
  Value *o1 = nullptr;
  if (auto st = dyn_cast<StoreInst>(otherInst)) {
    o1 = st->getPointerOperand();
  }

  if (o1 == nullptr) {
    return false;
  }

  /*
   * Check if the memory objects accessed cannot overlap between the two
   * instructions.
   */
  auto doOverlap = false;
  for (auto o0 : objects) {
    if (mpa.mayAlias(o0, o1)) {
      doOverlap = true;
      break;
    }
  }
  if (!doOverlap) {
    return true;
  }

  return false;
}

// NOTE: Loads between random parts of separate GVs and both edges between GVs
// should be removed
bool PDGAnalysis::edgeIsNotLoopCarriedMemoryDependency(
    DGEdge<Value, Value> *edge) {

  /*
   * Check if this is a memory dependence.
   */
  if (!isa<MemoryDependence<Value, Value>>(edge)) {
    return false;
  }

  /*
   * Fetch the source and destination of the dependence.
   */
  auto outgoingT = edge->getSrc();
  auto incomingT = edge->getDst();

  /*
   * Handle only memory instructions.
   */
  if (isa<CallBase>(outgoingT) || isa<CallBase>(incomingT)) {
    return false;
  }

  /*
   * Assert: must be a WAR load-store OR a RAW store-load
   */
  if (edge->isWARDependence()) {
    assert(isa<StoreInst>(incomingT) && isa<LoadInst>(outgoingT));
  } else if (edge->isRAWDependence()) {
    assert(isa<LoadInst>(incomingT) && isa<StoreInst>(outgoingT));
  }

  auto loopCarried = true;
  if (isMemoryAccessIntoDifferentArrays(edge)
      || isBackedgeIntoSameGlobal(edge)) {
    loopCarried = false;
  }

  if (!loopCarried) {
    // NOTE: We are actually removing must dependencies, but only those that are
    // backedges where by the next iteration, the access is at a different
    // memory location assert(!edge->isMustDependence()
    //  && "LLVM AA states load store pair is a must dependence! Bad
    //  PDGAnalysis.");
    if (verbose >= PDGVerbosity::Maximal) {
      errs() << "PDGAnalysis:  Memory dependence removed! From - to:\n";
      outgoingT->print(errs() << "PDGAnalysis:  Outgoing: ");
      errs() << "\n";
      incomingT->print(errs() << "PDGAnalysis:  Incoming: ");
      errs() << "\n";
    }
  }
  return !loopCarried;
}

bool PDGAnalysis::isBackedgeIntoSameGlobal(DGEdge<Value, Value> *edge) {
  auto access1 = allocAA->getPrimitiveArrayAccess(edge->getSrc());
  auto access2 = allocAA->getPrimitiveArrayAccess(edge->getDst());

  /*
   * Ensure the same global variable is accessed by the edge values
   */
  auto array1 = access1.first;
  auto array2 = access2.first;
  if (!array1 || !isa<GlobalValue>(array1))
    return false;
  if (array1 != array2)
    return false;

  /*
   * Ensure either of the following:
   *  1) two load accesses using the same IV governed GEP
   *  2) a store into the GEP and a load of the entire GV
   */
  auto GEP1 = access1.second;
  auto GEP2 = access2.second;
  if (GEP1 && !allocAA->areGEPIndicesConstantOrIV(GEP1))
    return false;
  if (GEP2 && !allocAA->areGEPIndicesConstantOrIV(GEP2))
    return false;
  if (GEP1 && GEP2) {
    if (!allocAA->areIdenticalGEPAccessesInSameLoop(GEP1, GEP2))
      return false;
    if (!isa<LoadInst>(edge->getSrc()) || !isa<LoadInst>(edge->getDst()))
      return false;
  } else if (GEP1) {
    if (!isa<StoreInst>(edge->getSrc()) || !isa<LoadInst>(edge->getDst()))
      return false;
  } else if (GEP2) {
    if (!isa<LoadInst>(edge->getSrc()) || !isa<StoreInst>(edge->getDst()))
      return false;
  } else
    return false;

  /*
   * Ensure that the edge is a backedge
   */
  auto outgoingI = (Instruction *)(edge->getSrc());
  auto incomingI = (Instruction *)(edge->getDst());
  if (canPrecedeInCurrentIteration(outgoingI, incomingI)) {
    return false;
  }

  return true;
}

bool PDGAnalysis::isMemoryAccessIntoDifferentArrays(
    DGEdge<Value, Value> *edge) {
  Value *array1 = allocAA->getPrimitiveArrayAccess(edge->getSrc()).first;
  Value *array2 = allocAA->getPrimitiveArrayAccess(edge->getDst()).first;
  return (array1 && array2 && array1 != array2);
}

bool PDGAnalysis::canPrecedeInCurrentIteration(Instruction *from,
                                               Instruction *to) {
  auto &LI =
      getAnalysis<LoopInfoWrapperPass>(*from->getFunction()).getLoopInfo();
  BasicBlock *fromBB = from->getParent();
  BasicBlock *toBB = to->getParent();
  auto loop = LI.getLoopFor(fromBB);
  BasicBlock *headerBB = nullptr;
  if (loop)
    headerBB = loop->getHeader();

  if (fromBB == toBB) {
    for (auto &I : *fromBB) {
      if (&I == from)
        return true;
      if (&I == to)
        return false;
    }
  }

  std::queue<BasicBlock *> bbToTraverse;
  std::set<BasicBlock *> bbReached;
  auto traverseOn = [&](BasicBlock *bb) -> void {
    bbToTraverse.push(bb);
    bbReached.insert(bb);
  };
  traverseOn(toBB);

  while (!bbToTraverse.empty()) {
    auto bb = bbToTraverse.front();
    bbToTraverse.pop();
    if (bb == fromBB)
      return true;
    if (bb == headerBB)
      continue;

    for (auto predBB : make_range(pred_begin(bb), pred_end(bb))) {
      if (bbReached.find(predBB) == bbReached.end()) {
        traverseOn(predBB);
      }
    }
  }

  return false;
}

bool PDGAnalysis::edgeIsAlongNonMemoryWritingFunctions(
    DGEdge<Value, Value> *edge) {

  /*
   * Check if this is a memory dependence.
   */
  if (!isa<MemoryDependence<Value, Value>>(edge)) {
    return false;
  }

  /*
   * Fetch the source and destination of the dependence.
   */
  auto outgoingT = edge->getSrc();
  auto incomingT = edge->getDst();

  /*
   * Auxiliary code.
   */
  auto isFunctionMemoryless = [&](StringRef funcName) -> bool {
    auto isMemoryless = allocAA->isMemoryless(funcName);
    return isMemoryless;
  };
  auto isFunctionNonWriting = [&](StringRef funcName) -> bool {
    if (isFunctionMemoryless(funcName)) {
      return true;
    }
    if (allocAA->isReadOnly(funcName)) {
      return true;
    }
    return false;
  };
  auto getCallFnName = [&](CallInst *call) -> StringRef {
    /*
     * Fetch the function being called
     */
    auto func = call->getCalledFunction();
    if (!func) {
      return "";
    }
    assert(func != nullptr);

    /*
     * Get the name of the callee
     */
    return func->getName();
  };

  /*
   * Handle the case both instructions are calls.
   */
  if (true && isa<CallInst>(outgoingT) && isa<CallInst>(incomingT)) {

    /*
     * If both callees do not write memory, then there is no memory dependence.
     */
    if (!isFunctionNonWriting(getCallFnName(cast<CallInst>(outgoingT))))
      return false;
    if (!isFunctionNonWriting(getCallFnName(cast<CallInst>(incomingT))))
      return false;
    return true;
  }

  /*
   * Handle the case where both instructions are not call.
   */
  if (true && (!isa<CallInst>(outgoingT)) && (!isa<CallInst>(incomingT))) {
    return false;
  }

  /*
   * Handle the case where just one of the instruction is a call.
   */
  CallInst *call;
  Value *mem;
  if (isa<CallInst>(outgoingT)) {
    call = cast<CallInst>(outgoingT);
    mem = incomingT;
  } else {
    assert(isa<CallInst>(incomingT));
    call = cast<CallInst>(incomingT);
    mem = outgoingT;
  }
  auto callName = getCallFnName(call);
  if (true && isa<LoadInst>(mem) && isFunctionNonWriting(callName)) {
    return true;
  }
  if (true && isa<StoreInst>(mem) && isFunctionMemoryless(callName)) {
    return true;
  }

  return false;
}

void PDGAnalysis::addAnalysis(DependenceAnalysis *a) {
  this->ddAnalyses.insert(a);

  return;
}

void PDGAnalysis::addAnalysis(CallGraphAnalysis *a) {
  this->cgAnalyses.insert(a);
}

PDGAnalysis::~PDGAnalysis() {
  if (this->programDependenceGraph) {
    delete this->programDependenceGraph;
  }
}

} // namespace arcana::noelle
