; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/s_fminimum_num_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/s_fminimum_num_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%union.IEEEd2bits = type { double }

; Function Attrs: nofree norecurse nounwind memory(inaccessiblemem: readwrite) uwtable
define dso_local double @ref_fminimum_num(double noundef %x, double noundef %y) local_unnamed_addr #0 {
entry:
  %u = alloca [2 x %union.IEEEd2bits], align 16
  %force_except = alloca double, align 8
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %u) #2
  store double %x, ptr %u, align 16, !tbaa !5
  %arrayidx1 = getelementptr inbounds [2 x %union.IEEEd2bits], ptr %u, i64 0, i64 1
  store double %y, ptr %arrayidx1, align 8, !tbaa !5
  %bf.load.cast = bitcast double %x to i64
  %0 = and i64 %bf.load.cast, 9218868437227405312
  %cmp = icmp eq i64 %0, 9218868437227405312
  %bf.lshr5 = lshr i64 %bf.load.cast, 32
  %1 = trunc i64 %bf.lshr5 to i32
  %bf.cast7 = and i32 %1, 1048575
  %bf.cast11 = trunc i64 %bf.load.cast to i32
  %or = or i32 %bf.cast7, %bf.cast11
  %cmp12 = icmp ne i32 %or, 0
  %2 = and i1 %cmp, %cmp12
  %3 = bitcast double %y to i64
  %4 = and i64 %3, 9218868437227405312
  %cmp18 = icmp eq i64 %4, 9218868437227405312
  %bf.lshr22 = lshr i64 %3, 32
  %5 = trunc i64 %bf.lshr22 to i32
  %bf.cast24 = and i32 %5, 1048575
  %bf.cast28 = trunc i64 %3 to i32
  %or29 = or i32 %bf.cast24, %bf.cast28
  %cmp30 = icmp ne i32 %or29, 0
  %6 = and i1 %cmp18, %cmp30
  %brmerge = or i1 %2, %6
  br i1 %brmerge, label %if.then, label %if.end40

if.then:                                          ; preds = %entry
  %7 = and i1 %2, %6
  br i1 %7, label %if.then36, label %if.end

if.then36:                                        ; preds = %if.then
  %add = fadd double %x, %y
  br label %cleanup58

if.end:                                           ; preds = %if.then
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %force_except)
  %add37 = fadd double %x, %y
  store volatile double %add37, ptr %force_except, align 8, !tbaa !8
  %force_except.0.force_except.0.force_except.0.force_except.0. = load volatile double, ptr %force_except, align 8, !tbaa !8
  %y.x = select i1 %2, double %y, double %x
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %force_except)
  br label %cleanup58

if.end40:                                         ; preds = %entry
  %bf.lshr43 = lshr i64 %bf.load.cast, 63
  %bf.lshr47 = lshr i64 %3, 63
  %cmp49.not = icmp eq i64 %bf.lshr43, %bf.lshr47
  br i1 %cmp49.not, label %if.end56, label %if.then50

if.then50:                                        ; preds = %if.end40
  %arrayidx55 = getelementptr inbounds [2 x %union.IEEEd2bits], ptr %u, i64 0, i64 %bf.lshr47
  %8 = load double, ptr %arrayidx55, align 8, !tbaa !5
  br label %cleanup58

if.end56:                                         ; preds = %if.end40
  %cmp57 = fcmp olt double %x, %y
  %cond = select i1 %cmp57, double %x, double %y
  br label %cleanup58

cleanup58:                                        ; preds = %if.end56, %if.then50, %if.end, %if.then36
  %retval.1 = phi double [ %add, %if.then36 ], [ %y.x, %if.end ], [ %8, %if.then50 ], [ %cond, %if.end56 ]
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %u) #2
  ret double %retval.1
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
!9 = !{!"double", !6, i64 0}
