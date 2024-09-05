#include <string>

#include "arcana/noelle/core/Lumberjack.hpp"

namespace arcana::noelle {

using namespace std;

Logger::Logger(Lumberjack &LJ, const char *name) : name(name), LJ(LJ) {}

Logger &Logger::operator()(int level) {
  if (this->LJ.isEnabled(this->name, level)) {
    this->LJ.getStream() << this->name << this->LJ.getSeparator();
    for (const auto &section : this->sections) {
      this->LJ.getStream() << section << this->LJ.getSeparator();
    }
    this->line_is_enabled = true;
  } else {
    this->line_is_enabled = false;
  }
  return *this;
}

Logger &Logger::debug() {
  return this->operator()(LOG_DEBUG);
}

Logger &Logger::info() {
  return this->operator()(LOG_INFO);
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
