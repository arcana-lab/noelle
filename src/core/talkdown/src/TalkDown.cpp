#define DEBUG_TYPE "talkdown"

#include "SystemHeaders.hpp"

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/IR/Function.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

// #include "liberty/LoopProf/Targets.h"
// #include "liberty/Utilities/ReportDump.h"

//#include "PDGAnalysis.hpp"

#include "TalkDown.hpp"
#include "Node.hpp"
#include "AnnotationParser.hpp"
#include "Annotation.hpp"

#include <iostream>
#include <map>
#include <string>

using namespace llvm;
using namespace llvm::noelle;
using namespace AutoMP;

namespace llvm
{
namespace {
  // this pass splits basic blocks whenever there are differing annotations
  struct SplitByAnnotation : public FunctionPass
  {
    SplitByAnnotation() : FunctionPass(ID) { }

    static char ID;

    bool runOnFunction(Function &F)
    {
      if ( F.isDeclaration() )
        return false;

      bool modified = splitBasicBlocksByAnnotation(F);
      modified |= fixBasicBlockAnnotations(F);

      return modified;
    }
    bool splitBasicBlocksByAnnotation(Function &F)
    {
      std::vector<Instruction *> split_points; // points to split basic blocks

      // go through all basic blocks first before performing the split to keep iterators valid
      for ( auto &bb : F )
      {
        AnnotationSet prev_annots; // seen annotations
        for ( auto &i : bb )
        {
          // XXX Once we transition to intrinsics, this will have to be changed
          if ( isa<IntrinsicInst>(&i) )
          {
            continue;
          }

          auto annots = parseAnnotationsForInst( &i );

          /* sometimes the frontend doesn't attach metadata to instructions that should have metadata attached
           * (e.g. on some getelementptr instructions)
           * for now, fix these and consider them as having the same annotations as the previous instruction
           */

          // found mismatch -- should split basic block between i-1 and i
          // XXX I don't think this is 100% correct. It will probably fail for the following code:
          // for (...) {
          //   #pragma note noelle
          //   {
          //     printf("With annotation\n");
          //   }
          //   printf("No annotation\n");
          // }
          if ( prev_annots.size() != 0 && annots.size() != 0 && annots != prev_annots )
          {
            // invalidated_bbs.insert( i.getParent() );
            errs() << "Split point found at " << *&i << "\n";
            errs() << "Previous metadata was:\n";
            for ( const auto &m : prev_annots )
              errs() << m;
            errs() << "Current metadata is:\n";
            for ( const auto &m : annots )
              errs() << m;
            split_points.push_back( &i );
          }

          prev_annots = annots;
        }
      }

      // if no split points are found, don't do anything
      if ( !split_points.size() )
        return false;

      // actually do the basic block splitting
      for ( auto &i : split_points )
      {
        BasicBlock *old = i->getParent();
        BasicBlock *new_block = SplitBlock( old, i );
      }

      return true;
    }

    // fix when the frontend doesn't attach annotation to every instructions
    // XXX doesn't work yet with nested annotations
    bool fixBasicBlockAnnotations(Function &F)
    {
      LLVMContext &ctx = F.getContext(); // a function is contained within a single context
      bool modified = false;

      for ( auto &bb : F )
      {
        MDNode *md = nullptr;
        MDNode *check_md = nullptr;

        // Making sure that splitBasicBlocksByAnnotation() worked correctly
        for ( auto &i : bb )
        {
          md = i.getMetadata( "note.noelle" );
          if ( !md )
            continue;
          if ( !check_md )
            check_md = md;
          assert(check_md == md &&
              "Mismatch found within a basic block after running splitBasicBlocksByAnnotation()" );
        }

        for ( auto &i : bb )
        {
          md = i.getMetadata( "note.noelle" );
          if ( md )
          {
            break;
          }
        }

        // if no metadata in the basic block
        if ( !md )
          continue;

        // insert it into each instruction in the basic block
        for (auto &i : bb )
        {
          MDNode *meta = i.getMetadata("note.noelle");
          if ( !meta )
          {
            errs() << "Setting noelle metadata for instruction " << *&i << "\n";
            i.setMetadata("note.noelle", md);
            modified |= true;
          }
        }
      }

      return modified;
    }
  };
} // anonymous namespace

  /*
   * Options for talkdown
   */
  // This was useful in noelle. But in cpf we have the `aa` script that we can add or remove LoopAA passes to/from
  static cl::opt<bool> TalkDownDisable("talkdown-disable", cl::ZeroOrMore, cl::Hidden, cl::desc("Disable TalkDown"));
  static cl::opt<bool> PrintFunctionTrees("print-function-trees", cl::ZeroOrMore, cl::Hidden, cl::desc("Print out function trees"));

  bool TalkDown::runOnModule(Module &M)
  {
    if ( !this->enabled )
      return false;

    // construct tree for each function
    for ( auto &f : M )
    {
      if ( f.isDeclaration() || f.size() == 0 )
        continue;
//      errs() << "Constructing trees\n";
      LoopInfo &loop_info = getAnalysis<LoopInfoWrapperPass>(f).getLoopInfo();
      FunctionTree tree = FunctionTree( &f );
      tree.constructTree( &f, loop_info );
      function_trees.push_back( tree );
    }

    // maybe use REPORT_DUMP for this?
    if ( PrintFunctionTrees.getNumOccurrences() != 0 )
    {
      errs() << "\n-------- Begin printing of function trees --------\n";
      for ( auto &tree : function_trees )
      {
        llvm::errs() << tree;
        // tree.writeDotFile(tree.getFunction()->getName().str());
      }
      errs() << "\n-------- Done printing function trees --------\n";
    }

    return false;
  }

  void TalkDown::getAnalysisUsage(AnalysisUsage &AU) const
  {
    AU.addRequired<LoopInfoWrapperPass>();
//    AU.addRequired<PDGAnalysis>();
    // AU.addRequiredTransitive<SplitByAnnotation>();
    // AU.addRequired<liberty::ModuleLoops>();
    AU.setPreservesAll();
  }

  bool TalkDown::doInitialization(Module &M)
  {
		this->enabled = (TalkDownDisable.getNumOccurrences() == 0);
    return false;
  }

  const AnnotationSet &TalkDown::getAnnotationsForInst(Instruction *i) const
  {
    assert(0);
  }

  const AnnotationSet &TalkDown::getAnnotationsForInst(Instruction *i, Loop *l) const
  {
    Function *f = l->getHeader()->getParent();
    const FunctionTree* tree = findTreeForFunction( f );
    return tree->getAnnotationsForInst(i, l);
  }

  bool TalkDown::containsAnnotation(Loop *l) const
  {
    const FunctionTree* ft = findTreeForFunction( l->getHeader()->getParent() );
    return ft->loopContainsAnnotation( l );
    Instruction *i = l->getHeader()->getFirstNonPHI();
    AnnotationSet as = parseAnnotationsForInst(i);
    if (!as.size())
      return false;
    return true;
  }

  const FunctionTree* TalkDown::findTreeForFunction(Function *f) const
  {
//    errs() << "size of function_trees: " << function_trees.size() << '\n';
    for ( const auto &ft : function_trees )
    {
      auto *af = ft.getFunction();
      assert( af && "Could not find function tree in T/alkdown" );
      if ( f == af ) {
    //    errs() << "Found function tree\n";
        return &ft;
      }
    }
  //  errs() << "Didn't find a tree :(\n";
    return NULL;
  }

} // namespace llvm

char TalkDown::ID = 0;
static RegisterPass<TalkDown> X("TalkDown", "The TalkDown pass", false, true);

char llvm::SplitByAnnotation::ID = 0;
static RegisterPass<SplitByAnnotation> Y("talkdown-split-basic-blocks", "Split basic blocks before invoking -TalkDown pass");

// Register pass with Clang
static TalkDown * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new TalkDown());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new TalkDown());}});// ** for -O0
