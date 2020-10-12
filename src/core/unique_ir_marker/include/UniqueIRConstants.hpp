#pragma once

#include "SystemHeaders.hpp"

using llvm::StringRef;

using IDType = uint64_t;

namespace llvm::noelle {

  class UniqueIRConstants {
  public:
    static const StringRef VIAInstruction;
    static const StringRef VIABasicBlock;
    static const StringRef VIALoop;
    static const StringRef VIAFunction;
    static const StringRef VIAModule;
  };

}
