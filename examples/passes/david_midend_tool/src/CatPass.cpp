#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include <algorithm>

#include "Noelle.hpp"

using namespace llvm::noelle ;

namespace {

  struct CAT : public ModulePass {
    static char ID; 

    CAT() : ModulePass(ID) {}

    bool doInitialization (Module &M) override {
      return false;
    }
/*
    bool checkISA (Inst &i) {
	    //dynamic cast instr to BinaryOperator, if it fails then
	    //dynamic cast to a LoadInst, if it fails then
	    //dynamic cast to a StoreInst, if it fails then next inst
	    //call .getType() on the inst to get its Type object then try casting that to an IntegerType then call getBitwidth on THAT
	    //loop through the operands: inst is a subclass of class user, which has all the operand-related functions
	    //
	    BinaryOperator* binop = dynamic_cast<BinaryOperator*>(i);
	    if(binop != nullptr) {
        Type t = binop.getType();
        IntegerType intT = dynamic_cast<IntegerType>(t);
        if(intT != nullptr) {
          if(intT.getBitwidth() == 8) {
            errs() << "getBW check succeeds\n";
            if(inst.getNumOperand() > 2) {
              errs() << "possible for binop to have 3+ operands, apparently\n";
              return false;
            }
            else {
              Use[2] uses = inst.getOperandList();
              bool hasImm = false;
              for(int i = 0; i < 2; i++) {
                Constant* testimm = dynamic_cast<Constant*>(&(uses[i]));
                if(testimm != nullptr) {
                  if(hasImm) {
                    return false;
                  }
                  else {
                    hasImm = true;
                    if(testimm->)
                  }
                }


                if(<Constant>(uses[i])) {

                }
              }
            }
          }
        }
	    }
      
      LoadInst* loadop = dynamic_cast<LoadInst*>(i);
      if(loadop != nullptr) {

      }

      StoreInst* storeop = dynamic_cast<StoreInst*>(i);
      if(storeop != nullptr) {

      }

		
    }*/

    raw_fd_ostream* coverData;
    std::error_code coverDataEC;

    bool runOnModule (Module &M) override {

      coverData = new raw_fd_ostream(StringRef("coverData.txt"), coverDataEC, sys::fs::CD_OpenExisting);
      
        double raw1Coverage = 0;
        double raw8Coverage = 0;
        double raw16Coverage = 0;
        double raw32Coverage = 0;
        double raw64Coverage = 0;
        long long int total1Insts = 0;
        long long int total8Insts = 0;
        long long int total16Insts = 0;
        long long int total32Insts = 0;
        long long int total64Insts = 0;


      /*
       * Fetch NOELLE
       */
      auto& noelle = getAnalysis<Noelle>();

      /*
       * Check if we have profiles.
       */
      auto hot = noelle.getProfiles();
      if (!hot->isAvailable()){
        return false;
      }
      errs() << "The profiler is available\n";
      errs() << "Which instructions are printed: only those with return type i8, of those, only those with a nonzero coverage as described by a double.\n";

      /*
       * Print the profiles for each function.
       */
      for (auto &F : M){
        if (F.empty()){
          continue ;
        }

        /*
         * Check if the function has been executed at all.
         */
        auto executed = hot->hasBeenExecuted(&F);
        if (!executed){
          continue ;
        }

        /*
         * The function has been executed.
         * Print its profile.
         */
        errs() << "Function: \"" << F.getName() << "\": Invocations = " << hot->getInvocations(&F) << "\n";
        errs() << "Function: \"" << F.getName() << "\": Static      = " << hot->getStaticInstructions(&F)  << "\n";
        errs() << "Function: \"" << F.getName() << "\": Self        = " << hot->getSelfInstructions(&F)  << "\n";
        errs() << "Function: \"" << F.getName() << "\": Total       = " << hot->getTotalInstructions(&F) << "\n";
        errs() << "Function: \"" << F.getName() << "\": Coverage    = " << (hot->getDynamicTotalInstructionCoverage(&F)) * 100 << "%\n";
        double f8coverage = 0;
      for (auto& bb : F){
        for (auto &inst : bb){
          Type* t = inst.getType();
          if(t->isIntegerTy()) {
            if(t->getIntegerBitWidth() == 8) {
              double instCoverage = ((double) hot->getTotalInstructions(&inst) * (double) hot->getDynamicTotalInstructionCoverage(bb.getParent()) / (double) hot->getTotalInstructions(bb.getParent())) * 100;
              if(instCoverage == 0) {
                continue;
              }
              raw8Coverage += instCoverage;
              total8Insts += hot->getTotalInstructions(&inst);
              f8coverage += instCoverage;
            }
            if(t->getIntegerBitWidth() == 1) {
              total1Insts += hot->getTotalInstructions(&inst);
              raw1Coverage += ((double) hot->getTotalInstructions(&inst) * (double) hot->getDynamicTotalInstructionCoverage(bb.getParent()) / (double) hot->getTotalInstructions(bb.getParent())) * 100;
            }
            if(t->getIntegerBitWidth() == 16) {
              total16Insts += hot->getTotalInstructions(&inst);
              raw16Coverage += ((double) hot->getTotalInstructions(&inst) * (double) hot->getDynamicTotalInstructionCoverage(bb.getParent()) / (double) hot->getTotalInstructions(bb.getParent())) * 100;
            }
            if(t->getIntegerBitWidth() == 32) {
              total32Insts += hot->getTotalInstructions(&inst);
              raw32Coverage += ((double) hot->getTotalInstructions(&inst) * (double) hot->getDynamicTotalInstructionCoverage(bb.getParent()) / (double) hot->getTotalInstructions(bb.getParent())) * 100;
            }
            if(t->getIntegerBitWidth() == 64) {
              total64Insts += hot->getTotalInstructions(&inst);
              raw64Coverage += ((double) hot->getTotalInstructions(&inst) * (double) hot->getDynamicTotalInstructionCoverage(bb.getParent()) / (double) hot->getTotalInstructions(bb.getParent())) * 100;
            }
          }
//          errs() << "Instruction: \"" << inst << "\": Total = " << hot->getTotalInstructions(&inst) << "\n";
	  
        }
      }
      errs() << "Function: \"" << F.getName() << "\": 8bit Coverage    = " << f8coverage << "%\n";

      //DDLOTT: this is pretty, uh, barbaric, but hey, lol
      for (auto& bb : F){
        for (auto &inst : bb){
          Type* t = inst.getType();
          if(t->isIntegerTy()) {
            if(t->getIntegerBitWidth() <= 8) {
              double instCoverage = ((double) hot->getTotalInstructions(&inst) * (double) hot->getDynamicTotalInstructionCoverage(bb.getParent()) / (double) hot->getTotalInstructions(bb.getParent())) * 100;
              if(instCoverage == 0) {
                continue;
              }
              errs() << F.getName() << " " << bb.getName() <<  " Instruction: \"" << inst << "\": Self|Total|Coverage  = " << hot->getSelfInstructions(&inst) << "|" << hot->getTotalInstructions(&inst) << "|" << instCoverage << "\n";//          errs() << "Instruction: \"" << inst << "\": Total = " << hot->getTotalInstructions(&inst) << "\n";
            }
          }
//          errs() << "Instruction: \"" << inst << "\": Total = " << hot->getTotalInstructions(&inst) << "\n";
	  
        }
      }
      }

      /*
       * Use the profiles.
       */
      //auto loops = noelle.getLoops();
      //for (auto loop : *loops){

        /*
         * Print the loop ID.
         */
        /*auto LS = loop->getLoopStructure();
        auto loopFunction = LS->getFunction();
        auto entryInst = LS->getEntryInstruction();
        errs() << "Loop:\n" ;
        errs() << "  " << loopFunction->getName() << "\n";
        errs() << "  " << *entryInst << "\n";
        errs() << "  Self  = " << hot->getSelfInstructions(LS) << "\n";
        errs() << "  Total = " << hot->getTotalInstructions(LS) << "\n";
	*/
        /*
         * Print loop statistics.
         */
        /*errs() << "    Number of invocations of the loop = " << hot->getInvocations(LS) << "\n";
        errs() << "    Average number of iterations per invocations = " << hot->getAverageLoopIterationsPerInvocation(LS) << "\n";
        errs() << "    Average number of total instructions per invocations = " << hot->getAverageTotalInstructionsPerInvocation(LS) << "\n";
        errs() << "    Coverage in terms of total instructions = " << (hot->getDynamicTotalInstructionCoverage(LS) * 100) << "%\n";
      }*/

      /*
       * Fetch the entry point.
       */
      auto fm = noelle.getFunctionsManager();
      auto mainF = fm->getEntryFunction();

      /*
       * Print the coverage per instruction of the loop.
       */
      double f8coverage = 0;
      for (auto& bb : *mainF){
        for (auto &inst : bb){
          Type* t = inst.getType();
          if(t->isIntegerTy()) {
            if(t->getIntegerBitWidth() == 8) {
              double instCoverage = ((double) hot->getTotalInstructions(&inst) * (double) hot->getDynamicTotalInstructionCoverage(bb.getParent()) / (double) hot->getTotalInstructions(bb.getParent())) * 100;
              if(instCoverage == 0) {
                continue;
              }
              raw8Coverage += instCoverage;
              f8coverage += instCoverage;
              total8Insts += hot->getTotalInstructions(&inst);
            }
            if(t->getIntegerBitWidth() == 1) {
              total1Insts += hot->getTotalInstructions(&inst);
              raw1Coverage += ((double) hot->getTotalInstructions(&inst) * (double) hot->getDynamicTotalInstructionCoverage(bb.getParent()) / (double) hot->getTotalInstructions(bb.getParent())) * 100;
            }
            if(t->getIntegerBitWidth() == 16) {
              total16Insts += hot->getTotalInstructions(&inst);
              raw16Coverage += ((double) hot->getTotalInstructions(&inst) * (double) hot->getDynamicTotalInstructionCoverage(bb.getParent()) / (double) hot->getTotalInstructions(bb.getParent())) * 100;
            }
            if(t->getIntegerBitWidth() == 32) {
              total32Insts += hot->getTotalInstructions(&inst);
              raw32Coverage += ((double) hot->getTotalInstructions(&inst) * (double) hot->getDynamicTotalInstructionCoverage(bb.getParent()) / (double) hot->getTotalInstructions(bb.getParent())) * 100;
            }
            if(t->getIntegerBitWidth() == 64) {
              total64Insts += hot->getTotalInstructions(&inst);
              raw64Coverage += ((double) hot->getTotalInstructions(&inst) * (double) hot->getDynamicTotalInstructionCoverage(bb.getParent()) / (double) hot->getTotalInstructions(bb.getParent())) * 100;
            }
          }
//          errs() << "Instruction: \"" << inst << "\": Total = " << hot->getTotalInstructions(&inst) << "\n";
	  
        }
      }
      errs() << "Function: \"" << mainF->getName() << "\": 8bit Coverage    = " << f8coverage << "%\n";      
      for (auto& bb : *mainF){
        for (auto &inst : bb){
          Type* t = inst.getType();
          if(t->isIntegerTy()) {
            if(t->getIntegerBitWidth() <= 8) {
              double instCoverage = ((double) hot->getTotalInstructions(&inst) * (double) hot->getDynamicTotalInstructionCoverage(bb.getParent()) / (double) hot->getTotalInstructions(bb.getParent())) * 100;
              if(instCoverage == 0) {
                continue;
              }
              errs() << mainF->getName() << " " << bb.getName() << " Instruction: \"" << inst << "\": Self|Total|Coverage  = " << hot->getSelfInstructions(&inst) << "|" << hot->getTotalInstructions(&inst) << "|" << instCoverage << "\n";
            }
          }
//          errs() << "Instruction: \"" << inst << "\": Total = " << hot->getTotalInstructions(&inst) << "\n";
        }
      }

      errs() << "Total coverage of all 8bit-resulting instructions: " << raw8Coverage << "\n";
      errs() << "plotBW " << total64Insts << " " << total32Insts << " " << total16Insts << " " << total8Insts << " " << total1Insts << "\n";
      *coverData << mainF->getName() << ": " << raw1Coverage << " " << raw8Coverage << " " << raw16Coverage << " " << raw32Coverage << " " << raw64Coverage << "\n";
      coverData->flush();

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
