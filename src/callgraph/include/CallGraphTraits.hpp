/*
 * Copyright 2019 - 2020 Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "SystemHeaders.hpp"
#include "CallGraph.hpp"

namespace llvm {
  namespace noelle {

    class CallGraphNodeWrapper ;

    class CallGraphWrapper {
      public:
        CallGraphWrapper (CallGraph *graph) ;

        ~CallGraphWrapper () ;

        using NodeRef = CallGraphNodeWrapper *;
        using ChildIteratorType = typename std::vector<NodeRef>::iterator;
        using nodes_iterator = typename std::unordered_set<NodeRef>::iterator;

        nodes_iterator nodes_begin() { return nodes.begin(); }

        nodes_iterator nodes_end() { return nodes.end(); }

        CallGraph *wrappedGraph;
        NodeRef entryNode;
        std::unordered_set<NodeRef> nodes;
    };

    class CallGraphNodeWrapper {
      public:
        CallGraphNodeWrapper (CallGraphFunctionNode *node) : wrappedNode{node} {}

        using NodeRef = CallGraphNodeWrapper *;
        using ChildIteratorType = typename std::vector<NodeRef>::iterator;

        ChildIteratorType child_begin() { return outgoingNodeInstances.begin(); }

        ChildIteratorType child_end() { return outgoingNodeInstances.end(); }

        CallGraphFunctionNode *wrappedNode;
        std::vector<NodeRef> outgoingNodeInstances;
    };

    struct CallGraphWrapperTraits {
      using NodeRef = CallGraphNodeWrapper *;
      using ChildIteratorType = typename std::vector<NodeRef>::iterator;
      using nodes_iterator = typename std::unordered_set<NodeRef>::iterator;

      static NodeRef getEntryNode(CallGraphWrapper *graph) { return graph->entryNode; }

      static nodes_iterator nodes_begin(CallGraphWrapper *graph) { return graph->nodes_begin(); }

      static nodes_iterator nodes_end(CallGraphWrapper *graph) { return graph->nodes_end(); }

      static ChildIteratorType child_begin(NodeRef node) { return node->child_begin(); }

      static ChildIteratorType child_end(NodeRef node) { return node->child_end(); }
    };

  }

  template<> struct GraphTraits<noelle::CallGraphWrapper *> : public noelle::CallGraphWrapperTraits {};

}
