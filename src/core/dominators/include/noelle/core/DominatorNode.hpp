/*
 * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni
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
#ifndef NOELLE_SRC_CORE_DOMINATORS_DOMINATORNODE_H_
#define NOELLE_SRC_CORE_DOMINATORS_DOMINATORNODE_H_

#include "llvm/Analysis/PostDominators.h"
#include "noelle/core/SystemHeaders.hpp"

namespace arcana::noelle {

namespace DTAliases {
using Node = DomTreeNodeBase<BasicBlock>;
}

class DominatorNode {
public:
  DominatorNode(const DTAliases::Node &node);
  DominatorNode(const DominatorNode &node);

  BasicBlock *getBlock(void) const;
  DominatorNode *getParent(void) const;
  std::vector<DominatorNode *> getChildren(void) const;
  uint32_t getLevel(void) const;
  DominatorNode *getIDom(void);

  raw_ostream &print(raw_ostream &stream, std::string prefixToUse = "");

  friend class DominatorForest;

private:
  BasicBlock *B;
  uint32_t level;

  DominatorNode *parent;
  std::vector<DominatorNode *> children;
  DominatorNode *iDom;
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_DOMINATORS_DOMINATORNODE_H_
