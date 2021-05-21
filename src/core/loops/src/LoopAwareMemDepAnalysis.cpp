/*
 * Copyright 2016 - 2020  Angelo Matni, Simone Campanoni, Brian Homerding
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "LoopAwareMemDepAnalysis.hpp"
#include "DataFlow.hpp"
#include "Annotation.hpp"
#include "AnnotationParser.hpp"

/*
 * SCAF headers
 */
#define ENABLE_SCAF
#ifdef ENABLE_SCAF
#include "scaf/MemoryAnalysisModules/LoopAA.h"
#include "scaf/Utilities/PDGQueries.h"
#endif

static cl::opt<bool> TalkdownDisable("loop-aware-talkdown-disable", cl::ZeroOrMore, cl::Hidden, cl::desc("Disable Talkdown loop aware dependence analyses"));

namespace llvm::noelle {

/*
 * SCAF
 */
#ifdef ENABLE_SCAF
static liberty::LoopAA *NoelleSCAFAA = nullptr;
#endif

class NoelleSCAFIntegration : public ModulePass {
  public:
    static char ID;

    NoelleSCAFIntegration ();
    bool doInitialization (Module &M) override ;
    void getAnalysisUsage(AnalysisUsage &AU) const override ;
    bool runOnModule (Module &M) override ;
};

// Next there is code to register your pass to "opt"
char NoelleSCAFIntegration::ID = 0;
static RegisterPass<NoelleSCAFIntegration> X("noellescaf", "Integration with SCAF");

// Next there is code to register your pass to "clang"
static NoelleSCAFIntegration * _PassMaker = nullptr;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new NoelleSCAFIntegration()); }}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new NoelleSCAFIntegration()); }});// ** for -O0

void refinePDGWithLoopAwareMemDepAnalysis(
  PDG *loopDG,
  Loop *l,
  LoopStructure *loopStructure,
  LoopsSummary *liSummary,
  TalkDown *talkdown,
  LoopIterationDomainSpaceAnalysis *LIDS
) {

  refinePDGWithSCAF(loopDG, l);

  if (LIDS) {
    refinePDGWithLIDS(loopDG, loopStructure, liSummary, LIDS);
  }

  if (talkdown && TalkdownDisable.getNumOccurrences() == 0) {
      refinePDGWithTalkdown(loopDG, l, loopStructure, liSummary, talkdown);
  }   

}

void refinePDGWithSCAF (PDG *loopDG, Loop *l) {
  #ifdef ENABLE_SCAF
  assert(NoelleSCAFAA != nullptr);
  // Iterate over all the edges of the loop PDG and
  // collect memory deps to be queried.
  // For each pair of instructions with a memory dependence map it to
  // a small vector of found edges (0th element is for RAW, 1st for WAW, 2nd for WAR)
  map<pair<Instruction *, Instruction *>, SmallVector<DGEdge<Value> *, 3>>
      memDeps;
  for (auto edge : make_range(loopDG->begin_edges(), loopDG->end_edges())) {
    if (!loopDG->isInternal(edge->getIncomingT()) ||
        !loopDG->isInternal(edge->getOutgoingT()))
      continue;

    if (!edge->isMemoryDependence())
      continue;

    Value *pdgValueI = edge->getOutgoingT();
    Instruction *i = dyn_cast<Instruction>(pdgValueI);
    assert(i && "Expecting an instruction as the value of a PDG node");

    Value *pdgValueJ = edge->getIncomingT();
    Instruction *j = dyn_cast<Instruction>(pdgValueJ);
    assert(j && "Expecting an instruction as the value of a PDG node");

    if (!memDeps.count({i,j})) {
      memDeps[{i,j}] = {nullptr, nullptr, nullptr};
    }

    if (edge->isRAWDependence()) {
      memDeps[{i,j}][0] = edge;
    }
    else if (edge->isWAWDependence()) {
      memDeps[{i,j}][1] = edge;
    }
    else if (edge->isWARDependence()) {
      memDeps[{i,j}][2] = edge;
    }

  }

  // For each memory depedence perform loop-aware dependence analysis to
  // disprove it. Queries for loop-carried and intra-iteration deps.
  for (auto memDep : memDeps) {
    auto instPair = memDep.first;
    Instruction *i = instPair.first;
    Instruction *j = instPair.second;

    auto edges = memDep.second;

    // encode the found dependences in a bit vector.
    // set least significant bit for RAW, 2nd bit for WAW, 3rd bit for WAR
    uint8_t depTypes = 0;
    for (uint8_t i = 0; i <= 2; ++i) {
      if (edges[i]) {
        depTypes |= 1 << i;
      }
    }
    // Try to disprove all the reported loop-carried deps
    uint8_t disprovedLCDepTypes =
        disproveLoopCarriedMemoryDep(i, j, depTypes, l, NoelleSCAFAA);

    // for every disproved loop-carried dependence
    // check if there is a intra-iteration dependence
    uint8_t disprovedIIDepTypes = 0;
    if (disprovedLCDepTypes) {
      disprovedIIDepTypes = disproveIntraIterationMemoryDep(
          i, j, disprovedLCDepTypes, l, NoelleSCAFAA);

      // remove any edge that SCAF disproved both its loop-carried and
      // intra-iteration version
      for (uint8_t i = 0; i <= 2; ++i) {
        if (disprovedIIDepTypes & (1 << i)) {
          auto &e = edges[i];
          loopDG->removeEdge(e);
        }
      }

      // set LoopCarried bit false for all the non-disproved intra-iteration edges
      // (but were not loop-carried)
      uint8_t iiDepTypes = disprovedLCDepTypes - disprovedIIDepTypes;
      for (uint8_t i = 0; i <= 2; ++i) {
        if (iiDepTypes & (1 << i)) {
          auto &e = edges[i];
          e->setLoopCarried(false);
        }
      }
    }
  }
  #endif

  return ;
}

  bool isSelectedLoop(LoopStructure* ls, const FunctionTree* Ftree) {
    auto head = ls->getHeader();
    for(auto &I : *head) {
      auto annots = parseAnnotationsForInst(&I);
      for (auto A : annots) {
        if (A.getKey() == "selected") {
          if(A.getValue() == "1") {
            return true;
          }   
        }   
      }   
    }   
    return false;
  }

  void refinePDGWithTalkdown(PDG *loopDG, Loop *l, LoopStructure* loopStructure, LoopsSummary* liSummary, TalkDown *talkdown)
  {
    errs() << "BRIAN, LETS REFINE THE PDG WITH TALKDOWN\n";

    auto Ftree = talkdown->findTreeForFunction(l->getHeader()->getParent());

    // early exit if current loop does not have selected
    if (!isSelectedLoop(loopStructure, Ftree)) {
      errs() << "Loop does not have selected\n";
      errs() << *(loopStructure->getHeader());
      return;
    }

    // If parent is selected loop also, then this loop is no longer independent
    auto pl = loopStructure->getParentLoop();
    if (pl && isSelectedLoop(pl, Ftree)) {
      errs() << "Talkdown: parent loop is selected, inner loop is not independent\n";
      return;
    }

 //   for (auto dependency : LoopCarriedDependencies::getLoopCarriedDependenciesForLoop(*loopStructure, *liSummary, *loopDG)) {
    for (auto dependency : loopDG->getEdges()) {
 //     if (!dependency->isLoopCarriedDependence()) continue;

      // TODO: if (!isMemoryDependence()) then continue;
      if (!dependency->isMemoryDependence()) { continue; }

      auto out = dependency->getOutgoingT();
      auto in = dependency->getIncomingT();
      if (auto outI = dyn_cast<Instruction>(out)) {
        //errs() << "It's an instruction, out.\n";
      
//        auto outAnnot = Ftree->getAnnotationsForInst(outI);
        auto outAnnot = parseAnnotationsForInst(outI);
        for (auto &annot : outAnnot) {
//          errs() << "BRIAN: Annotations are " << annot.getKey() << " : " << annot.getValue() << '\n';
          if (annot.getKey() == "independent" && annot.getValue() == "1") {
            errs() << "Brian: FOUND AN EDGE from OUT!!\n";
            errs() << "BRIAN: the out is " << *out << '\n';
            dependency->setLoopCarried(false);
          }
        }
      }

      if (auto inI = dyn_cast<Instruction>(in)) {
  //      errs() << "It's an instruction, out.\n";
      
//        auto inAnnot = Ftree->getAnnotationsForInst(inI);
        auto inAnnot = parseAnnotationsForInst(inI);

        for (auto &annot : inAnnot) {
          if (annot.getKey() == "independent" && annot.getValue() == "1") {
            errs() << "Brian: FOUND AN EDGE from IN!!\n";
            errs() << "BRIAN: the in is " << *in << '\n';
            dependency->setLoopCarried(false);
          }
        }
      }
    }
  for (auto edge : loopDG->getEdges()) {

    if (edge->isMemoryDependence() ) { 
      if(edge->isLoopCarriedDependence()) { 
        errs() << "This shouldn't fail 0 : " << edge->toString() << '\n'; 
        auto out = edge->getOutgoingT();
        auto in = edge->getIncomingT();
        errs() << "Out = " << *out << '\n';
        errs() << "In = " << *in << '\n';

        if (auto outI = dyn_cast<Instruction>(out)) {
          errs() << "It's Out is an Instruction, annotations are : \n";
          auto outAnnot = parseAnnotationsForInst(outI);

          for (auto &annot : outAnnot) {    
            errs() << annot.getKey() << ": " << annot.getValue() << '\n';
          }
        }
        if (auto inI = dyn_cast<Instruction>(out)) {
          errs() << "It's In is an Instruction, annotations are : \n";
          auto inAnnot = parseAnnotationsForInst(inI);

          for (auto &annot : inAnnot) {    
            errs() << annot.getKey() << ": " << annot.getValue() << '\n';
          }
        }
      
      }   
//        assert(!edge->isLoopCarriedDependence() && "flag was already set on loopDG"); 
    }   
  }
/*    for (auto dependency : LoopCarriedDependencies::getLoopCarriedDependenciesForLoop(*loopStructure,     *liSummary, *loopDG)) {
      auto out = dependency->getOutgoingT();
      auto in = dependency->getIncomingT();

      errs() << "BRIAN: OUT " << *out << '\n';
      if (auto outI = dyn_cast<Instruction>(out)) {
        auto outAnnot = Ftree->getAnnotationsForInst(outI);

        for (auto &annot : outAnnot) {    
          errs() << annot.getKey() << ": " << annot.getValue() << '\n';
        }
      }
      errs() << "BRIAN: IN " << *in << '\n';
      if (auto inI = dyn_cast<Instruction>(in)) {
        auto inAnnot = Ftree->getAnnotationsForInst(inI);

        for (auto &annot : inAnnot) {    
          errs() << annot.getKey() << ": " << annot.getValue() << '\n';
        }
      } 
    }

    auto Ftree = talkdown->findTreeForFunction(loop->getHeader()->getParent());
    if (tree) {
      auto leaves = tree->getLeaves();
    for (auto & leaf : leaves) {
      if (leaf->containsAnnotationWithKey("independent") {
        for (auto A : leaf->getAnnotations()) {
          if ("1" == A.getValue()) {
            
          }
          }
      }
    }
*/
    /* Brian: 
     * 1. Get Function from loopDG
     * 2. Get Tree for Function
     * 3. for edge in LCD edges
     * 4. If Incoming and Outgoing both have independent and not critical
     * 5. unset LCD edge
     */
    
    // XXX CHANGE THIS
    // This is very naive, since the rodinia benchmarks have such simple pragmas
    /* if ( !talkdown->containsAnnotation(l) ) */
    /*   return; */

    // XXX this is a hack since the loop pointer changed and Talkdown couldn't find the node for the loop
/*    MDNode *lmd = l->getHeader()->getFirstNonPHIOrDbgOrLifetime()->getMetadata("note.noelle");
    if ( !lmd )
      return;
    if ( l->getParentLoop() && lmd == l->getParentLoop()->getHeader()->getFirstNonPHIOrDbgOrLifetime()->getMetadata("note.noelle") )
      return;

    Instruction *first_inst = &*l->getHeader()->begin();
    errs() << "Found real annotation for loop at " << *first_inst << "\n\t";
    liberty::printInstDebugInfo(first_inst);
    errs() << "\n";

    for (auto edge : make_range(loopDG->begin_edges(), loopDG->end_edges())) {
      if (!loopDG->isInternal(edge->getIncomingT()) ||
          !loopDG->isInternal(edge->getOutgoingT()))
        continue;
*/
      // only target memory dependences (???)
      /* if (!edge->isMemoryDependence()) */
      /*   continue; */

      // don't remove edges that aren't loop-carried
/*      if (!edge->isLoopCarriedDependence())
        continue;

      errs() << "Removed a LC dep with talkdown:\n";
      edge->print(errs()) << "\n";
      edge->setLoopCarried(false);
      talkdownRemoved++;
    }   */
  }


// TODO: Refactor along with HELIX's exact same implementation of this method
DataFlowResult *computeReachabilityFromInstructions (LoopStructure *loopStructure) {

  auto loopHeader = loopStructure->getHeader();
  auto loopFunction = loopStructure->getFunction();

  /*
   * Run the data flow analysis needed to identify the locations where signal instructions will be placed.
   */
  auto dfa = DataFlowEngine{};
  auto computeGEN = [](Instruction *i, DataFlowResult *df) {
    auto& gen = df->GEN(i);
    gen.insert(i);
    return ;
  };
  auto computeOUT = [loopHeader](std::set<Value *>& OUT, Instruction *succ, DataFlowResult *df) {

    /*
    * Check if the successor is the header.
    * In this case, we do not propagate the reachable instructions.
    * We do this because we are interested in understanding the reachability of instructions within a single iteration.
    */
    auto succBB = succ->getParent();
    if (succ == &*loopHeader->begin()) {
      return ;
    }

    /*
    * Propagate the data flow values.
    */
    auto& inS = df->IN(succ);
    OUT.insert(inS.begin(), inS.end());
    return ;
  } ;
  auto computeIN = [](std::set<Value *>& IN, Instruction *inst, DataFlowResult *df) {
    auto& genI = df->GEN(inst);
    auto& outI = df->OUT(inst);
    IN.insert(outI.begin(), outI.end());
    IN.insert(genI.begin(), genI.end());
    return ;
  };

  return dfa.applyBackward(loopFunction, computeGEN, computeIN, computeOUT);
}

void refinePDGWithLIDS(
  PDG *loopDG,
  LoopStructure *loopStructure,
  LoopsSummary *liSummary,
  LoopIterationDomainSpaceAnalysis *LIDS
) {

  auto dfr = computeReachabilityFromInstructions(loopStructure);

  std::unordered_set<DGEdge<Value> *> edgesToRemove;
  for (auto dependency : LoopCarriedDependencies::getLoopCarriedDependenciesForLoop(*loopStructure, *liSummary, *loopDG)) {

    /*
    * Do not waste time on edges that aren't memory dependencies
    */
    if (!dependency->isMemoryDependence()) continue;

    auto fromInst = dyn_cast<Instruction>(dependency->getOutgoingT());
    auto toInst = dyn_cast<Instruction>(dependency->getIncomingT());
    if (!fromInst || !toInst) continue;

    /*
    * Loop carried dependencies are conservatively marked as such; we can only
    * remove dependencies between a producer and consumer where we know the producer
    * can NEVER reach the consumer during the same iteration
    */
    auto &afterInstructions = dfr->OUT(fromInst);
    if (afterInstructions.find(toInst) != afterInstructions.end()) continue;

    if (LIDS->areInstructionsAccessingDisjointMemoryLocationsBetweenIterations(fromInst, toInst)) {
      edgesToRemove.insert(dependency);
    }
  }

  for (auto edge : edgesToRemove) {
    edge->setLoopCarried(false);
    loopDG->removeEdge(edge);
  }

}
      
NoelleSCAFIntegration::NoelleSCAFIntegration ()
  : ModulePass{ID}
{
  return ;
}

bool NoelleSCAFIntegration::doInitialization (Module &M) {
  return false;
}

void NoelleSCAFIntegration::getAnalysisUsage(AnalysisUsage &AU) const {
  #ifdef ENABLE_SCAF
  AU.addRequired<liberty::LoopAA>();
  #endif
  return ;
}
      
bool NoelleSCAFIntegration::runOnModule (Module &M) {
  #ifdef ENABLE_SCAF
  NoelleSCAFAA = getAnalysis<liberty::LoopAA>().getTopAA();
  #endif

  return false;
}

}
