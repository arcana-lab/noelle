/*
 * Copyright 2021 - 2022  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/LoopStructure.hpp"
#include "noelle/core/MetadataEntry.hpp"

namespace llvm::noelle {

  class MetadataManager {
    public:
      MetadataManager (Module &M);

      /*
       * Return true if the module has the metadata requested.
       */
      bool doesHaveMetadata (
        const std::string &metadataName
        ) const ;

      /*
       * Add metadata to the module.
       *
       * Warning: this modifies the IR code.
       */
      void addMetadata (
        const std::string &metadataName,
        const std::string &metadataValue
        ) ;

      /*
       * Return true if the loop has the metadata requested.
       */
      bool doesHaveMetadata (
        LoopStructure *loop,
        const std::string &metadataName
        ) ;

      /*
       * Fetch the metadata attached to the loop.
       */
      std::string getMetadata (
        LoopStructure *loop,
        const std::string &metadataName
        ) ;

      /*
       * Add metadata to the loop.
       *
       * Warning: this modifies the IR code.
       */
      void addMetadata (
        LoopStructure *loop,
        const std::string &metadataName, 
        const std::string &metadataValue
        ) ;

      /*
       * Set an existing metadata of a loop.
       *
       * Warning: this modifies the IR code.
       */
      void setMetadata (
        LoopStructure *loop,
        const std::string &metadataName, 
        const std::string &metadataValue
        ) ;

      /*
       * Delete metadata of a loop.
       *
       * Warning: this modifies the IR code.
       */
      void deleteMetadata (
        LoopStructure *loop,
        const std::string &metadataName, 
        const std::string &metadataValue
        ) ;

    private:
      Module &program;
      std::unordered_map<LoopStructure *, std::unordered_map<std::string, MetadataEntry *>> metadata;

      void addMetadata (
        LoopStructure *loop,
        const std::string &metadataName
        );

  };

}
