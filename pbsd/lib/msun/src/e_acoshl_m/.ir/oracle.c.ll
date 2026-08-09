; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/e_acoshl_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/e_acoshl_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: mustprogress nofree nounwind willreturn memory(write) uwtable
define dso_local x86_fp80 @ref_acoshl(x86_fp80 noundef %x) local_unnamed_addr #0 {
entry:
  %0 = bitcast x86_fp80 %x to i80
  %sh.diff = lshr i80 %0, 48
  %tr.sh.diff = trunc i80 %sh.diff to i32
  %conv1 = ashr i32 %tr.sh.diff, 16
  %cmp = icmp slt i32 %conv1, 16383
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %sub = fsub x86_fp80 %x, %x
  %div = fdiv x86_fp80 %sub, %sub
  br label %cleanup

if.else:                                          ; preds = %entry
  %cmp5 = icmp ugt i32 %conv1, 16416
  br i1 %cmp5, label %if.then7, label %if.else14

if.then7:                                         ; preds = %if.else
  %cmp9 = icmp ugt i32 %conv1, 32766
  br i1 %cmp9, label %if.then11, label %if.else12

if.then11:                                        ; preds = %if.then7
  %add = fadd x86_fp80 %x, %x
  br label %cleanup

if.else12:                                        ; preds = %if.then7
  %call = tail call x86_fp80 @logl(x86_fp80 noundef %x) #3
  %add13 = fadd x86_fp80 %call, 0xK3FFEB17217F7D1CF79AC
  br label %cleanup

if.else14:                                        ; preds = %if.else
  %cmp16 = icmp eq i32 %conv1, 16383
  %cmp18 = fcmp oeq x86_fp80 %x, 0xK3FFF8000000000000000
  %or.cond = and i1 %cmp18, %cmp16
  br i1 %or.cond, label %cleanup, label %if.else21

if.else21:                                        ; preds = %if.else14
  br i1 %cmp16, label %if.else32, label %if.then25

if.then25:                                        ; preds = %if.else21
  %mul = fmul x86_fp80 %x, %x
  %sub27 = fadd x86_fp80 %mul, 0xKBFFF8000000000000000
  %call28 = tail call x86_fp80 @sqrtl(x86_fp80 noundef %sub27) #3
  %add29 = fadd x86_fp80 %call28, %x
  %neg = fdiv x86_fp80 0xKBFFF8000000000000000, %add29
  %1 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %x, x86_fp80 0xK40008000000000000000, x86_fp80 %neg)
  %call31 = tail call x86_fp80 @logl(x86_fp80 noundef %1) #3
  br label %cleanup

if.else32:                                        ; preds = %if.else21
  %sub33 = fadd x86_fp80 %x, 0xKBFFF8000000000000000
  %mul35 = fmul x86_fp80 %sub33, %sub33
  %2 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub33, x86_fp80 0xK40008000000000000000, x86_fp80 %mul35)
  %call36 = tail call x86_fp80 @sqrtl(x86_fp80 noundef %2) #3
  %add37 = fadd x86_fp80 %sub33, %call36
  %call38 = tail call x86_fp80 @log1pl(x86_fp80 noundef %add37) #3
  br label %cleanup

cleanup:                                          ; preds = %if.else14, %if.else32, %if.then25, %if.else12, %if.then11, %if.then
  %retval.0 = phi x86_fp80 [ %div, %if.then ], [ %add, %if.then11 ], [ %add13, %if.else12 ], [ %call31, %if.then25 ], [ %call38, %if.else32 ], [ 0xK00000000000000000000, %if.else14 ]
  ret x86_fp80 %retval.0
}

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare x86_fp80 @logl(x86_fp80 noundef) local_unnamed_addr #1

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare x86_fp80 @sqrtl(x86_fp80 noundef) local_unnamed_addr #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare x86_fp80 @llvm.fmuladd.f80(x86_fp80, x86_fp80, x86_fp80) #2

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare x86_fp80 @log1pl(x86_fp80 noundef) local_unnamed_addr #1

attributes #0 = { mustprogress nofree nounwind willreturn memory(write) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nofree nounwind willreturn memory(write) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
