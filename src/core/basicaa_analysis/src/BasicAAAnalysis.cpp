#include "arcana/noelle/core/BasicAAAnalysis.hpp"

namespace arcana::noelle {

MiniBasicAA::MiniBasicAA() {}

FunctionModRefBehavior MiniBasicAA::getModRefBehavior(const Function *F) {
  if (F->doesNotAccessMemory()) {
    return FMRB_DoesNotAccessMemory;
  }

  FunctionModRefBehavior min = FMRB_UnknownModRefBehavior;

  if (F->onlyReadsMemory()) {
    min = FMRB_OnlyReadsMemory;
  } else if (F->onlyWritesMemory()) {
    min = FMRB_OnlyWritesMemory;
  }

  if (F->onlyAccessesArgMemory()) {
    min = FunctionModRefBehavior(min & FMRB_OnlyAccessesArgumentPointees);
  } else if (F->onlyAccessesInaccessibleMemory()) {
    min = FunctionModRefBehavior(min & FMRB_OnlyAccessesInaccessibleMem);
  } else if (F->onlyAccessesInaccessibleMemOrArgMem()) {
    min = FunctionModRefBehavior(min & FMRB_OnlyAccessesInaccessibleOrArgMem);
  }

  return min;
}

FunctionModRefBehavior MiniBasicAA::getModRefBehavior(const CallBase *call) {
  if (call->doesNotAccessMemory()) {
    return FMRB_DoesNotAccessMemory;
  }

  FunctionModRefBehavior min = FMRB_UnknownModRefBehavior;

  if (call->onlyReadsMemory()) {
    min = FMRB_OnlyReadsMemory;
  } else if (call->onlyWritesMemory()) {
    min = FMRB_OnlyWritesMemory;
  }

  if (call->onlyAccessesArgMemory()) {
    min = FunctionModRefBehavior(min & FMRB_OnlyAccessesArgumentPointees);
  } else if (call->onlyAccessesInaccessibleMemory()) {
    min = FunctionModRefBehavior(min & FMRB_OnlyAccessesInaccessibleMem);
  } else if (call->onlyAccessesInaccessibleMemOrArgMem()) {
    min = FunctionModRefBehavior(min & FMRB_OnlyAccessesInaccessibleOrArgMem);
  }

  if (!call->hasOperandBundles()) {
    if (const Function *F = call->getCalledFunction()) {
      min = FunctionModRefBehavior(min & getModRefBehavior(F));
    }
  }

  return min;
}

ModRefInfo MiniBasicAA::getArgModRefInfo(const CallBase *call,
                                         unsigned argIdx) {
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

ModRefInfo BasicAAAnalysis::getModRefInfo(const CallBase *call,
                                          const MemoryLocation &loc) {
  ModRefInfo result = ModRefInfo::ModRef;

  // BasicAA derives FMRB from function attributes. Use them to
  // improve our solution.
  FunctionModRefBehavior MRB = miniBasicAA.getModRefBehavior(call);
  if (AAResults::onlyAccessesInaccessibleMem(MRB)) {
    return ModRefInfo::NoModRef;
  }
  if (AAResults::onlyReadsMemory(MRB)) {
    result = clearMod(result);
  } else if (AAResults::onlyWritesMemory(MRB)) {
    result = clearRef(result);
  }

  if (AAResults::onlyAccessesArgPointees(MRB)
      || AAResults::onlyAccessesInaccessibleOrArgMem(MRB)) {
    // If the call only accesses memory through its pointer arguments,
    // then we may be able to improve results with argument attributes.
    //
    // `allArgsMask` holds the union of all ModRefInfo collected from
    // the call's pointer arguments.
    ModRefInfo allArgsMask = ModRefInfo::NoModRef;
    if (AAResults::doesAccessArgPointees(MRB)) {
      for (size_t i = 0; i < call->arg_size(); ++i) {
        if (!call->getArgOperand(i)->getType()->isPointerTy()) {
          continue;
        }
        ModRefInfo argMask = miniBasicAA.getArgModRefInfo(call, i);
        allArgsMask = unionModRef(allArgsMask, argMask);
      }
    }
    // Improve the result
    result = intersectModRef(result, allArgsMask);
  }

  return result;
}

ModRefInfo BasicAAAnalysis::getModRefInfo(const CallBase *call1,
                                          const CallBase *call2) {
  ModRefInfo result = ModRefInfo::ModRef;

  // No dependence between the calls if call1 doesn't access memory.
  FunctionModRefBehavior call1B = miniBasicAA.getModRefBehavior(call1);
  if (call1B == FMRB_DoesNotAccessMemory) {
    return ModRefInfo::NoModRef;
  }

  // No dependence between the calls if call2 doesn't access memory.
  FunctionModRefBehavior call2B = miniBasicAA.getModRefBehavior(call2);
  if (call2B == FMRB_DoesNotAccessMemory) {
    return ModRefInfo::NoModRef;
  }

  // No dependence between the calls if both calls only read memory.
  if (AAResults::onlyReadsMemory(call1B)
      && AAResults::onlyReadsMemory(call2B)) {
    return ModRefInfo::NoModRef;
  }

  // Call1 can't mod call2 if it only reads memory.
  // Call1 can't ref call2 if it only writes memory.
  if (AAResults::onlyReadsMemory(call1B)) {
    result = clearMod(result);
  } else if (AAResults::onlyWritesMemory(call1B)) {
    result = clearRef(result);
  }

  if (AAResults::onlyAccessesArgPointees(call2B)) {
    if (!AAResults::doesAccessArgPointees(call2B)) {
      return ModRefInfo::NoModRef;
    }
    // If call2 only accesses memory through its pointer arguments,
    // we may be able to improve the result based on what call1
    // might do to the memory locations referred by call2's args.
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
      ModRefInfo argModRefC2 = miniBasicAA.getArgModRefInfo(call2, i);
      ModRefInfo argMask = ModRefInfo::NoModRef;
      if (isModSet(argModRefC2)) {
        argMask = ModRefInfo::ModRef;
      } else if (isRefSet(argModRefC2)) {
        argMask = ModRefInfo::Mod;
      }

      ModRefInfo modRefC1 = getModRefInfo(call1, call2ArgLoc);
      argMask = intersectModRef(argMask, modRefC1);
      R = intersectModRef(unionModRef(R, argMask), result);

      if (R == result) {
        // Early exit because we won't do better.
        break;
      }
    }
    return R;
  }

  if (AAResults::onlyAccessesArgPointees(call1B)) {
    if (!AAResults::doesAccessArgPointees(call1B)) {
      return ModRefInfo::NoModRef;
    }
    // Similar to the previous case, we can use arguments to improve
    // results if call1 only accesses memory via its pointer args.
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
      ModRefInfo argModRefC1 = miniBasicAA.getArgModRefInfo(call1, i);
      ModRefInfo modRefC2 = getModRefInfo(call2, call1ArgLoc);
      if ((isModSet(argModRefC1) && isModOrRefSet(modRefC2))
          || (isRefSet(argModRefC1) && isModSet(modRefC2))) {
        R = intersectModRef(unionModRef(R, argModRefC1), result);
      }

      if (R == result) {
        // Early exit because we won't do better.
        break;
      }
    }
    return R;
  }

  return result;
}

BasicAAAnalysis::BasicAAAnalysis() : DependenceAnalysis("BasicAAAnalysis") {}

void printMsg(DataDependenceType t,
              Instruction *fromInst,
              Instruction *toInst) {
#if 1
  errs() << ">>> Eliminated a dependence:\n";
  std::string s;
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
#endif
}

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
        ModRefInfo MR = getModRefInfo(call, loc);
        if (isRefSet(MR)) {
          return isMustSet(MR) ? MUST_EXIST : MAY_EXIST;
        }
      } else if (isa<CallBase>(fromInst) && isa<LoadInst>(toInst)) {
        CallBase *call = cast<CallBase>(fromInst);
        MemoryLocation loc = MemoryLocation::get(toInst);
        ModRefInfo MR = getModRefInfo(call, loc);
        if (isModSet(MR)) {
          return isMustSet(MR) ? MUST_EXIST : MAY_EXIST;
        }
      } else if (isa<CallBase>(fromInst) && isa<CallBase>(toInst)) {
        CallBase *call1 = cast<CallBase>(fromInst);
        CallBase *call2 = cast<CallBase>(toInst);
        ModRefInfo MR = getModRefInfo(call2, call1);
        if (isRefSet(MR)) {
          return isMustSet(MR) ? MUST_EXIST : MAY_EXIST;
        }
      }
      break;

    case DG_DATA_WAR:
      // Check whether toInst writes the memory read by fromInst
      if (isa<LoadInst>(fromInst) && isa<CallBase>(toInst)) {
        CallBase *call = cast<CallBase>(toInst);
        MemoryLocation loc = MemoryLocation::get(fromInst);
        ModRefInfo MR = getModRefInfo(call, loc);
        if (isModSet(MR)) {
          return isMustSet(MR) ? MUST_EXIST : MAY_EXIST;
        }
      } else if (isa<CallBase>(fromInst) && isa<StoreInst>(toInst)) {
        CallBase *call = cast<CallBase>(fromInst);
        MemoryLocation loc = MemoryLocation::get(toInst);
        ModRefInfo MR = getModRefInfo(call, loc);
        if (isRefSet(MR)) {
          return isMustSet(MR) ? MUST_EXIST : MAY_EXIST;
        }
      } else if (isa<CallBase>(fromInst) && isa<CallBase>(toInst)) {
        CallBase *call1 = cast<CallBase>(fromInst);
        CallBase *call2 = cast<CallBase>(toInst);
        ModRefInfo MR = getModRefInfo(call1, call2);
        if (isRefSet(MR)) {
          return isMustSet(MR) ? MUST_EXIST : MAY_EXIST;
        }
      }
      break;

    case DG_DATA_WAW:
      // Check whether toInst writes the memory written by fromInst
      if (isa<StoreInst>(fromInst) && isa<CallBase>(toInst)) {
        CallBase *call = cast<CallBase>(toInst);
        MemoryLocation loc = MemoryLocation::get(fromInst);
        ModRefInfo MR = getModRefInfo(call, loc);
        if (isModSet(MR)) {
          return isMustSet(MR) ? MUST_EXIST : MAY_EXIST;
        }
      } else if (isa<CallBase>(fromInst) && isa<StoreInst>(toInst)) {
        CallBase *call = cast<CallBase>(fromInst);
        MemoryLocation loc = MemoryLocation::get(toInst);
        ModRefInfo MR = getModRefInfo(call, loc);
        if (isModSet(MR)) {
          return isMustSet(MR) ? MUST_EXIST : MAY_EXIST;
        }
      } else if (isa<CallBase>(fromInst) && isa<CallBase>(toInst)) {
        CallBase *call1 = cast<CallBase>(fromInst);
        CallBase *call2 = cast<CallBase>(toInst);
        ModRefInfo MR = getModRefInfo(call2, call1);
        if (isModSet(MR)) {
          return isMustSet(MR) ? MUST_EXIST : MAY_EXIST;
        }
      }
      break;
  }

  // DEBUG
  printMsg(t, fromInst, toInst);

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
