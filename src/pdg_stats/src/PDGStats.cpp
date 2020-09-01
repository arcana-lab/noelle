/*
 * Copyright 2016 - 2020  Yian Su, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "SystemHeaders.hpp"
#include "PDGStats.hpp"

using namespace llvm;

bool PDGStats::doInitialization(Module &M) {
  return false;
}

void PDGStats::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  return;
}

bool PDGStats::runOnModule(Module &M) {
  for (auto &F : M) {
    collectStatsForNodes(F);
    collectStatsForPotentialEdges(F);
    collectStatsForEdges(F);
  }
  printStats();

  return false;
}

void PDGStats::collectStatsForNodes(Function &F) {
  if (MDNode *argsM = F.getMetadata("noelle.pdg.args.id")) {
    this->numberOfNodes += argsM->getNumOperands();
  }

  for (auto &B : F) {
    for (auto &I : B) {
      if (MDNode *m = I.getMetadata("noelle.pdg.inst.id")) {
        this->numberOfNodes++;
      }
    }
  }

  return;
}

void PDGStats::collectStatsForPotentialEdges (Function &F) {
  uint64_t totMemoryInsts = 0;
  for (auto& inst : instructions(F)){
    if (  false
          || isa<LoadInst>(&inst)
          || isa<StoreInst>(&inst)
          || isa<CallInst>(&inst)
          || isa<InvokeInst>(&inst)
      ){
      totMemoryInsts++;
    }
  }
  this->numberOfPotentialMemoryDependences += (totMemoryInsts * totMemoryInsts);

  return ;
}

void PDGStats::collectStatsForEdges(Function &F) {
  if (auto edgesM = F.getMetadata("noelle.pdg.edges")) {
    this->numberOfEdges += edgesM->getNumOperands();

    for (auto &operand : edgesM->operands()) {
      if (MDNode *edgeM = dyn_cast<MDNode>(operand)) {

        // Collect stats for memory dependence
        if (edgeIsDependenceOf(edgeM, IS_MEMORY_DEPENDENCE)) {
          this->numberOfMemoryDependence++;
          if (edgeIsDependenceOf(edgeM, IS_MUST_DEPENDENCE)) {
            this->numberOfMemoryMustDependence++;
          }
        }

        // Collect stats for variable dependence
        else if (edgeIsDependenceOf(edgeM, IS_MUST_DEPENDENCE)) {
          this->numberOfVariableDependence++;
        }
        
        // Collect stats for control dependence
        else if (edgeIsDependenceOf(edgeM, IS_CONTROL_DEPENDENCE)) {
          this->numberOfControlDependence++;
        }
      }
    }
  }

  return;
}

bool PDGStats::edgeIsDependenceOf(MDNode *edgeM, const EDGE_ATTRIBUTE edgeAttribute) {
  if (MDNode *m = dyn_cast<MDNode>(edgeM->getOperand(edgeAttribute))) {
    if (MDString *s = dyn_cast<MDString>(m->getOperand(0))) {
      return s->getString() == "true" ? true : false;
    }
  }

  assert(false && "Error fetching edge attribute from Metadata");
}

void PDGStats::printStats() {
  errs() << "Number of Nodes: " << this->numberOfNodes << "\n";
  errs() << "Number of Edges (a.k.a. dependences): " << this->numberOfEdges << "\n";
  errs() << " Number of control dependences: " << this->numberOfControlDependence << "\n";
  errs() << " Number of data dependences: " << this->numberOfEdges - this->numberOfControlDependence << "\n";
  errs() << "   Number of variable dependences: " << this->numberOfVariableDependence << "\n";
  errs() << "   Number of memory dependences: " << this->numberOfMemoryDependence << "\n";
  errs() << "     Number of memory must dependences: " << this->numberOfMemoryMustDependence << "\n";
  errs() << "     Number of memory may dependences: " << this->numberOfMemoryDependence - this->numberOfMemoryMustDependence << "\n";
  errs() << "     Number of potential memory dependences: " << this->numberOfPotentialMemoryDependences << "\n";

  return;
}

PDGStats::PDGStats()
  : ModulePass{ID} {
  return;
}

PDGStats::~PDGStats() {
  return;
}

// Next there is code to register your pass to "opt"
char PDGStats::ID = 0;
static RegisterPass<PDGStats> X("PDGStats", "Generate statistics output of PDG");

// Next there is code to register your pass to "clang"
static PDGStats * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new PDGStats());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new PDGStats()); }}); // ** for -O0
