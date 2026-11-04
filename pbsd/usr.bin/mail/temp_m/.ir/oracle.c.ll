; ModuleID = '/home/odin/pbsd/pbsd/usr.bin/mail/temp_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/usr.bin/mail/temp_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

module asm ".symver __qsort_r_compat, qsort_r@FBSD_1.0"

@.str = private unnamed_addr constant [7 x i8] c"TMPDIR\00", align 1
@tmpdir = dso_local local_unnamed_addr global ptr null, align 8
@.str.1 = private unnamed_addr constant [6 x i8] c"/tmp/\00", align 1
@.str.2 = private unnamed_addr constant [14 x i8] c"Out of memory\00", align 1
@myname = external local_unnamed_addr global ptr, align 8
@.str.4 = private unnamed_addr constant [7 x i8] c"ubluit\00", align 1
@rcvmode = external local_unnamed_addr global i32, align 4
@.str.5 = private unnamed_addr constant [14 x i8] c"Who are you!?\00", align 1
@.str.6 = private unnamed_addr constant [5 x i8] c"HOME\00", align 1
@homedir = external local_unnamed_addr global ptr, align 8
@debug = external local_unnamed_addr global i32, align 4
@.str.7 = private unnamed_addr constant [25 x i8] c"user = %s, homedir = %s\0A\00", align 1
@.str.8 = private unnamed_addr constant [5 x i8] c"NONE\00", align 1

; Function Attrs: nounwind uwtable
define dso_local void @ref_tinit() local_unnamed_addr #0 {
entry:
  %call = tail call ptr @getenv(ptr noundef nonnull @.str)
  store ptr %call, ptr @tmpdir, align 8, !tbaa !5
  %cmp = icmp eq ptr %call, null
  br i1 %cmp, label %if.then, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %entry
  %0 = load i8, ptr %call, align 1, !tbaa !9
  %cmp1 = icmp eq i8 %0, 0
  br i1 %cmp1, label %if.then, label %if.end

if.then:                                          ; preds = %lor.lhs.false, %entry
  store ptr @.str.1, ptr @tmpdir, align 8, !tbaa !5
  br label %if.end

if.end:                                           ; preds = %if.then, %lor.lhs.false
  %1 = phi ptr [ @.str.1, %if.then ], [ %call, %lor.lhs.false ]
  %call3 = tail call noalias ptr @strdup(ptr noundef nonnull %1)
  store ptr %call3, ptr @tmpdir, align 8, !tbaa !5
  %cmp4 = icmp eq ptr %call3, null
  br i1 %cmp4, label %if.then6, label %if.end7

if.then6:                                         ; preds = %if.end
  tail call void (i32, ptr, ...) @errx(i32 noundef 1, ptr noundef nonnull @.str.2) #7
  unreachable

if.end7:                                          ; preds = %if.end
  %call8 = tail call i64 @strlen(ptr noundef nonnull dereferenceable(1) %call3) #8
  %add.ptr = getelementptr inbounds i8, ptr %call3, i64 %call8
  %cp.060 = getelementptr inbounds i8, ptr %add.ptr, i64 -1
  %cmp1061 = icmp ugt ptr %cp.060, %call3
  br i1 %cmp1061, label %land.rhs, label %while.end

land.rhs:                                         ; preds = %if.end7, %while.body
  %cp.062 = phi ptr [ %cp.0, %while.body ], [ %cp.060, %if.end7 ]
  %2 = load i8, ptr %cp.062, align 1, !tbaa !9
  %cmp13 = icmp eq i8 %2, 47
  br i1 %cmp13, label %while.body, label %while.end

while.body:                                       ; preds = %land.rhs
  store i8 0, ptr %cp.062, align 1, !tbaa !9
  %cp.0 = getelementptr inbounds i8, ptr %cp.062, i64 -1
  %cmp10 = icmp ugt ptr %cp.0, %call3
  br i1 %cmp10, label %land.rhs, label %while.end, !llvm.loop !10

while.end:                                        ; preds = %land.rhs, %while.body, %if.end7
  %3 = load ptr, ptr @myname, align 8, !tbaa !5
  %cmp15.not = icmp eq ptr %3, null
  br i1 %cmp15.not, label %if.else, label %if.then17

if.then17:                                        ; preds = %while.end
  %call18 = tail call i32 @getuserid(ptr noundef nonnull %3) #9
  br label %if.end32

if.else:                                          ; preds = %while.end
  %call23 = tail call ptr @username() #9
  %cmp24 = icmp eq ptr %call23, null
  br i1 %cmp24, label %if.then26, label %if.else29

if.then26:                                        ; preds = %if.else
  store ptr @.str.4, ptr @myname, align 8, !tbaa !5
  %4 = load i32, ptr @rcvmode, align 4, !tbaa !12
  %tobool.not = icmp eq i32 %4, 0
  br i1 %tobool.not, label %if.end32, label %if.then27

if.then27:                                        ; preds = %if.then26
  tail call void (i32, ptr, ...) @errx(i32 noundef 1, ptr noundef nonnull @.str.5) #7
  unreachable

if.else29:                                        ; preds = %if.else
  %call30 = tail call ptr @savestr(ptr noundef nonnull %call23) #9
  store ptr %call30, ptr @myname, align 8, !tbaa !5
  br label %if.end32

if.end32:                                         ; preds = %if.then17, %if.else29, %if.then26
  %call33 = tail call ptr @getenv(ptr noundef nonnull @.str.6)
  %cmp34 = icmp eq ptr %call33, null
  br i1 %cmp34, label %if.end47, label %lor.lhs.false36

lor.lhs.false36:                                  ; preds = %if.end32
  %5 = load i8, ptr %call33, align 1, !tbaa !9
  %cmp38 = icmp eq i8 %5, 0
  br i1 %cmp38, label %if.end47, label %lor.lhs.false40

lor.lhs.false40:                                  ; preds = %lor.lhs.false36
  %call41 = tail call i64 @strlen(ptr noundef nonnull dereferenceable(1) %call33) #8
  %cmp42 = icmp ugt i64 %call41, 1023
  br i1 %cmp42, label %if.end47, label %if.else45

if.else45:                                        ; preds = %lor.lhs.false40
  %call46 = tail call ptr @savestr(ptr noundef nonnull %call33) #9
  br label %if.end47

if.end47:                                         ; preds = %if.end32, %lor.lhs.false36, %lor.lhs.false40, %if.else45
  %storemerge = phi ptr [ %call46, %if.else45 ], [ null, %lor.lhs.false40 ], [ null, %lor.lhs.false36 ], [ null, %if.end32 ]
  store ptr %storemerge, ptr @homedir, align 8, !tbaa !5
  %6 = load i32, ptr @debug, align 4, !tbaa !12
  %tobool48.not = icmp eq i32 %6, 0
  br i1 %tobool48.not, label %if.end52, label %if.then49

if.then49:                                        ; preds = %if.end47
  %7 = load ptr, ptr @myname, align 8, !tbaa !5
  %tobool50.not = icmp eq ptr %storemerge, null
  %cond = select i1 %tobool50.not, ptr @.str.8, ptr %storemerge
  %call51 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.7, ptr noundef %7, ptr noundef nonnull %cond)
  br label %if.end52

if.end52:                                         ; preds = %if.then49, %if.end47
  ret void
}

; Function Attrs: nofree nounwind memory(read)
declare noundef ptr @getenv(ptr nocapture noundef) local_unnamed_addr #1

; Function Attrs: mustprogress nofree nounwind willreturn memory(argmem: readwrite, inaccessiblemem: readwrite)
declare noalias ptr @strdup(ptr nocapture noundef readonly) local_unnamed_addr #2

; Function Attrs: noreturn
declare void @errx(i32 noundef, ptr noundef, ...) local_unnamed_addr #3

; Function Attrs: mustprogress nofree nounwind willreturn memory(argmem: read)
declare i64 @strlen(ptr nocapture noundef) local_unnamed_addr #4

declare i32 @getuserid(ptr noundef) local_unnamed_addr #5

declare ptr @username() local_unnamed_addr #5

declare ptr @savestr(ptr noundef) local_unnamed_addr #5

; Function Attrs: nofree nounwind
declare noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #6

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nofree nounwind memory(read) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { mustprogress nofree nounwind willreturn memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { noreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { mustprogress nofree nounwind willreturn memory(argmem: read) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { nofree nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #7 = { noreturn nounwind }
attributes #8 = { nounwind willreturn memory(read) }
attributes #9 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!5 = !{!6, !6, i64 0}
!6 = !{!"any pointer", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{!7, !7, i64 0}
!10 = distinct !{!10, !11}
!11 = !{!"llvm.loop.mustprogress"}
!12 = !{!13, !13, i64 0}
!13 = !{!"int", !7, i64 0}
