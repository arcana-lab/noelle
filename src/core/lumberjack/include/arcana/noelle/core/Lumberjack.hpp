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

class Lumberjack; // Forward declaration

extern Lumberjack NoelleLumberjack;

enum LVerbosity {
  LOG_BYPASS = -1,
  LOG_DISABLED = 0,
  LOG_INFO = 1,
  LOG_DEBUG = 2
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

class Logger; // Forward declaration

class LogStream {
public:
  LogStream(llvm::raw_ostream &ostream, bool enabled, std::string prefix)
    : ostream(ostream),
      enabled(enabled),
      prefix(std::move(prefix)),
      prefixHasBeenPrinted(false) {}

  template <typename F>
  typename std::enable_if_t<std::is_invocable_v<F>, LogStream &> operator<<(
      F &func);

  template <typename T>
  typename std::enable_if_t<!std::is_invocable_v<T>, LogStream &> operator<<(
      const T &value);

  template <typename T>
  LogStream &print(const T &obj, bool usePrefix = false);

private:
  llvm::raw_ostream &ostream;
  bool enabled;
  std::string prefix;
  bool prefixHasBeenPrinted;
};

class Logger {
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
  const char *name;
  std::vector<std::string> sections;
  Lumberjack &LJ;
};

template <typename F>
typename std::enable_if_t<std::is_invocable_v<F>, LogStream &> LogStream::
operator<<(F &func) {
  if (this->enabled) {
    if (!this->prefixHasBeenPrinted) {
      this->ostream << this->prefix;
      this->prefixHasBeenPrinted = true;
    }
    this->ostream << func();
  }
  return *this;
}

template <typename T>
typename std::enable_if_t<!std::is_invocable_v<T>, LogStream &> LogStream::
operator<<(const T &value) {
  if (this->enabled) {
    if (!this->prefixHasBeenPrinted) {
      this->ostream << this->prefix;
      this->prefixHasBeenPrinted = true;
    }
    this->ostream << value;
  }
  return *this;
}

template <typename T>
LogStream &LogStream::print(const T &obj, bool usePrefix) {
  if (this->enabled) {
    if (!this->prefixHasBeenPrinted) {
      if (!usePrefix) {
        this->ostream << this->prefix;
        this->prefixHasBeenPrinted = true;
      }
    }
    if (usePrefix) {
      obj.print(this->ostream, this->prefix);
    } else {
      obj.print(this->ostream);
    }
  }
  return *this;
}

} // namespace arcana::noelle

#endif // #ifndef __NOELLE_CORE_LUMBERJACK_HPP__
