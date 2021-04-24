#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Analysis/TargetLibraryInfo.h"

#include "Noelle.hpp"

using namespace llvm::noelle ;

namespace {

  struct CAT : public ModulePass {
    static char ID; 
    Module *currentModule;

    CAT() : ModulePass(ID) {}

    bool doInitialization (Module &M) override {
      currentModule = &M;
      return false;
    }

    bool runOnModule (Module &M) override {

      /*
       * Fetch NOELLE
       */
      auto& noelle = getAnalysis<Noelle>();

      /*
       * Use NOELLE
       */
      // auto insts = noelle.numberOfProgramInstructions();
      // errs() << "The program has " << insts << " instructions\n";

      LLVMContext &context = M.getContext();

      PointerType* pointerType = PointerType::get(IntegerType::get(context, 8), 0);

      FunctionCallee mallocFunc = M.getOrInsertFunction(
        "jemalloc",                     // name of function
        pointerType,                    // return type
        Type::getInt64Ty(context)       // first parameter type
      );
      Value* malloc = mallocFunc.getCallee();

      FunctionCallee freeFunc = M.getOrInsertFunction(
        "jefree",                     // name of function
        Type::getVoidTy(context),     // return type
        pointerType                   // first parameter type
      );
      Value* free = freeFunc.getCallee();

      vector<Instruction*> instructionsToReplace;

      errs() << "going through insts: \n";

      for (auto &function : M) {
        for (BasicBlock &bb : function) {
          for (Instruction &inst : bb) {

            if (isa<CallInst>(&inst)) {
              const CallInst* callInst = llvm::dyn_cast<CallInst>(&inst);
              auto functionName = callInst->getCalledFunction()->getName();
              // errs() << functionName << "\n";

              if (functionName != "malloc" && functionName != "free") {
                continue;
              }

              instructionsToReplace.push_back(&inst);
            }
          }
        }
      }

      for (auto inst : instructionsToReplace) {
        const CallInst* callInst = llvm::dyn_cast<CallInst>(inst);
        auto functionName = callInst->getCalledFunction()->getName();

        std::vector<Value*> args;
        for (auto i = 0; i < callInst->getNumArgOperands(); i++) {
          auto argAddress = llvm::cast<Value>(callInst->getArgOperand(i));
          args.push_back(argAddress);
        }

        BasicBlock::iterator ii(inst);

        // errs() << "Collected args and got bb iterator\n";

        CallInst* replacementInst;
        if (functionName == "malloc") {
          replacementInst = CallInst::Create(malloc, ArrayRef<Value*>(args), "jemalloc");
        }

        if (functionName == "free") {
          // errs() << "replacement for free\n";
          replacementInst = CallInst::Create(free, ArrayRef<Value*>(args));
        }

        // errs() << "Created replacement inst\n";

        ReplaceInstWithInst(inst->getParent()->getInstList(), ii, replacementInst);
      }

      return false;
    }

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<Noelle>();
    }
  };

}

// Next there is code to register your pass to "opt"
char CAT::ID = 0;
static RegisterPass<CAT> X("CAT", "Simple user of the Noelle framework");

// Next there is code to register your pass to "clang"
static CAT * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new CAT());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new CAT()); }}); // ** for -O0
