#include <unordered_map>
#include <set>
#include <queue>
#include <deque>
#include <sstream>

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
#include "llvm/Support/MemoryBuffer.h"

#include "PDGAnalysis.hpp"
#include "Parallelization.hpp"

using namespace llvm;

bool llvm::Parallelization::doInitialization (Module &M) {
  int1 = IntegerType::get(M.getContext(), 1);
  int8 = IntegerType::get(M.getContext(), 8);
  int16 = IntegerType::get(M.getContext(), 16);
  int32 = IntegerType::get(M.getContext(), 32);
  int64 = IntegerType::get(M.getContext(), 64);

  return false;
}

void llvm::Parallelization::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<CallGraphWrapperPass>();
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<AssumptionCacheTracker>();
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<PostDominatorTreeWrapperPass>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
  AU.addRequired<PDGAnalysis>();

  return ;
}

bool llvm::Parallelization::runOnModule (Module &M){
  errs() << "Parallelization at \"runOnModule\"\n" ;

  return false;
}

llvm::Parallelization::Parallelization() : ModulePass{ID}{
  return ;
}

std::vector<Function *> * llvm::Parallelization::getModuleFunctionsReachableFrom (Module *module, Function *startingPoint){
  auto functions = new std::vector<Function *>();

  /*
   * Fetch the call graph.
   */
  auto &callGraph = getAnalysis<CallGraphWrapperPass>().getCallGraph();

  /* 
   * Compute the set of functions reachable from the starting point.
   */
  std::set<Function *> funcSet ;
  std::queue<Function *> funcToTraverse;
  funcToTraverse.push(startingPoint);
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
   * Iterate over functions of the module and add to the vector only the ones that are reachable from the starting point.
   * This will enforce that the order of the functions returned follows the one of the module.
   */
  for (auto &f : *module){
    if (funcSet.find(&f) == funcSet.end()){
      continue ;
    }
    functions->push_back(&f);
  }

  /*
   * Sort the functions.
   */
  auto compareFunctions = [] (Function *f1, Function *f2) -> bool {
    auto f1Name = f1->getName();
    auto f2Name = f2->getName();
    return (f1Name.compare(f2Name) < 0) ? true : false;
  };

  std::sort(functions->begin(), functions->end(), compareFunctions);

  return functions;
}

std::vector<LoopDependenceInfo *> * llvm::Parallelization::getModuleLoops (
  Module *module, 
  std::function<LoopDependenceInfo * (Function *, PDG *, Loop *, LoopInfo &)> allocationFunction
  ){

  /* 
   * Fetch the PDG.
   */
  auto graph = getAnalysis<PDGAnalysis>().getPDG();

  /*
   * Allocate the vector of loops.
   */
  auto allLoops = new std::vector<LoopDependenceInfo *>();

  /*
   * Fetch the list of functions of the module.
   */
  auto mainFunction = module->getFunction("main");
  auto functions = this->getModuleFunctionsReachableFrom(module, mainFunction);

  /*
   * Check if we should filter out loops.
   */
  int32_t loopIndex = -1;
  auto indexFileName = getenv("INDEX_FILE");
  if (indexFileName){
    auto indexBuf = MemoryBuffer::getFileAsStream(indexFileName);
    if (auto ec = indexBuf.getError()){
      errs() << "Failed to read \"INDEX_FILE\"\n";
      abort();
    }
    std::stringstream indexString;
    indexString << indexBuf.get()->getBuffer().str();
    if (!indexString.rdbuf()->in_avail()) {
      errs() << "Failed to read \"INDEX_FILE\"\n";
      abort();
    }
    loopIndex = stoi(indexString.str());
  }
  auto filterLoops = (loopIndex != -1) ? true : false;

  /*
   * Append loops of each function.
   */
  auto currentLoopIndex = loopIndex;
  for (auto function : *functions){

    /*
     * Fetch the loop analysis.
     */
    auto& LI = getAnalysis<LoopInfoWrapperPass>(*function).getLoopInfo();

    /*
     * Check if the function has loops.
     */
    if (std::distance(LI.begin(), LI.end()) == 0){
      continue ;
    }

    /*
     * Fetch the function dependence graph.
     */
    auto funcPDG = graph->createFunctionSubgraph(*function);

    /*
     * Fetch the dominators.
     */
    //auto& DT = getAnalysis<DominatorTreeWrapperPass>(*function).getDomTree();
    //auto& PDT = getAnalysis<PostDominatorTreeWrapperPass>(*function).getPostDomTree();
    //auto& SE = getAnalysis<ScalarEvolutionWrapperPass>(*function).getSE();

    /*
     * Fetch all loops of the current function.
     */
    auto loops = LI.getLoopsInPreorder();

    /*
     * Append these loops.
     */
    for (auto loop : loops){
      auto ldi = allocationFunction(function, funcPDG, loop, LI);
      if (filterLoops){
        if (currentLoopIndex == 0){
          allLoops->push_back(ldi);
        }
        currentLoopIndex--;

      } else {
        allLoops->push_back(ldi);
      }
    }
  }

  return allLoops;
}

void llvm::Parallelization::linkParallelizedLoopToOriginalFunction (Module *module, BasicBlock *originalPreHeader, BasicBlock *startOfParallelizedLoopWithinOriginalFunction){

  /*
   * Create the global variable for the parallelized loop.
   */
  auto globalBool = new GlobalVariable(*module, int32, /*isConstant=*/ false, GlobalValue::ExternalLinkage, Constant::getNullValue(int32));
  auto const0 = ConstantInt::get(int32, 0);
  auto const1 = ConstantInt::get(int32, 1);

  /*
   * Fetch the terminator of the preheader.
   */
  auto originalTerminator = originalPreHeader->getTerminator();

  /*
   * Fetch the header of the original loop.
   */
  auto originalHeader = originalTerminator->getSuccessor(0);

  /*
   * Check if another invocation of the loop is running in parallel.
   */
  IRBuilder<> loopSwitchBuilder(originalTerminator);
  auto globalLoad = loopSwitchBuilder.CreateLoad(globalBool);
  auto compareInstruction = loopSwitchBuilder.CreateICmpEQ(globalLoad, const0);
  loopSwitchBuilder.CreateCondBr(compareInstruction, startOfParallelizedLoopWithinOriginalFunction, originalHeader);
  originalTerminator->eraseFromParent();

  /*
   * Set/Reset global variable so only one invocation of the loop is run in parallel at a time.
   */
  IRBuilder<> pipelineBuilder(&*startOfParallelizedLoopWithinOriginalFunction->begin());
  pipelineBuilder.CreateStore(const1, globalBool);
  pipelineBuilder.SetInsertPoint(startOfParallelizedLoopWithinOriginalFunction->getTerminator());
  pipelineBuilder.CreateStore(const0, globalBool);

  return ;
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
