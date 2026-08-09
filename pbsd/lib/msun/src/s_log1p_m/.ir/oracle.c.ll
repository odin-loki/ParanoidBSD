; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/s_log1p_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/s_log1p_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@vzero = internal global double 0.000000e+00, align 8

; Function Attrs: mustprogress nofree norecurse nounwind willreturn memory(readwrite, argmem: none) uwtable
define dso_local double @ref_log1p(double noundef %x) local_unnamed_addr #0 {
entry:
  %0 = bitcast double %x to i64
  %gh_u.sroa.0.4.extract.shift = lshr i64 %0, 32
  %gh_u.sroa.0.4.extract.trunc = trunc i64 %gh_u.sroa.0.4.extract.shift to i32
  %and = and i32 %gh_u.sroa.0.4.extract.trunc, 2147483647
  %cmp = icmp slt i32 %gh_u.sroa.0.4.extract.trunc, 1071284858
  br i1 %cmp, label %if.then, label %if.end19

if.then:                                          ; preds = %entry
  %cmp1 = icmp ugt i32 %and, 1072693247
  br i1 %cmp1, label %if.then2, label %if.end

if.then2:                                         ; preds = %if.then
  %cmp3 = fcmp oeq double %x, -1.000000e+00
  br i1 %cmp3, label %if.then4, label %if.else

if.then4:                                         ; preds = %if.then2
  %1 = load volatile double, ptr @vzero, align 8, !tbaa !5
  %div = fdiv double 0xC350000000000000, %1
  br label %cleanup

if.else:                                          ; preds = %if.then2
  %sub = fsub double %x, %x
  %div6 = fdiv double %sub, %sub
  br label %cleanup

if.end:                                           ; preds = %if.then
  %cmp7 = icmp ult i32 %and, 1042284544
  br i1 %cmp7, label %if.then8, label %if.end23

if.then8:                                         ; preds = %if.end
  %add = fadd double %x, 0x4350000000000000
  %cmp9 = fcmp ogt double %add, 0.000000e+00
  %cmp10 = icmp ult i32 %and, 1016070144
  %or.cond = and i1 %cmp9, %cmp10
  br i1 %or.cond, label %cleanup, label %if.else12

if.else12:                                        ; preds = %if.then8
  %2 = fneg double %x
  %neg = fmul double %2, %x
  %3 = tail call double @llvm.fmuladd.f64(double %neg, double 5.000000e-01, double %x)
  br label %cleanup

if.end19:                                         ; preds = %entry
  %cmp20 = icmp ugt i32 %gh_u.sroa.0.4.extract.trunc, 2146435071
  br i1 %cmp20, label %if.then21, label %if.then25

if.then21:                                        ; preds = %if.end19
  %add22 = fadd double %x, %x
  br label %cleanup

if.end23:                                         ; preds = %if.end
  %4 = add i32 %gh_u.sroa.0.4.extract.trunc, -1
  %or.cond135 = icmp ult i32 %4, -1076707644
  br i1 %or.cond135, label %if.end69.thread, label %if.then27

if.end69.thread:                                  ; preds = %if.end23
  %mul70223 = fmul double %x, 5.000000e-01
  %mul71224 = fmul double %mul70223, %x
  br label %if.end97

if.then25:                                        ; preds = %if.end19
  %cmp26 = icmp ult i32 %gh_u.sroa.0.4.extract.trunc, 1128267776
  br i1 %cmp26, label %if.then27, label %if.else41

if.then27:                                        ; preds = %if.end23, %if.then25
  %add28 = fadd double %x, 1.000000e+00
  %5 = bitcast double %add28 to i64
  %gh_u30.sroa.0.4.extract.shift = lshr i64 %5, 32
  %gh_u30.sroa.0.4.extract.trunc = trunc i64 %gh_u30.sroa.0.4.extract.shift to i32
  %shr = ashr i32 %gh_u30.sroa.0.4.extract.trunc, 20
  %cmp35 = icmp sgt i32 %shr, 1023
  %6 = fsub double %x, %add28
  %sub37 = fadd double %6, 1.000000e+00
  %sub38 = fadd double %add28, -1.000000e+00
  %sub39 = fsub double %x, %sub38
  %cond = select i1 %cmp35, double %sub37, double %sub39
  %div40 = fdiv double %cond, %add28
  br label %if.end49

if.else41:                                        ; preds = %if.then25
  %shr47 = lshr i32 %gh_u.sroa.0.4.extract.trunc, 20
  br label %if.end49

if.end49:                                         ; preds = %if.else41, %if.then27
  %hu.1 = phi i32 [ %gh_u30.sroa.0.4.extract.trunc, %if.then27 ], [ %gh_u.sroa.0.4.extract.trunc, %if.else41 ]
  %k.1.in = phi i32 [ %shr, %if.then27 ], [ %shr47, %if.else41 ]
  %u.0 = phi double [ %add28, %if.then27 ], [ %x, %if.else41 ]
  %c.0 = phi double [ %div40, %if.then27 ], [ 0.000000e+00, %if.else41 ]
  %and50 = and i32 %hu.1, 1048575
  %cmp51 = icmp ult i32 %and50, 434334
  %7 = bitcast double %u.0 to i64
  %sh_u.sroa.0.4.insert.mask = and i64 %7, 4294967295
  br i1 %cmp51, label %do.body53, label %if.else57

do.body53:                                        ; preds = %if.end49
  %k.1 = add nsw i32 %k.1.in, -1023
  %or = or disjoint i32 %and50, 1072693248
  br label %if.end69

if.else57:                                        ; preds = %if.end49
  %add58 = add nsw i32 %k.1.in, -1022
  %or61 = or disjoint i32 %and50, 1071644672
  %sub65 = sub nuw nsw i32 1048576, %and50
  %shr66 = lshr i32 %sub65, 2
  br label %if.end69

if.end69:                                         ; preds = %do.body53, %if.else57
  %hu.2 = phi i32 [ %and50, %do.body53 ], [ %shr66, %if.else57 ]
  %k.2 = phi i32 [ %k.1, %do.body53 ], [ %add58, %if.else57 ]
  %sh_u.sroa.0.4.insert.shift.pn.in.in = phi i32 [ %or, %do.body53 ], [ %or61, %if.else57 ]
  %sh_u.sroa.0.4.insert.shift.pn.in = zext i32 %sh_u.sroa.0.4.insert.shift.pn.in.in to i64
  %sh_u.sroa.0.4.insert.shift.pn = shl nuw nsw i64 %sh_u.sroa.0.4.insert.shift.pn.in, 32
  %u.1.in = or disjoint i64 %sh_u.sroa.0.4.insert.mask, %sh_u.sroa.0.4.insert.shift.pn
  %u.1 = bitcast i64 %u.1.in to double
  %sub68 = fadd double %u.1, -1.000000e+00
  %8 = icmp eq i32 %hu.2, 0
  %mul70 = fmul double %sub68, 5.000000e-01
  %mul71 = fmul double %sub68, %mul70
  br i1 %8, label %if.then73, label %if.end97

if.then73:                                        ; preds = %if.end69
  %cmp74 = fcmp oeq double %sub68, 0.000000e+00
  br i1 %cmp74, label %if.then75, label %if.end82

if.then75:                                        ; preds = %if.then73
  %cmp76 = icmp eq i32 %k.2, 0
  br i1 %cmp76, label %cleanup, label %if.else78

if.else78:                                        ; preds = %if.then75
  %conv = sitofp i32 %k.2 to double
  %9 = tail call double @llvm.fmuladd.f64(double %conv, double 0x3DEA39EF35793C76, double %c.0)
  %10 = tail call double @llvm.fmuladd.f64(double %conv, double 0x3FE62E42FEE00000, double %9)
  br label %cleanup

if.end82:                                         ; preds = %if.then73
  %11 = tail call double @llvm.fmuladd.f64(double %sub68, double 0xBFE5555555555555, double 1.000000e+00)
  %mul84 = fmul double %11, %mul71
  %cmp85 = icmp eq i32 %k.2, 0
  br i1 %cmp85, label %if.then87, label %if.else89

if.then87:                                        ; preds = %if.end82
  %sub88 = fsub double %sub68, %mul84
  br label %cleanup

if.else89:                                        ; preds = %if.end82
  %conv90 = sitofp i32 %k.2 to double
  %12 = tail call double @llvm.fmuladd.f64(double %conv90, double 0x3DEA39EF35793C76, double %c.0)
  %sub94 = fsub double %mul84, %12
  %sub95 = fsub double %sub94, %sub68
  %neg96 = fneg double %sub95
  %13 = tail call double @llvm.fmuladd.f64(double %conv90, double 0x3FE62E42FEE00000, double %neg96)
  br label %cleanup

if.end97:                                         ; preds = %if.end69.thread, %if.end69
  %mul71228 = phi double [ %mul71224, %if.end69.thread ], [ %mul71, %if.end69 ]
  %f.1227 = phi double [ %x, %if.end69.thread ], [ %sub68, %if.end69 ]
  %c.1226 = phi double [ undef, %if.end69.thread ], [ %c.0, %if.end69 ]
  %k.3225 = phi i32 [ 0, %if.end69.thread ], [ %k.2, %if.end69 ]
  %add98 = fadd double %f.1227, 2.000000e+00
  %div99 = fdiv double %f.1227, %add98
  %mul100 = fmul double %div99, %div99
  %14 = tail call double @llvm.fmuladd.f64(double %mul100, double 0x3FC2F112DF3E5244, double 0x3FC39A09D078C69F)
  %15 = tail call double @llvm.fmuladd.f64(double %mul100, double %14, double 0x3FC7466496CB03DE)
  %16 = tail call double @llvm.fmuladd.f64(double %mul100, double %15, double 0x3FCC71C51D8E78AF)
  %17 = tail call double @llvm.fmuladd.f64(double %mul100, double %16, double 0x3FD2492494229359)
  %18 = tail call double @llvm.fmuladd.f64(double %mul100, double %17, double 0x3FD999999997FA04)
  %19 = tail call double @llvm.fmuladd.f64(double %mul100, double %18, double 0x3FE5555555555593)
  %mul107 = fmul double %mul100, %19
  %cmp108 = icmp eq i32 %k.3225, 0
  br i1 %cmp108, label %if.then110, label %if.else115

if.then110:                                       ; preds = %if.end97
  %add111 = fadd double %mul71228, %mul107
  %neg113 = fneg double %div99
  %20 = tail call double @llvm.fmuladd.f64(double %neg113, double %add111, double %mul71228)
  %sub114 = fsub double %f.1227, %20
  br label %cleanup

if.else115:                                       ; preds = %if.end97
  %conv116 = sitofp i32 %k.3225 to double
  %add118 = fadd double %mul71228, %mul107
  %21 = tail call double @llvm.fmuladd.f64(double %conv116, double 0x3DEA39EF35793C76, double %c.1226)
  %22 = tail call double @llvm.fmuladd.f64(double %div99, double %add118, double %21)
  %sub122 = fsub double %mul71228, %22
  %sub123 = fsub double %sub122, %f.1227
  %neg124 = fneg double %sub123
  %23 = tail call double @llvm.fmuladd.f64(double %conv116, double 0x3FE62E42FEE00000, double %neg124)
  br label %cleanup

cleanup:                                          ; preds = %if.then75, %if.then8, %if.else115, %if.then110, %if.else89, %if.then87, %if.else78, %if.then21, %if.else12, %if.else, %if.then4
  %retval.0 = phi double [ %div, %if.then4 ], [ %div6, %if.else ], [ %3, %if.else12 ], [ %add22, %if.then21 ], [ %10, %if.else78 ], [ %sub88, %if.then87 ], [ %13, %if.else89 ], [ %sub114, %if.then110 ], [ %23, %if.else115 ], [ %x, %if.then8 ], [ 0.000000e+00, %if.then75 ]
  ret double %retval.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fmuladd.f64(double, double, double) #1

attributes #0 = { mustprogress nofree norecurse nounwind willreturn memory(readwrite, argmem: none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }

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
