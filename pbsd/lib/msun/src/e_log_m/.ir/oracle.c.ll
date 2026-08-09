; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/e_log_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/e_log_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@vzero = internal global double 0.000000e+00, align 8

; Function Attrs: mustprogress nofree norecurse nounwind willreturn memory(readwrite, argmem: none) uwtable
define dso_local double @ref_log(double noundef %x) local_unnamed_addr #0 {
entry:
  %0 = bitcast double %x to i64
  %ew_u.sroa.0.4.extract.shift = lshr i64 %0, 32
  %ew_u.sroa.0.4.extract.trunc = trunc i64 %ew_u.sroa.0.4.extract.shift to i32
  %cmp = icmp slt i32 %ew_u.sroa.0.4.extract.trunc, 1048576
  br i1 %cmp, label %if.then, label %if.end12

if.then:                                          ; preds = %entry
  %ew_u.sroa.0.0.extract.trunc = trunc i64 %0 to i32
  %and = and i32 %ew_u.sroa.0.4.extract.trunc, 2147483647
  %or = or i32 %and, %ew_u.sroa.0.0.extract.trunc
  %cmp1 = icmp eq i32 %or, 0
  br i1 %cmp1, label %if.then2, label %if.end

if.then2:                                         ; preds = %if.then
  %1 = load volatile double, ptr @vzero, align 8, !tbaa !5
  %div = fdiv double 0xC350000000000000, %1
  br label %cleanup

if.end:                                           ; preds = %if.then
  %cmp3 = icmp slt i64 %0, 0
  br i1 %cmp3, label %if.then4, label %if.end6

if.then4:                                         ; preds = %if.end
  %sub = fsub double %x, %x
  %div5 = fdiv double %sub, 0.000000e+00
  br label %cleanup

if.end6:                                          ; preds = %if.end
  %mul = fmul double %x, 0x4350000000000000
  %2 = bitcast double %mul to i64
  %gh_u.sroa.0.4.extract.shift = lshr i64 %2, 32
  %gh_u.sroa.0.4.extract.trunc = trunc i64 %gh_u.sroa.0.4.extract.shift to i32
  br label %if.end12

if.end12:                                         ; preds = %if.end6, %entry
  %hx.0 = phi i32 [ %gh_u.sroa.0.4.extract.trunc, %if.end6 ], [ %ew_u.sroa.0.4.extract.trunc, %entry ]
  %k.0 = phi i32 [ -54, %if.end6 ], [ 0, %entry ]
  %x.addr.0 = phi double [ %mul, %if.end6 ], [ %x, %entry ]
  %cmp13 = icmp sgt i32 %hx.0, 2146435071
  br i1 %cmp13, label %if.then14, label %if.end15

if.then14:                                        ; preds = %if.end12
  %add = fadd double %x.addr.0, %x.addr.0
  br label %cleanup

if.end15:                                         ; preds = %if.end12
  %shr = ashr i32 %hx.0, 20
  %sub16 = add nsw i32 %shr, -1023
  %add17 = add nsw i32 %sub16, %k.0
  %and18 = and i32 %hx.0, 1048575
  %add19 = add nuw nsw i32 %and18, 614244
  %and20 = and i32 %add19, 1048576
  %3 = or disjoint i32 %and20, %and18
  %or22 = xor i32 %3, 1072693248
  %4 = bitcast double %x.addr.0 to i64
  %sh_u.sroa.0.4.insert.ext = zext nneg i32 %or22 to i64
  %sh_u.sroa.0.4.insert.shift = shl nuw nsw i64 %sh_u.sroa.0.4.insert.ext, 32
  %sh_u.sroa.0.4.insert.mask = and i64 %4, 4294967295
  %sh_u.sroa.0.4.insert.insert = or disjoint i64 %sh_u.sroa.0.4.insert.shift, %sh_u.sroa.0.4.insert.mask
  %5 = bitcast i64 %sh_u.sroa.0.4.insert.insert to double
  %shr26 = lshr i32 %add19, 20
  %add27 = add nsw i32 %add17, %shr26
  %sub28 = fadd double %5, -1.000000e+00
  %add29 = add nsw i32 %hx.0, 2
  %and30 = and i32 %add29, 1048575
  %cmp31 = icmp ult i32 %and30, 3
  br i1 %cmp31, label %if.then32, label %if.end53

if.then32:                                        ; preds = %if.end15
  %cmp33 = fcmp oeq double %sub28, 0.000000e+00
  br i1 %cmp33, label %if.then34, label %if.end39

if.then34:                                        ; preds = %if.then32
  %cmp35 = icmp eq i32 %add27, 0
  br i1 %cmp35, label %cleanup, label %if.else

if.else:                                          ; preds = %if.then34
  %conv = sitofp i32 %add27 to double
  %mul38 = fmul double %conv, 0x3DEA39EF35793C76
  %6 = tail call double @llvm.fmuladd.f64(double %conv, double 0x3FE62E42FEE00000, double %mul38)
  br label %cleanup

if.end39:                                         ; preds = %if.then32
  %mul40 = fmul double %sub28, %sub28
  %7 = tail call double @llvm.fmuladd.f64(double %sub28, double 0xBFD5555555555555, double 5.000000e-01)
  %mul42 = fmul double %mul40, %7
  %cmp43 = icmp eq i32 %add27, 0
  br i1 %cmp43, label %if.then45, label %if.else47

if.then45:                                        ; preds = %if.end39
  %sub46 = fsub double %sub28, %mul42
  br label %cleanup

if.else47:                                        ; preds = %if.end39
  %conv48 = sitofp i32 %add27 to double
  %neg = fneg double %conv48
  %8 = tail call double @llvm.fmuladd.f64(double %neg, double 0x3DEA39EF35793C76, double %mul42)
  %sub51 = fsub double %8, %sub28
  %neg52 = fneg double %sub51
  %9 = tail call double @llvm.fmuladd.f64(double %conv48, double 0x3FE62E42FEE00000, double %neg52)
  br label %cleanup

if.end53:                                         ; preds = %if.end15
  %add54 = fadd double %sub28, 2.000000e+00
  %div55 = fdiv double %sub28, %add54
  %conv56 = sitofp i32 %add27 to double
  %mul57 = fmul double %div55, %div55
  %sub58 = add nsw i32 %and18, -398458
  %mul59 = fmul double %mul57, %mul57
  %sub60 = sub nsw i32 440401, %and18
  %10 = tail call double @llvm.fmuladd.f64(double %mul59, double 0x3FC39A09D078C69F, double 0x3FCC71C51D8E78AF)
  %11 = tail call double @llvm.fmuladd.f64(double %mul59, double %10, double 0x3FD999999997FA04)
  %mul63 = fmul double %mul59, %11
  %12 = tail call double @llvm.fmuladd.f64(double %mul59, double 0x3FC2F112DF3E5244, double 0x3FC7466496CB03DE)
  %13 = tail call double @llvm.fmuladd.f64(double %mul59, double %12, double 0x3FD2492494229359)
  %14 = tail call double @llvm.fmuladd.f64(double %mul59, double %13, double 0x3FE5555555555593)
  %mul67 = fmul double %mul57, %14
  %or68 = or i32 %sub58, %sub60
  %add69 = fadd double %mul63, %mul67
  %cmp70 = icmp sgt i32 %or68, 0
  br i1 %cmp70, label %if.then72, label %if.else90

if.then72:                                        ; preds = %if.end53
  %mul73 = fmul double %sub28, 5.000000e-01
  %mul74 = fmul double %sub28, %mul73
  %cmp75 = icmp eq i32 %add27, 0
  %add78 = fadd double %mul74, %add69
  br i1 %cmp75, label %if.then77, label %if.else82

if.then77:                                        ; preds = %if.then72
  %neg80 = fneg double %div55
  %15 = tail call double @llvm.fmuladd.f64(double %neg80, double %add78, double %mul74)
  %sub81 = fsub double %sub28, %15
  br label %cleanup

if.else82:                                        ; preds = %if.then72
  %mul86 = fmul double %conv56, 0x3DEA39EF35793C76
  %16 = tail call double @llvm.fmuladd.f64(double %div55, double %add78, double %mul86)
  %sub87 = fsub double %mul74, %16
  %sub88 = fsub double %sub87, %sub28
  %neg89 = fneg double %sub88
  %17 = tail call double @llvm.fmuladd.f64(double %conv56, double 0x3FE62E42FEE00000, double %neg89)
  br label %cleanup

if.else90:                                        ; preds = %if.end53
  %cmp91 = icmp eq i32 %add27, 0
  %sub94 = fsub double %sub28, %add69
  br i1 %cmp91, label %if.then93, label %if.else97

if.then93:                                        ; preds = %if.else90
  %neg96 = fneg double %div55
  %18 = tail call double @llvm.fmuladd.f64(double %neg96, double %sub94, double %sub28)
  br label %cleanup

if.else97:                                        ; preds = %if.else90
  %neg102 = fmul double %conv56, 0xBDEA39EF35793C76
  %19 = tail call double @llvm.fmuladd.f64(double %div55, double %sub94, double %neg102)
  %sub103 = fsub double %19, %sub28
  %neg104 = fneg double %sub103
  %20 = tail call double @llvm.fmuladd.f64(double %conv56, double 0x3FE62E42FEE00000, double %neg104)
  br label %cleanup

cleanup:                                          ; preds = %if.then34, %if.else97, %if.then93, %if.else82, %if.then77, %if.else47, %if.then45, %if.else, %if.then14, %if.then4, %if.then2
  %retval.0 = phi double [ %div, %if.then2 ], [ %div5, %if.then4 ], [ %add, %if.then14 ], [ %6, %if.else ], [ %sub46, %if.then45 ], [ %9, %if.else47 ], [ %sub81, %if.then77 ], [ %17, %if.else82 ], [ %18, %if.then93 ], [ %20, %if.else97 ], [ 0.000000e+00, %if.then34 ]
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
