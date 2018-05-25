; ModuleID = 'test.bc'
source_filename = "test.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [8 x i8] c"%f, %f\0A\00", align 1

; Function Attrs: noinline uwtable
define void @_Z1fPdd(double*, double) #0 {
  br label %3

; <label>:3:                                      ; preds = %12, %2
  %.02 = phi i32 [ 0, %2 ], [ %15, %12 ]
  %.01 = phi double [ %1, %2 ], [ %14, %12 ]
  %exitcond3 = icmp eq i32 %.02, 100
  br i1 %exitcond3, label %16, label %4

; <label>:4:                                      ; preds = %3
  br label %5

; <label>:5:                                      ; preds = %6, %4
  %.0 = phi i32 [ 0, %4 ], [ %11, %6 ]
  %exitcond = icmp eq i32 %.0, 10
  br i1 %exitcond, label %12, label %6

; <label>:6:                                      ; preds = %5
  %7 = uitofp i32 %.0 to double
  %8 = fmul double %7, 2.521000e+01
  %9 = load double, double* %0, align 8
  %10 = fadd double %9, %8
  store double %10, double* %0, align 8
  %11 = add nuw nsw i32 %.0, 1
  br label %5

; <label>:12:                                     ; preds = %5
  %13 = load double, double* %0, align 8
  %14 = fadd double %.01, %13
  %15 = add nuw nsw i32 %.02, 1
  br label %3

; <label>:16:                                     ; preds = %3
  %.01.lcssa = phi double [ %.01, %3 ]
  %17 = load double, double* %0, align 8
  %18 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i64 0, i64 0), double %17, double %.01.lcssa)
  ret void
}

declare i32 @printf(i8*, ...) #1

; Function Attrs: noinline norecurse uwtable
define i32 @main(i32, i8**) #2 {
  %3 = alloca double, align 8
  %4 = sitofp i32 %0 to double
  %5 = fadd double %4, 3.100000e-01
  store double %5, double* %3, align 8
  %6 = mul nsw i32 %0, 5
  %7 = sdiv i32 %6, 2
  %8 = sitofp i32 %7 to double
  %9 = fadd double %8, 1.532400e+00
  call void @_Z1fPdd(double* nonnull %3, double %9)
  ret i32 0
}

attributes #0 = { noinline uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { noinline norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 5.0.0 (tags/RELEASE_500/final)"}
