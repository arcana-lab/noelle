; ModuleID = 'test.bc'
source_filename = "test.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [8 x i8] c"%f, %f\0A\00", align 1

; Function Attrs: noinline norecurse uwtable
define i32 @main(i32, i8**) #0 {
  %3 = sitofp i32 %0 to double
  %4 = fadd double %3, 3.100000e-01
  %5 = mul nsw i32 %0, 5
  %6 = sdiv i32 %5, 2
  %7 = sitofp i32 %6 to double
  %8 = fadd double %7, 1.532400e+00
  br label %9

; <label>:9:                                      ; preds = %17, %2
  %.03 = phi double [ %4, %2 ], [ %.1.lcssa, %17 ]
  %.02 = phi double [ %8, %2 ], [ %18, %17 ]
  %.01 = phi i32 [ 0, %2 ], [ %19, %17 ]
  %exitcond4 = icmp eq i32 %.01, 100
  br i1 %exitcond4, label %20, label %10

; <label>:10:                                     ; preds = %9
  br label %11

; <label>:11:                                     ; preds = %12, %10
  %.1 = phi double [ %.03, %10 ], [ %15, %12 ]
  %.0 = phi i32 [ 0, %10 ], [ %16, %12 ]
  %exitcond = icmp eq i32 %.0, 10
  br i1 %exitcond, label %17, label %12

; <label>:12:                                     ; preds = %11
  %13 = uitofp i32 %.0 to double
  %14 = fmul double %13, 2.521000e+01
  %15 = fadd double %.1, %14
  %16 = add nuw nsw i32 %.0, 1
  br label %11

; <label>:17:                                     ; preds = %11
  %.1.lcssa = phi double [ %.1, %11 ]
  %18 = fadd double %.02, %.1.lcssa
  %19 = add nuw nsw i32 %.01, 1
  br label %9

; <label>:20:                                     ; preds = %9
  %.03.lcssa = phi double [ %.03, %9 ]
  %.02.lcssa = phi double [ %.02, %9 ]
  %21 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i64 0, i64 0), double %.03.lcssa, double %.02.lcssa)
  ret i32 0
}

declare i32 @printf(i8*, ...) #1

attributes #0 = { noinline norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 5.0.0 (tags/RELEASE_500/final)"}
