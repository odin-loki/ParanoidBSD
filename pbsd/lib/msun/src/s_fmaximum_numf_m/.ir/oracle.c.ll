; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/s_fmaximum_numf_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/s_fmaximum_numf_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%union.IEEEf2bits = type { float }

; Function Attrs: nofree norecurse nounwind memory(inaccessiblemem: readwrite) uwtable
define dso_local float @ref_fmaximum_numf(float noundef %x, float noundef %y) local_unnamed_addr #0 {
entry:
  %u = alloca [2 x %union.IEEEf2bits], align 4
  %force_except = alloca float, align 4
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %u) #2
  store float %x, ptr %u, align 4, !tbaa !5
  %arrayidx1 = getelementptr inbounds [2 x %union.IEEEf2bits], ptr %u, i64 0, i64 1
  store float %y, ptr %arrayidx1, align 4, !tbaa !5
  %bf.load.cast = bitcast float %x to i32
  %0 = and i32 %bf.load.cast, 2139095040
  %cmp = icmp eq i32 %0, 2139095040
  %bf.clear5 = and i32 %bf.load.cast, 8388607
  %cmp6 = icmp ne i32 %bf.clear5, 0
  %1 = and i1 %cmp, %cmp6
  %2 = bitcast float %y to i32
  %3 = and i32 %2, 2139095040
  %cmp11 = icmp eq i32 %3, 2139095040
  %bf.clear15 = and i32 %2, 8388607
  %cmp16 = icmp ne i32 %bf.clear15, 0
  %4 = and i1 %cmp11, %cmp16
  %brmerge = or i1 %1, %4
  br i1 %brmerge, label %if.then, label %if.end26

if.then:                                          ; preds = %entry
  %5 = and i1 %1, %4
  br i1 %5, label %if.then22, label %if.end

if.then22:                                        ; preds = %if.then
  %add = fadd float %x, %y
  br label %cleanup41

if.end:                                           ; preds = %if.then
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %force_except)
  %add23 = fadd float %x, %y
  store volatile float %add23, ptr %force_except, align 4, !tbaa !8
  %force_except.0.force_except.0.force_except.0.force_except.0. = load volatile float, ptr %force_except, align 4, !tbaa !8
  %y.x = select i1 %1, float %y, float %x
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %force_except)
  br label %cleanup41

if.end26:                                         ; preds = %entry
  %bf.lshr29 = lshr i32 %bf.load.cast, 31
  %bf.lshr32 = lshr i32 %2, 31
  %cmp33.not = icmp eq i32 %bf.lshr29, %bf.lshr32
  br i1 %cmp33.not, label %if.end39, label %if.then34

if.then34:                                        ; preds = %if.end26
  %idxprom = zext nneg i32 %bf.lshr29 to i64
  %arrayidx38 = getelementptr inbounds [2 x %union.IEEEf2bits], ptr %u, i64 0, i64 %idxprom
  %6 = load float, ptr %arrayidx38, align 4, !tbaa !5
  br label %cleanup41

if.end39:                                         ; preds = %if.end26
  %cmp40 = fcmp ogt float %x, %y
  %cond = select i1 %cmp40, float %x, float %y
  br label %cleanup41

cleanup41:                                        ; preds = %if.end39, %if.then34, %if.end, %if.then22
  %retval.1 = phi float [ %add, %if.then22 ], [ %y.x, %if.end ], [ %6, %if.then34 ], [ %cond, %if.end39 ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %u) #2
  ret float %retval.1
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { nofree norecurse nounwind memory(inaccessiblemem: readwrite) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { nounwind }

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
