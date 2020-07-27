/*
 * Copyright 2019 - 2020 Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "SystemHeaders.hpp"
#include "SCCCAG.hpp"

using namespace llvm;
using namespace noelle;

noelle::SCCCAGNode_SCC::SCCCAGNode_SCC (std::unordered_set<CallGraphNode *> const &nodes)
  : nodes{nodes}
  {
  return ;
}
        
bool noelle::SCCCAGNode_SCC::isAnSCC (void) const {
  return true;
}

SCCCAGNode_Function::SCCCAGNode_Function (Function & F) 
  : func{F}
  {
  return ;
}

bool SCCCAGNode_Function::isAnSCC (void) const {
  return false;
}

noelle::SCCCAG::SCCCAG (noelle::CallGraph *cg){
  //TODO
  return ;
}

SCCCAGNode * SCCCAG::getNode (CallGraphNode *n) const {
  if (this->nodes.find(n) == this->nodes.end()){
    return nullptr;
  }

  auto node = this->nodes.at(n);
  return node;
}
