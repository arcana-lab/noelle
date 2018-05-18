#include <unordered_map>
#include <set>
#include <queue>
#include <deque>

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/CallGraph.h"

#include "Parallelization.hpp"

using namespace llvm;

bool llvm::Parallelization::doInitialization (Module &M) {

  return false;
}

void llvm::Parallelization::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<CallGraphWrapperPass>();

  return ;
}

bool llvm::Parallelization::runOnModule (Module &M){
  errs() << "Parallelization at \"runOnModule\"\n" ;

  return false;
}

llvm::Parallelization::Parallelization() : ModulePass{ID}{
  return ;
}

std::vector<Function *> * llvm::Parallelization::getModuleFunctions (Module *module){
  auto functions = new std::vector<Function *>();

  /*
   * Fetch the call graph.
   */
  auto &callGraph = getAnalysis<CallGraphWrapperPass>().getCallGraph();

  /* 
   * Compute the set of functions reachable from main.
   */
  std::set<Function *> funcSet ;
  std::queue<Function *> funcToTraverse;
  funcToTraverse.push(module->getFunction("main"));
  while (!funcToTraverse.empty()) {
    auto func = funcToTraverse.front();
    funcToTraverse.pop();
    if (funcSet.find(func) != funcSet.end()) continue;
    funcSet.insert(func);

    auto funcCGNode = callGraph[func];
    for (auto &callRecord : make_range(funcCGNode->begin(), funcCGNode->end())) {
      auto F = callRecord.second->getFunction();
      if (F->empty()) continue;
      funcToTraverse.push(F);
    }
  }

  /*
   * Iterate over functions of the module and add to the vector only the ones that are reachable from "main".
   * This will enforce that the order of the functions returned follows the one of the module.
   */
  for (auto &f : *module){
    if (funcSet.find(&f) == funcSet.end()){
      continue ;
    }
    functions->push_back(&f);
  }

  return functions;
}

std::vector<Loop *> * llvm::Parallelization::getModuleLoops (void){

}

llvm::Function * llvm::Parallelization::createFunctionForTheLoopBody (){

}

llvm::Parallelization::~Parallelization(){
  return ;
}

// Next there is code to register your pass to "opt"
char llvm::Parallelization::ID = 0;
static RegisterPass<Parallelization> X("parallelization", "Computing the Program Dependence Graph");

// Next there is code to register your pass to "clang"
static Parallelization * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new Parallelization());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new Parallelization());}});// ** for -O0
