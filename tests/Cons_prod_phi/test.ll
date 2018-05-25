; ModuleID = 'test.bc'
source_filename = "test.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [8 x i8] c"%d, %d\0A\00", align 1

; Function Attrs: noinline norecurse uwtable
define i32 @main(i32, i8**) #0 {
  %3 = mul nsw i32 %0, 5
  %4 = sdiv i32 %3, 2
  %5 = add nsw i32 %4, 1
  br label %6

; <label>:6:                                      ; preds = %15, %2
  %.04 = phi i32 [ %0, %2 ], [ %16, %15 ]
  %.02 = phi i32 [ %5, %2 ], [ %11, %15 ]
  %.01 = phi i32 [ 3, %2 ], [ %.02, %15 ]
  %.0 = phi i32 [ 0, %2 ], [ %17, %15 ]
  %7 = icmp ult i32 %.0, 100
  br i1 %7, label %8, label %.loopexit

; <label>:8:                                      ; preds = %6
  %9 = sub nsw i32 %.02, %.04
  %10 = mul nsw i32 %9, 3
  %11 = sdiv i32 %10, 2
  %12 = icmp sgt i32 %.04, 6
  br i1 %12, label %13, label %15

; <label>:13:                                     ; preds = %8
  %.lcssa = phi i32 [ %11, %8 ]
  %.02.lcssa13 = phi i32 [ %.02, %8 ]
  %14 = add nsw i32 %.lcssa, -4
  br label %18

; <label>:15:                                     ; preds = %8
  %16 = add nsw i32 %.04, 6
  %17 = add nuw nsw i32 %.0, 1
  br label %6

.loopexit:                                        ; preds = %6
  %.04.lcssa = phi i32 [ %.04, %6 ]
  %.02.lcssa = phi i32 [ %.02, %6 ]
  %.01.lcssa = phi i32 [ %.01, %6 ]
  br label %18

; <label>:18:                                     ; preds = %.loopexit, %13
  %.15 = phi i32 [ %14, %13 ], [ %.04.lcssa, %.loopexit ]
  %.13 = phi i32 [ %.lcssa, %13 ], [ %.02.lcssa, %.loopexit ]
  %.1 = phi i32 [ %.02.lcssa13, %13 ], [ %.01.lcssa, %.loopexit ]
  %19 = add nsw i32 %.13, %.1
  %20 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i64 0, i64 0), i32 %.15, i32 %19)
  ret i32 0
}

declare i32 @printf(i8*, ...) #1

attributes #0 = { noinline norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 5.0.0 (tags/RELEASE_500/final)"}
