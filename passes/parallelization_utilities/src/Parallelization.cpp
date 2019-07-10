/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <unordered_map>
#include <set>
#include <queue>
#include <deque>
#include <sstream>
#include <thread>

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
#include "HotProfiler.hpp"
#include "Architecture.hpp"

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
  AU.addRequired<HotProfiler>();

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
      if (!F) {
        continue ;
      }
      if (F->empty()) {
        continue;
      }
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
  double minimumHotness
  ){

  /*
   * Fetch the profiles.
   */
  auto& profiles = getAnalysis<HotProfiler>().getHot();

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
  std::vector<uint32_t> loopThreads{};
  std::vector<uint32_t> DOALLChunkSize{};
  std::vector<uint32_t> techniquesToDisable{};
  auto indexFileName = getenv("INDEX_FILE");
  auto filterLoops = this->filterOutLoops(indexFileName, loopThreads, techniquesToDisable, DOALLChunkSize);

  /*
   * Append loops of each function.
   */
  auto currentLoopIndex = 0;
  errs() << "Parallelizer: Filter out cold code\n" ;
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
     * Check if the function is hot.
     */
    if (profiles.isAvailable()){
      auto mInsts = profiles.getModuleInstructions();
      auto fInsts = profiles.getFunctionInstructions(function);
      auto hotness = ((double)fInsts) / ((double)mInsts);
      if (hotness <= minimumHotness){
        errs() << "Parallelizer:  Disable \"" << function->getName() << "\" as cold function\n";
        continue ;
      }
    }

    /*
     * Fetch the function dependence graph.
     */
    // FIXME: Possible memory leak, this is the only pointer t funcPDG (so owner)
    // FIXME: LDI doesn't clean up funcPDG
    auto funcPDG = getAnalysis<PDGAnalysis>().getFunctionPDG(*function);

    /*
     * Fetch the post dominators and scalar evolutions
     */
    auto& PDT = getAnalysis<PostDominatorTreeWrapperPass>(*function).getPostDomTree();
    auto& SE = getAnalysis<ScalarEvolutionWrapperPass>(*function).getSE();

    /*
     * Fetch all loops of the current function.
     */
    auto loops = LI.getLoopsInPreorder();

    /*
     * Consider these loops.
     */
    for (auto loop : loops){

      /*
       * Check if the loop is hot enough.
       */
       if (profiles.isAvailable()){
        auto mInsts = profiles.getModuleInstructions();
        auto lInsts = profiles.getLoopInstructions(loop);
        auto hotness = ((double)lInsts) / ((double)mInsts);
        if (hotness <= minimumHotness){
          errs() << "Parallelizer:  Disable loop \"" << currentLoopIndex << "\" as cold code\n";
          currentLoopIndex++;
          continue ;
        }
        errs() << "Parallelizer:  Loop hotness = " << hotness << "\n" ;
      }

      /*
       * Check if we have to filter loops.
       */
      if (!filterLoops){

        /*
         * Allocate the loop wrapper.
         */
        auto ldi = new LoopDependenceInfo(function, funcPDG, loop, LI, SE, PDT);

        allLoops->push_back(ldi);
        currentLoopIndex++;
        continue ;
      }

      /*
       * We need to filter loops.
       *
       * Check if more than one thread is assigned to the current loop.
       * If that's the case, then we have to enable that loop.
       */
      auto maximumNumberOfCoresForTheParallelization = loopThreads[currentLoopIndex];
      if (maximumNumberOfCoresForTheParallelization <= 1){

        /*
         * Only one thread has been assigned to the current loop.
         * Hence, the current loop will not be parallelized.
         */
        currentLoopIndex++;

        /*
         * Jump to the next loop.
         */
        continue ;
      }

      /*
       * Safety code.
       */
      if (currentLoopIndex >= loopThreads.size()){
        errs() << "ERROR: the 'INDEX_FILE' file isn't correct. There are more than " << loopThreads.size() << " loops available in the program\n";
        abort();
      }

      /*
       * The current loop has more than one core assigned to it.
       * Therefore, we need to parallelize this loop.
       *
       * Allocate the loop wrapper.
       */
      auto ldi = new LoopDependenceInfo(function, funcPDG, loop, LI, SE, PDT);

      /*
       * Set the loop constraints specified by INDEX_FILE.
       *
       * DOALL chunk size is the one defined by INDEX_FILE + 1. This is because chunk size must start from 1.
       */
      ldi->DOALLChunkSize = DOALLChunkSize[currentLoopIndex] + 1;

      /*
       * Set the maximum number of threads that we can extract from the current loop.
       */
      ldi->maximumNumberOfCoresForTheParallelization = maximumNumberOfCoresForTheParallelization;

      /*
       * Set the techniques that are enabled.
       */
      auto disableTechniques = techniquesToDisable[currentLoopIndex];
      switch (disableTechniques){

        case 0:
          ldi->enableAllTechniques();
          break ;

        case 1:
          ldi->disableTechnique(DSWP_ID);
          break ;

        case 2:
          ldi->disableTechnique(HELIX_ID);
          break ;

        case 3:
          ldi->disableTechnique(DOALL_ID);
          break ;

        case 4:
          ldi->disableTechnique(DSWP_ID);
          ldi->disableTechnique(HELIX_ID);
          break ;

        case 5:
          ldi->disableTechnique(DSWP_ID);
          ldi->disableTechnique(DOALL_ID);
          break ;

        case 6:
          ldi->disableTechnique(HELIX_ID);
          ldi->disableTechnique(DOALL_ID);
          break ;

        default:
          abort();
      }

      /*
       * The current loop needs to be considered as specified by the user.
       */
      allLoops->push_back(ldi);
      currentLoopIndex++;
    }
  }

  return allLoops;
}

uint32_t Parallelization::getNumberOfModuleLoops (
  Module *module,
  double minimumHotness
  ){
  uint32_t counter = 0;

  /*
   * Fetch the profiles.
   */
  auto& profiles = getAnalysis<HotProfiler>().getHot();

  /*
   * Fetch the list of functions of the module.
   */
  auto mainFunction = module->getFunction("main");
  auto functions = this->getModuleFunctionsReachableFrom(module, mainFunction);

  /*
   * Check if we should filter out loops.
   */
  std::vector<uint32_t> loopThreads{};
  std::vector<uint32_t> techniquesToDisable{};
  std::vector<uint32_t> DOALLChunkSize{};
  auto indexFileName = getenv("INDEX_FILE");
  auto filterLoops = this->filterOutLoops(indexFileName, loopThreads, techniquesToDisable, DOALLChunkSize);

  /*
   * Append loops of each function.
   */
  auto currentLoopIndex = 0;
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
     * Check if the function is hot.
     */
    if (profiles.isAvailable()){
      auto mInsts = profiles.getModuleInstructions();
      auto fInsts = profiles.getFunctionInstructions(function);
      auto hotness = ((double)fInsts) / ((double)mInsts);
      if (hotness <= minimumHotness){
        continue ;
      }
    }

    /*
     * Fetch all loops of the current function.
     */
    auto loops = LI.getLoopsInPreorder();

    /*
     * Consider these loops.
     */
    for (auto loop : loops){

      /*
       * Check if the loop is hot enough.
       */
       if (profiles.isAvailable()){
        auto mInsts = profiles.getModuleInstructions();
        auto lInsts = profiles.getLoopInstructions(loop);
        auto hotness = ((double)lInsts) / ((double)mInsts);
        if (hotness <= minimumHotness){
          currentLoopIndex++;
          continue ;
        }
      }

      /*
       * Check if we have to filter loops.
       */
      if (!filterLoops){
        counter++;
        currentLoopIndex++;
        continue ;
      }

      /*
       * We need to filter loops.
       *
       * Check if more than one thread is assigned to the current loop.
       * If that's the case, then we have to enable that loop.
       */
      auto maximumNumberOfCoresForTheParallelization = loopThreads[currentLoopIndex];
      if (maximumNumberOfCoresForTheParallelization <= 1){

        /*
         * Only one thread has been assigned to the current loop.
         * Hence, the current loop will not be parallelized.
         */
        currentLoopIndex++;

        /*
         * Jump to the next loop.
         */
        continue ;
      }

      /*
       * The current loop has more than one core assigned to it.
       * Therefore, we need to parallelize this loop.
       * In other words, the current loop needs to be considered as specified by the user.
       */
      counter++;
      currentLoopIndex++;
    }
  }

  return counter;
}

void llvm::Parallelization::linkParallelizedLoopToOriginalFunction (
  Module *module,
  BasicBlock *originalPreHeader,
  BasicBlock *startOfParLoopInOriginalFunc,
  BasicBlock *endOfParLoopInOriginalFunc,
  Value *envArray,
  Value *envIndexForExitVariable,
  SmallVector<BasicBlock *, 10> &loopExitBlocks
  ){

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
  loopSwitchBuilder.CreateCondBr(
    compareInstruction,
    startOfParLoopInOriginalFunc,
    originalHeader
  );
  originalTerminator->eraseFromParent();

  IRBuilder<> endBuilder(endOfParLoopInOriginalFunc);

  /*
   * Load exit block environment variable and branch to the correct loop exit block
   */
  if (loopExitBlocks.size() == 1) {
    endBuilder.CreateBr(loopExitBlocks[0]);
  } else {

    /*
     * Compute how many values can fit in a cache line.
     */
    auto valuesInCacheLine = Architecture::getCacheLineBytes() / sizeof(int64_t);

    auto exitEnvPtr = endBuilder.CreateInBoundsGEP(
      envArray,
      ArrayRef<Value*>({
        cast<Value>(ConstantInt::get(int64, 0)),
        endBuilder.CreateMul(envIndexForExitVariable, ConstantInt::get(int64, valuesInCacheLine))
      })
    );
    auto exitEnvCast = endBuilder.CreateIntCast(endBuilder.CreateLoad(exitEnvPtr), int32, /*isSigned=*/false);
    auto exitSwitch = endBuilder.CreateSwitch(exitEnvCast, loopExitBlocks[0]);
    for (int i = 1; i < loopExitBlocks.size(); ++i) {
      exitSwitch->addCase(ConstantInt::get(int32, i), loopExitBlocks[i]);
    }
  }

  /*
   * NOTE(angelo): LCSSA constants need to be replicated for parallelized code path
   */
  for (auto bb : loopExitBlocks) {
    for (auto &I : *bb) {
      if (auto phi = dyn_cast<PHINode>(&I)) {
        auto bbIndex = phi->getBasicBlockIndex(originalHeader);
        if (bbIndex == -1) {
          continue;
        }
        auto val = phi->getIncomingValue(bbIndex);
        if (isa<Constant>(val)) {
          phi->addIncoming(val, endOfParLoopInOriginalFunc);
        }
        continue;
      }
      break;
    }
  }

  /*
   * Set/Reset global variable so only one invocation of the loop is run in parallel at a time.
   */
  if (startOfParLoopInOriginalFunc == endOfParLoopInOriginalFunc) {
    endBuilder.SetInsertPoint(&*endOfParLoopInOriginalFunc->begin());
    endBuilder.CreateStore(const1, globalBool);
  } else {
    IRBuilder<> startBuilder(&*startOfParLoopInOriginalFunc->begin());
    startBuilder.CreateStore(const1, globalBool);
  }
  endBuilder.SetInsertPoint(endOfParLoopInOriginalFunc->getTerminator());
  endBuilder.CreateStore(const0, globalBool);

  return ;
}

uint32_t llvm::Parallelization::fetchTheNextValue (std::stringstream &stream){
  uint32_t currentValueRead;

  /*
   * Skip separators
   */
  auto peekChar = stream.peek();
  if (  (peekChar == ' ')   ||
        (peekChar == '\n')  ){
    stream.ignore();
  }

  /*
   * Parse the value.
   */
  stream >> currentValueRead;

  /*
   * Skip separators
   */
  peekChar = stream.peek();
  if (  (peekChar == ' ')   ||
        (peekChar == '\n')  ){
    stream.ignore();
  }

  return currentValueRead;
}
      
bool Parallelization::filterOutLoops (
  char *fileName,
  std::vector<uint32_t>& loopThreads,
  std::vector<uint32_t>& techniquesToDisable,
  std::vector<uint32_t>& DOALLChunkSize
  ){

  /*
   * Check the name of the file that lists the loops to consider.
   */
  if (!fileName){
    return false;
  }

  /*
   * We need to filter out loops.
   *
   * Open the file that specifies which loops to keep.
   */
  auto indexBuf = MemoryBuffer::getFileAsStream(fileName);
  if (auto ec = indexBuf.getError()){
    errs() << "Failed to read INDEX_FILE = \"" << fileName << "\":" << ec.message() << "\n";
    abort();
  }

  /*
   * Read the file.
   */
  auto fileAsString = indexBuf.get()->getBuffer().str();
  std::stringstream indexString{fileAsString};

  /*
   * Parse the file
   */
  auto filterLoops = false;
  while (indexString.peek() != EOF){
    filterLoops = true;

    /*
     * Should the loop be parallelized?
     */
    auto shouldBeParallelized = this->fetchTheNextValue(indexString);
    assert(shouldBeParallelized == 0 || shouldBeParallelized == 1);

    /*
     * Unroll factor
     */
    auto unrollFactor = this->fetchTheNextValue(indexString);

    /*
     * Peel factor
     */
    auto peelFactor = this->fetchTheNextValue(indexString);

    /*
     * Techniques to disable
     * 0: None
     * 1: DSWP
     * 2: HELIX
     * 3: DOALL
     * 4: DSWP, HELIX
     * 5: DSWP, DOALL
     * 6: HELIX, DOALL
     */
    auto technique = this->fetchTheNextValue(indexString);

    /*
     * Number of cores
     */
    auto cores = this->fetchTheNextValue(indexString);

    /*
     * DOALL: chunk factor
     */
    auto DOALLChunkFactor = this->fetchTheNextValue(indexString);

    /*
     * Skip
     */
    this->fetchTheNextValue(indexString);
    this->fetchTheNextValue(indexString);
    this->fetchTheNextValue(indexString);

    /*
     * If the loop needs to be parallelized, then we enable it.
     */
    if (  (shouldBeParallelized)    &&
          (cores >= 2)              ){
      loopThreads.push_back(cores);
      techniquesToDisable.push_back(technique);
      DOALLChunkSize.push_back(DOALLChunkFactor);

    } else{
      loopThreads.push_back(1);
      techniquesToDisable.push_back(0);
      DOALLChunkSize.push_back(0);
    }
  }
  
  return filterLoops;
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
