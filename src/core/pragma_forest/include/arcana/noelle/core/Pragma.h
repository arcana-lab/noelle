#ifndef __NOELLE_PRAGMA_H__
#define __NOELLE_PRAGMA_H__

// We support a predefine set of special functions that work in both C and C++.
// On top of this, C++ comes with a templated `begin` that reduces boilerplate
// code

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((warn_unused_result)) int noelle_pragma_begin(const char *);

void noelle_pragma_arg_str(int, const char *);

void noelle_pragma_arg_int(int, int);

void noelle_pragma_end(int);

#ifdef __cplusplus
}

template <typename... T>
__attribute__((warn_unused_result)) int noelle_pragma_begin(const char *, T...);

#endif

#endif // #ifndef __NOELLE_PRAGMA___
