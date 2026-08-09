; ModuleID = '/home/odin/pbsd/pbsd/usr.bin/mesg/mesg_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/usr.bin/mesg/mesg_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

module asm ".symver __qsort_r_compat, qsort_r@FBSD_1.0"

%struct.stat = type { i64, i64, i64, i16, i16, i32, i32, i32, i64, %struct.timespec, %struct.timespec, %struct.timespec, %struct.timespec, i64, i64, i32, i32, i64, i64, [9 x i64] }
%struct.timespec = type { i64, i64 }

@.str = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@optind = external local_unnamed_addr global i32, align 4
@.str.1 = private unnamed_addr constant [8 x i8] c"ttyname\00", align 1
@.str.2 = private unnamed_addr constant [3 x i8] c"%s\00", align 1
@.str.3 = private unnamed_addr constant [5 x i8] c"is y\00", align 1
@.str.4 = private unnamed_addr constant [5 x i8] c"is n\00", align 1
@__stderrp = external local_unnamed_addr global ptr, align 8
@.str.5 = private unnamed_addr constant [21 x i8] c"usage: mesg [n | y]\0A\00", align 1

; Function Attrs: noreturn nounwind uwtable
define dso_local noundef i32 @ref_main(i32 noundef %argc, ptr noundef %argv) local_unnamed_addr #0 {
entry:
  %sb = alloca %struct.stat, align 8
  call void @llvm.lifetime.start.p0(i64 224, ptr nonnull %sb) #6
  %call = tail call i32 @getopt(i32 noundef %argc, ptr noundef %argv, ptr noundef nonnull @.str) #6
  %cmp.not = icmp eq i32 %call, -1
  br i1 %cmp.not, label %while.end, label %while.body

while.body:                                       ; preds = %entry
  tail call fastcc void @ref_usage() #7
  unreachable

while.end:                                        ; preds = %entry
  %0 = load i32, ptr @optind, align 4, !tbaa !5
  %idx.ext = sext i32 %0 to i64
  %add.ptr = getelementptr inbounds ptr, ptr %argv, i64 %idx.ext
  %call1 = tail call ptr @ttyname(i32 noundef 0) #6
  %cmp2 = icmp eq ptr %call1, null
  br i1 %cmp2, label %land.lhs.true, label %if.end

land.lhs.true:                                    ; preds = %while.end
  %call3 = tail call ptr @ttyname(i32 noundef 1) #6
  %cmp4 = icmp eq ptr %call3, null
  br i1 %cmp4, label %land.lhs.true5, label %if.end

land.lhs.true5:                                   ; preds = %land.lhs.true
  %call6 = tail call ptr @ttyname(i32 noundef 2) #6
  %cmp7 = icmp eq ptr %call6, null
  br i1 %cmp7, label %if.then, label %if.end

if.then:                                          ; preds = %land.lhs.true5
  tail call void (i32, ptr, ...) @err(i32 noundef 2, ptr noundef nonnull @.str.1) #8
  unreachable

if.end:                                           ; preds = %land.lhs.true5, %land.lhs.true, %while.end
  %tty.0 = phi ptr [ %call6, %land.lhs.true5 ], [ %call3, %land.lhs.true ], [ %call1, %while.end ]
  %call8 = call i32 @stat(ptr noundef nonnull %tty.0, ptr noundef nonnull %sb)
  %cmp9 = icmp slt i32 %call8, 0
  br i1 %cmp9, label %if.then10, label %if.end11

if.then10:                                        ; preds = %if.end
  tail call void (i32, ptr, ...) @err(i32 noundef 2, ptr noundef nonnull @.str.2, ptr noundef nonnull %tty.0) #8
  unreachable

if.end11:                                         ; preds = %if.end
  %1 = load ptr, ptr %add.ptr, align 8, !tbaa !9
  %cmp12 = icmp eq ptr %1, null
  br i1 %cmp12, label %if.then13, label %if.end18

if.then13:                                        ; preds = %if.end11
  %st_mode = getelementptr inbounds %struct.stat, ptr %sb, i64 0, i32 3
  %2 = load i16, ptr %st_mode, align 8, !tbaa !11
  %3 = and i16 %2, 16
  %tobool.not = icmp eq i16 %3, 0
  br i1 %tobool.not, label %if.end16, label %if.then14

if.then14:                                        ; preds = %if.then13
  %call15 = tail call i32 @puts(ptr noundef nonnull dereferenceable(1) @.str.3)
  tail call void @exit(i32 noundef 0) #8
  unreachable

if.end16:                                         ; preds = %if.then13
  %call17 = tail call i32 @puts(ptr noundef nonnull dereferenceable(1) @.str.4)
  tail call void @exit(i32 noundef 1) #8
  unreachable

if.end18:                                         ; preds = %if.end11
  %4 = load i8, ptr %1, align 1, !tbaa !16
  switch i8 %4, label %sw.epilog [
    i8 121, label %sw.bb20
    i8 110, label %sw.bb29
  ]

sw.bb20:                                          ; preds = %if.end18
  %st_mode21 = getelementptr inbounds %struct.stat, ptr %sb, i64 0, i32 3
  %5 = load i16, ptr %st_mode21, align 8, !tbaa !11
  %6 = or i16 %5, 16
  %call24 = tail call i32 @chmod(ptr noundef nonnull %tty.0, i16 noundef zeroext %6)
  %cmp25 = icmp slt i32 %call24, 0
  br i1 %cmp25, label %if.then27, label %if.end28

if.then27:                                        ; preds = %sw.bb20
  tail call void (i32, ptr, ...) @err(i32 noundef 2, ptr noundef nonnull @.str.2, ptr noundef nonnull %tty.0) #8
  unreachable

if.end28:                                         ; preds = %sw.bb20
  tail call void @exit(i32 noundef 0) #8
  unreachable

sw.bb29:                                          ; preds = %if.end18
  %st_mode30 = getelementptr inbounds %struct.stat, ptr %sb, i64 0, i32 3
  %7 = load i16, ptr %st_mode30, align 8, !tbaa !11
  %8 = and i16 %7, -17
  %call34 = tail call i32 @chmod(ptr noundef nonnull %tty.0, i16 noundef zeroext %8)
  %cmp35 = icmp slt i32 %call34, 0
  br i1 %cmp35, label %if.then37, label %if.end38

if.then37:                                        ; preds = %sw.bb29
  tail call void (i32, ptr, ...) @err(i32 noundef 2, ptr noundef nonnull @.str.2, ptr noundef nonnull %tty.0) #8
  unreachable

if.end38:                                         ; preds = %sw.bb29
  tail call void @exit(i32 noundef 1) #8
  unreachable

sw.epilog:                                        ; preds = %if.end18
  tail call fastcc void @ref_usage() #7
  unreachable
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

declare i32 @getopt(i32 noundef, ptr noundef, ptr noundef) local_unnamed_addr #2

; Function Attrs: noreturn nounwind uwtable
define internal fastcc void @ref_usage() unnamed_addr #0 {
entry:
  %0 = load ptr, ptr @__stderrp, align 8, !tbaa !9
  %1 = tail call i64 @fwrite(ptr nonnull @.str.5, i64 20, i64 1, ptr %0)
  tail call void @exit(i32 noundef 2) #8
  unreachable
}

declare ptr @ttyname(i32 noundef) local_unnamed_addr #2

; Function Attrs: noreturn
declare void @err(i32 noundef, ptr noundef, ...) local_unnamed_addr #3

; Function Attrs: nofree nounwind
declare noundef i32 @stat(ptr nocapture noundef readonly, ptr nocapture noundef) local_unnamed_addr #4

; Function Attrs: nofree nounwind
declare noundef i32 @puts(ptr nocapture noundef readonly) local_unnamed_addr #4

; Function Attrs: noreturn
declare void @exit(i32 noundef) local_unnamed_addr #3

; Function Attrs: nofree nounwind
declare noundef i32 @chmod(ptr nocapture noundef readonly, i16 noundef zeroext) local_unnamed_addr #4

; Function Attrs: nofree nounwind
declare noundef i64 @fwrite(ptr nocapture noundef, i64 noundef, i64 noundef, ptr nocapture noundef) local_unnamed_addr #5

attributes #0 = { noreturn nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { noreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nofree nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { nofree nounwind }
attributes #6 = { nounwind }
attributes #7 = { noreturn }
attributes #8 = { noreturn nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{!10, !10, i64 0}
!10 = !{!"any pointer", !7, i64 0}
!11 = !{!12, !14, i64 24}
!12 = !{!"stat", !13, i64 0, !13, i64 8, !13, i64 16, !14, i64 24, !14, i64 26, !6, i64 28, !6, i64 32, !6, i64 36, !13, i64 40, !15, i64 48, !15, i64 64, !15, i64 80, !15, i64 96, !13, i64 112, !13, i64 120, !6, i64 128, !6, i64 132, !13, i64 136, !13, i64 144, !7, i64 152}
!13 = !{!"long", !7, i64 0}
!14 = !{!"short", !7, i64 0}
!15 = !{!"timespec", !13, i64 0, !13, i64 8}
!16 = !{!7, !7, i64 0}
