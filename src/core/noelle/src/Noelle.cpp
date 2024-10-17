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
#include "arcana/noelle/core/Architecture.hpp"
#include "arcana/noelle/core/Noelle.hpp"

namespace arcana::noelle {

Noelle::Noelle(
    Module &m,
    std::function<llvm::ScalarEvolution &(Function &F)> getSCEV,
    std::function<llvm::LoopInfo &(Function &F)> getLoopInfo,
    std::function<llvm::PostDominatorTree &(Function &F)> getPDT,
    std::function<llvm::DominatorTree &(Function &F)> getDT,
    std::function<llvm::AssumptionCache &(Function &F)> getAssumptionCache,
    std::function<llvm::CallGraph &(void)> getCallGraph,
    std::function<llvm::AAResults &(Function &F)> getAA,
    std::function<llvm::BlockFrequencyInfo &(Function &F)> getBFI,
    std::function<llvm::BranchProbabilityInfo &(Function &F)> getBPI,
    std::unordered_set<Transformation> enabledTransformations,
    Verbosity v,
    PDGVerbosity pdgVerbose,
    double minHot,
    LDGGenerator ldgGenerator,
    CompilationOptionsManager *om,
    bool dumpPDG,
    bool performThePDGComparison,
    bool disableSVF,
    bool disableSVFCallGraph,
    bool disableAllocAA,
    bool disableRA)
  : minHot{ minHot },
    program{ m },
    profiles{ nullptr },
    programDependenceGraph{ nullptr },
    enabledTransformations{ enabledTransformations },
    verbose{ v },
    pdgGenerator{ m,
                  getSCEV,
                  getLoopInfo,
                  getPDT,
                  getCallGraph,
                  getAA,
                  dumpPDG,
                  performThePDGComparison,
                  disableSVF,
                  disableSVFCallGraph,
                  disableAllocAA,
                  disableRA,
                  pdgVerbose },
    ldgGenerator{ ldgGenerator },
    filterFileName{ nullptr },
    hasReadFilterFile{ false },
    loopThreads{},
    techniquesToDisable{},
    DOALLChunkSize{},
    fm{ nullptr },
    gm{ nullptr },
    tm{ nullptr },
    cm{ nullptr },
    om{ om },
    mm{ nullptr },
    linker{ nullptr },
    lt{ getSCEV, getLoopInfo, getPDT, getDT, getAssumptionCache },
    getSCEV{ getSCEV },
    getLoopInfo{ getLoopInfo },
    getPDT{ getPDT },
    getDT{ getDT },
    getCallGraph{ getCallGraph },
    getBFI{ getBFI },
    getBPI{ getBPI },
    aaEngines{},
    log{ NoelleLumberjack, "Noelle" } {

  this->filterFileName = getenv("INDEX_FILE");

  return;
}

Module *Noelle::getProgram(void) const {
  return &(this->program);
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
    this->profiles = new Hot(this->program, this->getBFI, this->getBPI);
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

MayPointsToAnalysis Noelle::getMayPointsToAnalysis(void) const {
  return MayPointsToAnalysis{};
}

LoopTransformer &Noelle::getLoopTransformer(void) {
  auto pdg = this->getProgramDependenceGraph();
  this->lt.setPDG(pdg);

  return lt;
}

uint64_t Noelle::numberOfProgramInstructions(void) const {
  uint64_t t = 0;
  for (auto &F : this->program) {
    if (F.empty()) {
      continue;
    }
    for (auto &BB : F) {
      t += BB.size();
    }
  }

  return t;
}

Noelle::~Noelle() {

  return;
}

TypesManager *Noelle::getTypesManager(void) {
  if (!this->tm) {
    this->tm = new TypesManager(this->program);
  }

  return this->tm;
}

ConstantsManager *Noelle::getConstantsManager(void) {
  if (!this->cm) {
    auto typesManager = this->getTypesManager();
    this->cm = new ConstantsManager(this->program, typesManager);
  }

  return this->cm;
}

Linker *Noelle::getLinker(void) {
  if (!this->linker) {
    auto tm = this->getTypesManager();
    this->linker = new Linker(this->program, tm);
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

  /*
   * Check the entire program.
   */
  auto incorrect = llvm::verifyModule(this->program, &errs());

  return !incorrect;
}

std::set<AliasAnalysisEngine *> Noelle::getAliasAnalysisEngines(void) {

  /*
   * Check if we have collected the engines already.
   */
  if (this->aaEngines.size() == 0) {

    /*
     * We didn't collect the engines yet.
     * Let's collect them now.
     */
    this->aaEngines = LDGGenerator::getLoopAliasAnalysisEngines();
    auto programAAEngines = PDGGenerator::getProgramAliasAnalysisEngines();
    this->aaEngines.insert(programAAEngines.begin(), programAAEngines.end());
  }

  return this->aaEngines;
}

GlobalsManager *Noelle::getGlobalsManager(void) {
  if (!this->gm) {
    this->gm = new GlobalsManager(this->program);
  }

  return this->gm;
}

PDGGenerator &Noelle::getPDGGenerator(void) {
  return this->pdgGenerator;
}

} // namespace arcana::noelle
