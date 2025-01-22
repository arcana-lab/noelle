#ifndef __NOELLE_PRAGMA_DEF_H__
#define __NOELLE_PRAGMA_DEF_H__

#include "arcana/noelle/core/PragmaDecl.h"

pragma_t noelle_pragma_begin(const char *directive, ...) {
  return 0;
}

void noelle_pragma_end(pragma_t p) {}

#endif // #ifndef __NOELLE_PRAGMA_DEF_H__
