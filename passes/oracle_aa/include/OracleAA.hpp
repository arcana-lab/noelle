#ifndef CAT_ORACLEAA_H
#define CAT_ORACLEAA_H

#include <llvm/Pass.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Analysis/AliasAnalysis.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/IR/LegacyPassManager.h>

#include <memory>
#include <functional>
#include <optional>

#include "OracleAliasResult.hpp"

namespace llvm {

class OracleDDGAAResult : public AAResultBase<OracleDDGAAResult> {
  friend AAResultBase<OracleDDGAAResult>;

  std::unique_ptr<OracleAliasResults> Res;

  ModulePass &MP;

 public:
  static char ID;
  OracleDDGAAResult(ModulePass &AA);

  OracleAliasResults *getAliasResults() { Res.get(); }

  bool invalidate(Function &, const PreservedAnalyses &, FunctionAnalysisManager::Invalidator &) {
    return false;
  }

  AliasResult  query(const MemoryLocation &, const MemoryLocation &);

  AliasResult  alias(const MemoryLocation &, const MemoryLocation &);

  ModRefInfo  getModRefInfo(const ImmutableCallSite CS, const MemoryLocation &Loc);

  ModRefInfo  getModRefInfo(ImmutableCallSite CS1, ImmutableCallSite CS2);
  Loop *getTopMostLoop(Loop *La) const;
 private:
  template <typename V>
  V SearchResult(const Instruction *, const Instruction *,
                 std::function<std::optional<V>(OracleAliasFunctionResults::Dependencies, OracleAliasFunctionResults::DependencyType)> f, V none, std::function<V(void)> NoMatch);
};



  class OracleAAWrapperPass : public ModulePass {
    std::unique_ptr<OracleDDGAAResult> Result;

   public:
    static char ID;
    OracleAAWrapperPass() : ModulePass(ID) {
      errs() << "made pass AA wrapper\n";
    }

    bool doInitialization(Module&) override;

    void *getAdjustedAnalysisPointer(AnalysisID ID) override {
      if (ID == &OracleAAWrapperPass::ID) {
        return this;
      }
      return this;
    }

    bool runOnModule(Module &) override;

    OracleDDGAAResult &getResult();

    const OracleDDGAAResult &getResult() const;

    void getAnalysisUsage( AnalysisUsage& ) const override;
  };

ModulePass* createOracleDDGAAWrapperPass();

//void registerOracleAAPasses(llvm::legacy::PassManagerBase &PM) {
//  errs() << "called register external oracle aa pass\n";
//  auto pass = llvm::createExternalAAWrapperPass([](Pass &P, Function &F, AAResults &AAR) {
//    errs() << "called external oracle aa pass\n";
//    auto &wrapper = P.getAnalysis<OracleAAWrapperPass>(F);
//    AAR.addAAResult(wrapper.getResult());
//  });
//  PM.add(pass);
//}


//class OracleAAMix : AnalysisInfoMixin<OracleAAMix> {
//  friend AnalysisInfoMixin<OracleAAMix>;
//};

};


#endif //CAT_ORACLEAA_H
