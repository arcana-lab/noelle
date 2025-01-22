#ifndef __NOELLE_PRAGMA_DECL_H__
#define __NOELLE_PRAGMA_DECL_H__

typedef int pragma_t;

__attribute__((warn_unused_result)) pragma_t noelle_pragma_begin(
    const char *directive,
    ...);

void noelle_pragma_end(pragma_t p);

#endif // #ifndef __NOELLE_PRAGMA_DECL_H__
