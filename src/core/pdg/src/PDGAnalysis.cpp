/*
 * Copyright 2016 - 2020  Angelo Matni, Yian Su, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "SystemHeaders.hpp"

#include "Util/SVFModule.h"
#include "WPA/Andersen.h"
#include "TalkDown.hpp"
#include "PDGPrinter.hpp"
#include "PDGAnalysis.hpp"

using namespace llvm;

PDGAnalysis::PDGAnalysis()
  : ModulePass{ID}
    , M{nullptr}
    , programDependenceGraph{nullptr}
    , CGUnderMain{}
    , dfa{}
    , embedPDG{false}
    , dumpPDG{false}
    , performThePDGComparison{false}
    , disableSVF{false}
    , disableAllocAA{false}
    , disableRA{false}
    , printer{} 
  {

  return ;
}

void PDGAnalysis::initializeSVF(Module &M) {
  SVFModule svfModule{M};
  this->pta = new AndersenWaveDiff();
  this->pta->analyze(svfModule);
  this->callGraph = this->pta->getPTACallGraph();
  this->mssa = new MemSSA((BVDataPTAImpl *)this->pta, false);

  return;
}

void PDGAnalysis::releaseMemory () {
  if (this->programDependenceGraph) delete this->programDependenceGraph;
  this->programDependenceGraph = nullptr;

  for (auto functionFDGPair : this->functionToFDGMap) {
    auto fdg = functionFDGPair.second;
    delete fdg;
  }
  this->functionToFDGMap.clear();

  return ;
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
    errs() << "Reachable external functions of " << pair.first->getName() << "\n";
    for (auto &external : pair.second) {
      errs() << "\t" << external->getName() << "\n";
    }
  }

  return;
}

PDG * PDGAnalysis::getFunctionPDG (Function &F) {

  /*
   * If the module PDG has been built, take the subset related to the input function
   * Else, construct the function DG from scratch (or from metadata)
   */
  PDG *pdg = nullptr;
  if (this->programDependenceGraph){

    /*
     * Check and get/update the function cache
     */
    if (this->functionToFDGMap.find(&F) == this->functionToFDGMap.end()) {
      pdg = this->programDependenceGraph->createFunctionSubgraph(F);
      this->functionToFDGMap.insert(std::make_pair(&F, pdg));
    } else {
      pdg = this->functionToFDGMap.at(&F);
    }

  } else {

    /*
     * Check and get/update the function cache
     */
    if (this->functionToFDGMap.find(&F) == this->functionToFDGMap.end()) {

      /*
       * Determine whether metadata can be used to construct the graph
       */
      if (this->hasPDGAsMetadata(*this->M)) {
        pdg = constructFunctionDGFromMetadata(F);
      } else {
        pdg = constructFunctionDGFromAnalysis(F);
      }
      this->functionToFDGMap.insert(std::make_pair(&F, pdg));

    } else {
      pdg = this->functionToFDGMap.at(&F);
    }

  }

  /*
   * Print the PDG
   */
  if (this->dumpPDG){
    this->printer.printGraphsForFunction(F, pdg, getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo());
  }

  return pdg;
}

PDG * PDGAnalysis::getPDG (void){

  /*
   * Check if we have already built the PDG.
   */
  if (this->programDependenceGraph){
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
    if (this->performThePDGComparison){
      auto PDGFromAnalysis = constructPDGFromAnalysis(*this->M);
      auto arePDGsEquivalent = this->comparePDGs(PDGFromAnalysis, this->programDependenceGraph);
      if (!arePDGsEquivalent){
        errs() << "PDGAnalysis: Error = PDGs constructed are not the same";
        abort();
      }
      delete PDGFromAnalysis ;
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
    if (this->embedPDG){
      embedPDGAsMetadata(this->programDependenceGraph);
      if (this->performThePDGComparison){
        auto PDGFromMetadata = this->constructPDGFromMetadata(*this->M);
        auto arePDGsEquivalen = this->comparePDGs(this->programDependenceGraph, PDGFromMetadata);
        if (!arePDGsEquivalen){
          errs() << "PDGAnalysis: Error = PDGs constructed are not the same";
          abort();
        }
        delete PDGFromMetadata;
      }
    }
  }

  return this->programDependenceGraph;
}

bool PDGAnalysis::hasPDGAsMetadata(Module &M) {
  if (auto n = M.getNamedMetadata("noelle.module.pdg")) {
    if (auto m = dyn_cast<MDNode>(n->getOperand(0))) {
      if (cast<MDString>(m->getOperand(0))->getString() == "true") {
        return true;
      }
    }
  }

  return false;
}

PDG * PDGAnalysis::constructPDGFromAnalysis(Module &M) {
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

PDG * PDGAnalysis::constructFunctionDGFromAnalysis(Function &F) {
  if (verbose >= PDGVerbosity::Maximal) {
    errs() << "PDGAnalysis: Construct function DG from Analysis\n";
  }

  auto pdg = new PDG(F);
  constructEdgesFromUseDefs(pdg);
  constructEdgesFromAliasesForFunction(pdg, F);
  constructEdgesFromControlForFunction(pdg, F);

  return pdg;
}

PDG * PDGAnalysis::constructPDGFromMetadata(Module &M) {
  if (verbose >= PDGVerbosity::Maximal) {
    errs() << "PDGAnalysis: Construct PDG from Metadata\n";
  }

  /*
   * Create the PDG.
   */
  auto pdg = new PDG(M);

  /*
   * Fill up the PDG.
   */
  std::unordered_map<MDNode *, Value *> IDNodeMap;
  for (auto &F : M) {
    constructNodesFromMetadata(pdg, F, IDNodeMap);
    constructEdgesFromMetadata(pdg, F, IDNodeMap);
  }

  return pdg;
}

PDG * PDGAnalysis::constructFunctionDGFromMetadata(Function &F) {
  if (verbose >= PDGVerbosity::Maximal) {
    errs() << "PDGAnalysis: Construct function DG from Metadata\n";
  }

  auto pdg = new PDG(F);
  std::unordered_map<MDNode *, Value *> IDNodeMap;
  constructNodesFromMetadata(pdg, F, IDNodeMap);
  constructEdgesFromMetadata(pdg, F, IDNodeMap);
  return pdg;
}

void PDGAnalysis::constructNodesFromMetadata(PDG *pdg, Function &F, unordered_map<MDNode *, Value *> &IDNodeMap) {

  /*
   * Construct id to node map and add nodes of arguments to pdg
   */
  if (MDNode *argsM = F.getMetadata("noelle.pdg.args.id")) {
    for (auto &arg : F.args()) {
      if (MDNode *m = dyn_cast<MDNode>(argsM->getOperand(arg.getArgNo()))) {
        IDNodeMap[m] = &arg;
      }
    }
  }

  /*
   * Construct id to node map and add nodes of instructions to pdg
   */
  for (auto &B : F) {
    for (auto &I : B) {
      if (MDNode *m = I.getMetadata("noelle.pdg.inst.id")) {
        IDNodeMap[m] = &I;
      }
    }
  }

  return;
}

void PDGAnalysis::constructEdgesFromMetadata(PDG *pdg, Function &F, unordered_map<MDNode *, Value *> &IDNodeMap) {

  /*
   * Construct edges and set attributes
   */
  if (MDNode *edgesM = F.getMetadata("noelle.pdg.edges")) {
    for (auto &operand : edgesM->operands()) {
      if (MDNode *edgeM = dyn_cast<MDNode>(operand)) {
        auto edge = constructEdgeFromMetadata(pdg, edgeM, IDNodeMap);
  
        /*
         * Construct subEdges and set attributes
         */        
        if (MDNode *subEdgesM = dyn_cast<MDNode>(edgeM->getOperand(8))) {
          for (auto &subOperand : subEdgesM->operands()) {
            if (MDNode *subEdgeM = dyn_cast<MDNode>(subOperand)) {
              DGEdge<Value> *subEdge = constructEdgeFromMetadata(pdg, subEdgeM, IDNodeMap);
              edge->addSubEdge(subEdge);
            }
          }
        }
        
        /*
         * Add edge to pdg
         */ 
        pdg->copyAddEdge(*edge);

        /*
         * Free the memory.
         */
        delete edge;
      }
    }
  }

  return;
}

DGEdge<Value> * PDGAnalysis::constructEdgeFromMetadata(PDG *pdg, MDNode *edgeM, unordered_map<MDNode *, Value *> &IDNodeMap) {
  DGEdge<Value> *edge = nullptr;

  if (MDNode *fromM = dyn_cast<MDNode>(edgeM->getOperand(0))) {
    if (MDNode *toM = dyn_cast<MDNode>(edgeM->getOperand(1))) {
      Value *from = IDNodeMap[fromM];
      Value *to = IDNodeMap[toM];
      edge = new DGEdge<Value>(pdg->fetchNode(from), pdg->fetchNode(to));
      edge->setEdgeAttributes(
        cast<MDString>(cast<MDNode>(edgeM->getOperand(2))->getOperand(0))->getString() == "true",
        cast<MDString>(cast<MDNode>(edgeM->getOperand(3))->getOperand(0))->getString() == "true",
        cast<MDString>(cast<MDNode>(edgeM->getOperand(4))->getOperand(0))->getString().str(),
        cast<MDString>(cast<MDNode>(edgeM->getOperand(5))->getOperand(0))->getString() == "true",
        cast<MDString>(cast<MDNode>(edgeM->getOperand(6))->getOperand(0))->getString() == "true",
        cast<MDString>(cast<MDNode>(edgeM->getOperand(7))->getOperand(0))->getString() == "true"
      );
    }
  }

  return edge;
}

MDNode * PDGAnalysis::getEdgeMetadata(DGEdge<Value> *edge, LLVMContext &C, unordered_map<Value *, MDNode *> &nodeIDMap) {
  Metadata *edgeM[] = {
    nodeIDMap[edge->getOutgoingT()],
    nodeIDMap[edge->getIncomingT()],
    MDNode::get(C, MDString::get(C, edge->isMemoryDependence() ? "true" : "false")),
    MDNode::get(C, MDString::get(C, edge->isMustDependence() ? "true" : "false")),
    MDNode::get(C, MDString::get(C, edge->dataDepToString())),
    MDNode::get(C, MDString::get(C, edge->isControlDependence() ? "true" : "false")),
    MDNode::get(C, MDString::get(C, edge->isLoopCarriedDependence() ? "true" : "false")),
    MDNode::get(C, MDString::get(C, edge->isRemovableDependence() ? "true" : "false")),
    getSubEdgesMetadata(edge, C, nodeIDMap)
  };

  return MDNode::get(C, edgeM);
}

MDNode * PDGAnalysis::getSubEdgesMetadata(DGEdge<Value> *edge, LLVMContext &C, unordered_map<Value *, MDNode *> &nodeIDMap) {
  vector<Metadata *> subEdgesVec;

  for (auto &subEdge : edge->getSubEdges()) {
    Metadata *subEdgeM[] = {
      nodeIDMap[subEdge->getOutgoingT()],
      nodeIDMap[subEdge->getIncomingT()],
      MDNode::get(C, MDString::get(C, edge->isMemoryDependence() ? "true" : "false")),
      MDNode::get(C, MDString::get(C, edge->isMustDependence() ? "true" : "false")),
      MDNode::get(C, MDString::get(C, edge->dataDepToString())),
      MDNode::get(C, MDString::get(C, edge->isControlDependence() ? "true" : "false")),
      MDNode::get(C, MDString::get(C, edge->isLoopCarriedDependence() ? "true" : "false")),
      MDNode::get(C, MDString::get(C, edge->isRemovableDependence() ? "true" : "false")),
    };
    subEdgesVec.push_back(MDNode::get(C, subEdgeM));
  }

  return MDTuple::get(C, subEdgesVec);
}

void PDGAnalysis::trimDGUsingCustomAliasAnalysis (PDG *pdg) {

  /*
   * Fetch AllocAA
   */
  collectCGUnderFunctionMain(*this->M);
  this->allocAA = &getAnalysis<AllocAA>();
  if (this->disableAllocAA){
    return ;
  }

  /*
   * Invoke AllocAA
   */
  removeEdgesNotUsedByParSchemes(pdg);

  /*
   * Invoke the TalkDown
   */
  auto& talkDown = getAnalysis<TalkDown>();
  //TODO

  return ;
}

void PDGAnalysis::collectCGUnderFunctionMain (Module &M) {
  auto main = M.getFunction("main");
  auto &callGraph = getAnalysis<CallGraphWrapperPass>().getCallGraph();
  std::queue<Function *> funcToTraverse;
  std::set<Function *> reached;
  funcToTraverse.push(main);
  reached.insert(main);
  while (!funcToTraverse.empty()) {
    auto func = funcToTraverse.front();
    funcToTraverse.pop();

    auto funcCGNode = callGraph[func];
    for (auto &callRecord : make_range(funcCGNode->begin(), funcCGNode->end())) {
      auto F = callRecord.second->getFunction();
      if (!F || F->empty()) continue;

      if (reached.find(F) != reached.end()) continue;
      reached.insert(F);
      funcToTraverse.push(F);
    }
  }

  CGUnderMain.clear();
  CGUnderMain.insert(reached.begin(), reached.end());

  return ;
}

void PDGAnalysis::constructEdgesFromUseDefs (PDG *pdg){
  for (auto node : make_range(pdg->begin_nodes(), pdg->end_nodes())) {
    auto pdgValue = node->getT();
    if (pdgValue->getNumUses() == 0)
      continue;

    for (auto& U : pdgValue->uses()) {
      auto user = U.getUser();

      if (isa<Instruction>(user) || isa<Argument>(user)) {
        auto edge = pdg->addEdge(pdgValue, user);
        edge->setMemMustType(false, true, DG_DATA_NONE);
      }
    }
  }
}

void PDGAnalysis::constructEdgesFromAliases (PDG *pdg, Module &M){

  /*
   * Use alias analysis on stores, loads, and function calls to construct PDG edges
   */
  for (auto &F : M) {

    /*
     * Check if the function has a body.
     */
    if (F.empty()) continue ;

    /*
     * Add the edges to the PDG.
     */
    constructEdgesFromAliasesForFunction(pdg, F);
  }
}

void PDGAnalysis::constructEdgesFromAliasesForFunction (PDG *pdg, Function &F){

  /*
   * Fetch the alias analysis.
   */
  auto &AA = getAnalysis<AAResultsWrapperPass>(F).getAAResults();

  /*
   * Run the reachable analysis.
   */
  auto onlyMemoryInstructionFilter = [](Instruction *i) -> bool {
    if (isa<LoadInst>(i)){
      return true;
    }
    if (isa<StoreInst>(i)){
      return true;
    }
    if (isa<CallInst>(i)){
      return true;
    }
    if (isa<InvokeInst>(i)){
      return true;
    }
    return false;
  };
  auto dfr = this->disableRA ? this->dfa.getFullSets(&F) : this->dfa.runReachableAnalysis(&F, onlyMemoryInstructionFilter);

  for (auto &B : F) {
    for (auto &I : B) {
      if (auto store = dyn_cast<StoreInst>(&I)) {
        iterateInstForStore(pdg, F, AA, dfr, store);
      } else if (auto load = dyn_cast<LoadInst>(&I)) {
        iterateInstForLoad(pdg, F, AA, dfr, load);
      } else if (auto call = dyn_cast<CallInst>(&I)) {
        iterateInstForCall(pdg, F, AA, dfr, call);
      }
    }
  }

  /*
   * Free the memory.
   */
  delete dfr;
}

void PDGAnalysis::iterateInstForCall (PDG *pdg, Function &F, AAResults &AA, DataFlowResult *dfr, CallInst *call) {

  for (auto I : dfr->OUT(call)) {

    /*
     * Check stores.
     */
    if (auto store = dyn_cast<StoreInst>(I)) {
      addEdgeFromFunctionModRef(pdg, F, AA, call, store, true);
      continue ;
    }

    /*
     * Check loads.
     */
    if (auto load = dyn_cast<LoadInst>(I)) {
      addEdgeFromFunctionModRef(pdg, F, AA, call, load, true);
      continue ;
    }

    /*
     * Check calls.
     */
    if (auto otherCall = dyn_cast<CallInst>(I)) {
      addEdgeFromFunctionModRef(pdg, F, AA, call, otherCall);
      continue ;
    }
  }

  return ;
}

void PDGAnalysis::removeEdgesNotUsedByParSchemes (PDG *pdg) {
  std::set<DGEdge<Value> *> removeEdges;

  /*
   * Collect the edges in the PDG that can be safely removed.
   */
  for (auto edge : pdg->getEdges()) {

    /*
     * Fetch the source of the dependence.
     */
    auto source = edge->getOutgoingT();
    if (!isa<Instruction>(source)) continue;

    /*
     * Check if the function of the dependence destiation cannot be reached from main.
     */
    auto F = cast<Instruction>(source)->getFunction();
    if (CGUnderMain.find(F) == CGUnderMain.end()) continue;

    if (  false
        || edgeIsNotLoopCarriedMemoryDependency(edge)
        || edgeIsAlongNonMemoryWritingFunctions(edge)
      ) {
      removeEdges.insert(edge);
    }
  }

  /*
   * Remove the tagged edges.
   */
  for (auto edge : removeEdges) {
    pdg->removeEdge(edge);
  }

  return ;
}

// NOTE: Loads between random parts of separate GVs and both edges between GVs should be removed
bool PDGAnalysis::edgeIsNotLoopCarriedMemoryDependency (DGEdge<Value> *edge) {

  /*
   * Check if this is a memory dependence.
   */
  if (!edge->isMemoryDependence()) {
    return false;
  }

  /*
   * Fetch the source and destination of the dependence.
   */
  auto outgoingT = edge->getOutgoingT();
  auto incomingT = edge->getIncomingT();

  /*
   * Handle only memory instructions.
   */
  if (isa<CallInst>(outgoingT) || isa<CallInst>(incomingT)) {
    return false;
  }

  /*
   * Assert: must be a WAR load-store OR a RAW store-load
   */
  LoadInst *load = nullptr;
  StoreInst *store = nullptr;
  if (edge->isWARDependence()) {
    assert(isa<StoreInst>(incomingT) && isa<LoadInst>(outgoingT));
    load = (LoadInst*)outgoingT;
    store = (StoreInst*)incomingT;
  } else if (edge->isRAWDependence()) {
    assert(isa<LoadInst>(incomingT) && isa<StoreInst>(outgoingT));
    store = (StoreInst*)outgoingT;
    load = (LoadInst*)incomingT;
  }

  bool loopCarried = true;
  if (isMemoryAccessIntoDifferentArrays(edge) ||
      // (store && load && isBackedgeOfLoadStoreIntoSameOffsetOfArray(edge, load, store)) ||
      isBackedgeIntoSameGlobal(edge)) {
    loopCarried = false;
  }

  if (!loopCarried) {
    // NOTE: We are actually removing must dependencies, but only those that are
    // backedges where by the next iteration, the access is at a different memory location
    // assert(!edge->isMustDependence()
    //  && "LLVM AA states load store pair is a must dependence! Bad PDGAnalysis.");
    if (verbose >= PDGVerbosity::Maximal) {
      errs() << "PDGAnalysis:  Memory dependence removed! From - to:\n";
      outgoingT->print(errs() << "PDGAnalysis:  Outgoing: "); errs() << "\n";
      incomingT->print(errs() << "PDGAnalysis:  Incoming: "); errs() << "\n";
    }
  }
  return !loopCarried;
}

bool PDGAnalysis::isBackedgeOfLoadStoreIntoSameOffsetOfArray (
  DGEdge<Value> *edge,
  LoadInst *load,
  StoreInst *store
) {
  auto access1 = allocAA->getPrimitiveArrayAccess(load);
  auto access2 = allocAA->getPrimitiveArrayAccess(store);

  auto gep1 = access1.second;
  auto gep2 = access2.second;
  if (!gep1 || !gep2) return false;
  if (!allocAA->areIdenticalGEPAccessesInSameLoop(gep1, gep2)) return false;;
  if (!allocAA->areGEPIndicesConstantOrIV(gep1)) return false;

  auto outgoingI = (Instruction*)(edge->getOutgoingT());
  auto incomingI = (Instruction*)(edge->getIncomingT());
  if (canPrecedeInCurrentIteration(outgoingI, incomingI)) {
    return false;
  }

  return true;
}

bool PDGAnalysis::isBackedgeIntoSameGlobal (
  DGEdge<Value> *edge
) {
  auto access1 = allocAA->getPrimitiveArrayAccess(edge->getOutgoingT());
  auto access2 = allocAA->getPrimitiveArrayAccess(edge->getIncomingT());

  /*
   * Ensure the same global variable is accessed by the edge values
   */
  auto array1 = access1.first;
  auto array2 = access2.first;
  if (!array1 || !isa<GlobalValue>(array1)) return false;
  if (array1 != array2) return false;

  /*
   * Ensure either of the following:
   *  1) two load accesses using the same IV governed GEP
   *  2) a store into the GEP and a load of the entire GV
   */
  auto GEP1 = access1.second;
  auto GEP2 = access2.second;
  if (GEP1 && !allocAA->areGEPIndicesConstantOrIV(GEP1)) return false;
  if (GEP2 && !allocAA->areGEPIndicesConstantOrIV(GEP2)) return false;
  if (GEP1 && GEP2) {
    if (!allocAA->areIdenticalGEPAccessesInSameLoop(GEP1, GEP2)) return false;
    if (!isa<LoadInst>(edge->getOutgoingT()) ||
        !isa<LoadInst>(edge->getIncomingT())) return false;
  } else if (GEP1) {
    if (!isa<StoreInst>(edge->getOutgoingT()) ||
        !isa<LoadInst>(edge->getIncomingT())) return false;
  } else if (GEP2) {
    if (!isa<LoadInst>(edge->getOutgoingT()) ||
        !isa<StoreInst>(edge->getIncomingT())) return false;
  } else return false;

  /*
   * Ensure that the edge is a backedge
   */
  auto outgoingI = (Instruction*)(edge->getOutgoingT());
  auto incomingI = (Instruction*)(edge->getIncomingT());
  if (canPrecedeInCurrentIteration(outgoingI, incomingI)) {
    return false;
  }

  return true;
}

bool PDGAnalysis::isMemoryAccessIntoDifferentArrays (DGEdge<Value> *edge) {
  Value *array1 = allocAA->getPrimitiveArrayAccess(edge->getOutgoingT()).first;
  Value *array2 = allocAA->getPrimitiveArrayAccess(edge->getIncomingT()).first;
  return (array1 && array2 && array1 != array2);
}

bool PDGAnalysis::canPrecedeInCurrentIteration (Instruction *from, Instruction *to) {
  auto &LI = getAnalysis<LoopInfoWrapperPass>(*from->getFunction()).getLoopInfo();
  BasicBlock *fromBB = from->getParent();
  BasicBlock *toBB = to->getParent();
  auto loop = LI.getLoopFor(fromBB);
  BasicBlock *headerBB = nullptr;
  if (loop) headerBB = loop->getHeader();

  if (fromBB == toBB) {
    for (auto &I : *fromBB) {
      if (&I == from) return true;
      if (&I == to) return false;
    }
  }

  std::queue<BasicBlock *> bbToTraverse;
  std::set<BasicBlock *> bbReached;
  auto traverseOn = [&](BasicBlock *bb) -> void {
    bbToTraverse.push(bb); bbReached.insert(bb);
  };
  traverseOn(toBB);

  while (!bbToTraverse.empty()) {
    auto bb = bbToTraverse.front();
    bbToTraverse.pop();
    if (bb == fromBB) return true;
    if (bb == headerBB) continue;

    for (auto predBB : make_range(pred_begin(bb), pred_end(bb))) {
      if (bbReached.find(predBB) == bbReached.end()) {
        traverseOn(predBB);
      }
    }
  }

  return false;
}

bool PDGAnalysis::edgeIsAlongNonMemoryWritingFunctions (DGEdge<Value> *edge) {

  /*
   * Check if this is a memory dependence.
   */
  if (!edge->isMemoryDependence()) {
    return false;
  }

  /*
   * Fetch the source and destination of the dependence.
   */
  auto outgoingT = edge->getOutgoingT();
  auto incomingT = edge->getIncomingT();

  /*
   * Auxiliary code.
   */
  auto isFunctionMemoryless = [&](StringRef funcName) -> bool {
    auto isMemoryless = allocAA->isMemoryless(funcName);
    return isMemoryless;
  };
  auto isFunctionNonWriting = [&](StringRef funcName) -> bool {
    if (isFunctionMemoryless(funcName)){
      return true;
    }
    if (allocAA->isReadOnly(funcName)){
      return true;
    }
    return false;
  };
  auto getCallFnName = [&](CallInst *call) -> StringRef {
    auto func = call->getCalledFunction();
    if (func && !func->empty()) {
      return func->getName();
    }
    return call->getCalledValue()->getName();
  };

  /*
   * Handle the case both instructions are calls.
   */
  if (  true
        && isa<CallInst>(outgoingT) 
        && isa<CallInst>(incomingT)
    ) {

    /*
     * If both callees do not write memory, then there is no memory dependence.
     */
    if (!isFunctionNonWriting(getCallFnName(cast<CallInst>(outgoingT)))) return false;
    if (!isFunctionNonWriting(getCallFnName(cast<CallInst>(incomingT)))) return false;
    return true;
  }

  /*
   * Handle the case where both instructions are not call.
   */
  if (  true
        && (!isa<CallInst>(outgoingT))
        && (!isa<CallInst>(incomingT))
    ) {
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
  return isa<LoadInst>(mem) && isFunctionNonWriting(callName)
    || isa<StoreInst>(mem) && isFunctionMemoryless(callName);
}
      
PDGAnalysis::~PDGAnalysis() {
  if (this->programDependenceGraph)
    delete this->programDependenceGraph;

  for (auto functionFDGPair : this->functionToFDGMap) {
    auto fdg = functionFDGPair.second;
    delete fdg;
  }
  this->functionToFDGMap.clear();
}
