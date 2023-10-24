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

namespace llvm::noelle {

bool MetadataManager::doesHaveMetadata(Instruction *inst,
                                       const std::string &metadataName) {

  /*
   * Check if the metadata exists.
   */
  auto metaNode = inst->getMetadata(metadataName);
  if (!metaNode) {
    return false;
  }

  return true;
}

std::string MetadataManager::getMetadata(Instruction *inst,
                                         const std::string &metadataName) {

  /*
   * Get the metadata.
   */
  auto metaNode = inst->getMetadata(metadataName);
  if (metaNode) {
    errs() << "MetadataManager::getMetadata: ERROR = the metadata \""
           << metadataName << "\" already exists in the instruction " << *inst
           << "\n";
    abort();
  }

  /*
   * Get the string.
   */
  auto metaString = cast<MDString>(metaNode->getOperand(0))->getString();

  return metaString.str();
}

void MetadataManager::addMetadata(Instruction *inst,
                                  const std::string &metadataName,
                                  const std::string &metadataValue) {

  /*
   * Get the metadata.
   */
  auto metaNode = inst->getMetadata(metadataName);
  if (metaNode) {
    errs() << "MetadataManager::addMetadata: ERROR = the metadata \""
           << metadataName << "\" already exists in the instruction " << *inst
           << "\n";
    abort();
  }

  /*
   * Create the metadata value.
   */
  auto &cxt = this->program.getContext();
  auto s = MDString::get(cxt, metadataValue);
  auto n = MDNode::get(cxt, s);

  /*
   * Add the metadata to the instruction
   */
  inst->setMetadata(metadataName, n);

  return;
}

void MetadataManager::setMetadata(Instruction *inst,
                                  const std::string &metadataName,
                                  const std::string &metadataValue) {

  /*
   * Check if the metadata node already exists.
   */
  auto metaNode = inst->getMetadata(metadataName);
  if (!metaNode) {
    errs() << "MetadataManager::setMetadata: ERROR = the metadata \""
           << metadataName << "\" does not exists in the instruction " << *inst
           << "\n";
    abort();
  }

  /*
   * Set the metadata
   */
  auto &cxt = this->program.getContext();
  auto s = MDString::get(cxt, metadataValue);
  auto n = MDNode::get(cxt, s);
  inst->setMetadata(metadataName, n);

  return;
}

void MetadataManager::deleteMetadata(Instruction *inst,
                                     const std::string &metadataName,
                                     const std::string &metadataValue) {

  /*
   * Check if the metadata node already exists.
   */
  auto metaNode = inst->getMetadata(metadataName);
  if (!metaNode) {
    errs() << "MetadataManager::deleteMetadata: ERROR = the metadata \""
           << metadataName << "\" does not exists in the instruction " << *inst
           << "\n";
    abort();
  }

  /*
   * Delete the metadata
   */
  inst->setMetadata(metadataName, nullptr);

  return;
}

} // namespace llvm::noelle
