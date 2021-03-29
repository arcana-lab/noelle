/*
 * Copyright 2020 Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "TalkDown.hpp"
#include "Annotation.hpp"
#include "OutlinerPass.hpp"
#include <sstream>

using namespace llvm;
using namespace llvm::noelle;

static cl::opt<std::string> AnnotationToOutline("outline-annotation", cl::ZeroOrMore, cl::Hidden, cl::desc("Annotation to outline: 'string=string'"));

bool OutlinerPass::doInitialization (Module &M) {
  std::istringstream ss(AnnotationToOutline);
  std::string key, value;
  ss >> key;
  ss >> value;
  this->annotationToOutline = Annotation(nullptr, key, value);
  return false;
}

void OutlinerPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addUsedIfAvailable<AssumptionCacheTracker>();
  AU.addRequired<TalkDown>();
  return ;
}

struct OutlineInfo {
  Instruction* insertionPoint; 
  BasicBlock* BBtoOutline;
  Function* sourceF;
  OutlineInfo(Instruction* I, BasicBlock* BB, Function* F) : insertionPoint(I), BBtoOutline(BB), sourceF(F) {}
};

bool OutlinerPass::runOnModule (Module &M){
  AssumptionCache* AC;
  Instruction* outlineI;
  Function* outlineF;
  std::vector<OutlineInfo> outlineSet;

  errs() << "Annotation to outline = "<< this->annotationToOutline << '\n';

  auto& talkdown = getAnalysis<TalkDown>();

  
  int counter = 0;
  for ( auto &F : M) {
    auto tree = talkdown.findTreeForFunction(&F);
    if (tree) {
//      errs() << *tree << '\n';
      auto leaves = tree->getLeaves();
      for (auto & leaf : leaves) {
        if (leaf->containsAnnotationWithKey(this->annotationToOutline.getKey())) {
          for (auto A : leaf->getAnnotations()) {
            if (this->annotationToOutline.getValue() == A.getValue()) {
              errs() << "Outline Me!! : " << leaf << '\n';
              auto BB = leaf->getBB();
              auto insertionPoint = &*BB->begin();
              auto F = BB->getParent();
              OutlineInfo outlineIt(insertionPoint, BB, F);
              outlineSet.push_back(outlineIt);
            }
          }
        }
      }
    }
  }

  auto o = getOutliner();
  for ( auto outlineInfo : outlineSet ) {
    errs() << "Going to Outline\n";
    o->outline( outlineInfo.BBtoOutline, outlineInfo.insertionPoint, outlineInfo.sourceF, AC);
  }
  return false;
}

Outliner * OutlinerPass::getOutliner (void) const {
  auto o = new Outliner();

  return o;
}

bool OutlinedInlinerPass::doInitialization (Module &M) {
  return false;
}

void OutlinedInlinerPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addUsedIfAvailable<AssumptionCacheTracker>();
  AU.addRequired<TalkDown>();
  return ;
}

bool OutlinedInlinerPass::runOnModule(Module &M) {
  std::vector<CallInst*> inlineVector;
  std::vector<Function*> functionVector;

  errs() << "Lets inline that outlined!\n";
  for( auto &F : M ) { 
    for( auto &BB : F ) {
      for( auto &I : BB ) {
        if (auto callI = dyn_cast<CallInst>(&I)) {
          auto calledFunc = callI->getCalledFunction();
          if(calledFunc) {
            auto name = calledFunc->getName();
            if (name.find("outliner.") != std::string::npos) {
              inlineVector.push_back(callI);
              functionVector.push_back(calledFunc);
            }
          }
        }
        // if callInst
        // if name has "outliner."
        // add to inline set
      }
    }
  }

/*  for (auto F : functionVector) {
    F->removeFnAttr(Attribute::NoInline);
  }*/

  for (auto callI : inlineVector) {
    errs() << "Trying to remove noinline\n";
    callI->removeAttribute(AttributeList::FunctionIndex, Attribute::NoInline);
    InlineFunctionInfo IFI;
    if (InlineFunction(callI, IFI)) {
      errs() << "Inlined it!!\n";
    }
  }
  // for inline set
  // remove no inline
  // Inline
  // Reapply annotation that is on the function call?
  // remove call inst 
  // remove called function

  return false;
}

// Next there is code to register your pass to "opt"
char OutlinerPass::ID = 0;
static RegisterPass<OutlinerPass> X("outliner", "Outline code");

char OutlinedInlinerPass::ID = 0;
static RegisterPass<OutlinedInlinerPass> Y("outlined-inliner", "Inline outlined code");

// Next there is code to register your pass to "clang"
static OutlinerPass * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new OutlinerPass());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new OutlinerPass());}});// ** for -O0

static OutlinedInlinerPass * _PassMaker1 = NULL;
static RegisterStandardPasses _RegPass3(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker1){ PM.add(_PassMaker1 = new OutlinedInlinerPass());}}); // ** for -Ox
static RegisterStandardPasses _RegPass4(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker1){ PM.add(_PassMaker1 = new OutlinedInlinerPass());}});// ** for -O0
