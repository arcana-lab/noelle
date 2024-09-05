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
#include <fstream>
#include <sstream>

#include "llvm/Support/raw_ostream.h"
#include "rapidjson/document.h"

#include "arcana/noelle/core/Lumberjack.hpp"
#include "Config.hpp"

using namespace std;
using namespace llvm;

namespace arcana::noelle {

Lumberjack NoelleLumberjack(NOELLE_LUMBERJACK_JSON_DEFAULT_PATH, llvm::errs());

Lumberjack::Lumberjack(const char *filename, raw_ostream &ostream)
  : default_verbosity(LOG_INFO),
    ostream(ostream) {
  using namespace std;

  stringstream input;
  ifstream ifs(filename);
  input << ifs.rdbuf();

  rapidjson::Document json;
  json.Parse(input.str().c_str());

  if (!json.IsObject()) {
    return;
  }

  if (json.HasMember("default_verbosity")) {
    auto &verbosity = json["default_verbosity"];
    assert(verbosity.IsInt());
    if (verbosity.IsInt()) {
      auto v = verbosity.GetInt();
      if (LOG_NONE <= v && v <= LOG_DEBUG) {
        this->default_verbosity = static_cast<Verbosity>(v);
      }
    }
  }

  if (json.HasMember("separator")) {
    this->separator = json["separator"].GetString();
  }

  if (json.HasMember("names")) {
    auto &classes = json["names"];
    assert(classes.IsObject());
    for (auto &member : classes.GetObject()) {
      assert(member.value.IsInt());
      Verbosity v = this->default_verbosity;
      auto mv = member.value.GetInt();
      if (LOG_NONE <= mv && mv <= LOG_DEBUG) {
        v = static_cast<Verbosity>(mv);
      }
      this->classes[member.name.GetString()] = v;
    }
  }
}

Lumberjack::~Lumberjack() {}

bool Lumberjack::isEnabled(const char *name, int level) {
  auto it = this->classes.find(name);
  if (it != this->classes.end()) {
    int max = it->second;
    if (level <= max) {
      return true;
    }
  } else {
    if (level <= this->default_verbosity) {
      return true;
    }
  }
  return false;
}

const std::string &Lumberjack::getSeparator() const {
  return this->separator;
}

raw_ostream &Lumberjack::getStream() {
  return this->ostream;
}

} // namespace arcana::noelle
