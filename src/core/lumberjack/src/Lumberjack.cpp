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
#include <fstream>
#include <sstream>
#include <tuple>

#include "llvm/Support/raw_ostream.h"
#include "rapidjson/document.h"

#include "arcana/noelle/core/Lumberjack.hpp"
#include "Config.hpp"

using namespace std;
using namespace llvm;

namespace arcana::noelle {

Lumberjack NoelleLumberjack(NOELLE_LUMBERJACK_JSON_DEFAULT_PATH, errs());

Lumberjack::Lumberjack(const char *filename, raw_ostream &ostream)
  : ostream(ostream) {

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
      if (LOG_BYPASS <= v && v <= LOG_DISABLED) {
        this->default_verbosity = static_cast<LVerbosity>(v);
      } else {
        assert(false && "Unexpected verbosity level");
      }
    }
  } else {
    assert(false && "Corrupted Lumberjack configuration file");
  }

  if (json.HasMember("separator")) {
    this->separator = json["separator"].GetString();
  } else {
    assert(false && "Corrupted Lumberjack configuration file");
  }

  if (json.HasMember("verbosity_override")) {
    auto &classes = json["verbosity_override"];
    assert(classes.IsObject());
    for (auto &member : classes.GetObject()) {
      assert(member.value.IsInt());
      LVerbosity v = this->default_verbosity;
      auto mv = member.value.GetInt();
      if (LOG_BYPASS <= mv && mv <= LOG_DISABLED) {
        v = static_cast<LVerbosity>(mv);
      } else {
        assert(false && "Unexpected verbosity level");
      }
      this->classes[member.name.GetString()] = v;
    }
  }
}

Lumberjack::~Lumberjack() {}

bool Lumberjack::isEnabled(const char *name, LVerbosity verbosity) {
  auto it = this->classes.find(name);
  if (it != this->classes.end()) {
    auto desiredVerbosity = get<LVerbosity>(*it);
    if (verbosity <= desiredVerbosity) {
      return true;
    }
  } else {
    if (verbosity <= this->default_verbosity) {
      return true;
    }
  }
  return false;
}

std::string Lumberjack::getSeparator() const {
  return this->separator;
}

raw_ostream &Lumberjack::getStream() {
  return this->ostream;
}

} // namespace arcana::noelle
