; ModuleID = '/home/odin/pbsd/pbsd/sbin/ipf/libipf/facpri_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/sbin/ipf/libipf/facpri_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

module asm ".symver __qsort_r_compat, qsort_r@FBSD_1.0"

%struct.table = type { ptr, i32 }

@.str = private unnamed_addr constant [5 x i8] c"kern\00", align 1
@.str.1 = private unnamed_addr constant [5 x i8] c"user\00", align 1
@.str.2 = private unnamed_addr constant [5 x i8] c"mail\00", align 1
@.str.3 = private unnamed_addr constant [7 x i8] c"daemon\00", align 1
@.str.4 = private unnamed_addr constant [5 x i8] c"auth\00", align 1
@.str.5 = private unnamed_addr constant [7 x i8] c"syslog\00", align 1
@.str.6 = private unnamed_addr constant [4 x i8] c"lpr\00", align 1
@.str.7 = private unnamed_addr constant [5 x i8] c"news\00", align 1
@.str.8 = private unnamed_addr constant [5 x i8] c"uucp\00", align 1
@.str.9 = private unnamed_addr constant [5 x i8] c"cron\00", align 1
@.str.10 = private unnamed_addr constant [4 x i8] c"ftp\00", align 1
@.str.11 = private unnamed_addr constant [9 x i8] c"authpriv\00", align 1
@.str.12 = private unnamed_addr constant [9 x i8] c"security\00", align 1
@.str.13 = private unnamed_addr constant [7 x i8] c"local0\00", align 1
@.str.14 = private unnamed_addr constant [7 x i8] c"local1\00", align 1
@.str.15 = private unnamed_addr constant [7 x i8] c"local2\00", align 1
@.str.16 = private unnamed_addr constant [7 x i8] c"local3\00", align 1
@.str.17 = private unnamed_addr constant [7 x i8] c"local4\00", align 1
@.str.18 = private unnamed_addr constant [7 x i8] c"local5\00", align 1
@.str.19 = private unnamed_addr constant [7 x i8] c"local6\00", align 1
@.str.20 = private unnamed_addr constant [7 x i8] c"local7\00", align 1
@facs = dso_local local_unnamed_addr global [23 x %struct.table] [%struct.table { ptr @.str, i32 0 }, %struct.table { ptr @.str.1, i32 8 }, %struct.table { ptr @.str.2, i32 16 }, %struct.table { ptr @.str.3, i32 24 }, %struct.table { ptr @.str.4, i32 32 }, %struct.table { ptr @.str.5, i32 40 }, %struct.table { ptr @.str.6, i32 48 }, %struct.table { ptr @.str.7, i32 56 }, %struct.table { ptr @.str.8, i32 64 }, %struct.table { ptr @.str.9, i32 72 }, %struct.table { ptr @.str.10, i32 88 }, %struct.table { ptr @.str.11, i32 80 }, %struct.table { ptr @.str.9, i32 120 }, %struct.table { ptr @.str.12, i32 104 }, %struct.table { ptr @.str.13, i32 128 }, %struct.table { ptr @.str.14, i32 136 }, %struct.table { ptr @.str.15, i32 144 }, %struct.table { ptr @.str.16, i32 152 }, %struct.table { ptr @.str.17, i32 160 }, %struct.table { ptr @.str.18, i32 168 }, %struct.table { ptr @.str.19, i32 176 }, %struct.table { ptr @.str.20, i32 184 }, %struct.table zeroinitializer], align 16
@.str.21 = private unnamed_addr constant [6 x i8] c"emerg\00", align 1
@.str.22 = private unnamed_addr constant [6 x i8] c"alert\00", align 1
@.str.23 = private unnamed_addr constant [5 x i8] c"crit\00", align 1
@.str.24 = private unnamed_addr constant [4 x i8] c"err\00", align 1
@.str.25 = private unnamed_addr constant [5 x i8] c"warn\00", align 1
@.str.26 = private unnamed_addr constant [7 x i8] c"notice\00", align 1
@.str.27 = private unnamed_addr constant [5 x i8] c"info\00", align 1
@.str.28 = private unnamed_addr constant [6 x i8] c"debug\00", align 1
@pris = dso_local local_unnamed_addr global [9 x %struct.table] [%struct.table { ptr @.str.21, i32 0 }, %struct.table { ptr @.str.22, i32 1 }, %struct.table { ptr @.str.23, i32 2 }, %struct.table { ptr @.str.24, i32 3 }, %struct.table { ptr @.str.25, i32 4 }, %struct.table { ptr @.str.26, i32 5 }, %struct.table { ptr @.str.27, i32 6 }, %struct.table { ptr @.str.28, i32 7 }, %struct.table zeroinitializer], align 16

; Function Attrs: nofree norecurse nosync nounwind memory(read, argmem: none, inaccessiblemem: none) uwtable
define dso_local ptr @ref_fac_toname(i32 noundef %facpri) local_unnamed_addr #0 {
entry:
  %and = and i32 %facpri, 1016
  %cmp = icmp ult i32 %and, 184
  br i1 %cmp, label %if.then, label %if.end7

if.then:                                          ; preds = %entry
  %shr = lshr exact i32 %and, 3
  %conv = zext nneg i32 %shr to i64
  %value = getelementptr inbounds [23 x %struct.table], ptr @facs, i64 0, i64 %conv, i32 1
  %0 = load i32, ptr %value, align 8, !tbaa !5
  %cmp2 = icmp eq i32 %0, %and
  br i1 %cmp2, label %if.then4, label %if.end7

if.then4:                                         ; preds = %if.then
  %arrayidx = getelementptr inbounds [23 x %struct.table], ptr @facs, i64 0, i64 %conv
  %1 = load ptr, ptr %arrayidx, align 16, !tbaa !11
  br label %cleanup

if.end7:                                          ; preds = %if.then, %entry
  %2 = load ptr, ptr @facs, align 16, !tbaa !11
  %tobool.not30 = icmp eq ptr %2, null
  br i1 %tobool.not30, label %cleanup, label %for.body.preheader

for.body.preheader:                               ; preds = %if.end7
  %3 = load i32, ptr getelementptr inbounds ([23 x %struct.table], ptr @facs, i64 0, i64 0, i32 1), align 8, !tbaa !5
  %cmp1435 = icmp eq i32 %and, %3
  br i1 %cmp1435, label %cleanup, label %for.cond

for.cond:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv36 = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %indvars.iv.next = add nuw i64 %indvars.iv36, 1
  %arrayidx9 = getelementptr inbounds [23 x %struct.table], ptr @facs, i64 0, i64 %indvars.iv.next
  %4 = load ptr, ptr %arrayidx9, align 16, !tbaa !11
  %tobool.not = icmp eq ptr %4, null
  br i1 %tobool.not, label %cleanup, label %for.body, !llvm.loop !12

for.body:                                         ; preds = %for.cond
  %value13 = getelementptr inbounds [23 x %struct.table], ptr @facs, i64 0, i64 %indvars.iv.next, i32 1
  %5 = load i32, ptr %value13, align 8, !tbaa !5
  %cmp14 = icmp eq i32 %and, %5
  br i1 %cmp14, label %cleanup, label %for.cond, !llvm.loop !12

cleanup:                                          ; preds = %for.cond, %for.body, %for.body.preheader, %if.end7, %if.then4
  %retval.0 = phi ptr [ %1, %if.then4 ], [ null, %if.end7 ], [ %2, %for.body.preheader ], [ %4, %for.body ], [ null, %for.cond ]
  ret ptr %retval.0
}

; Function Attrs: nofree nounwind memory(read, inaccessiblemem: none) uwtable
define dso_local i32 @ref_fac_findname(ptr nocapture noundef readonly %name) local_unnamed_addr #1 {
entry:
  %0 = load ptr, ptr @facs, align 16, !tbaa !11
  %tobool.not12 = icmp eq ptr %0, null
  br i1 %tobool.not12, label %cleanup, label %for.body

for.cond:                                         ; preds = %for.body
  %indvars.iv.next = add nuw i64 %indvars.iv, 1
  %arrayidx = getelementptr inbounds [23 x %struct.table], ptr @facs, i64 0, i64 %indvars.iv.next
  %1 = load ptr, ptr %arrayidx, align 16, !tbaa !11
  %tobool.not = icmp eq ptr %1, null
  br i1 %tobool.not, label %cleanup, label %for.body, !llvm.loop !14

for.body:                                         ; preds = %entry, %for.cond
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.cond ], [ 0, %entry ]
  %2 = phi ptr [ %1, %for.cond ], [ %0, %entry ]
  %call = tail call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %2, ptr noundef nonnull dereferenceable(1) %name) #3
  %tobool5.not = icmp eq i32 %call, 0
  br i1 %tobool5.not, label %if.then, label %for.cond

if.then:                                          ; preds = %for.body
  %value = getelementptr inbounds [23 x %struct.table], ptr @facs, i64 0, i64 %indvars.iv, i32 1
  %3 = load i32, ptr %value, align 8, !tbaa !5
  br label %cleanup

cleanup:                                          ; preds = %for.cond, %entry, %if.then
  %retval.0 = phi i32 [ %3, %if.then ], [ -1, %entry ], [ -1, %for.cond ]
  ret i32 %retval.0
}

; Function Attrs: mustprogress nofree nounwind willreturn memory(argmem: read)
declare i32 @strcmp(ptr nocapture noundef, ptr nocapture noundef) local_unnamed_addr #2

; Function Attrs: nofree nounwind memory(read, inaccessiblemem: none) uwtable
define dso_local i32 @ref_pri_findname(ptr nocapture noundef readonly %name) local_unnamed_addr #1 {
entry:
  %0 = load ptr, ptr @pris, align 16, !tbaa !11
  %tobool.not12 = icmp eq ptr %0, null
  br i1 %tobool.not12, label %cleanup, label %for.body

for.cond:                                         ; preds = %for.body
  %indvars.iv.next = add nuw i64 %indvars.iv, 1
  %arrayidx = getelementptr inbounds [9 x %struct.table], ptr @pris, i64 0, i64 %indvars.iv.next
  %1 = load ptr, ptr %arrayidx, align 16, !tbaa !11
  %tobool.not = icmp eq ptr %1, null
  br i1 %tobool.not, label %cleanup, label %for.body, !llvm.loop !15

for.body:                                         ; preds = %entry, %for.cond
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.cond ], [ 0, %entry ]
  %2 = phi ptr [ %1, %for.cond ], [ %0, %entry ]
  %call = tail call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %2, ptr noundef nonnull dereferenceable(1) %name) #3
  %tobool5.not = icmp eq i32 %call, 0
  br i1 %tobool5.not, label %if.then, label %for.cond

if.then:                                          ; preds = %for.body
  %value = getelementptr inbounds [9 x %struct.table], ptr @pris, i64 0, i64 %indvars.iv, i32 1
  %3 = load i32, ptr %value, align 8, !tbaa !5
  br label %cleanup

cleanup:                                          ; preds = %for.cond, %entry, %if.then
  %retval.0 = phi i32 [ %3, %if.then ], [ -1, %entry ], [ -1, %for.cond ]
  ret i32 %retval.0
}

; Function Attrs: nofree norecurse nosync nounwind memory(read, argmem: none, inaccessiblemem: none) uwtable
define dso_local ptr @ref_pri_toname(i32 noundef %facpri) local_unnamed_addr #0 {
entry:
  %and = and i32 %facpri, 7
  %idxprom = zext nneg i32 %and to i64
  %value = getelementptr inbounds [9 x %struct.table], ptr @pris, i64 0, i64 %idxprom, i32 1
  %0 = load i32, ptr %value, align 8, !tbaa !5
  %cmp = icmp eq i32 %0, %and
  br i1 %cmp, label %if.then, label %for.cond.preheader

for.cond.preheader:                               ; preds = %entry
  %1 = load ptr, ptr @pris, align 16, !tbaa !11
  %tobool.not22 = icmp eq ptr %1, null
  br i1 %tobool.not22, label %cleanup, label %for.body.preheader

for.body.preheader:                               ; preds = %for.cond.preheader
  %2 = load i32, ptr getelementptr inbounds ([9 x %struct.table], ptr @pris, i64 0, i64 0, i32 1), align 8, !tbaa !5
  %cmp927 = icmp eq i32 %and, %2
  br i1 %cmp927, label %cleanup, label %for.cond

if.then:                                          ; preds = %entry
  %arrayidx = getelementptr inbounds [9 x %struct.table], ptr @pris, i64 0, i64 %idxprom
  %3 = load ptr, ptr %arrayidx, align 16, !tbaa !11
  br label %cleanup

for.cond:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv28 = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %indvars.iv.next = add nuw i64 %indvars.iv28, 1
  %arrayidx4 = getelementptr inbounds [9 x %struct.table], ptr @pris, i64 0, i64 %indvars.iv.next
  %4 = load ptr, ptr %arrayidx4, align 16, !tbaa !11
  %tobool.not = icmp eq ptr %4, null
  br i1 %tobool.not, label %cleanup, label %for.body, !llvm.loop !16

for.body:                                         ; preds = %for.cond
  %value8 = getelementptr inbounds [9 x %struct.table], ptr @pris, i64 0, i64 %indvars.iv.next, i32 1
  %5 = load i32, ptr %value8, align 8, !tbaa !5
  %cmp9 = icmp eq i32 %and, %5
  br i1 %cmp9, label %cleanup, label %for.cond, !llvm.loop !16

cleanup:                                          ; preds = %for.cond, %for.body, %for.body.preheader, %for.cond.preheader, %if.then
  %retval.0 = phi ptr [ %3, %if.then ], [ null, %for.cond.preheader ], [ %1, %for.body.preheader ], [ %4, %for.body ], [ null, %for.cond ]
  ret ptr %retval.0
}

attributes #0 = { nofree norecurse nosync nounwind memory(read, argmem: none, inaccessiblemem: none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nofree nounwind memory(read, inaccessiblemem: none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { mustprogress nofree nounwind willreturn memory(argmem: read) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nounwind willreturn memory(read) }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!5 = !{!6, !10, i64 8}
!6 = !{!"table", !7, i64 0, !10, i64 8}
!7 = !{!"any pointer", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{!"int", !8, i64 0}
!11 = !{!6, !7, i64 0}
!12 = distinct !{!12, !13}
!13 = !{!"llvm.loop.mustprogress"}
!14 = distinct !{!14, !13}
!15 = distinct !{!15, !13}
!16 = distinct !{!16, !13}
