#define DEBUG_TYPE "talkdown"

#include "llvm/IR/Metadata.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/IntrinsicInst.h"

// #include "Noelle.hpp"

#include "ReportDump.hpp"
#include "Tree.hpp"
#include "Annotation.hpp"
#include "AnnotationParser.hpp"

// #include <graphviz/cgraph.h>

#include <cstdio>
#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <utility>

using namespace llvm;

namespace AutoMP
{
  static bool ancestorsContainAnnotations(Node *n, const AnnotationSet &as)
  {
    using namespace std;
    while ( (n = n->getParent()) != nullptr )
    {
      auto as_p = n->getRealAnnotations();
      int count = 0;
      for ( const auto &a : as )
      {
        if ( as_p.end() != as_p.find(a) )
          count++;
      }
      if ( as.size() == count )
        return true;
    }

    return false;
  }

  FunctionTree::~FunctionTree()
  {
  }

  Node * FunctionTree::getRoot() const {
    return root;
  }

  void findLeaves(Node *n, std::set<Node *> *leaves) {
    auto children = n->getChildren();
    if( children.empty() ) {
      leaves->insert(n);
      return;
    }
    for ( auto &child : children ) {
      findLeaves(child, leaves);
    }
  }

  std::set<Node *> FunctionTree::getLeaves() const {
    auto root = getRoot();
    std::set<Node *> leaves;
    findLeaves(root, &leaves);
    return leaves;
  }

  // returns inner-most loop container for loop l
  Node *FunctionTree::findNodeForLoop( Node *start, Loop *l ) const
  {
    if ( start->getLoop() == l )
      return start;

    for ( auto &n : start->getChildren() )
    {
      Node *found = findNodeForLoop( n, l );
      if ( found )
        return found;
    }

    return nullptr;
  }

  Node *FunctionTree::findNodeForBasicBlock( Node *start, BasicBlock *bb ) const
  {
    if ( start->getBB() == bb )
      return start;

    for ( auto &n : start->getChildren() )
    {
      Node *found = findNodeForBasicBlock( n, bb );
      if ( found )
        return found;
    }

    return nullptr;
  }

  Node *FunctionTree::findNodeForInstruction(Node *start, llvm::Instruction *i) const
  {
    return findNodeForBasicBlock( start, i->getParent() );
  }

  Node *searchUpForAnnotation(Node *start, std::pair<std::string, std::string> a)
  {
    while ( start->getParent() )
    {
      auto as = start->getAnnotations();
      for ( const auto &annot : as )
      {
        // if key and value match an annotaion of the current node, return that node
        if ( !annot.getKey().compare(a.first) && !annot.getValue().compare(a.second) )
          return start;
      }

      start = start->getParent();
    }

    return nullptr;
  }

  std::vector<Node *> FunctionTree::getNodesInPreorder(Node *start) const
  {
    std::vector<Node *> retval = { start };
    if ( start->getChildren().size() == 0 )
      return retval;
    for ( auto &n : start->getChildren() )
    {
      std::vector<Node *> child_nodes = getNodesInPreorder( n );
      retval.insert( retval.end(), child_nodes.begin(), child_nodes.end() );
    }
    return retval;
  }

  std::vector<LoopContainerNode *> FunctionTree::getAllLoopContainerNodes(void) const
  {
    std::vector<LoopContainerNode *> retval;
    std::vector<Node *> all_nodes = getNodesInPreorder( root );
    for ( auto &n : all_nodes )
      if ( n->containsAnnotationWithKey( "__loop_container" ) )
        retval.push_back( (LoopContainerNode *) n );

    return retval;
  }

  std::vector<Node *> FunctionTree::getAllLoopBasicBlockNodes(void) const
  {
    std::vector<Node *> retval;
    auto all_nodes = getNodesInPreorder( root );
    for ( auto &n : all_nodes )
    {
      if ( n->containsAnnotationWithKey("__loop_bb") )
        retval.push_back(n);
    }

    return retval;
  }

  bool FunctionTree::loopContainsAnnotation(Loop *l) const
  {
    errs() << "Looking for container node for loop " << l << "\n";
    LoopContainerNode *ln = (LoopContainerNode *) findNodeForLoop(root, l);
    assert( ln != root && "Loop container is root???" );
    assert( ln && "No loop container node for loop???" );
    errs() << "Found!\n";
    AnnotationSet as;
    // AnnotationSet as = ln->getRealAnnotations();
    if ( !as.size() )
      return false;
    return true;
  }

  // this function adds loop-container nodes to the tree that will end up being parents to all respective subloops (both
  // subloop-container nodes and basic blocks )
  // Note that this does not add any basic blocks to the tree
  // TODO Change to LoopContainerNode
  void FunctionTree::addLoopContainersToTree(LoopInfo &li)
  {
    auto loops = li.getLoopsInPreorder(); // this shit is amazing!

    for ( auto const &l : loops )
    {
      LoopContainerNode *new_node;

      // create node with root as parent
      if ( l->getParentLoop() == nullptr )
        new_node = new LoopContainerNode( root, l );

      // find the node containing parent loop and add as child to that node
      else
      {
        Node *parent = const_cast<Node *>(findNodeForLoop( root, l->getParentLoop() )); // pretty stupid but I'll change it later maybe
        assert( parent != nullptr && "Subloop doesn't have a parent loop -- something is wrong with getLoopsInPreorder()" );
        new_node = new LoopContainerNode( parent, l );
      }

      // add an internal annotation
      AnnotationSet as{{l, "__loop_container", "yes"}, {l, "__level", std::to_string(l->getLoopDepth())}};
      new_node->addAnnotations( std::move(as) );

      // add loop line number
      MDNode *loopMD = l->getLoopID();
      if ( loopMD )
      {
        auto *diloc = dyn_cast<DILocation>(loopMD->getOperand(1));
        auto dbgloc = DebugLoc(diloc);
        new_node->setDebugLoc( std::to_string(dbgloc.getLine()) );
      }

      BasicBlock *header = l->getHeader();
      assert( header && "Loop doesn't have header!" );

      nodes.push_back( new_node ); // FIXME: remove once iterator done

      // add annotations that apply to whole loop (namely ones in the header)
      /* AnnotationSet as = parseAnnotationsForInst( header->getFirstNonPHI() ); */
      /* new_node->addAnnotations( std::move(as) ); */
    }

    // annotateLoops();
  }

  void FunctionTree::annotateBasicBlocks(void)
  {
    auto loop_bb_nodes = getAllLoopBasicBlockNodes(); // returns bb nodes of preordered loops
    for ( auto &bn : loop_bb_nodes )
    {
      Instruction *i = bn->getBB()->getFirstNonPHI();
      bn->addAnnotations( parseAnnotationsForInst(i) );
    }
  }

  // This function is pretty stupid right now as FunctionTree doesn't have an iterator yet.
  // Seems like metadata is only attached to branch instruction in loop header and not the icmp instruction before...
  void FunctionTree::annotateLoops()
  {
    auto loop_nodes = getAllLoopContainerNodes(); // returns nodes in preorder

    // traverse nodes in tree
    for ( auto &node : loop_nodes )
    {
      Node *header = node->getHeaderNode();
      assert( header && "Loop doesn't have header!" );

      Instruction *i = header->getBB()->getFirstNonPHI();
      auto as = parseAnnotationsForInst( i );
      if ( as.size() == 0 )
        continue;

      // inherit annotations from outer loops...
      // UNLESS they contain the same annotations. That means they only apply to the outer loop
      if ( !ancestorsContainAnnotations(node, as) )
        node->addAnnotations( std::move(node->getParent()->getRealAnnotations()) );
      else
        REPORT_DUMP(errs() << "Parent loop contains same annotation as subloop\n";);

      /*
      if ( node->getParent() != root )
      {
        auto parent_as = node->getParent()->getRealAnnotations();
        if ( parent_as != as )
          node->addAnnotations( std::move(node->getParent()->getRealAnnotations()) );
        else
          REPORT_DUMP(errs() << "Parent loop contains same annotation as subloop\n";);
      }
      */

      // found annotation in loop header, propagate to all direct children
      AnnotationSet new_annots;
      for ( auto &a : as )
      {
        new_annots.emplace( node->getLoop(), a.getKey(), a.getValue() );
      }
      node->addAnnotations( std::move(as) );
    }
  }

  // creates nodes for basic blocks that belong to a loop and links to the correct loop container node
  // NOTE: doesn't add annotations to basic block node
  void FunctionTree::addBasicBlocksToLoops(LoopInfo &li)
  {

    for ( auto &bb : *associated_function )
    {
      Loop *l = li.getLoopFor( &bb );

      // if not belonging to loop, continue
      if ( !l )
        continue;

      LoopContainerNode *insert_pt = (LoopContainerNode *) const_cast<Node *>(findNodeForLoop( root, l ));
      assert( insert_pt && "No node found for loop" );

      Node *new_node = new Node( insert_pt );
      new_node->setBB( &bb );
      AnnotationSet as;
      as.emplace(nullptr, "__loop_bb", "true");
      if ( l->getHeader() == &bb )
      {
        as.emplace(nullptr, "__loop_header", "true");
        insert_pt->setHeaderNode( new_node );
      }
      new_node->addAnnotations( std::move(as) );
      nodes.push_back( new_node );
    }
  }

  // we don't care about annotations for non-loop basic blocks
  // XXX Long-term: support #pragma omp parallel region (not necessitating "for" clause)
  void FunctionTree::addNonLoopBasicBlocks(LoopInfo &li)
  {
    for ( auto &bb : *associated_function )
    {
      Loop *l = li.getLoopFor( &bb );

      // Add blocks that don't belong to any loop to the tree as direct children of root node
      if ( !l )
      {
        Node *new_node = new Node( root );
        AnnotationSet as = { {nullptr, "__non_loop_bb", "true"} };
        new_node->addAnnotations( std::move(as) );
        new_node->setBB( &bb );
        nodes.push_back( new_node ); // BAD: remove once iterator done
      }
    }
  }

  // construct a tree for each function in program order
  // steps:
  //  1. Basic blocks that don't belong to any loops don't have any annotations. They should be direct children of the root node
  //  2. Create container nodes for each outer loop, with root as a parent
  //  3. For each subloop, create a container node that a child of the parent loop
  //  4. Annotate each loop with annotations from its header basic block
  bool FunctionTree::constructTree(Function *f, LoopInfo &li)
  {
    bool modified = false;
    std::set<BasicBlock *> visited_bbs;

    associated_function = f;

    // construct root node
    // TODO(greg): with annotation of function if there is one
    this->root = new Node();
    AnnotationSet as = { {nullptr, "__root", "yes"} };
    root->addAnnotations( std::move(as) );
    root->annotations.emplace( nullptr, "__root", "yes" );
    nodes.push_back( root ); // BAD: remove once iterator done

    // split basic blocks based on annotation before adding them to the tree
    // modified |= splitBasicBlocksByAnnotation();

    // fix the fact that the frontend misses adding annotations to some instructions
    // modified |= fixBasicBlockAnnotations();

    // add all loops containsers (including subloops) to the tree
    addLoopContainersToTree( li );

    // add all basic blocks to loop nodes
    addBasicBlocksToLoops( li );

    // add all basic blocks not in a loop
    addNonLoopBasicBlocks( li );

    // supposedly adds annotations to loop container...
    annotateLoops();

    // add annotations to basic block nodes
    annotateBasicBlocks();

    return modified;
  }

#if 0
  SESENode *FunctionTree::getInnermostNode(Instruction *inst)
  {

  }

  SESENode *FunctionTree::getParent(SESENode *node)
  {
    return node->getParent();
  }

  SESENode *FunctionTree::getFirstCommonAncestor(SESENode *n1, SESENode *n2)
  {
    SESENode *deeper;
    SESENode *shallower;

    // if same node, return one of them
    if ( n1 == n2 )
      return n1;

    // find which node is deeper in the tree
    if ( n1->getDepth() > n2->getDepth() )
    {
      deeper = n1;
      shallower = n2;
    }
    else
    {
      deeper = n2;
      shallower = n1;
    }

    // get the two search nodes to the same depth
    while ( shallower->getDepth() != deeper->getDepth() )
      deeper = deeper->getParent();

    while ( deeper != root )
    {
      if ( deeper == shallower )
        return deeper;

      deeper = deeper->getParent();
      shallower = shallower->getParent();
    }

    // if we get to root, then there was no common ancestor
    return nullptr;
  }
#endif

  Node* FunctionTree::findInst(Node* node, Instruction* I) const {
    if (node->getBB() == I->getParent()) {
      return node;
    }

    for (auto &child : node->getChildren()) {
      auto n = findInst(child, I);
      if (n != nullptr) {
        return n;
      }
    }
    return nullptr;
  }

  const AnnotationSet &FunctionTree::getAnnotationsForInst(Instruction * I) const
  {
    auto root = getRoot();

    auto Inode = findInst(root, I);
    if (Inode != nullptr) {
      return Inode->getAnnotations();
    }
    // from root
    //
    // if node-getBB == I-getParent
    // return node->getAnnotations
    //
//    assert(0 && "getAnnotationsForInst(llvm::Instruction *) not implemented yet");
  }

  const AnnotationSet &FunctionTree::getAnnotationsForInst(Instruction *i, Loop *l) const
  {
    const Node *n = findNodeForLoop(root, l);
    const BasicBlock *target = i->getParent();
    for ( const auto &bbn : n->getChildren() )
      if ( bbn->getBB() == target )
        return bbn->getAnnotations();
  }

  /*
  void FunctionTree::writeDotFile( const std::string filename )
  {
    std::FILE *fp = std::fopen(filename.c_str(), "w");
    // assert( 0 && "writeDotFile() not implemented yet" );
    Agraph_t *g = agopen((char*) filename.c_str(), Agdirected, NULL);
    Agnode_t *n0 = agnode(g, "node0", 1);
    Agnode_t *n1 = agnode(g, "node1", 1);
    Agedge_t *e = agedge(g, n0, n1, "e0", 1);
    agwrite(g, fp);
    agclose(g);
  }
  */

  // verify that the current tree is value
  bool FunctionTree::isValidTree(void) const
  {
    /*
     * Steps to perform:
     *  1. Verify all loop container nodes are in the tree
     *  2. Verify there are no duplicate basic block nodes
     */

    return false;
  }

  void FunctionTree::printNodeToInstructionMap(void) const
  {
    for ( const auto &n : nodes )
    {
      BasicBlock *bb = n->getBB();
      Instruction *first_inst = nullptr;
      if ( bb )
        first_inst = bb->getFirstNonPHI();
      else
      {
        errs() << "Node " << n->getID() << " has no basic block\n";
        continue;
      }

      if ( !first_inst )
        errs() << "Node " << n->getID() << " has no non-PHI instructions\n";
      else
        errs() << "Node " << n->getID() << " ==> BB " << bb << ":" << *first_inst << "\n";
    }
  }

  raw_ostream &operator<<(raw_ostream &os, const FunctionTree &tree)
  {
    os << "------- FunctionTree for function " << tree.associated_function->getName().str() << " --------\n\n";
    os << "Nodes to instruction map:\n";
    tree.printNodeToInstructionMap();
    Function *af = tree.getFunction();
    os << "Contains " << tree.getNodesInPreorder(tree.root).size() << " nodes\n";
    assert( af && "Function associated with a FunctionTree null" );

    // XXX For heavy debugging
    /* for ( auto &bb : *af) */
    /* { */
    /*   for ( auto &i : bb ) */
    /*   { */
    /*     const AnnotationSet as = parseAnnotationsForInst( &i ); */
    /*     const std::pair<const llvm::Instruction *, const AnnotationSet &> inst_annot = std::make_pair(&i, as); */
    /*     os << inst_annot; */
    /*   } */
    /* } */
    return tree.root->recursivePrint( os );
  }

} // namespace llvm
