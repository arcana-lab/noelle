#include "arcana/noelle/core/BasicAAAnalysis.hpp"

namespace arcana::noelle {

class AdaptedBasicAA {
public:
  static MemoryEffects getMemoryEffects(const Function *F) {
    switch (F->getIntrinsicID()) {
      case Intrinsic::experimental_guard:
      case Intrinsic::experimental_deoptimize:
        // These intrinsics can read arbitrary memory, and additionally modref
        // inaccessible memory to model control dependence.
        return MemoryEffects::readOnly()
               | MemoryEffects::inaccessibleMemOnly(ModRefInfo::ModRef);
    }
    return F->getMemoryEffects();
  }

  static MemoryEffects getMemoryEffects(const CallBase *call) {
    MemoryEffects min = call->getAttributes().getMemoryEffects();

    if (const Function *F = dyn_cast<Function>(call->getCalledOperand())) {
      MemoryEffects funcME = getMemoryEffects(F);
      // Operand bundles on the call may also read or write memory, in addition
      // to the behavior of the called function
      if (call->hasReadingOperandBundles()) {
        funcME |= MemoryEffects::readOnly();
      }
      if (call->hasClobberingOperandBundles()) {
        funcME |= MemoryEffects::writeOnly();
      }
      min &= funcME;
    }

    return min;
  }

  static ModRefInfo getArgModRefInfo(const CallBase *call, unsigned argIdx) {
    if (call->paramHasAttr(argIdx, Attribute::WriteOnly)) {
      return ModRefInfo::Mod;
    }
    if (call->paramHasAttr(argIdx, Attribute::ReadOnly)) {
      return ModRefInfo::Ref;
    }
    if (call->paramHasAttr(argIdx, Attribute::ReadNone)) {
      return ModRefInfo::NoModRef;
    }
    return ModRefInfo::ModRef;
  }
};

class AdaptedAA {
public:
  static ModRefInfo getModRefInfo(const CallBase *call,
                                  const MemoryLocation &loc) {
    ModRefInfo result = ModRefInfo::ModRef;

    auto ME = AdaptedBasicAA::getMemoryEffects(call).getWithoutLoc(
        IRMemLocation::InaccessibleMem);
    if (ME.doesNotAccessMemory()) {
      return ModRefInfo::NoModRef;
    }

    ModRefInfo argMR = ME.getModRef(IRMemLocation::ArgMem);
    ModRefInfo otherMR = ME.getWithoutLoc(IRMemLocation::ArgMem).getModRef();
    if ((argMR | otherMR) != otherMR) {
      // Refine result for argument memory (argMR).
      // Only do this if argMR is not a subset of otherMR
      ModRefInfo allArgsMask = ModRefInfo::NoModRef;
      for (const auto &i : llvm::enumerate(call->args())) {
        const Value *arg = i.value();
        if (!arg->getType()->isPointerTy()) {
          continue;
        }
        unsigned argIdx = i.index();
        // NOTE: we can further improve this by fetching the MemoryLocation
        // pointed by arg, and test its aliasing with loc.  The current arg
        // can be skipped if there is no alias.
        allArgsMask |= AdaptedBasicAA::getArgModRefInfo(call, argIdx);
      }
      argMR &= allArgsMask;
    }

    result &= argMR | otherMR;
    // NOTE: skip getModRefInfoMask refinement
    return result;
  }

  static ModRefInfo getModRefInfo(const CallBase *call1,
                                  const CallBase *call2) {
    ModRefInfo result = ModRefInfo::ModRef;

    // No dependence between the calls if call1 doesn't access memory.
    auto call1ME = AdaptedBasicAA::getMemoryEffects(call1);
    if (call1ME.doesNotAccessMemory()) {
      return ModRefInfo::NoModRef;
    }

    // No dependence between the calls if call2 doesn't access memory.
    auto call2ME = AdaptedBasicAA::getMemoryEffects(call2);
    if (call1ME.doesNotAccessMemory()) {
      return ModRefInfo::NoModRef;
    }

    // No dependence between the calls if both calls only read memory.
    if (call1ME.onlyReadsMemory() && call2ME.onlyReadsMemory()) {
      return ModRefInfo::NoModRef;
    }

    // Call1 can't mod call2 if it only reads memory.
    // Call1 can't ref call2 if it only writes memory.
    if (call1ME.onlyReadsMemory()) {
      result &= ModRefInfo::Ref;
    } else if (call1ME.onlyWritesMemory()) {
      result &= ModRefInfo::Mod;
    }

    // Improve results if call2 only access memory via its arguments
    if (call2ME.onlyAccessesArgPointees()) {
      if (!call2ME.doesAccessArgPointees()) {
        return ModRefInfo::NoModRef;
      }
      ModRefInfo R = ModRefInfo::NoModRef;
      for (size_t i = 0; i < call2->arg_size(); ++i) {
        const Value *arg = call2->getArgOperand(i);
        if (!arg->getType()->isPointerTy()) {
          continue;
        }

        // Fetch the memory location referred by call2's current argument.
        // If call2 writes it, dependence exists when call1 reads or writes.
        // If call2 reads it, dependence exists only when call1 writes.
        MemoryLocation call2ArgLoc =
            MemoryLocation::getForArgument(call2, i, nullptr);
        ModRefInfo argModRefC2 = AdaptedBasicAA::getArgModRefInfo(call2, i);
        ModRefInfo argMask = ModRefInfo::NoModRef;
        if (isModSet(argModRefC2)) {
          argMask = ModRefInfo::ModRef;
        } else if (isRefSet(argModRefC2)) {
          argMask = ModRefInfo::Mod;
        }
        argMask &= getModRefInfo(call1, call2ArgLoc);

        R = (R | argMask) & result;
        if (R == result) {
          // Early exit because we won't do better.
          break;
        }
      }
      return R;
    }

    // Improve results if call1 only access memory via its arguments
    if (call1ME.onlyAccessesArgPointees()) {
      if (!call1ME.doesAccessArgPointees()) {
        return ModRefInfo::NoModRef;
      }
      ModRefInfo R = ModRefInfo::NoModRef;
      for (size_t i = 0; i < call1->arg_size(); ++i) {
        const Value *arg = call1->getArgOperand(i);
        if (!arg->getType()->isPointerTy()) {
          continue;
        }
        // Fetch the memory location referred by call1's current argument.
        // If call1 writes it, dependence exists when call2 reads or writes.
        // If call1 reads it, dependence exists only when call2 writes.
        MemoryLocation call1ArgLoc =
            MemoryLocation::getForArgument(call1, i, nullptr);
        ModRefInfo argModRefC1 = AdaptedBasicAA::getArgModRefInfo(call1, i);
        ModRefInfo modRefC2 = getModRefInfo(call2, call1ArgLoc);
        if ((isModSet(argModRefC1) && isModOrRefSet(modRefC2))
            || (isRefSet(argModRefC1) && isModSet(modRefC2))) {
          R = (R | argModRefC1) & result;
        }
        if (R == result) {
          // Early exit because we won't do better.
          break;
        }
      }
      return R;
    }

    // No argument-related improvements
    return result;
  }
};

/// For debugging
void printMsg(DataDependenceType t,
              Instruction *fromInst,
              Instruction *toInst) {
  errs() << ">>> Eliminated a dependence:\n";
  llvm::StringRef s;
  switch (t) {
    case DG_DATA_RAW:
      s = "RAW";
      break;
    case DG_DATA_WAR:
      s = "WAR";
      break;
    case DG_DATA_WAW:
      s = "WAW";
      break;
  }
  errs() << "Type: " << s << "\n";
  errs() << "FromInst: " << *fromInst << "\n";
  errs() << "ToInst:   " << *toInst << "\n";
  errs() << "\n";
}

BasicAAAnalysis::BasicAAAnalysis() : DependenceAnalysis("BasicAAAnalysis") {}

MemoryDataDependenceStrength BasicAAAnalysis::
    isThereThisMemoryDataDependenceType(DataDependenceType t,
                                        Instruction *fromInst,
                                        Instruction *toInst) {
  // We only care about cases related to function attributes,
  // so we expect at least 1 input instruction to be a call.
  if (!isa<CallBase>(fromInst) && !isa<CallBase>(toInst)) {
    return MAY_EXIST;
  }

  //
  // For each possible DataDependenceType input, only 3 combinations
  // of (fromInst,toInst) may lead to dependence.
  //
  //       |    (I,call)     |     (call,I)    |  (call1,call2)
  // ------+-----------------+-----------------+------------------
  //   RAW | store,call(ref) | call(mod),load  | call,call(ref)
  //   WAR | load ,call(mod) | call(ref),store | call(ref),call
  //   WAW | store,call(mod) | call(mod),store | call,call(mod)
  //
  switch (t) {
    case DG_DATA_RAW:
      // Check whether toInst reads the memory written by fromInst
      if (isa<StoreInst>(fromInst) && isa<CallBase>(toInst)) {
        CallBase *call = cast<CallBase>(toInst);
        MemoryLocation loc = MemoryLocation::get(fromInst);
        ModRefInfo MR = AdaptedAA::getModRefInfo(call, loc);
        if (isRefSet(MR)) {
          return MAY_EXIST;
        }
      } else if (isa<CallBase>(fromInst) && isa<LoadInst>(toInst)) {
        CallBase *call = cast<CallBase>(fromInst);
        MemoryLocation loc = MemoryLocation::get(toInst);
        ModRefInfo MR = AdaptedAA::getModRefInfo(call, loc);
        if (isModSet(MR)) {
          return MAY_EXIST;
        }
      } else if (isa<CallBase>(fromInst) && isa<CallBase>(toInst)) {
        CallBase *call1 = cast<CallBase>(fromInst);
        CallBase *call2 = cast<CallBase>(toInst);
        ModRefInfo MR = AdaptedAA::getModRefInfo(call2, call1);
        if (isRefSet(MR)) {
          return MAY_EXIST;
        }
      }
      break;

    case DG_DATA_WAR:
      // Check whether toInst writes the memory read by fromInst
      if (isa<LoadInst>(fromInst) && isa<CallBase>(toInst)) {
        CallBase *call = cast<CallBase>(toInst);
        MemoryLocation loc = MemoryLocation::get(fromInst);
        ModRefInfo MR = AdaptedAA::getModRefInfo(call, loc);
        if (isModSet(MR)) {
          return MAY_EXIST;
        }
      } else if (isa<CallBase>(fromInst) && isa<StoreInst>(toInst)) {
        CallBase *call = cast<CallBase>(fromInst);
        MemoryLocation loc = MemoryLocation::get(toInst);
        ModRefInfo MR = AdaptedAA::getModRefInfo(call, loc);
        if (isRefSet(MR)) {
          return MAY_EXIST;
        }
      } else if (isa<CallBase>(fromInst) && isa<CallBase>(toInst)) {
        CallBase *call1 = cast<CallBase>(fromInst);
        CallBase *call2 = cast<CallBase>(toInst);
        ModRefInfo MR = AdaptedAA::getModRefInfo(call1, call2);
        if (isRefSet(MR)) {
          return MAY_EXIST;
        }
      }
      break;

    case DG_DATA_WAW:
      // Check whether toInst writes the memory written by fromInst
      if (isa<StoreInst>(fromInst) && isa<CallBase>(toInst)) {
        CallBase *call = cast<CallBase>(toInst);
        MemoryLocation loc = MemoryLocation::get(fromInst);
        ModRefInfo MR = AdaptedAA::getModRefInfo(call, loc);
        if (isModSet(MR)) {
          return MAY_EXIST;
        }
      } else if (isa<CallBase>(fromInst) && isa<StoreInst>(toInst)) {
        CallBase *call = cast<CallBase>(fromInst);
        MemoryLocation loc = MemoryLocation::get(toInst);
        ModRefInfo MR = AdaptedAA::getModRefInfo(call, loc);
        if (isModSet(MR)) {
          return MAY_EXIST;
        }
      } else if (isa<CallBase>(fromInst) && isa<CallBase>(toInst)) {
        CallBase *call1 = cast<CallBase>(fromInst);
        CallBase *call2 = cast<CallBase>(toInst);
        ModRefInfo MR = AdaptedAA::getModRefInfo(call2, call1);
        if (isModSet(MR)) {
          return MAY_EXIST;
        }
      }
      break;
  }

#if 0
  // Debug
  printMsg(t, fromInst, toInst);
#endif

  return CANNOT_EXIST;
}

MemoryDataDependenceStrength BasicAAAnalysis::
    isThereThisMemoryDataDependenceType(DataDependenceType t,
                                        Instruction *fromInst,
                                        Instruction *toInst,
                                        Function &function) {
  return isThereThisMemoryDataDependenceType(t, fromInst, toInst);
}

MemoryDataDependenceStrength BasicAAAnalysis::
    isThereThisMemoryDataDependenceType(DataDependenceType t,
                                        Instruction *fromInst,
                                        Instruction *toInst,
                                        LoopStructure &loop) {
  return isThereThisMemoryDataDependenceType(t, fromInst, toInst);
}

} // namespace arcana::noelle
