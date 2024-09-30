#ifndef __NOELLE_PRAGMA_H__
#define __NOELLE_PRAGMA_H__

// We support a predefine set of special functions that work in both C and C++.
// On top of this, C++ comes with a templated `begin` that reduces boilerplate
// code

typedef int pragma_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NOELLE_PRAGMA_BODY_DISABLE

__attribute__((warn_unused_result, pure)) pragma_t noelle_pragma_begin(
    const char *);

__attribute__((pure)) void noelle_pragma_arg_str(pragma_t, const char *);

__attribute__((pure)) void noelle_pragma_arg_int(pragma_t, int);

__attribute__((pure)) void noelle_pragma_end(pragma_t);

#else

__attribute__((noinline)) pragma_t noelle_pragma_begin(const char *) {
  return 0;
}

__attribute__((noinline)) void noelle_pragma_arg_str(pragma_t, const char *) {}

__attribute__((noinline)) void noelle_pragma_arg_int(pragma_t, int) {}

__attribute__((noinline)) void noelle_pragma_end(pragma_t) {}

#endif

#ifdef __cplusplus
}

#  ifdef NOELLE_PRAGMA_BODY_DISABLE

template <typename... T>
__attribute__((warn_unused_result, pure)) pragma_t noelle_pragma_begin(
    const char *,
    T...);

#  else

template <typename... T>
__attribute__((noinline)) pragma_t noelle_pragma_begin(const char *, T...) {
  return 0;
}

#  endif

#endif

#endif // #ifndef __NOELLE_PRAGMA_H__
