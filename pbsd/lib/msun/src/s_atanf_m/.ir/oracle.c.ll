; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/s_atanf_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/s_atanf_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@atanhi = internal unnamed_addr constant [4 x float] [float 0x3FDDAC6700000000, float 0x3FE921FB40000000, float 0x3FEF730BC0000000, float 0x3FF921FB40000000], align 16
@atanlo = internal constant [4 x float] [float 0x3E3586ED20000000, float 0x3E64442D00000000, float 0x3E6281F680000000, float 0x3E74442D00000000], align 16

; Function Attrs: mustprogress nofree norecurse nounwind willreturn memory(inaccessiblemem: readwrite) uwtable
define dso_local float @ref_atanf(float noundef %x) local_unnamed_addr #0 {
entry:
  %0 = bitcast float %x to i32
  %1 = tail call float @llvm.fabs.f32(float %x)
  %and = bitcast float %1 to i32
  %cmp = icmp sgt i32 %and, 1283457023
  br i1 %cmp, label %if.then, label %if.end6

if.then:                                          ; preds = %entry
  %cmp1 = icmp ugt i32 %and, 2139095040
  br i1 %cmp1, label %if.then2, label %if.end

if.then2:                                         ; preds = %if.then
  %add = fadd float %x, %x
  br label %cleanup

if.end:                                           ; preds = %if.then
  %cmp3 = icmp sgt i32 %0, 0
  %2 = load volatile float, ptr getelementptr inbounds ([4 x float], ptr @atanlo, i64 0, i64 3), align 4, !tbaa !5
  br i1 %cmp3, label %if.then4, label %if.else

if.then4:                                         ; preds = %if.end
  %add5 = fadd float %2, 0x3FF921FB40000000
  br label %cleanup

if.else:                                          ; preds = %if.end
  %sub = fsub float 0xBFF921FB40000000, %2
  br label %cleanup

if.end6:                                          ; preds = %entry
  %cmp7 = icmp slt i32 %and, 1054867456
  br i1 %cmp7, label %if.then8, label %if.else16

if.then8:                                         ; preds = %if.end6
  %cmp9 = icmp slt i32 %and, 964689920
  %add11 = fadd float %x, 0x46293E5940000000
  %cmp12 = fcmp ogt float %add11, 1.000000e+00
  %or.cond = and i1 %cmp12, %cmp9
  br i1 %or.cond, label %cleanup, label %if.end36

if.else16:                                        ; preds = %if.end6
  %cmp17 = icmp ult i32 %and, 1066926080
  br i1 %cmp17, label %if.then18, label %if.else27

if.then18:                                        ; preds = %if.else16
  %cmp19 = icmp ult i32 %and, 1060110336
  br i1 %cmp19, label %if.then20, label %if.else22

if.then20:                                        ; preds = %if.then18
  %3 = tail call float @llvm.fmuladd.f32(float %1, float 2.000000e+00, float -1.000000e+00)
  %add21 = fadd float %1, 2.000000e+00
  %div = fdiv float %3, %add21
  br label %if.end36

if.else22:                                        ; preds = %if.then18
  %sub23 = fadd float %1, -1.000000e+00
  %add24 = fadd float %1, 1.000000e+00
  %div25 = fdiv float %sub23, %add24
  br label %if.end36

if.else27:                                        ; preds = %if.else16
  %cmp28 = icmp ult i32 %and, 1075576832
  br i1 %cmp28, label %if.then29, label %if.else32

if.then29:                                        ; preds = %if.else27
  %sub30 = fadd float %1, -1.500000e+00
  %4 = tail call float @llvm.fmuladd.f32(float %1, float 1.500000e+00, float 1.000000e+00)
  %div31 = fdiv float %sub30, %4
  br label %if.end36

if.else32:                                        ; preds = %if.else27
  %div33 = fdiv float -1.000000e+00, %1
  br label %if.end36

if.end36:                                         ; preds = %if.then8, %if.else22, %if.then20, %if.else32, %if.then29
  %x.addr.0 = phi float [ %div, %if.then20 ], [ %div25, %if.else22 ], [ %div31, %if.then29 ], [ %div33, %if.else32 ], [ %x, %if.then8 ]
  %id.0 = phi i64 [ 0, %if.then20 ], [ 1, %if.else22 ], [ 2, %if.then29 ], [ 3, %if.else32 ], [ 4294967295, %if.then8 ]
  %mul = fmul float %x.addr.0, %x.addr.0
  %mul37 = fmul float %mul, %mul
  %5 = tail call float @llvm.fmuladd.f32(float %mul37, float 0x3FAF9584A0000000, float 0x3FC23EA1A0000000)
  %6 = tail call float @llvm.fmuladd.f32(float %mul37, float %5, float 0x3FD5555520000000)
  %mul40 = fmul float %mul, %6
  %7 = tail call float @llvm.fmuladd.f32(float %mul37, float 0xBFBB4248E0000000, float 0xBFC9995300000000)
  %mul42 = fmul float %mul37, %7
  br i1 %cmp7, label %if.then44, label %if.else47

if.then44:                                        ; preds = %if.end36
  %add45 = fadd float %mul42, %mul40
  %neg = fneg float %x.addr.0
  %8 = tail call float @llvm.fmuladd.f32(float %neg, float %add45, float %x.addr.0)
  br label %cleanup

if.else47:                                        ; preds = %if.end36
  %arrayidx = getelementptr inbounds [4 x float], ptr @atanhi, i64 0, i64 %id.0
  %9 = load float, ptr %arrayidx, align 4, !tbaa !5
  %add48 = fadd float %mul42, %mul40
  %arrayidx51 = getelementptr inbounds [4 x float], ptr @atanlo, i64 0, i64 %id.0
  %10 = load float, ptr %arrayidx51, align 4, !tbaa !5
  %neg52 = fneg float %10
  %11 = tail call float @llvm.fmuladd.f32(float %x.addr.0, float %add48, float %neg52)
  %sub53 = fsub float %11, %x.addr.0
  %sub54 = fsub float %9, %sub53
  %cmp55 = icmp slt i32 %0, 0
  %fneg56 = fneg float %sub54
  %cond = select i1 %cmp55, float %fneg56, float %sub54
  br label %cleanup

cleanup:                                          ; preds = %if.then8, %if.else47, %if.then44, %if.else, %if.then4, %if.then2
  %retval.0 = phi float [ %add, %if.then2 ], [ %add5, %if.then4 ], [ %sub, %if.else ], [ %8, %if.then44 ], [ %cond, %if.else47 ], [ %x, %if.then8 ]
  ret float %retval.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fabs.f32(float) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fmuladd.f32(float, float, float) #1

attributes #0 = { mustprogress nofree norecurse nounwind willreturn memory(inaccessiblemem: readwrite) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }

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
