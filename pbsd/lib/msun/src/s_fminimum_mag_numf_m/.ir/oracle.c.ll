; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/s_fminimum_mag_numf_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/s_fminimum_mag_numf_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%union.IEEEf2bits = type { float }

; Function Attrs: nofree norecurse nounwind memory(inaccessiblemem: readwrite) uwtable
define dso_local float @ref_fminimum_mag_numf(float noundef %x, float noundef %y) local_unnamed_addr #0 {
entry:
  %u = alloca [2 x %union.IEEEf2bits], align 4
  %force_except = alloca float, align 4
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %u) #3
  store float %x, ptr %u, align 4, !tbaa !5
  %arrayidx1 = getelementptr inbounds [2 x %union.IEEEf2bits], ptr %u, i64 0, i64 1
  store float %y, ptr %arrayidx1, align 4, !tbaa !5
  %cmp.i = fcmp ord float %x, 0.000000e+00
  %brmerge.demorgan = fcmp uno float %x, %y
  %0 = bitcast float %x to i32
  %1 = bitcast float %y to i32
  br i1 %brmerge.demorgan, label %if.then, label %if.end13

if.then:                                          ; preds = %entry
  %cmp.i55 = fcmp ord float %y, 0.000000e+00
  %brmerge54 = or i1 %cmp.i, %cmp.i55
  br i1 %brmerge54, label %if.end, label %if.then9

if.then9:                                         ; preds = %if.then
  %add = fadd float %x, %y
  br label %cleanup32

if.end:                                           ; preds = %if.then
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %force_except)
  %add10 = fadd float %x, %y
  store volatile float %add10, ptr %force_except, align 4, !tbaa !8
  %force_except.0.force_except.0.force_except.0.force_except.0. = load volatile float, ptr %force_except, align 4, !tbaa !8
  %x.y = select i1 %cmp.i, float %x, float %y
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %force_except)
  br label %cleanup32

if.end13:                                         ; preds = %entry
  %2 = tail call float @llvm.fabs.f32(float %x)
  %3 = tail call float @llvm.fabs.f32(float %y)
  %cmp = fcmp olt float %3, %2
  br i1 %cmp, label %cleanup32, label %if.end15

if.end15:                                         ; preds = %if.end13
  %cmp16 = fcmp olt float %2, %3
  br i1 %cmp16, label %cleanup32, label %if.end18

if.end18:                                         ; preds = %if.end15
  %bf.lshr = lshr i32 %0, 31
  %bf.lshr22 = lshr i32 %1, 31
  %cmp23.not = icmp eq i32 %bf.lshr, %bf.lshr22
  br i1 %cmp23.not, label %cleanup32, label %if.then24

if.then24:                                        ; preds = %if.end18
  %idxprom = zext nneg i32 %bf.lshr22 to i64
  %arrayidx28 = getelementptr inbounds [2 x %union.IEEEf2bits], ptr %u, i64 0, i64 %idxprom
  %4 = load float, ptr %arrayidx28, align 4, !tbaa !5
  br label %cleanup32

cleanup32:                                        ; preds = %if.then24, %if.end13, %if.end15, %if.end18, %if.end, %if.then9
  %retval.2 = phi float [ %add, %if.then9 ], [ %x.y, %if.end ], [ %4, %if.then24 ], [ %y, %if.end13 ], [ %x, %if.end15 ], [ %x, %if.end18 ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %u) #3
  ret float %retval.2
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fabs.f32(float) #2

attributes #0 = { nofree norecurse nounwind memory(inaccessiblemem: readwrite) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
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
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!9, !9, i64 0}
!9 = !{!"float", !6, i64 0}
