/*
 * Copyright 2016 - 2021  Angelo Matni, Simone Campanoni
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
#ifndef NOELLE_SRC_CORE_PDG_H_
#define NOELLE_SRC_CORE_PDG_H_

#include "arcana/noelle/core/SystemHeaders.hpp"
#include "noelle/core/DGBase.hpp"

namespace arcana::noelle {

/*
 * Program Dependence Graph.
 */
class PDG : public DG<Value> {
public:
  /*
   * Constructor:
   * Add all instructions included in the module M as nodes to the PDG.
   */
  PDG(Module &M);

  /*
   * Constructor:
   * Add all instructions included in the function F as nodes to the PDG.
   */
  PDG(Function &F);

  /*
   * Constructor:
   * Add all instructions included in the loop only.
   */
  PDG(Loop *loop);

  /*
   * Constructor:
   * Add only the instructions given as parameter.
   */
  PDG(std::vector<Value *> &values);

  PDG() = delete;

  /*
   * Return the number of instructions included in the PDG.
   */
  uint64_t getNumberOfInstructionsIncluded(void) const;

  /*
   * Return the number of dependences of the PDG including dependences that
   * connect instructions outside the PDG.
   */
  uint64_t getNumberOfDependencesBetweenInstructions(void) const;

  /*
   * Fetch dependences between two values/instructions.
   */
  std::unordered_set<DGEdge<Value, Value> *> getDependences(Value *v1,
                                                            Value *v2);

  /*
   * Iterator: iterate over the instructions that depend on @param fromValue
   * until @functionToInvokePerDependence returns true or there is no other
   * dependence to iterate.
   *
   * This means there is an edge from @param fromValue to @param toValue of the
   * type specified by the other parameters. For each of this edge, the function
   * @param functionToInvokePerDependence is invoked.
   *
   * This function returns true if the iteration ends earlier.
   * It returns false otherwise.
   */
  bool iterateOverDependencesFrom(
      Value *fromValue,
      bool includeControlDependences,
      bool includeMemoryDataDependences,
      bool includeRegisterDataDependences,
      std::function<bool(Value *to, DGEdge<Value, Value> *dependence)>
          functionToInvokePerDependence);

  /*
   * Iterator: iterate over the instructions that @param toValue depends from
   * until @functionToInvokePerDependence returns true or there is no other
   * dependence to iterate.
   *
   * This means there is an edge from @param fromValue to @param toValue of the
   * type specified by the other parameters. For each of this edge, the function
   * @param functionToInvokePerDependence is invoked.
   *
   * This function returns true if the iteration ends earlier.
   * It returns false otherwise.
   */
  bool iterateOverDependencesTo(
      Value *toValue,
      bool includeControlDependences,
      bool includeMemoryDataDependences,
      bool includeRegisterDataDependences,
      std::function<bool(Value *fromValue, DGEdge<Value, Value> *dependence)>
          functionToInvokePerDependence);

  /*
   * Creating Program Dependence Subgraphs
   */
  PDG *createFunctionSubgraph(Function &F);
  PDG *createLoopsSubgraph(Loop *loop);

  PDG *createSubgraphFromValues(std::vector<Value *> &valueList,
                                bool linkToExternal);
  PDG *createSubgraphFromValues(
      std::vector<Value *> &valueList,
      bool linkToExternal,
      std::unordered_set<DGEdge<Value, Value> *> edgesToIgnore);

  PDG *clone(bool includeExternalNodes);
  std::vector<Value *> getSortedValues(void);

  std::vector<DGEdge<Value, Value> *> getSortedDependences(void);

  /*
   * Destructor
   */
  ~PDG();

protected:
  void addNodesOf(Function &F);

  void setEntryPointAt(Function &F);

  void copyEdgesInto(PDG *newPDG, bool linkToExternal);

  void copyEdgesInto(
      PDG *newPDG,
      bool linkToExternal,
      std::unordered_set<DGEdge<Value, Value> *> const &edgesToIgnore);
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_PDG_H_
