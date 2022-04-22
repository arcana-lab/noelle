/*
 * Copyright 2021 - 2022  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/core/LoopTransformationsOptions.hpp"

namespace llvm::noelle {

LoopTransformationsManager::LoopTransformationsManager (
  uint32_t maxNumberOfCores,
  uint32_t chunkSize,
  std::unordered_set<LoopDependenceInfoOptimization> optimizations,
  bool enableLoopAwareDependenceAnalyses
  ) :   chunkSize{chunkSize}
      , maxCores{maxNumberOfCores}
      , _areLoopAwareAnalysesEnabled{enableLoopAwareDependenceAnalyses}
      , enabledOptimizations{optimizations}
  {

  return ;
}

LoopTransformationsManager::LoopTransformationsManager (
  const LoopTransformationsManager &other
  ){
  this->chunkSize = other.chunkSize;
  this->maxCores = other.maxCores;
  this->enabledTransformations = other.enabledTransformations;
  this->_areLoopAwareAnalysesEnabled = other._areLoopAwareAnalysesEnabled;

  return ;
}
     
uint32_t LoopTransformationsManager::getMaximumNumberOfCores (void) const {
  return this->maxCores;
}

uint32_t LoopTransformationsManager::getChunkSize (void) const {
  return this->chunkSize;
}

bool LoopTransformationsManager::isTransformationEnabled (Transformation transformation){
  auto exist = this->enabledTransformations.find(transformation) != this->enabledTransformations.end();

  return exist;
}

void LoopTransformationsManager::enableAllTransformations (void){
  for (int32_t i = Transformation::First; i <= Transformation::Last; i++){
    auto t = static_cast<Transformation>(i);
    this->enabledTransformations.insert(t);
  }

  return ;
}

void LoopTransformationsManager::disableTransformation (Transformation transformationToDisable){
  this->enabledTransformations.erase(transformationToDisable);

  return ;
}

bool LoopTransformationsManager::isOptimizationEnabled (LoopDependenceInfoOptimization optimization) const {
  auto enabled = this->enabledOptimizations.find(optimization) != this->enabledOptimizations.end();
  return enabled;
}
      
bool LoopTransformationsManager::areLoopAwareAnalysesEnabled (void) const {
  return this->_areLoopAwareAnalysesEnabled;
}

}
