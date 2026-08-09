; ModuleID = '/home/odin/pbsd/pbsd/lib/libc/net/rthdr_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/libc/net/rthdr_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.cmsghdr = type { i32, i32, i32 }
%struct.in6_addr = type { %union.anon }
%union.anon = type { [4 x i32] }
%struct.ip6_rthdr0 = type { i8, i8, i8, i8, i32 }
%struct.ip6_rthdr = type { i8, i8, i8, i8 }

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local i64 @ref_inet6_rthdr_space(i32 noundef %type, i32 noundef %seg) local_unnamed_addr #0 {
entry:
  %cond = icmp ne i32 %type, 0
  %0 = add i32 %seg, -24
  %or.cond = icmp ult i32 %0, -23
  %or.cond6 = or i1 %cond, %or.cond
  %1 = shl nuw nsw i32 %seg, 4
  %narrow = add nuw nsw i32 %1, 24
  %add3 = zext nneg i32 %narrow to i64
  %retval.0 = select i1 %or.cond6, i64 0, i64 %add3
  ret i64 %retval.0
}

; Function Attrs: mustprogress nofree nounwind willreturn memory(argmem: readwrite) uwtable
define dso_local noundef ptr @ref_inet6_rthdr_init(ptr noundef writeonly %bp, i32 noundef %type) local_unnamed_addr #1 {
entry:
  %cmsg_level = getelementptr inbounds %struct.cmsghdr, ptr %bp, i64 0, i32 1
  store i32 41, ptr %cmsg_level, align 4, !tbaa !5
  %cmsg_type = getelementptr inbounds %struct.cmsghdr, ptr %bp, i64 0, i32 2
  store i32 51, ptr %cmsg_type, align 4, !tbaa !10
  %cond = icmp eq i32 %type, 0
  br i1 %cond, label %sw.bb, label %cleanup

sw.bb:                                            ; preds = %entry
  %add.ptr = getelementptr inbounds i8, ptr %bp, i64 16
  store i32 24, ptr %bp, align 4, !tbaa !11
  tail call void @bzero(ptr noundef nonnull %add.ptr, i64 noundef 8)
  %ip6r_type = getelementptr inbounds i8, ptr %bp, i64 18
  store i8 0, ptr %ip6r_type, align 1, !tbaa !12
  br label %cleanup

cleanup:                                          ; preds = %entry, %sw.bb
  %retval.0 = phi ptr [ %bp, %sw.bb ], [ null, %entry ]
  ret ptr %retval.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: mustprogress nofree nounwind willreturn memory(argmem: readwrite)
declare void @bzero(ptr nocapture noundef writeonly, i64 noundef) local_unnamed_addr #3

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: readwrite) uwtable
define dso_local noundef i32 @ref_inet6_rthdr_add(ptr nocapture noundef %cmsg, ptr nocapture noundef readonly %addr, i32 noundef %flags) local_unnamed_addr #4 {
entry:
  %add.ptr = getelementptr inbounds i8, ptr %cmsg, i64 16
  %ip6r_type = getelementptr inbounds i8, ptr %cmsg, i64 18
  %0 = load i8, ptr %ip6r_type, align 1, !tbaa !12
  %cond = icmp ne i8 %0, 0
  %or.cond = icmp ugt i32 %flags, 1
  %or.cond39 = or i1 %or.cond, %cond
  br i1 %or.cond39, label %cleanup27, label %if.end

if.end:                                           ; preds = %entry
  %cmp.not = icmp ne i32 %flags, 0
  %ip6r0_segleft = getelementptr inbounds i8, ptr %cmsg, i64 19
  %1 = load i8, ptr %ip6r0_segleft, align 1, !tbaa !14
  %cmp5 = icmp eq i8 %1, 23
  %brmerge = or i1 %cmp.not, %cmp5
  br i1 %brmerge, label %cleanup27, label %2

2:                                                ; preds = %if.end
  %inc = add i8 %1, 1
  store i8 %inc, ptr %ip6r0_segleft, align 1, !tbaa !14
  %ip6r0_len = getelementptr inbounds i8, ptr %cmsg, i64 17
  %3 = load i8, ptr %ip6r0_len, align 1, !tbaa !16
  %conv14 = zext i8 %3 to i64
  %add = shl nuw nsw i64 %conv14, 3
  %4 = getelementptr i8, ptr %add.ptr, i64 %add
  %add.ptr15 = getelementptr i8, ptr %4, i64 8
  tail call void @llvm.memmove.p0.p0.i64(ptr noundef nonnull align 1 dereferenceable(16) %add.ptr15, ptr noundef nonnull align 1 dereferenceable(16) %addr, i64 16, i1 false)
  %add18 = add i8 %3, 2
  store i8 %add18, ptr %ip6r0_len, align 1, !tbaa !16
  %conv21 = zext i8 %add18 to i32
  %add22 = shl nuw nsw i32 %conv21, 3
  %add25 = add nuw nsw i32 %add22, 24
  store i32 %add25, ptr %cmsg, align 4, !tbaa !11
  br label %cleanup27

cleanup27:                                        ; preds = %if.end, %2, %entry
  %retval.1 = phi i32 [ -1, %entry ], [ 0, %2 ], [ -1, %if.end ]
  ret i32 %retval.1
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable
define dso_local i32 @ref_inet6_rthdr_lasthop(ptr nocapture noundef readonly %cmsg, i32 noundef %flags) local_unnamed_addr #5 {
entry:
  %ip6r_type = getelementptr inbounds i8, ptr %cmsg, i64 18
  %0 = load i8, ptr %ip6r_type, align 1, !tbaa !12
  %cond = icmp eq i8 %0, 0
  br i1 %cond, label %sw.bb, label %cleanup7

sw.bb:                                            ; preds = %entry
  %ip6r0_segleft = getelementptr inbounds i8, ptr %cmsg, i64 19
  %1 = load i8, ptr %ip6r0_segleft, align 1, !tbaa !14
  %cmp = icmp ugt i8 %1, 23
  %cmp3.not = icmp ne i32 %flags, 0
  %or.cond.not = or i1 %cmp3.not, %cmp
  %spec.select = sext i1 %or.cond.not to i32
  br label %cleanup7

cleanup7:                                         ; preds = %sw.bb, %entry
  %retval.1 = phi i32 [ -1, %entry ], [ %spec.select, %sw.bb ]
  ret i32 %retval.1
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable
define dso_local i32 @ref_inet6_rthdr_segments(ptr nocapture noundef readonly %cmsg) local_unnamed_addr #5 {
entry:
  %ip6r_type = getelementptr inbounds i8, ptr %cmsg, i64 18
  %0 = load i8, ptr %ip6r_type, align 1, !tbaa !12
  %cond = icmp eq i8 %0, 0
  br i1 %cond, label %sw.bb, label %cleanup9

sw.bb:                                            ; preds = %entry
  %ip6r0_len = getelementptr inbounds i8, ptr %cmsg, i64 17
  %1 = load i8, ptr %ip6r0_len, align 1, !tbaa !16
  %conv1 = zext i8 %1 to i32
  %rem = and i32 %conv1, 1
  %tobool.not = icmp ne i32 %rem, 0
  %cmp = icmp ugt i8 %1, 46
  %or.cond = or i1 %cmp, %tobool.not
  %2 = lshr exact i32 %conv1, 1
  %retval.0 = select i1 %or.cond, i32 -1, i32 %2
  br label %cleanup9

cleanup9:                                         ; preds = %entry, %sw.bb
  %retval.1 = phi i32 [ %retval.0, %sw.bb ], [ -1, %entry ]
  ret i32 %retval.1
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable
define dso_local ptr @ref_inet6_rthdr_getaddr(ptr noundef readonly %cmsg, i32 noundef %idx) local_unnamed_addr #5 {
entry:
  %ip6r_type = getelementptr inbounds i8, ptr %cmsg, i64 18
  %0 = load i8, ptr %ip6r_type, align 1, !tbaa !12
  %cond = icmp eq i8 %0, 0
  br i1 %cond, label %sw.bb, label %cleanup19

sw.bb:                                            ; preds = %entry
  %ip6r0_len = getelementptr inbounds i8, ptr %cmsg, i64 17
  %1 = load i8, ptr %ip6r0_len, align 1, !tbaa !16
  %conv1 = zext i8 %1 to i32
  %rem = and i32 %conv1, 1
  %tobool.not = icmp ne i32 %rem, 0
  %cmp = icmp ugt i8 %1, 46
  %or.cond = or i1 %cmp, %tobool.not
  br i1 %or.cond, label %cleanup19, label %if.end

if.end:                                           ; preds = %sw.bb
  %cmp9 = icmp slt i32 %idx, 1
  %2 = lshr exact i32 %conv1, 1
  %cmp12 = icmp ult i32 %2, %idx
  %or.cond26 = select i1 %cmp9, i1 true, i1 %cmp12
  br i1 %or.cond26, label %cleanup19, label %if.end15

if.end15:                                         ; preds = %if.end
  %add.ptr16 = getelementptr inbounds i8, ptr %cmsg, i64 24
  %idx.ext = zext nneg i32 %idx to i64
  %add.ptr17 = getelementptr inbounds %struct.in6_addr, ptr %add.ptr16, i64 %idx.ext
  br label %cleanup19

cleanup19:                                        ; preds = %entry, %if.end15, %sw.bb, %if.end
  %retval.1 = phi ptr [ %add.ptr17, %if.end15 ], [ null, %sw.bb ], [ null, %if.end ], [ null, %entry ]
  ret ptr %retval.1
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable
define dso_local i32 @ref_inet6_rthdr_getflags(ptr nocapture noundef readonly %cmsg, i32 noundef %idx) local_unnamed_addr #5 {
entry:
  %ip6r_type = getelementptr inbounds i8, ptr %cmsg, i64 18
  %0 = load i8, ptr %ip6r_type, align 1, !tbaa !12
  %cond = icmp eq i8 %0, 0
  br i1 %cond, label %sw.bb, label %cleanup17

sw.bb:                                            ; preds = %entry
  %ip6r0_len = getelementptr inbounds i8, ptr %cmsg, i64 17
  %1 = load i8, ptr %ip6r0_len, align 1, !tbaa !16
  %conv1 = zext i8 %1 to i32
  %rem = and i32 %conv1, 1
  %tobool.not = icmp ne i32 %rem, 0
  %cmp = icmp ugt i8 %1, 46
  %or.cond = or i1 %cmp, %tobool.not
  br i1 %or.cond, label %cleanup17, label %if.end

if.end:                                           ; preds = %sw.bb
  %cmp9 = icmp slt i32 %idx, 0
  %2 = lshr exact i32 %conv1, 1
  %cmp12 = icmp ult i32 %2, %idx
  %or.cond22 = select i1 %cmp9, i1 true, i1 %cmp12
  %spec.select = sext i1 %or.cond22 to i32
  br label %cleanup17

cleanup17:                                        ; preds = %if.end, %entry, %sw.bb
  %retval.1 = phi i32 [ -1, %sw.bb ], [ -1, %entry ], [ %spec.select, %if.end ]
  ret i32 %retval.1
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local i32 @ref_inet6_rth_space(i32 noundef %type, i32 noundef %segments) local_unnamed_addr #0 {
entry:
  %cond = icmp eq i32 %type, 0
  %or.cond = icmp ult i32 %segments, 128
  %or.cond4 = and i1 %cond, %or.cond
  %mul = shl nuw nsw i32 %segments, 4
  %add = or disjoint i32 %mul, 8
  %retval.0 = select i1 %or.cond4, i32 %add, i32 0
  ret i32 %retval.0
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: write) uwtable
define dso_local noundef ptr @ref_inet6_rth_init(ptr noundef writeonly %bp, i32 noundef %bp_len, i32 noundef %type, i32 noundef %segments) local_unnamed_addr #6 {
entry:
  %cond = icmp eq i32 %type, 0
  br i1 %cond, label %sw.bb, label %cleanup

sw.bb:                                            ; preds = %entry
  %mul.i = shl nuw nsw i32 %segments, 4
  %add.i = or disjoint i32 %mul.i, 8
  %cmp17 = icmp ugt i32 %add.i, %bp_len
  %or.cond = icmp ugt i32 %segments, 127
  %or.cond16 = select i1 %or.cond, i1 true, i1 %cmp17
  br i1 %or.cond16, label %cleanup, label %if.end4

if.end4:                                          ; preds = %sw.bb
  %conv = zext i32 %bp_len to i64
  tail call void @llvm.memset.p0.i64(ptr align 1 %bp, i8 0, i64 %conv, i1 false)
  %segments.tr = trunc i32 %segments to i8
  %conv5 = shl nuw i8 %segments.tr, 1
  %ip6r0_len = getelementptr inbounds %struct.ip6_rthdr0, ptr %bp, i64 0, i32 1
  store i8 %conv5, ptr %ip6r0_len, align 1, !tbaa !16
  %ip6r0_type = getelementptr inbounds %struct.ip6_rthdr0, ptr %bp, i64 0, i32 2
  store i8 0, ptr %ip6r0_type, align 1, !tbaa !17
  %ip6r0_segleft = getelementptr inbounds %struct.ip6_rthdr0, ptr %bp, i64 0, i32 3
  store i8 0, ptr %ip6r0_segleft, align 1, !tbaa !14
  %ip6r0_reserved = getelementptr inbounds %struct.ip6_rthdr0, ptr %bp, i64 0, i32 4
  store i32 0, ptr %ip6r0_reserved, align 1, !tbaa !18
  br label %cleanup

cleanup:                                          ; preds = %entry, %sw.bb, %if.end4
  %retval.0 = phi ptr [ %bp, %if.end4 ], [ null, %sw.bb ], [ null, %entry ]
  ret ptr %retval.0
}

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #7

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: readwrite) uwtable
define dso_local noundef i32 @ref_inet6_rth_add(ptr nocapture noundef %bp, ptr nocapture noundef readonly %addr) local_unnamed_addr #4 {
entry:
  %ip6r_type = getelementptr inbounds %struct.ip6_rthdr, ptr %bp, i64 0, i32 2
  %0 = load i8, ptr %ip6r_type, align 1, !tbaa !12
  %cond = icmp eq i8 %0, 0
  br i1 %cond, label %sw.bb, label %cleanup

sw.bb:                                            ; preds = %entry
  %ip6r0_segleft = getelementptr inbounds %struct.ip6_rthdr0, ptr %bp, i64 0, i32 3
  %1 = load i8, ptr %ip6r0_segleft, align 1, !tbaa !14
  %ip6r0_len = getelementptr inbounds %struct.ip6_rthdr0, ptr %bp, i64 0, i32 1
  %2 = load i8, ptr %ip6r0_len, align 1, !tbaa !16
  %3 = lshr i8 %2, 1
  %cmp = icmp eq i8 %1, %3
  br i1 %cmp, label %cleanup, label %if.end

if.end:                                           ; preds = %sw.bb
  %add.ptr = getelementptr inbounds %struct.ip6_rthdr0, ptr %bp, i64 1
  %idx.ext = zext i8 %1 to i64
  %add.ptr6 = getelementptr inbounds %struct.in6_addr, ptr %add.ptr, i64 %idx.ext
  tail call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 4 dereferenceable(16) %add.ptr6, ptr noundef nonnull align 4 dereferenceable(16) %addr, i64 16, i1 false), !tbaa.struct !19
  %inc = add i8 %1, 1
  store i8 %inc, ptr %ip6r0_segleft, align 1, !tbaa !14
  br label %cleanup

cleanup:                                          ; preds = %entry, %sw.bb, %if.end
  %retval.0 = phi i32 [ 0, %if.end ], [ -1, %sw.bb ], [ -1, %entry ]
  ret i32 %retval.0
}

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #8

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define dso_local noundef i32 @ref_inet6_rth_reverse(ptr nocapture noundef readonly %in, ptr nocapture noundef %out) local_unnamed_addr #9 {
entry:
  %addr_tmp.sroa.0 = alloca [4 x i32], align 4
  %ip6r_type = getelementptr inbounds %struct.ip6_rthdr, ptr %in, i64 0, i32 2
  %0 = load i8, ptr %ip6r_type, align 1, !tbaa !12
  %cond = icmp eq i8 %0, 0
  br i1 %cond, label %sw.bb, label %cleanup

sw.bb:                                            ; preds = %entry
  %ip6r0_len = getelementptr inbounds %struct.ip6_rthdr0, ptr %in, i64 0, i32 1
  %1 = load i8, ptr %ip6r0_len, align 1, !tbaa !16
  %conv1 = zext i8 %1 to i32
  %rem = and i32 %conv1, 1
  %tobool.not = icmp eq i32 %rem, 0
  br i1 %tobool.not, label %if.end, label %cleanup

if.end:                                           ; preds = %sw.bb
  %div33 = lshr exact i32 %conv1, 1
  %add = shl nuw nsw i32 %conv1, 3
  %shl = or disjoint i32 %add, 8
  %conv6 = zext nneg i32 %shl to i64
  tail call void @llvm.memmove.p0.p0.i64(ptr noundef nonnull align 1 dereferenceable(1) %out, ptr noundef nonnull align 1 dereferenceable(1) %in, i64 %conv6, i1 false)
  %conv7 = trunc i32 %div33 to i8
  %ip6r0_segleft = getelementptr inbounds %struct.ip6_rthdr0, ptr %out, i64 0, i32 3
  store i8 %conv7, ptr %ip6r0_segleft, align 1, !tbaa !14
  %cmp35.not = icmp ult i8 %1, 4
  br i1 %cmp35.not, label %cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %if.end
  %div834 = lshr i32 %conv1, 2
  %add.ptr = getelementptr inbounds %struct.ip6_rthdr0, ptr %out, i64 1
  %2 = zext nneg i32 %div33 to i64
  %wide.trip.count = zext nneg i32 %div834 to i64
  %3 = getelementptr %struct.in6_addr, ptr %add.ptr, i64 %2
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %addr_tmp.sroa.0)
  %add.ptr10 = getelementptr inbounds %struct.in6_addr, ptr %add.ptr, i64 %indvars.iv
  %4 = xor i64 %indvars.iv, -1
  %add.ptr14 = getelementptr %struct.in6_addr, ptr %3, i64 %4
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 4 dereferenceable(16) %addr_tmp.sroa.0, ptr noundef nonnull align 4 dereferenceable(16) %add.ptr10, i64 16, i1 false), !tbaa.struct !19
  tail call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 4 dereferenceable(16) %add.ptr10, ptr noundef nonnull align 4 dereferenceable(16) %add.ptr14, i64 16, i1 false), !tbaa.struct !19
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 4 dereferenceable(16) %add.ptr14, ptr noundef nonnull align 4 dereferenceable(16) %addr_tmp.sroa.0, i64 16, i1 false), !tbaa.struct !19
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %addr_tmp.sroa.0)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %cleanup, label %for.body, !llvm.loop !21

cleanup:                                          ; preds = %for.body, %if.end, %entry, %sw.bb
  %retval.0 = phi i32 [ -1, %sw.bb ], [ -1, %entry ], [ 0, %if.end ], [ 0, %for.body ]
  ret i32 %retval.0
}

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memmove.p0.p0.i64(ptr nocapture writeonly, ptr nocapture readonly, i64, i1 immarg) #8

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable
define dso_local i32 @ref_inet6_rth_segments(ptr nocapture noundef readonly %bp) local_unnamed_addr #5 {
entry:
  %ip6r_type = getelementptr inbounds %struct.ip6_rthdr, ptr %bp, i64 0, i32 2
  %0 = load i8, ptr %ip6r_type, align 1, !tbaa !12
  %cond = icmp eq i8 %0, 0
  br i1 %cond, label %sw.bb, label %cleanup

sw.bb:                                            ; preds = %entry
  %ip6r0_len = getelementptr inbounds %struct.ip6_rthdr0, ptr %bp, i64 0, i32 1
  %1 = load i8, ptr %ip6r0_len, align 1, !tbaa !16
  %conv1 = zext i8 %1 to i32
  %rem = and i32 %conv1, 1
  %cmp.not = icmp eq i32 %rem, 0
  br i1 %cmp.not, label %lor.lhs.false, label %cleanup

lor.lhs.false:                                    ; preds = %sw.bb
  %shr = lshr exact i32 %conv1, 1
  %ip6r0_segleft = getelementptr inbounds %struct.ip6_rthdr0, ptr %bp, i64 0, i32 3
  %2 = load i8, ptr %ip6r0_segleft, align 1, !tbaa !14
  %conv5 = zext i8 %2 to i32
  %cmp6 = icmp ult i32 %shr, %conv5
  %spec.select = select i1 %cmp6, i32 -1, i32 %shr
  br label %cleanup

cleanup:                                          ; preds = %lor.lhs.false, %entry, %sw.bb
  %retval.0 = phi i32 [ -1, %sw.bb ], [ -1, %entry ], [ %spec.select, %lor.lhs.false ]
  ret i32 %retval.0
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable
define dso_local ptr @ref_inet6_rth_getaddr(ptr noundef readonly %bp, i32 noundef %idx) local_unnamed_addr #5 {
entry:
  %ip6r_type = getelementptr inbounds %struct.ip6_rthdr, ptr %bp, i64 0, i32 2
  %0 = load i8, ptr %ip6r_type, align 1, !tbaa !12
  %cond = icmp eq i8 %0, 0
  br i1 %cond, label %sw.bb, label %cleanup

sw.bb:                                            ; preds = %entry
  %ip6r0_len = getelementptr inbounds %struct.ip6_rthdr0, ptr %bp, i64 0, i32 1
  %1 = load i8, ptr %ip6r0_len, align 1, !tbaa !16
  %conv1 = zext i8 %1 to i32
  %rem = and i32 %conv1, 1
  %cmp.not = icmp eq i32 %rem, 0
  br i1 %cmp.not, label %lor.lhs.false, label %cleanup

lor.lhs.false:                                    ; preds = %sw.bb
  %shr = lshr exact i32 %conv1, 1
  %ip6r0_segleft = getelementptr inbounds %struct.ip6_rthdr0, ptr %bp, i64 0, i32 3
  %2 = load i8, ptr %ip6r0_segleft, align 1, !tbaa !14
  %conv5 = zext i8 %2 to i32
  %cmp6 = icmp uge i32 %shr, %conv5
  %or.cond = icmp ugt i32 %shr, %idx
  %or.cond24 = and i1 %or.cond, %cmp6
  br i1 %or.cond24, label %if.end14, label %cleanup

if.end14:                                         ; preds = %lor.lhs.false
  %add.ptr = getelementptr inbounds %struct.ip6_rthdr0, ptr %bp, i64 1
  %idx.ext = zext nneg i32 %idx to i64
  %add.ptr15 = getelementptr inbounds %struct.in6_addr, ptr %add.ptr, i64 %idx.ext
  br label %cleanup

cleanup:                                          ; preds = %entry, %sw.bb, %lor.lhs.false, %if.end14
  %retval.0 = phi ptr [ %add.ptr15, %if.end14 ], [ null, %lor.lhs.false ], [ null, %sw.bb ], [ null, %entry ]
  ret ptr %retval.0
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nofree nounwind willreturn memory(argmem: readwrite) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #3 = { mustprogress nofree nounwind willreturn memory(argmem: readwrite) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: readwrite) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: write) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #7 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #8 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #9 = { nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!5 = !{!6, !7, i64 4}
!6 = !{!"cmsghdr", !7, i64 0, !7, i64 4, !7, i64 8}
!7 = !{!"int", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{!6, !7, i64 8}
!11 = !{!6, !7, i64 0}
!12 = !{!13, !8, i64 2}
!13 = !{!"ip6_rthdr", !8, i64 0, !8, i64 1, !8, i64 2, !8, i64 3}
!14 = !{!15, !8, i64 3}
!15 = !{!"ip6_rthdr0", !8, i64 0, !8, i64 1, !8, i64 2, !8, i64 3, !7, i64 4}
!16 = !{!15, !8, i64 1}
!17 = !{!15, !8, i64 2}
!18 = !{!15, !7, i64 4}
!19 = !{i64 0, i64 16, !20, i64 0, i64 16, !20, i64 0, i64 16, !20}
!20 = !{!8, !8, i64 0}
!21 = distinct !{!21, !22}
!22 = !{!"llvm.loop.mustprogress"}
