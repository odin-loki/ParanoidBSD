; ModuleID = '/home/odin/pbsd/pbsd/lib/libc/db/btree/bt_delete_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/libc/db/btree/bt_delete_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.__db = type { i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr }
%struct._btree = type { ptr, ptr, %struct._epg, ptr, %struct._cursor, [50 x %struct._epgno], ptr, %struct.DBT, %struct.DBT, i32, i32, i32, i16, i32, i32, %struct._epgno, ptr, ptr, ptr, ptr, i32, ptr, ptr, ptr, i64, i32, i64, i8, i32 }
%struct._epg = type { ptr, i16 }
%struct._cursor = type { %struct._epgno, %struct.DBT, i32, i8 }
%struct.DBT = type { ptr, i64 }
%struct._epgno = type { i32, i16 }
%struct._page = type { i32, i32, i32, i32, i16, i16, [1 x i16] }
%struct._binternal = type { i32, i32, i8, [1 x i8] }
%struct._bleaf = type { i32, i32, i8, [1 x i8] }

; Function Attrs: nounwind uwtable
define dso_local noundef i32 @ref___bt_delete(ptr nocapture noundef readonly %dbp, ptr noundef %key, i32 noundef %flags) local_unnamed_addr #0 {
entry:
  %exact.i = alloca i32, align 4
  %h = alloca ptr, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %h) #4
  %internal = getelementptr inbounds %struct.__db, ptr %dbp, i64 0, i32 7
  %0 = load ptr, ptr %internal, align 8, !tbaa !5
  %bt_pinned = getelementptr inbounds %struct._btree, ptr %0, i64 0, i32 3
  %1 = load ptr, ptr %bt_pinned, align 8, !tbaa !11
  %cmp.not = icmp eq ptr %1, null
  br i1 %cmp.not, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %2 = load ptr, ptr %0, align 8, !tbaa !19
  %call = tail call i32 @mpool_put(ptr noundef %2, ptr noundef nonnull %1, i32 noundef 0) #4
  store ptr null, ptr %bt_pinned, align 8, !tbaa !11
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %flags3 = getelementptr inbounds %struct._btree, ptr %0, i64 0, i32 28
  %3 = load i32, ptr %flags3, align 4, !tbaa !20
  %and = and i32 %3, 16
  %tobool.not = icmp eq i32 %and, 0
  br i1 %tobool.not, label %if.end6, label %if.then4

if.then4:                                         ; preds = %if.end
  %call5 = tail call ptr @__error() #4
  store i32 1, ptr %call5, align 4, !tbaa !21
  br label %cleanup

if.end6:                                          ; preds = %if.end
  switch i32 %flags, label %sw.default [
    i32 0, label %sw.bb
    i32 1, label %sw.bb8
  ]

sw.bb:                                            ; preds = %if.end6
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %exact.i) #4
  %call153.i = call ptr @__bt_search(ptr noundef nonnull %0, ptr noundef %key, ptr noundef nonnull %exact.i) #4
  %cmp154.i = icmp eq ptr %call153.i, null
  br i1 %cmp154.i, label %ref___bt_bdelete.exit, label %if.end.i.preheader

if.end.i.preheader:                               ; preds = %sw.bb
  %4 = load i32, ptr %exact.i, align 4, !tbaa !21
  %tobool1.not.i110 = icmp eq i32 %4, 0
  br i1 %tobool1.not.i110, label %if.then2.i, label %if.end6.i

if.end.i:                                         ; preds = %loop.backedge.i
  %5 = load i32, ptr %exact.i, align 4, !tbaa !21
  %tobool1.not.i = icmp eq i32 %5, 0
  br i1 %tobool1.not.i, label %if.then2.i, label %if.end6.i

if.then2.i:                                       ; preds = %if.end.i, %if.end.i.preheader
  %call156.i.lcssa = phi ptr [ %call153.i, %if.end.i.preheader ], [ %call.i, %if.end.i ]
  %tobool4.not155.i.lcssa = phi i32 [ 1, %if.end.i.preheader ], [ 0, %if.end.i ]
  %6 = load ptr, ptr %0, align 8, !tbaa !19
  %7 = load ptr, ptr %call156.i.lcssa, align 8, !tbaa !22
  %call3.i = call i32 @mpool_put(ptr noundef %6, ptr noundef %7, i32 noundef 0) #4
  br label %ref___bt_bdelete.exit

if.end6.i:                                        ; preds = %if.end.i.preheader, %if.end.i
  %call156.i111 = phi ptr [ %call.i, %if.end.i ], [ %call153.i, %if.end.i.preheader ]
  %8 = load ptr, ptr %call156.i111, align 8, !tbaa !22
  %index.i = getelementptr inbounds %struct._epg, ptr %call156.i111, i64 0, i32 1
  %lower30.i = getelementptr inbounds %struct._page, ptr %8, i64 0, i32 4
  br label %do.body.i

do.body.i:                                        ; preds = %land.rhs.i, %if.end6.i
  %9 = load i16, ptr %index.i, align 8, !tbaa !23
  %conv.i = zext i16 %9 to i32
  %call8.i = call i32 @ref___bt_dleaf(ptr noundef %0, ptr noundef %key, ptr noundef %8, i32 noundef %conv.i), !range !24
  %tobool9.not.i = icmp eq i32 %call8.i, 0
  br i1 %tobool9.not.i, label %if.end13.i, label %if.then10.i

if.then10.i:                                      ; preds = %do.body.i
  %10 = load ptr, ptr %0, align 8, !tbaa !19
  %call12.i = call i32 @mpool_put(ptr noundef %10, ptr noundef %8, i32 noundef 0) #4
  br label %ref___bt_bdelete.exit

if.end13.i:                                       ; preds = %do.body.i
  %11 = load i32, ptr %flags3, align 4, !tbaa !20
  %and.i = and i32 %11, 32
  %tobool14.not.i = icmp eq i32 %and.i, 0
  br i1 %tobool14.not.i, label %if.end27.i, label %if.then15.i

if.then15.i:                                      ; preds = %if.end13.i
  %12 = load i16, ptr %lower30.i, align 4, !tbaa !25
  %13 = and i16 %12, -2
  %cmp17.i = icmp eq i16 %13, 20
  br i1 %cmp17.i, label %if.then19.i, label %if.else.i

if.then19.i:                                      ; preds = %if.then15.i
  %call20.i = call fastcc i32 @ref___bt_pdelete(ptr noundef nonnull %0, ptr noundef nonnull %8), !range !27
  %tobool21.not.i = icmp eq i32 %call20.i, 0
  br i1 %tobool21.not.i, label %if.end26.i, label %ref___bt_bdelete.exit

if.else.i:                                        ; preds = %if.then15.i
  %14 = load ptr, ptr %0, align 8, !tbaa !19
  %call25.i = call i32 @mpool_put(ptr noundef %14, ptr noundef nonnull %8, i32 noundef 1) #4
  br label %if.end26.i

if.end26.i:                                       ; preds = %if.else.i, %if.then19.i
  br label %ref___bt_bdelete.exit

if.end27.i:                                       ; preds = %if.end13.i
  %15 = load i16, ptr %index.i, align 8, !tbaa !23
  %conv29.i = zext i16 %15 to i64
  %16 = load i16, ptr %lower30.i, align 4, !tbaa !25
  %conv31.i = zext i16 %16 to i64
  %sub32.i = add nsw i64 %conv31.i, -20
  %div33131.i = lshr i64 %sub32.i, 1
  %cmp34.i = icmp ugt i64 %div33131.i, %conv29.i
  br i1 %cmp34.i, label %land.rhs.i, label %do.end.i

land.rhs.i:                                       ; preds = %if.end27.i
  %call36.i = call i32 @__bt_cmp(ptr noundef nonnull %0, ptr noundef %key, ptr noundef nonnull %call156.i111) #4
  %cmp37.i = icmp eq i32 %call36.i, 0
  br i1 %cmp37.i, label %do.body.i, label %land.rhs.do.end_crit_edge.i, !llvm.loop !28

land.rhs.do.end_crit_edge.i:                      ; preds = %land.rhs.i
  %.pre.i = load i16, ptr %index.i, align 8, !tbaa !23
  %.pre171.i = load i16, ptr %lower30.i, align 4, !tbaa !25
  %.pre173.i = zext i16 %.pre.i to i64
  %.pre174.i = zext i16 %.pre171.i to i64
  %.pre175.i = add nsw i64 %.pre174.i, -20
  %.pre176.i = lshr i64 %.pre175.i, 1
  br label %do.end.i

do.end.i:                                         ; preds = %if.end27.i, %land.rhs.do.end_crit_edge.i
  %div44132.pre-phi.i = phi i64 [ %.pre176.i, %land.rhs.do.end_crit_edge.i ], [ %div33131.i, %if.end27.i ]
  %conv40.pre-phi.i = phi i64 [ %.pre173.i, %land.rhs.do.end_crit_edge.i ], [ %conv29.i, %if.end27.i ]
  %17 = phi i16 [ %.pre171.i, %land.rhs.do.end_crit_edge.i ], [ %16, %if.end27.i ]
  %.pr.i = phi i16 [ %.pre.i, %land.rhs.do.end_crit_edge.i ], [ %15, %if.end27.i ]
  %cmp45.i = icmp eq i64 %div44132.pre-phi.i, %conv40.pre-phi.i
  %spec.select.i = zext i1 %cmp45.i to i32
  %dec149.i = add i16 %.pr.i, -1
  store i16 %dec149.i, ptr %index.i, align 8, !tbaa !23
  %cmp51.not150.i = icmp eq i16 %.pr.i, 0
  br i1 %cmp51.not150.i, label %while.end.i, label %while.body.i

while.body.i:                                     ; preds = %do.end.i, %if.end66.i
  %call53.i = call i32 @__bt_cmp(ptr noundef %0, ptr noundef %key, ptr noundef nonnull %call156.i111) #4
  %cmp54.not.i = icmp eq i32 %call53.i, 0
  br i1 %cmp54.not.i, label %if.end57.i, label %while.end.loopexit.i

if.end57.i:                                       ; preds = %while.body.i
  %18 = load i16, ptr %index.i, align 8, !tbaa !23
  %conv59.i = zext i16 %18 to i32
  %call60.i = call i32 @ref___bt_dleaf(ptr noundef %0, ptr noundef %key, ptr noundef %8, i32 noundef %conv59.i), !range !24
  %cmp61.i = icmp eq i32 %call60.i, -1
  br i1 %cmp61.i, label %if.then63.i, label %if.end66.i

if.then63.i:                                      ; preds = %if.end57.i
  %19 = load ptr, ptr %0, align 8, !tbaa !19
  %call65.i = call i32 @mpool_put(ptr noundef %19, ptr noundef %8, i32 noundef 0) #4
  br label %ref___bt_bdelete.exit

if.end66.i:                                       ; preds = %if.end57.i
  %20 = load i16, ptr %index.i, align 8, !tbaa !23
  %cmp69.i = icmp eq i16 %20, 0
  %dec.i = add i16 %20, -1
  store i16 %dec.i, ptr %index.i, align 8, !tbaa !23
  br i1 %cmp69.i, label %while.end.loopexit.i, label %while.body.i, !llvm.loop !30

while.end.loopexit.i:                             ; preds = %if.end66.i, %while.body.i
  %redo.1.lcssa.ph.i = phi i32 [ 1, %if.end66.i ], [ %spec.select.i, %while.body.i ]
  %.pre172.i = load i16, ptr %lower30.i, align 4, !tbaa !25
  br label %while.end.i

while.end.i:                                      ; preds = %while.end.loopexit.i, %do.end.i
  %21 = phi i16 [ %17, %do.end.i ], [ %.pre172.i, %while.end.loopexit.i ]
  %redo.1.lcssa.i = phi i32 [ %spec.select.i, %do.end.i ], [ %redo.1.lcssa.ph.i, %while.end.loopexit.i ]
  %22 = and i16 %21, -2
  %cmp77.i = icmp eq i16 %22, 20
  br i1 %cmp77.i, label %if.then79.i, label %if.end84.i

if.then79.i:                                      ; preds = %while.end.i
  %call80.i = call fastcc i32 @ref___bt_pdelete(ptr noundef %0, ptr noundef nonnull %8), !range !27
  %tobool81.not.i = icmp eq i32 %call80.i, 0
  br i1 %tobool81.not.i, label %loop.backedge.i, label %ref___bt_bdelete.exit

loop.backedge.i:                                  ; preds = %if.end84.i, %if.then79.i
  %call.i = call ptr @__bt_search(ptr noundef %0, ptr noundef %key, ptr noundef nonnull %exact.i) #4
  %cmp.i = icmp eq ptr %call.i, null
  br i1 %cmp.i, label %ref___bt_bdelete.exit, label %if.end.i

if.end84.i:                                       ; preds = %while.end.i
  %23 = load ptr, ptr %0, align 8, !tbaa !19
  %call86.i = call i32 @mpool_put(ptr noundef %23, ptr noundef nonnull %8, i32 noundef 1) #4
  %tobool87.not.i = icmp eq i32 %redo.1.lcssa.i, 0
  br i1 %tobool87.not.i, label %ref___bt_bdelete.exit, label %loop.backedge.i

ref___bt_bdelete.exit:                            ; preds = %if.then79.i, %loop.backedge.i, %if.end84.i, %sw.bb, %if.then2.i, %if.then10.i, %if.then19.i, %if.end26.i, %if.then63.i
  %retval.0.i = phi i32 [ -1, %if.then10.i ], [ 0, %if.end26.i ], [ -1, %if.then63.i ], [ %tobool4.not155.i.lcssa, %if.then2.i ], [ -1, %if.then19.i ], [ -1, %sw.bb ], [ 0, %if.end84.i ], [ -1, %if.then79.i ], [ 0, %loop.backedge.i ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %exact.i) #4
  br label %sw.epilog

sw.bb8:                                           ; preds = %if.end6
  %bt_cursor = getelementptr inbounds %struct._btree, ptr %0, i64 0, i32 4
  %flags9 = getelementptr inbounds %struct._btree, ptr %0, i64 0, i32 4, i32 3
  %24 = load i8, ptr %flags9, align 4, !tbaa !31
  %conv = zext i8 %24 to i32
  %and10 = and i32 %conv, 8
  %tobool11.not = icmp eq i32 %and10, 0
  br i1 %tobool11.not, label %sw.default, label %if.then12

if.then12:                                        ; preds = %sw.bb8
  %and15 = and i32 %conv, 7
  %tobool16.not = icmp eq i32 %and15, 0
  br i1 %tobool16.not, label %if.end18, label %cleanup

if.end18:                                         ; preds = %if.then12
  %25 = load ptr, ptr %0, align 8, !tbaa !19
  %26 = load i32, ptr %bt_cursor, align 8, !tbaa !32
  %call20 = tail call ptr @mpool_get(ptr noundef %25, i32 noundef %26, i32 noundef 0) #4
  store ptr %call20, ptr %h, align 8, !tbaa !33
  %cmp21 = icmp eq ptr %call20, null
  br i1 %cmp21, label %cleanup, label %if.end24

if.end24:                                         ; preds = %if.end18
  %lower = getelementptr inbounds %struct._page, ptr %call20, i64 0, i32 4
  %27 = load i16, ptr %lower, align 4, !tbaa !25
  %28 = and i16 %27, -2
  %cmp26 = icmp eq i16 %28, 22
  br i1 %cmp26, label %if.then28, label %if.end34

if.then28:                                        ; preds = %if.end24
  %call30 = call fastcc i32 @ref___bt_stkacq(ptr noundef nonnull %0, ptr noundef nonnull %h, ptr noundef nonnull %bt_cursor), !range !34
  %tobool31.not = icmp eq i32 %call30, 0
  br i1 %tobool31.not, label %if.then28.if.end34_crit_edge, label %cleanup

if.then28.if.end34_crit_edge:                     ; preds = %if.then28
  %.pre = load ptr, ptr %h, align 8, !tbaa !33
  br label %if.end34

if.end34:                                         ; preds = %if.then28.if.end34_crit_edge, %if.end24
  %29 = phi ptr [ %.pre, %if.then28.if.end34_crit_edge ], [ %call20, %if.end24 ]
  %index = getelementptr inbounds %struct._btree, ptr %0, i64 0, i32 4, i32 0, i32 1
  %30 = load i16, ptr %index, align 4, !tbaa !35
  %conv36 = zext i16 %30 to i32
  %call37 = tail call i32 @ref___bt_dleaf(ptr noundef nonnull %0, ptr noundef null, ptr noundef %29, i32 noundef %conv36), !range !24
  %lower38 = getelementptr inbounds %struct._page, ptr %29, i64 0, i32 4
  %31 = load i16, ptr %lower38, align 4, !tbaa !25
  %32 = and i16 %31, -2
  %cmp42 = icmp eq i16 %32, 20
  %cmp44 = icmp eq i32 %call37, 0
  %or.cond = and i1 %cmp44, %cmp42
  br i1 %or.cond, label %if.then46, label %if.else

if.then46:                                        ; preds = %if.end34
  %call47 = tail call fastcc i32 @ref___bt_pdelete(ptr noundef nonnull %0, ptr noundef nonnull %29), !range !27
  %tobool48.not = icmp eq i32 %call47, 0
  br i1 %tobool48.not, label %if.then60, label %cleanup

if.else:                                          ; preds = %if.end34
  %33 = load ptr, ptr %0, align 8, !tbaa !19
  %cond = zext i1 %cmp44 to i32
  %call54 = tail call i32 @mpool_put(ptr noundef %33, ptr noundef nonnull %29, i32 noundef %cond) #4
  br label %sw.epilog

sw.default:                                       ; preds = %sw.bb8, %if.end6
  %call57 = tail call ptr @__error() #4
  store i32 22, ptr %call57, align 4, !tbaa !21
  br label %cleanup

sw.epilog:                                        ; preds = %if.else, %ref___bt_bdelete.exit
  %status.0 = phi i32 [ %call37, %if.else ], [ %retval.0.i, %ref___bt_bdelete.exit ]
  %cmp58 = icmp eq i32 %status.0, 0
  br i1 %cmp58, label %if.then60, label %cleanup

if.then60:                                        ; preds = %if.then46, %sw.epilog
  %34 = load i32, ptr %flags3, align 4, !tbaa !20
  %or = or i32 %34, 4
  store i32 %or, ptr %flags3, align 4, !tbaa !20
  br label %cleanup

cleanup:                                          ; preds = %sw.epilog, %if.then60, %if.then46, %if.then28, %if.end18, %if.then12, %sw.default, %if.then4
  %retval.0 = phi i32 [ -1, %if.then4 ], [ -1, %sw.default ], [ 1, %if.then12 ], [ -1, %if.end18 ], [ -1, %if.then28 ], [ -1, %if.then46 ], [ 0, %if.then60 ], [ %status.0, %sw.epilog ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %h) #4
  ret i32 %retval.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

declare i32 @mpool_put(ptr noundef, ptr noundef, i32 noundef) local_unnamed_addr #2

declare ptr @__error() local_unnamed_addr #2

declare ptr @mpool_get(ptr noundef, i32 noundef, i32 noundef) local_unnamed_addr #2

; Function Attrs: nounwind uwtable
define internal fastcc i32 @ref___bt_stkacq(ptr noundef %t, ptr nocapture noundef %hp, ptr noundef %c) unnamed_addr #0 {
entry:
  %exact = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %exact) #4
  %0 = load ptr, ptr %hp, align 8, !tbaa !33
  %1 = load ptr, ptr %t, align 8, !tbaa !19
  %call = tail call i32 @mpool_put(ptr noundef %1, ptr noundef %0, i32 noundef 0) #4
  %key = getelementptr inbounds %struct._cursor, ptr %c, i64 0, i32 1
  %call1 = call ptr @__bt_search(ptr noundef nonnull %t, ptr noundef nonnull %key, ptr noundef nonnull %exact) #4
  %cmp = icmp eq ptr %call1, null
  br i1 %cmp, label %cleanup, label %if.end

if.end:                                           ; preds = %entry
  %2 = load ptr, ptr %call1, align 8, !tbaa !22
  %3 = load i32, ptr %2, align 4, !tbaa !36
  %4 = load i32, ptr %c, align 8, !tbaa !32
  %cmp4 = icmp eq i32 %3, %4
  br i1 %cmp4, label %ret, label %while.cond.preheader

while.cond.preheader:                             ; preds = %if.end
  %bt_sp = getelementptr inbounds %struct._btree, ptr %t, i64 0, i32 6
  %bt_stack = getelementptr inbounds %struct._btree, ptr %t, i64 0, i32 5
  br label %while.cond

while.cond:                                       ; preds = %while.cond.preheader, %while.end
  %h.0 = phi ptr [ %call65, %while.end ], [ %2, %while.cond.preheader ]
  %idx.0 = phi i16 [ %idx.2.lcssa, %while.end ], [ undef, %while.cond.preheader ]
  %5 = load i32, ptr %h.0, align 4, !tbaa !36
  %6 = load i32, ptr %c, align 8, !tbaa !32
  %cmp10.not = icmp eq i32 %5, %6
  br i1 %cmp10.not, label %ret, label %while.body

while.body:                                       ; preds = %while.cond
  %nextpg11 = getelementptr inbounds %struct._page, ptr %h.0, i64 0, i32 2
  %7 = load i32, ptr %nextpg11, align 4, !tbaa !37
  %cmp12 = icmp eq i32 %7, 0
  %8 = load ptr, ptr %t, align 8, !tbaa !19
  %call79 = call i32 @mpool_put(ptr noundef %8, ptr noundef nonnull %h.0, i32 noundef 0) #4
  br i1 %cmp12, label %if.end77, label %if.end14

if.end14:                                         ; preds = %while.body
  %9 = load ptr, ptr %bt_sp, align 8, !tbaa !38
  %cmp17337 = icmp eq ptr %9, %bt_stack
  br i1 %cmp17337, label %while.end, label %for.body

for.body:                                         ; preds = %if.end14, %if.end41
  %10 = phi ptr [ %16, %if.end41 ], [ %9, %if.end14 ]
  %level.0338 = phi i32 [ %inc, %if.end41 ], [ 0, %if.end14 ]
  %incdec.ptr = getelementptr inbounds %struct._epgno, ptr %10, i64 -1
  store ptr %incdec.ptr, ptr %bt_sp, align 8, !tbaa !38
  %11 = load ptr, ptr %t, align 8, !tbaa !19
  %12 = load i32, ptr %incdec.ptr, align 4, !tbaa !39
  %call22 = call ptr @mpool_get(ptr noundef %11, i32 noundef %12, i32 noundef 0) #4
  %cmp23 = icmp eq ptr %call22, null
  br i1 %cmp23, label %cleanup, label %if.end25

if.end25:                                         ; preds = %for.body
  %index = getelementptr %struct._epgno, ptr %10, i64 -1, i32 1
  %13 = load i16, ptr %index, align 4, !tbaa !40
  %conv = zext i16 %13 to i64
  %lower = getelementptr inbounds %struct._page, ptr %call22, i64 0, i32 4
  %14 = load i16, ptr %lower, align 4, !tbaa !25
  %conv26 = zext i16 %14 to i64
  %sub = add nsw i64 %conv26, -20
  %div291 = lshr i64 %sub, 1
  %sub27 = add nsw i64 %div291, -1
  %cmp28.not = icmp eq i64 %sub27, %conv
  br i1 %cmp28.not, label %if.end41, label %for.end

if.end41:                                         ; preds = %if.end25
  %15 = load ptr, ptr %t, align 8, !tbaa !19
  %call43 = call i32 @mpool_put(ptr noundef %15, ptr noundef nonnull %call22, i32 noundef 0) #4
  %inc = add nuw nsw i32 %level.0338, 1
  %16 = load ptr, ptr %bt_sp, align 8, !tbaa !38
  %cmp17 = icmp eq ptr %16, %bt_stack
  br i1 %cmp17, label %while.body45.peel, label %for.body, !llvm.loop !41

for.end:                                          ; preds = %if.end25
  %add = add i16 %13, 1
  %17 = load i32, ptr %call22, align 4, !tbaa !36
  %18 = load ptr, ptr %bt_sp, align 8, !tbaa !38
  store i32 %17, ptr %18, align 4, !tbaa !39
  %index38 = getelementptr inbounds %struct._epgno, ptr %18, i64 0, i32 1
  store i16 %add, ptr %index38, align 4, !tbaa !40
  %incdec.ptr40 = getelementptr inbounds %struct._epgno, ptr %18, i64 1
  store ptr %incdec.ptr40, ptr %bt_sp, align 8, !tbaa !38
  %tobool.not.peel = icmp eq i32 %level.0338, 0
  br i1 %tobool.not.peel, label %while.end, label %while.body45.peel

while.body45.peel:                                ; preds = %if.end41, %for.end
  %idx.1397 = phi i16 [ %add, %for.end ], [ %idx.0, %if.end41 ]
  %level.0322395 = phi i32 [ %level.0338, %for.end ], [ %inc, %if.end41 ]
  %19 = phi ptr [ %incdec.ptr40, %for.end ], [ %bt_stack, %if.end41 ]
  %idxprom.peel = zext i16 %idx.1397 to i64
  %arrayidx.peel = getelementptr inbounds %struct._page, ptr %call22, i64 0, i32 6, i64 %idxprom.peel
  %20 = load i16, ptr %arrayidx.peel, align 2, !tbaa !42
  %idx.ext.peel = zext i16 %20 to i64
  %add.ptr.peel = getelementptr inbounds i8, ptr %call22, i64 %idx.ext.peel
  %pgno47.peel = getelementptr inbounds %struct._binternal, ptr %add.ptr.peel, i64 0, i32 1
  %21 = load i32, ptr %pgno47.peel, align 4, !tbaa !43
  store i32 %21, ptr %19, align 4, !tbaa !39
  %index51.peel = getelementptr inbounds %struct._epgno, ptr %19, i64 0, i32 1
  store i16 0, ptr %index51.peel, align 4, !tbaa !40
  %incdec.ptr53.peel = getelementptr inbounds %struct._epgno, ptr %19, i64 1
  store ptr %incdec.ptr53.peel, ptr %bt_sp, align 8, !tbaa !38
  %22 = load ptr, ptr %t, align 8, !tbaa !19
  %call55.peel = call i32 @mpool_put(ptr noundef %22, ptr noundef nonnull %call22, i32 noundef 0) #4
  %23 = load ptr, ptr %t, align 8, !tbaa !19
  %call57.peel = call ptr @mpool_get(ptr noundef %23, i32 noundef %21, i32 noundef 0) #4
  %cmp58.peel = icmp eq ptr %call57.peel, null
  br i1 %cmp58.peel, label %cleanup, label %while.cond44

while.cond44:                                     ; preds = %while.body45.peel, %while.body45
  %h.3 = phi ptr [ %call57, %while.body45 ], [ %call57.peel, %while.body45.peel ]
  %level.1.in = phi i32 [ %level.1, %while.body45 ], [ %level.0322395, %while.body45.peel ]
  %level.1 = add nsw i32 %level.1.in, -1
  %tobool.not = icmp eq i32 %level.1, 0
  br i1 %tobool.not, label %while.end, label %while.body45

while.body45:                                     ; preds = %while.cond44
  %arrayidx = getelementptr inbounds %struct._page, ptr %h.3, i64 0, i32 6, i64 0
  %24 = load i16, ptr %arrayidx, align 2, !tbaa !42
  %idx.ext = zext i16 %24 to i64
  %add.ptr = getelementptr inbounds i8, ptr %h.3, i64 %idx.ext
  %pgno47 = getelementptr inbounds %struct._binternal, ptr %add.ptr, i64 0, i32 1
  %25 = load i32, ptr %pgno47, align 4, !tbaa !43
  %26 = load ptr, ptr %bt_sp, align 8, !tbaa !38
  store i32 %25, ptr %26, align 4, !tbaa !39
  %index51 = getelementptr inbounds %struct._epgno, ptr %26, i64 0, i32 1
  store i16 0, ptr %index51, align 4, !tbaa !40
  %incdec.ptr53 = getelementptr inbounds %struct._epgno, ptr %26, i64 1
  store ptr %incdec.ptr53, ptr %bt_sp, align 8, !tbaa !38
  %27 = load ptr, ptr %t, align 8, !tbaa !19
  %call55 = call i32 @mpool_put(ptr noundef %27, ptr noundef nonnull %h.3, i32 noundef 0) #4
  %28 = load ptr, ptr %t, align 8, !tbaa !19
  %call57 = call ptr @mpool_get(ptr noundef %28, i32 noundef %25, i32 noundef 0) #4
  %cmp58 = icmp eq ptr %call57, null
  br i1 %cmp58, label %cleanup, label %while.cond44, !llvm.loop !45

while.end:                                        ; preds = %while.cond44, %if.end14, %for.end
  %h.3.lcssa = phi ptr [ %call22, %for.end ], [ %h.0, %if.end14 ], [ %h.3, %while.cond44 ]
  %idx.2.lcssa = phi i16 [ %add, %for.end ], [ %idx.0, %if.end14 ], [ 0, %while.cond44 ]
  %29 = load ptr, ptr %t, align 8, !tbaa !19
  %call63 = call i32 @mpool_put(ptr noundef %29, ptr noundef nonnull %h.3.lcssa, i32 noundef 0) #4
  %30 = load ptr, ptr %t, align 8, !tbaa !19
  %call65 = call ptr @mpool_get(ptr noundef %30, i32 noundef %7, i32 noundef 0) #4
  %cmp66 = icmp eq ptr %call65, null
  br i1 %cmp66, label %cleanup, label %while.cond, !llvm.loop !47

if.end77:                                         ; preds = %while.body
  %call81 = call ptr @__bt_search(ptr noundef nonnull %t, ptr noundef nonnull %key, ptr noundef nonnull %exact) #4
  %cmp82 = icmp eq ptr %call81, null
  br i1 %cmp82, label %cleanup, label %if.end85

if.end85:                                         ; preds = %if.end77
  %31 = load ptr, ptr %call81, align 8, !tbaa !22
  br label %while.cond87

while.cond87:                                     ; preds = %while.end176, %if.end85
  %h.4 = phi ptr [ %31, %if.end85 ], [ %call180, %while.end176 ]
  %idx.3 = phi i16 [ %idx.0, %if.end85 ], [ %idx.5.lcssa, %while.end176 ]
  %32 = load i32, ptr %h.4, align 4, !tbaa !36
  %33 = load i32, ptr %c, align 8, !tbaa !32
  %cmp91.not = icmp eq i32 %32, %33
  br i1 %cmp91.not, label %ret, label %while.body93

while.body93:                                     ; preds = %while.cond87
  %prevpg94 = getelementptr inbounds %struct._page, ptr %h.4, i64 0, i32 1
  %34 = load i32, ptr %prevpg94, align 4, !tbaa !48
  %cmp95 = icmp eq i32 %34, 0
  br i1 %cmp95, label %ret, label %if.end98

if.end98:                                         ; preds = %while.body93
  %35 = load ptr, ptr %t, align 8, !tbaa !19
  %call100 = call i32 @mpool_put(ptr noundef %35, ptr noundef nonnull %h.4, i32 noundef 0) #4
  %36 = load ptr, ptr %bt_sp, align 8, !tbaa !38
  %cmp105341 = icmp eq ptr %36, %bt_stack
  br i1 %cmp105341, label %while.end176, label %for.body115

for.body115:                                      ; preds = %if.end98, %if.end139
  %37 = phi ptr [ %42, %if.end139 ], [ %36, %if.end98 ]
  %level.2342 = phi i32 [ %inc143, %if.end139 ], [ 0, %if.end98 ]
  %incdec.ptr110 = getelementptr inbounds %struct._epgno, ptr %37, i64 -1
  store ptr %incdec.ptr110, ptr %bt_sp, align 8, !tbaa !38
  %38 = load ptr, ptr %t, align 8, !tbaa !19
  %39 = load i32, ptr %incdec.ptr110, align 4, !tbaa !39
  %call118 = call ptr @mpool_get(ptr noundef %38, i32 noundef %39, i32 noundef 0) #4
  %cmp119 = icmp eq ptr %call118, null
  br i1 %cmp119, label %cleanup, label %if.end122

if.end122:                                        ; preds = %for.body115
  %index123 = getelementptr %struct._epgno, ptr %37, i64 -1, i32 1
  %40 = load i16, ptr %index123, align 4, !tbaa !40
  %cmp125.not = icmp eq i16 %40, 0
  br i1 %cmp125.not, label %if.end139, label %for.end144

if.end139:                                        ; preds = %if.end122
  %41 = load ptr, ptr %t, align 8, !tbaa !19
  %call141 = call i32 @mpool_put(ptr noundef %41, ptr noundef nonnull %call118, i32 noundef 0) #4
  %inc143 = add nuw nsw i32 %level.2342, 1
  %42 = load ptr, ptr %bt_sp, align 8, !tbaa !38
  %cmp105 = icmp eq ptr %42, %bt_stack
  br i1 %cmp105, label %while.body148.preheader, label %for.body115, !llvm.loop !49

for.end144:                                       ; preds = %if.end122
  %sub130 = add i16 %40, -1
  %43 = load i32, ptr %call118, align 4, !tbaa !36
  %44 = load ptr, ptr %bt_sp, align 8, !tbaa !38
  store i32 %43, ptr %44, align 4, !tbaa !39
  %index136 = getelementptr inbounds %struct._epgno, ptr %44, i64 0, i32 1
  store i16 %sub130, ptr %index136, align 4, !tbaa !40
  %incdec.ptr138 = getelementptr inbounds %struct._epgno, ptr %44, i64 1
  store ptr %incdec.ptr138, ptr %bt_sp, align 8, !tbaa !38
  %tobool147.not347 = icmp eq i32 %level.2342, 0
  br i1 %tobool147.not347, label %while.end176, label %while.body148.preheader

while.body148.preheader:                          ; preds = %if.end139, %for.end144
  %dec146350.in.ph = phi i32 [ %level.2342, %for.end144 ], [ %inc143, %if.end139 ]
  %idx.5349.ph = phi i16 [ %sub130, %for.end144 ], [ %idx.3, %if.end139 ]
  br label %while.body148

while.body148:                                    ; preds = %while.body148.preheader, %if.end163
  %dec146350.in = phi i32 [ %dec146350, %if.end163 ], [ %dec146350.in.ph, %while.body148.preheader ]
  %idx.5349 = phi i16 [ %conv169, %if.end163 ], [ %idx.5349.ph, %while.body148.preheader ]
  %h.7348 = phi ptr [ %call159, %if.end163 ], [ %call118, %while.body148.preheader ]
  %idxprom150 = zext i16 %idx.5349 to i64
  %arrayidx151 = getelementptr inbounds %struct._page, ptr %h.7348, i64 0, i32 6, i64 %idxprom150
  %45 = load i16, ptr %arrayidx151, align 2, !tbaa !42
  %idx.ext153 = zext i16 %45 to i64
  %add.ptr154 = getelementptr inbounds i8, ptr %h.7348, i64 %idx.ext153
  %pgno155 = getelementptr inbounds %struct._binternal, ptr %add.ptr154, i64 0, i32 1
  %46 = load i32, ptr %pgno155, align 4, !tbaa !43
  %47 = load ptr, ptr %t, align 8, !tbaa !19
  %call157 = call i32 @mpool_put(ptr noundef %47, ptr noundef nonnull %h.7348, i32 noundef 0) #4
  %48 = load ptr, ptr %t, align 8, !tbaa !19
  %call159 = call ptr @mpool_get(ptr noundef %48, i32 noundef %46, i32 noundef 0) #4
  %cmp160 = icmp eq ptr %call159, null
  br i1 %cmp160, label %cleanup, label %if.end163

if.end163:                                        ; preds = %while.body148
  %dec146350 = add nsw i32 %dec146350.in, -1
  %lower164 = getelementptr inbounds %struct._page, ptr %call159, i64 0, i32 4
  %49 = load i16, ptr %lower164, align 4, !tbaa !25
  %conv165 = zext i16 %49 to i32
  %sub166 = add nuw nsw i32 %conv165, 131052
  %div167292 = lshr i32 %sub166, 1
  %50 = trunc i32 %div167292 to i16
  %conv169 = add nsw i16 %50, -1
  %51 = load ptr, ptr %bt_sp, align 8, !tbaa !38
  store i32 %46, ptr %51, align 4, !tbaa !39
  %index173 = getelementptr inbounds %struct._epgno, ptr %51, i64 0, i32 1
  store i16 %conv169, ptr %index173, align 4, !tbaa !40
  %incdec.ptr175 = getelementptr inbounds %struct._epgno, ptr %51, i64 1
  store ptr %incdec.ptr175, ptr %bt_sp, align 8, !tbaa !38
  %tobool147.not = icmp eq i32 %dec146350, 0
  br i1 %tobool147.not, label %while.end176, label %while.body148, !llvm.loop !50

while.end176:                                     ; preds = %if.end163, %if.end98, %for.end144
  %h.7.lcssa = phi ptr [ %call118, %for.end144 ], [ %h.4, %if.end98 ], [ %call159, %if.end163 ]
  %idx.5.lcssa = phi i16 [ %sub130, %for.end144 ], [ %idx.3, %if.end98 ], [ %conv169, %if.end163 ]
  %52 = load ptr, ptr %t, align 8, !tbaa !19
  %call178 = call i32 @mpool_put(ptr noundef %52, ptr noundef nonnull %h.7.lcssa, i32 noundef 0) #4
  %53 = load ptr, ptr %t, align 8, !tbaa !19
  %call180 = call ptr @mpool_get(ptr noundef %53, i32 noundef %34, i32 noundef 0) #4
  %cmp181 = icmp eq ptr %call180, null
  br i1 %cmp181, label %cleanup, label %while.cond87, !llvm.loop !51

ret:                                              ; preds = %while.cond, %while.cond87, %while.body93, %if.end
  %h.8 = phi ptr [ %2, %if.end ], [ %h.4, %while.body93 ], [ %h.4, %while.cond87 ], [ %h.0, %while.cond ]
  %54 = load ptr, ptr %t, align 8, !tbaa !19
  %call187 = call i32 @mpool_put(ptr noundef %54, ptr noundef nonnull %h.8, i32 noundef 0) #4
  %55 = load ptr, ptr %t, align 8, !tbaa !19
  %56 = load i32, ptr %c, align 8, !tbaa !32
  %call191 = call ptr @mpool_get(ptr noundef %55, i32 noundef %56, i32 noundef 0) #4
  store ptr %call191, ptr %hp, align 8, !tbaa !33
  %cmp192 = icmp eq ptr %call191, null
  %conv193 = zext i1 %cmp192 to i32
  br label %cleanup

cleanup:                                          ; preds = %while.end, %while.body45.peel, %for.body, %while.body45, %while.end176, %for.body115, %while.body148, %if.end77, %entry, %ret
  %retval.0 = phi i32 [ %conv193, %ret ], [ 1, %entry ], [ 1, %if.end77 ], [ 1, %while.body148 ], [ 1, %for.body115 ], [ 1, %while.end176 ], [ 1, %while.body45 ], [ 1, %for.body ], [ 1, %while.body45.peel ], [ 1, %while.end ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %exact) #4
  ret i32 %retval.0
}

; Function Attrs: nounwind uwtable
define internal fastcc i32 @ref___bt_pdelete(ptr noundef %t, ptr noundef %h) unnamed_addr #0 {
entry:
  %bt_sp = getelementptr inbounds %struct._btree, ptr %t, i64 0, i32 6
  %bt_stack = getelementptr inbounds %struct._btree, ptr %t, i64 0, i32 5
  br label %while.cond

while.cond:                                       ; preds = %lor.lhs.false, %entry
  %0 = load ptr, ptr %bt_sp, align 8, !tbaa !38
  %cmp = icmp eq ptr %0, %bt_stack
  br i1 %cmp, label %while.end, label %while.body

while.body:                                       ; preds = %while.cond
  %incdec.ptr = getelementptr inbounds %struct._epgno, ptr %0, i64 -1
  store ptr %incdec.ptr, ptr %bt_sp, align 8, !tbaa !38
  %1 = load ptr, ptr %t, align 8, !tbaa !19
  %2 = load i32, ptr %incdec.ptr, align 4, !tbaa !39
  %call = tail call ptr @mpool_get(ptr noundef %1, i32 noundef %2, i32 noundef 0) #4
  %cmp3 = icmp eq ptr %call, null
  br i1 %cmp3, label %cleanup, label %if.end

if.end:                                           ; preds = %while.body
  %index = getelementptr %struct._epgno, ptr %0, i64 -1, i32 1
  %3 = load i16, ptr %index, align 4, !tbaa !40
  %idxprom = zext i16 %3 to i64
  %arrayidx = getelementptr inbounds %struct._page, ptr %call, i64 0, i32 6, i64 %idxprom
  %4 = load i16, ptr %arrayidx, align 2, !tbaa !42
  %idx.ext = zext i16 %4 to i64
  %add.ptr = getelementptr inbounds i8, ptr %call, i64 %idx.ext
  %flags = getelementptr inbounds %struct._binternal, ptr %add.ptr, i64 0, i32 2
  %5 = load i8, ptr %flags, align 4, !tbaa !52
  %6 = and i8 %5, 2
  %tobool.not = icmp eq i8 %6, 0
  br i1 %tobool.not, label %if.end12, label %land.lhs.true

land.lhs.true:                                    ; preds = %if.end
  %bytes = getelementptr inbounds %struct._binternal, ptr %add.ptr, i64 0, i32 3
  %call6 = tail call i32 @__ovfl_delete(ptr noundef nonnull %t, ptr noundef nonnull %bytes) #4
  %cmp7 = icmp eq i32 %call6, -1
  br i1 %cmp7, label %if.then9, label %if.end12

if.then9:                                         ; preds = %land.lhs.true
  %7 = load ptr, ptr %t, align 8, !tbaa !19
  %call11 = tail call i32 @mpool_put(ptr noundef %7, ptr noundef nonnull %call, i32 noundef 0) #4
  br label %cleanup

if.end12:                                         ; preds = %land.lhs.true, %if.end
  %lower = getelementptr inbounds %struct._page, ptr %call, i64 0, i32 4
  %8 = load i16, ptr %lower, align 4, !tbaa !25
  %9 = and i16 %8, -2
  %cmp14 = icmp eq i16 %9, 22
  br i1 %cmp14, label %if.then16, label %if.else31

if.then16:                                        ; preds = %if.end12
  %10 = load i32, ptr %call, align 4, !tbaa !36
  %cmp18 = icmp eq i32 %10, 1
  br i1 %cmp18, label %if.then20, label %if.else

if.then20:                                        ; preds = %if.then16
  %lower.le = getelementptr inbounds %struct._page, ptr %call, i64 0, i32 4
  store i16 20, ptr %lower.le, align 4, !tbaa !25
  %bt_psize = getelementptr inbounds %struct._btree, ptr %t, i64 0, i32 11
  %11 = load i32, ptr %bt_psize, align 8, !tbaa !53
  %conv22 = trunc i32 %11 to i16
  %upper = getelementptr inbounds %struct._page, ptr %call, i64 0, i32 5
  store i16 %conv22, ptr %upper, align 2, !tbaa !54
  %flags23 = getelementptr inbounds %struct._page, ptr %call, i64 0, i32 3
  store i32 2, ptr %flags23, align 4, !tbaa !55
  br label %if.end99

if.else:                                          ; preds = %if.then16
  %call24 = tail call fastcc i32 @ref___bt_relink(ptr noundef nonnull %t, ptr noundef nonnull %call), !range !24
  %tobool25.not = icmp eq i32 %call24, 0
  br i1 %tobool25.not, label %lor.lhs.false, label %cleanup

lor.lhs.false:                                    ; preds = %if.else
  %call26 = tail call i32 @__bt_free(ptr noundef nonnull %t, ptr noundef nonnull %call) #4
  %tobool27.not = icmp eq i32 %call26, 0
  br i1 %tobool27.not, label %while.cond, label %cleanup, !llvm.loop !56

if.else31:                                        ; preds = %if.end12
  %arrayidx.le = getelementptr inbounds %struct._page, ptr %call, i64 0, i32 6, i64 %idxprom
  %lower.le224 = getelementptr inbounds %struct._page, ptr %call, i64 0, i32 4
  %linp.le = getelementptr inbounds %struct._page, ptr %call, i64 0, i32 6
  %12 = load i32, ptr %add.ptr, align 4, !tbaa !57
  %13 = and i32 %12, -4
  %and35 = add i32 %13, 12
  %upper37 = getelementptr inbounds %struct._page, ptr %call, i64 0, i32 5
  %14 = load i16, ptr %upper37, align 2, !tbaa !54
  %idx.ext39 = zext i16 %14 to i64
  %add.ptr40 = getelementptr inbounds i8, ptr %call, i64 %idx.ext39
  %idx.ext41 = zext i32 %and35 to i64
  %add.ptr42 = getelementptr inbounds i8, ptr %add.ptr40, i64 %idx.ext41
  %sub.ptr.lhs.cast = ptrtoint ptr %add.ptr to i64
  %sub.ptr.rhs.cast = ptrtoint ptr %add.ptr40 to i64
  %sub.ptr.sub = sub i64 %sub.ptr.lhs.cast, %sub.ptr.rhs.cast
  tail call void @llvm.memmove.p0.p0.i64(ptr nonnull align 1 %add.ptr42, ptr nonnull align 1 %add.ptr40, i64 %sub.ptr.sub, i1 false)
  %15 = load i16, ptr %upper37, align 2, !tbaa !54
  %16 = trunc i32 %and35 to i16
  %conv46 = add i16 %15, %16
  store i16 %conv46, ptr %upper37, align 2, !tbaa !54
  %17 = load i16, ptr %arrayidx.le, align 2, !tbaa !42
  %tobool52.not196 = icmp eq i16 %3, 0
  br i1 %tobool52.not196, label %for.end, label %for.body.preheader

for.body.preheader:                               ; preds = %if.else31
  %18 = zext i16 %3 to i64
  %min.iters.check = icmp ult i16 %3, 16
  br i1 %min.iters.check, label %for.body.preheader310, label %vector.ph

vector.ph:                                        ; preds = %for.body.preheader
  %n.vec = and i64 %18, 65520
  %.cast = trunc i64 %n.vec to i16
  %ind.end = sub i16 %3, %.cast
  %19 = shl nuw nsw i64 %n.vec, 1
  %ind.end237 = getelementptr i8, ptr %linp.le, i64 %19
  %broadcast.splatinsert = insertelement <8 x i16> poison, i16 %17, i64 0
  %broadcast.splat = shufflevector <8 x i16> %broadcast.splatinsert, <8 x i16> poison, <8 x i32> zeroinitializer
  br label %vector.body

vector.body:                                      ; preds = %pred.store.continue285, %vector.ph
  %index239 = phi i64 [ 0, %vector.ph ], [ %index.next, %pred.store.continue285 ]
  %20 = shl i64 %index239, 1
  %next.gep = getelementptr i8, ptr %linp.le, i64 %20
  %21 = shl i64 %index239, 1
  %22 = or disjoint i64 %21, 2
  %next.gep240 = getelementptr i8, ptr %linp.le, i64 %22
  %23 = shl i64 %index239, 1
  %24 = or disjoint i64 %23, 4
  %next.gep241 = getelementptr i8, ptr %linp.le, i64 %24
  %25 = shl i64 %index239, 1
  %26 = or disjoint i64 %25, 6
  %next.gep242 = getelementptr i8, ptr %linp.le, i64 %26
  %27 = shl i64 %index239, 1
  %28 = or disjoint i64 %27, 8
  %next.gep243 = getelementptr i8, ptr %linp.le, i64 %28
  %29 = shl i64 %index239, 1
  %30 = or disjoint i64 %29, 10
  %next.gep244 = getelementptr i8, ptr %linp.le, i64 %30
  %31 = shl i64 %index239, 1
  %32 = or disjoint i64 %31, 12
  %next.gep245 = getelementptr i8, ptr %linp.le, i64 %32
  %33 = shl i64 %index239, 1
  %34 = or disjoint i64 %33, 14
  %next.gep246 = getelementptr i8, ptr %linp.le, i64 %34
  %35 = shl i64 %index239, 1
  %36 = or disjoint i64 %35, 16
  %next.gep247 = getelementptr i8, ptr %linp.le, i64 %36
  %37 = shl i64 %index239, 1
  %38 = or disjoint i64 %37, 18
  %next.gep248 = getelementptr i8, ptr %linp.le, i64 %38
  %39 = shl i64 %index239, 1
  %40 = or disjoint i64 %39, 20
  %next.gep249 = getelementptr i8, ptr %linp.le, i64 %40
  %41 = shl i64 %index239, 1
  %42 = or disjoint i64 %41, 22
  %next.gep250 = getelementptr i8, ptr %linp.le, i64 %42
  %43 = shl i64 %index239, 1
  %44 = or disjoint i64 %43, 24
  %next.gep251 = getelementptr i8, ptr %linp.le, i64 %44
  %45 = shl i64 %index239, 1
  %46 = or disjoint i64 %45, 26
  %next.gep252 = getelementptr i8, ptr %linp.le, i64 %46
  %47 = shl i64 %index239, 1
  %48 = or disjoint i64 %47, 28
  %next.gep253 = getelementptr i8, ptr %linp.le, i64 %48
  %49 = shl i64 %index239, 1
  %50 = or disjoint i64 %49, 30
  %next.gep254 = getelementptr i8, ptr %linp.le, i64 %50
  %51 = getelementptr i16, ptr %next.gep, i64 8
  %wide.load = load <8 x i16>, ptr %next.gep, align 2, !tbaa !42
  %wide.load255 = load <8 x i16>, ptr %51, align 2, !tbaa !42
  %52 = icmp ult <8 x i16> %wide.load, %broadcast.splat
  %53 = icmp ult <8 x i16> %wide.load255, %broadcast.splat
  %54 = extractelement <8 x i1> %52, i64 0
  br i1 %54, label %pred.store.if, label %pred.store.continue

pred.store.if:                                    ; preds = %vector.body
  %55 = extractelement <8 x i16> %wide.load, i64 0
  %56 = add i16 %55, %16
  store i16 %56, ptr %next.gep, align 2, !tbaa !42
  br label %pred.store.continue

pred.store.continue:                              ; preds = %pred.store.if, %vector.body
  %57 = extractelement <8 x i1> %52, i64 1
  br i1 %57, label %pred.store.if256, label %pred.store.continue257

pred.store.if256:                                 ; preds = %pred.store.continue
  %58 = extractelement <8 x i16> %wide.load, i64 1
  %59 = add i16 %58, %16
  store i16 %59, ptr %next.gep240, align 2, !tbaa !42
  br label %pred.store.continue257

pred.store.continue257:                           ; preds = %pred.store.if256, %pred.store.continue
  %60 = extractelement <8 x i1> %52, i64 2
  br i1 %60, label %pred.store.if258, label %pred.store.continue259

pred.store.if258:                                 ; preds = %pred.store.continue257
  %61 = extractelement <8 x i16> %wide.load, i64 2
  %62 = add i16 %61, %16
  store i16 %62, ptr %next.gep241, align 2, !tbaa !42
  br label %pred.store.continue259

pred.store.continue259:                           ; preds = %pred.store.if258, %pred.store.continue257
  %63 = extractelement <8 x i1> %52, i64 3
  br i1 %63, label %pred.store.if260, label %pred.store.continue261

pred.store.if260:                                 ; preds = %pred.store.continue259
  %64 = extractelement <8 x i16> %wide.load, i64 3
  %65 = add i16 %64, %16
  store i16 %65, ptr %next.gep242, align 2, !tbaa !42
  br label %pred.store.continue261

pred.store.continue261:                           ; preds = %pred.store.if260, %pred.store.continue259
  %66 = extractelement <8 x i1> %52, i64 4
  br i1 %66, label %pred.store.if262, label %pred.store.continue263

pred.store.if262:                                 ; preds = %pred.store.continue261
  %67 = extractelement <8 x i16> %wide.load, i64 4
  %68 = add i16 %67, %16
  store i16 %68, ptr %next.gep243, align 2, !tbaa !42
  br label %pred.store.continue263

pred.store.continue263:                           ; preds = %pred.store.if262, %pred.store.continue261
  %69 = extractelement <8 x i1> %52, i64 5
  br i1 %69, label %pred.store.if264, label %pred.store.continue265

pred.store.if264:                                 ; preds = %pred.store.continue263
  %70 = extractelement <8 x i16> %wide.load, i64 5
  %71 = add i16 %70, %16
  store i16 %71, ptr %next.gep244, align 2, !tbaa !42
  br label %pred.store.continue265

pred.store.continue265:                           ; preds = %pred.store.if264, %pred.store.continue263
  %72 = extractelement <8 x i1> %52, i64 6
  br i1 %72, label %pred.store.if266, label %pred.store.continue267

pred.store.if266:                                 ; preds = %pred.store.continue265
  %73 = extractelement <8 x i16> %wide.load, i64 6
  %74 = add i16 %73, %16
  store i16 %74, ptr %next.gep245, align 2, !tbaa !42
  br label %pred.store.continue267

pred.store.continue267:                           ; preds = %pred.store.if266, %pred.store.continue265
  %75 = extractelement <8 x i1> %52, i64 7
  br i1 %75, label %pred.store.if268, label %pred.store.continue269

pred.store.if268:                                 ; preds = %pred.store.continue267
  %76 = extractelement <8 x i16> %wide.load, i64 7
  %77 = add i16 %76, %16
  store i16 %77, ptr %next.gep246, align 2, !tbaa !42
  br label %pred.store.continue269

pred.store.continue269:                           ; preds = %pred.store.if268, %pred.store.continue267
  %78 = extractelement <8 x i1> %53, i64 0
  br i1 %78, label %pred.store.if270, label %pred.store.continue271

pred.store.if270:                                 ; preds = %pred.store.continue269
  %79 = extractelement <8 x i16> %wide.load255, i64 0
  %80 = add i16 %79, %16
  store i16 %80, ptr %next.gep247, align 2, !tbaa !42
  br label %pred.store.continue271

pred.store.continue271:                           ; preds = %pred.store.if270, %pred.store.continue269
  %81 = extractelement <8 x i1> %53, i64 1
  br i1 %81, label %pred.store.if272, label %pred.store.continue273

pred.store.if272:                                 ; preds = %pred.store.continue271
  %82 = extractelement <8 x i16> %wide.load255, i64 1
  %83 = add i16 %82, %16
  store i16 %83, ptr %next.gep248, align 2, !tbaa !42
  br label %pred.store.continue273

pred.store.continue273:                           ; preds = %pred.store.if272, %pred.store.continue271
  %84 = extractelement <8 x i1> %53, i64 2
  br i1 %84, label %pred.store.if274, label %pred.store.continue275

pred.store.if274:                                 ; preds = %pred.store.continue273
  %85 = extractelement <8 x i16> %wide.load255, i64 2
  %86 = add i16 %85, %16
  store i16 %86, ptr %next.gep249, align 2, !tbaa !42
  br label %pred.store.continue275

pred.store.continue275:                           ; preds = %pred.store.if274, %pred.store.continue273
  %87 = extractelement <8 x i1> %53, i64 3
  br i1 %87, label %pred.store.if276, label %pred.store.continue277

pred.store.if276:                                 ; preds = %pred.store.continue275
  %88 = extractelement <8 x i16> %wide.load255, i64 3
  %89 = add i16 %88, %16
  store i16 %89, ptr %next.gep250, align 2, !tbaa !42
  br label %pred.store.continue277

pred.store.continue277:                           ; preds = %pred.store.if276, %pred.store.continue275
  %90 = extractelement <8 x i1> %53, i64 4
  br i1 %90, label %pred.store.if278, label %pred.store.continue279

pred.store.if278:                                 ; preds = %pred.store.continue277
  %91 = extractelement <8 x i16> %wide.load255, i64 4
  %92 = add i16 %91, %16
  store i16 %92, ptr %next.gep251, align 2, !tbaa !42
  br label %pred.store.continue279

pred.store.continue279:                           ; preds = %pred.store.if278, %pred.store.continue277
  %93 = extractelement <8 x i1> %53, i64 5
  br i1 %93, label %pred.store.if280, label %pred.store.continue281

pred.store.if280:                                 ; preds = %pred.store.continue279
  %94 = extractelement <8 x i16> %wide.load255, i64 5
  %95 = add i16 %94, %16
  store i16 %95, ptr %next.gep252, align 2, !tbaa !42
  br label %pred.store.continue281

pred.store.continue281:                           ; preds = %pred.store.if280, %pred.store.continue279
  %96 = extractelement <8 x i1> %53, i64 6
  br i1 %96, label %pred.store.if282, label %pred.store.continue283

pred.store.if282:                                 ; preds = %pred.store.continue281
  %97 = extractelement <8 x i16> %wide.load255, i64 6
  %98 = add i16 %97, %16
  store i16 %98, ptr %next.gep253, align 2, !tbaa !42
  br label %pred.store.continue283

pred.store.continue283:                           ; preds = %pred.store.if282, %pred.store.continue281
  %99 = extractelement <8 x i1> %53, i64 7
  br i1 %99, label %pred.store.if284, label %pred.store.continue285

pred.store.if284:                                 ; preds = %pred.store.continue283
  %100 = extractelement <8 x i16> %wide.load255, i64 7
  %101 = add i16 %100, %16
  store i16 %101, ptr %next.gep254, align 2, !tbaa !42
  br label %pred.store.continue285

pred.store.continue285:                           ; preds = %pred.store.if284, %pred.store.continue283
  %index.next = add nuw i64 %index239, 16
  %102 = icmp eq i64 %index.next, %n.vec
  br i1 %102, label %middle.block, label %vector.body, !llvm.loop !58

middle.block:                                     ; preds = %pred.store.continue285
  %cmp.n = icmp eq i64 %n.vec, %18
  br i1 %cmp.n, label %for.end, label %for.body.preheader310

for.body.preheader310:                            ; preds = %for.body.preheader, %middle.block
  %dec198.in.ph = phi i16 [ %3, %for.body.preheader ], [ %ind.end, %middle.block ]
  %ip.0197.ph = phi ptr [ %linp.le, %for.body.preheader ], [ %ind.end237, %middle.block ]
  br label %for.body

for.body:                                         ; preds = %for.body.preheader310, %for.inc
  %dec198.in = phi i16 [ %dec198, %for.inc ], [ %dec198.in.ph, %for.body.preheader310 ]
  %ip.0197 = phi ptr [ %incdec.ptr64, %for.inc ], [ %ip.0197.ph, %for.body.preheader310 ]
  %dec198 = add i16 %dec198.in, -1
  %103 = load i16, ptr %ip.0197, align 2, !tbaa !42
  %cmp56 = icmp ult i16 %103, %17
  br i1 %cmp56, label %if.then58, label %for.inc

if.then58:                                        ; preds = %for.body
  %conv62 = add i16 %103, %16
  store i16 %conv62, ptr %ip.0197, align 2, !tbaa !42
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then58
  %incdec.ptr64 = getelementptr inbounds i16, ptr %ip.0197, i64 1
  %tobool52.not = icmp eq i16 %dec198, 0
  br i1 %tobool52.not, label %for.end, label %for.body, !llvm.loop !61

for.end:                                          ; preds = %for.inc, %middle.block, %if.else31
  %ip.0.lcssa = phi ptr [ %linp.le, %if.else31 ], [ %ind.end237, %middle.block ], [ %incdec.ptr64, %for.inc ]
  %104 = load i16, ptr %lower.le224, align 4, !tbaa !25
  %conv66 = zext i16 %104 to i32
  %sub67 = add nuw nsw i32 %conv66, 131052
  %div68175 = lshr i32 %sub67, 1
  %105 = trunc i32 %div68175 to i16
  %106 = xor i16 %3, -1
  %dec73199 = add i16 %106, %105
  %tobool74.not200 = icmp eq i16 %dec73199, 0
  br i1 %tobool74.not200, label %for.end94, label %for.body75.preheader

for.body75.preheader:                             ; preds = %for.end
  %107 = add i16 %105, -2
  %108 = sub i16 %107, %3
  %109 = zext i16 %108 to i64
  %110 = add nuw nsw i64 %109, 1
  %min.iters.check288 = icmp ult i16 %108, 15
  br i1 %min.iters.check288, label %for.body75.preheader309, label %vector.ph289

vector.ph289:                                     ; preds = %for.body75.preheader
  %n.vec291 = and i64 %110, 131056
  %.cast292 = trunc i64 %n.vec291 to i16
  %ind.end293 = sub i16 %dec73199, %.cast292
  %111 = shl nuw nsw i64 %n.vec291, 1
  %ind.end295 = getelementptr i8, ptr %ip.0.lcssa, i64 %111
  %broadcast.splatinsert304 = insertelement <8 x i16> poison, i16 %17, i64 0
  %broadcast.splat305 = shufflevector <8 x i16> %broadcast.splatinsert304, <8 x i16> poison, <8 x i32> zeroinitializer
  %broadcast.splatinsert306 = insertelement <8 x i16> poison, i16 %16, i64 0
  %broadcast.splat307 = shufflevector <8 x i16> %broadcast.splatinsert306, <8 x i16> poison, <8 x i32> zeroinitializer
  br label %vector.body298

vector.body298:                                   ; preds = %vector.body298, %vector.ph289
  %index299 = phi i64 [ 0, %vector.ph289 ], [ %index.next308, %vector.body298 ]
  %112 = shl i64 %index299, 1
  %next.gep300 = getelementptr i8, ptr %ip.0.lcssa, i64 %112
  %113 = getelementptr inbounds i16, ptr %next.gep300, i64 1
  %114 = getelementptr inbounds i16, ptr %next.gep300, i64 9
  %wide.load302 = load <8 x i16>, ptr %113, align 2, !tbaa !42
  %wide.load303 = load <8 x i16>, ptr %114, align 2, !tbaa !42
  %115 = icmp ult <8 x i16> %wide.load302, %broadcast.splat305
  %116 = icmp ult <8 x i16> %wide.load303, %broadcast.splat305
  %117 = select <8 x i1> %115, <8 x i16> %broadcast.splat307, <8 x i16> zeroinitializer
  %118 = select <8 x i1> %116, <8 x i16> %broadcast.splat307, <8 x i16> zeroinitializer
  %119 = add <8 x i16> %117, %wide.load302
  %120 = add <8 x i16> %118, %wide.load303
  %121 = getelementptr i16, ptr %next.gep300, i64 8
  store <8 x i16> %119, ptr %next.gep300, align 2, !tbaa !42
  store <8 x i16> %120, ptr %121, align 2, !tbaa !42
  %index.next308 = add nuw i64 %index299, 16
  %122 = icmp eq i64 %index.next308, %n.vec291
  br i1 %122, label %middle.block286, label %vector.body298, !llvm.loop !62

middle.block286:                                  ; preds = %vector.body298
  %cmp.n297 = icmp eq i64 %110, %n.vec291
  br i1 %cmp.n297, label %for.end94.loopexit, label %for.body75.preheader309

for.body75.preheader309:                          ; preds = %for.body75.preheader, %middle.block286
  %dec73202.ph = phi i16 [ %dec73199, %for.body75.preheader ], [ %ind.end293, %middle.block286 ]
  %ip.1201.ph = phi ptr [ %ip.0.lcssa, %for.body75.preheader ], [ %ind.end295, %middle.block286 ]
  br label %for.body75

for.body75:                                       ; preds = %for.body75.preheader309, %for.body75
  %dec73202 = phi i16 [ %dec73, %for.body75 ], [ %dec73202.ph, %for.body75.preheader309 ]
  %ip.1201 = phi ptr [ %arrayidx76, %for.body75 ], [ %ip.1201.ph, %for.body75.preheader309 ]
  %arrayidx76 = getelementptr inbounds i16, ptr %ip.1201, i64 1
  %123 = load i16, ptr %arrayidx76, align 2, !tbaa !42
  %cmp79 = icmp ult i16 %123, %17
  %124 = select i1 %cmp79, i16 %16, i16 0
  %conv90 = add i16 %124, %123
  store i16 %conv90, ptr %ip.1201, align 2, !tbaa !42
  %dec73 = add i16 %dec73202, -1
  %tobool74.not = icmp eq i16 %dec73, 0
  br i1 %tobool74.not, label %for.end94.loopexit, label %for.body75, !llvm.loop !63

for.end94.loopexit:                               ; preds = %for.body75, %middle.block286
  %.pre = load i16, ptr %lower.le224, align 4, !tbaa !25
  br label %for.end94

for.end94:                                        ; preds = %for.end94.loopexit, %for.end
  %125 = phi i16 [ %.pre, %for.end94.loopexit ], [ %104, %for.end ]
  %sub97 = add i16 %125, -2
  store i16 %sub97, ptr %lower.le224, align 4, !tbaa !25
  br label %if.end99

if.end99:                                         ; preds = %for.end94, %if.then20
  %126 = load ptr, ptr %t, align 8, !tbaa !19
  %call101 = tail call i32 @mpool_put(ptr noundef %126, ptr noundef nonnull %call, i32 noundef 1) #4
  br label %while.end

while.end:                                        ; preds = %while.cond, %if.end99
  %127 = load i32, ptr %h, align 4, !tbaa !36
  %cmp103 = icmp eq i32 %127, 1
  br i1 %cmp103, label %if.then105, label %if.end108

if.then105:                                       ; preds = %while.end
  %128 = load ptr, ptr %t, align 8, !tbaa !19
  %call107 = tail call i32 @mpool_put(ptr noundef %128, ptr noundef nonnull %h, i32 noundef 1) #4
  br label %cleanup

if.end108:                                        ; preds = %while.end
  %call109 = tail call fastcc i32 @ref___bt_relink(ptr noundef nonnull %t, ptr noundef nonnull %h), !range !24
  %tobool110.not = icmp eq i32 %call109, 0
  br i1 %tobool110.not, label %lor.rhs, label %cleanup

lor.rhs:                                          ; preds = %if.end108
  %call111 = tail call i32 @__bt_free(ptr noundef nonnull %t, ptr noundef nonnull %h) #4
  %tobool112 = icmp ne i32 %call111, 0
  %129 = zext i1 %tobool112 to i32
  br label %cleanup

cleanup:                                          ; preds = %if.else, %lor.lhs.false, %while.body, %if.end108, %lor.rhs, %if.then105, %if.then9
  %retval.0 = phi i32 [ -1, %if.then9 ], [ 0, %if.then105 ], [ 1, %if.end108 ], [ %129, %lor.rhs ], [ -1, %while.body ], [ -1, %lor.lhs.false ], [ -1, %if.else ]
  ret i32 %retval.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nounwind uwtable
define dso_local noundef i32 @ref___bt_dleaf(ptr noundef %t, ptr noundef %key, ptr noundef %h, i32 noundef %idx) local_unnamed_addr #0 {
entry:
  %e.i = alloca %struct._epg, align 8
  %bt_cursor = getelementptr inbounds %struct._btree, ptr %t, i64 0, i32 4
  %flags = getelementptr inbounds %struct._btree, ptr %t, i64 0, i32 4, i32 3
  %0 = load i8, ptr %flags, align 4, !tbaa !31
  %1 = and i8 %0, 9
  %or.cond = icmp eq i8 %1, 8
  br i1 %or.cond, label %land.lhs.true6, label %if.end

land.lhs.true6:                                   ; preds = %entry
  %2 = load i32, ptr %bt_cursor, align 8, !tbaa !64
  %3 = load i32, ptr %h, align 4, !tbaa !36
  %cmp = icmp eq i32 %2, %3
  br i1 %cmp, label %land.lhs.true10, label %if.end

land.lhs.true10:                                  ; preds = %land.lhs.true6
  %index = getelementptr inbounds %struct._btree, ptr %t, i64 0, i32 4, i32 0, i32 1
  %4 = load i16, ptr %index, align 4, !tbaa !65
  %conv13 = zext i16 %4 to i32
  %cmp14 = icmp eq i32 %conv13, %idx
  br i1 %cmp14, label %land.lhs.true16, label %if.end

land.lhs.true16:                                  ; preds = %land.lhs.true10
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %e.i) #4
  %5 = and i8 %0, -8
  store i8 %5, ptr %flags, align 4, !tbaa !31
  %flags2.i = getelementptr inbounds %struct._btree, ptr %t, i64 0, i32 28
  %6 = load i32, ptr %flags2.i, align 4, !tbaa !20
  %and3.i = and i32 %6, 32
  %tobool.not.i = icmp eq i32 %and3.i, 0
  br i1 %tobool.not.i, label %if.then.i, label %if.end121.thread.i

if.end121.thread.i:                               ; preds = %land.lhs.true16
  store ptr %h, ptr %e.i, align 8, !tbaa !22
  %conv123196.i = trunc i32 %idx to i16
  %index124197.i = getelementptr inbounds %struct._epg, ptr %e.i, i64 0, i32 1
  store i16 %conv123196.i, ptr %index124197.i, align 8, !tbaa !23
  br label %lor.lhs.false.i

if.then.i:                                        ; preds = %land.lhs.true16
  %cmp.not.i = icmp eq ptr %key, null
  br i1 %cmp.not.i, label %if.then5.i, label %if.end13.i

if.then5.i:                                       ; preds = %if.then.i
  store ptr %h, ptr %e.i, align 8, !tbaa !22
  %conv6.i = trunc i32 %idx to i16
  %index.i = getelementptr inbounds %struct._epg, ptr %e.i, i64 0, i32 1
  store i16 %conv6.i, ptr %index.i, align 8, !tbaa !23
  %key7.i = getelementptr inbounds %struct._btree, ptr %t, i64 0, i32 4, i32 1
  %call.i = call i32 @__bt_ret(ptr noundef nonnull %t, ptr noundef nonnull %e.i, ptr noundef nonnull %key7.i, ptr noundef nonnull %key7.i, ptr noundef null, ptr noundef null, i32 noundef 1) #4
  %cmp9.not.i = icmp eq i32 %call.i, 0
  br i1 %cmp9.not.i, label %if.end13.i, label %ref___bt_curdel.exit.thread

if.end13.i:                                       ; preds = %if.then5.i, %if.then.i
  %key.addr.0.i = phi ptr [ %key, %if.then.i ], [ %key7.i, %if.then5.i ]
  %cmp14.not.i = icmp eq i32 %idx, 0
  br i1 %cmp14.not.i, label %if.end28.i, label %if.then16.i

if.then16.i:                                      ; preds = %if.end13.i
  store ptr %h, ptr %e.i, align 8, !tbaa !22
  %7 = trunc i32 %idx to i16
  %conv18.i = add i16 %7, -1
  %index19.i = getelementptr inbounds %struct._epg, ptr %e.i, i64 0, i32 1
  store i16 %conv18.i, ptr %index19.i, align 8, !tbaa !23
  %call20.i = call i32 @__bt_cmp(ptr noundef nonnull %t, ptr noundef nonnull %key.addr.0.i, ptr noundef nonnull %e.i) #4
  %cmp21.i = icmp eq i32 %call20.i, 0
  br i1 %cmp21.i, label %if.then23.i, label %if.end28.i

if.then23.i:                                      ; preds = %if.then16.i
  %8 = load i8, ptr %flags, align 4, !tbaa !31
  %9 = or i8 %8, 4
  store i8 %9, ptr %flags, align 4, !tbaa !31
  br label %dup2.i

if.end28.i:                                       ; preds = %if.then16.i, %if.end13.i
  %conv29.i = zext nneg i32 %idx to i64
  %lower.i = getelementptr inbounds %struct._page, ptr %h, i64 0, i32 4
  %10 = load i16, ptr %lower.i, align 4, !tbaa !25
  %conv30.i = zext i16 %10 to i64
  %sub31.i = add nsw i64 %conv30.i, -20
  %div192.i = lshr i64 %sub31.i, 1
  %sub32.i = add nsw i64 %div192.i, -1
  %cmp33.i = icmp ugt i64 %sub32.i, %conv29.i
  br i1 %cmp33.i, label %if.then35.i, label %if.end48.i

if.then35.i:                                      ; preds = %if.end28.i
  store ptr %h, ptr %e.i, align 8, !tbaa !22
  %11 = trunc i32 %idx to i16
  %conv37.i = add i16 %11, 1
  %index38.i = getelementptr inbounds %struct._epg, ptr %e.i, i64 0, i32 1
  store i16 %conv37.i, ptr %index38.i, align 8, !tbaa !23
  %call39.i = call i32 @__bt_cmp(ptr noundef nonnull %t, ptr noundef nonnull %key.addr.0.i, ptr noundef nonnull %e.i) #4
  %cmp40.i = icmp eq i32 %call39.i, 0
  br i1 %cmp40.i, label %if.then42.i, label %if.end48.i

if.then42.i:                                      ; preds = %if.then35.i
  %12 = load i8, ptr %flags, align 4, !tbaa !31
  %13 = or i8 %12, 2
  store i8 %13, ptr %flags, align 4, !tbaa !31
  br label %dup2.i

if.end48.i:                                       ; preds = %if.then35.i, %if.end28.i
  br i1 %cmp14.not.i, label %land.lhs.true.i, label %if.end79.i

land.lhs.true.i:                                  ; preds = %if.end48.i
  %prevpg.i = getelementptr inbounds %struct._page, ptr %h, i64 0, i32 1
  %14 = load i32, ptr %prevpg.i, align 4, !tbaa !48
  %cmp51.not.i = icmp eq i32 %14, 0
  br i1 %cmp51.not.i, label %if.end79.i, label %if.then53.i

if.then53.i:                                      ; preds = %land.lhs.true.i
  %15 = load ptr, ptr %t, align 8, !tbaa !19
  %call55.i = call ptr @mpool_get(ptr noundef %15, i32 noundef %14, i32 noundef 0) #4
  %cmp56.i = icmp eq ptr %call55.i, null
  br i1 %cmp56.i, label %ref___bt_curdel.exit.thread, label %if.end59.i

if.end59.i:                                       ; preds = %if.then53.i
  store ptr %call55.i, ptr %e.i, align 8, !tbaa !22
  %lower61.i = getelementptr inbounds %struct._page, ptr %call55.i, i64 0, i32 4
  %16 = load i16, ptr %lower61.i, align 4, !tbaa !25
  %conv62.i = zext i16 %16 to i32
  %sub63.i = add nuw nsw i32 %conv62.i, 131052
  %div64193.i = lshr i32 %sub63.i, 1
  %17 = trunc i32 %div64193.i to i16
  %conv66.i = add nsw i16 %17, -1
  %index67.i = getelementptr inbounds %struct._epg, ptr %e.i, i64 0, i32 1
  store i16 %conv66.i, ptr %index67.i, align 8, !tbaa !23
  %call68.i = call i32 @__bt_cmp(ptr noundef nonnull %t, ptr noundef nonnull %key.addr.0.i, ptr noundef nonnull %e.i) #4
  %cmp69.i = icmp eq i32 %call68.i, 0
  br i1 %cmp69.i, label %dup1.i, label %if.end76.i

if.end76.i:                                       ; preds = %if.end59.i
  %18 = load ptr, ptr %t, align 8, !tbaa !19
  %call78.i = call i32 @mpool_put(ptr noundef %18, ptr noundef nonnull %call55.i, i32 noundef 0) #4
  br label %if.end79.i

if.end79.i:                                       ; preds = %if.end76.i, %land.lhs.true.i, %if.end48.i
  %19 = load i16, ptr %lower.i, align 4, !tbaa !25
  %conv82.i = zext i16 %19 to i64
  %sub83.i = add nsw i64 %conv82.i, -20
  %div84194.i = lshr i64 %sub83.i, 1
  %sub85.i = add nsw i64 %div84194.i, -1
  %cmp86.i = icmp eq i64 %sub85.i, %conv29.i
  br i1 %cmp86.i, label %land.lhs.true88.i, label %if.end121.i

land.lhs.true88.i:                                ; preds = %if.end79.i
  %nextpg.i = getelementptr inbounds %struct._page, ptr %h, i64 0, i32 2
  %20 = load i32, ptr %nextpg.i, align 4, !tbaa !37
  %cmp89.not.i = icmp eq i32 %20, 0
  br i1 %cmp89.not.i, label %if.end121.i, label %if.then91.i

if.then91.i:                                      ; preds = %land.lhs.true88.i
  %21 = load ptr, ptr %t, align 8, !tbaa !19
  %call94.i = call ptr @mpool_get(ptr noundef %21, i32 noundef %20, i32 noundef 0) #4
  %cmp95.i = icmp eq ptr %call94.i, null
  br i1 %cmp95.i, label %ref___bt_curdel.exit.thread, label %if.end98.i

if.end98.i:                                       ; preds = %if.then91.i
  store ptr %call94.i, ptr %e.i, align 8, !tbaa !22
  %index100.i = getelementptr inbounds %struct._epg, ptr %e.i, i64 0, i32 1
  store i16 0, ptr %index100.i, align 8, !tbaa !23
  %call101.i = call i32 @__bt_cmp(ptr noundef nonnull %t, ptr noundef nonnull %key.addr.0.i, ptr noundef nonnull %e.i) #4
  %cmp102.i = icmp eq i32 %call101.i, 0
  br i1 %cmp102.i, label %dup1.i, label %if.end117.i

dup1.i:                                           ; preds = %if.end98.i, %if.end59.i
  %.sink199.i = phi i8 [ 4, %if.end59.i ], [ 2, %if.end98.i ]
  %pg.0.i = phi ptr [ %call55.i, %if.end59.i ], [ %call94.i, %if.end98.i ]
  %22 = load i8, ptr %flags, align 4, !tbaa !31
  %23 = or i8 %22, %.sink199.i
  store i8 %23, ptr %flags, align 4, !tbaa !31
  %24 = load ptr, ptr %t, align 8, !tbaa !19
  %call110.i = call i32 @mpool_put(ptr noundef %24, ptr noundef nonnull %pg.0.i, i32 noundef 0) #4
  br label %dup2.i

dup2.i:                                           ; preds = %dup1.i, %if.then42.i, %if.then23.i
  %25 = load ptr, ptr %e.i, align 8, !tbaa !22
  %26 = load i32, ptr %25, align 4, !tbaa !36
  store i32 %26, ptr %bt_cursor, align 8, !tbaa !32
  %index114.i = getelementptr inbounds %struct._epg, ptr %e.i, i64 0, i32 1
  %27 = load i16, ptr %index114.i, align 8, !tbaa !23
  store i16 %27, ptr %index, align 4, !tbaa !35
  br label %ref___bt_curdel.exit

if.end117.i:                                      ; preds = %if.end98.i
  %28 = load ptr, ptr %t, align 8, !tbaa !19
  %call119.i = call i32 @mpool_put(ptr noundef %28, ptr noundef nonnull %call94.i, i32 noundef 0) #4
  br label %if.end121.i

if.end121.i:                                      ; preds = %if.end117.i, %land.lhs.true88.i, %if.end79.i
  store ptr %h, ptr %e.i, align 8, !tbaa !22
  %conv123.i = trunc i32 %idx to i16
  %index124.i = getelementptr inbounds %struct._epg, ptr %e.i, i64 0, i32 1
  store i16 %conv123.i, ptr %index124.i, align 8, !tbaa !23
  br i1 %cmp.not.i, label %if.then131.i, label %lor.lhs.false.i

lor.lhs.false.i:                                  ; preds = %if.end121.i, %if.end121.thread.i
  %key126.i = getelementptr inbounds %struct._btree, ptr %t, i64 0, i32 4, i32 1
  %call128.i = call i32 @__bt_ret(ptr noundef nonnull %t, ptr noundef nonnull %e.i, ptr noundef nonnull %key126.i, ptr noundef nonnull %key126.i, ptr noundef null, ptr noundef null, i32 noundef 1) #4
  %cmp129.i = icmp eq i32 %call128.i, 0
  br i1 %cmp129.i, label %if.then131.i, label %ref___bt_curdel.exit.thread

if.then131.i:                                     ; preds = %lor.lhs.false.i, %if.end121.i
  %29 = load i8, ptr %flags, align 4, !tbaa !31
  %30 = or i8 %29, 1
  store i8 %30, ptr %flags, align 4, !tbaa !31
  br label %ref___bt_curdel.exit

ref___bt_curdel.exit.thread:                      ; preds = %if.then5.i, %if.then53.i, %if.then91.i, %lor.lhs.false.i
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %e.i) #4
  br label %cleanup

ref___bt_curdel.exit:                             ; preds = %dup2.i, %if.then131.i
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %e.i) #4
  br label %if.end

if.end:                                           ; preds = %ref___bt_curdel.exit, %land.lhs.true10, %land.lhs.true6, %entry
  %linp = getelementptr inbounds %struct._page, ptr %h, i64 0, i32 6
  %idxprom = zext i32 %idx to i64
  %arrayidx = getelementptr inbounds %struct._page, ptr %h, i64 0, i32 6, i64 %idxprom
  %31 = load i16, ptr %arrayidx, align 2, !tbaa !42
  %idx.ext = zext i16 %31 to i64
  %add.ptr = getelementptr inbounds i8, ptr %h, i64 %idx.ext
  %flags19 = getelementptr inbounds %struct._bleaf, ptr %add.ptr, i64 0, i32 2
  %32 = load i8, ptr %flags19, align 4, !tbaa !66
  %33 = and i8 %32, 2
  %tobool22.not = icmp eq i8 %33, 0
  br i1 %tobool22.not, label %if.end28, label %land.lhs.true23

land.lhs.true23:                                  ; preds = %if.end
  %bytes = getelementptr inbounds %struct._bleaf, ptr %add.ptr, i64 0, i32 3
  %call24 = call i32 @__ovfl_delete(ptr noundef nonnull %t, ptr noundef nonnull %bytes) #4
  %cmp25 = icmp eq i32 %call24, -1
  br i1 %cmp25, label %cleanup, label %land.lhs.true23.if.end28_crit_edge

land.lhs.true23.if.end28_crit_edge:               ; preds = %land.lhs.true23
  %.pre = load i8, ptr %flags19, align 4, !tbaa !66
  br label %if.end28

if.end28:                                         ; preds = %land.lhs.true23.if.end28_crit_edge, %if.end
  %34 = phi i8 [ %.pre, %land.lhs.true23.if.end28_crit_edge ], [ %32, %if.end ]
  %35 = and i8 %34, 1
  %tobool32.not = icmp eq i8 %35, 0
  br i1 %tobool32.not, label %if.end42, label %land.lhs.true33

land.lhs.true33:                                  ; preds = %if.end28
  %bytes34 = getelementptr inbounds %struct._bleaf, ptr %add.ptr, i64 0, i32 3
  %36 = load i32, ptr %add.ptr, align 4, !tbaa !68
  %idx.ext36 = zext i32 %36 to i64
  %add.ptr37 = getelementptr inbounds i8, ptr %bytes34, i64 %idx.ext36
  %call38 = call i32 @__ovfl_delete(ptr noundef nonnull %t, ptr noundef nonnull %add.ptr37) #4
  %cmp39 = icmp eq i32 %call38, -1
  br i1 %cmp39, label %cleanup, label %if.end42

if.end42:                                         ; preds = %land.lhs.true33, %if.end28
  %37 = load i32, ptr %add.ptr, align 4, !tbaa !68
  %dsize = getelementptr inbounds %struct._bleaf, ptr %add.ptr, i64 0, i32 1
  %38 = load i32, ptr %dsize, align 4, !tbaa !69
  %add46 = add i32 %37, 12
  %sub = add i32 %add46, %38
  %and48 = and i32 %sub, -4
  %upper = getelementptr inbounds %struct._page, ptr %h, i64 0, i32 5
  %39 = load i16, ptr %upper, align 2, !tbaa !54
  %idx.ext51 = zext i16 %39 to i64
  %add.ptr52 = getelementptr inbounds i8, ptr %h, i64 %idx.ext51
  %idx.ext53 = zext i32 %and48 to i64
  %add.ptr54 = getelementptr inbounds i8, ptr %add.ptr52, i64 %idx.ext53
  %sub.ptr.lhs.cast = ptrtoint ptr %add.ptr to i64
  %sub.ptr.rhs.cast = ptrtoint ptr %add.ptr52 to i64
  %sub.ptr.sub = sub i64 %sub.ptr.lhs.cast, %sub.ptr.rhs.cast
  call void @llvm.memmove.p0.p0.i64(ptr align 1 %add.ptr54, ptr align 1 %add.ptr52, i64 %sub.ptr.sub, i1 false)
  %40 = load i16, ptr %upper, align 2, !tbaa !54
  %41 = trunc i32 %and48 to i16
  %conv58 = add i16 %40, %41
  store i16 %conv58, ptr %upper, align 2, !tbaa !54
  %42 = load i16, ptr %arrayidx, align 2, !tbaa !42
  %conv62 = trunc i32 %idx to i16
  %tobool65.not195 = icmp eq i16 %conv62, 0
  br i1 %tobool65.not195, label %for.end, label %for.body.preheader

for.body.preheader:                               ; preds = %if.end42
  %conv62.mask = and i32 %idx, 65535
  %43 = zext nneg i32 %conv62.mask to i64
  %min.iters.check = icmp ult i32 %conv62.mask, 16
  br i1 %min.iters.check, label %for.body.preheader276, label %vector.ph

vector.ph:                                        ; preds = %for.body.preheader
  %n.vec = and i64 %43, 65520
  %.cast = trunc i64 %n.vec to i16
  %ind.end = sub i16 %conv62, %.cast
  %44 = shl nuw nsw i64 %n.vec, 1
  %ind.end203 = getelementptr i8, ptr %linp, i64 %44
  %broadcast.splatinsert = insertelement <8 x i16> poison, i16 %42, i64 0
  %broadcast.splat = shufflevector <8 x i16> %broadcast.splatinsert, <8 x i16> poison, <8 x i32> zeroinitializer
  br label %vector.body

vector.body:                                      ; preds = %pred.store.continue251, %vector.ph
  %index205 = phi i64 [ 0, %vector.ph ], [ %index.next, %pred.store.continue251 ]
  %45 = shl i64 %index205, 1
  %next.gep = getelementptr i8, ptr %linp, i64 %45
  %46 = shl i64 %index205, 1
  %47 = or disjoint i64 %46, 2
  %next.gep206 = getelementptr i8, ptr %linp, i64 %47
  %48 = shl i64 %index205, 1
  %49 = or disjoint i64 %48, 4
  %next.gep207 = getelementptr i8, ptr %linp, i64 %49
  %50 = shl i64 %index205, 1
  %51 = or disjoint i64 %50, 6
  %next.gep208 = getelementptr i8, ptr %linp, i64 %51
  %52 = shl i64 %index205, 1
  %53 = or disjoint i64 %52, 8
  %next.gep209 = getelementptr i8, ptr %linp, i64 %53
  %54 = shl i64 %index205, 1
  %55 = or disjoint i64 %54, 10
  %next.gep210 = getelementptr i8, ptr %linp, i64 %55
  %56 = shl i64 %index205, 1
  %57 = or disjoint i64 %56, 12
  %next.gep211 = getelementptr i8, ptr %linp, i64 %57
  %58 = shl i64 %index205, 1
  %59 = or disjoint i64 %58, 14
  %next.gep212 = getelementptr i8, ptr %linp, i64 %59
  %60 = shl i64 %index205, 1
  %61 = or disjoint i64 %60, 16
  %next.gep213 = getelementptr i8, ptr %linp, i64 %61
  %62 = shl i64 %index205, 1
  %63 = or disjoint i64 %62, 18
  %next.gep214 = getelementptr i8, ptr %linp, i64 %63
  %64 = shl i64 %index205, 1
  %65 = or disjoint i64 %64, 20
  %next.gep215 = getelementptr i8, ptr %linp, i64 %65
  %66 = shl i64 %index205, 1
  %67 = or disjoint i64 %66, 22
  %next.gep216 = getelementptr i8, ptr %linp, i64 %67
  %68 = shl i64 %index205, 1
  %69 = or disjoint i64 %68, 24
  %next.gep217 = getelementptr i8, ptr %linp, i64 %69
  %70 = shl i64 %index205, 1
  %71 = or disjoint i64 %70, 26
  %next.gep218 = getelementptr i8, ptr %linp, i64 %71
  %72 = shl i64 %index205, 1
  %73 = or disjoint i64 %72, 28
  %next.gep219 = getelementptr i8, ptr %linp, i64 %73
  %74 = shl i64 %index205, 1
  %75 = or disjoint i64 %74, 30
  %next.gep220 = getelementptr i8, ptr %linp, i64 %75
  %76 = getelementptr i16, ptr %next.gep, i64 8
  %wide.load = load <8 x i16>, ptr %next.gep, align 2, !tbaa !42
  %wide.load221 = load <8 x i16>, ptr %76, align 2, !tbaa !42
  %77 = icmp ult <8 x i16> %wide.load, %broadcast.splat
  %78 = icmp ult <8 x i16> %wide.load221, %broadcast.splat
  %79 = extractelement <8 x i1> %77, i64 0
  br i1 %79, label %pred.store.if, label %pred.store.continue

pred.store.if:                                    ; preds = %vector.body
  %80 = extractelement <8 x i16> %wide.load, i64 0
  %81 = add i16 %80, %41
  store i16 %81, ptr %next.gep, align 2, !tbaa !42
  br label %pred.store.continue

pred.store.continue:                              ; preds = %pred.store.if, %vector.body
  %82 = extractelement <8 x i1> %77, i64 1
  br i1 %82, label %pred.store.if222, label %pred.store.continue223

pred.store.if222:                                 ; preds = %pred.store.continue
  %83 = extractelement <8 x i16> %wide.load, i64 1
  %84 = add i16 %83, %41
  store i16 %84, ptr %next.gep206, align 2, !tbaa !42
  br label %pred.store.continue223

pred.store.continue223:                           ; preds = %pred.store.if222, %pred.store.continue
  %85 = extractelement <8 x i1> %77, i64 2
  br i1 %85, label %pred.store.if224, label %pred.store.continue225

pred.store.if224:                                 ; preds = %pred.store.continue223
  %86 = extractelement <8 x i16> %wide.load, i64 2
  %87 = add i16 %86, %41
  store i16 %87, ptr %next.gep207, align 2, !tbaa !42
  br label %pred.store.continue225

pred.store.continue225:                           ; preds = %pred.store.if224, %pred.store.continue223
  %88 = extractelement <8 x i1> %77, i64 3
  br i1 %88, label %pred.store.if226, label %pred.store.continue227

pred.store.if226:                                 ; preds = %pred.store.continue225
  %89 = extractelement <8 x i16> %wide.load, i64 3
  %90 = add i16 %89, %41
  store i16 %90, ptr %next.gep208, align 2, !tbaa !42
  br label %pred.store.continue227

pred.store.continue227:                           ; preds = %pred.store.if226, %pred.store.continue225
  %91 = extractelement <8 x i1> %77, i64 4
  br i1 %91, label %pred.store.if228, label %pred.store.continue229

pred.store.if228:                                 ; preds = %pred.store.continue227
  %92 = extractelement <8 x i16> %wide.load, i64 4
  %93 = add i16 %92, %41
  store i16 %93, ptr %next.gep209, align 2, !tbaa !42
  br label %pred.store.continue229

pred.store.continue229:                           ; preds = %pred.store.if228, %pred.store.continue227
  %94 = extractelement <8 x i1> %77, i64 5
  br i1 %94, label %pred.store.if230, label %pred.store.continue231

pred.store.if230:                                 ; preds = %pred.store.continue229
  %95 = extractelement <8 x i16> %wide.load, i64 5
  %96 = add i16 %95, %41
  store i16 %96, ptr %next.gep210, align 2, !tbaa !42
  br label %pred.store.continue231

pred.store.continue231:                           ; preds = %pred.store.if230, %pred.store.continue229
  %97 = extractelement <8 x i1> %77, i64 6
  br i1 %97, label %pred.store.if232, label %pred.store.continue233

pred.store.if232:                                 ; preds = %pred.store.continue231
  %98 = extractelement <8 x i16> %wide.load, i64 6
  %99 = add i16 %98, %41
  store i16 %99, ptr %next.gep211, align 2, !tbaa !42
  br label %pred.store.continue233

pred.store.continue233:                           ; preds = %pred.store.if232, %pred.store.continue231
  %100 = extractelement <8 x i1> %77, i64 7
  br i1 %100, label %pred.store.if234, label %pred.store.continue235

pred.store.if234:                                 ; preds = %pred.store.continue233
  %101 = extractelement <8 x i16> %wide.load, i64 7
  %102 = add i16 %101, %41
  store i16 %102, ptr %next.gep212, align 2, !tbaa !42
  br label %pred.store.continue235

pred.store.continue235:                           ; preds = %pred.store.if234, %pred.store.continue233
  %103 = extractelement <8 x i1> %78, i64 0
  br i1 %103, label %pred.store.if236, label %pred.store.continue237

pred.store.if236:                                 ; preds = %pred.store.continue235
  %104 = extractelement <8 x i16> %wide.load221, i64 0
  %105 = add i16 %104, %41
  store i16 %105, ptr %next.gep213, align 2, !tbaa !42
  br label %pred.store.continue237

pred.store.continue237:                           ; preds = %pred.store.if236, %pred.store.continue235
  %106 = extractelement <8 x i1> %78, i64 1
  br i1 %106, label %pred.store.if238, label %pred.store.continue239

pred.store.if238:                                 ; preds = %pred.store.continue237
  %107 = extractelement <8 x i16> %wide.load221, i64 1
  %108 = add i16 %107, %41
  store i16 %108, ptr %next.gep214, align 2, !tbaa !42
  br label %pred.store.continue239

pred.store.continue239:                           ; preds = %pred.store.if238, %pred.store.continue237
  %109 = extractelement <8 x i1> %78, i64 2
  br i1 %109, label %pred.store.if240, label %pred.store.continue241

pred.store.if240:                                 ; preds = %pred.store.continue239
  %110 = extractelement <8 x i16> %wide.load221, i64 2
  %111 = add i16 %110, %41
  store i16 %111, ptr %next.gep215, align 2, !tbaa !42
  br label %pred.store.continue241

pred.store.continue241:                           ; preds = %pred.store.if240, %pred.store.continue239
  %112 = extractelement <8 x i1> %78, i64 3
  br i1 %112, label %pred.store.if242, label %pred.store.continue243

pred.store.if242:                                 ; preds = %pred.store.continue241
  %113 = extractelement <8 x i16> %wide.load221, i64 3
  %114 = add i16 %113, %41
  store i16 %114, ptr %next.gep216, align 2, !tbaa !42
  br label %pred.store.continue243

pred.store.continue243:                           ; preds = %pred.store.if242, %pred.store.continue241
  %115 = extractelement <8 x i1> %78, i64 4
  br i1 %115, label %pred.store.if244, label %pred.store.continue245

pred.store.if244:                                 ; preds = %pred.store.continue243
  %116 = extractelement <8 x i16> %wide.load221, i64 4
  %117 = add i16 %116, %41
  store i16 %117, ptr %next.gep217, align 2, !tbaa !42
  br label %pred.store.continue245

pred.store.continue245:                           ; preds = %pred.store.if244, %pred.store.continue243
  %118 = extractelement <8 x i1> %78, i64 5
  br i1 %118, label %pred.store.if246, label %pred.store.continue247

pred.store.if246:                                 ; preds = %pred.store.continue245
  %119 = extractelement <8 x i16> %wide.load221, i64 5
  %120 = add i16 %119, %41
  store i16 %120, ptr %next.gep218, align 2, !tbaa !42
  br label %pred.store.continue247

pred.store.continue247:                           ; preds = %pred.store.if246, %pred.store.continue245
  %121 = extractelement <8 x i1> %78, i64 6
  br i1 %121, label %pred.store.if248, label %pred.store.continue249

pred.store.if248:                                 ; preds = %pred.store.continue247
  %122 = extractelement <8 x i16> %wide.load221, i64 6
  %123 = add i16 %122, %41
  store i16 %123, ptr %next.gep219, align 2, !tbaa !42
  br label %pred.store.continue249

pred.store.continue249:                           ; preds = %pred.store.if248, %pred.store.continue247
  %124 = extractelement <8 x i1> %78, i64 7
  br i1 %124, label %pred.store.if250, label %pred.store.continue251

pred.store.if250:                                 ; preds = %pred.store.continue249
  %125 = extractelement <8 x i16> %wide.load221, i64 7
  %126 = add i16 %125, %41
  store i16 %126, ptr %next.gep220, align 2, !tbaa !42
  br label %pred.store.continue251

pred.store.continue251:                           ; preds = %pred.store.if250, %pred.store.continue249
  %index.next = add nuw i64 %index205, 16
  %127 = icmp eq i64 %index.next, %n.vec
  br i1 %127, label %middle.block, label %vector.body, !llvm.loop !70

middle.block:                                     ; preds = %pred.store.continue251
  %cmp.n = icmp eq i64 %n.vec, %43
  br i1 %cmp.n, label %for.end, label %for.body.preheader276

for.body.preheader276:                            ; preds = %for.body.preheader, %middle.block
  %dec197.in.ph = phi i16 [ %conv62, %for.body.preheader ], [ %ind.end, %middle.block ]
  %ip.0196.ph = phi ptr [ %linp, %for.body.preheader ], [ %ind.end203, %middle.block ]
  br label %for.body

for.body:                                         ; preds = %for.body.preheader276, %for.inc
  %dec197.in = phi i16 [ %dec197, %for.inc ], [ %dec197.in.ph, %for.body.preheader276 ]
  %ip.0196 = phi ptr [ %incdec.ptr, %for.inc ], [ %ip.0196.ph, %for.body.preheader276 ]
  %dec197 = add i16 %dec197.in, -1
  %128 = load i16, ptr %ip.0196, align 2, !tbaa !42
  %cmp69 = icmp ult i16 %128, %42
  br i1 %cmp69, label %if.then71, label %for.inc

if.then71:                                        ; preds = %for.body
  %conv75 = add i16 %128, %41
  store i16 %conv75, ptr %ip.0196, align 2, !tbaa !42
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then71
  %incdec.ptr = getelementptr inbounds i16, ptr %ip.0196, i64 1
  %tobool65.not = icmp eq i16 %dec197, 0
  br i1 %tobool65.not, label %for.end, label %for.body, !llvm.loop !71

for.end:                                          ; preds = %for.inc, %middle.block, %if.end42
  %ip.0.lcssa = phi ptr [ %linp, %if.end42 ], [ %ind.end203, %middle.block ], [ %incdec.ptr, %for.inc ]
  %lower = getelementptr inbounds %struct._page, ptr %h, i64 0, i32 4
  %129 = load i16, ptr %lower, align 4, !tbaa !25
  %conv77 = zext i16 %129 to i32
  %sub78 = add nuw nsw i32 %conv77, 131052
  %div190 = lshr i32 %sub78, 1
  %sub80 = sub i32 %div190, %idx
  %conv81 = trunc i32 %sub80 to i16
  %dec83198 = add i16 %conv81, -1
  %tobool84.not199 = icmp eq i16 %dec83198, 0
  br i1 %tobool84.not199, label %for.end100, label %for.body85.preheader

for.body85.preheader:                             ; preds = %for.end
  %130 = trunc i32 %div190 to i16
  %131 = add i16 %130, -2
  %132 = sub i16 %131, %conv62
  %133 = zext i16 %132 to i64
  %134 = add nuw nsw i64 %133, 1
  %min.iters.check254 = icmp ult i16 %132, 15
  br i1 %min.iters.check254, label %for.body85.preheader275, label %vector.ph255

vector.ph255:                                     ; preds = %for.body85.preheader
  %n.vec257 = and i64 %134, 131056
  %.cast258 = trunc i64 %n.vec257 to i16
  %ind.end259 = sub i16 %dec83198, %.cast258
  %135 = shl nuw nsw i64 %n.vec257, 1
  %ind.end261 = getelementptr i8, ptr %ip.0.lcssa, i64 %135
  %broadcast.splatinsert270 = insertelement <8 x i16> poison, i16 %42, i64 0
  %broadcast.splat271 = shufflevector <8 x i16> %broadcast.splatinsert270, <8 x i16> poison, <8 x i32> zeroinitializer
  %broadcast.splatinsert272 = insertelement <8 x i16> poison, i16 %41, i64 0
  %broadcast.splat273 = shufflevector <8 x i16> %broadcast.splatinsert272, <8 x i16> poison, <8 x i32> zeroinitializer
  br label %vector.body264

vector.body264:                                   ; preds = %vector.body264, %vector.ph255
  %index265 = phi i64 [ 0, %vector.ph255 ], [ %index.next274, %vector.body264 ]
  %136 = shl i64 %index265, 1
  %next.gep266 = getelementptr i8, ptr %ip.0.lcssa, i64 %136
  %137 = getelementptr inbounds i16, ptr %next.gep266, i64 1
  %138 = getelementptr inbounds i16, ptr %next.gep266, i64 9
  %wide.load268 = load <8 x i16>, ptr %137, align 2, !tbaa !42
  %wide.load269 = load <8 x i16>, ptr %138, align 2, !tbaa !42
  %139 = icmp ult <8 x i16> %wide.load268, %broadcast.splat271
  %140 = icmp ult <8 x i16> %wide.load269, %broadcast.splat271
  %141 = select <8 x i1> %139, <8 x i16> %broadcast.splat273, <8 x i16> zeroinitializer
  %142 = select <8 x i1> %140, <8 x i16> %broadcast.splat273, <8 x i16> zeroinitializer
  %143 = add <8 x i16> %141, %wide.load268
  %144 = add <8 x i16> %142, %wide.load269
  %145 = getelementptr i16, ptr %next.gep266, i64 8
  store <8 x i16> %143, ptr %next.gep266, align 2, !tbaa !42
  store <8 x i16> %144, ptr %145, align 2, !tbaa !42
  %index.next274 = add nuw i64 %index265, 16
  %146 = icmp eq i64 %index.next274, %n.vec257
  br i1 %146, label %middle.block252, label %vector.body264, !llvm.loop !72

middle.block252:                                  ; preds = %vector.body264
  %cmp.n263 = icmp eq i64 %134, %n.vec257
  br i1 %cmp.n263, label %for.end100.loopexit, label %for.body85.preheader275

for.body85.preheader275:                          ; preds = %for.body85.preheader, %middle.block252
  %dec83201.ph = phi i16 [ %dec83198, %for.body85.preheader ], [ %ind.end259, %middle.block252 ]
  %ip.1200.ph = phi ptr [ %ip.0.lcssa, %for.body85.preheader ], [ %ind.end261, %middle.block252 ]
  br label %for.body85

for.body85:                                       ; preds = %for.body85.preheader275, %for.body85
  %dec83201 = phi i16 [ %dec83, %for.body85 ], [ %dec83201.ph, %for.body85.preheader275 ]
  %ip.1200 = phi ptr [ %arrayidx86, %for.body85 ], [ %ip.1200.ph, %for.body85.preheader275 ]
  %arrayidx86 = getelementptr inbounds i16, ptr %ip.1200, i64 1
  %147 = load i16, ptr %arrayidx86, align 2, !tbaa !42
  %cmp89 = icmp ult i16 %147, %42
  %148 = select i1 %cmp89, i16 %41, i16 0
  %conv96 = add i16 %148, %147
  store i16 %conv96, ptr %ip.1200, align 2, !tbaa !42
  %dec83 = add i16 %dec83201, -1
  %tobool84.not = icmp eq i16 %dec83, 0
  br i1 %tobool84.not, label %for.end100.loopexit, label %for.body85, !llvm.loop !73

for.end100.loopexit:                              ; preds = %for.body85, %middle.block252
  %.pre202 = load i16, ptr %lower, align 4, !tbaa !25
  br label %for.end100

for.end100:                                       ; preds = %for.end100.loopexit, %for.end
  %149 = phi i16 [ %.pre202, %for.end100.loopexit ], [ %129, %for.end ]
  %sub103 = add i16 %149, -2
  store i16 %sub103, ptr %lower, align 4, !tbaa !25
  %150 = load i8, ptr %flags, align 4, !tbaa !74
  %151 = and i8 %150, 9
  %or.cond191 = icmp eq i8 %151, 8
  br i1 %or.cond191, label %land.lhs.true116, label %cleanup

land.lhs.true116:                                 ; preds = %for.end100
  %152 = load i32, ptr %bt_cursor, align 8, !tbaa !64
  %153 = load i32, ptr %h, align 4, !tbaa !36
  %cmp121 = icmp eq i32 %152, %153
  br i1 %cmp121, label %land.lhs.true123, label %cleanup

land.lhs.true123:                                 ; preds = %land.lhs.true116
  %index126 = getelementptr inbounds %struct._btree, ptr %t, i64 0, i32 4, i32 0, i32 1
  %154 = load i16, ptr %index126, align 4, !tbaa !65
  %conv127 = zext i16 %154 to i32
  %cmp128 = icmp ugt i32 %conv127, %idx
  br i1 %cmp128, label %if.then130, label %cleanup

if.then130:                                       ; preds = %land.lhs.true123
  %dec134 = add i16 %154, -1
  store i16 %dec134, ptr %index126, align 4, !tbaa !65
  br label %cleanup

cleanup:                                          ; preds = %ref___bt_curdel.exit.thread, %for.end100, %land.lhs.true116, %land.lhs.true123, %if.then130, %land.lhs.true33, %land.lhs.true23
  %retval.0 = phi i32 [ -1, %land.lhs.true23 ], [ -1, %land.lhs.true33 ], [ 0, %if.then130 ], [ 0, %land.lhs.true123 ], [ 0, %land.lhs.true116 ], [ 0, %for.end100 ], [ -1, %ref___bt_curdel.exit.thread ]
  ret i32 %retval.0
}

declare i32 @__ovfl_delete(ptr noundef, ptr noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memmove.p0.p0.i64(ptr nocapture writeonly, ptr nocapture readonly, i64, i1 immarg) #3

declare ptr @__bt_search(ptr noundef, ptr noundef, ptr noundef) local_unnamed_addr #2

declare i32 @__bt_cmp(ptr noundef, ptr noundef, ptr noundef) local_unnamed_addr #2

; Function Attrs: nounwind uwtable
define internal fastcc noundef i32 @ref___bt_relink(ptr nocapture noundef readonly %t, ptr nocapture noundef readonly %h) unnamed_addr #0 {
entry:
  %nextpg = getelementptr inbounds %struct._page, ptr %h, i64 0, i32 2
  %0 = load i32, ptr %nextpg, align 4, !tbaa !37
  %cmp.not = icmp eq i32 %0, 0
  br i1 %cmp.not, label %if.end7, label %if.then

if.then:                                          ; preds = %entry
  %1 = load ptr, ptr %t, align 8, !tbaa !19
  %call = tail call ptr @mpool_get(ptr noundef %1, i32 noundef %0, i32 noundef 0) #4
  %cmp2 = icmp eq ptr %call, null
  br i1 %cmp2, label %cleanup, label %if.end

if.end:                                           ; preds = %if.then
  %prevpg = getelementptr inbounds %struct._page, ptr %h, i64 0, i32 1
  %2 = load i32, ptr %prevpg, align 4, !tbaa !48
  %prevpg4 = getelementptr inbounds %struct._page, ptr %call, i64 0, i32 1
  store i32 %2, ptr %prevpg4, align 4, !tbaa !48
  %3 = load ptr, ptr %t, align 8, !tbaa !19
  %call6 = tail call i32 @mpool_put(ptr noundef %3, ptr noundef nonnull %call, i32 noundef 1) #4
  br label %if.end7

if.end7:                                          ; preds = %if.end, %entry
  %prevpg8 = getelementptr inbounds %struct._page, ptr %h, i64 0, i32 1
  %4 = load i32, ptr %prevpg8, align 4, !tbaa !48
  %cmp9.not = icmp eq i32 %4, 0
  br i1 %cmp9.not, label %cleanup, label %if.then10

if.then10:                                        ; preds = %if.end7
  %5 = load ptr, ptr %t, align 8, !tbaa !19
  %call13 = tail call ptr @mpool_get(ptr noundef %5, i32 noundef %4, i32 noundef 0) #4
  %cmp14 = icmp eq ptr %call13, null
  br i1 %cmp14, label %cleanup, label %if.end16

if.end16:                                         ; preds = %if.then10
  %6 = load i32, ptr %nextpg, align 4, !tbaa !37
  %nextpg18 = getelementptr inbounds %struct._page, ptr %call13, i64 0, i32 2
  store i32 %6, ptr %nextpg18, align 4, !tbaa !37
  %7 = load ptr, ptr %t, align 8, !tbaa !19
  %call20 = tail call i32 @mpool_put(ptr noundef %7, ptr noundef nonnull %call13, i32 noundef 1) #4
  br label %cleanup

cleanup:                                          ; preds = %if.end7, %if.end16, %if.then10, %if.then
  %retval.0 = phi i32 [ -1, %if.then ], [ -1, %if.then10 ], [ 0, %if.end16 ], [ 0, %if.end7 ]
  ret i32 %retval.0
}

declare i32 @__bt_free(ptr noundef, ptr noundef) local_unnamed_addr #2

declare i32 @__bt_ret(ptr noundef, ptr noundef, ptr noundef, ptr noundef, ptr noundef, ptr noundef, i32 noundef) local_unnamed_addr #2

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!5 = !{!6, !10, i64 56}
!6 = !{!"__db", !7, i64 0, !10, i64 8, !10, i64 16, !10, i64 24, !10, i64 32, !10, i64 40, !10, i64 48, !10, i64 56, !10, i64 64}
!7 = !{!"int", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{!"any pointer", !8, i64 0}
!11 = !{!12, !10, i64 32}
!12 = !{!"_btree", !10, i64 0, !10, i64 8, !13, i64 16, !10, i64 32, !15, i64 40, !8, i64 72, !10, i64 472, !17, i64 480, !17, i64 496, !7, i64 512, !7, i64 516, !7, i64 520, !14, i64 524, !7, i64 528, !7, i64 532, !16, i64 536, !10, i64 544, !10, i64 552, !10, i64 560, !10, i64 568, !7, i64 576, !10, i64 584, !10, i64 592, !10, i64 600, !18, i64 608, !7, i64 616, !18, i64 624, !8, i64 632, !7, i64 636}
!13 = !{!"_epg", !10, i64 0, !14, i64 8}
!14 = !{!"short", !8, i64 0}
!15 = !{!"_cursor", !16, i64 0, !17, i64 8, !7, i64 24, !8, i64 28}
!16 = !{!"_epgno", !7, i64 0, !14, i64 4}
!17 = !{!"", !10, i64 0, !18, i64 8}
!18 = !{!"long", !8, i64 0}
!19 = !{!12, !10, i64 0}
!20 = !{!12, !7, i64 636}
!21 = !{!7, !7, i64 0}
!22 = !{!13, !10, i64 0}
!23 = !{!13, !14, i64 8}
!24 = !{i32 -1, i32 1}
!25 = !{!26, !14, i64 16}
!26 = !{!"_page", !7, i64 0, !7, i64 4, !7, i64 8, !7, i64 12, !14, i64 16, !14, i64 18, !8, i64 20}
!27 = !{i32 -1, i32 2}
!28 = distinct !{!28, !29}
!29 = !{!"llvm.loop.mustprogress"}
!30 = distinct !{!30, !29}
!31 = !{!15, !8, i64 28}
!32 = !{!15, !7, i64 0}
!33 = !{!10, !10, i64 0}
!34 = !{i32 0, i32 2}
!35 = !{!15, !14, i64 4}
!36 = !{!26, !7, i64 0}
!37 = !{!26, !7, i64 8}
!38 = !{!12, !10, i64 472}
!39 = !{!16, !7, i64 0}
!40 = !{!16, !14, i64 4}
!41 = distinct !{!41, !29}
!42 = !{!14, !14, i64 0}
!43 = !{!44, !7, i64 4}
!44 = !{!"_binternal", !7, i64 0, !7, i64 4, !8, i64 8, !8, i64 9}
!45 = distinct !{!45, !29, !46}
!46 = !{!"llvm.loop.peeled.count", i32 1}
!47 = distinct !{!47, !29}
!48 = !{!26, !7, i64 4}
!49 = distinct !{!49, !29}
!50 = distinct !{!50, !29}
!51 = distinct !{!51, !29}
!52 = !{!44, !8, i64 8}
!53 = !{!12, !7, i64 520}
!54 = !{!26, !14, i64 18}
!55 = !{!26, !7, i64 12}
!56 = distinct !{!56, !29}
!57 = !{!44, !7, i64 0}
!58 = distinct !{!58, !29, !59, !60}
!59 = !{!"llvm.loop.isvectorized", i32 1}
!60 = !{!"llvm.loop.unroll.runtime.disable"}
!61 = distinct !{!61, !29, !60, !59}
!62 = distinct !{!62, !29, !59, !60}
!63 = distinct !{!63, !29, !60, !59}
!64 = !{!12, !7, i64 40}
!65 = !{!12, !14, i64 44}
!66 = !{!67, !8, i64 8}
!67 = !{!"_bleaf", !7, i64 0, !7, i64 4, !8, i64 8, !8, i64 9}
!68 = !{!67, !7, i64 0}
!69 = !{!67, !7, i64 4}
!70 = distinct !{!70, !29, !59, !60}
!71 = distinct !{!71, !29, !60, !59}
!72 = distinct !{!72, !29, !59, !60}
!73 = distinct !{!73, !29, !60, !59}
!74 = !{!12, !8, i64 68}
