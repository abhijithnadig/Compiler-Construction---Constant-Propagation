; ModuleID = 'test5.c'
source_filename = "test5.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local void @test() #0 {
entry:
  %a = alloca i32, align 4
  %b = alloca i32, align 4
  %c = alloca i32, align 4
  %d = alloca i32, align 4
  store i32 5, i32* %a, align 4
  store i32 4, i32* %b, align 4
  store i32 5, i32* %c, align 4
  store i32 4, i32* %d, align 4
  br label %while.cond

while.cond:                                       ; preds = %if.end, %entry
  %0 = load i32, i32* %a, align 4
  %cmp = icmp slt i32 %0, 3
  br i1 %cmp, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %1 = load i32, i32* %c, align 4
  %sub = sub nsw i32 %1, 1
  store i32 %sub, i32* %c, align 4
  %2 = load i32, i32* %a, align 4
  %add = add nsw i32 %2, 2
  store i32 %add, i32* %a, align 4
  %3 = load i32, i32* %b, align 4
  %4 = load i32, i32* %d, align 4
  %cmp1 = icmp sgt i32 %3, %4
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %while.body
  %5 = load i32, i32* %b, align 4
  %6 = load i32, i32* %d, align 4
  %sub2 = sub nsw i32 %5, %6
  store i32 %sub2, i32* %b, align 4
  br label %if.end

if.else:                                          ; preds = %while.body
  %7 = load i32, i32* %b, align 4
  %8 = load i32, i32* %d, align 4
  %add3 = add nsw i32 %7, %8
  store i32 %add3, i32* %b, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %9 = load i32, i32* %b, align 4
  %div = sdiv i32 %9, 2
  store i32 %div, i32* %b, align 4
  br label %while.cond, !llvm.loop !2

while.end:                                        ; preds = %while.cond
  ret void
}

attributes #0 = { noinline nounwind uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 12.0.1"}
!2 = distinct !{!2, !3}
!3 = !{!"llvm.loop.mustprogress"}
