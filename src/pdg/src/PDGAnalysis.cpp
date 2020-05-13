/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "SystemHeaders.hpp"

#include "WPA/WPAPass.h"
#include "TalkDown.hpp"
#include "PDGPrinter.hpp"
#include "PDGAnalysis.hpp"

static cl::opt<int> Verbose("noelle-pdg-verbose", cl::ZeroOrMore, cl::Hidden, cl::desc("Verbose output (0: disabled, 1: minimal, 2: maximal, 3:maximal plus dumping PDG"));

using namespace llvm;

bool llvm::PDGAnalysis::doInitialization (Module &M){
  this->verbose = static_cast<PDGVerbosity>(Verbose.getValue());
  return false;
}

void llvm::PDGAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<AAResultsWrapperPass>();
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<PostDominatorTreeWrapperPass>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
  AU.addRequired<CallGraphWrapperPass>();
  AU.addRequired<AllocAA>();
  AU.addRequired<TalkDown>();
  AU.addRequired<WPAPass>();
  AU.setPreservesAll();
  return ;
}

bool llvm::PDGAnalysis::runOnModule (Module &M){

  /*
   * Store global information.
   */
  this->M = &M;

  /*
   * Check if we should print the PDG
   */
  if (this->verbose >= PDGVerbosity::MaximalAndPDG){

    /*
     * Print the PDG
     */
    auto currentPDG = this->getPDG();
    auto localPDGPrinter = new PDGPrinter();
    auto &callGraph = getAnalysis<CallGraphWrapperPass>().getCallGraph();
    auto getLoopInfo = [this](Function *f) -> LoopInfo& {
      auto& LI = getAnalysis<LoopInfoWrapperPass>(*f).getLoopInfo();
      return LI;
    };
    localPDGPrinter->printPDG(M, callGraph, currentPDG, getLoopInfo);
  }

  return false;
}

llvm::PDGAnalysis::PDGAnalysis()
  : ModulePass{ID}, M{nullptr}, programDependenceGraph{nullptr}, CGUnderMain{}, printer{} {
  return ;
}

llvm::PDGAnalysis::~PDGAnalysis() {
  if (this->programDependenceGraph)
    delete this->programDependenceGraph;
}

llvm::PDG * PDGAnalysis::getFunctionPDG (Function &F) {

  /*
   * Make sure the module PDG has been constructed.
   */
  if (!this->programDependenceGraph){
    this->getPDG();
  }

  /*
   * The module PDG has been built.
   *
   * Take the subset related to the function given as input.
   */
  auto pdg = this->programDependenceGraph->createFunctionSubgraph(F);

  /*
   * Print the PDG
   */
  if (this->verbose >= PDGVerbosity::Maximal){
    this->printer.printGraphsForFunction(F, pdg, getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo());
  }

  return pdg;
}

llvm::PDG * llvm::PDGAnalysis::getPDG (){
  if (this->programDependenceGraph)
    delete this->programDependenceGraph;

  if (hasPDGAsMetadata(*this->M)) {
    this->programDependenceGraph = constructPDGFromMetadata(*this->M);
    assert(comparePDGs(constructPDGFromAnalysis(*this->M), this->programDependenceGraph) && "PDGs constructed are not the same");
  }
  else {
    this->programDependenceGraph = constructPDGFromAnalysis(*this->M);
    embedPDGAsMetadata(this->programDependenceGraph);
    assert(comparePDGs(this->programDependenceGraph, constructPDGFromMetadata(*this->M)) && "PDGs constructed are not the same");
  }

  return this->programDependenceGraph;
}

bool llvm::PDGAnalysis::comparePDGs(PDG *pdg1, PDG *pdg2) {
  return compareNodes(pdg1, pdg2) && compareEdges(pdg1, pdg2);
}

bool llvm::PDGAnalysis::compareNodes(PDG *pdg1, PDG *pdg2) {
  errs() << "Compare PDG Nodes\n";

  if (pdg1->numNodes() != pdg2->numNodes()) {
    errs() << "number of pdg nodes are not the same\n";
    return false;
  }

  for (auto &node : pdg1->getNodes()) {
    if (pdg2->fetchNode(node->getT()) == nullptr) {
      return false;
    }
  }

  return true;
}

bool llvm::PDGAnalysis::compareEdges(PDG *pdg1, PDG *pdg2) {
  errs() << "Compare PDG Edges\n";

  if (pdg1->numEdges() != pdg2->numEdges()) {
    errs() << "number of pdg edges are not the same\n";
    return false;
  }

  for (auto &edge1 : pdg1->getEdges()) {
    DGNode<Value> *outgoingNode = pdg2->fetchNode(edge1->getOutgoingT());
    DGNode<Value> *incomingNode = pdg2->fetchNode(edge1->getIncomingT());
    if (!outgoingNode || !incomingNode) {
      return false;
    }
    set<DGEdge<Value> *> edgeSet = pdg2->fetchEdges(outgoingNode, incomingNode);
    if (edgeSet.empty()) {
      return false;
    }

    bool match = false;
    for (auto &edge2 : edgeSet) {
      if (edge1->isMemoryDependence() == edge2->isMemoryDependence() &&
          edge1->isMustDependence() == edge2->isMustDependence() &&
          edge1->isControlDependence() == edge2->isControlDependence() &&
          edge1->isLoopCarriedDependence() == edge2->isLoopCarriedDependence() &&
          edge1->isRemovableDependence() == edge2->isRemovableDependence() &&
          edge1->dataDependenceType() == edge2->dataDependenceType()) {
        match = true;
        break;
      }
    }
    if (!match) {
      return false;
    }
  }

  return true;
}

bool llvm::PDGAnalysis::hasPDGAsMetadata(Module &M) {
  errs() << "Check if PDG has been embeded as metadata\n";
  
  if (NamedMDNode *n = M.getNamedMetadata("module.pdg")) {
    if (MDNode *m = dyn_cast<MDNode>(n->getOperand(0))) {
      if (cast<MDString>(m->getOperand(0))->getString() == "true") {
        return true;
      }
    }
  }

  return false;
}

PDG * llvm::PDGAnalysis::constructPDGFromAnalysis(Module &M) {
  errs() << "Construct PDG from Analysis\n";

  PDG *pdg = new PDG(M);

  constructEdgesFromUseDefs(pdg);
  constructEdgesFromAliases(pdg, M);
  constructEdgesFromControl(pdg, M);

  trimDGUsingCustomAliasAnalysis(pdg);

  return pdg; 
}

PDG * llvm::PDGAnalysis::constructPDGFromMetadata(Module &M) {
  errs() << "Construct PDG from Metadata\n";

  PDG *pdg = new PDG(M);
  unordered_map<MDNode *, Value *> IDNodeMap;

  for (auto &F : M) {
    constructNodesFromMetadata(pdg, F, IDNodeMap);
    constructEdgesFromMetadata(pdg, F, IDNodeMap);
  }

  return pdg;
}

void llvm::PDGAnalysis::constructNodesFromMetadata(PDG *pdg, Function &F, unordered_map<MDNode *, Value *> &IDNodeMap) {
  /*
   * Construct id to node map and add nodes of arguments to pdg
   */
  if (MDNode *argsM = F.getMetadata("pdg.args.id")) {
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
      if (MDNode *m = I.getMetadata("pdg.inst.id")) {
        IDNodeMap[m] = &I;
      }
    }
  }

  return;
}

void llvm::PDGAnalysis::constructEdgesFromMetadata(PDG *pdg, Function &F, unordered_map<MDNode *, Value *> &IDNodeMap) {
  /*
   * Construct edges and set attributes
   */
  if (MDNode *edgesM = F.getMetadata("pdg.edges")) {
    for (auto &operand : edgesM->operands()) {
      if (MDNode *edgeM = dyn_cast<MDNode>(operand)) {
        DGEdge<Value> *edge = constructEdgeFromMetadata(pdg, edgeM, IDNodeMap);
  
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
      }
    }
  }

  return;
}

DGEdge<Value> * llvm::PDGAnalysis::constructEdgeFromMetadata(PDG *pdg, MDNode *edgeM, unordered_map<MDNode *, Value *> &IDNodeMap) {
  DGEdge<Value> *edge;  

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

void llvm::PDGAnalysis::embedPDGAsMetadata(PDG *pdg) {
  errs() << "Embed PDG as Metadata\n";

  LLVMContext &C = this->M->getContext();
  unordered_map<Value *, MDNode *> nodeIDMap;

  embedNodesAsMetadata(pdg, C, nodeIDMap);
  embedEdgesAsMetadata(pdg, C, nodeIDMap);

  NamedMDNode *n = this->M->getOrInsertNamedMetadata("module.pdg");
  n->addOperand(MDNode::get(C, MDString::get(C, "true")));

  return;
}

void llvm::PDGAnalysis::embedNodesAsMetadata(PDG *pdg, LLVMContext &C, unordered_map<Value *, MDNode *> &nodeIDMap) {
  uint64_t i = 0;
  unordered_map<Function *, unordered_map<uint64_t, Metadata *>> functionArgsIDMap;

  /*
   * Construct node to id map and embed metadata of instruction nodes to instruction
   */
  for (auto &node : pdg->getNodes()) {
    Value *v = node->getT();
    Constant *id = ConstantInt::get(Type::getInt64Ty(C), i++);
    MDNode *m = MDNode::get(C, ConstantAsMetadata::get(id));
    if (Argument *arg = dyn_cast<Argument>(v)) {
      functionArgsIDMap[arg->getParent()][arg->getArgNo()] = m;
    }
    else if (Instruction *inst = dyn_cast<Instruction>(v)) {
      inst->setMetadata("pdg.inst.id", m);
    }
    nodeIDMap[v] = m;
  }

  /*
   * Embed metadta of argument nodes to function
   */
  for (auto &funArgs : functionArgsIDMap) {
    vector<Metadata *> argsVec;
    for (uint64_t i = 0; i < funArgs.second.size(); i++) {
      argsVec.push_back(funArgs.second[i]);
    }

    MDNode *m = MDTuple::get(C, argsVec);
    funArgs.first->setMetadata("pdg.args.id", m);
  }

  return;
}

void llvm::PDGAnalysis::embedEdgesAsMetadata(PDG *pdg, LLVMContext &C, unordered_map<Value *, MDNode *> &nodeIDMap) {
  unordered_map<Function *, vector<Metadata *>> functionEdgesMap;

  /*
   * Construct edge metadata
   */
  for (auto &edge : pdg->getEdges()) {
    MDNode *edgeM = getEdgeMetadata(edge, C, nodeIDMap);
    if (Argument *arg = dyn_cast<Argument>(edge->getOutgoingT())) {
      functionEdgesMap[arg->getParent()].push_back(edgeM);
    }
    else if (Instruction *inst = dyn_cast<Instruction>(edge->getOutgoingT())) {
      functionEdgesMap[inst->getFunction()].push_back(edgeM);
    }
  }

  /*
   * Embed metadata of edges to function
   */
  for (auto &funEdge : functionEdgesMap) {
    MDNode *m = MDTuple::get(C, funEdge.second);
    funEdge.first->setMetadata("pdg.edges", m);
  }

  return;
}

MDNode * llvm::PDGAnalysis::getEdgeMetadata(DGEdge<Value> *edge, LLVMContext &C, unordered_map<Value *, MDNode *> &nodeIDMap) {
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

MDNode * llvm::PDGAnalysis::getSubEdgesMetadata(DGEdge<Value> *edge, LLVMContext &C, unordered_map<Value *, MDNode *> &nodeIDMap) {
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

void llvm::PDGAnalysis::trimDGUsingCustomAliasAnalysis (PDG *pdg) {

  /*
   * Invoke AllocAA
   */
  collectCGUnderFunctionMain(*this->M);
  this->allocAA = &getAnalysis<AllocAA>();
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

void llvm::PDGAnalysis::constructEdgesFromUseDefs (PDG *pdg){
  for (auto node : make_range(pdg->begin_nodes(), pdg->end_nodes())) {
    auto pdgValue = node->getT();
    if (pdgValue->getNumUses() == 0)
      continue;

    for (auto& U : pdgValue->uses()) {
      auto user = U.getUser();

      if (isa<Instruction>(user) || isa<Argument>(user)) {
        auto edge = pdg->addEdge(pdgValue, user);
        edge->setMemMustType(false, true, DG_DATA_RAW);
      }
    }
  }
}

template <class InstI, class InstJ>
void llvm::PDGAnalysis::addEdgeFromMemoryAlias (PDG *pdg, Function &F, AAResults &AA, InstI *memI, InstJ *memJ, bool WAW){
  auto makeEdge = false, must = false;

  /*
   * Query the LLVM alias analyses.
   */
  switch (AA.alias(MemoryLocation::get(memI), MemoryLocation::get(memJ))) {
    case PartialAlias:
    case MayAlias:
      makeEdge = true;
      break;
    case MustAlias:
      makeEdge = must = true;
    break;
  }
  if (!makeEdge) {
    return;
  }

  /*
   * Check other alias analyses
   */
  WPAPass &wpa = getAnalysis<WPAPass>();
  switch (wpa.alias(MemoryLocation::get(memI), MemoryLocation::get(memJ))) {
    case PartialAlias:
    case MayAlias:
      makeEdge = true;
      break;
    case MustAlias:
      makeEdge = must = true;
      break;
  }
  if (!makeEdge) {
    return;
  }

  /*
   * There is a dependence.
   */
  DataDependenceType dataDepType = WAW ? DG_DATA_WAW : DG_DATA_RAW;
  pdg->addEdge((Value*)memI, (Value*)memJ)->setMemMustType(true, must, dataDepType);

  dataDepType = WAW ? DG_DATA_WAW : DG_DATA_WAR;
  pdg->addEdge((Value*)memJ, (Value*)memI)->setMemMustType(true, must, dataDepType);

  return ;
}

void llvm::PDGAnalysis::addEdgeFromFunctionModRef (PDG *pdg, Function &F, AAResults &AA, StoreInst *memI, CallInst *call){
  auto makeRefEdge = false, makeModEdge = false;

  /*
   * Query the LLVM alias analyses.
   */
  switch (AA.getModRefInfo(call, MemoryLocation::get(memI))) {
    case ModRefInfo::Ref:
      makeRefEdge = true;
      break;
    case ModRefInfo::Mod:
      makeModEdge = true;
      break;
    case ModRefInfo::ModRef:
      makeRefEdge = makeModEdge = true;
      break;
  }
  if (  true
        && (!makeRefEdge)
        && (!makeModEdge)
    ){
    return ;
  }

  /*
   * Check other alias analyses
   */
  //TODO

  /*
   * There is a dependence.
   */
  if (makeRefEdge) {
    pdg->addEdge((Value*)memI, (Value*)call)->setMemMustType(true, false, DG_DATA_RAW);
    pdg->addEdge((Value*)call, (Value*)memI)->setMemMustType(true, false, DG_DATA_WAR);
  }
  if (makeModEdge) {
    pdg->addEdge((Value*)memI, (Value*)call)->setMemMustType(true, false, DG_DATA_WAW);
    pdg->addEdge((Value*)call, (Value*)memI)->setMemMustType(true, false, DG_DATA_WAW);
  }

  return ;
}

void llvm::PDGAnalysis::addEdgeFromFunctionModRef (PDG *pdg, Function &F, AAResults &AA, LoadInst *memI, CallInst *call){
  auto makeModEdge = false;

  /*
   * Query the LLVM alias analyses.
   */
  switch (AA.getModRefInfo(call, MemoryLocation::get(memI))) {
    case ModRefInfo::Ref:
      break;
    case ModRefInfo::Mod:
    case ModRefInfo::ModRef:
      makeModEdge = true;
      break;
  }
  if (!makeModEdge){
    return ;
  }

  /*
   * Check other alias analyses
   */
  //TODO

  /*
   * There is a dependence.
   */
  pdg->addEdge((Value*)call, (Value*)memI)->setMemMustType(true, false, DG_DATA_RAW);
  pdg->addEdge((Value*)memI, (Value*)call)->setMemMustType(true, false, DG_DATA_WAR);

  return ;
}

void llvm::PDGAnalysis::addEdgeFromFunctionModRef (PDG *pdg, Function &F, AAResults &AA, CallInst *otherCall, CallInst *call){
  auto makeRefEdge = false, makeModEdge = false, makeModRefEdge = false;
  auto reverseRefEdge = false, reverseModEdge = false, reverseModRefEdge = false;

  /*
   * Query the LLVM alias analyses.
   */
  switch (AA.getModRefInfo(call, otherCall)) {
    case ModRefInfo::Ref:
      makeRefEdge = true;
      break;
    case ModRefInfo::Mod:
      makeModEdge = true;
      switch (AA.getModRefInfo(otherCall, call)) {
        case ModRefInfo::Ref:
          reverseRefEdge = true;
          break;
        case ModRefInfo::Mod:
          reverseModEdge = true;
          break;
        case ModRefInfo::ModRef:
          reverseModRefEdge = true;
          break;
        default:
          abort();
      }
      break;
    case ModRefInfo::ModRef:
      makeModRefEdge = true;
      break;
  }
  if (  true
        && (!makeRefEdge)
        && (!makeModEdge)
        && (!makeModRefEdge)
    ){
    return ;
  }

  /*
   * Check other alias analyses
   */
  //TODO

  /*
   * There is a dependence.
   */
  if (makeRefEdge) {
    pdg->addEdge((Value*)call, (Value*)otherCall)->setMemMustType(true, false, DG_DATA_WAR);
    pdg->addEdge((Value*)otherCall, (Value*)call)->setMemMustType(true, false, DG_DATA_RAW);
  }
  if (makeModEdge) {
    /*
     * Dependency of Mod result between call and otherCall is depend on the reverse getModRefInfo result
     */
    if (reverseRefEdge) {
      pdg->addEdge((Value*)call, (Value*)otherCall)->setMemMustType(true, false, DG_DATA_RAW);
      pdg->addEdge((Value*)otherCall, (Value*)call)->setMemMustType(true, false, DG_DATA_WAR);
    }
    if (reverseModEdge) {
      pdg->addEdge((Value*)call, (Value*)otherCall)->setMemMustType(true, false, DG_DATA_WAW);
      pdg->addEdge((Value*)otherCall, (Value*)call)->setMemMustType(true, false, DG_DATA_WAW);
    }
    if (reverseModRefEdge) {
      pdg->addEdge((Value*)call, (Value*)otherCall)->setMemMustType(true, false, DG_DATA_RAW);
      pdg->addEdge((Value*)call, (Value*)otherCall)->setMemMustType(true, false, DG_DATA_WAW);
      pdg->addEdge((Value*)otherCall, (Value*)call)->setMemMustType(true, false, DG_DATA_WAR);
      pdg->addEdge((Value*)otherCall, (Value*)call)->setMemMustType(true, false, DG_DATA_WAW);
    }
  }
  if (makeModRefEdge) {
    pdg->addEdge((Value*)call, (Value*)otherCall)->setMemMustType(true, false, DG_DATA_WAR);
    pdg->addEdge((Value*)call, (Value*)otherCall)->setMemMustType(true, false, DG_DATA_WAW);
    pdg->addEdge((Value*)otherCall, (Value*)call)->setMemMustType(true, false, DG_DATA_RAW);
    pdg->addEdge((Value*)otherCall, (Value*)call)->setMemMustType(true, false, DG_DATA_WAW);    
  }

  return ;
}

void llvm::PDGAnalysis::iterateInstForStoreAliases (PDG *pdg, Function &F, AAResults &AA, StoreInst *store) {
  for (auto &B : F) {
    for (auto &I : B) {

      /*
       * Check stores.
       */
      if (auto otherStore = dyn_cast<StoreInst>(&I)) {
        if (store != otherStore){
          addEdgeFromMemoryAlias<StoreInst, StoreInst>(pdg, F, AA, store, otherStore, true);
        }

      /* 
       * Check loads.
       */
      } else if (auto load = dyn_cast<LoadInst>(&I)) {
        addEdgeFromMemoryAlias<StoreInst, LoadInst>(pdg, F, AA, store, load, false);
      }
    }
  }
}

void llvm::PDGAnalysis::iterateInstForModRef(PDG *pdg, Function &F, AAResults &AA, CallInst &call) {
  for (auto &B : F) {
    for (auto &I : B) {
      if (auto *load = dyn_cast<LoadInst>(&I)) {
        addEdgeFromFunctionModRef(pdg, F, AA, load, &call);
      } else if (auto *store = dyn_cast<StoreInst>(&I)) {
        addEdgeFromFunctionModRef(pdg, F, AA, store, &call);
      } else if (auto *otherCall = dyn_cast<CallInst>(&I)) {
        addEdgeFromFunctionModRef(pdg, F, AA, otherCall, &call);
      }
    }
  }
}

void llvm::PDGAnalysis::constructEdgesFromAliases (PDG *pdg, Module &M){

  /*
   * Use alias analysis on stores, loads, and function calls to construct PDG edges
   */
  for (auto &F : M) {
    if (F.empty()) continue ;
    auto &AA = getAnalysis<AAResultsWrapperPass>(F).getAAResults();
    constructEdgesFromAliasesForFunction(pdg, F, AA);
  }
}

void llvm::PDGAnalysis::constructEdgesFromAliasesForFunction (PDG *pdg, Function &F, AAResults &AA){
  for (auto &B : F) {
    for (auto &I : B) {
      if (auto store = dyn_cast<StoreInst>(&I)) {
        iterateInstForStoreAliases(pdg, F, AA, store);
      } else if (auto call = dyn_cast<CallInst>(&I)) {
        iterateInstForModRef(pdg, F, AA, *call);
      }
    }
  }
}

void llvm::PDGAnalysis::constructEdgesFromControl (PDG *pdg, Module &M){
  for (auto &F : M) {
    if (F.empty()) continue ;
    auto &postDomTree = getAnalysis<PostDominatorTreeWrapperPass>(F).getPostDomTree();
    this->constructEdgesFromControlForFunction(pdg, F, postDomTree);
  }
}

void llvm::PDGAnalysis::constructEdgesFromControlForFunction (PDG *pdg, Function &F, PostDominatorTree &postDomTree) {
  for (auto &B : F)
  {
    SmallVector<BasicBlock *, 10> dominatedBBs;
    postDomTree.getDescendants(&B, dominatedBBs);

    /*
     * For each basic block that B post dominates, check if B doesn't stricly post dominate its predecessor
     * If it does not, then there is a control dependency from the predecessor to B 
     */
    for (auto dominatedBB : dominatedBBs)
    {
      for (auto predBB : make_range(pred_begin(dominatedBB), pred_end(dominatedBB)))
      {
        if (postDomTree.properlyDominates(&B, predBB)) continue;
        auto controlTerminator = predBB->getTerminator();
        for (auto &I : B)
        {
          auto edge = pdg->addEdge((Value*)controlTerminator, (Value*)&I);
          edge->setControl(true);
        }
      }
    }
  }
}

void llvm::PDGAnalysis::removeEdgesNotUsedByParSchemes (PDG *pdg) {
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
      (store && load && isBackedgeOfLoadStoreIntoSameOffsetOfArray(edge, load, store)) ||
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

bool llvm::PDGAnalysis::isBackedgeOfLoadStoreIntoSameOffsetOfArray (
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

bool llvm::PDGAnalysis::isBackedgeIntoSameGlobal (
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

bool llvm::PDGAnalysis::isMemoryAccessIntoDifferentArrays (DGEdge<Value> *edge) {
  Value *array1 = allocAA->getPrimitiveArrayAccess(edge->getOutgoingT()).first;
  Value *array2 = allocAA->getPrimitiveArrayAccess(edge->getIncomingT()).first;
  return (array1 && array2 && array1 != array2);
}

bool llvm::PDGAnalysis::canPrecedeInCurrentIteration (Instruction *from, Instruction *to) {
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
