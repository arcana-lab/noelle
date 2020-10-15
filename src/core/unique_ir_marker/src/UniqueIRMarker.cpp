
#include <set>
#include <UniqueIRMarker.hpp>
#include <UniqueIRVerifier.hpp>

#include "UniqueIRMarker.hpp"

using std::addressof;
using namespace llvm;

namespace llvm::noelle {

  void UniqueIRMarker::visitModule(Module &M) {
    auto metaNode = M.getOrInsertNamedMetadata(UniqueIRConstants::VIAModule);

    switch (Mode) {
      case MarkerMode::Renumber:
        assert(metaNode->getNumOperands() == 1);
        break;
      case MarkerMode::Instrument:
        assert (metaNode->getNumOperands() == 0 && "Must not already be instrumented ");
        break;
      case MarkerMode::Reinstrument:
        assert(metaNode->getNumOperands() == 0 || metaNode->getNumOperands() == 1);
        break;
    }

    auto moduleUID = uniqueModuleCounter();
    auto *meta = buildNode(M.getContext(), moduleUID);
    metaNode->clearOperands();
    metaNode->addOperand(meta);
  }


  void UniqueIRMarker::visitFunction(Function &F) {
    auto meta = F.getMetadata(UniqueIRConstants::VIAFunction);
    switch (Mode) {
      case MarkerMode::Instrument:
        assert(meta == nullptr && "Should not have a Unique IR Number.");
        break;
      case MarkerMode::Reinstrument:
        break;
      case MarkerMode::Renumber:
        if (meta == nullptr) return;
        break;
    }

    LLVMContext &Context = F.getContext();
    auto *countMeta = buildNode(Context, FunctionCounter++);
    F.setMetadata(UniqueIRConstants::VIAFunction, countMeta);

    if (F.empty()) return;

    auto &LoopInfo = MP.getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();

    for (auto &Loop : LoopInfo.getLoopsInPreorder()) {
      switch (Mode) {
        case MarkerMode::Instrument:break;
        case MarkerMode::Reinstrument:break;
        case MarkerMode::Renumber:
          if (Loop->getLoopID() == nullptr) continue;
          break;
      }
      SmallVector<Metadata *, 2> MDs;

      MDs.push_back(ConstantAsMetadata::get(ConstantInt::get(Context, llvm::APInt(IDSize, 0, false))));

      SmallVector<Metadata *, 1> ID;
      {
        auto LoopVIAName = MDString::get(Context, UniqueIRConstants::VIALoop);
        ID.push_back(LoopVIAName);
        auto LoopIDMeta = ConstantAsMetadata::get(ConstantInt::get(Context, llvm::APInt(IDSize, LoopCounter++, false)));
        ID.push_back(LoopIDMeta);
      }
      MDs.push_back(MDNode::get(Context, ID));
      auto *node = MDNode::get(Context, MDs);
      node->replaceOperandWith(0, node);
      Loop->setLoopID(node);
      node->print(errs());
    }
  }

  void UniqueIRMarker::visitBasicBlock(BasicBlock &BB) {
    if( BB.empty() ) return;
    switch (Mode) {
      case MarkerMode::Instrument:
        assert (BB.front().getMetadata(UniqueIRConstants::VIABasicBlock) == nullptr && "Must not already have an U IR Num.");
        break;
      case MarkerMode::Reinstrument:
        break;
      case MarkerMode::Renumber:
        if(BB.front().getMetadata(UniqueIRConstants::VIABasicBlock) == nullptr)
          return;
        break;
    }
    auto *countMeta = buildNode(BB.getContext(), BasicBlockCounter++);
    BB.front().setMetadata(UniqueIRConstants::VIABasicBlock, countMeta);
  }

  void UniqueIRMarker::visitInstruction(Instruction &I) {
    switch (Mode) {
      case MarkerMode::Instrument:
        assert (I.getMetadata(UniqueIRConstants::VIAInstruction) == nullptr && "Must not already have an U IR Num.");
        break;
      case MarkerMode::Reinstrument:
        break;
      case MarkerMode::Renumber:
        if(I.getMetadata(UniqueIRConstants::VIAInstruction) == nullptr)
          return;
        break;
    }
    auto *countMeta = buildNode(I.getContext(), uniqueInstructionCounter());
    I.setMetadata(UniqueIRConstants::VIAInstruction, countMeta);
  }

  MDNode *UniqueIRMarker::buildNode(LLVMContext& C, IDType value) {
    return MDNode::get(C, ConstantAsMetadata::get(ConstantInt::get(C, llvm::APInt(IDSize, value, false))));
  }

  UniqueIRMarker::UniqueIRMarker(ModulePass& MP, MarkerMode mode) :
    MP(MP), Mode(mode),
    InstructionCounter(0), FunctionCounter(0), BasicBlockCounter(0), LoopCounter(0), ModuleCounter(0) {}

  uint64_t UniqueIRMarker::uniqueInstructionCounter() {
    // uint64_t max is reserved for null.
    assert(InstructionCounter <= (std::numeric_limits<uint64_t>::max() - 1) && "InstructionCounter has overrun" );
    return InstructionCounter++;
  }

  IDType UniqueIRMarker::uniqueModuleCounter() {
    assert(ModuleCounter <= (std::numeric_limits<uint64_t>::max() - 1) && "ModuleCounter has overrun" );
    return ModuleCounter++;
  }

}