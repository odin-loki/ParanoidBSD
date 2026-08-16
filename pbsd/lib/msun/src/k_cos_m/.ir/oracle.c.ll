; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/k_cos_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/k_cos_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local double @ref___kernel_cos(double noundef %x, double noundef %y) local_unnamed_addr #0 {
entry:
  %mul = fmul double %x, %x
  %mul1 = fmul double %mul, %mul
  %0 = tail call double @llvm.fmuladd.f64(double %mul, double 0x3EFA01A019CB1590, double 0xBF56C16C16C15177)
  %1 = tail call double @llvm.fmuladd.f64(double %mul, double %0, double 0x3FA555555555554C)
  %mul5 = fmul double %mul1, %mul1
  %2 = tail call double @llvm.fmuladd.f64(double %mul, double 0xBDA8FAE9BE8838D4, double 0x3E21EE9EBDB4B1C4)
  %3 = tail call double @llvm.fmuladd.f64(double %mul, double %2, double 0xBE927E4F809C52AD)
  %mul8 = fmul double %mul5, %3
  %4 = tail call double @llvm.fmuladd.f64(double %mul, double %1, double %mul8)
  %mul9 = fmul double %mul, 5.000000e-01
  %sub = fsub double 1.000000e+00, %mul9
  %sub10 = fsub double 1.000000e+00, %sub
  %sub11 = fsub double %sub10, %mul9
  %5 = fneg double %x
  %neg = fmul double %5, %y
  %6 = tail call double @llvm.fmuladd.f64(double %mul, double %4, double %neg)
  %add = fadd double %sub11, %6
  %add14 = fadd double %sub, %add
  ret double %add14
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
