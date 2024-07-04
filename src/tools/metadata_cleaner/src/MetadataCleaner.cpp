/*
 * Copyright 2016 - 2024  Yian Su, Simone Campanoni
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
#include "MetadataCleaner.hpp"

namespace arcana::noelle {

void MetadataCleaner::cleanLoopMetadata(Module &M) {
  errs() << "noelle/core/Clean Loop Metadata\n";

  for (auto &F : M) {
    for (auto &B : F) {
      for (auto &I : B) {
        if (I.getMetadata("noelle.loop.id")) {
          I.setMetadata("noelle.loop.id", nullptr);
        }
      }
    }
  }

  return;
}

void MetadataCleaner::cleanPDGMetadata(Module &M) {
  errs() << "noelle/core/Clean PDG Metadata\n";

  for (auto &F : M) {
    if (F.hasMetadata("noelle.pdg.args.id")) {
      F.setMetadata("noelle.pdg.args.id", nullptr);
    }
    if (F.hasMetadata("noelle.pdg.edges")) {
      F.setMetadata("noelle.pdg.edges", nullptr);
    }

    for (auto &B : F) {
      for (auto &I : B) {
        if (I.getMetadata("noelle.pdg.inst.id")) {
          I.setMetadata("noelle.pdg.inst.id", nullptr);
        }
      }
    }
  }

  if (auto n = M.getNamedMetadata("noelle.module.pdg")) {
    M.eraseNamedMetadata(n);
  }

  return;
}

void MetadataCleaner::cleanSCCMetadata(Module &M) {
  errs() << "noelle/core/Clean PDG-SCC Metadata\n";

  for (auto &F : M) {
    for (auto &B : F) {
      for (auto &I : B) {
        if (I.getMetadata("noelle.pdg.scc.id")) {
          I.setMetadata("noelle.pdg.scc.id", nullptr);
        }
      }
    }
  }

  return;
}

void MetadataCleaner::cleanProfMetadata(Module &M) {
  errs() << "noelle/core/Clean profiler metadata\n";

  for (auto &F : M) {
    if (F.hasMetadata("prof")) {
      F.setMetadata("prof", nullptr);
    }
    if (F.hasMetadata("PGOFuncName")) {
      F.setMetadata("PGOFuncName", nullptr);
    }

    for (auto &B : F) {
      for (auto &I : B) {
        if (I.getMetadata("prof")) {
          I.setMetadata("prof", nullptr);
        }
      }
    }
  }

  return;
}

} // namespace arcana::noelle
