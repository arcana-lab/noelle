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
#include "arcana/noelle/core/Lumberjack.hpp"

namespace arcana::noelle {

IndentedSection::IndentedSection(Logger &logger) : logger(logger) {
  this->logger.sections.push_back("  ");
}

IndentedSection::~IndentedSection() {
  this->logger.sections.pop_back();
  if (this->exitText.size() > 0) {
    this->logger.level(this->exitTextVerbosity) << this->exitText;
  }
}

void IndentedSection::onExit(LVerbosity verbosity, std::string text) {
  this->exitTextVerbosity = verbosity;
  this->exitText = std::move(text);
}

NamedSection::NamedSection(Logger &logger, std::string name) : logger(logger) {
  this->logger.sections.push_back(std::move(name)
                                  + this->logger.LJ.getSeparator());
}

NamedSection::~NamedSection() {
  logger.sections.pop_back();
  if (this->exitText.size() > 0) {
    this->logger.level(this->exitTextVerbosity) << this->exitText;
  }
}

void NamedSection::onExit(LVerbosity verbosity, std::string text) {
  this->exitTextVerbosity = verbosity;
  this->exitText = std::move(text);
}

} // namespace arcana::noelle
