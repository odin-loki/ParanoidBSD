; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/e_logf_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/e_logf_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@vzero = internal global float 0.000000e+00, align 4

; Function Attrs: mustprogress nofree norecurse nounwind willreturn memory(readwrite, argmem: none) uwtable
define dso_local float @ref_logf(float noundef %x) local_unnamed_addr #0 {
entry:
  %0 = bitcast float %x to i32
  %cmp = icmp slt i32 %0, 8388608
  br i1 %cmp, label %if.then, label %if.end12

if.then:                                          ; preds = %entry
  %1 = tail call float @llvm.fabs.f32(float %x)
  %and = bitcast float %1 to i32
  %cmp1 = icmp eq i32 %and, 0
  br i1 %cmp1, label %if.then2, label %if.end

if.then2:                                         ; preds = %if.then
  %2 = load volatile float, ptr @vzero, align 4, !tbaa !5
  %div = fdiv float 0xC180000000000000, %2
  br label %cleanup

if.end:                                           ; preds = %if.then
  %cmp3 = icmp slt i32 %0, 0
  br i1 %cmp3, label %if.then4, label %if.end6

if.then4:                                         ; preds = %if.end
  %sub = fsub float %x, %x
  %div5 = fdiv float %sub, 0.000000e+00
  br label %cleanup

if.end6:                                          ; preds = %if.end
  %mul = fmul float %x, 0x4180000000000000
  %3 = bitcast float %mul to i32
  br label %if.end12

if.end12:                                         ; preds = %if.end6, %entry
  %k.0 = phi i32 [ -152, %if.end6 ], [ -127, %entry ]
  %ix.0 = phi i32 [ %3, %if.end6 ], [ %0, %entry ]
  %x.addr.0 = phi float [ %mul, %if.end6 ], [ %x, %entry ]
  %cmp13 = icmp sgt i32 %ix.0, 2139095039
  br i1 %cmp13, label %if.then14, label %if.end15

if.then14:                                        ; preds = %if.end12
  %add = fadd float %x.addr.0, %x.addr.0
  br label %cleanup

if.end15:                                         ; preds = %if.end12
  %shr = ashr i32 %ix.0, 23
  %and18 = and i32 %ix.0, 8388607
  %add19 = add nuw nsw i32 %and18, 4913952
  %and20 = and i32 %add19, 8388608
  %4 = or disjoint i32 %and20, %and18
  %or = xor i32 %4, 1065353216
  %5 = bitcast i32 %or to float
  %shr24 = lshr i32 %add19, 23
  %add17 = add nsw i32 %k.0, %shr
  %add25 = add nsw i32 %add17, %shr24
  %sub26 = fadd float %5, -1.000000e+00
  %add27 = add nsw i32 %ix.0, 32768
  %and28 = and i32 %add27, 8372224
  %cmp29 = icmp ult i32 %and28, 49152
  br i1 %cmp29, label %if.then30, label %if.end51

if.then30:                                        ; preds = %if.end15
  %cmp31 = fcmp oeq float %sub26, 0.000000e+00
  br i1 %cmp31, label %if.then32, label %if.end37

if.then32:                                        ; preds = %if.then30
  %cmp33 = icmp eq i32 %add25, 0
  br i1 %cmp33, label %cleanup, label %if.else

if.else:                                          ; preds = %if.then32
  %conv = sitofp i32 %add25 to float
  %mul36 = fmul float %conv, 0x3EE2FEFA20000000
  %6 = tail call float @llvm.fmuladd.f32(float %conv, float 0x3FE62E3000000000, float %mul36)
  br label %cleanup

if.end37:                                         ; preds = %if.then30
  %mul38 = fmul float %sub26, %sub26
  %7 = tail call float @llvm.fmuladd.f32(float %sub26, float 0xBFD5555560000000, float 5.000000e-01)
  %mul40 = fmul float %mul38, %7
  %cmp41 = icmp eq i32 %add25, 0
  br i1 %cmp41, label %if.then43, label %if.else45

if.then43:                                        ; preds = %if.end37
  %sub44 = fsub float %sub26, %mul40
  br label %cleanup

if.else45:                                        ; preds = %if.end37
  %conv46 = sitofp i32 %add25 to float
  %neg = fneg float %conv46
  %8 = tail call float @llvm.fmuladd.f32(float %neg, float 0x3EE2FEFA20000000, float %mul40)
  %sub49 = fsub float %8, %sub26
  %neg50 = fneg float %sub49
  %9 = tail call float @llvm.fmuladd.f32(float %conv46, float 0x3FE62E3000000000, float %neg50)
  br label %cleanup

if.end51:                                         ; preds = %if.end15
  %add52 = fadd float %sub26, 2.000000e+00
  %div53 = fdiv float %sub26, %add52
  %conv54 = sitofp i32 %add25 to float
  %mul55 = fmul float %div53, %div53
  %sub56 = add nsw i32 %and18, -3187664
  %mul57 = fmul float %mul55, %mul55
  %sub58 = sub nsw i32 3523208, %and18
  %10 = tail call float @llvm.fmuladd.f32(float %mul57, float 0x3FCF13C4C0000000, float 0x3FD999C260000000)
  %mul60 = fmul float %mul57, %10
  %11 = tail call float @llvm.fmuladd.f32(float %mul57, float 0x3FD23D3DC0000000, float 0x3FE5555540000000)
  %mul62 = fmul float %mul55, %11
  %or63 = or i32 %sub56, %sub58
  %add64 = fadd float %mul62, %mul60
  %cmp65 = icmp sgt i32 %or63, 0
  br i1 %cmp65, label %if.then67, label %if.else85

if.then67:                                        ; preds = %if.end51
  %mul68 = fmul float %sub26, 5.000000e-01
  %mul69 = fmul float %sub26, %mul68
  %cmp70 = icmp eq i32 %add25, 0
  %add73 = fadd float %mul69, %add64
  br i1 %cmp70, label %if.then72, label %if.else77

if.then72:                                        ; preds = %if.then67
  %neg75 = fneg float %div53
  %12 = tail call float @llvm.fmuladd.f32(float %neg75, float %add73, float %mul69)
  %sub76 = fsub float %sub26, %12
  br label %cleanup

if.else77:                                        ; preds = %if.then67
  %mul81 = fmul float %conv54, 0x3EE2FEFA20000000
  %13 = tail call float @llvm.fmuladd.f32(float %div53, float %add73, float %mul81)
  %sub82 = fsub float %mul69, %13
  %sub83 = fsub float %sub82, %sub26
  %neg84 = fneg float %sub83
  %14 = tail call float @llvm.fmuladd.f32(float %conv54, float 0x3FE62E3000000000, float %neg84)
  br label %cleanup

if.else85:                                        ; preds = %if.end51
  %cmp86 = icmp eq i32 %add25, 0
  %sub89 = fsub float %sub26, %add64
  br i1 %cmp86, label %if.then88, label %if.else92

if.then88:                                        ; preds = %if.else85
  %neg91 = fneg float %div53
  %15 = tail call float @llvm.fmuladd.f32(float %neg91, float %sub89, float %sub26)
  br label %cleanup

if.else92:                                        ; preds = %if.else85
  %neg97 = fmul float %conv54, 0xBEE2FEFA20000000
  %16 = tail call float @llvm.fmuladd.f32(float %div53, float %sub89, float %neg97)
  %sub98 = fsub float %16, %sub26
  %neg99 = fneg float %sub98
  %17 = tail call float @llvm.fmuladd.f32(float %conv54, float 0x3FE62E3000000000, float %neg99)
  br label %cleanup

cleanup:                                          ; preds = %if.then32, %if.else92, %if.then88, %if.else77, %if.then72, %if.else45, %if.then43, %if.else, %if.then14, %if.then4, %if.then2
  %retval.0 = phi float [ %div, %if.then2 ], [ %div5, %if.then4 ], [ %add, %if.then14 ], [ %6, %if.else ], [ %sub44, %if.then43 ], [ %9, %if.else45 ], [ %sub76, %if.then72 ], [ %14, %if.else77 ], [ %15, %if.then88 ], [ %17, %if.else92 ], [ 0.000000e+00, %if.then32 ]
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
