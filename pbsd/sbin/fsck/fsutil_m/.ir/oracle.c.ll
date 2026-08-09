; ModuleID = '/home/odin/pbsd/pbsd/sbin/fsck/fsutil_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/sbin/fsck/fsutil_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

module asm ".ident\09\22$NetBSD: fsutil.c,v 1.15 2006/06/05 16:52:05 christos Exp $\22"
module asm ".symver __qsort_r_compat, qsort_r@FBSD_1.0"

%struct.fstab = type { ptr, ptr, ptr, ptr, ptr, i32, i32 }
%struct.__va_list_tag = type { i32, i32, ptr, ptr }
%struct.stat = type { i64, i64, i64, i16, i16, i32, i32, i32, i64, %struct.timespec, %struct.timespec, %struct.timespec, %struct.timespec, i64, i64, i32, i32, i64, i64, [9 x i64] }
%struct.timespec = type { i64, i64 }

@.str = private unnamed_addr constant [2 x i8] c",\00", align 1
@dev = internal unnamed_addr global ptr null, align 8
@preen = internal unnamed_addr global i32 0, align 4
@.str.1 = private unnamed_addr constant [2 x i8] c"/\00", align 1
@.str.2 = private unnamed_addr constant [15 x i8] c"Can't stat `/'\00", align 1
@.str.3 = private unnamed_addr constant [15 x i8] c"Can't stat %s\0A\00", align 1
@.str.4 = private unnamed_addr constant [25 x i8] c"%s is not a char device\0A\00", align 1
@.str.5 = private unnamed_addr constant [14 x i8] c"malloc failed\00", align 1
@.str.6 = private unnamed_addr constant [15 x i8] c"realloc failed\00", align 1
@.str.7 = private unnamed_addr constant [14 x i8] c"strdup failed\00", align 1
@.str.8 = private unnamed_addr constant [5 x i8] c"%s: \00", align 1
@.str.10 = private unnamed_addr constant [48 x i8] c"%s: UNEXPECTED INCONSISTENCY; RUN %s MANUALLY.\0A\00", align 1

; Function Attrs: nounwind uwtable
define dso_local i32 @ref_getfsopt(ptr nocapture noundef readonly %fs, ptr nocapture noundef readonly %option) local_unnamed_addr #0 {
entry:
  %0 = load i8, ptr %option, align 1, !tbaa !5
  %cmp = icmp eq i8 %0, 110
  br i1 %cmp, label %land.lhs.true, label %if.end

land.lhs.true:                                    ; preds = %entry
  %arrayidx2 = getelementptr inbounds i8, ptr %option, i64 1
  %1 = load i8, ptr %arrayidx2, align 1, !tbaa !5
  %cmp4 = icmp eq i8 %1, 111
  br i1 %cmp4, label %if.then, label %if.end

if.then:                                          ; preds = %land.lhs.true
  %add.ptr = getelementptr inbounds i8, ptr %option, i64 2
  br label %if.end

if.end:                                           ; preds = %entry, %land.lhs.true, %if.then
  %tobool27.not = phi i32 [ 0, %if.then ], [ 1, %land.lhs.true ], [ 1, %entry ]
  %negative.0 = phi i32 [ 1, %if.then ], [ 0, %land.lhs.true ], [ 0, %entry ]
  %option.addr.0 = phi ptr [ %add.ptr, %if.then ], [ %option, %land.lhs.true ], [ %option, %entry ]
  %fs_mntops = getelementptr inbounds %struct.fstab, ptr %fs, i64 0, i32 3
  %2 = load ptr, ptr %fs_mntops, align 8, !tbaa !8
  %call = tail call noalias ptr @strdup(ptr noundef %2)
  %call641 = tail call ptr @strtok(ptr noundef %call, ptr noundef nonnull @.str)
  %cmp7.not42 = icmp eq ptr %call641, null
  br i1 %cmp7.not42, label %for.end, label %for.body

for.body:                                         ; preds = %if.end, %for.inc
  %call644 = phi ptr [ %call6, %for.inc ], [ %call641, %if.end ]
  %found.043 = phi i32 [ %spec.select40, %for.inc ], [ 0, %if.end ]
  %3 = load i8, ptr %call644, align 1, !tbaa !5
  %cmp11 = icmp eq i8 %3, 110
  br i1 %cmp11, label %land.lhs.true13, label %for.inc

land.lhs.true13:                                  ; preds = %for.body
  %arrayidx14 = getelementptr inbounds i8, ptr %call644, i64 1
  %4 = load i8, ptr %arrayidx14, align 1, !tbaa !5
  %cmp16 = icmp eq i8 %4, 111
  %spec.select.idx = select i1 %cmp16, i64 2, i64 0
  %spec.select = getelementptr inbounds i8, ptr %call644, i64 %spec.select.idx
  %spec.select45 = select i1 %cmp16, i32 %negative.0, i32 %tobool27.not
  br label %for.inc

for.inc:                                          ; preds = %land.lhs.true13, %for.body
  %call644.sink = phi ptr [ %call644, %for.body ], [ %spec.select, %land.lhs.true13 ]
  %tobool27.not.sink = phi i32 [ %tobool27.not, %for.body ], [ %spec.select45, %land.lhs.true13 ]
  %call24 = tail call i32 @strcasecmp(ptr noundef nonnull %call644.sink, ptr noundef nonnull %option.addr.0) #17
  %tobool25.not = icmp eq i32 %call24, 0
  %spec.select40 = select i1 %tobool25.not, i32 %tobool27.not.sink, i32 %found.043
  %call6 = tail call ptr @strtok(ptr noundef null, ptr noundef nonnull @.str)
  %cmp7.not = icmp eq ptr %call6, null
  br i1 %cmp7.not, label %for.end, label %for.body, !llvm.loop !12

for.end:                                          ; preds = %for.inc, %if.end
  %found.0.lcssa = phi i32 [ 0, %if.end ], [ %spec.select40, %for.inc ]
  tail call void @free(ptr noundef %call)
  ret i32 %found.0.lcssa
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nofree nounwind willreturn memory(argmem: readwrite, inaccessiblemem: readwrite)
declare noalias ptr @strdup(ptr nocapture noundef readonly) local_unnamed_addr #2

; Function Attrs: mustprogress nofree nounwind willreturn
declare ptr @strtok(ptr noundef, ptr nocapture noundef readonly) local_unnamed_addr #3

; Function Attrs: mustprogress nofree nounwind willreturn memory(read)
declare i32 @strcasecmp(ptr nocapture noundef, ptr nocapture noundef) local_unnamed_addr #4

; Function Attrs: mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite)
declare void @free(ptr allocptr nocapture noundef) local_unnamed_addr #5

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(write, argmem: none, inaccessiblemem: none) uwtable
define dso_local void @ref_setcdevname(ptr noundef %cd, i32 noundef %pr) local_unnamed_addr #6 {
entry:
  store ptr %cd, ptr @dev, align 8, !tbaa !14
  store i32 %pr, ptr @preen, align 4, !tbaa !15
  ret void
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(read, argmem: none, inaccessiblemem: none) uwtable
define dso_local ptr @ref_cdevname() local_unnamed_addr #7 {
entry:
  %0 = load ptr, ptr @dev, align 8, !tbaa !14
  ret ptr %0
}

; Function Attrs: nounwind uwtable
define dso_local void @ref_pfatal(ptr nocapture noundef readonly %fmt, ...) local_unnamed_addr #0 {
entry:
  %ap = alloca [1 x %struct.__va_list_tag], align 16
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %ap) #18
  call void @llvm.va_start(ptr nonnull %ap)
  %call2.i = call i32 @vprintf(ptr noundef %fmt, ptr noundef nonnull %ap)
  %0 = load i32, ptr @preen, align 4
  %tobool5.i.not = icmp eq i32 %0, 0
  br i1 %tobool5.i.not, label %ref_vmsg.exit, label %if.end8.i

if.end8.i:                                        ; preds = %entry
  %putchar.i = call i32 @putchar(i32 10)
  %.pre.i = load i32, ptr @preen, align 4
  %tobool11.i.not = icmp eq i32 %.pre.i, 0
  br i1 %tobool11.i.not, label %ref_vmsg.exit, label %if.then12.i

if.then12.i:                                      ; preds = %if.end8.i
  %1 = load ptr, ptr @dev, align 8, !tbaa !14
  %call13.i = call ptr @getprogname() #18
  %call14.i = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.10, ptr noundef %1, ptr noundef %call13.i)
  call void @exit(i32 noundef 8) #19
  unreachable

ref_vmsg.exit:                                    ; preds = %entry, %if.end8.i
  call void @llvm.va_end(ptr nonnull %ap)
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %ap) #18
  ret void
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.va_start(ptr) #8

; Function Attrs: nounwind uwtable
define internal fastcc void @ref_vmsg(i32 noundef %fatal, ptr nocapture noundef readonly %fmt, ptr noundef %ap) unnamed_addr #0 {
entry:
  %tobool = icmp eq i32 %fatal, 0
  %0 = load i32, ptr @preen, align 4
  %tobool1 = icmp ne i32 %0, 0
  %or.cond = select i1 %tobool, i1 %tobool1, i1 false
  br i1 %or.cond, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %1 = load ptr, ptr @dev, align 8, !tbaa !14
  %call = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.8, ptr noundef %1)
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %call2 = tail call i32 @vprintf(ptr noundef %fmt, ptr noundef %ap)
  %tobool3 = icmp ne i32 %fatal, 0
  %2 = load i32, ptr @preen, align 4
  %tobool5 = icmp ne i32 %2, 0
  %or.cond16 = select i1 %tobool3, i1 %tobool5, i1 false
  br i1 %or.cond16, label %if.then6, label %if.end8

if.then6:                                         ; preds = %if.end
  %putchar = tail call i32 @putchar(i32 10)
  %.pre = load i32, ptr @preen, align 4
  br label %if.end8

if.end8:                                          ; preds = %if.then6, %if.end
  %3 = phi i32 [ %.pre, %if.then6 ], [ %2, %if.end ]
  %tobool11 = icmp ne i32 %3, 0
  %or.cond17 = select i1 %tobool3, i1 %tobool11, i1 false
  br i1 %or.cond17, label %if.then12, label %if.end15

if.then12:                                        ; preds = %if.end8
  %4 = load ptr, ptr @dev, align 8, !tbaa !14
  %call13 = tail call ptr @getprogname() #18
  %call14 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.10, ptr noundef %4, ptr noundef %call13)
  tail call void @exit(i32 noundef 8) #19
  unreachable

if.end15:                                         ; preds = %if.end8
  ret void
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.va_end(ptr) #8

; Function Attrs: nofree nounwind uwtable
define dso_local void @ref_pwarn(ptr nocapture noundef readonly %fmt, ...) local_unnamed_addr #9 {
entry:
  %ap = alloca [1 x %struct.__va_list_tag], align 16
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %ap) #18
  call void @llvm.va_start(ptr nonnull %ap)
  %0 = load i32, ptr @preen, align 4
  %tobool1.i.not = icmp eq i32 %0, 0
  br i1 %tobool1.i.not, label %ref_vmsg.exit, label %if.then.i

if.then.i:                                        ; preds = %entry
  %1 = load ptr, ptr @dev, align 8, !tbaa !14
  %call.i = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.8, ptr noundef %1)
  br label %ref_vmsg.exit

ref_vmsg.exit:                                    ; preds = %entry, %if.then.i
  %call2.i = call i32 @vprintf(ptr noundef %fmt, ptr noundef nonnull %ap)
  call void @llvm.va_end(ptr nonnull %ap)
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %ap) #18
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @ref_perr(ptr nocapture noundef readonly %fmt, ...) local_unnamed_addr #0 {
entry:
  %ap = alloca [1 x %struct.__va_list_tag], align 16
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %ap) #18
  call void @llvm.va_start(ptr nonnull %ap)
  %call2.i = call i32 @vprintf(ptr noundef %fmt, ptr noundef nonnull %ap)
  %0 = load i32, ptr @preen, align 4
  %tobool5.i.not = icmp eq i32 %0, 0
  br i1 %tobool5.i.not, label %ref_vmsg.exit, label %if.end8.i

if.end8.i:                                        ; preds = %entry
  %putchar.i = call i32 @putchar(i32 10)
  %.pre.i = load i32, ptr @preen, align 4
  %tobool11.i.not = icmp eq i32 %.pre.i, 0
  br i1 %tobool11.i.not, label %ref_vmsg.exit, label %if.then12.i

if.then12.i:                                      ; preds = %if.end8.i
  %1 = load ptr, ptr @dev, align 8, !tbaa !14
  %call13.i = call ptr @getprogname() #18
  %call14.i = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.10, ptr noundef %1, ptr noundef %call13.i)
  call void @exit(i32 noundef 8) #19
  unreachable

ref_vmsg.exit:                                    ; preds = %entry, %if.end8.i
  call void @llvm.va_end(ptr nonnull %ap)
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %ap) #18
  ret void
}

; Function Attrs: noreturn nounwind uwtable
define dso_local void @ref_panic(ptr nocapture noundef readonly %fmt, ...) local_unnamed_addr #10 {
entry:
  %ap = alloca [1 x %struct.__va_list_tag], align 16
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %ap) #18
  call void @llvm.va_start(ptr nonnull %ap)
  call fastcc void @ref_vmsg(i32 noundef 1, ptr noundef %fmt, ptr noundef nonnull %ap)
  call void @llvm.va_end(ptr nonnull %ap)
  call void @exit(i32 noundef 8) #19
  unreachable
}

; Function Attrs: noreturn
declare void @exit(i32 noundef) local_unnamed_addr #11

; Function Attrs: nounwind uwtable
define dso_local noundef ptr @ref_devcheck(ptr noundef returned %origname) local_unnamed_addr #0 {
entry:
  %stslash = alloca %struct.stat, align 8
  %stchar = alloca %struct.stat, align 8
  call void @llvm.lifetime.start.p0(i64 224, ptr nonnull %stslash) #18
  call void @llvm.lifetime.start.p0(i64 224, ptr nonnull %stchar) #18
  %call = call i32 @stat(ptr noundef nonnull @.str.1, ptr noundef nonnull %stslash)
  %cmp = icmp slt i32 %call, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  tail call void (ptr, ...) @ref_perr(ptr noundef nonnull @.str.2)
  br label %cleanup

if.end:                                           ; preds = %entry
  %call1 = call i32 @stat(ptr noundef %origname, ptr noundef nonnull %stchar)
  %cmp2 = icmp slt i32 %call1, 0
  br i1 %cmp2, label %if.then3, label %if.end4

if.then3:                                         ; preds = %if.end
  tail call void (ptr, ...) @ref_perr(ptr noundef nonnull @.str.3, ptr noundef %origname)
  br label %cleanup

if.end4:                                          ; preds = %if.end
  %st_mode = getelementptr inbounds %struct.stat, ptr %stchar, i64 0, i32 3
  %0 = load i16, ptr %st_mode, align 8, !tbaa !16
  %1 = and i16 %0, -4096
  %cmp5 = icmp eq i16 %1, 8192
  br i1 %cmp5, label %cleanup, label %if.then7

if.then7:                                         ; preds = %if.end4
  tail call void (ptr, ...) @ref_perr(ptr noundef nonnull @.str.4, ptr noundef %origname)
  br label %cleanup

cleanup:                                          ; preds = %if.end4, %if.then7, %if.then3, %if.then
  call void @llvm.lifetime.end.p0(i64 224, ptr nonnull %stchar) #18
  call void @llvm.lifetime.end.p0(i64 224, ptr nonnull %stslash) #18
  ret ptr %origname
}

; Function Attrs: nofree nounwind
declare noundef i32 @stat(ptr nocapture noundef readonly, ptr nocapture noundef) local_unnamed_addr #12

; Function Attrs: nounwind uwtable
define dso_local noalias noundef ptr @ref_emalloc(i64 noundef %s) local_unnamed_addr #0 {
entry:
  %call = tail call noalias ptr @malloc(i64 noundef %s) #20
  %cmp = icmp eq ptr %call, null
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  tail call void (i32, ptr, ...) @err(i32 noundef 1, ptr noundef nonnull @.str.5) #19
  unreachable

if.end:                                           ; preds = %entry
  ret ptr %call
}

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #13

; Function Attrs: noreturn
declare void @err(i32 noundef, ptr noundef, ...) local_unnamed_addr #11

; Function Attrs: nounwind uwtable
define dso_local noalias noundef ptr @ref_erealloc(ptr nocapture noundef %p, i64 noundef %s) local_unnamed_addr #0 {
entry:
  %call = tail call ptr @realloc(ptr noundef %p, i64 noundef %s) #21
  %cmp = icmp eq ptr %call, null
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  tail call void (i32, ptr, ...) @err(i32 noundef 1, ptr noundef nonnull @.str.6) #19
  unreachable

if.end:                                           ; preds = %entry
  ret ptr %call
}

; Function Attrs: mustprogress nounwind willreturn allockind("realloc") allocsize(1) memory(argmem: readwrite, inaccessiblemem: readwrite)
declare noalias noundef ptr @realloc(ptr allocptr nocapture noundef, i64 noundef) local_unnamed_addr #14

; Function Attrs: nounwind uwtable
define dso_local noalias ptr @ref_estrdup(ptr nocapture noundef readonly %s) local_unnamed_addr #0 {
entry:
  %call = tail call noalias ptr @strdup(ptr noundef %s)
  %cmp = icmp eq ptr %call, null
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  tail call void (i32, ptr, ...) @err(i32 noundef 1, ptr noundef nonnull @.str.7) #19
  unreachable

if.end:                                           ; preds = %entry
  ret ptr %call
}

; Function Attrs: nofree nounwind
declare noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #12

; Function Attrs: nofree nounwind
declare noundef i32 @vprintf(ptr nocapture noundef readonly, ptr noundef) local_unnamed_addr #12

declare ptr @getprogname() local_unnamed_addr #15

; Function Attrs: nofree nounwind
declare noundef i32 @putchar(i32 noundef) local_unnamed_addr #16

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { mustprogress nofree nounwind willreturn memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { mustprogress nofree nounwind willreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { mustprogress nofree nounwind willreturn memory(read) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { mustprogress nofree norecurse nosync nounwind willreturn memory(write, argmem: none, inaccessiblemem: none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #7 = { mustprogress nofree norecurse nosync nounwind willreturn memory(read, argmem: none, inaccessiblemem: none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #8 = { mustprogress nocallback nofree nosync nounwind willreturn }
attributes #9 = { nofree nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #10 = { noreturn nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #11 = { noreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #12 = { nofree nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #13 = { mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #14 = { mustprogress nounwind willreturn allockind("realloc") allocsize(1) memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #15 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #16 = { nofree nounwind }
attributes #17 = { nounwind willreturn memory(read) }
attributes #18 = { nounwind }
attributes #19 = { noreturn nounwind }
attributes #20 = { allocsize(0) }
attributes #21 = { allocsize(1) }

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
!8 = !{!9, !10, i64 24}
!9 = !{!"fstab", !10, i64 0, !10, i64 8, !10, i64 16, !10, i64 24, !10, i64 32, !11, i64 40, !11, i64 44}
!10 = !{!"any pointer", !6, i64 0}
!11 = !{!"int", !6, i64 0}
!12 = distinct !{!12, !13}
!13 = !{!"llvm.loop.mustprogress"}
!14 = !{!10, !10, i64 0}
!15 = !{!11, !11, i64 0}
!16 = !{!17, !19, i64 24}
!17 = !{!"stat", !18, i64 0, !18, i64 8, !18, i64 16, !19, i64 24, !19, i64 26, !11, i64 28, !11, i64 32, !11, i64 36, !18, i64 40, !20, i64 48, !20, i64 64, !20, i64 80, !20, i64 96, !18, i64 112, !18, i64 120, !11, i64 128, !11, i64 132, !18, i64 136, !18, i64 144, !6, i64 152}
!18 = !{!"long", !6, i64 0}
!19 = !{!"short", !6, i64 0}
!20 = !{!"timespec", !18, i64 0, !18, i64 8}
