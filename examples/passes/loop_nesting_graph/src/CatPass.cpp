#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include <algorithm>

#include "arcana/noelle/core/NoellePass.hpp"

using namespace arcana::noelle;

namespace {

struct CAT : public ModulePass {
  static char ID;

  CAT() : ModulePass(ID) {}

  bool doInitialization(Module &M) override {
    return false;
  }

  void printGraph(LoopNestingGraphLoopNode *node,
                  int level,
                  std::unordered_set<LoopNestingGraphLoopNode *> &allNodes,
                  Noelle *noelle,
                  Hot *profiles,
                  bool must) {
    char ic = '-';
    if (!must) {
      ic = '?';
    }
    std::string indent = std::string(level * 3, ic);
    std::string spacedent = std::string(level * 3, ' ');
    auto LS = node->getLoop();
    auto LDI = noelle->getLoopContent(LS);
    auto sccManager = LDI->getSCCManager();
    auto numberOfUnknownSCCs =
        sccManager->getSCCsOfKind(GenericSCC::SCCKind::LOOP_CARRIED_UNKNOWN);
    // bool isOMP = LDI->isOMPLoop();

    // Iterate through the header BB until an instruction has debug information.
    // Use that instruction to print source code info.
    Instruction *inst = nullptr;
    BasicBlock *bb = LS->getHeader();
    for (auto &I : *bb) {
      DILocation *debugLoc = I.getDebugLoc();
      if (debugLoc == nullptr) {
        continue;
      }

      inst = &I;
      break;
    }

    DILocation *debugLoc;
    if (inst) {
      debugLoc = inst->getDebugLoc();
    }

    errs() << indent << ' ';
    /*
     * if (isOMP) {
     *   errs() << "\033[0;33m !!OMP!!  \033[m";
     * }
     */
    if (numberOfUnknownSCCs.size() == 0) {
      errs() << "\033[0;32m !!DOALL!!  \033[m";
    }
    node->print();
    if (inst) {
      errs() << spacedent << " " << debugLoc->getFilename() << ":"
             << debugLoc->getLine() << '\n';
    }
    if (profiles->isAvailable()) {
      auto invo = profiles->getInvocations(LS);
      if (invo) {
        errs() << spacedent << " " << profiles->getIterations(LS)
               << " Iterations, "
               << profiles->getAverageTotalInstructionsPerInvocation(LS)
               << " Avg. Inst/Iter," << profiles->getInvocations(LS)
               << " Invocations, " << profiles->getIterations(LS) / invo
               << " Iter/Inv\n";
      }
    }
    allNodes.erase(node);

    for (auto child : node->getOutgoingEdges()) {
      bool must = child->isAMustEdge();
      printGraph(child->getChild(),
                 level + 1,
                 allNodes,
                 noelle,
                 profiles,
                 must);
    }

    //      errs() << '\n';
    // print node with dashes based on level
    // collect children
    // recurse
  }

  bool runOnModule(Module &M) override {

    /*
     * Fetch NOELLE
     */
    auto &noelle = getAnalysis<NoellePass>().getNoelle();
    auto profiles = noelle.getProfiles();

    /*
     * Fetch the loop nesting graph
     */
    auto lng = noelle.getLoopNestingGraphForProgram();
    std::vector<LoopNestingGraphLoopNode *> rootNodes;

    auto allNodes = lng->getLoopNodes();
    for (auto parent : lng->getLoopNodes()) {
      if (parent->getIncomingEdges().size() == 0) {
        rootNodes.push_back(parent);
      }
    }

    errs() << "\n###############################\n";
    for (auto node : rootNodes) {
      printGraph(node, 0, allNodes, &noelle, profiles, true);
      errs() << "\n###############################\n";
    }

    errs() << "\nLoops not printed at least once = " << allNodes.size() << '\n';

    // Get set of all nodes
    // Find nodes that have no incoming edges
    // From root nodes recurse and print
    // If doall: add color
    // Remove node after it is printed
    // Verify all nodes have been printed

    /*      for (auto parent: lng->getLoopNodes()) {
            // for each node, get the edges
            for (auto edge :  parent->getOutgoingEdges()) {
              edge->print();
              errs() << edge->getParent() << "==>" << edge->getChild() <<  "\n";
            }

          }*/
    // for (auto node : ->getFunctionNodes()){

    /*
     * Fetch the next program's function.
     */
    //   auto f = node->getFunction();
    //   if (f->empty()){
    //     continue ;
    //   }

    /*
     * Fetch the outgoing edges.
     */
    //   auto outEdges = node->getOutgoingEdges();
    //   if (outEdges.size() == 0){
    //     errs() << " The function \"" << f->getName() << "\" has no calls\n";
    //     continue ;
    //   }

    /*
     * Print the outgoing edges.
     */
    //   if (pcf->doesItBelongToASCC(f)){
    //     errs() << " The function \"" << f->getName() << "\" is involved in an
    //     SCC\n";
    //   }
    //   errs() << " The function \"" << f->getName() << "\"";
    //   errs() << " invokes the following functions:\n";
    //   for (auto callEdge : outEdges){
    //     auto calleeNode = callEdge->getCallee();
    //     auto calleeF = calleeNode->getFunction();
    //     errs() << "   [" ;
    //     if (callEdge->isAMustCall()){
    //       errs() << "must";
    //     } else {
    //       errs() << "may";
    //     }
    //     errs() << "] \"" << calleeF->getName() << "\"\n";

    /*
     * Print the sub-edges.
     */
    //     for (auto subEdge : callEdge->getSubEdges()){
    //       auto callerSubEdge = subEdge->getCaller();
    //       errs() << "     [";
    //       if (subEdge->isAMustCall()){
    //         errs() << "must";
    //       } else {
    //         errs() << "may";
    //       }
    //       errs() << "] " << *callerSubEdge->getInstruction() << "\n";
    //     }
    //   }
    // }

    /*
     * Fetch the islands.
     */
    // errs() << "Islands of the program call graph\n";
    // auto islands = pcf->getIslands();
    // auto islandOfMain = islands[mainF];
    // for (auto& F : M){
    //   auto islandOfF = islands[&F];
    //   if (islandOfF != islandOfMain){
    //     errs() << " Function " << F.getName() << " is not in the same island
    //     of main\n";
    //   }
    // }

    /*
     * Fetch the SCCCAG
     */
    // auto sccCAG = pcf->getSCCCAG();
    // auto mainNode = pcf->getFunctionNode(mainF);
    // auto sccOfMain = sccCAG->getNode(mainNode);

    return false;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<NoellePass>();
  }
};
} // namespace

// Next there is code to register your pass to "opt"
char CAT::ID = 0;
static RegisterPass<CAT> X("CAT", "Simple user of the Noelle framework");

// Next there is code to register your pass to "clang"
static CAT *_PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
                                        [](const PassManagerBuilder &,
                                           legacy::PassManagerBase &PM) {
                                          if (!_PassMaker) {
                                            PM.add(_PassMaker = new CAT());
                                          }
                                        }); // ** for -Ox
static RegisterStandardPasses _RegPass2(
    PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
      if (!_PassMaker) {
        PM.add(_PassMaker = new CAT());
      }
    }); // ** for -O0
