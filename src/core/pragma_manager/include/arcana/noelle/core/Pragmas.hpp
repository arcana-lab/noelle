#ifndef __NOELLE_PRAGMAS_HPP__
#define __NOELLE_PRAGMAS_HPP__

template <typename... T>
__attribute__((noinline, optnone)) void noelle_pragma_begin(T...) {}

template <typename... T>
__attribute__((noinline, optnone)) void noelle_pragma_end(T...) {}

#endif
