#ifndef CAT_UNIQUEIRCONSTANTS_HPP
#define CAT_UNIQUEIRCONSTANTS_HPP

#include <llvm/ADT/StringRef.h>

using llvm::StringRef;

class UniqueIRConstants {
 public:
  static const StringRef VIAInstruction;
  static const StringRef VIABasicBlock;
  static const StringRef VIAFunction;
  static const StringRef VIAModule;
};

#endif //CAT_UNIQUEIRCONSTANTS_HPP
