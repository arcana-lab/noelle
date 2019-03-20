#ifndef CAT_COMMUTATIVEDEPENDENCYSOURCE_HPP
#define CAT_COMMUTATIVEDEPENDENCYSOURCE_HPP

#include "PDG.hpp"


class CommutativeDependenceSource {
 public:
  explicit CommutativeDependenceSource(llvm::Module &);

  void loadCommutativeDependencies();

  typedef std::set<std::pair<Value *, Value *>> Dependencies;
  typedef Dependencies::iterator comm_dep_iterator;

  comm_dep_iterator
  commutativeEdges_begin(Function *F) { CommutativeDependencies[F].begin(); }

  comm_dep_iterator
  commutativeEdges_end(Function *F) { CommutativeDependencies[F].end(); }

  iterator_range<comm_dep_iterator>
  getCommutativeEdges(Function *F) { return make_range(CommutativeDependencies[F].begin(), CommutativeDependencies[F].end()); }


 private:
  void loadFromFile(StringRef &value);

  std::map<Function *, Dependencies> CommutativeDependencies;

  Module &M;
};

#endif //CAT_COMMUTATIVEDEPENDENCYSOURCE_HPP
