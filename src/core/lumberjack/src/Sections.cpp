#include "arcana/noelle/core/Lumberjack.hpp"

namespace arcana::noelle {

IndentedSection::IndentedSection(Logger &logger) : logger(logger) {
  this->logger.openIndentedSection();
}

IndentedSection::~IndentedSection() {
  this->logger.closeIndentedSection();
}

NamedSection::NamedSection(Logger &logger, std::string name) : logger(logger) {
  this->logger.openNamedSection(name);
}

NamedSection::~NamedSection() {
  this->logger.closeNamedSection();
}

} // namespace arcana::noelle
