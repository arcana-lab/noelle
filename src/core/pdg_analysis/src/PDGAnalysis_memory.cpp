/*
 * Copyright 2016 - 2023  Angelo Matni, Yian Su, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/TalkDown.hpp"
#include "noelle/core/PDGPrinter.hpp"
#include "noelle/core/PDGAnalysis.hpp"
#include "IntegrationWithSVF.hpp"
#include "noelle/core/Utils.hpp"

namespace llvm::noelle {

bool PDGAnalysis::canThereBeAMemoryDataDependence(Instruction *fromInst,
                                                  Instruction *toInst,
                                                  Function &F) {

  /*
   * Check if any of the data dependence analyses can assert the lack of
   * dependence from @fromInst to @toInst.
   */
  for (auto ddAnalysis : this->ddAnalyses) {
    if (!ddAnalysis->canThereBeAMemoryDataDependence(fromInst, toInst, F)) {

      /*
       * We found a dependence analysis that can assert the lack of dependence.
       */
      return false;
    }
  }

  /*
   * We did not find a single dependence analysis that can assert the lack of
   * dependence. So we must assume this dependence can happen at run time.
   */
  return true;
}

std::pair<bool, bool> PDGAnalysis::isThereThisMemoryDataDependenceType(
    DataDependenceType t,
    Instruction *fromInst,
    Instruction *toInst,
    Function &F) {
  auto noDep = false;
  auto mustExist = false;

  for (auto ddAnalysis : this->ddAnalyses) {
    auto resp =
        ddAnalysis->isThereThisMemoryDataDependenceType(t, fromInst, toInst, F);
    if (resp == CANNOT_EXIST) {
      noDep = true;
      break;
    }
    if (resp == MUST_EXIST) {
      mustExist = true;
      break;
    }
  }

  return make_pair(noDep, mustExist);
}

void PDGAnalysis::iterateInstForStore(PDG *pdg,
                                      Function &F,
                                      AAResults &AA,
                                      DataFlowResult *dfr,
                                      StoreInst *store) {

  for (auto I : dfr->OUT(store)) {

    /*
     * Check if the instruction can access memory.
     */
    auto inst = dyn_cast<Instruction>(I);
    if (inst == nullptr) {
      continue;
    }
    if (!PDGAnalysis::canAccessMemory(inst)) {
      continue;
    }

    /*
     * The instruction can access memory.
     *
     * Check if any of the data dependence analyses can assert the lack of
     * dependence from @store to @I.
     */
    if (!this->canThereBeAMemoryDataDependence(store, inst, F)) {
      continue;
    }

    /*
     * Check stores.
     */
    if (auto otherStore = dyn_cast<StoreInst>(inst)) {
      this->addEdgeFromMemoryAlias(pdg, F, AA, store, otherStore, DG_DATA_WAW);
      continue;
    }

    /*
     * Check loads.
     */
    if (auto load = dyn_cast<LoadInst>(inst)) {
      this->addEdgeFromMemoryAlias(pdg, F, AA, store, load, DG_DATA_RAW);
      continue;
    }

    /*
     * Check calls.
     */
    if (auto call = dyn_cast<CallBase>(inst)) {
      if (!Utils::isActualCode(call)) {
        continue;
      }
      this->addEdgeFromFunctionModRef(pdg, F, AA, call, store, false);
      continue;
    }
  }

  return;
}

void PDGAnalysis::iterateInstForLoad(PDG *pdg,
                                     Function &F,
                                     AAResults &AA,
                                     DataFlowResult *dfr,
                                     LoadInst *load) {

  for (auto I : dfr->OUT(load)) {

    /*
     * Check if the instruction can access memory.
     */
    auto inst = dyn_cast<Instruction>(I);
    if (inst == nullptr) {
      continue;
    }
    if (!PDGAnalysis::canAccessMemory(inst)) {
      continue;
    }

    /*
     * The instruction can access memory.
     *
     * Check if any of the data dependence analyses can assert the lack of
     * dependence from @load to @I.
     */
    if (!this->canThereBeAMemoryDataDependence(load, inst, F)) {
      continue;
    }

    /*
     * Check stores.
     */
    if (auto store = dyn_cast<StoreInst>(inst)) {
      this->addEdgeFromMemoryAlias(pdg, F, AA, load, store, DG_DATA_WAR);
      continue;
    }

    /*
     * Check calls.
     */
    if (auto call = dyn_cast<CallBase>(inst)) {
      this->addEdgeFromFunctionModRef(pdg, F, AA, call, load, false);
      continue;
    }
  }

  return;
}

void PDGAnalysis::iterateInstForCall(PDG *pdg,
                                     Function &F,
                                     AAResults &AA,
                                     DataFlowResult *dfr,
                                     CallBase *call) {

  /*
   * Check if the call instruction is not actual code.
   */
  if (!Utils::isActualCode(call)) {
    return;
  }

  /*
   * Check if the call instruction is pure.
   */
  if (this->hasNoMemoryOperations(call)) {
    return;
  }

  /*
   * Check if the instruction can access memory.
   */
  if (!PDGAnalysis::canAccessMemory(call)) {
    return;
  }

  /*
   * Identify all dependences from @call.
   */
  for (auto I : dfr->OUT(call)) {

    /*
     * Check if the instruction can access memory.
     */
    auto inst = dyn_cast<Instruction>(I);
    if (inst == nullptr) {
      continue;
    }
    if (!PDGAnalysis::canAccessMemory(inst)) {
      continue;
    }

    /*
     * The instruction can access memory.
     *
     * Check if any of the data dependence analyses can assert the lack of
     * dependence from @call to @I.
     */
    if (!this->canThereBeAMemoryDataDependence(call, inst, F)) {
      continue;
    }

    /*
     * Check stores.
     */
    if (auto store = dyn_cast<StoreInst>(inst)) {
      addEdgeFromFunctionModRef(pdg, F, AA, call, store, true);
      continue;
    }

    /*
     * Check loads.
     */
    if (auto load = dyn_cast<LoadInst>(inst)) {
      addEdgeFromFunctionModRef(pdg, F, AA, call, load, true);
      continue;
    }

    /*
     * Check calls.
     */
    if (auto baseOtherCall = dyn_cast<CallBase>(inst)) {

      /*
       * Check direct calls
       */
      if (auto otherCall = dyn_cast<CallInst>(baseOtherCall)) {
        if (!Utils::isActualCode(otherCall)) {
          continue;
        }
      }
      bool isCallReachableFromOtherCall =
          dfr->OUT(baseOtherCall).count(call) > 0 ? true : false;
      this->addEdgeFromFunctionModRef(pdg,
                                      F,
                                      AA,
                                      call,
                                      baseOtherCall,
                                      isCallReachableFromOtherCall);
      continue;
    }
  }

  return;
}

bool PDGAnalysis::hasNoMemoryOperations(CallBase *call) {
  assert(call != nullptr);

  /*
   * Check if SVF is enabled.
   */
  if (this->disableSVF) {
    return false;
  }

  /*
   * Check if the callee is a library function.
   */
  auto calleeFunction = call->getCalledFunction();
  if (calleeFunction != nullptr) {
    if (calleeFunction->empty()) {
      if (this->isTheLibraryFunctionPure(calleeFunction)) {
        return true;
      }
      return false;
    }
  }

  /*
   * SVF is enabled.
   * We can use it.
   */
  if (NoelleSVFIntegration::getModRefInfo(call) == ModRefInfo::NoModRef) {
    return true;
  }

  return false;
}

void PDGAnalysis::addEdgeFromFunctionModRef(PDG *pdg,
                                            Function &F,
                                            AAResults &AA,
                                            CallBase *call,
                                            StoreInst *store,
                                            bool addEdgeFromCall) {
  BitVector bv{ 3, false };
  auto makeRefEdge = false, makeModEdge = false;

  /*
   * We cannot have memory dependences from a call to a deallocator (e.g.,
   * free).
   */
  if (Utils::isDeallocator(call)) {
    return;
  }

  /*
   * Query the LLVM alias analyses.
   */
  switch (AA.getModRefInfo(call, MemoryLocation::get(store))) {
    case ModRefInfo::NoModRef:
      return;
    case ModRefInfo::Ref:
      bv[0] = true;
      break;
    case ModRefInfo::Mod:
      bv[1] = true;
      break;
    case ModRefInfo::ModRef:
      bv[2] = true;
      break;
  }

  /*
   * Check other alias analyses
   *
   * Check if SVF is enabled.
   */
  if (this->disableSVF) {

    /*
     * SVF is disabled.
     */

  } else {

    /*
     * SVF is enabled; let's use it.
     */
    auto weCanRelyOnSVF = cannotReachUnhandledExternalFunction(call);
    if (weCanRelyOnSVF && hasNoMemoryOperations(call)) {
      return;
    }

    /*
     * Check if it is safe to use SVF.
     * This is due to a bug in SVF that doesn't model I/O library calls
     * correctly.
     */
    if (this->isSafeToQueryModRefOfSVF(call, bv)) {
      auto const &loc = MemoryLocation::get(store);
      switch (NoelleSVFIntegration::getModRefInfo(call, loc)) {
        case ModRefInfo::NoModRef:
          return;
        case ModRefInfo::Ref:
          bv[0] = true;
          break;
        case ModRefInfo::Mod:
          bv[1] = true;
          break;
        case ModRefInfo::ModRef:
          bv[2] = true;
          break;
      }
    }
  }

  /*
   * NoModRef when one says Mod and another says Ref
   */
  if (bv[0] && bv[1]) {
    return;
  }
  if (bv[0]) {
    makeRefEdge = true;
  } else if (bv[1]) {
    makeModEdge = true;
  } else {
    makeRefEdge = makeModEdge = true;
  }

  /*
   * There is a dependence.
   */
  if (makeRefEdge) {
    if (addEdgeFromCall) {

      /*
       * Our alias analyses cannot disprove a may, WAR dependence.
       *
       * Check if any of the data dependence analyses can assert something
       * better.
       */
      auto [noDep, mustExist] =
          this->isThereThisMemoryDataDependenceType(DG_DATA_WAR,
                                                    call,
                                                    store,
                                                    F);
      if (!noDep) {
        pdg->addEdge(call, store)->setMemMustType(true, mustExist, DG_DATA_WAR);
      }

    } else {

      /*
       * We cannot have memory dependences from a memory instruction to
       * allocators as they always return new memory.
       */
      if (!Utils::isAllocator(call)) {

        /*
         * Our alias analyses cannot disprove a may, WAR dependence.
         *
         * Check if any of the data dependence analyses can assert something
         * better.
         */
        auto [noDep, mustExist] =
            this->isThereThisMemoryDataDependenceType(DG_DATA_RAW,
                                                      call,
                                                      store,
                                                      F);
        if (!noDep) {
          pdg->addEdge(store, call)
              ->setMemMustType(true, mustExist, DG_DATA_RAW);
        }
      }
    }
  }
  if (makeModEdge) {
    if (addEdgeFromCall) {

      /*
       * Our alias analyses cannot disprove a may, WAR dependence.
       *
       * Check if any of the data dependence analyses can assert something
       * better.
       */
      auto [noDep, mustExist] =
          this->isThereThisMemoryDataDependenceType(DG_DATA_WAW,
                                                    call,
                                                    store,
                                                    F);
      if (!noDep) {
        pdg->addEdge(call, store)->setMemMustType(true, mustExist, DG_DATA_WAW);
      }

    } else {

      /*
       * We cannot have memory dependences from a memory instruction to
       * allocators as they always return new memory.
       */
      if (!Utils::isAllocator(call)) {

        /*
         * Our alias analyses cannot disprove a may, WAR dependence.
         *
         * Check if any of the data dependence analyses can assert something
         * better.
         */
        auto [noDep, mustExist] =
            this->isThereThisMemoryDataDependenceType(DG_DATA_WAW,
                                                      call,
                                                      store,
                                                      F);
        if (!noDep) {
          pdg->addEdge(store, call)
              ->setMemMustType(true, mustExist, DG_DATA_WAW);
        }
      }
    }
  }

  return;
}

void PDGAnalysis::addEdgeFromFunctionModRef(PDG *pdg,
                                            Function &F,
                                            AAResults &AA,
                                            CallBase *call,
                                            LoadInst *load,
                                            bool addEdgeFromCall) {
  BitVector bv{ 3, false };

  /*
   * We cannot have memory dependences from a call to a deallocator (e.g.,
   * free).
   */
  if (Utils::isDeallocator(call)) {
    return;
  }

  /*
   * Query the LLVM alias analyses.
   */
  switch (AA.getModRefInfo(call, MemoryLocation::get(load))) {
    case ModRefInfo::NoModRef:
    case ModRefInfo::Ref:
      return;
    case ModRefInfo::Mod:
    case ModRefInfo::ModRef:
      break;
  }

  /*
   * Check other alias analyses
   *
   * Check if SVF is enabled.
   */
  if (this->disableSVF) {

    /*
     * SVF is disabled.
     */

  } else {

    /*
     * SVF is enabled; let's use it.
     */
    auto weCanRelyOnSVF = cannotReachUnhandledExternalFunction(call);
    if (weCanRelyOnSVF && hasNoMemoryOperations(call)) {
      return;
    }

    /*
     * Check if it is safe to use SVF.
     * This is due to a bug in SVF that doesn't model I/O library calls
     * correctly.
     */
    if (isSafeToQueryModRefOfSVF(call, bv)) {
      switch (NoelleSVFIntegration::getModRefInfo(call,
                                                  MemoryLocation::get(load))) {
        case ModRefInfo::NoModRef:
        case ModRefInfo::Ref:
          return;

        case ModRefInfo::Mod:
        case ModRefInfo::ModRef:
          break;
      }
    }
  }

  /*
   * There is a dependence.
   */
  if (addEdgeFromCall) {

    /*
     * Our alias analyses cannot disprove a may, WAR dependence.
     *
     * Check if any of the data dependence analyses can assert something better.
     */
    auto [noDep, mustExist] =
        this->isThereThisMemoryDataDependenceType(DG_DATA_RAW, call, load, F);
    if (!noDep) {
      pdg->addEdge(call, load)->setMemMustType(true, false, DG_DATA_RAW);
    }

  } else {

    /*
     * We cannot have memory dependences from a memory instruction to allocators
     * as they always return new memory.
     */
    if (!Utils::isAllocator(call)) {

      /*
       * Our alias analyses cannot disprove a may, WAR dependence.
       *
       * Check if any of the data dependence analyses can assert something
       * better.
       */
      auto [noDep, mustExist] =
          this->isThereThisMemoryDataDependenceType(DG_DATA_WAR, call, load, F);
      if (!noDep) {
        pdg->addEdge(load, call)->setMemMustType(true, false, DG_DATA_WAR);
      }
    }
  }

  return;
}

void PDGAnalysis::addEdgeFromFunctionModRef(PDG *pdg,
                                            Function &F,
                                            AAResults &AA,
                                            CallBase *call,
                                            CallBase *otherCall,
                                            bool isCallReachableFromOtherCall) {
  BitVector bv{ 3, false };
  BitVector rbv{ 3, false };

  /*
   * There is no dependence between allocators
   */
  if (Utils::isAllocator(call) && Utils::isAllocator(otherCall)
      && (!Utils::isReallocator(call)) && (!Utils::isReallocator(otherCall))) {
    return;
  }

  /*
   * Check if the call instructions are about an allocator and a deallocator.
   */
  CallBase *allocatorCall = nullptr;
  CallBase *deallocatorCall = nullptr;
  if (Utils::isAllocator(call)) {
    allocatorCall = call;
  }
  if (Utils::isAllocator(otherCall)) {
    allocatorCall = otherCall;
  }
  if (Utils::isDeallocator(call)) {
    deallocatorCall = call;
  }
  if (Utils::isDeallocator(otherCall)) {
    deallocatorCall = otherCall;
  }
  if ((allocatorCall != nullptr) && (deallocatorCall != nullptr)) {

    /*
     * The call instructions are one allocator and one deallocator.
     * In this case, we only need to check if the pointer accessed by the
     * deallocator can alias the pointer returned by the allocator.
     */
    auto objectAllocated = Utils::getAllocatedObject(allocatorCall);
    assert(objectAllocated != nullptr);
    auto objectFreed = Utils::getFreedObject(deallocatorCall);
    assert(objectFreed != nullptr);
    auto doesAlias =
        this->doTheyAlias(pdg, F, AA, objectAllocated, objectFreed);
    if (doesAlias == NoAlias) {
      return;
    }
  }

  /*
   * Query the LLVM alias analyses.
   */
  switch (AA.getModRefInfo(otherCall, call)) {
    case ModRefInfo::NoModRef:
      return;

    case ModRefInfo::Ref:

      /*
       * @otherCall may read memory locations written by @call
       * Add RAW data dependence from call to otherCall
       */
      bv[0] = true;

      if (isCallReachableFromOtherCall) {
        switch (AA.getModRefInfo(call, otherCall)) {
          case ModRefInfo::NoModRef:
          case ModRefInfo::Ref:
            /*
             * Contradicting
             * if @otherCall Ref @call, and @call is reachable from @otherCall
             * then @call should at least Mod @otherCall
             */
            return;
          case ModRefInfo::Mod:
          case ModRefInfo::ModRef:
            break;
        }
      }
      break;

    case ModRefInfo::Mod:

      /*
       * @otherCall may write a memory location that can be read or written by
       * @call
       */
      bv[1] = true;

      if (isCallReachableFromOtherCall) {
        switch (AA.getModRefInfo(call, otherCall)) {
          case ModRefInfo::NoModRef:
            return;
          case ModRefInfo::Ref:
            rbv[0] = true;
            break;
          case ModRefInfo::Mod:
            rbv[1] = true;
            break;
          case ModRefInfo::ModRef:
            rbv[2] = true;
            break;
        }
      }
      break;

    case ModRefInfo::ModRef:

      /*
       * @otherCall may read or write a memory location that can be written by
       * @call
       */
      bv[2] = true;

      if (isCallReachableFromOtherCall) {
        switch (AA.getModRefInfo(call, otherCall)) {
          case ModRefInfo::NoModRef:
            return;
          case ModRefInfo::Ref:
            /*
             * Contradicting
             * if @otherCall ModRef @call, and @call is reachable from
             * @otherCall then @call should at least Mod @otherCall
             */
            return;
          case ModRefInfo::Mod:
            rbv[1] = true;
            break;
          case ModRefInfo::ModRef:
            rbv[2] = true;
            break;
        }
      }
      break;
  }

  /*
   * Check other alias analyses
   *
   * Check if SVF is enabled.
   */
  if (this->disableSVF) {

    /*
     * SVF is disabled.
     */

  } else {

    /*
     * SVF is enabled; let's use it.
     */
    auto weCanRelyOnSVF = cannotReachUnhandledExternalFunction(call);
    if (weCanRelyOnSVF && hasNoMemoryOperations(call)) {
      return;
    }
    weCanRelyOnSVF = cannotReachUnhandledExternalFunction(otherCall);
    if (weCanRelyOnSVF && hasNoMemoryOperations(otherCall)) {
      return;
    }

    /*
     * Check if it is safe to use SVF.
     * This is due to a bug in SVF that doesn't model I/O library calls
     * correctly.
     */
    if (isSafeToQueryModRefOfSVF(call, bv)
        && isSafeToQueryModRefOfSVF(otherCall, bv)) {
      switch (NoelleSVFIntegration::getModRefInfo(otherCall, call)) {
        case ModRefInfo::NoModRef:
          return;

        case ModRefInfo::Ref:
          bv[0] = true;
          if (isCallReachableFromOtherCall) {
            switch (NoelleSVFIntegration::getModRefInfo(call, otherCall)) {
              case ModRefInfo::NoModRef:
              case ModRefInfo::Ref:
                return;
              case ModRefInfo::Mod:
              case ModRefInfo::ModRef:
                break;
            }
          }
          break;

        case ModRefInfo::Mod:
          bv[1] = true;
          if (isCallReachableFromOtherCall) {
            switch (NoelleSVFIntegration::getModRefInfo(call, otherCall)) {
              case ModRefInfo::NoModRef:
                return;
              case ModRefInfo::Ref:
                rbv[0] = true;
                break;
              case ModRefInfo::Mod:
                rbv[1] = true;
                break;
              case ModRefInfo::ModRef:
                rbv[2] = true;
                break;
            }
          }
          break;

        case ModRefInfo::ModRef:
          bv[2] = true;
          if (isCallReachableFromOtherCall) {
            switch (NoelleSVFIntegration::getModRefInfo(call, otherCall)) {
              case ModRefInfo::NoModRef:
                return;
              case ModRefInfo::Ref:
                return;
              case ModRefInfo::Mod:
                rbv[1] = true;
                break;
              case ModRefInfo::ModRef:
                rbv[2] = true;
                break;
            }
          }
          break;
      }
    }
  }

  if (bv[0] && bv[1]) {
    /*
     * Contradicting
     */
    return;
  }
  if (bv[0]) {

    /*
     * @otherCall reads a memory location that @call writes.
     * there is a RAW memory dependence from @call to @otherCall
     *
     * Our alias analyses cannot disprove a may, WAR dependence.
     *
     * Check if any of the data dependence analyses can assert something better.
     */
    auto [noDep, mustExist] =
        this->isThereThisMemoryDataDependenceType(DG_DATA_RAW,
                                                  call,
                                                  otherCall,
                                                  F);
    if (!noDep) {
      pdg->addEdge(call, otherCall)
          ->setMemMustType(true, mustExist, DG_DATA_RAW);
    }

    /*
     * Check the unique case that @otherCall and @call are the same.
     * In this case, there is also a WAR dependence
     */
    if (otherCall == call) {

      /*
       * Our alias analyses cannot disprove a may, WAR dependence.
       *
       * Check if any of the data dependence analyses can assert something
       * better.
       */
      auto [noDep, mustExist] =
          this->isThereThisMemoryDataDependenceType(DG_DATA_WAR,
                                                    call,
                                                    otherCall,
                                                    F);
      if (!noDep) {
        pdg->addEdge(call, otherCall)
            ->setMemMustType(true, mustExist, DG_DATA_WAR);
      }
    }

  } else if (bv[1]) {
    if (rbv[0] && rbv[1]) {
      /*
       * Contradicting
       */
      return;
    }

    if (rbv[0]) {

      /*
       * Check the unique case that @otherCall and @call are the same.
       */
      if (otherCall == call) {
        /*
         * Contradicting
         * if @call Mod itself, the reverse query should also return Mod result
         */
        return;
      }

      /*
       * @call may read a memory location that can be written by @otherCall
       * only need to add WAR data dependence from @call to @otherCall
       *
       * Our alias analyses cannot disprove a may, WAR dependence.
       *
       * Check if any of the data dependence analyses can assert something
       * better.
       */
      auto [noDep, mustExist] =
          this->isThereThisMemoryDataDependenceType(DG_DATA_WAR,
                                                    call,
                                                    otherCall,
                                                    F);
      if (!noDep) {
        pdg->addEdge(call, otherCall)
            ->setMemMustType(true, mustExist, DG_DATA_WAR);
      }

    } else if (rbv[1]) {

      /*
       * @call may write a memory location that can be written by @otherCall
       * only need to add WAW data dependence from call to otherCall
       *
       * Our alias analyses cannot disprove a may, WAR dependence.
       *
       * Check if any of the data dependence analyses can assert something
       * better.
       */
      auto [noDep, mustExist] =
          this->isThereThisMemoryDataDependenceType(DG_DATA_WAW,
                                                    call,
                                                    otherCall,
                                                    F);
      if (!noDep) {
        pdg->addEdge(call, otherCall)
            ->setMemMustType(true, mustExist, DG_DATA_WAW);
      }

    } else {

      /*
       * Check the unique case that @otherCall and @call are the same.
       */
      if (otherCall == call) {

        /*
         * Contradicting
         * if @call Mod itself, the reverse query should also return Mod result
         */
        return;
      }

      /*
       * whatever rbv[2] (reverseModRef) is set or not
       * need to add both WAR and WAW from @call to @otherCall
       *
       * Our alias analyses cannot disprove a may, WAR dependence.
       *
       * Check if any of the data dependence analyses can assert something
       * better.
       */
      auto [noDep, mustExist] =
          this->isThereThisMemoryDataDependenceType(DG_DATA_WAR,
                                                    call,
                                                    otherCall,
                                                    F);
      if (!noDep) {
        pdg->addEdge(call, otherCall)
            ->setMemMustType(true, mustExist, DG_DATA_WAR);
      }
      auto [noDep2, mustExist2] =
          this->isThereThisMemoryDataDependenceType(DG_DATA_WAW,
                                                    call,
                                                    otherCall,
                                                    F);
      if (!noDep2) {
        pdg->addEdge(call, otherCall)
            ->setMemMustType(true, mustExist2, DG_DATA_WAW);
      }
    }

  } else {
    assert(bv[2] == true
           && "otherCall ModRef call but the bit isn't set correctnly\n");
    if (rbv[0]) {
      /*
       * Contradicting
       */
      return;

    } else if (rbv[1]) {
      /*
       * Check the unique case that @otherCall and @call are the same.
       */
      if (otherCall == call) {
        /*
         * Contradicting
         * if @call ModRef itself, the reverse query should also return ModRef
         * result
         */
        return;
      }

      /*
       * @call may write a memory location that can be read or written by
       * @otherCall need to add both RAW and WAW from call to otherCall
       *
       * Our alias analyses cannot disprove a may, WAR dependence.
       *
       * Check if any of the data dependence analyses can assert something
       * better.
       */
      auto [noDep, mustExist] =
          this->isThereThisMemoryDataDependenceType(DG_DATA_RAW,
                                                    call,
                                                    otherCall,
                                                    F);
      if (!noDep) {
        pdg->addEdge(call, otherCall)
            ->setMemMustType(true, mustExist, DG_DATA_RAW);
      }
      auto [noDep2, mustExist2] =
          this->isThereThisMemoryDataDependenceType(DG_DATA_WAW,
                                                    call,
                                                    otherCall,
                                                    F);
      if (!noDep2) {
        pdg->addEdge(call, otherCall)
            ->setMemMustType(true, mustExist2, DG_DATA_WAW);
      }

    } else if (rbv[2]) {

      /*
       * @call may read or write a memory location that can be written by
       * @otherCall need to add all RAW, WAW and WAR from @call to @otherCall
       *
       * Our alias analyses cannot disprove a may dependence.
       *
       * Check if any of the data dependence analyses can assert something
       * better.
       */
      {
        auto [noDep, mustExist] =
            this->isThereThisMemoryDataDependenceType(DG_DATA_RAW,
                                                      call,
                                                      otherCall,
                                                      F);
        if (!noDep) {
          pdg->addEdge(call, otherCall)
              ->setMemMustType(true, mustExist, DG_DATA_RAW);
        }
      }
      {
        auto [noDep, mustExist] =
            this->isThereThisMemoryDataDependenceType(DG_DATA_WAW,
                                                      call,
                                                      otherCall,
                                                      F);
        if (!noDep) {
          pdg->addEdge(call, otherCall)
              ->setMemMustType(true, mustExist, DG_DATA_WAW);
        }
      }
      {
        auto [noDep, mustExist] =
            this->isThereThisMemoryDataDependenceType(DG_DATA_WAR,
                                                      call,
                                                      otherCall,
                                                      F);
        if (!noDep) {
          pdg->addEdge(call, otherCall)
              ->setMemMustType(true, mustExist, DG_DATA_WAR);
        }
      }

    } else {
      assert(
          !rbv[0] && !rbv[1] && !rbv[2]
          && "otherCall ModRef call and call is unreachable from otherCall, but the bit isn't set correctly\n");

      /*
       * Check the unique case that @otherCall and @call are the same.
       */
      if (otherCall == call) {
        /*
         * Contradicting
         * if @call ModRef itself, the reverse query should also return ModRef
         * result
         */
        return;
      }

      /*
       * @otherCall may read or write a memory location that can be written by
       * @call need to add both RAW, WAW @call to @otherCall
       *
       * Our alias analyses cannot disprove a may dependence.
       *
       * Check if any of the data dependence analyses can assert something
       * better.
       */
      {
        auto [noDep, mustExist] =
            this->isThereThisMemoryDataDependenceType(DG_DATA_RAW,
                                                      call,
                                                      otherCall,
                                                      F);
        if (!noDep) {
          pdg->addEdge(call, otherCall)
              ->setMemMustType(true, mustExist, DG_DATA_RAW);
        }
      }
      {
        auto [noDep, mustExist] =
            this->isThereThisMemoryDataDependenceType(DG_DATA_WAW,
                                                      call,
                                                      otherCall,
                                                      F);
        if (!noDep) {
          pdg->addEdge(call, otherCall)
              ->setMemMustType(true, mustExist, DG_DATA_WAW);
        }
      }
    }
  }

  return;
}

bool PDGAnalysis::isSafeToQueryModRefOfSVF(CallBase *call, BitVector &bv) {

  /*
   * Check if SVF is enabled.
   */
  if (this->disableSVF) {

    /*
     * SVF is disabled.
     */
    return false;
  }

  if (NoelleSVFIntegration::hasIndCSCallees(call)) {
    auto callees = NoelleSVFIntegration::getIndCSCallees(call);
    for (auto &callee : callees) {
      if (this->isUnhandledExternalFunction(callee)
          || isInternalFunctionThatReachUnhandledExternalFunction(callee)) {
        return false;
      }
    }

  } else {
    auto callee = call->getCalledFunction();
    if (!callee) {
      bv[2] = true; // ModRef bit is set
      return false;
    }

    if (this->isUnhandledExternalFunction(callee)
        || isInternalFunctionThatReachUnhandledExternalFunction(callee)) {
      return false;
    }
  }

  return true;
}

void PDGAnalysis::addEdgeFromMemoryAlias(
    PDG *pdg,
    Function &F,
    AAResults &AA,
    Value *instI,
    Value *instJ,
    DataDependenceType dataDependenceType) {

  /*
   * Query all alias analyses that work at the program scope.
   */
  auto aaResult = this->doTheyAlias(pdg, F, AA, instI, instJ);

  /*
   * Translate the alias analysis result to dependences.
   */
  auto must = false;
  switch (aaResult) {
    case NoAlias:
      return;
    case PartialAlias:
    case MayAlias:
      break;
    case MustAlias:
      must = true;
      break;
  }

  /*
   * There is a dependence.
   */
  pdg->addEdge(instI, instJ)->setMemMustType(true, must, dataDependenceType);

  return;
}

AliasResult PDGAnalysis::doTheyAlias(PDG *pdg,
                                     Function &F,
                                     AAResults &AA,
                                     Value *instI,
                                     Value *instJ) {

  /*
   * Check if the parameters have memory locations.
   */
  auto haveMemoryLocations = false;
  auto instIAsInst = dyn_cast<Instruction>(instI);
  auto instJAsInst = dyn_cast<Instruction>(instJ);
  if ((instIAsInst != nullptr) && (instJAsInst != nullptr)) {
    if (MemoryLocation::getOrNone(instIAsInst)
        && MemoryLocation::getOrNone(instJAsInst)) {
      haveMemoryLocations = true;
    }
  }

  /*
   * Query the LLVM alias analyses.
   */
  AliasResult aaResult;
  if (haveMemoryLocations) {
    aaResult = AA.alias(MemoryLocation::get(instIAsInst),
                        MemoryLocation::get(instJAsInst));
  } else {
    aaResult = AA.alias(instI, instJ);
  }
  switch (aaResult) {
    case NoAlias:
      return NoAlias;
    case PartialAlias:
    case MayAlias:
      break;
    case MustAlias:
      return MustAlias;
  }

  /*
   * Check other alias analyses
   *
   * Check if SVF is enabled.
   */
  if (this->disableSVF) {

    /*
     * SVF is disabled.
     */

  } else {

    /*
     * SVF is enabled, so let's use it.
     */
    AliasResult SVFAAResult;
    if (haveMemoryLocations) {
      SVFAAResult =
          NoelleSVFIntegration::alias(MemoryLocation::get(instIAsInst),
                                      MemoryLocation::get(instJAsInst));
    } else {
      SVFAAResult = NoelleSVFIntegration::alias(instI, instJ);
    }
    switch (SVFAAResult) {
      case NoAlias:
        return NoAlias;
      case PartialAlias:
      case MayAlias:
        break;
      case MustAlias:
        return MustAlias;
    }
  }

  return MayAlias;
}

bool PDGAnalysis::canAccessMemory(Instruction *i) {

  /*
   * Check if @i just metadata.
   */
  if (!Utils::isActualCode(i)) {
    return false;
  }

  /*
   * Check if @i can access memory.
   */
  if (isa<StoreInst>(i)) {
    return true;
  }
  if (isa<LoadInst>(i)) {
    return true;
  }
  if (auto call = dyn_cast<CallBase>(i)) {
    auto callee = call->getCalledFunction();
    if (callee != nullptr) {
      if (PDGAnalysis::isTheLibraryFunctionPure(callee)) {
        return false;
      }
    }

    return true;
  }

  return false;
}

} // namespace llvm::noelle
