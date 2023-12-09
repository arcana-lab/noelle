/*
 * Copyright 2019 - 2023 Simone Campanoni
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
#ifndef NOELLE_SRC_CORE_CALL_GRAPH_CALLGRAPHEDGE_H_
#define NOELLE_SRC_CORE_CALL_GRAPH_CALLGRAPHEDGE_H_
#pragma once

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/CallGraphNode.hpp"

namespace arcana::noelle {

class CallGraphEdge {
public:
  CallGraphEdge() = default;

  void setMust(void);

  void unsetMust(void);

  bool isAMustCall(void) const;

  CallGraphFunctionNode *getCallee(void) const;

  virtual void print(void) = 0;

  virtual ~CallGraphEdge();

protected:
  bool isMust;
  CallGraphFunctionNode *callee;
};

class CallGraphInstructionFunctionEdge : public CallGraphEdge {
public:
  CallGraphInstructionFunctionEdge(CallGraphInstructionNode *caller,
                                   CallGraphFunctionNode *callee,
                                   bool isMust);

  CallGraphInstructionNode *getCaller(void) const;

  void print(void) override;

  virtual ~CallGraphInstructionFunctionEdge();

private:
  CallGraphInstructionNode *caller;
};

class CallGraphFunctionFunctionEdge : public CallGraphEdge {
public:
  CallGraphFunctionFunctionEdge(CallGraphFunctionNode *caller,
                                CallGraphFunctionNode *callee,
                                bool isMust);

  CallGraphFunctionNode *getCaller(void) const;

  std::unordered_set<CallGraphInstructionFunctionEdge *> getSubEdges(
      void) const;

  uint64_t getNumberOfSubEdges(void) const;

  void addSubEdge(CallGraphInstructionFunctionEdge *subEdge);

  void removeSubEdge(CallGraphInstructionFunctionEdge *subEdge);

  void print(void) override;

  virtual ~CallGraphFunctionFunctionEdge();

private:
  CallGraphFunctionNode *caller;
  std::unordered_set<CallGraphInstructionFunctionEdge *> subEdges;
  std::unordered_map<Instruction *, CallGraphInstructionFunctionEdge *>
      subEdgesMap;
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_CALL_GRAPH_CALLGRAPHEDGE_H_
