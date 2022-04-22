/*
 * Copyright 2021 - 2022  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/Transformations.hpp"

namespace llvm::noelle {

  class LoopTransformationsManager {
    public:
      LoopTransformationsManager (
        uint32_t maxNumberOfCores,
        uint32_t chunkSize,
        std::unordered_set<LoopDependenceInfoOptimization> optimizations,
        bool enableLoopAwareDependenceAnalyses
        );

      LoopTransformationsManager (
        const LoopTransformationsManager &other
        );

      uint32_t getChunkSize (void) const ;

      uint32_t getMaximumNumberOfCores (void) const ;

      /*
       * Check whether a transformation is enabled.
       */
      bool isTransformationEnabled (Transformation transformation);

      /*
       * Enable all transformations.
       */
      void enableAllTransformations (void);

      /*
       * Disable all transformations.
       */
      void disableTransformation (Transformation transformationToDisable);

      /*
       * Check whether an optimization is enabled
       */
      bool isOptimizationEnabled (LoopDependenceInfoOptimization optimization) const ;

      bool areLoopAwareAnalysesEnabled (void) const ;

    private:
      uint32_t chunkSize;
      uint32_t maxCores;
      std::set<Transformation> enabledTransformations;  /* Transformations enabled. */
      std::unordered_set<LoopDependenceInfoOptimization> enabledOptimizations;  /* Optimizations enabled. */
      bool _areLoopAwareAnalysesEnabled;
  };

}
