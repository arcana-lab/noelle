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
#include "noelle/core/SubCFGs.hpp"

using namespace llvm;
using namespace llvm::noelle;

SubCFGs::SubCFGs(Module &M) {
  for (auto &F : M) {
    for (auto &B : F) {
      addNode(&B, true);
    }
  }

  connectBasicBlockNodes();
}

SubCFGs::SubCFGs(Function &F) {
  for (auto &B : F) {
    addNode(&B, true);
  }

  connectBasicBlockNodes();
}

SubCFGs::SubCFGs(Loop *loop) {
  for (auto &B : loop->blocks()) {
    addNode(B, true);
  }

  connectBasicBlockNodes();
}

SubCFGs::SubCFGs(std::set<BasicBlock *> &bbs) {
  for (auto B : bbs) {
    addNode(B, true);
  }

  connectBasicBlockNodes();
}

void SubCFGs::connectBasicBlockNodes() {
  std::set<DGNode<BasicBlock> *> nodes(begin_nodes(), end_nodes());
  for (auto node : nodes) {
    auto bb = node->getT();
    for (auto succBB : successors(bb)) {
      fetchOrAddNode(succBB, false);
      addEdge(bb, succBB);
    }
  }
}
