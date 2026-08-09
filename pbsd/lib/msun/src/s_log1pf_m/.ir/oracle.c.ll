; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/s_log1pf_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/s_log1pf_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@vzero = internal global float 0.000000e+00, align 4

; Function Attrs: mustprogress nofree norecurse nounwind willreturn memory(readwrite, argmem: none) uwtable
define dso_local float @ref_log1pf(float noundef %x) local_unnamed_addr #0 {
entry:
  %0 = bitcast float %x to i32
  %1 = tail call float @llvm.fabs.f32(float %x)
  %and = bitcast float %1 to i32
  %cmp = icmp slt i32 %0, 1054086096
  br i1 %cmp, label %if.then, label %if.end19

if.then:                                          ; preds = %entry
  %cmp1 = icmp sgt i32 %and, 1065353215
  br i1 %cmp1, label %if.then2, label %if.end

if.then2:                                         ; preds = %if.then
  %cmp3 = fcmp oeq float %x, -1.000000e+00
  br i1 %cmp3, label %if.then4, label %if.else

if.then4:                                         ; preds = %if.then2
  %2 = load volatile float, ptr @vzero, align 4, !tbaa !5
  %div = fdiv float 0xC180000000000000, %2
  br label %cleanup

if.else:                                          ; preds = %if.then2
  %sub = fsub float %x, %x
  %div6 = fdiv float %sub, %sub
  br label %cleanup

if.end:                                           ; preds = %if.then
  %cmp7 = icmp slt i32 %and, 939524096
  br i1 %cmp7, label %if.then8, label %if.end23

if.then8:                                         ; preds = %if.end
  %add = fadd float %x, 0x4180000000000000
  %cmp9 = fcmp ogt float %add, 0.000000e+00
  %cmp10 = icmp slt i32 %and, 864026624
  %or.cond = and i1 %cmp9, %cmp10
  br i1 %or.cond, label %cleanup, label %if.else12

if.else12:                                        ; preds = %if.then8
  %3 = fneg float %x
  %neg = fmul float %3, %x
  %4 = tail call float @llvm.fmuladd.f32(float %neg, float 5.000000e-01, float %x)
  br label %cleanup

if.end19:                                         ; preds = %entry
  %cmp20 = icmp ugt i32 %0, 2139095039
  br i1 %cmp20, label %if.then21, label %if.then25

if.then21:                                        ; preds = %if.end19
  %add22 = fadd float %x, %x
  br label %cleanup

if.end23:                                         ; preds = %if.end
  %5 = add i32 %0, -1
  %or.cond131 = icmp ult i32 %5, -1097468391
  br i1 %or.cond131, label %if.end65.thread, label %if.then27

if.end65.thread:                                  ; preds = %if.end23
  %mul66217 = fmul float %x, 5.000000e-01
  %mul67218 = fmul float %mul66217, %x
  br label %if.end93

if.then25:                                        ; preds = %if.end19
  %cmp26 = icmp ult i32 %0, 1509949440
  br i1 %cmp26, label %if.then27, label %if.else40

if.then27:                                        ; preds = %if.end23, %if.then25
  %add28 = fadd float %x, 1.000000e+00
  %6 = bitcast float %add28 to i32
  %shr = ashr i32 %6, 23
  %cmp34 = icmp sgt i32 %shr, 127
  %7 = fsub float %x, %add28
  %sub36 = fadd float %7, 1.000000e+00
  %sub37 = fadd float %add28, -1.000000e+00
  %sub38 = fsub float %x, %sub37
  %cond = select i1 %cmp34, float %sub36, float %sub38
  %div39 = fdiv float %cond, %add28
  br label %if.end47

if.else40:                                        ; preds = %if.then25
  %shr45 = lshr i32 %0, 23
  br label %if.end47

if.end47:                                         ; preds = %if.else40, %if.then27
  %hu.1 = phi i32 [ %6, %if.then27 ], [ %0, %if.else40 ]
  %k.1.in = phi i32 [ %shr, %if.then27 ], [ %shr45, %if.else40 ]
  %c.0 = phi float [ %div39, %if.then27 ], [ 0.000000e+00, %if.else40 ]
  %and48 = and i32 %hu.1, 8388607
  %cmp49 = icmp ult i32 %and48, 3474676
  br i1 %cmp49, label %do.body51, label %if.else54

do.body51:                                        ; preds = %if.end47
  %k.1 = add nsw i32 %k.1.in, -127
  %or = or disjoint i32 %and48, 1065353216
  br label %if.end65

if.else54:                                        ; preds = %if.end47
  %add55 = add nsw i32 %k.1.in, -126
  %or58 = or disjoint i32 %and48, 1056964608
  %sub61 = sub nuw nsw i32 8388608, %and48
  %shr62 = lshr i32 %sub61, 2
  br label %if.end65

if.end65:                                         ; preds = %do.body51, %if.else54
  %hu.2 = phi i32 [ %and48, %do.body51 ], [ %shr62, %if.else54 ]
  %k.2 = phi i32 [ %k.1, %do.body51 ], [ %add55, %if.else54 ]
  %u.0.in = phi i32 [ %or, %do.body51 ], [ %or58, %if.else54 ]
  %u.0 = bitcast i32 %u.0.in to float
  %sub64 = fadd float %u.0, -1.000000e+00
  %8 = icmp eq i32 %hu.2, 0
  %mul66 = fmul float %sub64, 5.000000e-01
  %mul67 = fmul float %sub64, %mul66
  br i1 %8, label %if.then69, label %if.end93

if.then69:                                        ; preds = %if.end65
  %cmp70 = fcmp oeq float %sub64, 0.000000e+00
  br i1 %cmp70, label %if.then71, label %if.end78

if.then71:                                        ; preds = %if.then69
  %cmp72 = icmp eq i32 %k.2, 0
  br i1 %cmp72, label %cleanup, label %if.else74

if.else74:                                        ; preds = %if.then71
  %conv = sitofp i32 %k.2 to float
  %9 = tail call float @llvm.fmuladd.f32(float %conv, float 0x3EE2FEFA20000000, float %c.0)
  %10 = tail call float @llvm.fmuladd.f32(float %conv, float 0x3FE62E3000000000, float %9)
  br label %cleanup

if.end78:                                         ; preds = %if.then69
  %11 = tail call float @llvm.fmuladd.f32(float %sub64, float 0xBFE5555560000000, float 1.000000e+00)
  %mul80 = fmul float %11, %mul67
  %cmp81 = icmp eq i32 %k.2, 0
  br i1 %cmp81, label %if.then83, label %if.else85

if.then83:                                        ; preds = %if.end78
  %sub84 = fsub float %sub64, %mul80
  br label %cleanup

if.else85:                                        ; preds = %if.end78
  %conv86 = sitofp i32 %k.2 to float
  %12 = tail call float @llvm.fmuladd.f32(float %conv86, float 0x3EE2FEFA20000000, float %c.0)
  %sub90 = fsub float %mul80, %12
  %sub91 = fsub float %sub90, %sub64
  %neg92 = fneg float %sub91
  %13 = tail call float @llvm.fmuladd.f32(float %conv86, float 0x3FE62E3000000000, float %neg92)
  br label %cleanup

if.end93:                                         ; preds = %if.end65.thread, %if.end65
  %mul67222 = phi float [ %mul67218, %if.end65.thread ], [ %mul67, %if.end65 ]
  %f.1221 = phi float [ %x, %if.end65.thread ], [ %sub64, %if.end65 ]
  %c.1220 = phi float [ undef, %if.end65.thread ], [ %c.0, %if.end65 ]
  %k.3219 = phi i32 [ 0, %if.end65.thread ], [ %k.2, %if.end65 ]
  %add94 = fadd float %f.1221, 2.000000e+00
  %div95 = fdiv float %f.1221, %add94
  %mul96 = fmul float %div95, %div95
  %14 = tail call float @llvm.fmuladd.f32(float %mul96, float 0x3FC2F112E0000000, float 0x3FC39A09E0000000)
  %15 = tail call float @llvm.fmuladd.f32(float %mul96, float %14, float 0x3FC74664A0000000)
  %16 = tail call float @llvm.fmuladd.f32(float %mul96, float %15, float 0x3FCC71C520000000)
  %17 = tail call float @llvm.fmuladd.f32(float %mul96, float %16, float 0x3FD24924A0000000)
  %18 = tail call float @llvm.fmuladd.f32(float %mul96, float %17, float 0x3FD99999A0000000)
  %19 = tail call float @llvm.fmuladd.f32(float %mul96, float %18, float 0x3FE5555560000000)
  %mul103 = fmul float %mul96, %19
  %cmp104 = icmp eq i32 %k.3219, 0
  br i1 %cmp104, label %if.then106, label %if.else111

if.then106:                                       ; preds = %if.end93
  %add107 = fadd float %mul67222, %mul103
  %neg109 = fneg float %div95
  %20 = tail call float @llvm.fmuladd.f32(float %neg109, float %add107, float %mul67222)
  %sub110 = fsub float %f.1221, %20
  br label %cleanup

if.else111:                                       ; preds = %if.end93
  %conv112 = sitofp i32 %k.3219 to float
  %add114 = fadd float %mul67222, %mul103
  %21 = tail call float @llvm.fmuladd.f32(float %conv112, float 0x3EE2FEFA20000000, float %c.1220)
  %22 = tail call float @llvm.fmuladd.f32(float %div95, float %add114, float %21)
  %sub118 = fsub float %mul67222, %22
  %sub119 = fsub float %sub118, %f.1221
  %neg120 = fneg float %sub119
  %23 = tail call float @llvm.fmuladd.f32(float %conv112, float 0x3FE62E3000000000, float %neg120)
  br label %cleanup

cleanup:                                          ; preds = %if.then71, %if.then8, %if.else111, %if.then106, %if.else85, %if.then83, %if.else74, %if.then21, %if.else12, %if.else, %if.then4
  %retval.0 = phi float [ %div, %if.then4 ], [ %div6, %if.else ], [ %4, %if.else12 ], [ %add22, %if.then21 ], [ %10, %if.else74 ], [ %sub84, %if.then83 ], [ %13, %if.else85 ], [ %sub110, %if.then106 ], [ %23, %if.else111 ], [ %x, %if.then8 ], [ 0.000000e+00, %if.then71 ]
  ret float %retval.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fmuladd.f32(float, float, float) #1

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fabs.f32(float) #2

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
!6 = !{!"float", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
