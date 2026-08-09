; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/k_sin_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/k_sin_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local double @ref___kernel_sin(double noundef %x, double noundef %y, i32 noundef %iy) local_unnamed_addr #0 {
entry:
  %mul = fmul double %x, %x
  %mul1 = fmul double %mul, %mul
  %0 = tail call double @llvm.fmuladd.f64(double %mul, double 0x3EC71DE357B1FE7D, double 0xBF2A01A019C161D5)
  %1 = tail call double @llvm.fmuladd.f64(double %mul, double %0, double 0x3F8111111110F8A6)
  %mul4 = fmul double %mul, %mul1
  %2 = tail call double @llvm.fmuladd.f64(double %mul, double 0x3DE5D93A5ACFD57C, double 0xBE5AE5E68A2B9CEB)
  %3 = tail call double @llvm.fmuladd.f64(double %mul4, double %2, double %1)
  %mul7 = fmul double %mul, %x
  %cmp = icmp eq i32 %iy, 0
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %4 = tail call double @llvm.fmuladd.f64(double %mul, double %3, double 0xBFC5555555555549)
  %5 = tail call double @llvm.fmuladd.f64(double %mul7, double %4, double %x)
  br label %cleanup

if.else:                                          ; preds = %entry
  %6 = fneg double %mul7
  %neg = fmul double %3, %6
  %7 = tail call double @llvm.fmuladd.f64(double %y, double 5.000000e-01, double %neg)
  %neg13 = fneg double %y
  %8 = tail call double @llvm.fmuladd.f64(double %mul, double %7, double %neg13)
  %9 = tail call double @llvm.fmuladd.f64(double %6, double 0xBFC5555555555549, double %8)
  %sub = fsub double %x, %9
  br label %cleanup

cleanup:                                          ; preds = %if.else, %if.then
  %retval.0 = phi double [ %5, %if.then ], [ %sub, %if.else ]
  ret double %retval.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fmuladd.f64(double, double, double) #1

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
