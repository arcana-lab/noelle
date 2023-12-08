#ifndef NOELLE_SRC_CORE_UNIQUE_IR_MARKER_UNIQUEIRCONSTANTS_H_
#define NOELLE_SRC_CORE_UNIQUE_IR_MARKER_UNIQUEIRCONSTANTS_H_
#pragma once

#include "noelle/core/SystemHeaders.hpp"

using llvm::StringRef;

using IDType = uint64_t;

namespace arcana::noelle {

class UniqueIRConstants {
public:
  static const StringRef VIAInstruction;
  static const StringRef VIABasicBlock;
  static const StringRef VIALoop;
  static const StringRef VIAFunction;
  static const StringRef VIAModule;
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_UNIQUE_IR_MARKER_UNIQUEIRCONSTANTS_H_
