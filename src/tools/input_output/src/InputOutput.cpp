/*
 * Copyright 2023  Tzu-Hsuan Huang, Simone Campanoni
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

#include "InputOutput.hpp"

#include "noelle/core/Noelle.hpp"

namespace llvm::noelle {

std::unordered_map<std::string, std::string> stdioUnlockedFunctionMapping = {
  { "fgetc", "fgetc_unlocked" },       { "fgets", "fgets_unlocked" },
  { "fputc", "fputc_unlocked" },       { "fputs", "fputs_unlocked" },
  { "fread", "fread_unlocked" },       { "fprintf", "fprintf_unlocked" },
  { "fscanf", "fscanf_unlocked" },     { "getc", "getc_unlocked" },
  { "getchar", "getchar_unlocked" },   { "putc", "putc_unlocked" },
  { "putchar", "putchar_unlocked" },   { "printf", "printf_unlocked" },
  { "scanf", "scanf_unlocked" },       { "ungetc", "ungetc_unlocked" },
  { "vfprintf", "vfprintf_unlocked" }, { "vfscanf", "vfscanf_unlocked" },
};

InputOutput::InputOutput() : ModulePass{ ID } {}

bool InputOutput::runOnModule(Module &M) {
  for (auto [ioWithLock, ioWithoutLock] : stdioUnlockedFunctionMapping) {
    if (auto getcF = M.getFunction(ioWithLock)) {
      getcF->setName(ioWithoutLock);
    }
  }

  return false;
}

} // namespace llvm::noelle
