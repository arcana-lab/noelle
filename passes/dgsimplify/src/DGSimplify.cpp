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
   * Collect functions through call graph starting at function "main"
   */

  /*
  std::set<Function *> funcToCheck;
  collectAllFunctionsInCallGraph(M, funcToCheck);
  bool inlined = inlineCallsInFunctionsWithMassiveSCCs(funcToCheck);
  return inlined;
  */

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

void llvm::DGSimplify::collectAllFunctionsInCallGraph (Module &M, std::set<Function *> &funcSet) {
  auto &callGraph = getAnalysis<CallGraphWrapperPass>().getCallGraph();
  std::queue<Function *> funcToTraverse;
  funcToTraverse.push(M.getFunction("main"));
  while (!funcToTraverse.empty())
  {
    auto func = funcToTraverse.front();
    funcToTraverse.pop();
    if (funcSet.find(func) != funcSet.end()) continue;
    funcSet.insert(func);

    auto funcCGNode = callGraph[func];
    for (auto &callRecord : make_range(funcCGNode->begin(), funcCGNode->end()))
    {
      auto F = callRecord.second->getFunction();
      if (!F || F->empty()) continue;
      funcToTraverse.push(F);
    }
  }
}

bool llvm::DGSimplify::inlineCallsInFunctionsWithMassiveSCCs (std::set<Function *> &funcSet) {
  auto &PDGA = getAnalysis<PDGAnalysis>();
  bool inlined = false;
  for (auto F : funcSet) {
    auto fdg = PDGA.getFunctionPDG(*F);
    bool inlinedCall = checkToInlineCallInFunction(fdg, *F);
    delete fdg;

    inlined |= inlinedCall;
  }
  
  if (inlined) {
    ofstream inlineInfo("dgsimplify_continue.txt");
    inlineInfo << "1";
    inlineInfo.close();
  }

  return inlined;
}

/*
 * GOAL: Go through loops in function
 * If there is only one non-clonable/reducable SCC,
 * try inlining the function call in that SCC with the
 * most memory edges to other internal/external values
 */
bool llvm::DGSimplify::checkToInlineCallInFunction (PDG *fdg, Function &F) {
  auto& PDT = getAnalysis<PostDominatorTreeWrapperPass>(F).getPostDomTree();
  auto& LI = getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
  auto& SE = getAnalysis<ScalarEvolutionWrapperPass>(F).getSE();
  for (auto loop : LI.getLoopsInPreorder()) {
    auto LDI = new LoopDependenceInfo(&F, fdg, loop, LI, PDT);
    auto &attrs = LDI->sccdagAttrs;
    attrs.populate(LDI->loopSCCDAG, LDI->liSummary, SE);

    std::set<SCC *> sccsToCheck;
    for (auto sccNode : LDI->loopSCCDAG->getNodes()) {
      auto scc = sccNode->getT();

      if (attrs.executesCommutatively(scc)
          || attrs.executesIndependently(scc)
          || attrs.canBeCloned(scc)) {
        continue;
      }

      sccsToCheck.insert(scc);
    }

    /*
     * NOTE: if there are more than two non-trivial SCCs, then
     * there is less incentive to continue trying to inline.
     * Why 2? Because 2 is always a simple non-trivial number
     * to start a heuristic at.
     */
    if (sccsToCheck.size() > 2) continue;

    int64_t maxMemEdges = 0;
    CallInst *inlineCall = nullptr;

    for (auto scc : sccsToCheck) {
      for (auto valNode : scc->getNodes()) {
        auto val = valNode->getT();
        if (auto call = dyn_cast<CallInst>(val)) {
          auto callF = call->getCalledFunction();
          if (!callF || callF->empty()) continue;
          
          /*
           * NOTE(angelo): Do not consider inlining a recursive function call
           */
          if (callF == &F) continue;

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

    delete LDI;

    if (inlineCall && inlineFunctionCall(&F, inlineCall)) {
      return true;
    }
  }

  return false;
}

void llvm::DGSimplify::collectInDepthOrderFnsCallsAndLoops (Function *main) {
  auto &callGraph = getAnalysis<CallGraphWrapperPass>().getCallGraph();
  std::queue<Function *> funcToTraverse;

  /*
   * NOTE(angelo): Traverse call graph, collecting function "parents":
   *  Parent functions are those encountered before their children in a
   *  breadth-first traversal of the call graph
   */
  funcToTraverse.push(main);
  std::set<Function *> reached;
  std::unordered_map<Function *, std::set<Function *>> parents;
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
      parents[F].insert(func);
      funcToTraverse.push(F);
    }
  }
  reached.clear()

  /*
   * NOTE(angelo): Determine the depth of functions in the call graph:
   *  next-depth functions are those where every parent function
   *  has already been assigned a previous depth
   */
  funcToTraverse.push(main);
  depthOrderedFns.push_back(main);
  reached.insert(main);
  while (!funcToTraverse.empty()) {
    auto func = funcToTraverse.front();
    funcToTraverse.pop();

    auto funcCGNode = callGraph[func];
    for (auto &callRecord : make_range(funcCGNode->begin(), funcCGNode->end())) {
      auto F = callRecord.second->getFunction();
      if (!F || F->empty()) continue;
      if (reached.find(F) != reached.end()) continue;
      
      bool allParentsOrdered = true;
      for (auto parent : parents[F]) {
        if (reached.find(parent) == reached.end()) {
          allParentsOrdered = false;
          break;
        }
      }
      if (allParentsOrdered) {
        depthOrderedFns.push_back(F);
        funcToTraverse.push(F);
        reached.insert(F);
      }
    }
  }

  // NOTE(angelo): Order function calls by traversing depth ordered functions
  // TODO(angelo): From here
  int callInd = 0;
  while (!funcToTraverse.empty()) {
    auto func = funcToTraverse.front();
    funcToTraverse.pop();

    /*
     * NOTE(angelo): Existence of ordering for a function's loops is the
     *  most performant check for whether it has been encountered already 
     */
    if (preOrderedLoops.find(func) != preOrderedLoops.end()) continue;
    orderedFns.push_back(func);
    collectPreOrderedLoopsFor(func);

    auto funcCGNode = callGraph[func];
    for (auto &callRecord : make_range(funcCGNode->begin(), funcCGNode->end())) {
      if (!callRecord.first.pointsToAliveValue()) continue;
      auto *V = &*callRecord.first;

      /*
       * NOTE(angelo): Current implementation only traverses CallInst invocations,
       *  not InvokeInst invocations of functions
       */
      if (auto call = dyn_cast<CallInst>(V)) {
        auto F = callRecord.second->getFunction();
        if (!F || F->empty()) continue;
        preOrderedCalls[call] = callInd++;
        funcToTraverse.push(F);
      }
    }
  }
}

void llvm::DGSimplify::collectPreOrderedLoopsFor (Function *F) {
  /*
   * NOTE(angelo): Enforce managing order instead of recalculating it entirely
   */
  if (preOrderedLoops.find(F) != preOrderedLoops.end()) {
    errs() << "DGSimplify:   Misuse! Do not collect ordered loops more than once. Manage current ordering.\n";
  }
  preOrderedLoops[F] = new std::vector<Loop *>();

  auto& LI = getAnalysis<LoopInfoWrapperPass>(*F).getLoopInfo();
  for (auto loop : LI.getLoopsInPreorder()) {
    preOrderedLoops[F]->push_back(loop);
  }
}

bool llvm::DGSimplify::inlineFunctionCall (Function *F, CallInst *call) {
  /*
   * NOTE(angelo): Prevent inlining a call within a function already altered by inlining
   */
  if (fnsAffected.find(F) != fnsAffected.end()) return false ;

  InlineFunctionInfo IFI;
  call->print(errs() << "DGSimplify:   Inlining: "); errs() << "\n";
  if (InlineFunction(call, IFI)) {
    fnsAffected.insert(F); 
    return true;
  }
  return false;
}
