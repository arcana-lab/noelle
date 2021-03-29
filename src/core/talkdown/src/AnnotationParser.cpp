#include "AnnotationParser.hpp"

namespace AutoMP
{
  using namespace llvm;

  /*
   * XXX This function gets called a lot on the same instructions when constructing the tree
   * This is bad and we should maybe cache the results?
   */

  // Note that this does not populate the "loop" field of Annotation
  AnnotationSet parseAnnotationsForInst(const Instruction *i)
  {
    using namespace llvm;
    AnnotationSet annots;

    MDNode *meta = i->getMetadata("note.noelle");
    if ( meta )
    {
      auto operands = meta->operands();
      for ( auto &op : operands )
      {
        MDNode *casted_meta = dyn_cast<MDNode>(op.get());
        assert( casted_meta && "Couldn't cast operand to MDNode" );

        MDString *key = dyn_cast<MDString>(casted_meta->getOperand(0));
        MDString *value = dyn_cast<MDString>(casted_meta->getOperand(1));
        assert( key && value && "Couldn't cast key or value from annotation" );

        /* errs() << "In parseAnnotationsForInst -- " << *i << "\n"; */
        /* errs() << "\t\t" << key->getString() << " : " << value->getString() << "\n"; */

        // don't care about the loop right now...
        // strip possible double quotes around the key and value
        std::string key_s = key->getString();
        std::string value_s = value->getString();
        key_s.erase(std::remove( key_s.begin(), key_s.end(), '\"' ), key_s.end());
        value_s.erase(std::remove( value_s.begin(), value_s.end(), '\"' ), value_s.end());
        annots.emplace(nullptr, key_s, value_s);
      }
    }

    return annots;
  }
};
