; ModuleID = 'test.bc'
source_filename = "test.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1

; Function Attrs: noinline nounwind uwtable
define double @_Z16heavyComputationd(double) #0 {
  br label %2

; <label>:2:                                      ; preds = %13, %1
  %.01 = phi i32 [ 0, %1 ], [ %14, %13 ]
  %.0 = phi double [ %0, %1 ], [ %.1.lcssa, %13 ]
  %exitcond6 = icmp eq i32 %.01, 10
  br i1 %exitcond6, label %15, label %3

; <label>:3:                                      ; preds = %2
  br label %4

; <label>:4:                                      ; preds = %11, %3
  %.02 = phi i32 [ 0, %3 ], [ %12, %11 ]
  %.1 = phi double [ %.0, %3 ], [ %.03.lcssa, %11 ]
  %exitcond5 = icmp eq i32 %.02, 10
  br i1 %exitcond5, label %13, label %5

; <label>:5:                                      ; preds = %4
  %6 = fadd double %.1, 1.430000e-01
  br label %7

; <label>:7:                                      ; preds = %8, %5
  %.04 = phi i32 [ 0, %5 ], [ %10, %8 ]
  %.03 = phi double [ %6, %5 ], [ %9, %8 ]
  %exitcond = icmp eq i32 %.04, 3
  br i1 %exitcond, label %11, label %8

; <label>:8:                                      ; preds = %7
  %9 = tail call double @sqrt(double %.03) #4
  %10 = add nuw nsw i32 %.04, 1
  br label %7

; <label>:11:                                     ; preds = %7
  %.03.lcssa = phi double [ %.03, %7 ]
  %12 = add nuw nsw i32 %.02, 1
  br label %4

; <label>:13:                                     ; preds = %4
  %.1.lcssa = phi double [ %.1, %4 ]
  %14 = add nuw nsw i32 %.01, 1
  br label %2

; <label>:15:                                     ; preds = %2
  %.0.lcssa = phi double [ %.0, %2 ]
  ret double %.0.lcssa
}

; Function Attrs: nounwind
declare double @sqrt(double) #1

; Function Attrs: noinline norecurse uwtable
define i32 @main() #2 {
  %1 = tail call double @_Z16heavyComputationd(double 3.530000e+01)
  %2 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), double %1)
  ret i32 0
}

declare i32 @printf(i8*, ...) #3

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { noinline norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 5.0.0 (tags/RELEASE_500/final)"}
