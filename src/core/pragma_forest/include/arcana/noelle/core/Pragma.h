#ifndef __NOELLE_PRAGMA_H__
#define __NOELLE_PRAGMA_H__

typedef int pragma_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NOELLE_PRAGMA_BODY_DISABLE

__attribute__((warn_unused_result, pure, noinline)) pragma_t
noelle_pragma_begin(const char *directive, ...);

__attribute__((pure, noinline)) void noelle_pragma_end(pragma_t p);

#else

__attribute__((warn_unused_result, pure, noinline)) pragma_t
noelle_pragma_begin(const char *directive, ...) {
  return 0;
}

__attribute__((pure, noinline)) void noelle_pragma_end(pragma_t p) {}

#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif // #ifndef __NOELLE_PRAGMA_H__
