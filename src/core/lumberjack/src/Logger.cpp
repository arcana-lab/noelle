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
#include <string>

#include "arcana/noelle/core/Lumberjack.hpp"

namespace arcana::noelle {

using namespace std;

Logger::Logger(Lumberjack &LJ, const char *name) : name(name), LJ(LJ) {}

LogStream Logger::level(LVerbosity verbosity) {
  this->lineEnabled = this->LJ.isEnabled(this->name, verbosity);
  return LogStream(*this);
}

LogStream Logger::debug() {
  return level(LOG_DEBUG);
}

LogStream Logger::info() {
  return level(LOG_INFO);
}

LogStream Logger::bypass() {
  return level(LOG_BYPASS);
}

string Logger::makePrefix() const {
  string prefix = this->name;
  prefix += this->LJ.getSeparator();
  for (const auto &section : this->sections) {
    prefix += section;
  }
  return prefix;
}

IndentedSection Logger::indentedSection() {
  return IndentedSection(*this);
}

NamedSection Logger::namedSection(string name) {
  return NamedSection(*this, name);
}

} // namespace arcana::noelle
