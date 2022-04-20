/*
 * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "Inliner.hpp"

namespace llvm::noelle {

Inliner::Inliner ()
  : ModulePass{ID}
  , maxNumberOfFunctionCallsToInlinePerLoop{10}
  , maxProgramInstructions{50000}
  , fnsAffected{}
  , parentFns{}
  , childrenFns{}
  , loopsToCheck{}
  , depthOrderedFns{}
  , preOrderedLoops{}
{

  return ;
}

bool Inliner::runOnModule (Module &M) {

  /*
   * Fetch NOELLE.
   */
  auto& noelle = getAnalysis<Noelle>();

  /*
   * Check if the inliner has been enabled.
   */
  if (!noelle.isTransformationEnabled(INLINER_ID)){

    /*
     * The function inliner has been disabled.
     */
    return false;
  }
  errs() << "Inliner: Start\n";

  /*
   * Fetch the entry point of the program.
   */
  auto fm = noelle.getFunctionsManager();
  auto main = fm->getEntryFunction();
  if (main == nullptr){
    errs() << "Inliner:   No entry function\n";
    errs() << "Inliner: Exit\n";
    return false;
  }

  /*
   * Check if the program is already too big
   */
  auto programInstructions = noelle.numberOfProgramInstructions();
  errs() << "Inliner:   Number of program instructions = " << programInstructions << "\n";
  if (programInstructions >= this->maxProgramInstructions){
    errs() << "Inliner:     There are too many instructions. We'll not inline anything\n";
    return false;
  }

  /*
   * Fetch the call graph.
   */
  auto pcg = fm->getProgramCallGraph();

  /*
   * Collect function and loop ordering to track inlining progress
   */
  collectFnGraph(main);
  collectInDepthOrderFns(main);
  for (auto func : depthOrderedFns) {
    createPreOrderedLoopSummariesFor(func);
  }

  auto printFnInfo = [&]() -> void {
    if (this->verbose >= Verbosity::Maximal) {
      errs() << "Inliner:   Function graph and order\n";
      printFnCallGraph();
      printFnOrder();
    }
  };
  printFnInfo();

  /*
   * Fetch the profiles.
   */
  auto profiles = noelle.getProfiles();
  if (this->verbose != Verbosity::Disabled) {
    if (profiles->isAvailable()){
      errs() << "Inliner:   Profiles are available and will be used\n";
    } else{
      errs() << "Inliner:   Profiles are not available\n";
    }
  }

  /*
   * Inline calls involved in loop-carried data dependences.
   */
  getLoopsToInline(noelle, profiles);

  /*
   * Perform the inlining.
   */
  auto inlined = this->inlineCallsInvolvedInLoopCarriedDataDependences(noelle, pcg);
  if (inlined){
    errs() << "Inliner:   Inlined calls due to loop-carried data dependences\n";

    /*
     * Free the memory.
     */
    delete pcg;

    errs() << "Inliner: Exit\n";
    return true;
  }

  /*
   * No more calls need to be inlined for loop-carried dependences.
   */
  if (this->verbose != Verbosity::Disabled){
    errs() << "Inliner:   No remaining calls need to be inlined due to loop-carried data dependences\n";
  }
  printFnInfo();

  /*
   * Check if we should hoist loops to main.
   */
  if (!noelle.shouldLoopsBeHoistToMain()){
    errs() << "Inliner:   The code has not been modified\n";

    /*
     * Free the memory.
     */
    delete pcg;

    errs() << "Inliner: Exit\n";
    return false;
  }

  /*
   * Inline functions containing targeted loops so the loop is in main
   */
  std::string filename = "dgsimplify_loop_hoisting.txt";
  getFunctionsToInline(filename);

  inlined = this->inlineFnsOfLoopsToCGRoot(profiles);
  if (inlined) {
    errs() << "Inliner:   Inlined functions to hoist loops to the entry funtion of the program\n";
    getAnalysis<CallGraphWrapperPass>().runOnModule(M);
    parentFns.clear();
    childrenFns.clear();
    orderedCalled.clear();
    orderedCalls.clear();
    collectFnGraph(main);
    collectInDepthOrderFns(main);
    printFnOrder();
  }

  auto remaining = registerRemainingFunctions(filename);
  printFnInfo();
  if (!remaining && this->verbose != Verbosity::Disabled) {
    errs() << "Inliner:   No remaining hoists\n";
  }

  /*
   * Free the memory.
   */
  delete pcg;

  errs() << "Inliner: Exit\n";
  return inlined;

  /*
   * Free the memory.
   */
  delete pcg;

  errs() << "Inliner: Exit\n";
  return false;
}

/*
 * Progress Tracking using file system
 */
void Inliner::getLoopsToInline (Noelle &noelle, Hot *profiles) {
  assert(profiles != nullptr);

  for (auto funcLoops : preOrderedLoops) {
    auto F = funcLoops.first;
    for (auto summary : *funcLoops.second) {

      /*
       * Check if the profile is available.
       */
      if (profiles->isAvailable()){

        /* 
         * Check if the loop is hot enough.
         */
        auto hotness = profiles->getDynamicTotalInstructionCoverage(summary);
        if (hotness < noelle.getMinimumHotness()){

          /*
           * The loop isn't hot enough.
           */
          continue ;
        }
      }
      loopsToCheck[F].push_back(summary);
    }
  }
}

void Inliner::getFunctionsToInline (std::string filename) {
  fnsToCheck.clear();
  ifstream infile(filename);
  if (infile.good()) {
    std::string line;
    while(getline(infile, line)) {
      int fnInd = std::stoi(line);
      assert(fnInd > 0 && fnInd < depthOrderedFns.size());
      fnsToCheck.insert(depthOrderedFns[fnInd]);
    }
  } else {
    // NOTE(angelo): Default to select all functions with loops in them
    for (auto funcLoops : preOrderedLoops) {
      fnsToCheck.insert(funcLoops.first);
    }
  }
}

bool Inliner::registerRemainingFunctions (std::string filename) {
  remove(filename.c_str());
  if (fnsToCheck.empty()) {
    return false;
  }

  ofstream outfile(filename);
  std::vector<int> fnInds;
  for (auto F : fnsToCheck) {
    auto fID = fnOrders[F];
    if (fID == 0){
      continue ;
    }
    fnInds.push_back(fID);
  }

  std::sort(fnInds.begin(), fnInds.end());

  for (auto ind : fnInds) {
    outfile << ind << "\n";
  }
  outfile.close();

  if (fnInds.size() > 0){
    return true;
  }
  return false;
}

bool Inliner::inlineFnsOfLoopsToCGRoot (Hot *hot) {
  std::vector<Function *> orderedFns;
  for (auto F : fnsToCheck) {
    orderedFns.push_back(F);
  }
  sortInDepthOrderFns(orderedFns);

  int fnIndex = 0;
  std::set<Function *> fnsWillCheck(orderedFns.begin(), orderedFns.end());
  std::set<Function *> fnsToAvoid;
  auto inlined = false;
  while (fnIndex < orderedFns.size()) {
    auto childF = orderedFns[fnIndex++];

    // NOTE(angelo): If we avoid this function until next pass, we do the same with its parents
    if (fnsToAvoid.find(childF) != fnsToAvoid.end()) {
      for (auto parentF : parentFns[childF]) fnsToAvoid.insert(parentF);
      continue;
    }

    // NOTE(angelo): Cache parents as inlining may remove them
    std::set<Function *> parents;
    for (auto parent : parentFns[childF]) parents.insert(parent);

    // NOTE(angelo): Try to inline this child function in all of its parents
    bool inlinedInParents = true;
    for (auto parentF : parents) {
      if (fnsAffected.find(parentF) != fnsAffected.end()) continue;
      if (!canInlineWithoutRecursiveLoop(parentF, childF)) continue;

      // NOTE(angelo): Do not inline recursive function calls
      if (parentF == childF) continue;

      // NOTE(angelo): Do not inline into a parent deeper than the child (to avoid recursive chains)
      if (fnOrders[parentF] > fnOrders[childF]) continue;

      // NOTE(angelo): Cache calls as inlining affects the call list in childrenFns
      auto parentCalls = orderedCalls[parentF];
      std::set<CallInst *> cachedCalls(parentCalls.begin(), parentCalls.end());

      // NOTE(angelo): Since only one inline per function is permitted, this loop
      //  either inlines no calls (should the parent already be affected) or inlines
      //  the first call, indicating whether there are more calls to inline
      bool inlinedCalls = true;
      for (auto call : cachedCalls) {
        if (call->getCalledFunction() != childF) {
          continue;
        }

        /*
         * Try to inline
         */
        auto inlinedCall = inlineFunctionCall(hot, parentF, childF, call);
        if (inlinedCall && this->verbose != Verbosity::Disabled) {
          errs() << "Inliner:   Inlined " << childF->getName() << " into " << parentF->getName() << "\n";
        }

        inlined |= inlinedCall;
        inlinedCalls &= inlinedCall;
        if (inlined) break;
      }
      inlinedInParents &= inlinedCalls;

      // NOTE(angelo): Function isn't completely inlined in parent; avoid parent
      if (!inlinedCalls) {
        fnsToAvoid.insert(parentF);
        continue;
      }

      // NOTE(angelo): Insert parent to affect (in depth order, if not already present)
      if (fnsWillCheck.find(parentF) != fnsWillCheck.end()) continue;
      fnsWillCheck.insert(parentF);
      auto insertIndex = 0;
      assert(fnOrders.find(parentF) != fnOrders.end());
      auto parentFnOrder = fnOrders[parentF];
      for (auto insertIndex = 0; insertIndex < orderedFns.size() ; insertIndex++){
        auto currentFunction = orderedFns[insertIndex];
        auto currentFunctionFnOrder = fnOrders[currentFunction];
        if (currentFunctionFnOrder > parentFnOrder){
          break ;
        }
      }
      assert(insertIndex < orderedFns.size());
      orderedFns.insert(orderedFns.begin() + insertIndex, parentF);
    }

    if (inlinedInParents) fnsToCheck.erase(childF);
  }

  return inlined;
}

bool Inliner::canInlineWithoutRecursiveLoop (Function *parentF, Function *childF) {
  // NOTE(angelo): Prevent inlining a call to the entry of a recursive chain of functions
  if (recursiveChainEntranceFns.find(childF) != recursiveChainEntranceFns.end()) return false ;
  return true;
}

bool Inliner::inlineFunctionCall (
    Hot *p,
    Function *F, 
    Function *childF, 
    CallInst *call
    ){

  /*
   * Handle corner cases
   */
  if (  false
        || (F == nullptr)
        || (childF == nullptr)
        || (call == nullptr)
     ){
    return false;
  }
  assert(p != nullptr);

  /*
   * Prevent inlining a call within a function already altered by inlining
   */
  if (fnsAffected.find(F) != fnsAffected.end()) {
    return false ;
  }

  /*
   * Avoid inlininig recursive calls.
   */
  if (!canInlineWithoutRecursiveLoop(F, childF)) {
    return false ;
  }

  /*
   * Avoid inlining into a function that is too big.
   */
  if (p->getStaticInstructions(F) > 1000){
    return false;
  }

  /*
   * Try to inline the function.
   */
  if (this->verbose != Verbosity::Disabled) {
    call->print(errs() << "Inliner:   Inlining in: " << F->getName() << " (" << p->getStaticInstructions(F) << " instructions. The inlining will add " << p->getStaticInstructions(childF) << " instructions), ");
    errs() << "\n";
  }
  int loopIndAfterCall = getNextPreorderLoopAfter(F, call);
  auto &parentCalls = orderedCalls[F];
  auto callInd = std::find(parentCalls.begin(), parentCalls.end(), call) - parentCalls.begin();

  /*
   * Inline the call.
   */
  InlineFunctionInfo IFI;
  if (InlineFunction(call, IFI)) {
    fnsAffected.insert(F);
    adjustLoopOrdersAfterInline(F, childF, loopIndAfterCall);
    adjustFnGraphAfterInline(F, childF, callInd);
    return true;
  }
  return false;
}

int Inliner::getNextPreorderLoopAfter (Function *F, CallInst *call) {
  if (preOrderedLoops.find(F) == preOrderedLoops.end()) return 0;

  auto &summaries = *preOrderedLoops[F];
  auto getSummaryIfHeader = [&](BasicBlock *BB) -> int {
    for (auto i = 0; i < summaries.size(); ++i) {
      if (summaries[i]->getHeader() == BB) return i;
    }
    return 0;
  };

  // Check all basic blocks after that of the call instruction for the next loop header
  auto bbIter = call->getParent()->getIterator();
  while (++bbIter != F->end()) {
    auto sInd = getSummaryIfHeader(&*bbIter);
    if (sInd != -1) return sInd;
  }
  return 0;
}

/*
 * Function and loop ordering
 */
void Inliner::adjustLoopOrdersAfterInline (Function *parentF, Function *childF, int nextLoopInd) {
  bool parentHasLoops = preOrderedLoops.find(parentF) != preOrderedLoops.end();
  bool childHasLoops = preOrderedLoops.find(childF) != preOrderedLoops.end();
  if (!childHasLoops || preOrderedLoops[childF]->size() == 0) return ;
  if (!parentHasLoops) preOrderedLoops[parentF] = new std::vector<LoopStructure *>();

  /*
   * NOTE(angelo): Starting after the loop in the parent function, index all loops in the
   * child function as being now in the parent function and adjust the indices of loops
   * after the call site by the number of loops inserted
   */
  auto &parentLoops = *preOrderedLoops[parentF];
  auto &childLoops = *preOrderedLoops[childF];
  auto childLoopCount = childLoops.size();
  auto endInd = nextLoopInd + childLoopCount;

  // NOTE(angelo): Adjust parent loops after the call site
  parentLoops.resize(parentLoops.size() + childLoopCount);
  for (auto shiftIndex = parentLoops.size() - 1; shiftIndex >= endInd; --shiftIndex) {
    parentLoops[shiftIndex] = parentLoops[shiftIndex - childLoopCount];
  }

  // NOTE(angelo): Insert inlined loops from child function
  for (auto childIndex = nextLoopInd; childIndex < endInd; ++childIndex) {
    parentLoops[childIndex] = childLoops[childIndex - nextLoopInd];
  }
}

// NOTE(joe) This function doesn't correctly adjust Function Graph, since the function used to compute
// childrenFns and parentFns [collectFnGraph] and therefore depthOrdered and fnOrder [in collectInDepthOrderFns] doesn't
// take into account the defferent function that never got an order. This causes the number to be out between successive
// iterations of this inliner.
void Inliner::adjustFnGraphAfterInline (Function *parentF, Function *childF, int callInd) {
  auto &parentCalled = orderedCalled[parentF];
  auto &childCalled = orderedCalled[childF];

  parentCalled.erase(parentCalled.begin() + callInd);
  if (!childCalled.empty()) {
    auto childCallCount = childCalled.size();
    auto endInsertAt = callInd + childCallCount;

    // Shift over calls after the inlined call to make room for called function's calls
    parentCalled.resize(parentCalled.size() + childCallCount);
    for (auto shiftIndex = parentCalled.size() - 1; shiftIndex >= endInsertAt; --shiftIndex) {
      parentCalled[shiftIndex] = parentCalled[shiftIndex - childCallCount];
    }

    // Insert the called function's calls starting from the position of the inlined call
    for (auto childIndex = callInd; childIndex < endInsertAt; ++childIndex) {
      parentCalled[childIndex] = childCalled[childIndex - callInd];
    }
  }

  // Readjust function graph of the function inlined within
  std::set<Function *> reached;
  childrenFns[parentF].clear();
  parentFns[childF].erase(parentF);
  for (auto F : parentCalled) {
    if (reached.find(F) != reached.end()) continue;
    reached.insert(F);
    childrenFns[parentF].push_back(F);
    parentFns[F].insert(parentF);
  }
}

void Inliner::collectFnGraph (Function *main) {
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
    auto parentF = funcToTraverse.front();
    funcToTraverse.pop();

    collectFnCallsAndCalled(callGraph, parentF);

    // Collect functions' first invocations in program forward order
    childrenFns[parentF].clear();
    std::set<Function *> orderedFns;
    for (auto childF : orderedCalled[parentF]) {
      if (orderedFns.find(childF) != orderedFns.end()) continue;
      orderedFns.insert(childF);
      childrenFns[parentF].push_back(childF);
      parentFns[childF].insert(parentF);
    }

    // Traverse the children not already enqueued to be traversed
    for (auto childF : childrenFns[parentF]) {
      if (reached.find(childF) != reached.end()) continue;
      reached.insert(childF);
      funcToTraverse.push(childF);
    }
  }
}

void Inliner::collectFnCallsAndCalled (llvm::CallGraph &CG, Function *parentF) {

  // Collect call instructions to already linked functions
  std::set<CallInst *> unorderedCalls;
  auto funcCGNode = CG[parentF];
  for (auto &callRecord : make_range(funcCGNode->begin(), funcCGNode->end())) {
    auto weakVH = callRecord.first;
    if (!weakVH.pointsToAliveValue() || !isa<CallInst>(&*weakVH)) continue;
    auto call = (CallInst *)(&*weakVH);
    auto F = call->getCalledFunction();
    if (!F || F->empty()) continue;
    unorderedCalls.insert(call);
  }

  // Sort call instructions in program forward order
  std::unordered_map<BasicBlock *, std::set<CallInst *>> bbCalls;
  for (auto call : unorderedCalls) {
    bbCalls[call->getParent()].insert(call);
  }

  orderedCalls[parentF].clear();
  orderedCalled[parentF].clear();
  for (auto &B : *parentF) {
    if (bbCalls.find(&B) == bbCalls.end()) continue;
    if (bbCalls[&B].size() == 1) {
      auto call = *(bbCalls[&B].begin());
      orderedCalls[parentF].push_back(call);
      orderedCalled[parentF].push_back(call->getCalledFunction());
      continue;
    }

    for (auto &I : B) {
      if (!isa<CallInst>(&I)) continue;
      auto call = (CallInst*)(&I);
      if (bbCalls[&B].find(call) == bbCalls[&B].end()) continue;
      orderedCalls[parentF].push_back(call);
      orderedCalled[parentF].push_back(call->getCalledFunction());
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
void Inliner::collectInDepthOrderFns (Function *main) {
  depthOrderedFns.clear();
  recursiveChainEntranceFns.clear();
  fnOrders.clear();

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

      for (auto F : childrenFns[func]) {
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
          assert(depthOrderedFns.size() > 0);
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
        recursiveChainEntranceFns.insert(left);
        remaining->push_back(left);
        funcToTraverse.push(left);
        assert(depthOrderedFns.size() > 0);
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

void Inliner::createPreOrderedLoopSummariesFor (Function *F) {
  // NOTE(angelo): Enforce managing order instead of recalculating it entirely
  if (preOrderedLoops.find(F) != preOrderedLoops.end()) {
    errs() << "Inliner:   Misuse! Do not collect ordered loops more than once. Manage current ordering.\n";
  }

  auto& LI = getAnalysis<LoopInfoWrapperPass>(*F).getLoopInfo();
  if (LI.empty()) return;
  auto loops = collectPreOrderedLoopsFor(F, LI);

  /*
   * Define the function to get the LLVM loop.
   */
  auto getLLVMLoopFunction = [this](BasicBlock *h) -> Loop *{
    auto f = h->getParent();
    auto& LI = getAnalysis<LoopInfoWrapperPass>(*f).getLoopInfo();
    auto loop = LI.getLoopFor(h);
    return loop;
  };

  // Create summaries for the loops
  preOrderedLoops[F] = new std::vector<LoopStructure *>();
  auto &orderedLoops = *preOrderedLoops[F];
  std::unordered_map<Loop *, LoopStructure *> summaryMap;
  for (auto i = 0; i < loops->size(); ++i) {

    /*
     * Create the summary loop
     */
    auto loop = (*loops)[i];
    auto summary = new LoopStructure(loop);

    /*
     * Keep track of the summary loop.
     */
    loopSummaries.insert(summary);
    orderedLoops.push_back(summary);
    summaryMap[loop] = summary;
  }

  delete loops;
  preOrderedLoops[F] = &orderedLoops;
}

std::vector<Loop *> * Inliner::collectPreOrderedLoopsFor (Function *F, LoopInfo &LI) {
  // Collect loops in program forward order
  auto loops = new std::vector<Loop *>();
  for (auto &B : *F) {
    if (!LI.isLoopHeader(&B)) continue;
    loops->push_back(LI.getLoopFor(&B));
  }
  return loops;
}

void Inliner::sortInDepthOrderFns (std::vector<Function *> &inOrder) {
  std::sort(inOrder.begin(), inOrder.end(), [this](Function *a, Function *b) {
      // NOTE(angelo): Sort functions deepest first
      return fnOrders[a] > fnOrders[b];
      });
}

Inliner::~Inliner () {
  for (auto orderedLoops : preOrderedLoops) {
    delete orderedLoops.second;
  }
  for (auto l : loopSummaries) {
    delete l;
  }
}

}
