/*
 * Copyright 2021  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "SystemHeaders.hpp"
#include "LoopStructure.hpp"
#include "MetadataEntry.hpp"

namespace llvm::noelle {

  class MetadataManager {
    public:
      MetadataManager ();

      /*
       * Return true if the loop has the metadata requested.
       */
      bool doesHaveMetadata (
        LoopStructure *loop,
        const std::string &metadataName
        ) const ;

      /*
       * Fetch the metadata attached to the loop.
       */
      std::string getMetadata (
        LoopStructure *loop,
        const std::string &metadataName
        ) const ;

      /*
       * Add metadata to the loop.
       *
       * Warning: this modifies the IR code.
       */
      void setMetadata (
        LoopStructure *loop,
        const std::string &metadataName, 
        const std::string &metadataValue
        ) ;

    private:
      std::unordered_map<LoopStructure *, std::unordered_map<std::string, MetadataEntry *>> metadata;

      void addMetadata (
        LoopStructure *loop,
        const std::string &metadataName
        );

  };

}
