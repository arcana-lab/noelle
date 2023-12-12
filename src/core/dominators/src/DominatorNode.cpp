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
#include "noelle/core/DominatorNode.hpp"

namespace arcana::noelle {

DominatorNode::DominatorNode(const DTAliases::Node &node)
  : B{ node.getBlock() },
    level{ node.getLevel() },
    parent{ nullptr },
    children{},
    iDom{ nullptr } {

  return;
}

DominatorNode::DominatorNode(const DominatorNode &node)
  : B{ node.getBlock() },
    level{ node.getLevel() },
    parent{ nullptr },
    children{},
    iDom{ nullptr } {

  return;
}

raw_ostream &DominatorNode::print(raw_ostream &stream, std::string prefix) {
  stream << prefix << "Block: ";
  if (getBlock())
    getBlock()->printAsOperand(stream);
  else
    stream << "null";
  stream << " Level: " << getLevel() << " Parent: ";
  if (getParent() && getParent()->getBlock())
    getParent()->getBlock()->printAsOperand(stream);
  else
    stream << "null";
  stream << " I Dom: ";
  if (getIDom() && getIDom()->getBlock())
    getIDom()->getBlock()->printAsOperand(stream);
  else
    stream << "null";
  stream << "\n" << prefix << "Children: ";
  for (auto child : getChildren()) {
    child->getBlock()->printAsOperand(stream << "\t");
  }
  return stream << "\n";
}

BasicBlock *DominatorNode::getBlock(void) const {
  return B;
}

DominatorNode *DominatorNode::getParent(void) {
  return parent;
}

std::vector<DominatorNode *> DominatorNode::getChildren(void) {
  return children;
}

unsigned DominatorNode::getLevel(void) const {
  return level;
}

DominatorNode *DominatorNode::getIDom(void) {
  return iDom;
}

} // namespace arcana::noelle
