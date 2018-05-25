; ModuleID = 'test.bc'
source_filename = "test.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._N = type { i32, %struct._N* }

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: noinline nounwind uwtable
define i32 @_Z16heavyComputationi(i32) #0 {
  %2 = sitofp i32 %0 to double
  %3 = fadd double %2, 1.430000e-01
  %4 = tail call double @sqrt(double %3) #5
  %5 = fptosi double %4 to i32
  ret i32 %5
}

; Function Attrs: nounwind
declare double @sqrt(double) #1

; Function Attrs: noinline uwtable
define void @_Z10appendNodeP2_Nii(%struct._N*, i32, i32) #2 {
  br label %tailrecurse

tailrecurse:                                      ; preds = %11, %3
  %.tr = phi %struct._N* [ %0, %3 ], [ %12, %11 ]
  %.tr1 = phi i32 [ %1, %3 ], [ %13, %11 ]
  %.tr2 = phi i32 [ %2, %3 ], [ %14, %11 ]
  %4 = tail call noalias i8* @malloc(i64 16) #5
  %5 = bitcast i8* %4 to i32*
  store i32 %.tr1, i32* %5, align 8
  %6 = getelementptr inbounds i8, i8* %4, i64 8
  %7 = bitcast i8* %6 to %struct._N**
  store %struct._N* null, %struct._N** %7, align 8
  %8 = getelementptr inbounds %struct._N, %struct._N* %.tr, i64 0, i32 1
  %9 = bitcast %struct._N** %8 to i8**
  store i8* %4, i8** %9, align 8
  %10 = icmp sgt i32 %.tr2, 0
  br i1 %10, label %11, label %15

; <label>:11:                                     ; preds = %tailrecurse
  %12 = bitcast i8* %4 to %struct._N*
  %13 = add nsw i32 %.tr1, 1
  %14 = add nsw i32 %.tr2, -1
  br label %tailrecurse

; <label>:15:                                     ; preds = %tailrecurse
  ret void
}

; Function Attrs: nounwind
declare noalias i8* @malloc(i64) #1

; Function Attrs: noinline norecurse uwtable
define i32 @main() #3 {
  %1 = tail call noalias i8* @malloc(i64 16) #5
  %2 = bitcast i8* %1 to %struct._N*
  tail call void @_Z10appendNodeP2_Nii(%struct._N* %2, i32 42, i32 99)
  br label %3

; <label>:3:                                      ; preds = %11, %0
  %.02 = phi i32 [ 0, %0 ], [ %12, %11 ]
  %.01 = phi %struct._N* [ %2, %0 ], [ %14, %11 ]
  %4 = icmp eq %struct._N* %.01, null
  br i1 %4, label %15, label %5

; <label>:5:                                      ; preds = %3
  %6 = getelementptr inbounds %struct._N, %struct._N* %.01, i64 0, i32 0
  %7 = load i32, i32* %6, align 8
  %8 = icmp slt i32 %7, 40
  br i1 %8, label %9, label %11

; <label>:9:                                      ; preds = %5
  %10 = tail call i32 @_Z16heavyComputationi(i32 %7)
  br label %11

; <label>:11:                                     ; preds = %9, %5
  %.0 = phi i32 [ %10, %9 ], [ %7, %5 ]
  %12 = add nsw i32 %.02, %.0
  %13 = getelementptr inbounds %struct._N, %struct._N* %.01, i64 0, i32 1
  %14 = load %struct._N*, %struct._N** %13, align 8
  br label %3

; <label>:15:                                     ; preds = %3
  %.02.lcssa = phi i32 [ %.02, %3 ]
  %16 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32 %.02.lcssa)
  ret i32 0
}

declare i32 @printf(i8*, ...) #4

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { noinline uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noinline norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 5.0.0 (tags/RELEASE_500/final)"}
