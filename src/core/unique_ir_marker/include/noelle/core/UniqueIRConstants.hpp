#ifndef NOELLE_SRC_CORE_UNIQUEIRCONSTANTS_H
#define NOELLE_SRC_CORE_UNIQUEIRCONSTANTS_H
#pragma once

#include "noelle/core/SystemHeaders.hpp"

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

} // namespace llvm::noelle

#endif
