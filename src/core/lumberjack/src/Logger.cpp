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

Logger &Logger::print() {
  this->LJ.getStream() << this->name << this->LJ.getSeparator();
  for (const auto &section : this->sections) {
    this->LJ.getStream() << section << this->LJ.getSeparator();
  }
  this->lineIsEnabled = true;
  return *this;
}

Logger &Logger::print(LVerbosity verbosity) {
  if (this->LJ.isEnabled(this->name, verbosity)) {
    print();
  } else {
    this->lineIsEnabled = false;
  }
  return *this;
}

Logger &Logger::debug() {
  return print(LOG_DEBUG);
}

Logger &Logger::info() {
  return print(LOG_INFO);
}

void Logger::open(const char *name) {
  this->sections.push_back(string(name));
}

void Logger::open(string name) {
  this->sections.push_back(std::move(name));
}

void Logger::close() {
  this->sections.pop_back();
}

} // namespace arcana::noelle
