#ifndef __NOELLE_PRAGMA_DEF_H__
#define __NOELLE_PRAGMA_DEF_H__

#include "arcana/noelle/core/PragmaDecl.h"

#ifndef NOELLE_PRAGMA_INLINE
#  define NOELLE_PRAGMA_INLINE inline
#endif

NOELLE_PRAGMA_INLINE pragma_t noelle_pragma_begin(const char *directive, ...) {
  return 0;
}

NOELLE_PRAGMA_INLINE void noelle_pragma_end(pragma_t p) {}

#endif // #ifndef __NOELLE_PRAGMA_DEF_H__
