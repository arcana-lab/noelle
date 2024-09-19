/*
 * Copyright 2024 Federico Sossai
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
#include <type_traits>
#include <unordered_map>

#include "llvm/Support/raw_ostream.h"

namespace arcana::noelle {

class Lumberjack; // Forward declaration

extern Lumberjack NoelleLumberjack;

enum LVerbosity {
  LOG_BYPASS = 0, // this must always be the first
  LOG_INFO,
  LOG_DEBUG,
  LOG_DISABLED // this must always be the last
};

class Lumberjack {
public:
  Lumberjack(const char *filename, llvm::raw_ostream &ostream);

  ~Lumberjack();

  bool isEnabled(const char *name, LVerbosity verbosity);

  std::string getSeparator() const;

  llvm::raw_ostream &getStream();

private:
  LVerbosity default_verbosity;
  std::string separator;
  std::unordered_map<std::string, LVerbosity> classes;
  llvm::raw_ostream &ostream;
};

class LogStream; // Forward declaration

class Logger {
  friend class LogStream;

public:
  Logger(Lumberjack &LJ, const char *name);

  LogStream level(LVerbosity verbosity);

  LogStream debug();

  LogStream info();

  LogStream bypass();

  void openSection(std::string name);

  void openIndent();

  void closeSection();

  void closeIndent();

private:
  std::string makePrefix() const;

  const char *name;
  std::vector<std::string> sections;
  bool lineEnabled;
  Lumberjack &LJ;
};

// This trait descripts types that have a member function called `print` that
// accepts a raw_ostream& and a std::string
template <typename T>
class is_prefix_llvmprintable {
private:
  template <typename U>
  static auto check(int)
      -> decltype(std::declval<U>().print(std::declval<llvm::raw_ostream &>(),
                                          std::declval<std::string>()),
                  std::true_type{});

  template <typename>
  static std::false_type check(...);

public:
  static constexpr bool value = decltype(check<T>(0))::value;
};

// This is a trait that describes types that have a member function called
// `print` that it accepts only one raw_ostream&
template <typename T>
class is_llvmprintable {
private:
  template <typename U>
  static auto check(int)
      -> decltype(std::declval<U>().print(std::declval<llvm::raw_ostream &>()),
                  std::true_type{});

  template <typename>
  static std::false_type check(...);

public:
  static constexpr bool value = decltype(check<T>(0))::value;
};

// In simlpe words this trait to capture all non-LLVM types and non-lambda types
// like strings, numbers ecc..
template <typename T>
using is_not_llvmprintable_nor_invocable =
    std::negation<std::disjunction<std::is_invocable<T>,
                                   is_llvmprintable<T>,
                                   is_prefix_llvmprintable<T>>>;

template <typename T>
using is_not_prefix_but_llvmprintable =
    std::conjunction<is_llvmprintable<T>,
                     std::negation<is_prefix_llvmprintable<T>>>;

// Since this is the only Lumberjack class that provides the user with
// an `operator<<`, and the only verbosity-controlling functions of `Logger` are
// the only functions that return a `LogStream` the user is force to specify the
// verbosity for each log message. Having an `operator<<` for `Logger` would
// allow for `log << "abc"` which bypasses the verbosity level (e.g. no
// .debug()) that we want to avoid
class LogStream {
public:
  LogStream(Logger &logger) : logger(logger), needToPrintPrefix(true) {}

  LogStream &noPrefix() {
    this->needToPrintPrefix = false;
    return *this;
  }

  template <typename F>
  typename std::enable_if_t<std::is_invocable_v<F>, LogStream &> operator<<(
      F &func) {
    if (this->logger.lineEnabled) {
      return this << func();
    }
    return *this;
  }

  template <typename T>
  typename std::enable_if_t<is_prefix_llvmprintable<T>::value, LogStream &>
  operator<<(T &obj) {
    if (this->logger.lineEnabled) {
      auto &ostream = this->logger.LJ.getStream();
      auto prefix = this->logger.makePrefix();
      if (!this->needToPrintPrefix) {
        obj.print(ostream, "");
      } else {
        this->needToPrintPrefix = false;
        obj.print(ostream, this->logger.makePrefix());
      }
    }
    return *this;
  }

  template <typename T>
  typename std::enable_if_t<is_not_prefix_but_llvmprintable<T>::value,
                            LogStream &>
  operator<<(T &obj) {
    if (this->logger.lineEnabled) {
      auto &ostream = this->logger.LJ.getStream();
      if (this->needToPrintPrefix) {
        ostream << this->logger.makePrefix();
        this->needToPrintPrefix = false;
      }
      obj.print(ostream);
    }
    return *this;
  }

  template <typename T>
  typename std::enable_if_t<is_not_llvmprintable_nor_invocable<T>::value,
                            LogStream &>
  operator<<(const T &value) {
    if (this->logger.lineEnabled) {
      auto &ostream = this->logger.LJ.getStream();
      if (this->needToPrintPrefix) {
        ostream << this->logger.makePrefix();
        this->needToPrintPrefix = false;
      }
      ostream << value;
    }
    return *this;
  }

private:
  Logger &logger;
  bool needToPrintPrefix;
};

} // namespace arcana::noelle

#endif // #ifndef __NOELLE_CORE_LUMBERJACK_HPP__
