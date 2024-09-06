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
#include <string>

#include "arcana/noelle/core/Lumberjack.hpp"

namespace arcana::noelle {

using namespace std;

Logger::Logger(Lumberjack &LJ, const char *name) : name(name), LJ(LJ) {}

LogStream Logger::level(LVerbosity verbosity) {
  bool enabled = this->LJ.isEnabled(this->name, verbosity);
  string prefix = string(this->name) + this->LJ.getSeparator();
  for (auto section : this->sections) {
    prefix += section;
  }
  return LogStream(this->LJ.getStream(), enabled, prefix);
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

void Logger::openSection(string name) {
  this->sections.push_back(std::move(name) + this->LJ.getSeparator());
}

void Logger::closeSection() {
  this->sections.pop_back();
}

void Logger::openIndent() {
  this->sections.push_back("  ");
}

void Logger::closeIndent() {
  this->sections.pop_back();
}

} // namespace arcana::noelle
