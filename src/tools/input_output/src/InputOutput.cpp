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
#include "noelle/core/Noelle.hpp"
#include "InputOutput.hpp"

namespace arcana::noelle {

std::unordered_map<std::string, std::string> InputOutput::stdioUnlockedFunctionMapping = {
  { "getc",
    "getc_unlocked" }, // https://pubs.opengroup.org/onlinepubs/9699919799/functions/getc_unlocked.html
  { "getchar",
    "getchar_unlocked" }, // https://pubs.opengroup.org/onlinepubs/9699919799/functions/getchar_unlocked.html
  { "putc",
    "putc_unlocked" }, // https://pubs.opengroup.org/onlinepubs/9699919799/functions/putc_unlocked.html
  { "putchar",
    "putchar_unlocked" }, // https://pubs.opengroup.org/onlinepubs/9699919799/functions/putchar_unlocked.html
  { "clearerr",
    "clearerr_unlocked" }, // https://refspecs.linuxbase.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/baselib-clearerr-unlocked-1.html
  { "feof",
    "feof_unlocked" }, // https://refspecs.linuxbase.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/baselib-feof-unlocked-1.html
  { "ferror",
    "ferror_unlocked" }, // https://refspecs.linuxbase.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/baselib-ferror-unlocked-1.html
  { "fflush",
    "fflush_unlocked" }, // https://refspecs.linuxbase.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/baselib-fflush-unlocked-1.html
  { "fgetc",
    "fgetc_unlocked" }, // https://refspecs.linuxbase.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/baselib-fgetc-unlocked-1.html
  { "fgets",
    "fgets_unlocked" }, // https://refspecs.linuxbase.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/baselib-fgets-unlocked-1.html
  { "fileno",
    "fileno_unlocked" }, // https://refspecs.linuxbase.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/baselib-fileno-unlocked-1.html
  { "fputc",
    "fputc_unlocked" }, // https://refspecs.linuxbase.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/baselib-fputc-unlocked-1.html
  { "fputs",
    "fputs_unlocked" }, // https://refspecs.linuxbase.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/baselib-fputs-unlocked-1.html
  { "fread",
    "fread_unlocked" }, // https://refspecs.linuxbase.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/baselib-fread-unlocked-1.html
  { "fwrite",
    "fwrite_unlocked" }, // https://refspecs.linuxbase.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/baselib-fwrite-unlocked-1.html
  { "fgetwc",
    "fgetwc_unlocked" }, // https://refspecs.linuxbase.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/baselib-fgetwc-unlocked-1.html
  { "fgetws",
    "fgetws_unlocked" }, // https://refspecs.linuxbase.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/baselib-fgetws-unlocked-1.html
  { "fputwc",
    "fputwc_unlocked" }, // https://refspecs.linuxbase.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/baselib-fputwc-unlocked-1.html
  { "fputws",
    "fputws_unlocked" }, // https://refspecs.linuxbase.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/baselib-fputws-unlocked-1.html
  { "getwc",
    "getwc_unlocked" }, // https://refspecs.linuxbase.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/baselib-getwc-unlocked-1.html
  { "getwchar",
    "getwchar_unlocked" }, // https://refspecs.linuxbase.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/baselib-getwchar-unlocked-1.html
  { "putwc",
    "putwc_unlocked" }, // https://refspecs.linuxbase.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/baselib-putwc-unlocked-1.html
  { "putwchar",
    "putwchar_unlocked" }, // https://refspecs.linuxbase.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/baselib-putwchar-unlocked-1.html
};

InputOutput::InputOutput() : ModulePass{ ID } {}

bool InputOutput::runOnModule(Module &M) {
  for (auto [io, ioUnlocked] : stdioUnlockedFunctionMapping) {
    if (auto F = M.getFunction(io)) {
      F->setName(ioUnlocked);
    }
  }

  return false;
}

} // namespace arcana::noelle
