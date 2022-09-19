/*
 * Copyright 2016 - 2020  Angelo Matni, Simone Campanoni
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
#include "noelle/core/Architecture.hpp"
#include "noelle/core/Noelle.hpp"
#include "noelle/core/HotProfiler.hpp"

namespace llvm::noelle {

Noelle::Noelle()
  : ModulePass{ ID },
    verbose{ Verbosity::Disabled },
    enableFloatAsReal{ true },
    minHot{ 0.0 },
    program{ nullptr },
    profiles{ nullptr },
    programDependenceGraph{ nullptr },
    hoistLoopsToMain{ false },
    loopAwareDependenceAnalysis{ false },
    fm{ nullptr },
    tm{ nullptr },
    cm{ nullptr },
    om{ nullptr },
    mm{ nullptr },
    linker{ nullptr }{
  return;
}

bool Noelle::canFloatsBeConsideredRealNumbers(void) const {
  return this->enableFloatAsReal;
}

Module *Noelle::getProgram(void) const {
  return this->program;
}

LLVMContext &Noelle::getProgramContext(void) const {

  /*
   * Fetch the program.
   */
  auto p = this->getProgram();
  assert(p != nullptr);

  /*
   * Fetch the context of the program.
   */
  auto &cxt = p->getContext();

  return cxt;
}

std::vector<Function *> *Noelle::getModuleFunctionsReachableFrom(
    Module *module,
    Function *startingPoint) {
  auto functions = new std::vector<Function *>();

  /*
   * Fetch the call graph.
   */
  auto fm = this->getFunctionsManager();
  auto callGraph = fm->getProgramCallGraph();

  /*
   * Compute the set of functions reachable from the starting point.
   */
  std::set<Function *> funcSet;
  std::queue<Function *> funcToTraverse;
  funcToTraverse.push(startingPoint);
  while (!funcToTraverse.empty()) {
    auto func = funcToTraverse.front();
    funcToTraverse.pop();
    if (funcSet.find(func) != funcSet.end())
      continue;
    funcSet.insert(func);

    auto funcCGNode = callGraph->getFunctionNode(func);
    for (auto outEdge : funcCGNode->getOutgoingEdges()) {
      auto calleeNode = outEdge->getCallee();
      auto F = calleeNode->getFunction();
      if (!F) {
        continue;
      }
      if (F->empty()) {
        continue;
      }
      funcToTraverse.push(F);
    }
  }

  /*
   * Iterate over functions of the module and add to the vector only the ones
   * that are reachable from the starting point. This will enforce that the
   * order of the functions returned follows the one of the module.
   */
  for (auto &f : *module) {
    if (funcSet.find(&f) == funcSet.end()) {
      continue;
    }
    functions->push_back(&f);
  }

  /*
   * Sort the functions.
   */
  auto compareFunctions = [](Function *f1, Function *f2) -> bool {
    auto f1Name = f1->getName();
    auto f2Name = f2->getName();
    return (f1Name.compare(f2Name) < 0) ? true : false;
  };

  std::sort(functions->begin(), functions->end(), compareFunctions);

  return functions;
}

uint32_t Noelle::fetchTheNextValue(std::stringstream &stream) {
  uint32_t currentValueRead;

  /*
   * Skip separators
   */
  auto peekChar = stream.peek();
  if ((peekChar == ' ') || (peekChar == '\n')) {
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
  if ((peekChar == ' ') || (peekChar == '\n')) {
    stream.ignore();
  }

  return currentValueRead;
}

Verbosity Noelle::getVerbosity(void) const {
  return this->verbose;
}

double Noelle::getMinimumHotness(void) const {
  return this->minHot;
}

Hot *Noelle::getProfiles(void) {
  if (this->profiles == nullptr) {
    this->profiles = &getAnalysis<HotProfiler>().getHot();
  }

  return this->profiles;
}

DataFlowAnalysis Noelle::getDataFlowAnalyses(void) const {
  return DataFlowAnalysis{};
}

CFGAnalysis Noelle::getCFGAnalysis(void) const {
  return CFGAnalysis{};
}

CFGTransformer Noelle::getCFGTransformer(void) const {
  return CFGTransformer{};
}

DataFlowEngine Noelle::getDataFlowEngine(void) const {
  return DataFlowEngine{};
}

Scheduler Noelle::getScheduler(void) const {
  return Scheduler{};
}

LoopTransformer &Noelle::getLoopTransformer(void) {
  auto &lt = getAnalysis<LoopTransformer>();
  auto pdg = this->getProgramDependenceGraph();
  lt.setPDG(pdg);
  return lt;
}

uint64_t Noelle::numberOfProgramInstructions(void) const {
  uint64_t t = 0;
  for (auto &F : *this->program) {
    if (F.empty()) {
      continue;
    }
    for (auto &BB : F) {
      t += BB.size();
    }
  }

  return t;
}

bool Noelle::shouldLoopsBeHoistToMain(void) const {
  return this->hoistLoopsToMain;
}

Noelle::~Noelle() {

  return;
}

TypesManager *Noelle::getTypesManager(void) {
  if (!this->tm) {
    this->tm = new TypesManager(*this->program);
  }

  return this->tm;
}

ConstantsManager *Noelle::getConstantsManager(void) {
  if (!this->cm) {
    auto typesManager = this->getTypesManager();
    this->cm = new ConstantsManager(*this->program, typesManager);
  }

  return this->cm;
}
  
Linker *Noelle::getLinker(void){
  if (!this->linker){
    auto tm = this->getTypesManager();
    this->linker = new Linker(*this->program, tm);
  }

  return this->linker;
}

CompilationOptionsManager *Noelle::getCompilationOptionsManager(void) {
  assert(this->om != nullptr);
  return this->om;
}

MetadataManager *Noelle::getMetadataManager(void) {
  if (!this->mm) {
    this->mm = new MetadataManager(*this->getProgram());
  }
  return this->mm;
}

bool Noelle::verifyCode(void) const {
  assert(this->program != nullptr);

  /*
   * Check the entire program.
   */
  auto incorrect = llvm::verifyModule(*this->program);

  return !incorrect;
}

} // namespace llvm::noelle
