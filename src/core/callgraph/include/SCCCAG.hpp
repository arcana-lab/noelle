/*
 * Copyright 2019 - 2020 Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "SystemHeaders.hpp"
#include "CallGraph.hpp"
#include "CallGraphTraits.hpp"

namespace llvm::noelle {
  class CallGraph;

  class SCCCAGNode {
    public:
      SCCCAGNode() = default;

      virtual bool isAnSCC (void) const = 0;
  };

  class SCCCAGNode_SCC : public SCCCAGNode {
    public:
      SCCCAGNode_SCC (std::unordered_set<CallGraphNode *> const &nodes);

      bool isAnSCC (void) const override ;

    private:
      std::unordered_set<CallGraphNode *> nodes;
  };

  class SCCCAGNode_Function : public SCCCAGNode {
    public:
      SCCCAGNode_Function (Function &F) ;

      bool isAnSCC (void) const override ;

    private:
      Function &func;
  };

  class SCCCAG {
    public:
      SCCCAG (noelle::CallGraph *cg);

      SCCCAG () = delete ;

      SCCCAGNode * getNode (CallGraphNode *n) const ;

    private:
      std::unordered_map<CallGraphNode *, SCCCAGNode *> nodes;
  };

}
