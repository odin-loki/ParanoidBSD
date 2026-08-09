; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/e_atan2f_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/e_atan2f_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@tiny = internal global float 0x39B4484C00000000, align 4
@pi_lo = internal global float 0xBE7777A5C0000000, align 4

; Function Attrs: mustprogress nofree nounwind willreturn memory(readwrite, argmem: write) uwtable
define dso_local float @ref_atan2f(float noundef %y, float noundef %x) local_unnamed_addr #0 {
entry:
  %0 = bitcast float %x to i32
  %1 = tail call float @llvm.fabs.f32(float %x)
  %and = bitcast float %1 to i32
  %2 = bitcast float %y to i32
  %3 = tail call float @llvm.fabs.f32(float %y)
  %and5 = bitcast float %3 to i32
  %cmp = icmp sgt i32 %and, 2139095040
  %cmp6 = icmp sgt i32 %and5, 2139095040
  %or.cond = or i1 %cmp6, %cmp
  br i1 %or.cond, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %conv = fpext float %x to x86_fp80
  %add = fadd x86_fp80 %conv, 0xK00000000000000000000
  %add7 = fadd float %y, 0.000000e+00
  %conv8 = fpext float %add7 to x86_fp80
  %add9 = fadd x86_fp80 %add, %conv8
  %conv10 = fptrunc x86_fp80 %add9 to float
  br label %cleanup

if.end:                                           ; preds = %entry
  %cmp11 = icmp eq i32 %0, 1065353216
  br i1 %cmp11, label %if.then13, label %if.end14

if.then13:                                        ; preds = %if.end
  %call = tail call float @atanf(float noundef %y) #3
  br label %cleanup

if.end14:                                         ; preds = %if.end
  %shr126 = lshr i32 %2, 31
  %shr16 = lshr i32 %0, 30
  %and17 = and i32 %shr16, 2
  %or = or disjoint i32 %and17, %shr126
  %cmp18 = icmp eq i32 %and5, 0
  br i1 %cmp18, label %if.then20, label %if.end24

if.then20:                                        ; preds = %if.end14
  switch i32 %or, label %default.unreachable127 [
    i32 0, label %cleanup
    i32 1, label %cleanup
    i32 2, label %sw.bb21
    i32 3, label %sw.bb23
  ]

sw.bb21:                                          ; preds = %if.then20
  %4 = load volatile float, ptr @tiny, align 4, !tbaa !5
  %add22 = fadd float %4, 0x400921FB60000000
  br label %cleanup

sw.bb23:                                          ; preds = %if.then20
  %5 = load volatile float, ptr @tiny, align 4, !tbaa !5
  %sub = fsub float 0xC00921FB60000000, %5
  br label %cleanup

default.unreachable127:                           ; preds = %if.else, %if.then38, %if.then20
  unreachable

if.end24:                                         ; preds = %if.end14
  switch i32 %and, label %if.end56 [
    i32 0, label %if.then27
    i32 2139095040, label %if.then35
  ]

if.then27:                                        ; preds = %if.end24
  %cmp28 = icmp slt i32 %2, 0
  %6 = load volatile float, ptr @tiny, align 4, !tbaa !5
  br i1 %cmp28, label %cond.true, label %cond.false

cond.true:                                        ; preds = %if.then27
  %sub30 = fsub float 0xBFF921FB60000000, %6
  br label %cleanup

cond.false:                                       ; preds = %if.then27
  %add31 = fadd float %6, 0x3FF921FB60000000
  br label %cleanup

if.then35:                                        ; preds = %if.end24
  %cmp36 = icmp eq i32 %and5, 2139095040
  br i1 %cmp36, label %if.then38, label %if.else

if.then38:                                        ; preds = %if.then35
  switch i32 %or, label %default.unreachable127 [
    i32 0, label %sw.bb39
    i32 1, label %sw.bb41
    i32 2, label %sw.bb43
    i32 3, label %sw.bb45
  ]

sw.bb39:                                          ; preds = %if.then38
  %7 = load volatile float, ptr @tiny, align 4, !tbaa !5
  %add40 = fadd float %7, 0x3FE921FB60000000
  br label %cleanup

sw.bb41:                                          ; preds = %if.then38
  %8 = load volatile float, ptr @tiny, align 4, !tbaa !5
  %sub42 = fsub float 0xBFE921FB60000000, %8
  br label %cleanup

sw.bb43:                                          ; preds = %if.then38
  %9 = load volatile float, ptr @tiny, align 4, !tbaa !5
  %add44 = fadd float %9, 0x4002D97C80000000
  br label %cleanup

sw.bb45:                                          ; preds = %if.then38
  %10 = load volatile float, ptr @tiny, align 4, !tbaa !5
  %sub46 = fsub float 0xC002D97C80000000, %10
  br label %cleanup

if.else:                                          ; preds = %if.then35
  switch i32 %or, label %default.unreachable127 [
    i32 0, label %cleanup
    i32 1, label %sw.bb49
    i32 2, label %sw.bb50
    i32 3, label %sw.bb52
  ]

sw.bb49:                                          ; preds = %if.else
  br label %cleanup

sw.bb50:                                          ; preds = %if.else
  %11 = load volatile float, ptr @tiny, align 4, !tbaa !5
  %add51 = fadd float %11, 0x400921FB60000000
  br label %cleanup

sw.bb52:                                          ; preds = %if.else
  %12 = load volatile float, ptr @tiny, align 4, !tbaa !5
  %sub53 = fsub float 0xC00921FB60000000, %12
  br label %cleanup

if.end56:                                         ; preds = %if.end24
  %cmp57 = icmp eq i32 %and5, 2139095040
  br i1 %cmp57, label %if.then59, label %if.end68

if.then59:                                        ; preds = %if.end56
  %cmp60 = icmp slt i32 %2, 0
  %13 = load volatile float, ptr @tiny, align 4, !tbaa !5
  br i1 %cmp60, label %cond.true62, label %cond.false64

cond.true62:                                      ; preds = %if.then59
  %sub63 = fsub float 0xBFF921FB60000000, %13
  br label %cleanup

cond.false64:                                     ; preds = %if.then59
  %add65 = fadd float %13, 0x3FF921FB60000000
  br label %cleanup

if.end68:                                         ; preds = %if.end56
  %sub69 = sub nsw i32 %and5, %and
  %shr70 = ashr i32 %sub69, 23
  %cmp71 = icmp sgt i32 %shr70, 26
  br i1 %cmp71, label %if.then73, label %if.else75

if.then73:                                        ; preds = %if.end68
  %14 = load volatile float, ptr @pi_lo, align 4, !tbaa !5
  %15 = tail call float @llvm.fmuladd.f32(float %14, float 5.000000e-01, float 0x3FF921FB60000000)
  br label %if.end84

if.else75:                                        ; preds = %if.end68
  %cmp76 = icmp slt i32 %shr70, -26
  %cmp78 = icmp slt i32 %0, 0
  %or.cond98 = and i1 %cmp78, %cmp76
  br i1 %or.cond98, label %if.end84, label %if.else81

if.else81:                                        ; preds = %if.else75
  %div = fdiv float %y, %x
  %16 = tail call float @llvm.fabs.f32(float %div)
  %call82 = tail call float @atanf(float noundef %16) #3
  br label %if.end84

if.end84:                                         ; preds = %if.else75, %if.else81, %if.then73
  %m.0 = phi i32 [ %shr126, %if.then73 ], [ %or, %if.else81 ], [ %or, %if.else75 ]
  %z.0 = phi float [ %15, %if.then73 ], [ %call82, %if.else81 ], [ 0.000000e+00, %if.else75 ]
  switch i32 %m.0, label %sw.default [
    i32 0, label %cleanup
    i32 1, label %sw.bb86
    i32 2, label %sw.bb87
  ]

sw.bb86:                                          ; preds = %if.end84
  %fneg = fneg float %z.0
  br label %cleanup

sw.bb87:                                          ; preds = %if.end84
  %17 = load volatile float, ptr @pi_lo, align 4, !tbaa !5
  %18 = fsub float %17, %z.0
  %sub89 = fadd float %18, 0x400921FB60000000
  br label %cleanup

sw.default:                                       ; preds = %if.end84
  %19 = load volatile float, ptr @pi_lo, align 4, !tbaa !5
  %sub90 = fsub float %z.0, %19
  %sub91 = fadd float %sub90, 0xC00921FB60000000
  br label %cleanup

cleanup:                                          ; preds = %if.end84, %cond.true62, %cond.false64, %if.else, %cond.true, %cond.false, %if.then20, %if.then20, %sw.default, %sw.bb87, %sw.bb86, %sw.bb52, %sw.bb50, %sw.bb49, %sw.bb45, %sw.bb43, %sw.bb41, %sw.bb39, %sw.bb23, %sw.bb21, %if.then13, %if.then
  %retval.0 = phi float [ %conv10, %if.then ], [ %call, %if.then13 ], [ %sub91, %sw.default ], [ %sub89, %sw.bb87 ], [ %fneg, %sw.bb86 ], [ %sub46, %sw.bb45 ], [ %add44, %sw.bb43 ], [ %sub42, %sw.bb41 ], [ %add40, %sw.bb39 ], [ %sub53, %sw.bb52 ], [ %add51, %sw.bb50 ], [ -0.000000e+00, %sw.bb49 ], [ %sub, %sw.bb23 ], [ %add22, %sw.bb21 ], [ %y, %if.then20 ], [ %y, %if.then20 ], [ %sub30, %cond.true ], [ %add31, %cond.false ], [ 0.000000e+00, %if.else ], [ %sub63, %cond.true62 ], [ %add65, %cond.false64 ], [ %z.0, %if.end84 ]
  ret float %retval.0
}

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare float @atanf(float noundef) local_unnamed_addr #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fmuladd.f32(float, float, float) #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fabs.f32(float) #2

attributes #0 = { mustprogress nofree nounwind willreturn memory(readwrite, argmem: write) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
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
!5 = !{!6, !6, i64 0}
!6 = !{!"float", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
