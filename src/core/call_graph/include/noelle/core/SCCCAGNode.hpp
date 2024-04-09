/*
 * Copyright 2019 - 2024  Simone Campanoni
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
#ifndef NOELLE_SRC_CORE_CALL_GRAPH_SCCCAGNODE_H_
#define NOELLE_SRC_CORE_CALL_GRAPH_SCCCAGNODE_H_

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/CallGraph.hpp"

namespace arcana::noelle {

class SCCCAGNode {
public:
  SCCCAGNode();

  uint64_t getID(void) const;

  virtual bool isAnSCC(void) const = 0;

  virtual ~SCCCAGNode();

protected:
  uint64_t ID;
};

class SCCCAGNode_SCC : public SCCCAGNode {
public:
  SCCCAGNode_SCC(std::unordered_set<CallGraphFunctionNode *> const &nodes);

  bool isAnSCC(void) const override;

  std::unordered_set<CallGraphFunctionNode *> getInternalNodes(void) const;

  virtual ~SCCCAGNode_SCC();

private:
  std::unordered_set<CallGraphFunctionNode *> nodes;
};

class SCCCAGNode_Function : public SCCCAGNode {
public:
  SCCCAGNode_Function(CallGraphFunctionNode *n);

  bool isAnSCC(void) const override;

  CallGraphFunctionNode *getNode(void) const;

  virtual ~SCCCAGNode_Function();

private:
  CallGraphFunctionNode *node;
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_CALL_GRAPH_SCCCAG_H_
