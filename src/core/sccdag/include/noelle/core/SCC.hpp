/*
 * Copyright 2016 - 2023  Angelo Matni, Simone Campanoni
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
#ifndef NOELLE_SRC_CORE_SCCDAG_SCC_H_
#define NOELLE_SRC_CORE_SCCDAG_SCC_H_
#pragma once

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/DGBase.hpp"

namespace arcana::noelle {

/*
 * Strongly Connected Component
 */
class SCC : public DG<Value> {
public:
  /*
   * Constructors.
   */
  SCC(std::set<DGNode<Value> *> internalNodes);
  SCC(std::set<DGNode<Value> *> internalNodes,
      std::set<DGNode<Value> *> externalNodes);

  /*
   * Iterate over values inside the SCC until @funcToInvoke returns true or no
   * other one exists.
   */
  bool iterateOverValues(std::function<bool(Value *)> funcToInvoke);

  /*
   * Iterate over all values (internal and external) until @funcToInvoke returns
   * true or no other value exists. External nodes represent live-ins and
   * live-outs of the SCC.
   */
  bool iterateOverAllValues(std::function<bool(Value *)> funcToInvoke);

  /*
   * Iterate over instructions inside the SCC until @funcToInvoke returns true
   * or no other instruction exists.
   */
  bool iterateOverInstructions(std::function<bool(Instruction *)> funcToInvoke);

  /*
   * Return the instructions inside the SCC.
   * These are the same instructions that @iterateOverInstructions iterates
   * over.
   */
  std::set<Instruction *> getInstructions(void);

  /*
   * Iterate over all instructions (internal and external) until @funcToInvoke
   * returns true or no other instruction exists. External nodes represent
   * live-ins and live-outs of the SCC.
   */
  bool iterateOverAllInstructions(
      std::function<bool(Instruction *)> funcToInvoke);

  /*
   * Check if the SCC has cycles in it.
   */
  bool hasCycle(bool ignoreControlDep = false);

  /*
   * Return the number of instructions that compose the SCC.
   */
  int64_t numberOfInstructions(void) const;

  /*
   * Print
   */
  raw_ostream &print(raw_ostream &stream,
                     std::string prefixToUse = "",
                     int maxEdges = 15);

  /*
   * Print
   */
  raw_ostream &printMinimal(raw_ostream &stream, std::string prefixToUse = "");

  /*
   * Deconstructor.
   */
  ~SCC();

private:
  void copyNodesAndEdges(std::set<DGNode<Value> *> internalNodes,
                         std::set<DGNode<Value> *> externalNodes);
};

template <>
class DGEdge<SCC, SCC> : public DGEdge<SCC, Value> {
public:
  DGEdge(DGNode<SCC> *src, DGNode<SCC> *dst) : DGEdge<SCC, Value>(src, dst) {}
  DGEdge(const DGEdge<SCC, SCC> &oldEdge) : DGEdge<SCC, Value>(oldEdge) {}
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_SCCDAG_SCC_H_
