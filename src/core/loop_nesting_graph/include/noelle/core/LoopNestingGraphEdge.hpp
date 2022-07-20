/*
 * Copyright 2019 - 2020 Simone Campanoni
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
#pragma once

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/LoopNestingGraphNode.hpp"

namespace llvm::noelle {
class LoopNestingGraphLoopNode;
class LoopNestingGraphInstructionNode;

class LoopNestingGraphEdge {
public:
  LoopNestingGraphEdge() = default;

  void setMust(void);

  void unsetMust(void);

  bool isAMustEdge(void) const;

  LoopNestingGraphLoopNode *getChild(void) const;

  virtual void print(void) = 0;
  virtual ~LoopNestingGraphEdge() = default;

protected:
  bool isMust;
  LoopNestingGraphLoopNode *child;
};

class LoopNestingGraphInstructionLoopEdge : public LoopNestingGraphEdge {
public:
  LoopNestingGraphInstructionLoopEdge(LoopNestingGraphInstructionNode *caller,
                                      LoopNestingGraphLoopNode *child,
                                      bool isMust);

  LoopNestingGraphInstructionNode *getCaller(void) const;

  void print(void) override;

private:
  LoopNestingGraphInstructionNode *caller;
};

class LoopNestingGraphLoopLoopEdge : public LoopNestingGraphEdge {
public:
  LoopNestingGraphLoopLoopEdge(LoopNestingGraphLoopNode *parent,
                               LoopNestingGraphLoopNode *child,
                               bool isMust);

  LoopNestingGraphLoopNode *getParent(void) const;

  std::unordered_set<LoopNestingGraphInstructionLoopEdge *> getSubEdges(
      void) const;

  void addSubEdge(LoopNestingGraphInstructionLoopEdge *subEdge);

  void print(void) override;

private:
  LoopNestingGraphLoopNode *parent;
  std::unordered_set<LoopNestingGraphInstructionLoopEdge *> subEdges;
  std::unordered_map<Instruction *, LoopNestingGraphInstructionLoopEdge *>
      subEdgesMap;
};

} // namespace llvm::noelle
