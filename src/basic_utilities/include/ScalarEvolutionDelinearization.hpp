/*
 * Copyright 2016 - 2020  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once;

#include "SystemHeaders.hpp"

namespace llvm {

  class ScalarEvolutionDelinearization {
    public:

      /*
       * TODO: This is from lib/Analysis/ScalarEvolution.cpp from LLVM 11 extracted here as this was implemented on LLVM 9
       * Once on LLVM 11, just use the API directly instead of this hack
       */
      static bool getIndexExpressionsFromGEP(
        ScalarEvolution &SE,
        const GetElementPtrInst *GEP,
        SmallVectorImpl<const SCEV *> &Subscripts,
        SmallVectorImpl<int> &Sizes
      ) ;

      /*
       * Implemented similarly to ScalarEvolution::computeAccessFunctions.
       * Added capabiliity to handle casts (by using cast handling SCEVDivision)
       */
      static void computeAccessFunctions(
        ScalarEvolution &SE,
        const SCEV *Expr,
        SmallVectorImpl<const SCEV *> &Subscripts,
        SmallVectorImpl<const SCEV *> &Sizes
      ) ;

      /*
       * Implemented similarly to ScalarEvolution::delinearize.
       * Uses improved computeAccessFunctions
       */
      static void delinearize(
        ScalarEvolution &SE,
        const SCEV *Expr,
        SmallVectorImpl<const SCEV *> &Subscripts,
        SmallVectorImpl<const SCEV *> &Sizes,
        const SCEV *ElementSize
      ) ;

  };

  /*
   * Since this namespace for SCEVDivision implemented in ScalarEvolution is anonymous,
   * we kept that pattern here
   */
  namespace {

    // Returns the size of the SCEV S.
    static inline int sizeOfSCEV(const SCEV *S) {
      struct FindSCEVSize {
        int Size = 0;

        FindSCEVSize() = default;

        bool follow(const SCEV *S) {
          ++Size;
          // Keep looking at all operands of S.
          return true;
        }

        bool isDone() const {
          return false;
        }
      };

      FindSCEVSize F;
      SCEVTraversal<FindSCEVSize> ST(F);
      ST.visitAll(S);
      return F.Size;
    }

    struct SCEVDivision : public SCEVVisitor<SCEVDivision, void> {
    public:
      // Computes the Quotient and Remainder of the division of Numerator by
      // Denominator.
      static void divide(ScalarEvolution &SE, const SCEV *Numerator,
                        const SCEV *Denominator, const SCEV **Quotient,
                        const SCEV **Remainder) {
        assert(Numerator && Denominator && "Uninitialized SCEV");

        SCEVDivision D(SE, Numerator, Denominator);

        // Check for the trivial case here to avoid having to check for it in the
        // rest of the code.
        if (Numerator == Denominator) {
          *Quotient = D.One;
          *Remainder = D.Zero;
          return;
        }

        if (Numerator->isZero()) {
          *Quotient = D.Zero;
          *Remainder = D.Zero;
          return;
        }

        // A simple case when N/1. The quotient is N.
        if (Denominator->isOne()) {
          *Quotient = Numerator;
          *Remainder = D.Zero;
          return;
        }

        // Split the Denominator when it is a product.
        if (const SCEVMulExpr *T = dyn_cast<SCEVMulExpr>(Denominator)) {
          const SCEV *Q, *R;
          *Quotient = Numerator;
          for (const SCEV *Op : T->operands()) {
            divide(SE, *Quotient, Op, &Q, &R);
            *Quotient = Q;

            // Bail out when the Numerator is not divisible by one of the terms of
            // the Denominator.
            if (!R->isZero()) {
              *Quotient = D.Zero;
              *Remainder = Numerator;
              return;
            }
          }
          *Remainder = D.Zero;
          return;
        }

        D.visit(Numerator);
        *Quotient = D.Quotient;
        *Remainder = D.Remainder;
      }

      // Except in the trivial case described above, we do not know how to divide
      // Expr by Denominator for the following functions with empty implementation.
      void visitUDivExpr(const SCEVUDivExpr *Numerator) {}
      void visitSMaxExpr(const SCEVSMaxExpr *Numerator) {}
      void visitUMaxExpr(const SCEVUMaxExpr *Numerator) {}
      void visitSMinExpr(const SCEVSMinExpr *Numerator) {}
      void visitUMinExpr(const SCEVUMinExpr *Numerator) {}
      void visitUnknown(const SCEVUnknown *Numerator) {}
      void visitCouldNotCompute(const SCEVCouldNotCompute *Numerator) {}

      // CUSTOM ADDITION: Divide operand of cast, ignoring cast
      void visitTruncateExpr(const SCEVTruncateExpr *Numerator) {
        // divide(SE, Numerator->getOperand(), Denominator, &Quotient, &Remainder);
      }

      // CUSTOM ADDITION: Divide operand of cast, ignoring cast
      void visitZeroExtendExpr(const SCEVZeroExtendExpr *Numerator) {
        // divide(SE, Numerator->getOperand(), Denominator, &Quotient, &Remainder);
      }

      // CUSTOM ADDITION: Divide operand of cast, ignoring cast
      void visitSignExtendExpr(const SCEVSignExtendExpr *Numerator) {
        // divide(SE, Numerator->getOperand(), Denominator, &Quotient, &Remainder);
      }

      void visitConstant(const SCEVConstant *Numerator) {
        if (const SCEVConstant *D = dyn_cast<SCEVConstant>(Denominator)) {
          APInt NumeratorVal = Numerator->getAPInt();
          APInt DenominatorVal = D->getAPInt();
          uint32_t NumeratorBW = NumeratorVal.getBitWidth();
          uint32_t DenominatorBW = DenominatorVal.getBitWidth();

          if (NumeratorBW > DenominatorBW)
            DenominatorVal = DenominatorVal.sext(NumeratorBW);
          else if (NumeratorBW < DenominatorBW)
            NumeratorVal = NumeratorVal.sext(DenominatorBW);

          APInt QuotientVal(NumeratorVal.getBitWidth(), 0);
          APInt RemainderVal(NumeratorVal.getBitWidth(), 0);
          APInt::sdivrem(NumeratorVal, DenominatorVal, QuotientVal, RemainderVal);
          Quotient = SE.getConstant(QuotientVal);
          Remainder = SE.getConstant(RemainderVal);
          return;
        }
      }

      void visitAddRecExpr(const SCEVAddRecExpr *Numerator) {
        const SCEV *StartQ, *StartR, *StepQ, *StepR;
        if (!Numerator->isAffine())
          return cannotDivide(Numerator);
        divide(SE, Numerator->getStart(), Denominator, &StartQ, &StartR);
        divide(SE, Numerator->getStepRecurrence(SE), Denominator, &StepQ, &StepR);
        // Bail out if the types do not match.
        Type *Ty = Denominator->getType();
        if (Ty != StartQ->getType() || Ty != StartR->getType() ||
            Ty != StepQ->getType() || Ty != StepR->getType())
          return cannotDivide(Numerator);
        Quotient = SE.getAddRecExpr(StartQ, StepQ, Numerator->getLoop(),
                                    Numerator->getNoWrapFlags());
        Remainder = SE.getAddRecExpr(StartR, StepR, Numerator->getLoop(),
                                    Numerator->getNoWrapFlags());
      }

      void visitAddExpr(const SCEVAddExpr *Numerator) {
        SmallVector<const SCEV *, 2> Qs, Rs;
        Type *Ty = Denominator->getType();

        for (const SCEV *Op : Numerator->operands()) {
          const SCEV *Q, *R;
          divide(SE, Op, Denominator, &Q, &R);

          // Bail out if types do not match.
          if (Ty != Q->getType() || Ty != R->getType())
            return cannotDivide(Numerator);

          Qs.push_back(Q);
          Rs.push_back(R);
        }

        if (Qs.size() == 1) {
          Quotient = Qs[0];
          Remainder = Rs[0];
          return;
        }

        Quotient = SE.getAddExpr(Qs);
        Remainder = SE.getAddExpr(Rs);
      }

      void visitMulExpr(const SCEVMulExpr *Numerator) {
        SmallVector<const SCEV *, 2> Qs;
        Type *Ty = Denominator->getType();

        bool FoundDenominatorTerm = false;
        for (const SCEV *Op : Numerator->operands()) {
          // Bail out if types do not match.
          if (Ty != Op->getType())
            return cannotDivide(Numerator);

          if (FoundDenominatorTerm) {
            Qs.push_back(Op);
            continue;
          }

          // Check whether Denominator divides one of the product operands.
          const SCEV *Q, *R;
          divide(SE, Op, Denominator, &Q, &R);
          if (!R->isZero()) {
            Qs.push_back(Op);
            continue;
          }

          // Bail out if types do not match.
          if (Ty != Q->getType())
            return cannotDivide(Numerator);

          FoundDenominatorTerm = true;
          Qs.push_back(Q);
        }

        if (FoundDenominatorTerm) {
          Remainder = Zero;
          if (Qs.size() == 1)
            Quotient = Qs[0];
          else
            Quotient = SE.getMulExpr(Qs);
          return;
        }

        if (!isa<SCEVUnknown>(Denominator))
          return cannotDivide(Numerator);

        // The Remainder is obtained by replacing Denominator by 0 in Numerator.
        ValueToValueMap RewriteMap;
        RewriteMap[cast<SCEVUnknown>(Denominator)->getValue()] =
            cast<SCEVConstant>(Zero)->getValue();
        Remainder = SCEVParameterRewriter::rewrite(Numerator, SE, RewriteMap, true);

        if (Remainder->isZero()) {
          // The Quotient is obtained by replacing Denominator by 1 in Numerator.
          RewriteMap[cast<SCEVUnknown>(Denominator)->getValue()] =
              cast<SCEVConstant>(One)->getValue();
          Quotient =
              SCEVParameterRewriter::rewrite(Numerator, SE, RewriteMap, true);
          return;
        }

        // Quotient is (Numerator - Remainder) divided by Denominator.
        const SCEV *Q, *R;
        const SCEV *Diff = SE.getMinusSCEV(Numerator, Remainder);
        // This SCEV does not seem to simplify: fail the division here.
        if (sizeOfSCEV(Diff) > sizeOfSCEV(Numerator))
          return cannotDivide(Numerator);
        divide(SE, Diff, Denominator, &Q, &R);
        if (R != Zero)
          return cannotDivide(Numerator);
        Quotient = Q;
      }

    private:
      SCEVDivision(ScalarEvolution &S, const SCEV *Numerator,
                  const SCEV *Denominator)
          : SE(S), Denominator(Denominator) {
        Zero = SE.getZero(Denominator->getType());
        One = SE.getOne(Denominator->getType());

        // We generally do not know how to divide Expr by Denominator. We
        // initialize the division to a "cannot divide" state to simplify the rest
        // of the code.
        cannotDivide(Numerator);
      }

      // Convenience function for giving up on the division. We set the quotient to
      // be equal to zero and the remainder to be equal to the numerator.
      void cannotDivide(const SCEV *Numerator) {
        Quotient = Zero;
        Remainder = Numerator;
      }

      ScalarEvolution &SE;
      const SCEV *Denominator, *Quotient, *Remainder, *Zero, *One;
    };


  }
}
