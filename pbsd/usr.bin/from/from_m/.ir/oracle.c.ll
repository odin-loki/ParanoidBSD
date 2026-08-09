; ModuleID = '/home/odin/pbsd/pbsd/usr.bin/from/from_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/usr.bin/from/from_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

module asm ".symver __qsort_r_compat, qsort_r@FBSD_1.0"

%struct._RuneLocale = type { [8 x i8], [32 x i8], ptr, ptr, i32, [256 x i64], [256 x i32], [256 x i32], %struct._RuneRange, %struct._RuneRange, %struct._RuneRange, ptr, i32 }
%struct._RuneRange = type { i32, ptr }

@.str = private unnamed_addr constant [6 x i8] c"cf:s:\00", align 1
@optarg = external local_unnamed_addr global ptr, align 8
@optind = external local_unnamed_addr global i32, align 4
@.str.1 = private unnamed_addr constant [6 x i8] c"%s/%s\00", align 1
@.str.2 = private unnamed_addr constant [10 x i8] c"/var/mail\00", align 1
@.str.3 = private unnamed_addr constant [5 x i8] c"MAIL\00", align 1
@.str.4 = private unnamed_addr constant [31 x i8] c"no password file entry for you\00", align 1
@.str.5 = private unnamed_addr constant [2 x i8] c"-\00", align 1
@__stdinp = external local_unnamed_addr global ptr, align 8
@.str.6 = private unnamed_addr constant [2 x i8] c"r\00", align 1
@.str.7 = private unnamed_addr constant [14 x i8] c"can't read %s\00", align 1
@.str.8 = private unnamed_addr constant [6 x i8] c"From \00", align 1
@.str.9 = private unnamed_addr constant [3 x i8] c"%s\00", align 1
@.str.10 = private unnamed_addr constant [49 x i8] c"There %s %d message%s in your incoming mailbox.\0A\00", align 1
@.str.11 = private unnamed_addr constant [3 x i8] c"is\00", align 1
@.str.12 = private unnamed_addr constant [4 x i8] c"are\00", align 1
@.str.13 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@.str.14 = private unnamed_addr constant [2 x i8] c"s\00", align 1
@__mb_sb_limit = external local_unnamed_addr global i32, align 4
@_ThreadRuneLocale = external thread_local global ptr, align 8
@_CurrentRuneLocale = external local_unnamed_addr global ptr, align 8
@__stderrp = external local_unnamed_addr global ptr, align 8
@.str.15 = private unnamed_addr constant [47 x i8] c"usage: from [-c] [-f file] [-s sender] [user]\0A\00", align 1

; Function Attrs: noreturn nounwind uwtable
define dso_local noundef i32 @ref_main(i32 noundef %argc, ptr noundef %argv) local_unnamed_addr #0 {
entry:
  %buf = alloca [1024 x i8], align 16
  call void @llvm.lifetime.start.p0(i64 1024, ptr nonnull %buf) #10
  %0 = tail call align 8 ptr @llvm.threadlocal.address.p0(ptr align 8 @_ThreadRuneLocale)
  br label %while.cond

while.cond:                                       ; preds = %sw.epilog, %entry
  %count.0 = phi i32 [ -1, %entry ], [ %count.1, %sw.epilog ]
  %file.0 = phi ptr [ null, %entry ], [ %file.1, %sw.epilog ]
  %sender.0 = phi ptr [ null, %entry ], [ %sender.1, %sw.epilog ]
  %call = tail call i32 @getopt(i32 noundef %argc, ptr noundef %argv, ptr noundef nonnull @.str) #10
  switch i32 %call, label %sw.default [
    i32 -1, label %while.end
    i32 99, label %sw.epilog
    i32 102, label %sw.bb1
    i32 115, label %sw.bb2
  ]

sw.bb1:                                           ; preds = %while.cond
  %1 = load ptr, ptr @optarg, align 8, !tbaa !5
  br label %sw.epilog

sw.bb2:                                           ; preds = %while.cond
  %2 = load ptr, ptr @optarg, align 8, !tbaa !5
  %3 = load i8, ptr %2, align 1, !tbaa !9
  %tobool.not103 = icmp eq i8 %3, 0
  br i1 %tobool.not103, label %sw.epilog, label %for.body

for.body:                                         ; preds = %sw.bb2, %__sbtolower.exit
  %4 = phi i8 [ %9, %__sbtolower.exit ], [ %3, %sw.bb2 ]
  %p.0104 = phi ptr [ %incdec.ptr, %__sbtolower.exit ], [ %2, %sw.bb2 ]
  %conv = sext i8 %4 to i32
  %cmp.i = icmp sgt i8 %4, -1
  %5 = load i32, ptr @__mb_sb_limit, align 4
  %cmp1.not.i = icmp sgt i32 %5, %conv
  %or.cond.i = select i1 %cmp.i, i1 %cmp1.not.i, i1 false
  br i1 %or.cond.i, label %cond.false.i, label %__sbtolower.exit

cond.false.i:                                     ; preds = %for.body
  %6 = load ptr, ptr %0, align 8, !tbaa !5
  %tobool.not.i.i = icmp eq ptr %6, null
  %7 = load ptr, ptr @_CurrentRuneLocale, align 8
  %retval.0.i.i = select i1 %tobool.not.i.i, ptr %7, ptr %6
  %idxprom.i = zext nneg i32 %conv to i64
  %arrayidx.i = getelementptr inbounds %struct._RuneLocale, ptr %retval.0.i.i, i64 0, i32 6, i64 %idxprom.i
  %8 = load i32, ptr %arrayidx.i, align 4, !tbaa !10
  br label %__sbtolower.exit

__sbtolower.exit:                                 ; preds = %for.body, %cond.false.i
  %cond.i = phi i32 [ %8, %cond.false.i ], [ %conv, %for.body ]
  %conv4 = trunc i32 %cond.i to i8
  store i8 %conv4, ptr %p.0104, align 1, !tbaa !9
  %incdec.ptr = getelementptr inbounds i8, ptr %p.0104, i64 1
  %9 = load i8, ptr %incdec.ptr, align 1, !tbaa !9
  %tobool.not = icmp eq i8 %9, 0
  br i1 %tobool.not, label %sw.epilog, label %for.body, !llvm.loop !12

sw.default:                                       ; preds = %while.cond
  tail call fastcc void @ref_usage() #11
  unreachable

sw.epilog:                                        ; preds = %__sbtolower.exit, %sw.bb2, %while.cond, %sw.bb1
  %count.1 = phi i32 [ %count.0, %sw.bb1 ], [ 0, %while.cond ], [ %count.0, %sw.bb2 ], [ %count.0, %__sbtolower.exit ]
  %file.1 = phi ptr [ %1, %sw.bb1 ], [ %file.0, %while.cond ], [ %file.0, %sw.bb2 ], [ %file.0, %__sbtolower.exit ]
  %sender.1 = phi ptr [ %sender.0, %sw.bb1 ], [ %sender.0, %while.cond ], [ %2, %sw.bb2 ], [ %2, %__sbtolower.exit ]
  br label %while.cond, !llvm.loop !14

while.end:                                        ; preds = %while.cond
  %cmp6 = icmp eq ptr %file.0, null
  br i1 %cmp6, label %if.then, label %if.end24

if.then:                                          ; preds = %while.end
  %10 = load i32, ptr @optind, align 4, !tbaa !10
  %idx.ext = sext i32 %10 to i64
  %add.ptr = getelementptr inbounds ptr, ptr %argv, i64 %idx.ext
  %tobool8.not = icmp eq i32 %10, %argc
  br i1 %tobool8.not, label %if.else, label %if.end24.sink.split

if.else:                                          ; preds = %if.then
  %call12 = tail call ptr @getenv(ptr noundef nonnull @.str.3)
  %tobool13.not = icmp eq ptr %call12, null
  br i1 %tobool13.not, label %if.then14, label %if.end24

if.then14:                                        ; preds = %if.else
  %call15 = tail call i32 @getuid() #10
  %call16 = tail call ptr @getpwuid(i32 noundef %call15) #10
  %tobool17.not = icmp eq ptr %call16, null
  br i1 %tobool17.not, label %if.then18, label %if.end24.sink.split

if.then18:                                        ; preds = %if.then14
  tail call void (i32, ptr, ...) @errx(i32 noundef 1, ptr noundef nonnull @.str.4) #12
  unreachable

if.end24.sink.split:                              ; preds = %if.then14, %if.then
  %add.ptr.sink = phi ptr [ %add.ptr, %if.then ], [ %call16, %if.then14 ]
  %11 = load ptr, ptr %add.ptr.sink, align 8, !tbaa !5
  %call10 = call i32 (ptr, i64, ptr, ...) @snprintf(ptr noundef nonnull dereferenceable(1) %buf, i64 noundef 1024, ptr noundef nonnull @.str.1, ptr noundef nonnull @.str.2, ptr noundef %11)
  br label %if.end24

if.end24:                                         ; preds = %if.end24.sink.split, %if.else, %while.end
  %file.2 = phi ptr [ %call12, %if.else ], [ %file.0, %while.end ], [ %buf, %if.end24.sink.split ]
  %call25 = call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %file.2, ptr noundef nonnull dereferenceable(2) @.str.5) #13
  %cmp26 = icmp eq i32 %call25, 0
  br i1 %cmp26, label %if.then28, label %if.else29

if.then28:                                        ; preds = %if.end24
  %12 = load ptr, ptr @__stdinp, align 8, !tbaa !5
  br label %if.end35

if.else29:                                        ; preds = %if.end24
  %call30 = call ptr @fopen(ptr noundef nonnull %file.2, ptr noundef nonnull @.str.6)
  %cmp31 = icmp eq ptr %call30, null
  br i1 %cmp31, label %if.then33, label %if.end35

if.then33:                                        ; preds = %if.else29
  call void (i32, ptr, ...) @errx(i32 noundef 1, ptr noundef nonnull @.str.7, ptr noundef nonnull %file.2) #12
  unreachable

if.end35:                                         ; preds = %if.else29, %if.then28
  %mbox.0 = phi ptr [ %12, %if.then28 ], [ %call30, %if.else29 ]
  %tobool52.not = icmp eq ptr %sender.0, null
  %add.ptr54 = getelementptr inbounds i8, ptr %buf, i64 5
  %call38.peel125 = call ptr @fgets(ptr noundef nonnull %buf, i32 noundef 1024, ptr noundef %mbox.0)
  %tobool39.not.peel126 = icmp eq ptr %call38.peel125, null
  br i1 %tobool39.not.peel126, label %for.end66, label %for.body40.peel

for.body40.peel:                                  ; preds = %if.end35, %if.end65
  %tobool47.not.ph128 = phi i1 [ true, %if.end65 ], [ false, %if.end35 ]
  %count.2.ph127 = phi i32 [ %count.3, %if.end65 ], [ %count.0, %if.end35 ]
  %13 = load i8, ptr %buf, align 16, !tbaa !9
  %cmp43.peel = icmp eq i8 %13, 10
  br i1 %cmp43.peel, label %for.cond36, label %if.end46

for.cond36:                                       ; preds = %for.body40.peel, %for.body40
  %call38 = call ptr @fgets(ptr noundef nonnull %buf, i32 noundef 1024, ptr noundef %mbox.0)
  %tobool39.not = icmp eq ptr %call38, null
  br i1 %tobool39.not, label %for.end66, label %for.body40

for.body40:                                       ; preds = %for.cond36
  %14 = load i8, ptr %buf, align 16, !tbaa !9
  %cmp43 = icmp eq i8 %14, 10
  br i1 %cmp43, label %for.cond36, label %land.lhs.true, !llvm.loop !15

if.end46:                                         ; preds = %for.body40.peel
  br i1 %tobool47.not.ph128, label %if.end65, label %land.lhs.true

land.lhs.true:                                    ; preds = %for.body40, %if.end46
  %bcmp = call i32 @bcmp(ptr noundef nonnull dereferenceable(5) %buf, ptr noundef nonnull dereferenceable(5) @.str.8, i64 5)
  %tobool50.not = icmp eq i32 %bcmp, 0
  br i1 %tobool50.not, label %land.lhs.true51, label %if.end65

land.lhs.true51:                                  ; preds = %land.lhs.true
  br i1 %tobool52.not, label %if.then57, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %land.lhs.true51
  %15 = load i8, ptr %sender.0, align 1, !tbaa !9
  %16 = load i32, ptr @__mb_sb_limit, align 4
  %17 = load ptr, ptr @_CurrentRuneLocale, align 8
  %conv6.i = sext i8 %15 to i32
  br label %for.cond.i

for.cond.i:                                       ; preds = %for.cond.i.backedge, %lor.lhs.false
  %line.addr.0.i = phi ptr [ %add.ptr54, %lor.lhs.false ], [ %incdec.ptr149.i, %for.cond.i.backedge ]
  %18 = load i8, ptr %line.addr.0.i, align 1, !tbaa !9
  %conv.i = sext i8 %18 to i32
  %cmp.i.i.i = icmp sgt i8 %18, -1
  %cmp1.not.i.i.i = icmp sgt i32 %16, %conv.i
  %or.cond.i.i.i = select i1 %cmp.i.i.i, i1 %cmp1.not.i.i.i, i1 false
  br i1 %or.cond.i.i.i, label %__sbistype.exit.i, label %__sbtolower.exit.i

__sbistype.exit.i:                                ; preds = %for.cond.i
  %19 = load ptr, ptr %0, align 8, !tbaa !5
  %tobool.not.i.i.i.i = icmp eq ptr %19, null
  %retval.0.i.i.i.i = select i1 %tobool.not.i.i.i.i, ptr %17, ptr %19
  %idxprom.i.i.i = zext nneg i32 %conv.i to i64
  %arrayidx.i.i.i = getelementptr inbounds %struct._RuneLocale, ptr %retval.0.i.i.i.i, i64 0, i32 5, i64 %idxprom.i.i.i
  %20 = load i64, ptr %arrayidx.i.i.i, align 8, !tbaa !17
  %21 = and i64 %20, 16384
  %tobool.not.i = icmp eq i64 %21, 0
  br i1 %tobool.not.i, label %cond.false.i.i, label %if.end65

cond.false.i.i:                                   ; preds = %__sbistype.exit.i
  %arrayidx.i.i = getelementptr inbounds %struct._RuneLocale, ptr %retval.0.i.i.i.i, i64 0, i32 6, i64 %idxprom.i.i.i
  %22 = load i32, ptr %arrayidx.i.i, align 4, !tbaa !10
  br label %__sbtolower.exit.i

__sbtolower.exit.i:                               ; preds = %cond.false.i.i, %for.cond.i
  %cond.i.i = phi i32 [ %22, %cond.false.i.i ], [ %conv.i, %for.cond.i ]
  %incdec.ptr149.i = getelementptr inbounds i8, ptr %line.addr.0.i, i64 1
  %sext.i = shl i32 %cond.i.i, 24
  %conv5.i = ashr exact i32 %sext.i, 24
  %cmp.not.i = icmp eq i32 %conv5.i, %conv6.i
  br i1 %cmp.not.i, label %for.cond10.i, label %for.cond.i.backedge

for.cond.i.backedge:                              ; preds = %__sbtolower.exit45.i, %__sbtolower.exit.i
  br label %for.cond.i

for.cond10.i:                                     ; preds = %__sbtolower.exit.i, %__sbtolower.exit45.i
  %sender.pn.i = phi ptr [ %p.0.i, %__sbtolower.exit45.i ], [ %sender.0, %__sbtolower.exit.i ]
  %t.0.i = phi ptr [ %incdec.ptr18.i, %__sbtolower.exit45.i ], [ %incdec.ptr149.i, %__sbtolower.exit.i ]
  %p.0.i = getelementptr inbounds i8, ptr %sender.pn.i, i64 1
  %23 = load i8, ptr %p.0.i, align 1, !tbaa !9
  %tobool12.not.i = icmp eq i8 %23, 0
  br i1 %tobool12.not.i, label %if.then57, label %if.end14.i

if.end14.i:                                       ; preds = %for.cond10.i
  %24 = load i8, ptr %t.0.i, align 1, !tbaa !9
  %conv15.i = sext i8 %24 to i32
  %cmp.i36.i = icmp sgt i8 %24, -1
  %cmp1.not.i37.i = icmp sgt i32 %16, %conv15.i
  %or.cond.i38.i = select i1 %cmp.i36.i, i1 %cmp1.not.i37.i, i1 false
  br i1 %or.cond.i38.i, label %cond.false.i40.i, label %__sbtolower.exit45.i

cond.false.i40.i:                                 ; preds = %if.end14.i
  %25 = load ptr, ptr %0, align 8, !tbaa !5
  %tobool.not.i.i41.i = icmp eq ptr %25, null
  %retval.0.i.i42.i = select i1 %tobool.not.i.i41.i, ptr %17, ptr %25
  %idxprom.i43.i = zext nneg i32 %conv15.i to i64
  %arrayidx.i44.i = getelementptr inbounds %struct._RuneLocale, ptr %retval.0.i.i42.i, i64 0, i32 6, i64 %idxprom.i43.i
  %26 = load i32, ptr %arrayidx.i44.i, align 4, !tbaa !10
  br label %__sbtolower.exit45.i

__sbtolower.exit45.i:                             ; preds = %cond.false.i40.i, %if.end14.i
  %cond.i39.i = phi i32 [ %26, %cond.false.i40.i ], [ %conv15.i, %if.end14.i ]
  %incdec.ptr18.i = getelementptr inbounds i8, ptr %t.0.i, i64 1
  %sext35.i = shl i32 %cond.i39.i, 24
  %conv19.i = ashr exact i32 %sext35.i, 24
  %conv20.i = sext i8 %23 to i32
  %cmp21.not.i = icmp eq i32 %conv19.i, %conv20.i
  br i1 %cmp21.not.i, label %for.cond10.i, label %for.cond.i.backedge

if.then57:                                        ; preds = %for.cond10.i, %land.lhs.true51
  %cmp58.not = icmp eq i32 %count.2.ph127, -1
  br i1 %cmp58.not, label %if.else61, label %if.then60

if.then60:                                        ; preds = %if.then57
  %inc = add nuw nsw i32 %count.2.ph127, 1
  br label %if.end65

if.else61:                                        ; preds = %if.then57
  %call63 = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.9, ptr noundef nonnull %buf)
  br label %if.end65

if.end65:                                         ; preds = %__sbistype.exit.i, %if.then60, %if.else61, %land.lhs.true, %if.end46
  %count.3 = phi i32 [ %count.2.ph127, %land.lhs.true ], [ %inc, %if.then60 ], [ -1, %if.else61 ], [ %count.2.ph127, %if.end46 ], [ %count.2.ph127, %__sbistype.exit.i ]
  %call38.peel = call ptr @fgets(ptr noundef nonnull %buf, i32 noundef 1024, ptr noundef %mbox.0)
  %tobool39.not.peel = icmp eq ptr %call38.peel, null
  br i1 %tobool39.not.peel, label %for.end66, label %for.body40.peel, !llvm.loop !19

for.end66:                                        ; preds = %if.end65, %for.cond36, %if.end35
  %count.2.ph121 = phi i32 [ %count.0, %if.end35 ], [ %count.2.ph127, %for.cond36 ], [ %count.3, %if.end65 ]
  %cmp67.not = icmp eq i32 %count.2.ph121, -1
  br i1 %cmp67.not, label %if.end76, label %if.then69

if.then69:                                        ; preds = %for.end66
  %cmp70 = icmp eq i32 %count.2.ph121, 1
  %cond = select i1 %cmp70, ptr @.str.11, ptr @.str.12
  %cond74 = select i1 %cmp70, ptr @.str.13, ptr @.str.14
  %call75 = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.10, ptr noundef nonnull %cond, i32 noundef %count.2.ph121, ptr noundef nonnull %cond74)
  br label %if.end76

if.end76:                                         ; preds = %if.then69, %for.end66
  %call77 = call i32 @fclose(ptr noundef %mbox.0)
  call void @exit(i32 noundef 0) #12
  unreachable
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

declare i32 @getopt(i32 noundef, ptr noundef, ptr noundef) local_unnamed_addr #2

; Function Attrs: noreturn nounwind uwtable
define internal fastcc void @ref_usage() unnamed_addr #0 {
entry:
  %0 = load ptr, ptr @__stderrp, align 8, !tbaa !5
  %1 = tail call i64 @fwrite(ptr nonnull @.str.15, i64 46, i64 1, ptr %0)
  tail call void @exit(i32 noundef 1) #12
  unreachable
}

; Function Attrs: nofree nounwind
declare noundef i32 @snprintf(ptr noalias nocapture noundef writeonly, i64 noundef, ptr nocapture noundef readonly, ...) local_unnamed_addr #3

; Function Attrs: nofree nounwind memory(read)
declare noundef ptr @getenv(ptr nocapture noundef) local_unnamed_addr #4

declare ptr @getpwuid(i32 noundef) local_unnamed_addr #2

declare i32 @getuid() local_unnamed_addr #2

; Function Attrs: noreturn
declare void @errx(i32 noundef, ptr noundef, ...) local_unnamed_addr #5

; Function Attrs: mustprogress nofree nounwind willreturn memory(argmem: read)
declare i32 @strcmp(ptr nocapture noundef, ptr nocapture noundef) local_unnamed_addr #6

; Function Attrs: nofree nounwind
declare noalias noundef ptr @fopen(ptr nocapture noundef readonly, ptr nocapture noundef readonly) local_unnamed_addr #3

; Function Attrs: nofree nounwind
declare noundef ptr @fgets(ptr noundef, i32 noundef, ptr nocapture noundef) local_unnamed_addr #3

; Function Attrs: nofree nounwind
declare noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #3

; Function Attrs: nofree nounwind
declare noundef i32 @fclose(ptr nocapture noundef) local_unnamed_addr #3

; Function Attrs: noreturn
declare void @exit(i32 noundef) local_unnamed_addr #5

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare nonnull ptr @llvm.threadlocal.address.p0(ptr nonnull) #7

; Function Attrs: nofree nounwind willreturn memory(argmem: read)
declare i32 @bcmp(ptr nocapture, ptr nocapture, i64) local_unnamed_addr #8

; Function Attrs: nofree nounwind
declare noundef i64 @fwrite(ptr nocapture noundef, i64 noundef, i64 noundef, ptr nocapture noundef) local_unnamed_addr #9

attributes #0 = { noreturn nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nofree nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nofree nounwind memory(read) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { noreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { mustprogress nofree nounwind willreturn memory(argmem: read) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #7 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #8 = { nofree nounwind willreturn memory(argmem: read) }
attributes #9 = { nofree nounwind }
attributes #10 = { nounwind }
attributes #11 = { noreturn }
attributes #12 = { noreturn nounwind }
attributes #13 = { nounwind willreturn memory(read) }

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
!10 = !{!11, !11, i64 0}
!11 = !{!"int", !7, i64 0}
!12 = distinct !{!12, !13}
!13 = !{!"llvm.loop.mustprogress"}
!14 = distinct !{!14, !13}
!15 = distinct !{!15, !13, !16}
!16 = !{!"llvm.loop.peeled.count", i32 1}
!17 = !{!18, !18, i64 0}
!18 = !{!"long", !7, i64 0}
!19 = distinct !{!19, !13}
