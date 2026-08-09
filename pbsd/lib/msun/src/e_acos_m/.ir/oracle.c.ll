; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/e_acos_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/e_acos_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@pio2_lo = internal global double 0x3C91A62633145C07, align 8

; Function Attrs: mustprogress nofree nounwind willreturn memory(readwrite, argmem: write) uwtable
define dso_local double @ref_acos(double noundef %x) local_unnamed_addr #0 {
entry:
  %0 = bitcast double %x to i64
  %gh_u.sroa.0.4.extract.shift = lshr i64 %0, 32
  %gh_u.sroa.0.4.extract.trunc = trunc i64 %gh_u.sroa.0.4.extract.shift to i32
  %and = and i32 %gh_u.sroa.0.4.extract.trunc, 2147483647
  %cmp = icmp ugt i32 %and, 1072693247
  br i1 %cmp, label %if.then, label %if.end10

if.then:                                          ; preds = %entry
  %gl_u.sroa.0.0.extract.trunc = trunc i64 %0 to i32
  %sub = add nsw i32 %and, -1072693248
  %or = or i32 %sub, %gl_u.sroa.0.0.extract.trunc
  %cmp4 = icmp eq i32 %or, 0
  br i1 %cmp4, label %if.then5, label %if.end

if.then5:                                         ; preds = %if.then
  %cmp6 = icmp sgt i32 %gh_u.sroa.0.4.extract.trunc, 0
  br i1 %cmp6, label %cleanup76, label %if.else

if.else:                                          ; preds = %if.then5
  %1 = load volatile double, ptr @pio2_lo, align 8, !tbaa !5
  %2 = tail call double @llvm.fmuladd.f64(double %1, double 2.000000e+00, double 0x400921FB54442D18)
  br label %cleanup76

if.end:                                           ; preds = %if.then
  %sub8 = fsub double %x, %x
  %div = fdiv double %sub8, %sub8
  br label %cleanup76

if.end10:                                         ; preds = %entry
  %cmp11 = icmp ult i32 %and, 1071644672
  br i1 %cmp11, label %if.then12, label %if.else30

if.then12:                                        ; preds = %if.end10
  %cmp13 = icmp ult i32 %and, 1012924417
  br i1 %cmp13, label %if.then14, label %if.end15

if.then14:                                        ; preds = %if.then12
  %3 = load volatile double, ptr @pio2_lo, align 8, !tbaa !5
  %add = fadd double %3, 0x3FF921FB54442D18
  br label %cleanup76

if.end15:                                         ; preds = %if.then12
  %mul = fmul double %x, %x
  %4 = tail call double @llvm.fmuladd.f64(double %mul, double 0x3F023DE10DFDF709, double 0x3F49EFE07501B288)
  %5 = tail call double @llvm.fmuladd.f64(double %mul, double %4, double 0xBFA48228B5688F3B)
  %6 = tail call double @llvm.fmuladd.f64(double %mul, double %5, double 0x3FC9C1550E884455)
  %7 = tail call double @llvm.fmuladd.f64(double %mul, double %6, double 0xBFD4D61203EB6F7D)
  %8 = tail call double @llvm.fmuladd.f64(double %mul, double %7, double 0x3FC5555555555555)
  %mul21 = fmul double %mul, %8
  %9 = tail call double @llvm.fmuladd.f64(double %mul, double 0x3FB3B8C5B12E9282, double 0xBFE6066C1B8D0159)
  %10 = tail call double @llvm.fmuladd.f64(double %mul, double %9, double 0x40002AE59C598AC8)
  %11 = tail call double @llvm.fmuladd.f64(double %mul, double %10, double 0xC0033A271C8A2D4B)
  %12 = tail call double @llvm.fmuladd.f64(double %mul, double %11, double 1.000000e+00)
  %div26 = fdiv double %mul21, %12
  %13 = load volatile double, ptr @pio2_lo, align 8, !tbaa !5
  %neg = fneg double %x
  %14 = tail call double @llvm.fmuladd.f64(double %neg, double %div26, double %13)
  %15 = fsub double %14, %x
  %sub29 = fadd double %15, 0x3FF921FB54442D18
  br label %cleanup76

if.else30:                                        ; preds = %if.end10
  %cmp31 = icmp slt i64 %0, 0
  br i1 %cmp31, label %if.then32, label %if.else50

if.then32:                                        ; preds = %if.else30
  %add33 = fadd double %x, 1.000000e+00
  %mul34 = fmul double %add33, 5.000000e-01
  %16 = tail call double @llvm.fmuladd.f64(double %mul34, double 0x3F023DE10DFDF709, double 0x3F49EFE07501B288)
  %17 = tail call double @llvm.fmuladd.f64(double %mul34, double %16, double 0xBFA48228B5688F3B)
  %18 = tail call double @llvm.fmuladd.f64(double %mul34, double %17, double 0x3FC9C1550E884455)
  %19 = tail call double @llvm.fmuladd.f64(double %mul34, double %18, double 0xBFD4D61203EB6F7D)
  %20 = tail call double @llvm.fmuladd.f64(double %mul34, double %19, double 0x3FC5555555555555)
  %mul40 = fmul double %mul34, %20
  %21 = tail call double @llvm.fmuladd.f64(double %mul34, double 0x3FB3B8C5B12E9282, double 0xBFE6066C1B8D0159)
  %22 = tail call double @llvm.fmuladd.f64(double %mul34, double %21, double 0x40002AE59C598AC8)
  %23 = tail call double @llvm.fmuladd.f64(double %mul34, double %22, double 0xC0033A271C8A2D4B)
  %24 = tail call double @llvm.fmuladd.f64(double %mul34, double %23, double 1.000000e+00)
  %call = tail call double @sqrt(double noundef %mul34) #3
  %div45 = fdiv double %mul40, %24
  %25 = load volatile double, ptr @pio2_lo, align 8, !tbaa !5
  %neg47 = fneg double %25
  %26 = tail call double @llvm.fmuladd.f64(double %div45, double %call, double %neg47)
  %add48 = fadd double %call, %26
  %27 = tail call double @llvm.fmuladd.f64(double %add48, double -2.000000e+00, double 0x400921FB54442D18)
  br label %cleanup76

if.else50:                                        ; preds = %if.else30
  %sub51 = fsub double 1.000000e+00, %x
  %mul52 = fmul double %sub51, 5.000000e-01
  %call53 = tail call double @sqrt(double noundef %mul52) #3
  %28 = bitcast double %call53 to i64
  %sl_u.sroa.0.0.insert.mask = and i64 %28, -4294967296
  %29 = bitcast i64 %sl_u.sroa.0.0.insert.mask to double
  %neg59 = fneg double %29
  %30 = tail call double @llvm.fmuladd.f64(double %neg59, double %29, double %mul52)
  %add60 = fadd double %call53, %29
  %div61 = fdiv double %30, %add60
  %31 = tail call double @llvm.fmuladd.f64(double %mul52, double 0x3F023DE10DFDF709, double 0x3F49EFE07501B288)
  %32 = tail call double @llvm.fmuladd.f64(double %mul52, double %31, double 0xBFA48228B5688F3B)
  %33 = tail call double @llvm.fmuladd.f64(double %mul52, double %32, double 0x3FC9C1550E884455)
  %34 = tail call double @llvm.fmuladd.f64(double %mul52, double %33, double 0xBFD4D61203EB6F7D)
  %35 = tail call double @llvm.fmuladd.f64(double %mul52, double %34, double 0x3FC5555555555555)
  %mul67 = fmul double %mul52, %35
  %36 = tail call double @llvm.fmuladd.f64(double %mul52, double 0x3FB3B8C5B12E9282, double 0xBFE6066C1B8D0159)
  %37 = tail call double @llvm.fmuladd.f64(double %mul52, double %36, double 0x40002AE59C598AC8)
  %38 = tail call double @llvm.fmuladd.f64(double %mul52, double %37, double 0xC0033A271C8A2D4B)
  %39 = tail call double @llvm.fmuladd.f64(double %mul52, double %38, double 1.000000e+00)
  %div72 = fdiv double %mul67, %39
  %40 = tail call double @llvm.fmuladd.f64(double %div72, double %call53, double %div61)
  %add74 = fadd double %40, %29
  %mul75 = fmul double %add74, 2.000000e+00
  br label %cleanup76

cleanup76:                                        ; preds = %if.else, %if.end, %if.then5, %if.else50, %if.then32, %if.end15, %if.then14
  %retval.1 = phi double [ %add, %if.then14 ], [ %sub29, %if.end15 ], [ %27, %if.then32 ], [ %mul75, %if.else50 ], [ %2, %if.else ], [ %div, %if.end ], [ 0.000000e+00, %if.then5 ]
  ret double %retval.1
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fmuladd.f64(double, double, double) #1

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare double @sqrt(double noundef) local_unnamed_addr #2

attributes #0 = { mustprogress nofree nounwind willreturn memory(readwrite, argmem: write) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { mustprogress nofree nounwind willreturn memory(write) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!5 = !{!6, !6, i64 0}
!6 = !{!"double", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
