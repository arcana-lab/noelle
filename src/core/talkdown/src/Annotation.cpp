#include "Annotation.hpp"

namespace AutoMP
{
  using namespace llvm;

  bool withinAnnotationSet(const AnnotationSet &as, std::string key, std::string value, const Loop *loop)
  {
    llvm::Loop *cast_loop = const_cast<Loop *>(loop);
    for ( const auto &a : as )
    {
      if ( a == Annotation(cast_loop, key, value) )
        return true;
    }

    return false;
  }

  llvm::raw_ostream &operator<<(llvm::raw_ostream &os, const Annotation &a)
  {
    // TODO get this working... segfaulting now
    // os << "Loop " << getLoopLineNum(a.getLoop()) << " | " << a.getKey() << " : " << a.getValue() << "\n";

    os << "Loop " << a.getLoop() << " | " << a.getKey() << " : " << a.getValue() << "\n";
    return os;
  }

  llvm::raw_ostream &operator<<(llvm::raw_ostream &os, const std::pair<const llvm::Instruction *, const AnnotationSet &> &p)
  {
    os << *(p.first) << ":\n";
    for ( auto &a : p.second )
    {
      os << a << "\n";
    }

    return os;
  }

  /**** XXX Below this line is unimplemented code! ***/
  ReduxAnnotation::ReduxAnnotation()
  : Annotation()
  {
    assert(0 && "ReduxAnnotation not implemented");
  }
} // namespace llvm
