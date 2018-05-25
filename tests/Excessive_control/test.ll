; ModuleID = 'test.bc'
source_filename = "test.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

$_ZSt4sqrtf = comdat any

@.str = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1

; Function Attrs: noinline uwtable
define float @_Z16heavyComputationff(float, float) #0 {
  br label %3

; <label>:3:                                      ; preds = %30, %2
  %.04 = phi i32 [ 0, %2 ], [ %31, %30 ]
  %.01 = phi float [ %1, %2 ], [ %.3, %30 ]
  %.0 = phi float [ %0, %2 ], [ %.2, %30 ]
  %exitcond12 = icmp eq i32 %.04, 10
  br i1 %exitcond12, label %32, label %4

; <label>:4:                                      ; preds = %3
  %5 = icmp ugt i32 %.04, 3
  br i1 %5, label %6, label %18

; <label>:6:                                      ; preds = %4
  br label %7

; <label>:7:                                      ; preds = %16, %6
  %.05 = phi i32 [ 0, %6 ], [ %17, %16 ]
  %.1 = phi float [ %.0, %6 ], [ %.06.lcssa, %16 ]
  %exitcond10 = icmp eq i32 %.05, 10
  br i1 %exitcond10, label %.loopexit9, label %8

; <label>:8:                                      ; preds = %7
  %9 = fpext float %.1 to double
  %10 = fadd double %9, 1.430000e-01
  %11 = fptrunc double %10 to float
  br label %12

; <label>:12:                                     ; preds = %13, %8
  %.07 = phi i32 [ 0, %8 ], [ %15, %13 ]
  %.06 = phi float [ %11, %8 ], [ %14, %13 ]
  %exitcond = icmp eq i32 %.07, 3
  br i1 %exitcond, label %16, label %13

; <label>:13:                                     ; preds = %12
  %14 = tail call float @_ZSt4sqrtf(float %.06)
  %15 = add nuw nsw i32 %.07, 1
  br label %12

; <label>:16:                                     ; preds = %12
  %.06.lcssa = phi float [ %.06, %12 ]
  %17 = add nuw nsw i32 %.05, 1
  br label %7

.loopexit9:                                       ; preds = %7
  %.1.lcssa = phi float [ %.1, %7 ]
  br label %18

; <label>:18:                                     ; preds = %.loopexit9, %4
  %.2 = phi float [ %.0, %4 ], [ %.1.lcssa, %.loopexit9 ]
  %19 = icmp eq i32 %.04, 9
  %20 = fadd float %.01, %.2
  %..01 = select i1 %19, float %20, float %.01
  %21 = icmp ult i32 %.04, 7
  br i1 %21, label %22, label %30

; <label>:22:                                     ; preds = %18
  br label %23

; <label>:23:                                     ; preds = %24, %22
  %.08 = phi i32 [ 0, %22 ], [ %29, %24 ]
  %.23 = phi float [ %..01, %22 ], [ %28, %24 ]
  %exitcond11 = icmp eq i32 %.08, 5
  br i1 %exitcond11, label %.loopexit, label %24

; <label>:24:                                     ; preds = %23
  %25 = fpext float %.23 to double
  %26 = fadd double %25, -1.980000e-01
  %27 = fptrunc double %26 to float
  %28 = tail call float @_ZSt4sqrtf(float %27)
  %29 = add nuw nsw i32 %.08, 1
  br label %23

.loopexit:                                        ; preds = %23
  %.23.lcssa = phi float [ %.23, %23 ]
  br label %30

; <label>:30:                                     ; preds = %.loopexit, %18
  %.3 = phi float [ %..01, %18 ], [ %.23.lcssa, %.loopexit ]
  %31 = add nuw nsw i32 %.04, 1
  br label %3

; <label>:32:                                     ; preds = %3
  %.0.lcssa = phi float [ %.0, %3 ]
  ret float %.0.lcssa
}

; Function Attrs: noinline nounwind uwtable
define linkonce_odr float @_ZSt4sqrtf(float) #1 comdat {
  %2 = tail call float @sqrtf(float %0) #5
  ret float %2
}

; Function Attrs: noinline norecurse uwtable
define i32 @main() #2 {
  %1 = tail call float @_Z16heavyComputationff(float 0x4041A66660000000, float 0x4038B33340000000)
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
