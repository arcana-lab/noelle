; ModuleID = 'omp-critical.c'
source_filename = "omp-critical.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [12 x i8] c"%d, %d, %d\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @block_counter() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  store i32 0, i32* %2, align 4
  br label %6

6:                                                ; preds = %26, %0
  %7 = load i32, i32* %2, align 4, !note.noelle !2
  %8 = icmp slt i32 %7, 16, !note.noelle !2
  br i1 %8, label %9, label %29, !note.noelle !2

9:                                                ; preds = %6
  %10 = load i32, i32* %1, align 4, !note.noelle !4
  store i32 %10, i32* %3, align 4, !note.noelle !4
  %11 = load i32, i32* %1, align 4, !note.noelle !4
  %12 = add nsw i32 %11, 1, !note.noelle !4
  store i32 %12, i32* %1, align 4, !note.noelle !4
  %13 = load i32, i32* %1, align 4, !note.noelle !4
  %14 = add nsw i32 %13, 1, !note.noelle !4
  store i32 %14, i32* %1, align 4, !note.noelle !4
  %15 = load i32, i32* %1, align 4, !note.noelle !4
  %16 = add nsw i32 %15, 1, !note.noelle !4
  store i32 %16, i32* %1, align 4, !note.noelle !4
  %17 = load i32, i32* %3, align 4, !note.noelle !2
  %18 = add nsw i32 %17, 1, !note.noelle !2
  store i32 %18, i32* %4, align 4, !note.noelle !2
  %19 = load i32, i32* %4, align 4, !note.noelle !2
  %20 = load i32, i32* %3, align 4, !note.noelle !2
  %21 = mul nsw i32 %19, %20, !note.noelle !2
  store i32 %21, i32* %5, align 4, !note.noelle !2
  %22 = load i32, i32* %3, align 4, !note.noelle !2
  %23 = load i32, i32* %4, align 4, !note.noelle !2
  %24 = load i32, i32* %5, align 4, !note.noelle !2
  %25 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str, i64 0, i64 0), i32 %22, i32 %23, i32 %24), !note.noelle !2
  br label %26, !note.noelle !2

26:                                               ; preds = %9
  %27 = load i32, i32* %2, align 4, !note.noelle !2
  %28 = add nsw i32 %27, 1, !note.noelle !2
  store i32 %28, i32* %2, align 4, !note.noelle !2
  br label %6, !note.noelle !2

29:                                               ; preds = %6
  ret void
}

declare dso_local i32 @printf(i8*, ...) #1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  call void @block_counter()
  ret i32 0
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0 (https://github.com/scampanoni/LLVM_installer.git 0d876be2f90ee7ddfb16c2b131ab2c0e1f94708e)"}
!2 = !{!3}
!3 = !{!"independent", !"1"}
!4 = !{!5, !3}
!5 = !{!"critical", !"1"}
