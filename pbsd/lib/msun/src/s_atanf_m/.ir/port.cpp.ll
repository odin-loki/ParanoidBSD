; ModuleID = '/tmp/port-dabaf1.pcm'
source_filename = "/tmp/port-dabaf1.pcm"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@_ZN4pbsd3lib4msun3src7s_atanfL6atanloE = internal constant [4 x float] [float 0x3E3586ED20000000, float 0x3E64442D00000000, float 0x3E6281F680000000, float 0x3E74442D00000000], align 16
@llvm.global_ctors = appending global [0 x { i32, ptr, ptr }] zeroinitializer

; Function Attrs: mustprogress nofree norecurse nounwind willreturn memory(inaccessiblemem: readwrite) uwtable
define dso_local noundef float @_ZN4pbsd3lib4msun3src7s_atanfW4pbsdW3libW4msunW3srcW7s_atanf5atanfEf(float noundef %x) local_unnamed_addr #0 {
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
  %2 = load volatile float, ptr getelementptr inbounds ([4 x float], ptr @_ZN4pbsd3lib4msun3src7s_atanfL6atanloE, i64 0, i64 3), align 4, !tbaa !5
  br i1 %cmp3, label %if.then4, label %if.else

if.then4:                                         ; preds = %if.end
  %add5 = fadd float %2, 0x3FF921FB40000000
  br label %cleanup

if.else:                                          ; preds = %if.end
  %sub = fsub float 0xBFF921FB40000000, %2
  br label %cleanup

if.end6:                                          ; preds = %entry
  %cmp7 = icmp sgt i32 %and, 1054867455
  %3 = tail call float @llvm.fmuladd.f32(float %1, float 2.000000e+00, float -1.000000e+00)
  %add21 = fadd float %1, 2.000000e+00
  %div = fdiv float %3, %add21
  %x.addr.0 = select i1 %cmp7, float %x, float %div
  %mul = fmul float %x.addr.0, %x.addr.0
  %mul37 = fmul float %mul, %mul
  %4 = tail call float @llvm.fmuladd.f32(float %mul37, float 0x3FAF9584A0000000, float 0x3FC23EA1A0000000)
  %5 = tail call float @llvm.fmuladd.f32(float %mul37, float %4, float 0x3FD5555520000000)
  %mul40 = fmul float %mul, %5
  %6 = tail call float @llvm.fmuladd.f32(float %mul37, float 0xBFBB4248E0000000, float 0xBFC9995300000000)
  %mul42 = fmul float %mul37, %6
  %add45 = fadd float %mul42, %mul40
  br i1 %cmp7, label %if.then44, label %if.else47

if.then44:                                        ; preds = %if.end6
  %neg = fneg float %x
  %7 = tail call float @llvm.fmuladd.f32(float %neg, float %add45, float %x)
  br label %cleanup

if.else47:                                        ; preds = %if.end6
  %8 = tail call float @llvm.fmuladd.f32(float %div, float %add45, float 0xBE3586ED20000000)
  %9 = fsub float %div, %8
  %sub54 = fadd float %9, 0x3FDDAC6700000000
  %cmp55 = icmp slt i32 %0, 0
  %fneg56 = fneg float %sub54
  %cond = select i1 %cmp55, float %fneg56, float %sub54
  br label %cleanup

cleanup:                                          ; preds = %if.else47, %if.then44, %if.else, %if.then4, %if.then2
  %retval.0 = phi float [ %add, %if.then2 ], [ %add5, %if.then4 ], [ %sub, %if.else ], [ %7, %if.then44 ], [ %cond, %if.else47 ]
  ret float %retval.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fabs.f32(float) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fmuladd.f32(float, float, float) #1

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none)
define void @_ZGIW4pbsdW3libW4msunW3srcW7s_atanf() local_unnamed_addr #2 section ".text.startup" {
entry:
  ret void
}

attributes #0 = { mustprogress nofree norecurse nounwind willreturn memory(inaccessiblemem: readwrite) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) "min-legal-vector-width"="0" }

!llvm.linker.options = !{}
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
!8 = !{!"Simple C++ TBAA"}
