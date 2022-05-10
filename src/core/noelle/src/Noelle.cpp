/*
 * Copyright 2016 - 2020  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/core/Architecture.hpp"
#include "noelle/core/Noelle.hpp"
#include "noelle/core/HotProfiler.hpp"

namespace llvm::noelle{

Noelle::Noelle() 
  : ModulePass{ID}
  , verbose{Verbosity::Disabled}
  , enableFloatAsReal{true}
  , minHot{0.0}
  , program{nullptr}
  , profiles{nullptr}
  , programDependenceGraph{nullptr}
  , hoistLoopsToMain{false}
  , loopAwareDependenceAnalysis{false}
  , fm{nullptr}
  , tm{nullptr}
  , om{nullptr}
  , mm{nullptr}
{
  return ;
}
      
bool Noelle::canFloatsBeConsideredRealNumbers (void) const {
  return this->enableFloatAsReal;
}
      
Module * Noelle::getProgram (void) const {
  return this->program;
}

std::vector<Function *> * Noelle::getModuleFunctionsReachableFrom (Module *module, Function *startingPoint){
  auto functions = new std::vector<Function *>();

  /*
   * Fetch the call graph.
   */
  auto fm = this->getFunctionsManager();
  auto callGraph = fm->getProgramCallGraph();

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

    auto funcCGNode = callGraph->getFunctionNode(func);
    for (auto outEdge : funcCGNode->getOutgoingEdges()){
      auto calleeNode = outEdge->getCallee();
      auto F = calleeNode->getFunction();
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

void Noelle::linkTransformedLoopToOriginalFunction (
    Module *module,
    BasicBlock *originalPreHeader,
    BasicBlock *startOfParLoopInOriginalFunc,
    BasicBlock *endOfParLoopInOriginalFunc,
    Value *envArray,
    Value *envIndexForExitVariable,
    std::vector<BasicBlock *> &loopExitBlocks
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

uint32_t Noelle::fetchTheNextValue (std::stringstream &stream){
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

Verbosity Noelle::getVerbosity (void) const {
  return this->verbose;
}

double Noelle::getMinimumHotness (void) const {
  return this->minHot;
}

Hot * Noelle::getProfiles (void) {
  if (this->profiles == nullptr){
    this->profiles = &getAnalysis<HotProfiler>().getHot();
  }

  return this->profiles;
}

DataFlowAnalysis Noelle::getDataFlowAnalyses (void) const {
  return DataFlowAnalysis{};
}

DataFlowEngine Noelle::getDataFlowEngine (void) const {
  return DataFlowEngine{};
}

Scheduler Noelle::getScheduler (void) const {
  return Scheduler{};
}

LoopTransformer & Noelle::getLoopTransformer (void) {
  auto &lt = getAnalysis<LoopTransformer>();
  auto pdg = this->getProgramDependenceGraph();
  lt.setPDG(pdg);
  return lt;
}

uint64_t Noelle::numberOfProgramInstructions (void) const {
  uint64_t t = 0;
  for (auto &F : *this->program){
    if (F.empty()){
      continue ;
    }
    for (auto &BB : F){
      t += BB.size();
    }
  }

  return t;
}

bool Noelle::shouldLoopsBeHoistToMain (void) const {
  return this->hoistLoopsToMain;
}

Noelle::~Noelle(){

  return ;
}

TypesManager * Noelle::getTypesManager (void) {
  if (!this->tm){
    this->tm = new TypesManager(*this->program);
  }
  return this->tm;
}
      
CompilationOptionsManager * Noelle::getCompilationOptionsManager (void) {
  assert(this->om != nullptr);
  return this->om;
}

MetadataManager * Noelle::getMetadataManager (void) {
  if (!this->mm){
    this->mm = new MetadataManager(*this->getProgram());
  }
  return this->mm;
}
      
bool Noelle::verifyCode (void) const {
  assert(this->program != nullptr);

  /*
   * Check the entire program.
   */
  auto incorrect = llvm::verifyModule(*this->program);

  return !incorrect;
}

}
