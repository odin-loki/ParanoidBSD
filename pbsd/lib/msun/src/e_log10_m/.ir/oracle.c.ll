; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/e_log10_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/e_log10_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@vzero = internal global double 0.000000e+00, align 8

; Function Attrs: mustprogress nofree norecurse nounwind willreturn memory(readwrite, argmem: none) uwtable
define dso_local double @ref_log10(double noundef %x) local_unnamed_addr #0 {
entry:
  %0 = bitcast double %x to i64
  %ew_u.sroa.0.4.extract.shift = lshr i64 %0, 32
  %ew_u.sroa.0.4.extract.trunc = trunc i64 %ew_u.sroa.0.4.extract.shift to i32
  %ew_u.sroa.0.0.extract.trunc = trunc i64 %0 to i32
  %cmp = icmp slt i32 %ew_u.sroa.0.4.extract.trunc, 1048576
  br i1 %cmp, label %if.then, label %if.end12

if.then:                                          ; preds = %entry
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
  %cmp16 = icmp eq i32 %hx.0, 1072693248
  %cmp17 = icmp eq i32 %ew_u.sroa.0.0.extract.trunc, 0
  %or.cond = and i1 %cmp17, %cmp16
  br i1 %or.cond, label %cleanup, label %if.end19

if.end19:                                         ; preds = %if.end15
  %shr = ashr i32 %hx.0, 20
  %sub20 = add nsw i32 %shr, -1023
  %add21 = add nsw i32 %sub20, %k.0
  %and22 = and i32 %hx.0, 1048575
  %add23 = add nuw nsw i32 %and22, 614244
  %and24 = and i32 %add23, 1048576
  %3 = or disjoint i32 %and24, %and22
  %or26 = xor i32 %3, 1072693248
  %4 = bitcast double %x.addr.0 to i64
  %sh_u.sroa.0.4.insert.ext = zext nneg i32 %or26 to i64
  %sh_u.sroa.0.4.insert.shift = shl nuw nsw i64 %sh_u.sroa.0.4.insert.ext, 32
  %sh_u.sroa.0.4.insert.mask = and i64 %4, 4294967295
  %sh_u.sroa.0.4.insert.insert = or disjoint i64 %sh_u.sroa.0.4.insert.shift, %sh_u.sroa.0.4.insert.mask
  %5 = bitcast i64 %sh_u.sroa.0.4.insert.insert to double
  %shr30 = lshr i32 %add23, 20
  %add31 = add nsw i32 %add21, %shr30
  %conv = sitofp i32 %add31 to double
  %sub32 = fadd double %5, -1.000000e+00
  %mul33 = fmul double %sub32, 5.000000e-01
  %mul34 = fmul double %sub32, %mul33
  %add.i = fadd double %sub32, 2.000000e+00
  %div.i = fdiv double %sub32, %add.i
  %mul.i = fmul double %div.i, %div.i
  %mul1.i = fmul double %mul.i, %mul.i
  %6 = tail call double @llvm.fmuladd.f64(double %mul1.i, double 0x3FC2F112DF3E5244, double 0x3FC7466496CB03DE)
  %7 = insertelement <2 x double> poison, double %mul1.i, i64 0
  %8 = shufflevector <2 x double> %7, <2 x double> poison, <2 x i32> zeroinitializer
  %9 = insertelement <2 x double> <double 0x3FC39A09D078C69F, double poison>, double %6, i64 1
  %10 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %8, <2 x double> %9, <2 x double> <double 0x3FCC71C51D8E78AF, double 0x3FD2492494229359>)
  %11 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %8, <2 x double> %10, <2 x double> <double 0x3FD999999997FA04, double 0x3FE5555555555593>)
  %12 = insertelement <2 x double> %7, double %mul.i, i64 1
  %13 = fmul <2 x double> %12, %11
  %shift = shufflevector <2 x double> %13, <2 x double> poison, <2 x i32> <i32 1, i32 poison>
  %14 = fadd <2 x double> %13, %shift
  %add9.i = extractelement <2 x double> %14, i64 0
  %add12.i = fadd double %mul34, %add9.i
  %mul13.i = fmul double %div.i, %add12.i
  %sub35 = fsub double %sub32, %mul34
  %15 = bitcast double %sub35 to i64
  %sl_u.sroa.0.0.insert.mask = and i64 %15, -4294967296
  %16 = bitcast i64 %sl_u.sroa.0.0.insert.mask to double
  %sub40 = fsub double %sub32, %16
  %sub41 = fsub double %sub40, %mul34
  %add42 = fadd double %sub41, %mul13.i
  %mul43 = fmul double %16, 0x3FDBCB7B15200000
  %mul44 = fmul double %conv, 0x3FD34413509F6000
  %add46 = fadd double %add42, %16
  %mul47 = fmul double %add46, 0x3DBB9438CA9AADD5
  %17 = tail call double @llvm.fmuladd.f64(double %conv, double 0x3D59FEF311F12B36, double %mul47)
  %18 = tail call double @llvm.fmuladd.f64(double %add42, double 0x3FDBCB7B15200000, double %17)
  %add49 = fadd double %mul44, %mul43
  %sub50 = fsub double %mul44, %add49
  %add51 = fadd double %mul43, %sub50
  %add52 = fadd double %add51, %18
  %add53 = fadd double %add49, %add52
  br label %cleanup

cleanup:                                          ; preds = %if.end15, %if.end19, %if.then14, %if.then4, %if.then2
  %retval.0 = phi double [ %div, %if.then2 ], [ %div5, %if.then4 ], [ %add, %if.then14 ], [ %add53, %if.end19 ], [ 0.000000e+00, %if.end15 ]
  ret double %retval.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fmuladd.f64(double, double, double) #1

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare <2 x double> @llvm.fmuladd.v2f64(<2 x double>, <2 x double>, <2 x double>) #2

attributes #0 = { mustprogress nofree norecurse nounwind willreturn memory(readwrite, argmem: none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

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
