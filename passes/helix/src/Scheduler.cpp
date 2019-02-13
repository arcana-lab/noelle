/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "HELIX.hpp"

using namespace llvm ;

void HELIX::squeezeSequentialSegments (
  LoopDependenceInfo *LDI,
  std::vector<SequentialSegment *> *sss
  ){
  //TODO: Finish
  return ;

  /*
   * TODO: For each SCC of each SS:
   * Ensure all externals are outside the sequential segment
   * whether before (incoming) or after (outgoing)
   * Why do that when you can just try to squeeze firsts and lasts
   * as close to each other as possible given their dependencies!
   */

  for (auto ss : *sss) {
    for (auto scc : ss->getSCCs()) {
      // Determine first and last instructions
      std::set<Value *> firsts, lasts;
      auto &backEdges = LDI->sccdagAttrs.interIterDeps[scc];
      for (auto edge : backEdges) {
        lasts.insert(edge->getOutgoingT());
        firsts.insert(edge->getIncomingT());
      }

      std::set<Value *> extractBefore, extractAfter;

      for (auto externalPair : scc->externalNodePairs()) {
        auto externalV = externalPair.first;
        if (externalPair.second->numOutgoingEdges() > 0) {
          // Confirm the external is before the first instructions in the SCC
          auto executesBefore = false;
          for (auto first : firsts) {
            // TODO: Determine execution order between first and externalV
          }
          if (!executesBefore) extractBefore.insert(externalV);
        } else {
          // Confirm the external is after the last instructions in the SCC
          auto executesAfter = false;
          for (auto last : lasts) {
            // TODO: Determine execution order between last and externalV
          }
          if (!executesAfter) extractAfter.insert(externalV);
        }
      }
    }
  }
}

void HELIX::scheduleSequentialSegments (
  LoopDependenceInfo *LDI,
  std::vector<SequentialSegment *> *sss
  ){
  //TODO

  return ;
}
