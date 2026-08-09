; ModuleID = '/home/odin/pbsd/pbsd/lib/libc/net/getifmaddrs_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/libc/net/getifmaddrs_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

module asm ".symver __qsort_r_compat, qsort_r@FBSD_1.0"

%struct.rt_msghdr = type { i16, i8, i8, i16, i16, i32, i32, i32, i32, i32, i32, i64, %struct.rt_metrics }
%struct.rt_metrics = type { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, [2 x i64] }
%struct.ifma_msghdr = type { i16, i8, i8, i32, i32, i16, i16 }
%struct.ifmaddrs = type { ptr, ptr, ptr, ptr }

; Function Attrs: nounwind uwtable
define dso_local noundef i32 @ref_getifmaddrs(ptr nocapture noundef writeonly %pif) local_unnamed_addr #0 {
entry:
  %needed = alloca i64, align 8
  %mib = alloca [6 x i32], align 16
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %needed) #8
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %mib) #8
  store <4 x i32> <i32 4, i32 17, i32 0, i32 0>, ptr %mib, align 16, !tbaa !5
  %arrayidx4 = getelementptr inbounds [6 x i32], ptr %mib, i64 0, i64 4
  store i32 4, ptr %arrayidx4, align 16, !tbaa !5
  %arrayidx5 = getelementptr inbounds [6 x i32], ptr %mib, i64 0, i64 5
  store i32 0, ptr %arrayidx5, align 4, !tbaa !5
  %call264 = call i32 @sysctl(ptr noundef nonnull %mib, i32 noundef 6, ptr noundef null, ptr noundef nonnull %needed, ptr noundef null, i64 noundef 0) #8
  %cmp265 = icmp slt i32 %call264, 0
  br i1 %cmp265, label %cleanup, label %if.end

if.end:                                           ; preds = %entry, %do.cond
  %ntry.0266 = phi i32 [ %inc, %do.cond ], [ 0, %entry ]
  %0 = load i64, ptr %needed, align 8, !tbaa !9
  %call6 = call noalias ptr @malloc(i64 noundef %0) #9
  %cmp7 = icmp eq ptr %call6, null
  br i1 %cmp7, label %cleanup, label %if.end9

if.end9:                                          ; preds = %if.end
  %call11 = call i32 @sysctl(ptr noundef nonnull %mib, i32 noundef 6, ptr noundef nonnull %call6, ptr noundef nonnull %needed, ptr noundef null, i64 noundef 0) #8
  %cmp12 = icmp slt i32 %call11, 0
  br i1 %cmp12, label %if.then13, label %for.cond.preheader

if.then13:                                        ; preds = %if.end9
  %call14 = call ptr @__error() #8
  %1 = load i32, ptr %call14, align 4, !tbaa !5
  %cmp15.not = icmp ne i32 %1, 12
  %cmp16 = icmp ugt i32 %ntry.0266, 3
  %or.cond = select i1 %cmp15.not, i1 true, i1 %cmp16
  br i1 %or.cond, label %if.then17, label %do.cond

if.then17:                                        ; preds = %if.then13
  call void @free(ptr noundef nonnull %call6)
  br label %cleanup

do.cond:                                          ; preds = %if.then13
  %inc = add nuw nsw i32 %ntry.0266, 1
  call void @free(ptr noundef nonnull %call6)
  %call = call i32 @sysctl(ptr noundef nonnull %mib, i32 noundef 6, ptr noundef null, ptr noundef nonnull %needed, ptr noundef null, i64 noundef 0) #8
  %cmp = icmp slt i32 %call, 0
  br i1 %cmp, label %cleanup, label %if.end, !llvm.loop !11

for.cond.preheader:                               ; preds = %if.end9
  %2 = load i64, ptr %needed, align 8, !tbaa !9
  %add.ptr = getelementptr inbounds i8, ptr %call6, i64 %2
  %cmp21240 = icmp sgt i64 %2, 0
  br i1 %cmp21240, label %for.body, label %for.end56

for.body:                                         ; preds = %for.cond.preheader, %for.inc53
  %next.0243 = phi ptr [ %add.ptr55, %for.inc53 ], [ %call6, %for.cond.preheader ]
  %dcnt.0242 = phi i32 [ %dcnt.3, %for.inc53 ], [ 0, %for.cond.preheader ]
  %icnt.0241 = phi i32 [ %icnt.1, %for.inc53 ], [ 1, %for.cond.preheader ]
  %rtm_version = getelementptr inbounds %struct.rt_msghdr, ptr %next.0243, i64 0, i32 1
  %3 = load i8, ptr %rtm_version, align 2, !tbaa !13
  %cmp22.not = icmp eq i8 %3, 5
  br i1 %cmp22.not, label %if.end25, label %for.inc53

if.end25:                                         ; preds = %for.body
  %rtm_type = getelementptr inbounds %struct.rt_msghdr, ptr %next.0243, i64 0, i32 2
  %4 = load i8, ptr %rtm_type, align 1, !tbaa !17
  %cond156 = icmp eq i8 %4, 15
  br i1 %cond156, label %sw.bb, label %for.inc53

sw.bb:                                            ; preds = %if.end25
  %ifmam_addrs = getelementptr inbounds %struct.ifma_msghdr, ptr %next.0243, i64 0, i32 3
  %5 = load i32, ptr %ifmam_addrs, align 4, !tbaa !18
  %and = and i32 %5, 32
  %cmp27 = icmp eq i32 %and, 0
  br i1 %cmp27, label %for.inc53, label %for.inc

for.inc:                                          ; preds = %sw.bb
  %add.ptr32 = getelementptr inbounds %struct.ifma_msghdr, ptr %next.0243, i64 1
  %and39.1 = and i32 %5, 2
  %cmp40.1 = icmp eq i32 %and39.1, 0
  br i1 %cmp40.1, label %for.inc.3, label %if.end43.1

if.end43.1:                                       ; preds = %for.inc
  %6 = load i8, ptr %add.ptr32, align 1, !tbaa !20
  %tobool.not.1 = icmp eq i8 %6, 0
  %conv46.1 = zext i8 %6 to i64
  %add.1 = add nuw nsw i64 %conv46.1, 7
  %and47.1 = and i64 %add.1, 504
  %cond.1 = select i1 %tobool.not.1, i64 8, i64 %and47.1
  %7 = trunc i64 %cond.1 to i32
  %conv50.1 = add i32 %dcnt.0242, %7
  %add.ptr51.1 = getelementptr inbounds i8, ptr %add.ptr32, i64 %cond.1
  br label %for.inc.3

for.inc.3:                                        ; preds = %if.end43.1, %for.inc
  %dcnt.2.1 = phi i32 [ %dcnt.0242, %for.inc ], [ %conv50.1, %if.end43.1 ]
  %p.1.1 = phi ptr [ %add.ptr32, %for.inc ], [ %add.ptr51.1, %if.end43.1 ]
  %and39.4 = and i32 %5, 16
  %cmp40.4 = icmp eq i32 %and39.4, 0
  br i1 %cmp40.4, label %for.inc.7, label %if.end43.4

if.end43.4:                                       ; preds = %for.inc.3
  %8 = load i8, ptr %p.1.1, align 1, !tbaa !20
  %tobool.not.4 = icmp eq i8 %8, 0
  %conv46.4 = zext i8 %8 to i64
  %add.4 = add nuw nsw i64 %conv46.4, 7
  %and47.4 = and i64 %add.4, 504
  %cond.4 = select i1 %tobool.not.4, i64 8, i64 %and47.4
  %9 = trunc i64 %cond.4 to i32
  %conv50.4 = add i32 %dcnt.2.1, %9
  %add.ptr51.4 = getelementptr inbounds i8, ptr %p.1.1, i64 %cond.4
  br label %for.inc.7

for.inc.7:                                        ; preds = %if.end43.4, %for.inc.3
  %dcnt.2.4 = phi i32 [ %dcnt.2.1, %for.inc.3 ], [ %conv50.4, %if.end43.4 ]
  %p.1.4 = phi ptr [ %p.1.1, %for.inc.3 ], [ %add.ptr51.4, %if.end43.4 ]
  %10 = load i8, ptr %p.1.4, align 1, !tbaa !20
  %tobool.not.5 = icmp eq i8 %10, 0
  %conv46.5 = zext i8 %10 to i32
  %add.5 = add nuw nsw i32 %conv46.5, 7
  %and47.5 = and i32 %add.5, 504
  %11 = select i1 %tobool.not.5, i32 8, i32 %and47.5
  %conv50.5 = add i32 %dcnt.2.4, %11
  %inc31 = add nsw i32 %icnt.0241, 1
  br label %for.inc53

for.inc53:                                        ; preds = %for.inc.7, %if.end25, %sw.bb, %for.body
  %icnt.1 = phi i32 [ %icnt.0241, %for.body ], [ %icnt.0241, %sw.bb ], [ %icnt.0241, %if.end25 ], [ %inc31, %for.inc.7 ]
  %dcnt.3 = phi i32 [ %dcnt.0242, %for.body ], [ %dcnt.0242, %sw.bb ], [ %dcnt.0242, %if.end25 ], [ %conv50.5, %for.inc.7 ]
  %12 = load i16, ptr %next.0243, align 8, !tbaa !22
  %idx.ext = zext i16 %12 to i64
  %add.ptr55 = getelementptr inbounds i8, ptr %next.0243, i64 %idx.ext
  %cmp21 = icmp ult ptr %add.ptr55, %add.ptr
  br i1 %cmp21, label %for.body, label %for.end56.loopexit, !llvm.loop !23

for.end56.loopexit:                               ; preds = %for.inc53
  %13 = sext i32 %icnt.1 to i64
  %14 = shl nsw i64 %13, 5
  %15 = sext i32 %dcnt.3 to i64
  br label %for.end56

for.end56:                                        ; preds = %for.end56.loopexit, %for.cond.preheader
  %icnt.0.lcssa = phi i64 [ 32, %for.cond.preheader ], [ %14, %for.end56.loopexit ]
  %dcnt.0.lcssa = phi i64 [ 0, %for.cond.preheader ], [ %15, %for.end56.loopexit ]
  %add59 = add nsw i64 %icnt.0.lcssa, %dcnt.0.lcssa
  %call60 = call noalias ptr @malloc(i64 noundef %add59) #9
  %cmp61 = icmp eq ptr %call60, null
  br i1 %cmp61, label %if.then63, label %if.end64

if.then63:                                        ; preds = %for.end56
  call void @free(ptr noundef %call6)
  br label %cleanup

if.end64:                                         ; preds = %for.end56
  call void @llvm.memset.p0.i64(ptr nonnull align 8 %call60, i8 0, i64 %icnt.0.lcssa, i1 false)
  br i1 %cmp21240, label %for.body74.preheader, label %for.end134.thread

for.end134.thread:                                ; preds = %if.end64
  call void @free(ptr noundef %call6)
  br label %if.else

for.body74.preheader:                             ; preds = %if.end64
  %add.ptr67 = getelementptr inbounds i8, ptr %call60, i64 %icnt.0.lcssa
  br label %for.body74

for.body74:                                       ; preds = %for.body74.preheader, %for.inc129
  %16 = phi i64 [ %25, %for.inc129 ], [ %2, %for.body74.preheader ]
  %ift.0252 = phi ptr [ %ift.1, %for.inc129 ], [ %call60, %for.body74.preheader ]
  %next.1251 = phi ptr [ %add.ptr133, %for.inc129 ], [ %call6, %for.body74.preheader ]
  %data.0250 = phi ptr [ %data.4, %for.inc129 ], [ %add.ptr67, %for.body74.preheader ]
  %rtm_version75 = getelementptr inbounds %struct.rt_msghdr, ptr %next.1251, i64 0, i32 1
  %17 = load i8, ptr %rtm_version75, align 2, !tbaa !13
  %cmp77.not = icmp eq i8 %17, 5
  br i1 %cmp77.not, label %if.end80, label %for.inc129

if.end80:                                         ; preds = %for.body74
  %rtm_type81 = getelementptr inbounds %struct.rt_msghdr, ptr %next.1251, i64 0, i32 2
  %18 = load i8, ptr %rtm_type81, align 1, !tbaa !17
  %cond155 = icmp eq i8 %18, 15
  br i1 %cond155, label %sw.bb83, label %for.inc129

sw.bb83:                                          ; preds = %if.end80
  %ifmam_addrs84 = getelementptr inbounds %struct.ifma_msghdr, ptr %next.1251, i64 0, i32 3
  %19 = load i32, ptr %ifmam_addrs84, align 4, !tbaa !18
  %and85 = and i32 %19, 32
  %cmp86 = icmp eq i32 %and85, 0
  br i1 %cmp86, label %for.inc129, label %for.inc123

for.inc123:                                       ; preds = %sw.bb83
  %add.ptr90 = getelementptr inbounds %struct.ifma_msghdr, ptr %next.1251, i64 1
  %ifma_addr = getelementptr inbounds %struct.ifmaddrs, ptr %ift.0252, i64 0, i32 2
  %ifma_name = getelementptr inbounds %struct.ifmaddrs, ptr %ift.0252, i64 0, i32 1
  %and98.1 = and i32 %19, 2
  %cmp99.1 = icmp eq i32 %and98.1, 0
  br i1 %cmp99.1, label %for.inc123.3, label %sw.epilog121.1

sw.epilog121.1:                                   ; preds = %for.inc123
  %ifma_lladdr = getelementptr inbounds %struct.ifmaddrs, ptr %ift.0252, i64 0, i32 3
  %20 = load i8, ptr %add.ptr90, align 1, !tbaa !20
  %tobool105.not.1 = icmp eq i8 %20, 0
  %conv108.1 = zext i8 %20 to i64
  %add109.1 = add nuw nsw i64 %conv108.1, 7
  %and110.1 = and i64 %add109.1, 504
  %cond113.1 = select i1 %tobool105.not.1, i64 8, i64 %and110.1
  store ptr %data.0250, ptr %ifma_lladdr, align 8, !tbaa !24
  call void @llvm.memcpy.p0.p0.i64(ptr align 1 %data.0250, ptr nonnull align 1 %add.ptr90, i64 %cond113.1, i1 false)
  %data.2.1 = getelementptr inbounds i8, ptr %data.0250, i64 %cond113.1
  %add.ptr122.1 = getelementptr inbounds i8, ptr %add.ptr90, i64 %cond113.1
  br label %for.inc123.3

for.inc123.3:                                     ; preds = %sw.epilog121.1, %for.inc123
  %data.3.1 = phi ptr [ %data.0250, %for.inc123 ], [ %data.2.1, %sw.epilog121.1 ]
  %p.3.1 = phi ptr [ %add.ptr90, %for.inc123 ], [ %add.ptr122.1, %sw.epilog121.1 ]
  %21 = load i32, ptr %ifmam_addrs84, align 4, !tbaa !18
  %and98.4 = and i32 %21, 16
  %cmp99.4 = icmp eq i32 %and98.4, 0
  br i1 %cmp99.4, label %for.inc123.4, label %sw.epilog121.4

sw.epilog121.4:                                   ; preds = %for.inc123.3
  %22 = load i8, ptr %p.3.1, align 1, !tbaa !20
  %tobool105.not.4 = icmp eq i8 %22, 0
  %conv108.4 = zext i8 %22 to i64
  %add109.4 = add nuw nsw i64 %conv108.4, 7
  %and110.4 = and i64 %add109.4, 504
  %cond113.4 = select i1 %tobool105.not.4, i64 8, i64 %and110.4
  store ptr %data.3.1, ptr %ifma_name, align 8, !tbaa !27
  call void @llvm.memcpy.p0.p0.i64(ptr align 1 %data.3.1, ptr nonnull align 1 %p.3.1, i64 %cond113.4, i1 false)
  %data.2.4 = getelementptr inbounds i8, ptr %data.3.1, i64 %cond113.4
  %add.ptr122.4 = getelementptr inbounds i8, ptr %p.3.1, i64 %cond113.4
  %.pre = load i32, ptr %ifmam_addrs84, align 4, !tbaa !18
  br label %for.inc123.4

for.inc123.4:                                     ; preds = %sw.epilog121.4, %for.inc123.3
  %23 = phi i32 [ %21, %for.inc123.3 ], [ %.pre, %sw.epilog121.4 ]
  %data.3.4 = phi ptr [ %data.3.1, %for.inc123.3 ], [ %data.2.4, %sw.epilog121.4 ]
  %p.3.4 = phi ptr [ %p.3.1, %for.inc123.3 ], [ %add.ptr122.4, %sw.epilog121.4 ]
  %and98.5 = and i32 %23, 32
  %cmp99.5 = icmp eq i32 %and98.5, 0
  br i1 %cmp99.5, label %for.inc123.7, label %sw.epilog121.5

sw.epilog121.5:                                   ; preds = %for.inc123.4
  %24 = load i8, ptr %p.3.4, align 1, !tbaa !20
  %tobool105.not.5 = icmp eq i8 %24, 0
  %conv108.5 = zext i8 %24 to i64
  %add109.5 = add nuw nsw i64 %conv108.5, 7
  %and110.5 = and i64 %add109.5, 504
  %cond113.5 = select i1 %tobool105.not.5, i64 8, i64 %and110.5
  store ptr %data.3.4, ptr %ifma_addr, align 8, !tbaa !28
  call void @llvm.memcpy.p0.p0.i64(ptr align 1 %data.3.4, ptr nonnull align 1 %p.3.4, i64 %cond113.5, i1 false)
  %data.2.5 = getelementptr inbounds i8, ptr %data.3.4, i64 %cond113.5
  br label %for.inc123.7

for.inc123.7:                                     ; preds = %sw.epilog121.5, %for.inc123.4
  %data.3.5 = phi ptr [ %data.3.4, %for.inc123.4 ], [ %data.2.5, %sw.epilog121.5 ]
  %add.ptr126 = getelementptr inbounds %struct.ifmaddrs, ptr %ift.0252, i64 1
  store ptr %add.ptr126, ptr %ift.0252, align 8, !tbaa !29
  %.pre255 = load i64, ptr %needed, align 8, !tbaa !9
  br label %for.inc129

for.inc129:                                       ; preds = %for.inc123.7, %if.end80, %sw.bb83, %for.body74
  %25 = phi i64 [ %16, %for.body74 ], [ %16, %sw.bb83 ], [ %.pre255, %for.inc123.7 ], [ %16, %if.end80 ]
  %data.4 = phi ptr [ %data.0250, %for.body74 ], [ %data.0250, %sw.bb83 ], [ %data.3.5, %for.inc123.7 ], [ %data.0250, %if.end80 ]
  %ift.1 = phi ptr [ %ift.0252, %for.body74 ], [ %ift.0252, %sw.bb83 ], [ %add.ptr126, %for.inc123.7 ], [ %ift.0252, %if.end80 ]
  %26 = load i16, ptr %next.1251, align 8, !tbaa !22
  %idx.ext132 = zext i16 %26 to i64
  %add.ptr133 = getelementptr inbounds i8, ptr %next.1251, i64 %idx.ext132
  %add.ptr71 = getelementptr inbounds i8, ptr %call6, i64 %25
  %cmp72 = icmp ult ptr %add.ptr133, %add.ptr71
  br i1 %cmp72, label %for.body74, label %for.end134, !llvm.loop !30

for.end134:                                       ; preds = %for.inc129
  call void @free(ptr noundef %call6)
  %cmp135 = icmp ugt ptr %ift.1, %call60
  br i1 %cmp135, label %if.then137, label %if.else

if.then137:                                       ; preds = %for.end134
  %incdec.ptr = getelementptr inbounds %struct.ifmaddrs, ptr %ift.1, i64 -1
  store ptr null, ptr %incdec.ptr, align 8, !tbaa !29
  store ptr %call60, ptr %pif, align 8, !tbaa !31
  br label %cleanup

if.else:                                          ; preds = %for.end134.thread, %for.end134
  store ptr null, ptr %pif, align 8, !tbaa !31
  call void @free(ptr noundef %call60)
  br label %cleanup

cleanup:                                          ; preds = %do.cond, %if.end, %entry, %if.then137, %if.else, %if.then63, %if.then17
  %retval.0 = phi i32 [ -1, %if.then17 ], [ -1, %if.then63 ], [ 0, %if.else ], [ 0, %if.then137 ], [ -1, %entry ], [ -1, %if.end ], [ -1, %do.cond ]
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %mib) #8
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %needed) #8
  ret i32 %retval.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

declare i32 @sysctl(ptr noundef, i32 noundef, ptr noundef, ptr noundef, ptr noundef, i64 noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #3

declare ptr @__error() local_unnamed_addr #2

; Function Attrs: mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite)
declare void @free(ptr allocptr nocapture noundef) local_unnamed_addr #4

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #5

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #6

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nounwind willreturn memory(argmem: readwrite, inaccessiblemem: readwrite) uwtable
define dso_local void @ref_freeifmaddrs(ptr nocapture noundef %ifmp) local_unnamed_addr #7 {
entry:
  tail call void @free(ptr noundef %ifmp)
  ret void
}

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #6 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #7 = { mustprogress nounwind willreturn memory(argmem: readwrite, inaccessiblemem: readwrite) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #8 = { nounwind }
attributes #9 = { allocsize(0) }

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
!10 = !{!"long", !7, i64 0}
!11 = distinct !{!11, !12}
!12 = !{!"llvm.loop.mustprogress"}
!13 = !{!14, !7, i64 2}
!14 = !{!"rt_msghdr", !15, i64 0, !7, i64 2, !7, i64 3, !15, i64 4, !15, i64 6, !6, i64 8, !6, i64 12, !6, i64 16, !6, i64 20, !6, i64 24, !6, i64 28, !10, i64 32, !16, i64 40}
!15 = !{!"short", !7, i64 0}
!16 = !{!"rt_metrics", !10, i64 0, !10, i64 8, !10, i64 16, !10, i64 24, !10, i64 32, !10, i64 40, !10, i64 48, !10, i64 56, !10, i64 64, !10, i64 72, !10, i64 80, !10, i64 88, !7, i64 96}
!17 = !{!14, !7, i64 3}
!18 = !{!19, !6, i64 4}
!19 = !{!"ifma_msghdr", !15, i64 0, !7, i64 2, !7, i64 3, !6, i64 4, !6, i64 8, !15, i64 12, !15, i64 14}
!20 = !{!21, !7, i64 0}
!21 = !{!"sockaddr", !7, i64 0, !7, i64 1, !7, i64 2}
!22 = !{!14, !15, i64 0}
!23 = distinct !{!23, !12}
!24 = !{!25, !26, i64 24}
!25 = !{!"ifmaddrs", !26, i64 0, !26, i64 8, !26, i64 16, !26, i64 24}
!26 = !{!"any pointer", !7, i64 0}
!27 = !{!25, !26, i64 8}
!28 = !{!25, !26, i64 16}
!29 = !{!25, !26, i64 0}
!30 = distinct !{!30, !12}
!31 = !{!26, !26, i64 0}
