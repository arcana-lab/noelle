#define DEBUG_TYPE "talkdown"

#include "Node.hpp"

#include "llvm/Support/Debug.h"

#include <algorithm>

using namespace llvm;

namespace AutoMP
{
  unsigned int Node::how_many = 0;

  void Node::addAnnotations(AnnotationSet &&as)

  {
    annotations.insert( as.begin(), as.end() );
  }

  bool Node::containsAnnotationWithKey(std::string s) const
  {
    for ( auto &a : annotations )
      if ( !a.getKey().compare( s ) )
        return true;
    return false;
  }

  AnnotationSet Node::getRealAnnotations(void) const
  {
    using namespace std;
    AnnotationSet as;
    for ( auto &a : annotations )
    {
      // not a restricted annotation
      if ( end(restricted_keys) == find(begin(restricted_keys), end(restricted_keys), a.getKey()) )
      {
        as.emplace(a);
      }
    }

    return as;
  }

  llvm::raw_ostream & Node::recursivePrint(llvm::raw_ostream &os) const
  {
    os << this << "\n";
    if ( children.size() != 0 )
    {
      for ( auto child : children)
        child->recursivePrint( os );
    }

    return os;
    // return;
  }

  llvm::raw_ostream &operator<<(llvm::raw_ostream &os, const Node *node)
  {
    auto *lcn = dyn_cast<LoopContainerNode>(node);

    if ( !node->parent )
      os << "** Root node **\n";
    else if ( lcn )
      os << "** Loop container node **\n";
    else
      os << "** Basic block node **\n";

    os << "\tID: " << node->ID << "\n";
    if ( node->parent )
      os << "\tParent ID: " << node->parent->ID << "\n";
    if ( node->getBB() )
      os << "\tBasic block: " << node->getBB() << "\n";

    if ( lcn )
    {
      os << "\tLoop: " << node->loop << "\n";
      os << "\tLine num: " << node->getDebugLoc() << "\n";
      os << "\tHeader inst: " << *(lcn->getHeaderNode()->getBB()->getFirstNonPHI()) << "\n";
    }

    os << "\tAnnotations:\n";
    for ( auto &annot : node->annotations )
    {
      // os << "\t\t" << annot.getKey() << " : " << annot.getValue() << "\n";
      os << annot;
    }
    os << "\n";

#if 0
    if ( !node->is_leaf )
    {
      if ( !node->parent )
        os << "\033[1;31m** Root node **\033[0m\n";
      else
        os << "\033[36m-- Intermediate node --\033[0m\n";
    }
    else
    {
      os << "\033[32m++ Leaf node ++\033[0m\n";
      os << "\tFirst instruction:\n";
      assert(node->instructions.size() > 0);
      llvm::errs() << "\t\t" << *(node->instructions.begin()) << "\n";
    }

    os << "\tAnnotations:\n";
    os << "\t\tInherited:\n";
    for ( auto &annot : node->inherited_annotations )
      os << "\t\t\t" << annot.first << " : " << annot.second << "\n";

    os << "\t\tUninherited:\n";
    for ( auto &annot : node->annotations )
      os << "\t\t\t" << annot.first << " : " << annot.second << "\n";

#endif
    return os;
  }

} // namespace llvm
