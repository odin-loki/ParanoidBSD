; ModuleID = '/home/odin/pbsd/pbsd/lib/libc/stdio/open_wmemstream_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/libc/stdio/open_wmemstream_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

module asm ".symver __qsort_r_compat, qsort_r@FBSD_1.0"

%struct.wmemstream = type { ptr, ptr, i64, i64, %union.__mbstate_t }
%union.__mbstate_t = type { i64, [120 x i8] }

@__func__.ref_wmemstream_update = private unnamed_addr constant [22 x i8] c"ref_wmemstream_update\00", align 1
@.str = private unnamed_addr constant [63 x i8] c"/home/odin/pbsd/pbsd/lib/libc/stdio/open_wmemstream_m/oracle.c\00", align 1
@.str.1 = private unnamed_addr constant [32 x i8] c"ms->len >= 0 && ms->offset >= 0\00", align 1
@__func__.ref_wmemstream_seek = private unnamed_addr constant [20 x i8] c"ref_wmemstream_seek\00", align 1
@.str.2 = private unnamed_addr constant [9 x i8] c"pos >= 0\00", align 1
@.str.3 = private unnamed_addr constant [9 x i8] c"pos == 0\00", align 1

; Function Attrs: nounwind uwtable
define dso_local ptr @ref_open_wmemstream(ptr noundef %bufp, ptr noundef %sizep) local_unnamed_addr #0 {
entry:
  %cmp = icmp eq ptr %bufp, null
  %cmp1 = icmp eq ptr %sizep, null
  %or.cond = or i1 %cmp, %cmp1
  br i1 %or.cond, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %call = tail call ptr @__error() #12
  store i32 22, ptr %call, align 4, !tbaa !5
  br label %cleanup

if.end:                                           ; preds = %entry
  %call2 = tail call noalias dereferenceable_or_null(4) ptr @calloc(i64 noundef 1, i64 noundef 4) #13
  store ptr %call2, ptr %bufp, align 8, !tbaa !9
  %cmp3 = icmp eq ptr %call2, null
  br i1 %cmp3, label %cleanup, label %if.end5

if.end5:                                          ; preds = %if.end
  %call6 = tail call noalias dereferenceable_or_null(160) ptr @malloc(i64 noundef 160) #14
  %cmp7 = icmp eq ptr %call6, null
  br i1 %cmp7, label %if.then8, label %ref_wmemstream_update.exit

if.then8:                                         ; preds = %if.end5
  %call9 = tail call ptr @__error() #12
  %0 = load i32, ptr %call9, align 4, !tbaa !5
  %1 = load ptr, ptr %bufp, align 8, !tbaa !9
  tail call void @free(ptr noundef %1)
  store ptr null, ptr %bufp, align 8, !tbaa !9
  %call10 = tail call ptr @__error() #12
  store i32 %0, ptr %call10, align 4, !tbaa !5
  br label %cleanup

ref_wmemstream_update.exit:                       ; preds = %if.end5
  store ptr %bufp, ptr %call6, align 8, !tbaa !11
  %sizep13 = getelementptr inbounds %struct.wmemstream, ptr %call6, i64 0, i32 1
  store ptr %sizep, ptr %sizep13, align 8, !tbaa !14
  %len = getelementptr inbounds %struct.wmemstream, ptr %call6, i64 0, i32 2
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(144) %len, i8 0, i64 144, i1 false)
  store i64 0, ptr %sizep, align 8, !tbaa !15
  %call14 = tail call ptr @funopen(ptr noundef nonnull %call6, ptr noundef null, ptr noundef nonnull @ref_wmemstream_write, ptr noundef nonnull @ref_wmemstream_seek, ptr noundef nonnull @ref_wmemstream_close) #12
  %cmp15 = icmp eq ptr %call14, null
  br i1 %cmp15, label %if.then16, label %if.end19

if.then16:                                        ; preds = %ref_wmemstream_update.exit
  %call17 = tail call ptr @__error() #12
  %2 = load i32, ptr %call17, align 4, !tbaa !5
  tail call void @free(ptr noundef nonnull %call6)
  %3 = load ptr, ptr %bufp, align 8, !tbaa !9
  tail call void @free(ptr noundef %3)
  store ptr null, ptr %bufp, align 8, !tbaa !9
  %call18 = tail call ptr @__error() #12
  store i32 %2, ptr %call18, align 4, !tbaa !5
  br label %cleanup

if.end19:                                         ; preds = %ref_wmemstream_update.exit
  %call20 = tail call i32 @fwide(ptr noundef nonnull %call14, i32 noundef 1) #12
  br label %cleanup

cleanup:                                          ; preds = %if.end, %if.end19, %if.then16, %if.then8, %if.then
  %retval.0 = phi ptr [ null, %if.then ], [ null, %if.then8 ], [ null, %if.then16 ], [ %call14, %if.end19 ], [ null, %if.end ]
  ret ptr %retval.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

declare ptr @__error() local_unnamed_addr #2

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,zeroed") allocsize(0,1) memory(inaccessiblemem: readwrite)
declare noalias noundef ptr @calloc(i64 noundef, i64 noundef) local_unnamed_addr #3

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #4

; Function Attrs: mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite)
declare void @free(ptr allocptr nocapture noundef) local_unnamed_addr #5

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #6

declare ptr @funopen(ptr noundef, ptr noundef, ptr noundef, ptr noundef, ptr noundef) local_unnamed_addr #2

; Function Attrs: nounwind uwtable
define internal i32 @ref_wmemstream_write(ptr noundef %cookie, ptr noundef %buf, i32 noundef %len) #0 {
entry:
  %lenstate.i = alloca %union.__mbstate_t, align 8
  %mbstate = getelementptr inbounds %struct.wmemstream, ptr %cookie, i64 0, i32 4
  call void @llvm.lifetime.start.p0(i64 128, ptr nonnull %lenstate.i) #12
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 8 dereferenceable(128) %lenstate.i, ptr noundef nonnull align 8 dereferenceable(128) %mbstate, i64 128, i1 false), !tbaa.struct !16
  %cmp24.i = icmp sgt i32 %len, 0
  br i1 %cmp24.i, label %while.body.i, label %ref_wbuflen.exit.thread71

ref_wbuflen.exit.thread71:                        ; preds = %entry
  call void @llvm.lifetime.end.p0(i64 128, ptr nonnull %lenstate.i) #12
  br label %if.end

while.body.i:                                     ; preds = %entry, %if.end6.i
  %count.027.i = phi i64 [ %inc.i, %if.end6.i ], [ 0, %entry ]
  %len.addr.026.i = phi i32 [ %conv12.i, %if.end6.i ], [ %len, %entry ]
  %buf.addr.025.i = phi ptr [ %add.ptr.i, %if.end6.i ], [ %buf, %entry ]
  %conv.i = zext nneg i32 %len.addr.026.i to i64
  %call.i = call i64 @mbrlen(ptr noundef %buf.addr.025.i, i64 noundef %conv.i, ptr noundef nonnull %lenstate.i) #12
  switch i64 %call.i, label %if.end6.i [
    i64 -1, label %ref_wbuflen.exit.thread
    i64 -2, label %ref_wbuflen.exit
  ]

if.end6.i:                                        ; preds = %while.body.i
  %spec.store.select.i = call i64 @llvm.umax.i64(i64 %call.i, i64 1)
  %0 = trunc i64 %spec.store.select.i to i32
  %conv12.i = sub i32 %len.addr.026.i, %0
  %add.ptr.i = getelementptr inbounds i8, ptr %buf.addr.025.i, i64 %spec.store.select.i
  %inc.i = add i64 %count.027.i, 1
  %cmp.i = icmp sgt i32 %conv12.i, 0
  br i1 %cmp.i, label %while.body.i, label %ref_wbuflen.exit, !llvm.loop !18

ref_wbuflen.exit.thread:                          ; preds = %while.body.i
  call void @llvm.lifetime.end.p0(i64 128, ptr nonnull %lenstate.i) #12
  br label %if.then

ref_wbuflen.exit:                                 ; preds = %while.body.i, %if.end6.i
  %retval.0.i = phi i64 [ %count.027.i, %while.body.i ], [ %inc.i, %if.end6.i ]
  call void @llvm.lifetime.end.p0(i64 128, ptr nonnull %lenstate.i) #12
  %cmp = icmp slt i64 %retval.0.i, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %ref_wbuflen.exit.thread, %ref_wbuflen.exit
  %call1 = call ptr @__error() #12
  store i32 86, ptr %call1, align 4, !tbaa !5
  br label %cleanup

if.end:                                           ; preds = %ref_wbuflen.exit.thread71, %ref_wbuflen.exit
  %retval.0.i74 = phi i64 [ 0, %ref_wbuflen.exit.thread71 ], [ %retval.0.i, %ref_wbuflen.exit ]
  %offset = getelementptr inbounds %struct.wmemstream, ptr %cookie, i64 0, i32 3
  %1 = load i64, ptr %offset, align 8, !tbaa !20
  %add = add nsw i64 %1, %retval.0.i74
  %.newoff.i = call i64 @llvm.umin.i64(i64 %add, i64 2305843009213693950)
  %len.i = getelementptr inbounds %struct.wmemstream, ptr %cookie, i64 0, i32 2
  %2 = load i64, ptr %len.i, align 8, !tbaa !21
  %cmp2.i = icmp sgt i64 %.newoff.i, %2
  br i1 %cmp2.i, label %if.then3.i, label %ref_wmemstream_grow.exit

if.then3.i:                                       ; preds = %if.end
  %3 = load ptr, ptr %cookie, align 8, !tbaa !11
  %4 = load ptr, ptr %3, align 8, !tbaa !9
  %add.i = add nuw nsw i64 %.newoff.i, 1
  %call.i65 = call ptr @reallocarray(ptr noundef %4, i64 noundef %add.i, i64 noundef 4) #15
  %cmp4.not.i = icmp eq ptr %call.i65, null
  br i1 %cmp4.not.i, label %cleanup, label %if.then5.i

if.then5.i:                                       ; preds = %if.then3.i
  %5 = load i64, ptr %len.i, align 8, !tbaa !21
  %add.ptr.i66 = getelementptr inbounds i32, ptr %call.i65, i64 %5
  %add.ptr7.i = getelementptr inbounds i32, ptr %add.ptr.i66, i64 1
  %sub.i = sub nsw i64 %.newoff.i, %5
  %call9.i = call ptr @wmemset(ptr noundef nonnull %add.ptr7.i, i32 noundef 0, i64 noundef %sub.i) #12
  %6 = load ptr, ptr %cookie, align 8, !tbaa !11
  store ptr %call.i65, ptr %6, align 8, !tbaa !9
  store i64 %.newoff.i, ptr %len.i, align 8, !tbaa !21
  br label %ref_wmemstream_grow.exit

ref_wmemstream_grow.exit:                         ; preds = %if.end, %if.then5.i
  br i1 %cmp24.i, label %land.rhs, label %while.endthread-pre-split

land.rhs:                                         ; preds = %ref_wmemstream_grow.exit, %if.end35
  %consumed.085 = phi i64 [ %consumed.1, %if.end35 ], [ 0, %ref_wmemstream_grow.exit ]
  %buf.addr.084 = phi ptr [ %add.ptr31, %if.end35 ], [ %buf, %ref_wmemstream_grow.exit ]
  %len.addr.083 = phi i32 [ %conv33, %if.end35 ], [ %len, %ref_wmemstream_grow.exit ]
  %7 = load i64, ptr %offset, align 8, !tbaa !20
  %8 = load i64, ptr %len.i, align 8, !tbaa !21
  %cmp8 = icmp slt i64 %7, %8
  br i1 %cmp8, label %while.body, label %while.end

while.body:                                       ; preds = %land.rhs
  %9 = load ptr, ptr %cookie, align 8, !tbaa !11
  %10 = load ptr, ptr %9, align 8, !tbaa !9
  %add.ptr = getelementptr inbounds i32, ptr %10, i64 %7
  %conv = zext nneg i32 %len.addr.083 to i64
  %call11 = call i64 @mbrtowc(ptr noundef %add.ptr, ptr noundef %buf.addr.084, i64 noundef %conv, ptr noundef nonnull %mbstate) #12
  switch i64 %call11, label %if.end35 [
    i64 -1, label %if.then14
    i64 -2, label %if.end35.thread
  ]

if.end35.thread:                                  ; preds = %while.body
  %consumed.193 = add i64 %consumed.085, %conv
  br label %while.endthread-pre-split

if.then14:                                        ; preds = %while.body
  %cmp15 = icmp eq i64 %consumed.085, 0
  br i1 %cmp15, label %if.then17, label %while.endthread-pre-split

if.then17:                                        ; preds = %if.then14
  %call18 = call ptr @__error() #12
  store i32 86, ptr %call18, align 4, !tbaa !5
  br label %cleanup

if.end35:                                         ; preds = %while.body
  %spec.store.select = call i64 @llvm.umax.i64(i64 %call11, i64 1)
  %add.ptr31 = getelementptr inbounds i8, ptr %buf.addr.084, i64 %spec.store.select
  %11 = trunc i64 %spec.store.select to i32
  %conv33 = sub i32 %len.addr.083, %11
  %12 = load i64, ptr %offset, align 8, !tbaa !20
  %inc = add nsw i64 %12, 1
  store i64 %inc, ptr %offset, align 8, !tbaa !20
  %consumed.1 = add i64 %spec.store.select, %consumed.085
  %cmp5 = icmp sgt i32 %conv33, 0
  br i1 %cmp5, label %land.rhs, label %while.endthread-pre-split, !llvm.loop !22

while.endthread-pre-split:                        ; preds = %if.end35, %if.end35.thread, %ref_wmemstream_grow.exit, %if.then14
  %consumed.080 = phi i64 [ %consumed.085, %if.then14 ], [ 0, %ref_wmemstream_grow.exit ], [ %consumed.193, %if.end35.thread ], [ %consumed.1, %if.end35 ]
  %.pr = load i64, ptr %len.i, align 8, !tbaa !21
  br label %while.end

while.end:                                        ; preds = %land.rhs, %while.endthread-pre-split
  %consumed.079 = phi i64 [ %consumed.080, %while.endthread-pre-split ], [ %consumed.085, %land.rhs ]
  %13 = phi i64 [ %.pr, %while.endthread-pre-split ], [ %8, %land.rhs ]
  %cmp.i68 = icmp sgt i64 %13, -1
  br i1 %cmp.i68, label %land.lhs.true.i, label %cond.false.i

land.lhs.true.i:                                  ; preds = %while.end
  %14 = load i64, ptr %offset, align 8, !tbaa !20
  %cmp1.i = icmp sgt i64 %14, -1
  br i1 %cmp1.i, label %ref_wmemstream_update.exit, label %cond.false.i

cond.false.i:                                     ; preds = %land.lhs.true.i, %while.end
  call void @__assert(ptr noundef nonnull @__func__.ref_wmemstream_update, ptr noundef nonnull @.str, i32 noundef 85, ptr noundef nonnull @.str.1) #16
  unreachable

ref_wmemstream_update.exit:                       ; preds = %land.lhs.true.i
  %..i = call i64 @llvm.umin.i64(i64 %13, i64 %14)
  %sizep.i = getelementptr inbounds %struct.wmemstream, ptr %cookie, i64 0, i32 1
  %15 = load ptr, ptr %sizep.i, align 8, !tbaa !14
  store i64 %..i, ptr %15, align 8, !tbaa !15
  %conv36 = trunc i64 %consumed.079 to i32
  br label %cleanup

cleanup:                                          ; preds = %if.then3.i, %ref_wmemstream_update.exit, %if.then17, %if.then
  %retval.0 = phi i32 [ -1, %if.then ], [ -1, %if.then17 ], [ %conv36, %ref_wmemstream_update.exit ], [ -1, %if.then3.i ]
  ret i32 %retval.0
}

; Function Attrs: nounwind uwtable
define internal i64 @ref_wmemstream_seek(ptr nocapture noundef %cookie, i64 noundef %pos, i32 noundef %whence) #0 {
entry:
  %offset = getelementptr inbounds %struct.wmemstream, ptr %cookie, i64 0, i32 3
  %0 = load i64, ptr %offset, align 8, !tbaa !20
  switch i32 %whence, label %if.end23 [
    i32 0, label %sw.bb
    i32 1, label %sw.bb2
    i32 2, label %sw.bb7
  ]

sw.bb:                                            ; preds = %entry
  %cmp = icmp sgt i64 %pos, -1
  br i1 %cmp, label %sw.epilog, label %cond.false

cond.false:                                       ; preds = %sw.bb
  tail call void @__assert(ptr noundef nonnull @__func__.ref_wmemstream_seek, ptr noundef nonnull @.str, i32 noundef 182, ptr noundef nonnull @.str.2) #16
  unreachable

sw.bb2:                                           ; preds = %entry
  %cmp3 = icmp eq i64 %pos, 0
  br i1 %cmp3, label %if.end23, label %cond.false5

cond.false5:                                      ; preds = %sw.bb2
  tail call void @__assert(ptr noundef nonnull @__func__.ref_wmemstream_seek, ptr noundef nonnull @.str, i32 noundef 187, ptr noundef nonnull @.str.3) #16
  unreachable

sw.bb7:                                           ; preds = %entry
  %cmp8 = icmp slt i64 %pos, 0
  %len = getelementptr inbounds %struct.wmemstream, ptr %cookie, i64 0, i32 2
  %1 = load i64, ptr %len, align 8, !tbaa !21
  br i1 %cmp8, label %if.then, label %if.else

if.then:                                          ; preds = %sw.bb7
  %add = add nsw i64 %1, %pos
  %cmp9 = icmp slt i64 %add, 0
  br i1 %cmp9, label %if.then10, label %sw.epilog

if.then10:                                        ; preds = %if.then
  %call = tail call ptr @__error() #12
  store i32 22, ptr %call, align 4, !tbaa !5
  br label %cleanup

if.else:                                          ; preds = %sw.bb7
  %sub = sub nsw i64 9223372036854775807, %1
  %cmp12 = icmp ult i64 %sub, %pos
  br i1 %cmp12, label %if.then13, label %if.else.if.end16_crit_edge

if.else.if.end16_crit_edge:                       ; preds = %if.else
  %.pre = add nsw i64 %1, %pos
  br label %sw.epilog

if.then13:                                        ; preds = %if.else
  %call14 = tail call ptr @__error() #12
  store i32 84, ptr %call14, align 4, !tbaa !5
  br label %cleanup

sw.epilog:                                        ; preds = %if.then, %if.else.if.end16_crit_edge, %sw.bb
  %2 = phi i64 [ %pos, %sw.bb ], [ %.pre, %if.else.if.end16_crit_edge ], [ %add, %if.then ]
  store i64 %2, ptr %offset, align 8, !tbaa !20
  %cmp21.not = icmp eq i64 %2, %0
  br i1 %cmp21.not, label %if.end23, label %if.then22

if.then22:                                        ; preds = %sw.epilog
  %mbstate = getelementptr inbounds %struct.wmemstream, ptr %cookie, i64 0, i32 4
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(128) %mbstate, i8 0, i64 128, i1 false)
  br label %if.end23

if.end23:                                         ; preds = %entry, %sw.bb2, %if.then22, %sw.epilog
  %3 = phi i64 [ %2, %if.then22 ], [ %0, %sw.epilog ], [ %0, %sw.bb2 ], [ %0, %entry ]
  %len.i = getelementptr inbounds %struct.wmemstream, ptr %cookie, i64 0, i32 2
  %4 = load i64, ptr %len.i, align 8, !tbaa !21
  %cmp.i = icmp sgt i64 %4, -1
  %cmp1.i = icmp sgt i64 %3, -1
  %or.cond = select i1 %cmp.i, i1 %cmp1.i, i1 false
  br i1 %or.cond, label %ref_wmemstream_update.exit, label %cond.false.i

cond.false.i:                                     ; preds = %if.end23
  tail call void @__assert(ptr noundef nonnull @__func__.ref_wmemstream_update, ptr noundef nonnull @.str, i32 noundef 85, ptr noundef nonnull @.str.1) #16
  unreachable

ref_wmemstream_update.exit:                       ; preds = %if.end23
  %..i = tail call i64 @llvm.umin.i64(i64 %4, i64 %3)
  %sizep.i = getelementptr inbounds %struct.wmemstream, ptr %cookie, i64 0, i32 1
  %5 = load ptr, ptr %sizep.i, align 8, !tbaa !14
  store i64 %..i, ptr %5, align 8, !tbaa !15
  %6 = load i64, ptr %offset, align 8, !tbaa !20
  br label %cleanup

cleanup:                                          ; preds = %ref_wmemstream_update.exit, %if.then13, %if.then10
  %retval.0 = phi i64 [ %6, %ref_wmemstream_update.exit ], [ -1, %if.then10 ], [ -1, %if.then13 ]
  ret i64 %retval.0
}

; Function Attrs: mustprogress nounwind willreturn memory(argmem: readwrite, inaccessiblemem: readwrite) uwtable
define internal noundef i32 @ref_wmemstream_close(ptr nocapture noundef %cookie) #7 {
entry:
  tail call void @free(ptr noundef %cookie)
  ret i32 0
}

declare i32 @fwide(ptr noundef, i32 noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: noreturn
declare void @__assert(ptr noundef, ptr noundef, i32 noundef, ptr noundef) local_unnamed_addr #8

declare i64 @mbrtowc(ptr noundef, ptr noundef, i64 noundef, ptr noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #9

declare i64 @mbrlen(ptr noundef, i64 noundef, ptr noundef) local_unnamed_addr #2

; Function Attrs: allocsize(1,2)
declare ptr @reallocarray(ptr noundef, i64 noundef, i64 noundef) local_unnamed_addr #10

declare ptr @wmemset(ptr noundef, i32 noundef, i64 noundef) local_unnamed_addr #2

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umax.i64(i64, i64) #11

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umin.i64(i64, i64) #11

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { mustprogress nofree nounwind willreturn allockind("alloc,zeroed") allocsize(0,1) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #7 = { mustprogress nounwind willreturn memory(argmem: readwrite, inaccessiblemem: readwrite) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #8 = { noreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #9 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #10 = { allocsize(1,2) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #11 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #12 = { nounwind }
attributes #13 = { allocsize(0,1) }
attributes #14 = { allocsize(0) }
attributes #15 = { nounwind allocsize(1,2) }
attributes #16 = { noreturn nounwind }

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
!11 = !{!12, !10, i64 0}
!12 = !{!"wmemstream", !10, i64 0, !10, i64 8, !13, i64 16, !13, i64 24, !7, i64 32}
!13 = !{!"long", !7, i64 0}
!14 = !{!12, !10, i64 8}
!15 = !{!13, !13, i64 0}
!16 = !{i64 0, i64 128, !17, i64 0, i64 8, !15}
!17 = !{!7, !7, i64 0}
!18 = distinct !{!18, !19}
!19 = !{!"llvm.loop.mustprogress"}
!20 = !{!12, !13, i64 24}
!21 = !{!12, !13, i64 16}
!22 = distinct !{!22, !19}
