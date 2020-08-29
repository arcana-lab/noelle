/*
 * Copyright 2016 - 2020  Yian Su
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "Noelle.hpp"

using namespace llvm;

namespace llvm {
  
  struct LoopStats : public ModulePass {
    public:
      static char ID;

      LoopStats();
      virtual ~LoopStats();

      bool doInitialization(Module &M) override;
      void getAnalysisUsage(AnalysisUsage &AU) const override;
      bool runOnModule(Module &M) override;

    private:

      struct Stats {
        int64_t loopID = -1;
        int64_t numberOfIVs = 0;
        int64_t numberOfDynamicIVs = 0;
        int64_t isGovernedByIV = 0;
        int64_t numberOfDynamicGovernedIVs = 0;
        int64_t numberOfInvariants = 0;
        int64_t numberOfDynamicInvariants = 0;
        int64_t numberOfNodesInSCCDAG = 0;
        int64_t numberOfSCCs = 0;
        int64_t numberOfSequentialSCCs = 0;
        int64_t dynamicInstructionsOfSequentialSCCs = 0;
        uint64_t dynamicTotalInstructions = 0;

        Stats operator + (Stats const &obj) {
          Stats res;
          res.numberOfIVs = this->numberOfIVs + obj.numberOfIVs;
          res.numberOfDynamicIVs = this->numberOfDynamicIVs + obj.numberOfDynamicIVs;
          res.isGovernedByIV = this->isGovernedByIV + obj.isGovernedByIV;
          res.numberOfDynamicGovernedIVs = this->numberOfDynamicGovernedIVs + obj.numberOfDynamicGovernedIVs;
          res.numberOfInvariants = this->numberOfInvariants + obj.numberOfInvariants;
          res.numberOfDynamicInvariants = this->numberOfDynamicInvariants + obj.numberOfDynamicInvariants;
          res.numberOfNodesInSCCDAG = this->numberOfNodesInSCCDAG + obj.numberOfNodesInSCCDAG;
          res.numberOfSCCs = this->numberOfSCCs + obj.numberOfSCCs;
          res.numberOfSequentialSCCs = this->numberOfSequentialSCCs + obj.numberOfSequentialSCCs;
          res.dynamicInstructionsOfSequentialSCCs = this->dynamicInstructionsOfSequentialSCCs + obj.dynamicInstructionsOfSequentialSCCs;
          res.dynamicTotalInstructions = this->dynamicTotalInstructions + obj.dynamicTotalInstructions;

          return res;
        }
      };
      std::unordered_map<int, Stats *> statsByLoopAccordingToLLVM;
      std::unordered_map<int, Stats *> statsByLoopAccordingToNoelle;

      void collectStatsForLoops (Noelle &noelle, std::vector<LoopDependenceInfo *> const & loops);

      void collectStatsForLoop (Hot *profiles, int id, ScalarEvolution &SE, PDG *loopDG, Loop &llvmLoop);
      void collectStatsForLoop (Hot *profiles, LoopDependenceInfo &LDI);

      void collectStatsOnLLVMSCCs (Hot *profiles, PDG *loopDG, Stats *statsForLoop);
      void collectStatsOnLLVMIVs (Hot *profiles, ScalarEvolution &SE, Loop &llvmLoop, Stats *stats);
      void collectStatsOnLLVMInvariants (Hot *profiles, Loop &llvmLoop, Stats *stats);

      void collectStatsOnNoelleIVs (Hot *profiles, LoopDependenceInfo &LDI, Stats *stats);
      void collectStatsOnNoelleSCCs (Hot *profiles, LoopDependenceInfo &LDI, Stats *stats);
      void collectStatsOnNoelleInvariants (Hot *profiles, LoopDependenceInfo &LDI, Stats *stats);

      void collectStatsOnSCCDAG (Hot *profiles, SCCDAG *sccdag, SCCDAGAttrs *sccdagAttrs, LoopDependenceInfo *ldi, Stats *statsForLoop) ;

      void printPerLoopStats (Hot *profiles, Stats *stats);
      void printStatsHumanReadable (Hot *profiles);

  };

}
