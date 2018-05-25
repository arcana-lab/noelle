; ModuleID = 'test.bc'
source_filename = "test.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

$_ZSt4sqrtf = comdat any

@.str = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1

; Function Attrs: noinline uwtable
define float @_Z16heavyComputationf(float) #0 {
  br label %2

; <label>:2:                                      ; preds = %15, %1
  %.01 = phi i32 [ 0, %1 ], [ %16, %15 ]
  %.0 = phi float [ %0, %1 ], [ %.1.lcssa, %15 ]
  %exitcond6 = icmp eq i32 %.01, 10
  br i1 %exitcond6, label %17, label %3

; <label>:3:                                      ; preds = %2
  br label %4

; <label>:4:                                      ; preds = %13, %3
  %.02 = phi i32 [ 0, %3 ], [ %14, %13 ]
  %.1 = phi float [ %.0, %3 ], [ %.03.lcssa, %13 ]
  %exitcond5 = icmp eq i32 %.02, 10
  br i1 %exitcond5, label %15, label %5

; <label>:5:                                      ; preds = %4
  %6 = fpext float %.1 to double
  %7 = fadd double %6, 1.430000e-01
  %8 = fptrunc double %7 to float
  br label %9

; <label>:9:                                      ; preds = %10, %5
  %.04 = phi i32 [ 0, %5 ], [ %12, %10 ]
  %.03 = phi float [ %8, %5 ], [ %11, %10 ]
  %exitcond = icmp eq i32 %.04, 3
  br i1 %exitcond, label %13, label %10

; <label>:10:                                     ; preds = %9
  %11 = tail call float @_ZSt4sqrtf(float %.03)
  %12 = add nuw nsw i32 %.04, 1
  br label %9

; <label>:13:                                     ; preds = %9
  %.03.lcssa = phi float [ %.03, %9 ]
  %14 = add nuw nsw i32 %.02, 1
  br label %4

; <label>:15:                                     ; preds = %4
  %.1.lcssa = phi float [ %.1, %4 ]
  %16 = add nuw nsw i32 %.01, 1
  br label %2

; <label>:17:                                     ; preds = %2
  %.0.lcssa = phi float [ %.0, %2 ]
  ret float %.0.lcssa
}

; Function Attrs: noinline nounwind uwtable
define linkonce_odr float @_ZSt4sqrtf(float) #1 comdat {
  %2 = tail call float @sqrtf(float %0) #5
  ret float %2
}

; Function Attrs: noinline norecurse uwtable
define i32 @main() #2 {
  %1 = tail call float @_Z16heavyComputationf(float 0x4041A66660000000)
  %2 = fpext float %1 to double
  %3 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), double %2)
  ret i32 0
}

declare i32 @printf(i8*, ...) #3

; Function Attrs: nounwind readnone
declare float @sqrtf(float) #4

attributes #0 = { noinline uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { noinline norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { nounwind readnone }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 5.0.0 (tags/RELEASE_500/final)"}
