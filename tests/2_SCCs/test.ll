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

; <label>:6:                                      ; preds = %7, %2
  %.02 = phi i32 [ %0, %2 ], [ %8, %7 ]
  %.01 = phi i32 [ %5, %2 ], [ %11, %7 ]
  %.0 = phi i32 [ 0, %2 ], [ %12, %7 ]
  %exitcond = icmp eq i32 %.0, 100
  br i1 %exitcond, label %13, label %7

; <label>:7:                                      ; preds = %6
  %8 = add nsw i32 %.02, -2
  %9 = sub nsw i32 %.01, %.02
  %10 = mul nsw i32 %9, 3
  %11 = sdiv i32 %10, 2
  %12 = add nuw nsw i32 %.0, 1
  br label %6

; <label>:13:                                     ; preds = %6
  %.01.lcssa = phi i32 [ %.01, %6 ]
  %14 = add i32 %0, -200
  %15 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i64 0, i64 0), i32 %14, i32 %.01.lcssa)
  ret i32 0
}

declare i32 @printf(i8*, ...) #1

attributes #0 = { noinline norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 5.0.0 (tags/RELEASE_500/final)"}
