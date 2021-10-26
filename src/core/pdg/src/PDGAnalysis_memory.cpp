/*
 * Copyright 2016 - 2020  Angelo Matni, Yian Su, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "SystemHeaders.hpp"
#include "TalkDown.hpp"
#include "noelle/core/PDGPrinter.hpp"
#include "noelle/core/PDGAnalysis.hpp"
#include "IntegrationWithSVF.hpp"
#include "Utils.hpp"

namespace llvm::noelle {

void PDGAnalysis::iterateInstForStore (PDG *pdg, Function &F, AAResults &AA, DataFlowResult *dfr, StoreInst *store) {

  for (auto I : dfr->OUT(store)) {

    /*
     * Check stores.
     */
    if (auto otherStore = dyn_cast<StoreInst>(I)) {
      if (store != otherStore) {
        this->addEdgeFromMemoryAlias<StoreInst, StoreInst>(pdg, F, AA, store, otherStore, DG_DATA_WAW);
      }
      continue ;
    }

    /* 
     * Check loads.
     */
    if (auto load = dyn_cast<LoadInst>(I)) {
      this->addEdgeFromMemoryAlias<StoreInst, LoadInst>(pdg, F, AA, store, load, DG_DATA_RAW);
      continue ;
    }

    /*
     * Check calls.
     */
    if (auto call = dyn_cast<CallInst>(I)) {
      if (!Utils::isActualCode(call)){
        continue ;
      }
      this->addEdgeFromFunctionModRef(pdg, F, AA, call, store, false);
      continue ;
    }
  }

  return ;
}

void PDGAnalysis::iterateInstForLoad (PDG *pdg, Function &F, AAResults &AA, DataFlowResult *dfr, LoadInst *load) {

  for (auto I : dfr->OUT(load)) {

    /*
     * Check stores.
     */
    if (auto store = dyn_cast<StoreInst>(I)) {
      addEdgeFromMemoryAlias<LoadInst, StoreInst>(pdg, F, AA, load, store, DG_DATA_WAR);
      continue ;
    }

    /*
     * Check calls.
     */
    if (auto call = dyn_cast<CallInst>(I)) {
      if (!Utils::isActualCode(call)){
        continue ;
      }
      addEdgeFromFunctionModRef(pdg, F, AA, call, load, false);
      continue ;
    }
  }

  return ;
}

bool PDGAnalysis::hasNoMemoryOperations (CallInst *call) {

  /*
   * Check if SVF is enabled.
   */
  if (this->disableSVF){
    return false;
  }

  /*
   * Check if the callee is a library function.
   */
  auto calleeFunction = call->getCalledFunction();
  if (calleeFunction != nullptr){
    if (  true
          && calleeFunction->empty()
          && this->isTheLibraryFunctionPure(calleeFunction)
       ){
      return true ;
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

void PDGAnalysis::addEdgeFromFunctionModRef (PDG *pdg, Function &F, AAResults &AA, CallInst *call, StoreInst *store, bool addEdgeFromCall) {
  BitVector bv(3, false);
  auto makeRefEdge = false, makeModEdge = false;

  /*
   * We cannot have memory dependences from a call to a deallocator (e.g., free).
   */
  if (Utils::isDeallocator(call)){
    return ;
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
  if (this->disableSVF){

    /*
     * SVF is disabled.
     */

  } else {

    /*
     * SVF is enabled; let's use it.
     */
    auto weCanRelyOnSVF = cannotReachUnhandledExternalFunction(call);
    if (  true
          && weCanRelyOnSVF 
          && hasNoMemoryOperations(call)
      ) {
      return;
    }

    /*
     * Check if it is safe to use SVF.
     * This is due to a bug in SVF that doesn't model I/O library calls correctly.
     */
    if (isSafeToQueryModRefOfSVF(call, bv)) {
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
      pdg->addEdge(call, store)->setMemMustType(true, false, DG_DATA_WAR);

    } else {

      /*
       * We cannot have memory dependences from a memory instruction to allocators as they always return new memory.
       */
      if (!Utils::isAllocator(call)){
        pdg->addEdge(store, call)->setMemMustType(true, false, DG_DATA_RAW);
      }
    }
  }
  if (makeModEdge) {
    if (addEdgeFromCall) {
      pdg->addEdge(call, store)->setMemMustType(true, false, DG_DATA_WAW);

    } else {

      /*
       * We cannot have memory dependences from a memory instruction to allocators as they always return new memory.
       */
      if (!Utils::isAllocator(call)){
        pdg->addEdge(store, call)->setMemMustType(true, false, DG_DATA_WAW);
      }
    }
  }

  return ;
}

void PDGAnalysis::addEdgeFromFunctionModRef (PDG *pdg, Function &F, AAResults &AA, CallInst *call, LoadInst *load, bool addEdgeFromCall) {
  BitVector bv(3, false);

  /*
   * We cannot have memory dependences from a call to a deallocator (e.g., free).
   */
  if (Utils::isDeallocator(call)){
    return ;
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
  if (this->disableSVF){

    /*
     * SVF is disabled.
     */

  } else {

    /*
     * SVF is enabled; let's use it.
     */
    auto weCanRelyOnSVF = cannotReachUnhandledExternalFunction(call);
    if (  true
          && weCanRelyOnSVF 
          && hasNoMemoryOperations(call)
      ) {
      return;
    }

    /*
     * Check if it is safe to use SVF.
     * This is due to a bug in SVF that doesn't model I/O library calls correctly.
     */
    if (isSafeToQueryModRefOfSVF(call, bv)) {
      switch (NoelleSVFIntegration::getModRefInfo(call, MemoryLocation::get(load))) {
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
    pdg->addEdge(call, load)->setMemMustType(true, false, DG_DATA_RAW);

  } else {

    /*
     * We cannot have memory dependences from a memory instruction to allocators as they always return new memory.
     */
    if (!Utils::isAllocator(call)){
      pdg->addEdge(load, call)->setMemMustType(true, false, DG_DATA_WAR);
    }
  }

  return ;
}

void PDGAnalysis::addEdgeFromFunctionModRef (PDG *pdg, Function &F, AAResults &AA, CallInst *call, CallInst *otherCall) {
  BitVector bv(3, false);
  BitVector rbv(3, false);
  auto makeRefEdge = false, makeModEdge = false, makeModRefEdge = false;
  auto reverseRefEdge = false, reverseModEdge = false, reverseModRefEdge = false;

  /*
   * There is no dependence between allocators
   */
  if (  true
        && Utils::isAllocator(call)
        && Utils::isAllocator(otherCall)
        && (!Utils::isReallocator(call))
        && (!Utils::isReallocator(otherCall))
     ){
    return ;
  }

  /*
   * Check if the call instructions are about an allocator and a deallocator.
   */
  CallInst *allocatorCall = nullptr;
  CallInst *deallocatorCall = nullptr;
  if (Utils::isAllocator(call)){
    allocatorCall = call;
  }
  if (Utils::isAllocator(otherCall)){
    allocatorCall = otherCall;
  }
  if (Utils::isDeallocator(call)){
    deallocatorCall = call;
  }
  if (Utils::isDeallocator(otherCall)){
    deallocatorCall = otherCall;
  }
  if (  true
        && (allocatorCall != nullptr)
        && (deallocatorCall != nullptr)
     ){

    /*
     * The call instructions are one allocator and one deallocator.
     * In this case, we only need to check if the pointer accessed by the deallocator can alias the pointer returned by the allocator.
     */
    auto objectAllocated = Utils::getAllocatedObject(allocatorCall);
    assert(objectAllocated != nullptr);
    auto objectFreed = Utils::getFreedObject(deallocatorCall);
    assert(objectFreed != nullptr);
    auto doesAlias = this->doTheyAlias(pdg, F, AA, objectAllocated, objectFreed);
    if (doesAlias == NoAlias){
      return ;
    }
  }

  /*
   * Query the LLVM alias analyses.
   */
  switch (AA.getModRefInfo(call, otherCall)) {
    case ModRefInfo::NoModRef:
      return;

    case ModRefInfo::Ref:

      /*
       * @call may read memory locations written by @otherCall
       */
      bv[0] = true;
      break;

    case ModRefInfo::Mod:

      /*
       * @call may write a memory location that can be read or written by @otherCall
       */
      bv[1] = true;

      switch (AA.getModRefInfo(otherCall, call)) {
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
      break;

    case ModRefInfo::ModRef:

      /*
       * @call may read or write a memory location that can be written by @otherCall
       */
      bv[2] = true;
      break;
  }

  /*
   * Check other alias analyses
   *
   * Check if SVF is enabled.
   */
  if (this->disableSVF){

    /*
     * SVF is disabled.
     */

  } else {

    /*
     * SVF is enabled; let's use it.
     */
    auto weCanRelyOnSVF = cannotReachUnhandledExternalFunction(call);
    if (  true
          && weCanRelyOnSVF 
          && hasNoMemoryOperations(call)
      ) {
      return;
    }
    weCanRelyOnSVF = cannotReachUnhandledExternalFunction(otherCall);
    if (  true
          && weCanRelyOnSVF 
          && hasNoMemoryOperations(otherCall)
      ) {
      return;
    }

    /*
     * Check if it is safe to use SVF.
     * This is due to a bug in SVF that doesn't model I/O library calls correctly.
     */
    if (  true
          && isSafeToQueryModRefOfSVF(call, bv) 
          && isSafeToQueryModRefOfSVF(otherCall, bv)
      ) {
      switch (NoelleSVFIntegration::getModRefInfo(call, otherCall)) {
        case ModRefInfo::NoModRef:
          return;

        case ModRefInfo::Ref:
          bv[0] = true;
          break;

        case ModRefInfo::Mod:
          bv[1] = true;

          switch (NoelleSVFIntegration::getModRefInfo(otherCall, call)) {
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
          break;

        case ModRefInfo::ModRef:
          bv[2] = true;
          break;
      }
    }
  }

  if (bv[0] && bv[1]) {
    return;
  }
  if (bv[0]) {
    makeRefEdge = true;

  } else if (bv[1]) {
    makeModEdge = true ;
    if (rbv[0] && rbv[1]) {
      return ;
    }

    if (rbv[0]) {
      reverseRefEdge = true;
    } else if (rbv[1]) {
      reverseModEdge = true;
    } else {
      reverseModRefEdge = true;
    }

  } else {
    makeModRefEdge = true;
  }

  /*
   * There is a dependence.
   */
  if (makeRefEdge) {

    /*
     * @call reads a memory location that @otherCall writes.
     * The sequence of execution is @call and then @otherCall.
     * Hence, there is a WAR memory dependence from @call to @otherCall
     */
    pdg->addEdge(call, otherCall)->setMemMustType(true, false, DG_DATA_WAR);

    /*
     * Check the unique case that @call and @otherCall are the same. 
     * In this case, there is also a RAW dependence between them.
     */
    if (call == otherCall){
      pdg->addEdge(otherCall, call)->setMemMustType(true, false, DG_DATA_RAW);
    }

  } else if (makeModEdge) {

    /*
     * Dependency of a Mod-result between call and otherCall depends on the reverse getModRefInfo result
     */
    if (reverseRefEdge) {
      pdg->addEdge(call, otherCall)->setMemMustType(true, false, DG_DATA_RAW);

      /*
       * Check the unique case that @call and @otherCall are the same. 
       * In this case, there is also a WAR dependence between them.
       */
      if (call == otherCall){
        pdg->addEdge(otherCall, call)->setMemMustType(true, false, DG_DATA_WAR);
      }

    } else if (reverseModEdge) {
      pdg->addEdge(call, otherCall)->setMemMustType(true, false, DG_DATA_WAW);

    } else if (reverseModRefEdge) {
      pdg->addEdge(call, otherCall)->setMemMustType(true, false, DG_DATA_RAW);
      pdg->addEdge(call, otherCall)->setMemMustType(true, false, DG_DATA_WAW);

      /*
       * Check the unique case that @call and @otherCall are the same. 
       * In this case, there is also a WAR dependence between them.
       */
      if (call == otherCall){
        pdg->addEdge(otherCall, call)->setMemMustType(true, false, DG_DATA_WAR);
      }
    }

  } else if (makeModRefEdge) {
    pdg->addEdge(call, otherCall)->setMemMustType(true, false, DG_DATA_WAR);
    pdg->addEdge(call, otherCall)->setMemMustType(true, false, DG_DATA_WAW);

    /*
     * Check the unique case that @call and @otherCall are the same. 
     * In this case, there is also a RAW dependence between them.
     */
    if (call == otherCall){
      pdg->addEdge(otherCall, call)->setMemMustType(true, false, DG_DATA_RAW);
    }
  }

  return ;
}

bool PDGAnalysis::isSafeToQueryModRefOfSVF(CallInst *call, BitVector &bv) {

  /*
   * Check if SVF is enabled.
   */
  if (this->disableSVF){

    /*
     * SVF is disabled.
     */
    return false;
  }

  if (NoelleSVFIntegration::hasIndCSCallees(call)) {
    auto callees = NoelleSVFIntegration::getIndCSCallees(call);
    for (auto &callee : callees) {
      if (this->isUnhandledExternalFunction(callee) || isInternalFunctionThatReachUnhandledExternalFunction(callee)) {
        return false;
      }
    }

  } else {
    auto callee = call->getCalledFunction();
    if (!callee) {
      bv[2] = true; // ModRef bit is set
      return false;
    }

    if (this->isUnhandledExternalFunction(callee) || isInternalFunctionThatReachUnhandledExternalFunction(callee)) {
      return false;
    }
  }

  return true;
}

template<class InstI, class InstJ>
void PDGAnalysis::addEdgeFromMemoryAlias (PDG *pdg, Function &F, AAResults &AA, InstI *instI, InstJ *instJ, DataDependenceType dataDependenceType) {
  auto must = false;

  /*
   * Query the LLVM alias analyses.
   */
  switch (AA.alias(MemoryLocation::get(instI), MemoryLocation::get(instJ))) {
    case NoAlias:
      return ;
    case PartialAlias:
    case MayAlias:
      break;
    case MustAlias:
      pdg->addEdge(instI, instJ)->setMemMustType(true, true, dataDependenceType);
      return ;
  }

  /*
   * Check other alias analyses
   *
   * Check if SVF is enabled.
   */
  if (this->disableSVF){

    /*
     * SVF is disabled.
     */

  } else {

    /*
     * SVF is enabled, so let's use it.
     */
    switch (NoelleSVFIntegration::alias(MemoryLocation::get(instI), MemoryLocation::get(instJ))) {
      case NoAlias:
        return;
      case PartialAlias:
      case MayAlias:
        break;
      case MustAlias:
        must = true;
        break;
    }
  }

  /*
   * There is a dependence.
   */
  pdg->addEdge(instI, instJ)->setMemMustType(true, must, dataDependenceType);

  return ;
}

AliasResult PDGAnalysis::doTheyAlias (PDG *pdg, Function &F, AAResults &AA, Value *instI, Value *instJ){
  auto must = false;

  /*
   * Query the LLVM alias analyses.
   */
  switch (AA.alias(instI, instJ)) {
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
  if (this->disableSVF){

    /*
     * SVF is disabled.
     */

  } else {

    /*
     * SVF is enabled, so let's use it.
     */
    switch (NoelleSVFIntegration::alias(instI, instJ)) {
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

}
