#include "DGSimplify.hpp"

/*
 * Options of the dependence graph simplifier pass.
 */
static cl::opt<bool> ForceInlineToLoop("dgsimplify-inline-to-loop", cl::ZeroOrMore, cl::Hidden, cl::desc("Force inlining along the call graph from main to the loops being parallelized"));

DGSimplify::~DGSimplify () {
  for (auto orderedLoops : preOrderedLoops) {
    delete orderedLoops.second;
  }
}

bool llvm::DGSimplify::doInitialization (Module &M) {
  errs() << "DGSimplify at \"doInitialization\"\n" ;

  return false;
}

bool llvm::DGSimplify::runOnModule (Module &M) {
  errs() << "DGSimplify at \"runOnModule\"\n";

  /*
   * Collect function and loop ordering to track inlining progress
   */
  auto main = M.getFunction("main");
  collectFnGraph(main);
  collectInDepthOrderFns(main);

  // OPTIMIZATION(angelo): Do this lazily, depending on what functions are considered in algorithms
  for (auto func : depthOrderedFns) {
    collectPreOrderedLoopsFor(func);
  }

  printFnCallGraph();
  printFnOrder();

  auto writeToContinueFile = []() -> void {
    ofstream continuefile("dgsimplify_continue.txt");
    continuefile << "1\n";
    continuefile.close();
  };

  /*
   * Inline calls within large SCCs of targeted loops
   */
  std::string filename = "scc_call_inlining";
  getLoopsToInline(filename);
  bool inlined = inlineCallsInMassiveSCCsOfLoops();
  bool remaining = registerRemainingLoops(filename);
  if (remaining) writeToContinueFile();
  if (inlined) return true;

  /*
   * Inline functions containing targeted loops so the loop is in main
   */
  loopsToCheck.clear();
  filename = "loop_hoisting";
  getLoopsToInline(filename);
  inlined = inlineFnsOfLoopsToCGRoot();
  remaining = registerRemainingLoops(filename);
  if (remaining) writeToContinueFile();
  if (inlined) return true;

  return false;
}

void llvm::DGSimplify::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<CallGraphWrapperPass>();
  AU.addRequired<PDGAnalysis>();
  AU.addRequired<PostDominatorTreeWrapperPass>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
  AU.setPreservesAll();
  return ;
}

void llvm::DGSimplify::getLoopsToInline (std::string filename) {
  loopsToCheck.clear();
  ifstream infile("dgsimplify_" + filename);
  if (infile.good()) {
    std::string line;
    std::string delimiter = ",";
    while(getline(infile, line)) {
      size_t i = line.find(delimiter);
      int fnInd = std::stoi(line.substr(0, i));
      int loopInd = std::stoi(line.substr(i + delimiter.length()));
      errs() << "DGSimplify:   Checking: FN index: " << fnInd
        << ", LOOP index: " << loopInd << "\n";
      auto F = depthOrderedFns[fnInd];

      Loop *loop = nullptr;
      for (auto loopIndPair : *preOrderedLoops[F]) {
        if (loopIndPair.second == loopInd) {
          loop = loopIndPair.first;
        }
      }
      assert(loop != nullptr);
      loopsToCheck[F].insert(loop);
    }
    return;
  }

  // NOTE(angelo): Default to selecting all loops in the program
  for (auto funcLoops : preOrderedLoops) {
    auto F = funcLoops.first;
    for (auto loopIndPair : *funcLoops.second) {
      auto loop = loopIndPair.first;
      errs() << "DGSimplify:   Checking: FN index: " << fnOrders[F]
        << ", LOOP index: " << (*preOrderedLoops[F])[loop] << "\n";
      loopsToCheck[F].insert(loop);
    }
  }
}

bool llvm::DGSimplify::registerRemainingLoops (std::string filename) {
  std::string fullname = "dgsimplify_" + filename;
  remove(fullname.c_str());
  if (loopsToCheck.size() == 0) return false;

  ofstream outfile(fullname);
  for (auto funcLoops : loopsToCheck) {
    auto F = funcLoops.first;
    int fnInd = fnOrders[F];
    for (auto loop : funcLoops.second) {
      int loopInd = (*preOrderedLoops[F])[loop];
      errs() << "DGSimplify:   Remaining: FN index: " << fnInd
        << ", LOOP index: " << loopInd << "\n";
      outfile << fnInd << "," << loopInd << "\n";
    }
  }
  outfile.close();
  return true;
}

bool llvm::DGSimplify::inlineCallsInMassiveSCCsOfLoops () {
  auto &PDGA = getAnalysis<PDGAnalysis>();
  bool anyInlined = false;

  // NOTE(angelo): Order these functions to prevent duplicating loops yet to be checked
  std::vector<Function *> fnsToCheck;
  for (auto fnLoops : loopsToCheck) fnsToCheck.push_back(fnLoops.first);
  std::sort(fnsToCheck.begin(), fnsToCheck.end(), [this](Function *a, Function *b) {
    // NOTE(angelo): Sort functions deepest first
    return fnOrders[a] > fnOrders[b];
  });

  std::set<Function *> fnsToAvoid;
  for (auto F : fnsToCheck) {
    if (fnsToAvoid.find(F) != fnsToAvoid.end()) continue;
    auto& PDT = getAnalysis<PostDominatorTreeWrapperPass>(*F).getPostDomTree();
    auto& LI = getAnalysis<LoopInfoWrapperPass>(*F).getLoopInfo();
    auto& SE = getAnalysis<ScalarEvolutionWrapperPass>(*F).getSE();
    auto fdg = PDGA.getFunctionPDG(*F);

    bool inlined = false;
    std::set<Loop *> removeLoops;
    for (auto loop : loopsToCheck[F]) {
      errs() << "Loop issue? " << loop << "\n";
      loop->getHeader()->print(errs() << "Header: "); errs() << "\n";
      auto LDI = new LoopDependenceInfo(F, fdg, loop, LI, PDT);
      auto &attrs = LDI->sccdagAttrs;
      attrs.populate(LDI->loopSCCDAG, LDI->liSummary, SE);
      bool inlinedCall = inlineCallsInMassiveSCCs(F, LDI);
      if (!inlinedCall) removeLoops.insert(loop);

      inlined |= inlinedCall;
      delete LDI;
      if (inlined) break;
    }

    // NOTE(angelo): Avoid affected function parents as we will revisit them next simplify pass
    if (inlined) {
      for (auto parentF : parentFns[F]) fnsToAvoid.insert(parentF);
    }
    for (auto removeLoop : removeLoops) loopsToCheck[F].erase(removeLoop);
    anyInlined |= inlined;
    delete fdg;
  }

  return anyInlined;
}

/*
 * GOAL: Go through loops in function
 * If there is only one non-clonable/reducable SCC,
 * try inlining the function call in that SCC with the
 * most memory edges to other internal/external values
 */
bool llvm::DGSimplify::inlineCallsInMassiveSCCs (Function *F, LoopDependenceInfo *LDI) {
  std::set<SCC *> sccsToCheck;
  for (auto sccNode : LDI->loopSCCDAG->getNodes()) {
    auto scc = sccNode->getT();
    if (!LDI->sccdagAttrs.executesCommutatively(scc)
        && !LDI->sccdagAttrs.executesIndependently(scc)
        && !LDI->sccdagAttrs.canBeCloned(scc)) {
      sccsToCheck.insert(scc);
    }
  }

  /*
   * NOTE: if there are more than two non-trivial SCCs, then
   * there is less incentive to continue trying to inline.
   * Why 2? Because 2 is always a simple non-trivial number
   * to start a heuristic at.
   */
  if (sccsToCheck.size() > 2) return false;

  int64_t maxMemEdges = 0;
  CallInst *inlineCall = nullptr;
  for (auto scc : sccsToCheck) {
    for (auto valNode : scc->getNodes()) {
      auto val = valNode->getT();
      if (auto call = dyn_cast<CallInst>(val)) {
        auto callF = call->getCalledFunction();
        if (!callF || callF->empty()) continue;

        // NOTE(angelo): Do not consider inlining a recursive function call
        if (callF == F) continue;

        // NOTE(angelo): Do not consider inlining calls to functions of lower depth
        if (fnOrders[callF] < fnOrders[F]) continue;

        auto memEdgeCount = 0;
        for (auto edge : valNode->getAllConnectedEdges()) {
          if (edge->isMemoryDependence()) memEdgeCount++;
        }
        if (memEdgeCount > maxMemEdges) {
          maxMemEdges = memEdgeCount;
          inlineCall = call;
        }
      }
    }
  }

  return inlineCall && inlineFunctionCall(F, inlineCall);
}

bool llvm::DGSimplify::inlineFnsOfLoopsToCGRoot () {
  std::vector<Function *> fnsToCheck;
  for (auto fnLoops : loopsToCheck) fnsToCheck.push_back(fnLoops.first);
  std::sort(fnsToCheck.begin(), fnsToCheck.end(), [this](Function *a, Function *b) {
    // NOTE(angelo): Sort functions deepest first
    return fnOrders[a] > fnOrders[b];
  });

  int fnIndex = 0;
  std::set<Function *> fnsWillCheck(fnsToCheck.begin(), fnsToCheck.end());
  while (fnIndex < fnsToCheck.size()) {
    auto childF = fnsToCheck[fnIndex];
    bool inlinedFully = true;
    for (auto parentF : parentFns[childF]) {
      // NOTE(angelo): Do not inline from less deep to more deep (to avoid recursive chains)
      if (fnOrders[parentF] > fnOrders[childF]) continue;

      // NOTE(angelo): Since only one inline per function is permitted, this for loop
      //  isn't entirely necessary, but it better expresses conceptual intent
      for (auto call : childrenFns[parentF][childF]) {
        inlinedFully &= inlineFunctionCall(parentF, call);
      }

      // NOTE(angelo): Insert parent to inline up the CG if it isn't already inserted
      if (fnsWillCheck.find(parentF) != fnsWillCheck.end()) continue;
      int insertIndex = -1;
      while (fnOrders[fnsToCheck[++insertIndex]] > fnOrders[parentF]);
      fnsToCheck.insert(fnsToCheck.begin() + insertIndex, parentF);
      fnsWillCheck.insert(parentF);
    }
    if (!inlinedFully) break;
    loopsToCheck.erase(fnsToCheck[fnIndex]);
    ++fnIndex;
  }
}

bool llvm::DGSimplify::inlineFunctionCall (Function *F, CallInst *call) {
  // NOTE(angelo): Prevent inlining a call within a function already altered by inlining
  if (fnsAffected.find(F) != fnsAffected.end()) return false ;

  InlineFunctionInfo IFI;
  call->print(errs() << "DGSimplify:   Inlining: "); errs() << "\n";
  if (InlineFunction(call, IFI)) {
    fnsAffected.insert(F); 
    adjustOrdersAfterInline(F, call);
    return true;
  }
  return false;
}


void llvm::DGSimplify::adjustOrdersAfterInline (Function *parentF, CallInst *call) {
  auto childF = call->getCalledFunction();
  removeFnPairInstance(parentF, childF, call);
  for (auto newChild : childrenFns[childF]) {
    for (auto call : newChild.second) {
      addFnPairInstance(parentF, newChild.first, call);
    }
  }

  /*
   * NOTE(angelo): Starting after the loop in the parent function, index all loops in the
   * child function as being now in the parent function and adjust the indices of loops
   * after the call site by the number of loops inserted
   */
  auto &parentLoops = *preOrderedLoops[parentF];
  auto &childLoops = *preOrderedLoops[childF];
  auto nextLoopInParent = getNextPreorderLoopAfter(parentF, call);
  auto startInd = nextLoopInParent
    ? parentLoops[nextLoopInParent] : parentLoops.size();
  auto childLoopCount = childLoops.size();

  // NOTE(angelo): Adjust indices of loops after the call site (in forward program order)
  std::set<Loop *> loopsToAdjust;
  for (auto parentLoopIndPair : parentLoops) {
    if (parentLoopIndPair.second < startInd) continue;
    loopsToAdjust.insert(parentLoopIndPair.first);
  }
  for (auto loop : loopsToAdjust) parentLoops[loop] += childLoopCount;

  // NOTE(angelo): Insert indices of inlined loops
  for (auto childLoopIndPair : childLoops) {
    parentLoops[childLoopIndPair.first] = startInd + childLoopIndPair.second;
  }

  // DEBUG(angelo): loop order after inlining
  printFnLoopOrder(parentF);
}

Loop *llvm::DGSimplify::getNextPreorderLoopAfter (Function *F, CallInst *call) {
  auto& LI = getAnalysis<LoopInfoWrapperPass>(*F).getLoopInfo();
  auto callBB = call->getParent();
  auto callLoop = LI.getLoopFor(callBB);
  auto callDepth = LI.getLoopDepth(callBB);
  bool startSearch = false;
  Loop *nextLoop = nullptr;

  // NOTE(angelo): Search in forward program order for next loop header
  for (auto &B : *F) {
    if (callBB == &B) {
      startSearch = true;
      continue;
    }
    if (!startSearch) continue;
    auto depth = LI.getLoopDepth(&B);
    if (depth == 0) continue;

    /*
     * NOTE(angelo): Next loop header must either be:
     * 1) a direct child of the inner-most loop the call resides in
     * 2) a loop with a smaller depth than the call's inner-most loop
     */
    if (depth - 1 <= callDepth) {
      if (LI.isLoopHeader(&B)) {
        nextLoop = LI.getLoopFor(&B);
        break;
      }
    }
  }
  return nextLoop;
}

void llvm::DGSimplify::collectFnGraph (Function *main) {
  auto &callGraph = getAnalysis<CallGraphWrapperPass>().getCallGraph();
  std::queue<Function *> funcToTraverse;
  std::set<Function *> reached;

  /*
   * NOTE(angelo): Traverse call graph, collecting function "parents":
   *  Parent functions are those encountered before their children in a
   *  breadth-first traversal of the call graph
   */
  funcToTraverse.push(main);
  reached.insert(main);
  while (!funcToTraverse.empty()) {
    auto func = funcToTraverse.front();
    funcToTraverse.pop();

    auto funcCGNode = callGraph[func];
    for (auto &callRecord : make_range(funcCGNode->begin(), funcCGNode->end())) {
      auto weakVH = callRecord.first;
      if (!weakVH.pointsToAliveValue() || !isa<CallInst>(*&weakVH)) continue;
      auto F = callRecord.second->getFunction();
      if (!F || F->empty()) continue;

      addFnPairInstance(func, F, (CallInst *)(&*weakVH));
      if (reached.find(F) != reached.end()) continue;
      reached.insert(F);
      funcToTraverse.push(F);
    }
  }
}

/*
 * NOTE(angelo): Determine the depth of functions in the call graph:
 *  next-depth functions are those where every parent function
 *  has already been assigned a previous depth
 * Obviously, recursive loops by this definition have undefined depth.
 *  These groups, each with a chain of recursive functions, are ordered
 *  by their entry points' relative depths. They are assigned depths
 *  after all other directed acyclic portions of the call graph (starting
 *  from their common ancestor) is traversed.
 */
void llvm::DGSimplify::collectInDepthOrderFns (Function *main) {
  auto &callGraph = getAnalysis<CallGraphWrapperPass>().getCallGraph();
  std::queue<Function *> funcToTraverse;
  std::set<Function *> reached;
  std::vector<Function *> *deferred = new std::vector<Function *>();

  funcToTraverse.push(main);
  fnOrders[main] = 0;
  depthOrderedFns.push_back(main);
  reached.insert(main);
  // NOTE(angelo): Check to see whether any functions remain to be traversed
  while (!funcToTraverse.empty()) {
    // NOTE(angelo): Check to see whether any order-able functions remain
    while (!funcToTraverse.empty()) {
      auto func = funcToTraverse.front();
      funcToTraverse.pop();
      errs() << "Traversing: " << func->getName() << "\n";

      auto funcCGNode = callGraph[func];
      for (auto &callRecord : make_range(funcCGNode->begin(), funcCGNode->end())) {
        auto F = callRecord.second->getFunction();
        if (!F || F->empty()) continue;
        if (reached.find(F) != reached.end()) continue;
        
        bool allParentsOrdered = true;
        for (auto parent : parentFns[F]) {
          if (reached.find(parent) == reached.end()) {
            allParentsOrdered = false;
            break;
          }
        }
        if (allParentsOrdered) {
          funcToTraverse.push(F);
          fnOrders[F] = depthOrderedFns.size();
          depthOrderedFns.push_back(F);
          reached.insert(F);
        } else {
          deferred->push_back(F);
        }
      }
    }

    /*
     * NOTE(angelo): Collect all deferred functions that never got ordered.
     * By definition of the ordering, they must all be parts of recursive chains.
     * Order their entry points, add them to the queue to traverse.
     */
    auto remaining = new std::vector<Function *>();
    for (auto left : *deferred) {
      if (fnOrders.find(left) == fnOrders.end()) {
        remaining->push_back(left);
        funcToTraverse.push(left);
        fnOrders[left] = depthOrderedFns.size();
        depthOrderedFns.push_back(left);
        reached.insert(left);
      }
    }
    delete deferred;
    deferred = remaining;
  }

  delete deferred;
}

void llvm::DGSimplify::collectPreOrderedLoopsFor (Function *F) {
  // NOTE(angelo): Enforce managing order instead of recalculating it entirely
  if (preOrderedLoops.find(F) != preOrderedLoops.end()) {
    errs() << "DGSimplify:   Misuse! Do not collect ordered loops more than once. Manage current ordering.\n";
  }
  auto& LI = getAnalysis<LoopInfoWrapperPass>(*F).getLoopInfo();
  if (LI.empty()) return;

  int count = 0;
  auto *orderedLoops = new std::unordered_map<Loop *, int>();
  for (auto loop : LI.getLoopsInPreorder()) {
    (*orderedLoops)[loop] = count++;
  }
  preOrderedLoops[F] = orderedLoops;
}

void llvm::DGSimplify::addFnPairInstance (Function *parentF, Function *childF, CallInst *call) {
  auto &children = childrenFns[parentF];
  parentFns[childF].insert(parentF);
  children[childF].insert(call);
}

void llvm::DGSimplify::removeFnPairInstance (Function *parentF, Function *childF, CallInst *call) {
  auto &children = childrenFns[parentF];
  children[childF].erase(call);
  if (children[childF].size() == 0) {
    children.erase(childF);
  }
}

void llvm::DGSimplify::printFnCallGraph () {
  for (auto fns : parentFns) {
    errs() << "DGSimplify:   Parent function: " << fns.first->getName() << "\n";
    for (auto f : fns.second) {
      errs() << "DGSimplify:   \tChild: " << f->getName() << "\n";
    }
  }
}

void llvm::DGSimplify::printFnOrder () {
  int count = 0;
  for (auto fn : depthOrderedFns) {
    ++count;
    errs() << "DGSimplify:   Function: " << count << " " << fn->getName() << "\n";
  }
}

void llvm::DGSimplify::printFnLoopOrder (Function *F) {
  for (auto loopIndPair : *preOrderedLoops[F]) {
    auto headerBB = loopIndPair.first->getHeader();
    errs() << "DGSimplify:   Loop " << loopIndPair.second << "\n";
    headerBB->print(errs()); errs() << "\n";
  }
}
