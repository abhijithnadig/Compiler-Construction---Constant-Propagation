; ModuleID = 'test2.ll'
source_filename = "test2.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local void @test() #0 {
entry:
  %cmp = icmp ne i32 1, 2
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %add = add nsw i32 2, 1
  br label %if.end

if.else:                                          ; preds = %entry
  %sub = sub nsw i32 2, 1
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %z.0 = phi i32 [ %add, %if.then ], [ %sub, %if.else ]
  br label %do.body

do.body:                                          ; preds = %do.cond, %if.end
  %b.0 = phi i32 [ 2, %if.end ], [ %sub7, %do.cond ]
  %cmp1 = icmp slt i32 %z.0, 0
  br i1 %cmp1, label %if.then2, label %if.else4

if.then2:                                         ; preds = %do.body
  %add3 = add nsw i32 %z.0, 1
  br label %if.end6

if.else4:                                         ; preds = %do.body
  %sub5 = sub nsw i32 %z.0, 1
  br label %if.end6

if.end6:                                          ; preds = %if.else4, %if.then2
  %sub7 = sub nsw i32 %b.0, 1
  br label %do.cond

do.cond:                                          ; preds = %if.end6
  %cmp8 = icmp sge i32 %sub7, 0
  br i1 %cmp8, label %do.body, label %do.end, !llvm.loop !2

do.end:                                           ; preds = %do.cond
  ret void
}

attributes #0 = { noinline nounwind uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 12.0.1"}
!2 = distinct !{!2, !3}
!3 = !{!"llvm.loop.mustprogress"}
