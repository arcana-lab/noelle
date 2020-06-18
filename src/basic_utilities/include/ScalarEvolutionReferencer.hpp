/*
 * Copyright 2016 - 2020  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "SystemHeaders.hpp"

namespace llvm {

  class SCEVReference;
  class SCEVValueMapper;

  class ScalarEvolutionReferentialExpander {
    public:
      ScalarEvolutionReferentialExpander (ScalarEvolution &SE, Function &F) ;
      ~ScalarEvolutionReferentialExpander () ;

      SCEVReference *createReferenceTree (const SCEV *scev, std::set<Value *> valuesInScope);

      Value *expandUsingReferenceValues (
        SCEVReference *tree,
        std::set<Value *> valuesToReferenceAndNotExpand,
        IRBuilder<> &expansionBuilder
      );

    private:
      SCEVValueMapper *scevValueMapper;
  };

  class ReferenceTreeExpander : SCEVVisitor<ReferenceTreeExpander, Value *> {
    public:
      ReferenceTreeExpander (SCEVReference *tree, std::set<Value *> &valuesToReferenceAndNotExpand, IRBuilder<> &expansionBuilder) ;

      Value *getRootOfTree () ;

    private:
      friend class SCEVVisitor<ReferenceTreeExpander, Value *>;

      Value *visitConstant (const SCEVConstant *S) ;
      Value *visitTruncateExpr (const SCEVTruncateExpr *S) ;
      Value *visitZeroExtendExpr (const SCEVZeroExtendExpr *S) ;
      Value *visitSignExtendExpr (const SCEVSignExtendExpr *S) ;
      Value *visitAddExpr (const SCEVAddExpr *S) ;
      Value *visitMulExpr (const SCEVMulExpr *S) ;
      Value *visitUDivExpr (const SCEVUDivExpr *S) ;
      Value *visitAddRecExpr (const SCEVAddRecExpr *S) ;
      Value *visitSMaxExpr (const SCEVSMaxExpr *S) ;
      Value *visitUMaxExpr (const SCEVUMaxExpr *S) ;
      Value *visitSMinExpr (const SCEVSMinExpr *S) ;
      Value *visitUMinExpr (const SCEVUMinExpr *S) ;
      Value *visitUnknown (const SCEVUnknown *S) ;
      Value *visitCouldNotCompute (const SCEVCouldNotCompute* S) ;

      std::pair<Value *, Value *> visitTwoOperands (const SCEVNAryExpr *S) ;

      Value *rootValue;
      SCEVReference *currentNode;
      std::set<Value *> &valuesToReferenceAndNotExpand;
      IRBuilder<> &expansionBuilder;
  };

  class ReferenceTreeBuilder : SCEVVisitor<ReferenceTreeBuilder, SCEVReference *> {
    public:
      ReferenceTreeBuilder (const SCEV *scev, SCEVValueMapper &scevValueMapper, std::set<Value *> &valuesInScope) ;

      SCEVReference *getTree () ;

    private:
      friend class SCEVVisitor<ReferenceTreeBuilder, SCEVReference *>;

      SCEVReference *visitConstant (const SCEVConstant *S) ;
      SCEVReference *visitTruncateExpr (const SCEVTruncateExpr *S) ;
      SCEVReference *visitZeroExtendExpr (const SCEVZeroExtendExpr *S) ;
      SCEVReference *visitSignExtendExpr (const SCEVSignExtendExpr *S) ;
      SCEVReference *visitAddExpr (const SCEVAddExpr *S) ;
      SCEVReference *visitMulExpr (const SCEVMulExpr *S) ;
      SCEVReference *visitUDivExpr (const SCEVUDivExpr *S) ;
      SCEVReference *visitAddRecExpr (const SCEVAddRecExpr *S) ;
      SCEVReference *visitSMaxExpr (const SCEVSMaxExpr *S) ;
      SCEVReference *visitUMaxExpr (const SCEVUMaxExpr *S) ;
      SCEVReference *visitSMinExpr (const SCEVSMinExpr *S) ;
      SCEVReference *visitUMinExpr (const SCEVUMinExpr *S) ;
      SCEVReference *visitUnknown (const SCEVUnknown *S) ;
      SCEVReference *visitCouldNotCompute (const SCEVCouldNotCompute* S) ;

      Value * mapToSingleInScopeValue (const SCEV *S) ;
      SCEVReference *createReferenceOfSingleInScopeValue (const SCEV *S) ;
      SCEVReference *createReferenceOfNArySCEV (const SCEVNAryExpr *S) ;

      SCEVReference *tree;
      std::set<Value *> &valuesInScope;
      SCEVValueMapper &scevValueMapper;
  };

  class SCEVValueMapper {
    public:
      SCEVValueMapper (ScalarEvolution &SE, Function &F) ;

      Value *getSingleValueOf(const SCEV *scev) const ;
      const std::set<Value *> getValuesOf(const SCEV *scev) const ;
      const SCEV *getSCEVOf(Value *value) const ;

    private:
      std::unordered_map<const SCEV *, std::set<Value *>> scevToValues;
      std::unordered_map<Value *, const SCEV *> valueToSCEV;
  };

  class SCEVReference {
    public:
      SCEVReference(Value *V, const SCEV *scev) ;
      ~SCEVReference() ;

      Value *getValue() const ;
      const SCEV *getSCEV() const ;
      iterator_range<std::vector<SCEVReference *>::iterator> getChildReferences() ;
      SCEVReference *getChildReference (int32_t idx) ;
      int32_t getNumChildReferences() ;

      void addChildReference(SCEVReference *scevReference) ;

      std::set<SCEVReference *> collectAllReferences () ;

    private:
      Value *value;
      const SCEV *scev;
      std::vector<SCEVReference *> childReferences;
  };
}