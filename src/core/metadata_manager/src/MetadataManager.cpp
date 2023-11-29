/*
 * Copyright 2021 - 2022  Simone Campanoni
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
#include "noelle/core/MetadataManager.hpp"

namespace arcana::noelle {

MetadataManager::MetadataManager(Module &M) : program{ M } {

  /*
   * Collect variable metadata.
   */
  for (auto &F : M) {
    for (auto &inst : instructions(F)) {
      auto call = dyn_cast<CallInst>(&inst);
      if (call == nullptr) {
        continue;
      }
      auto callee = call->getCalledFunction();
      if (callee == nullptr) {
        continue;
      }
      if (callee->getName().compare("llvm.var.annotation") != 0) {
        continue;
      }
      auto ptr = dyn_cast<Instruction>(call->getOperand(0));
      if (ptr == nullptr) {
        continue;
      }
      if (auto aliasPtr = dyn_cast<BitCastInst>(ptr)) {
        auto origPtr = aliasPtr->getOperand(0);
        ptr = dyn_cast<Instruction>(origPtr);
      }
      if (ptr == nullptr) {
        continue;
      }
      auto var = dyn_cast<AllocaInst>(ptr);
      if (var == nullptr) {
        continue;
      }
      auto gep = dyn_cast<GetElementPtrInst>(call->getOperand(1));
      if (gep != nullptr) {
        auto annoteStr = dyn_cast<GlobalVariable>(gep->getOperand(0));
        if (annoteStr == nullptr) {
          continue;
        }
        auto data =
            dyn_cast<ConstantDataSequential>(annoteStr->getInitializer());
        if (data == nullptr) {
          continue;
        }
        if (data->isString()) {
          this->varMetadata[var].insert(data->getAsString().str());
        }
      }
    }
  }

  /*
   * Collect metadata attached to functions.
   */
  auto globalArray = M.getGlobalVariable("llvm.global.annotations");
  if (globalArray != nullptr) {
    for (auto &globalArrayEntry : globalArray->operands()) {
      auto globalArrayEntryConstant = dyn_cast<ConstantArray>(globalArrayEntry);
      if (globalArrayEntryConstant == nullptr) {
        continue;
      }
      for (auto &globalArrayEntryOperand :
           globalArrayEntryConstant->operands()) {

        /*
         * Fetch the annotation.
         */
        auto globalArrayEntryOperandStruct =
            dyn_cast<ConstantStruct>(globalArrayEntryOperand);
        if (globalArrayEntryOperandStruct == nullptr) {
          continue;
        }
        if (globalArrayEntryOperandStruct->getNumOperands() < 2) {
          continue;
        }
        auto annotationVariable = dyn_cast<GlobalVariable>(
            globalArrayEntryOperandStruct->getOperand(1)->getOperand(0));
        if (annotationVariable == nullptr) {
          continue;
        }
        auto A = dyn_cast<ConstantDataArray>(annotationVariable->getOperand(0));
        if (A == nullptr) {
          continue;
        }
        auto AS = A->getAsString();

        /*
         * Attach the annotation.
         *
         * Case 0: function
         */
        auto annotatedFunction = dyn_cast<Function>(
            globalArrayEntryOperandStruct->getOperand(0)->getOperand(0));
        if (annotatedFunction != nullptr) {
          this->functionMetadata[annotatedFunction].insert(AS.str());
          continue;
        }

        /*
         * Case 1: global
         */
        auto annotatedGlobal = dyn_cast<GlobalVariable>(
            globalArrayEntryOperandStruct->getOperand(0)->getOperand(0));
        if (annotatedGlobal != nullptr) {
          this->globalMetadata[annotatedGlobal].insert(AS.str());
          continue;
        }
      }
    }
  }

  return;
}

bool MetadataManager::doesHaveMetadata(LoopStructure *loop,
                                       const std::string &metadataName) {

  /*
   * Check if we have already cached the metadata.
   */
  auto loopEntriesIt = this->metadata.find(loop);
  if (loopEntriesIt != this->metadata.end()) {
    const auto loopEntriesPair = &*loopEntriesIt;
    const auto &loopEntries = loopEntriesPair->second;
    if (loopEntries.find(metadataName) != loopEntries.end()) {

      /*
       * We have already cached the metadata.
       */
      return true;
    }
  }

  /*
   * We did not have cached the metadata.
   * Check the IR.
   *
   * Fetch the header terminator.
   */
  auto headerTerm = loop->getHeader()->getTerminator();

  /*
   * Check if the metadata exists.
   */
  auto metaNode = headerTerm->getMetadata(metadataName);
  if (!metaNode) {
    return false;
  }

  /*
   * Cache the metadata since it exists.
   */
  auto metaString = cast<MDString>(metaNode->getOperand(0))->getString();
  this->metadata[loop][metadataName] =
      new MetadataEntry(metadataName, metaString.str());

  return true;
}

std::string MetadataManager::getMetadata(LoopStructure *loop,
                                         const std::string &metadataName) {

  /*
   * Check if the metadata exists.
   */
  if (!this->doesHaveMetadata(loop, metadataName)) {
    return "";
  }

  auto loopEntries = this->metadata.at(loop);
  auto metadataEntry = loopEntries.at(metadataName);
  return metadataEntry->getValue();
}

void MetadataManager::setMetadata(LoopStructure *loop,
                                  const std::string &metadataName,
                                  const std::string &metadataValue) {

  /*
   * Fetch the header terminator.
   */
  auto headerTerm = loop->getHeader()->getTerminator();

  /*
   * Check if the metadata node already exists.
   */
  auto metaNode = headerTerm->getMetadata(metadataName);
  if (!metaNode) {
    errs() << "MetadataManager::setMetadata: ERROR = the metadata \""
           << metadataName << "\" does not exists in the loop " << *headerTerm
           << "\n";
    abort();
  }

  /*
   * Set the metadata
   */
  auto &cxt = headerTerm->getContext();
  auto s = MDString::get(cxt, metadataValue);
  auto n = MDNode::get(cxt, s);
  headerTerm->setMetadata(metadataName, n);

  /*
   * Add the metadata to our mapping.
   */
  this->addMetadata(loop, metadataName);

  return;
}

void MetadataManager::deleteMetadata(LoopStructure *loop,
                                     const std::string &metadataName,
                                     const std::string &metadataValue) {

  /*
   * Fetch the header terminator.
   */
  auto headerTerm = loop->getHeader()->getTerminator();

  /*
   * Check if the metadata node already exists.
   */
  auto metaNode = headerTerm->getMetadata(metadataName);
  if (!metaNode) {
    errs() << "MetadataManager::deleteMetadata: ERROR = the metadata \""
           << metadataName << "\" does not exists in the loop " << *headerTerm
           << "\n";
    abort();
  }

  /*
   * Delete the metadata
   */
  headerTerm->setMetadata(metadataName, nullptr);

  /*
   * Remove the metadata from our mapping.
   */
  auto loopEntries = this->metadata[loop];
  delete loopEntries[metadataName];
  loopEntries.erase(metadataName);

  return;
}

void MetadataManager::addMetadata(LoopStructure *loop,
                                  const std::string &metadataName,
                                  const std::string &metadataValue) {

  /*
   * Fetch the header terminator.
   */
  auto headerTerm = loop->getHeader()->getTerminator();

  /*
   * Check if the metadata node already exists.
   */
  auto metaNode = headerTerm->getMetadata(metadataName);
  if (metaNode) {
    errs() << "MetadataManager::addMetadata: ERROR = the metadata \""
           << metadataName << "\" already exists in the loop " << *headerTerm
           << "\n";
    abort();
  }

  /*
   * Create the metadata and add it to the IR.
   */
  auto &cxt = headerTerm->getContext();
  auto s = MDString::get(cxt, metadataValue);
  auto n = MDNode::get(cxt, s);
  headerTerm->setMetadata(metadataName, n);

  /*
   * Add the metadata to our mapping.
   */
  this->addMetadata(loop, metadataName);

  return;
}

void MetadataManager::addMetadata(LoopStructure *loop,
                                  const std::string &metadataName) {

  /*
   * Fetch the header terminator.
   */
  auto headerTerm = loop->getHeader()->getTerminator();

  /*
   * Fetch the metadata node.
   */
  auto metaNode = headerTerm->getMetadata(metadataName);
  if (!metaNode) {
    return;
  }

  /*
   * Fetch the string.
   */
  auto metaString = cast<MDString>(metaNode->getOperand(0))->getString();

  /*
   * Add the metadata.
   */
  auto loopEntries = this->metadata[loop];
  loopEntries[metadataName] = new MetadataEntry(metadataName, metaString.str());

  return;
}

} // namespace arcana::noelle
