
#include "UniqueIRVerifier.hpp"

using std::addressof;
using namespace llvm;

namespace llvm::noelle {

  UniqueIRVerifier::UniqueIRVerifier(ModulePass& MP) :
      MP(MP) {}

  void UniqueIRVerifier::visitModule(Module &M) {
    auto metaNode = M.getOrInsertNamedMetadata(UniqueIRConstants::VIAModule);
    assert (metaNode->getNumOperands() == 1 && "A Module which has been marked must have a VIAModule marker");
    auto setModuleID = UniqueIRMarkerReader::getModuleID(addressof(M));
    assert(setModuleID && setModuleID.value() >= 0);
  }


  void UniqueIRVerifier::visitFunction(Function &F) {
    checkFunction(F);
  }


  void UniqueIRVerifier::visitInstruction(Instruction &I) {
    checkInstruction(I);
  }

  void UniqueIRVerifier::checkFunction(Function &F) {
    if (F.empty()) return;
    if(UniqueIRMarkerReader::getFunctionID(addressof(F))) {
      // Check that llvm.loop metadata is still correct.
      auto &LoopInfo = MP.getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
      auto LoopIDs = std::set<IDType>();

      for (auto &Loop : LoopInfo.getLoopsInPreorder()) {
        auto LoopIDOpt = UniqueIRMarkerReader::getLoopID(Loop);
        if(!LoopIDOpt) {
          F.getParent()->print(errs(), nullptr);
        }
        assert (LoopIDOpt);
        LoopIDs.insert(UniqueIRMarkerReader::getLoopID(Loop).value());
      }
    }
  }
  void UniqueIRVerifier::checkInstruction(Instruction &I) {
    auto *LoopMeta = I.getMetadata("llvm.loop");
    auto InstructionLoopIDOpt = UniqueIRMarkerReader::getIDFromLoopMeta(LoopMeta);
    if (LoopMeta && InstructionLoopIDOpt) {
      // Make sure that the loop id is the last instruction in the BB.
      assert (addressof(I.getParent()->back()) == addressof(I));
      auto &LoopInfo = MP.getAnalysis<LoopInfoWrapperPass>(*I.getFunction()).getLoopInfo();
      auto L = LoopInfo.getLoopFor(I.getParent());
      auto LILoopIDOpt = UniqueIRMarkerReader::getLoopID(L);
      assert(LILoopIDOpt == InstructionLoopIDOpt && "Loop ID from LoopInfo must match Loop ID from the IR");

      LoopIDsFromPreviousMarkerPass.insert(InstructionLoopIDOpt.value());
    }

  }
  bool UniqueIRVerifier::verifyLoops() {
    // check that the sets are equal
    return LoopIDsFromPreviousMarkerPass.size() == LoopIDsFromLoopInfo.size()
          && std::equal(LoopIDsFromPreviousMarkerPass.begin(), LoopIDsFromPreviousMarkerPass.end(),
                        LoopIDsFromLoopInfo.begin());

  }

}