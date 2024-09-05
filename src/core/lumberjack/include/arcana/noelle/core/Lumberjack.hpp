/*
 * Copyright 2024 Federico Sossai, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __NOELLE_CORE_LUMBERJACK_HPP__
#define __NOELLE_CORE_LUMBERJACK_HPP__

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "llvm/Support/raw_ostream.h"

namespace arcana::noelle {

enum Verbosity { LOG_NONE = 0, LOG_INFO = 1, LOG_DEBUG = 2 };

class Lumberjack {
public:
  Lumberjack(const char *filename, llvm::raw_ostream &ostream);

  ~Lumberjack();

  bool isEnabled(const char *name, int level);

  const std::string &getSeparator() const;

  llvm::raw_ostream &getStream();

private:
  Verbosity default_verbosity;
  std::string separator;
  std::unordered_map<std::string, Verbosity> classes;
  llvm::raw_ostream &ostream;
};

// Helper to detect if T has an operator()
// template <typename T>
// struct has_call_operator {
// private:
//     // Check for non-const call operator
//     template <typename U>
//     static auto check(U*) -> decltype(&U::operator(), std::true_type{});
//
//     // Check for const call operator
//     template <typename U>
//     static auto check(const U*) -> decltype(&U::operator(),
//     std::true_type{});
//
//     // Fallback for no call operator
//     template <typename>
//     static std::false_type check(...);
//
// public:
//     static constexpr bool value = decltype(check<T>(nullptr))::value;
// };

class Logger {
public:
  Logger(Lumberjack &LM, const char *name);

  Logger &operator()(int level);

  Logger &debug();

  Logger &info();

  void open(const char *name);

  void open(std::string name);

  void close();

  // template <typename T, typename =
  // std::enable_if_t<has_call_operator<T>::value>>
  template <typename T>
  Logger &operator<<(const T value);

private:
  const char *name;
  bool line_is_enabled = false;
  std::vector<std::string> sections;
  Lumberjack &LJ;
};

// template <typename T, typename =
// std::enable_if_t<has_call_operator<T>::value>>
template <typename T>
Logger &Logger::operator<<(const T value) {
  if (line_is_enabled) {
    LJ.getStream() << value;
  }
  return *this;
}

} // namespace arcana::noelle

#endif // #ifndef __NOELLE_CORE_LUMBERJACK_HPP__
