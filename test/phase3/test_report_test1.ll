; ModuleID = 'test_report_test1.ll'
source_filename = "test_report_test1.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local void @test() #0 {
entry:
  br label %while.cond

while.cond:                                       ; preds = %if.end, %entry
  %c.0 = phi i32 [ 5, %entry ], [ %sub, %if.end ]
  %b.0 = phi i32 [ 4, %entry ], [ %div, %if.end ]
  %a.0 = phi i32 [ -5, %entry ], [ %add, %if.end ]
  %cmp = icmp slt i32 %a.0, 3
  br i1 %cmp, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %sub = sub nsw i32 %c.0, 1
  %add = add nsw i32 %a.0, 2
  %cmp1 = icmp sgt i32 %b.0, 4
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %while.body
  %sub2 = sub nsw i32 %b.0, 4
  br label %if.end

if.else:                                          ; preds = %while.body
  %add3 = add nsw i32 %b.0, 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %b.1 = phi i32 [ %sub2, %if.then ], [ %add3, %if.else ]
  %div = sdiv i32 %b.1, 2
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
