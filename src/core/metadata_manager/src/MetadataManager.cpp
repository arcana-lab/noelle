/*
 * Copyright 2021 - 2022  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/core/MetadataManager.hpp"

namespace llvm::noelle {

MetadataManager::MetadataManager (Module &M)
  : program{M}
{
  return ;
}

bool MetadataManager::doesHaveMetadata (
  LoopStructure *loop,
  const std::string &metadataName
  ) {

  /*
   * Check if we have already cached the metadata.
   */
  auto loopEntriesIt = this->metadata.find(loop);
  if (loopEntriesIt != this->metadata.end()){
    const auto loopEntriesPair = &*loopEntriesIt;
    const auto & loopEntries = loopEntriesPair->second;
    if (loopEntries.find(metadataName) != loopEntries.end()){

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
  if (!metaNode){
    return false;
  }

  /*
   * Cache the metadata since it exists.
   */
  auto metaString = cast<MDString>(metaNode->getOperand(0))->getString();
  this->metadata[loop][metadataName] = new MetadataEntry(metadataName, metaString);

  return true;
}

std::string MetadataManager::getMetadata (
  LoopStructure *loop,
  const std::string &metadataName
  ) {

  /*
   * Check if the metadata exists.
   */
  if (!this->doesHaveMetadata(loop, metadataName)){
    return "";
  }

  auto loopEntries = this->metadata.at(loop);
  auto metadataEntry = loopEntries.at(metadataName);
  return metadataEntry->getValue();
}

void MetadataManager::setMetadata (
  LoopStructure *loop,
  const std::string &metadataName, 
  const std::string &metadataValue
  ) {

  /*
   * Fetch the header terminator.
   */
  auto headerTerm = loop->getHeader()->getTerminator();

  /*
   * Check if the metadata node already exists.
   */
  auto metaNode = headerTerm->getMetadata(metadataName);
  if (!metaNode){
    errs() << "MetadataManager::setMetadata: ERROR = the metadata \"" << metadataName << "\" does not exists in the loop " << *headerTerm << "\n";
    abort();
  }

  /*
   * Set the metadata
   */
  auto& cxt = headerTerm->getContext();
  auto s = MDString::get(cxt, metadataValue);
  auto n = MDNode::get(cxt, s);
  headerTerm->setMetadata(metadataName, n);

  /*
   * Add the metadata to our mapping.
   */
  this->addMetadata(loop, metadataName);

  return ;
}

void MetadataManager::deleteMetadata (
  LoopStructure *loop,
  const std::string &metadataName, 
  const std::string &metadataValue
  ) {

  /*
   * Fetch the header terminator.
   */
  auto headerTerm = loop->getHeader()->getTerminator();

  /*
   * Check if the metadata node already exists.
   */
  auto metaNode = headerTerm->getMetadata(metadataName);
  if (!metaNode){
    errs() << "MetadataManager::deleteMetadata: ERROR = the metadata \"" << metadataName << "\" does not exists in the loop " << *headerTerm << "\n";
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

  return ;
}

void MetadataManager::addMetadata (
  LoopStructure *loop,
  const std::string &metadataName, 
  const std::string &metadataValue
  ) {

  /*
   * Fetch the header terminator.
   */
  auto headerTerm = loop->getHeader()->getTerminator();

  /*
   * Check if the metadata node already exists.
   */
  auto metaNode = headerTerm->getMetadata(metadataName);
  if (metaNode){
    errs() << "MetadataManager::addMetadata: ERROR = the metadata \"" << metadataName << "\" already exists in the loop " << *headerTerm << "\n";
    abort();
  }

  /*
   * Create the metadata and add it to the IR.
   */
  auto& cxt = headerTerm->getContext();
  auto s = MDString::get(cxt, metadataValue);
  auto n = MDNode::get(cxt, s);
  headerTerm->setMetadata(metadataName, n);

  /*
   * Add the metadata to our mapping.
   */
  this->addMetadata(loop, metadataName);

  return ;
}

void MetadataManager::addMetadata (
  LoopStructure *loop,
  const std::string &metadataName
  ){

  /*
   * Fetch the header terminator.
   */
  auto headerTerm = loop->getHeader()->getTerminator();

  /*
   * Fetch the metadata node.
   */
  auto metaNode = headerTerm->getMetadata(metadataName);
  if (!metaNode){
    return ;
  }

  /*
   * Fetch the string.
   */
  auto metaString = cast<MDString>(metaNode->getOperand(0))->getString();

  /*
   * Add the metadata.
   */
  auto loopEntries = this->metadata[loop];
  loopEntries[metadataName] = new MetadataEntry(metadataName, metaString);

  return ;
}

}
