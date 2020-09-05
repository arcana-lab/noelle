/*
 * Copyright 2016 - 2020  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "ScalarEvolutionDelinearization.hpp"

using namespace llvm;

bool ScalarEvolutionDelinearization::getIndexExpressionsFromGEP(
  ScalarEvolution &SE,
  const GetElementPtrInst *GEP,
  SmallVectorImpl<const SCEV *> &Subscripts,
  SmallVectorImpl<int> &Sizes
) {
   assert(Subscripts.empty() && Sizes.empty() &&
          "Expected output lists to be empty on entry to this function.");
   assert(GEP && "getIndexExpressionsFromGEP called with a null GEP");
   Type *Ty = GEP->getPointerOperandType();
   bool DroppedFirstDim = false;
   for (unsigned i = 1; i < GEP->getNumOperands(); i++) {
     const SCEV *Expr = SE.getSCEV(GEP->getOperand(i));
     if (i == 1) {
       if (auto *PtrTy = dyn_cast<PointerType>(Ty)) {
         Ty = PtrTy->getElementType();
       } else if (auto *ArrayTy = dyn_cast<ArrayType>(Ty)) {
         Ty = ArrayTy->getElementType();
       } else {
         Subscripts.clear();
         Sizes.clear();
         return false;
       }
       if (auto *Const = dyn_cast<SCEVConstant>(Expr))
         if (Const->getValue()->isZero()) {
           DroppedFirstDim = true;
           continue;
         }
       Subscripts.push_back(Expr);
       continue;
     }
 
     auto *ArrayTy = dyn_cast<ArrayType>(Ty);
     if (!ArrayTy) {
       Subscripts.clear();
       Sizes.clear();
       return false;
     }
 
     Subscripts.push_back(Expr);
     if (!(DroppedFirstDim && i == 2))
       Sizes.push_back(ArrayTy->getNumElements());
 
     Ty = ArrayTy->getElementType();
   }
   return !Subscripts.empty();
 }
 
void ScalarEvolutionDelinearization::computeAccessFunctions(
  ScalarEvolution &SE,
  const SCEV *Expr,
  SmallVectorImpl<const SCEV *> &Subscripts,
  SmallVectorImpl<const SCEV *> &Sizes
) {
  // Early exit in case this SCEV is not an affine multivariate function.
  if (Sizes.empty())
    return;

  if (auto *AR = dyn_cast<SCEVAddRecExpr>(Expr))
    if (!AR->isAffine())
      return;

  /*
   * HACK: Ignore casts on subscripts
   * This is extremely dangerous for small integer sizes, so we guard
   * against any integer sizes < 32
   */
  std::function<const SCEV *(const SCEV *Res, const SCEV *Den)> peelCasts;
  peelCasts = [&peelCasts, &SE](const SCEV *Res, const SCEV *Den) -> const SCEV * {

    auto originalRes = Res;

    if (Res->getType()->getPrimitiveSizeInBits() < 32) return originalRes;

    // Res->print(errs() << "Res: " << Den->getType()->getIntegerBitWidth() << " "); errs() << "\n";
    // Den->print(errs() << "Den: " << Den->getType()->getIntegerBitWidth() << " "); errs() << "\n";

    if (isa<SCEVSignExtendExpr>(Res) || isa<SCEVTruncateExpr>(Res) || isa<SCEVZeroExtendExpr>(Res)) {
      Res = peelCasts(dyn_cast<SCEVCastExpr>(Res)->getOperand(), Den);
      if (Res->getType()->getPrimitiveSizeInBits() < 32) return originalRes;
    }

    if (isa<SCEVAddExpr>(Res) || isa<SCEVMulExpr>(Res) || isa<SCEVAddRecExpr>(Res)) {
      auto naryResult = dyn_cast<SCEVNAryExpr>(Res);
      SmallVector<const SCEV *, 4> ops;
      bool changedOperand = false;
      for (auto i = 0; i < naryResult->getNumOperands(); ++i) {
        auto opI = naryResult->getOperand(i);
        // opI->print(errs() << "\tOp: " << opI->getType()->getIntegerBitWidth() << " "); errs() << "\n";

        if (isa<SCEVSignExtendExpr>(opI) || isa<SCEVTruncateExpr>(opI) || isa<SCEVZeroExtendExpr>(opI)) {
          opI = dyn_cast<SCEVCastExpr>(opI)->getOperand();
          changedOperand = true;
        }

        if (isa<SCEVConstant>(opI) && opI->getType() != Den->getType()) {
          auto oldOpI = cast<SCEVConstant>(opI);
          opI = SE.getConstant(Den->getType(), oldOpI->getValue()->getSExtValue());
          changedOperand = true;
        }

        if (isa<SCEVAddExpr>(opI) || isa<SCEVMulExpr>(opI) || isa<SCEVAddRecExpr>(opI)) {
          auto oldOpI = opI;
          opI = peelCasts(opI, Den);
          changedOperand |= oldOpI != opI;
        }

        if (opI->getType() != Den->getType()) {
          // opI->print(errs() << "NOT DENOM TYPE: "); errs() << "\n";
          changedOperand = false;
          break;
        }

        auto oldOpI = opI;
        opI = peelCasts(opI, Den);
        changedOperand |= oldOpI != opI;

        if (opI->getType()->getPrimitiveSizeInBits() < 32) return originalRes;

        ops.push_back(opI);
      }

      if (changedOperand) {
        const SCEVAddRecExpr *addRecExpr;
        llvm::SCEV::NoWrapFlags flags = SCEVAddRecExpr::NoWrapMask;
        switch (Res->getSCEVType()) {
          case scAddRecExpr:
            addRecExpr = cast<SCEVAddRecExpr>(Res);
            Res = SE.getAddRecExpr(ops, addRecExpr->getLoop(), flags);
            break;
          case scAddExpr:
            Res = SE.getAddExpr(ops, flags);
            break;
          case scMulExpr:
            Res = SE.getMulExpr(ops, flags);
            break;
        }
      }
    }

    // Res->print(errs() << "\tRES peeled: "); errs() << "\n";

    return Res;
  };

  const SCEV *Res = Expr;
  int Last = Sizes.size() - 1;
  for (int i = Last; i >= 0; i--) {

    const SCEV *Q, *R;
    // Res = peelCasts(Res, Sizes[i]);
    SCEVDivision::divide(SE, Res, Sizes[i], &Q, &R);

    // LLVM_DEBUG({
    //   dbgs() << "Res: " << *Res << "\n";
    //   dbgs() << "Sizes[i]: " << *Sizes[i] << "\n";
    //   dbgs() << "Res divided by Sizes[i]:\n";
    //   dbgs() << "Quotient: " << *Q << "\n";
    //   dbgs() << "Remainder: " << *R << "\n";
    // });

    Res = Q;

    // Do not record the last subscript corresponding to the size of elements in
    // the array.
    if (i == Last) {

      // Bail out if the remainder is too complex.
      if (isa<SCEVAddRecExpr>(R)) {
        Subscripts.clear();
        Sizes.clear();
        return;
      }

      continue;
    }

    // Record the access function for the current subscript.
    Subscripts.push_back(R);
  }

  // Also push in last position the remainder of the last division: it will be
  // the access function of the innermost dimension.
  Subscripts.push_back(Res);

  std::reverse(Subscripts.begin(), Subscripts.end());

  // LLVM_DEBUG({
  //   dbgs() << "Subscripts:\n";
  //   for (const SCEV *S : Subscripts)
  //     dbgs() << *S << "\n";
  // });
}

void ScalarEvolutionDelinearization::delinearize(
  ScalarEvolution &SE,
  const SCEV *Expr,
  SmallVectorImpl<const SCEV *> &Subscripts,
  SmallVectorImpl<const SCEV *> &Sizes,
  const SCEV *ElementSize
) {
  // First step: collect parametric terms.
  SmallVector<const SCEV *, 4> Terms;
  SE.collectParametricTerms(Expr, Terms);

  if (Terms.empty())
    return;

  // Second step: find subscript sizes.
  SE.findArrayDimensions(Terms, Sizes, ElementSize);

  if (Sizes.empty())
    return;

  // Third step: compute the access functions for each subscript.
  ScalarEvolutionDelinearization::computeAccessFunctions(SE, Expr, Subscripts, Sizes);

  if (Subscripts.empty())
    return;

  // LLVM_DEBUG({
  //   dbgs() << "succeeded to delinearize " << *Expr << "\n";
  //   dbgs() << "ArrayDecl[UnknownSize]";
  //   for (const SCEV *S : Sizes)
  //     dbgs() << "[" << *S << "]";

  //   dbgs() << "\nArrayRef";
  //   for (const SCEV *S : Subscripts)
  //     dbgs() << "[" << *S << "]";
  //   dbgs() << "\n";
  // });
 }