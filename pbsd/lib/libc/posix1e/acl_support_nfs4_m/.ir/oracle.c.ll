; ModuleID = '/home/odin/pbsd/pbsd/lib/libc/posix1e/acl_support_nfs4_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/libc/posix1e/acl_support_nfs4_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

module asm ".symver __qsort_r_compat, qsort_r@FBSD_1.0"

%struct.flagnames_struct = type { i32, ptr, i8 }

@.str = private unnamed_addr constant [13 x i8] c"file_inherit\00", align 1
@.str.1 = private unnamed_addr constant [12 x i8] c"dir_inherit\00", align 1
@.str.2 = private unnamed_addr constant [13 x i8] c"inherit_only\00", align 1
@.str.3 = private unnamed_addr constant [13 x i8] c"no_propagate\00", align 1
@.str.4 = private unnamed_addr constant [19 x i8] c"successfull_access\00", align 1
@.str.5 = private unnamed_addr constant [14 x i8] c"failed_access\00", align 1
@.str.6 = private unnamed_addr constant [10 x i8] c"inherited\00", align 1
@a_flags = dso_local global [8 x %struct.flagnames_struct] [%struct.flagnames_struct { i32 1, ptr @.str, i8 102 }, %struct.flagnames_struct { i32 2, ptr @.str.1, i8 100 }, %struct.flagnames_struct { i32 8, ptr @.str.2, i8 105 }, %struct.flagnames_struct { i32 4, ptr @.str.3, i8 110 }, %struct.flagnames_struct { i32 16, ptr @.str.4, i8 83 }, %struct.flagnames_struct { i32 32, ptr @.str.5, i8 70 }, %struct.flagnames_struct { i32 128, ptr @.str.6, i8 73 }, %struct.flagnames_struct zeroinitializer], align 16
@.str.7 = private unnamed_addr constant [10 x i8] c"read_data\00", align 1
@.str.8 = private unnamed_addr constant [11 x i8] c"write_data\00", align 1
@.str.9 = private unnamed_addr constant [8 x i8] c"execute\00", align 1
@.str.10 = private unnamed_addr constant [12 x i8] c"append_data\00", align 1
@.str.11 = private unnamed_addr constant [13 x i8] c"delete_child\00", align 1
@.str.12 = private unnamed_addr constant [7 x i8] c"delete\00", align 1
@.str.13 = private unnamed_addr constant [16 x i8] c"read_attributes\00", align 1
@.str.14 = private unnamed_addr constant [17 x i8] c"write_attributes\00", align 1
@.str.15 = private unnamed_addr constant [11 x i8] c"read_xattr\00", align 1
@.str.16 = private unnamed_addr constant [12 x i8] c"write_xattr\00", align 1
@.str.17 = private unnamed_addr constant [9 x i8] c"read_acl\00", align 1
@.str.18 = private unnamed_addr constant [10 x i8] c"write_acl\00", align 1
@.str.19 = private unnamed_addr constant [12 x i8] c"write_owner\00", align 1
@.str.20 = private unnamed_addr constant [12 x i8] c"synchronize\00", align 1
@.str.21 = private unnamed_addr constant [9 x i8] c"full_set\00", align 1
@.str.22 = private unnamed_addr constant [11 x i8] c"modify_set\00", align 1
@.str.23 = private unnamed_addr constant [9 x i8] c"read_set\00", align 1
@.str.24 = private unnamed_addr constant [10 x i8] c"write_set\00", align 1
@a_access_masks = dso_local global [19 x %struct.flagnames_struct] [%struct.flagnames_struct { i32 8, ptr @.str.7, i8 114 }, %struct.flagnames_struct { i32 16, ptr @.str.8, i8 119 }, %struct.flagnames_struct { i32 1, ptr @.str.9, i8 120 }, %struct.flagnames_struct { i32 32, ptr @.str.10, i8 112 }, %struct.flagnames_struct { i32 256, ptr @.str.11, i8 68 }, %struct.flagnames_struct { i32 2048, ptr @.str.12, i8 100 }, %struct.flagnames_struct { i32 512, ptr @.str.13, i8 97 }, %struct.flagnames_struct { i32 1024, ptr @.str.14, i8 65 }, %struct.flagnames_struct { i32 64, ptr @.str.15, i8 82 }, %struct.flagnames_struct { i32 128, ptr @.str.16, i8 87 }, %struct.flagnames_struct { i32 4096, ptr @.str.17, i8 99 }, %struct.flagnames_struct { i32 8192, ptr @.str.18, i8 67 }, %struct.flagnames_struct { i32 16384, ptr @.str.19, i8 111 }, %struct.flagnames_struct { i32 32768, ptr @.str.20, i8 115 }, %struct.flagnames_struct { i32 65529, ptr @.str.21, i8 0 }, %struct.flagnames_struct { i32 40953, ptr @.str.22, i8 0 }, %struct.flagnames_struct { i32 4680, ptr @.str.23, i8 0 }, %struct.flagnames_struct { i32 1200, ptr @.str.24, i8 0 }, %struct.flagnames_struct zeroinitializer], align 16
@.str.25 = private unnamed_addr constant [6 x i8] c"flags\00", align 1
@.str.26 = private unnamed_addr constant [19 x i8] c"access permissions\00", align 1
@.str.27 = private unnamed_addr constant [4 x i8] c"%s/\00", align 1
@__func__.ref_format_flags_verbose = private unnamed_addr constant [25 x i8] c"ref_format_flags_verbose\00", align 1
@.str.28 = private unnamed_addr constant [66 x i8] c"/home/odin/pbsd/pbsd/lib/libc/posix1e/acl_support_nfs4_m/oracle.c\00", align 1
@.str.29 = private unnamed_addr constant [11 x i8] c"off < size\00", align 1
@.str.30 = private unnamed_addr constant [16 x i8] c"str[off] == '/'\00", align 1
@__func__.ref_format_flags_compact = private unnamed_addr constant [25 x i8] c"ref_format_flags_compact\00", align 1
@.str.31 = private unnamed_addr constant [9 x i8] c"i < size\00", align 1
@.str.32 = private unnamed_addr constant [3 x i8] c"/:\00", align 1
@.str.33 = private unnamed_addr constant [53 x i8] c"malformed ACL: \22%s\22 field contains invalid flag \22%s\22\00", align 1
@.str.34 = private unnamed_addr constant [53 x i8] c"malformed ACL: \22%s\22 field contains invalid flag \22%c\22\00", align 1

; Function Attrs: nounwind uwtable
define dso_local noundef i32 @ref__nfs4_format_flags(ptr nocapture noundef %str, i64 noundef %size, i16 noundef zeroext %var, i32 noundef %verbose) local_unnamed_addr #0 {
entry:
  %tobool.not = icmp eq i32 %verbose, 0
  %conv1 = zext i16 %var to i32
  br i1 %tobool.not, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  tail call fastcc void @ref_format_flags_verbose(ptr noundef %str, i64 noundef %size, i32 noundef %conv1, ptr noundef nonnull @a_flags)
  br label %return

if.end:                                           ; preds = %entry
  %0 = load i8, ptr getelementptr inbounds ([8 x %struct.flagnames_struct], ptr @a_flags, i64 0, i64 0, i32 2), align 16, !tbaa !5
  %cmp.not25.i = icmp eq i8 %0, 0
  br i1 %cmp.not25.i, label %ref_format_flags_compact.exit, label %for.body.i

for.body.i:                                       ; preds = %if.end, %cond.end.i
  %1 = phi i8 [ %4, %cond.end.i ], [ %0, %if.end ]
  %arrayidx27.i = phi ptr [ %arrayidx.i, %cond.end.i ], [ @a_flags, %if.end ]
  %i.026.i = phi i64 [ %inc.i, %cond.end.i ], [ 0, %if.end ]
  %exitcond.not.i = icmp eq i64 %i.026.i, %size
  br i1 %exitcond.not.i, label %cond.false.i, label %cond.end.i

cond.false.i:                                     ; preds = %for.body.i
  tail call void @__assert(ptr noundef nonnull @__func__.ref_format_flags_compact, ptr noundef nonnull @.str.28, i32 noundef 124, ptr noundef nonnull @.str.31) #8
  unreachable

cond.end.i:                                       ; preds = %for.body.i
  %2 = load i32, ptr %arrayidx27.i, align 8, !tbaa !11
  %and.i = and i32 %2, %conv1
  %cmp5.i = icmp eq i32 %and.i, 0
  %spec.select.i = select i1 %cmp5.i, i8 45, i8 %1
  %3 = getelementptr inbounds i8, ptr %str, i64 %i.026.i
  store i8 %spec.select.i, ptr %3, align 1
  %inc.i = add i64 %i.026.i, 1
  %arrayidx.i = getelementptr inbounds %struct.flagnames_struct, ptr @a_flags, i64 %inc.i
  %letter.i = getelementptr inbounds %struct.flagnames_struct, ptr @a_flags, i64 %inc.i, i32 2
  %4 = load i8, ptr %letter.i, align 8, !tbaa !5
  %cmp.not.i = icmp eq i8 %4, 0
  br i1 %cmp.not.i, label %ref_format_flags_compact.exit, label %for.body.i, !llvm.loop !12

ref_format_flags_compact.exit:                    ; preds = %cond.end.i, %if.end
  %i.0.lcssa.i = phi i64 [ 0, %if.end ], [ %inc.i, %cond.end.i ]
  %arrayidx11.i = getelementptr inbounds i8, ptr %str, i64 %i.0.lcssa.i
  store i8 0, ptr %arrayidx11.i, align 1, !tbaa !14
  br label %return

return:                                           ; preds = %ref_format_flags_compact.exit, %if.then
  ret i32 0
}

; Function Attrs: nounwind uwtable
define internal fastcc void @ref_format_flags_verbose(ptr nocapture noundef %str, i64 noundef %size, i32 noundef %var, ptr nocapture noundef readonly %flags) unnamed_addr #0 {
entry:
  %name13.i = getelementptr inbounds %struct.flagnames_struct, ptr %flags, i64 0, i32 1
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %var.addr.0 = phi i32 [ %var, %entry ], [ %and3.i, %while.body ]
  %off.0 = phi i64 [ 0, %entry ], [ %add, %while.body ]
  %0 = load ptr, ptr %name13.i, align 8, !tbaa !15
  %cmp.not14.i = icmp eq ptr %0, null
  br i1 %cmp.not14.i, label %while.end, label %for.body.i.preheader

for.body.i.preheader:                             ; preds = %while.cond
  %1 = load i32, ptr %flags, align 8, !tbaa !11
  %and.i32 = and i32 %1, %var.addr.0
  %cmp1.i33 = icmp eq i32 %and.i32, 0
  br i1 %cmp1.i33, label %for.cond.i, label %while.body

for.cond.i:                                       ; preds = %for.body.i.preheader, %for.body.i
  %flags.addr.015.i34 = phi ptr [ %incdec.ptr.i, %for.body.i ], [ %flags, %for.body.i.preheader ]
  %name.i = getelementptr inbounds %struct.flagnames_struct, ptr %flags.addr.015.i34, i64 1, i32 1
  %2 = load ptr, ptr %name.i, align 8, !tbaa !15
  %cmp.not.i = icmp eq ptr %2, null
  br i1 %cmp.not.i, label %while.end, label %for.body.i, !llvm.loop !16

for.body.i:                                       ; preds = %for.cond.i
  %incdec.ptr.i = getelementptr inbounds %struct.flagnames_struct, ptr %flags.addr.015.i34, i64 1
  %3 = load i32, ptr %incdec.ptr.i, align 8, !tbaa !11
  %and.i = and i32 %3, %var.addr.0
  %cmp1.i = icmp eq i32 %and.i, 0
  br i1 %cmp1.i, label %for.cond.i, label %while.body, !llvm.loop !16

while.body:                                       ; preds = %for.body.i, %for.body.i.preheader
  %.lcssa27 = phi ptr [ %0, %for.body.i.preheader ], [ %2, %for.body.i ]
  %.lcssa = phi i32 [ %1, %for.body.i.preheader ], [ %3, %for.body.i ]
  %not.i = xor i32 %.lcssa, -1
  %and3.i = and i32 %var.addr.0, %not.i
  %add.ptr = getelementptr inbounds i8, ptr %str, i64 %off.0
  %sub = sub i64 %size, %off.0
  %call1 = tail call i32 (ptr, i64, ptr, ...) @snprintf(ptr noundef %add.ptr, i64 noundef %sub, ptr noundef nonnull @.str.27, ptr noundef nonnull %.lcssa27)
  %conv = sext i32 %call1 to i64
  %add = add i64 %off.0, %conv
  %cmp2 = icmp ult i64 %add, %size
  br i1 %cmp2, label %while.cond, label %cond.false, !llvm.loop !17

cond.false:                                       ; preds = %while.body
  tail call void @__assert(ptr noundef nonnull @__func__.ref_format_flags_verbose, ptr noundef nonnull @.str.28, i32 noundef 102, ptr noundef nonnull @.str.29) #8
  unreachable

while.end:                                        ; preds = %while.cond, %for.cond.i
  %cmp4.not = icmp eq i64 %off.0, 0
  br i1 %cmp4.not, label %if.end, label %if.then

if.then:                                          ; preds = %while.end
  %dec = add i64 %off.0, -1
  %arrayidx = getelementptr inbounds i8, ptr %str, i64 %dec
  %4 = load i8, ptr %arrayidx, align 1, !tbaa !14
  %cmp7 = icmp eq i8 %4, 47
  br i1 %cmp7, label %if.end, label %cond.false10

cond.false10:                                     ; preds = %if.then
  tail call void @__assert(ptr noundef nonnull @__func__.ref_format_flags_verbose, ptr noundef nonnull @.str.28, i32 noundef 109, ptr noundef nonnull @.str.30) #8
  unreachable

if.end:                                           ; preds = %if.then, %while.end
  %off.1 = phi i64 [ %dec, %if.then ], [ 0, %while.end ]
  %arrayidx12 = getelementptr inbounds i8, ptr %str, i64 %off.1
  store i8 0, ptr %arrayidx12, align 1, !tbaa !14
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local noundef i32 @ref__nfs4_format_access_mask(ptr nocapture noundef %str, i64 noundef %size, i32 noundef %var, i32 noundef %verbose) local_unnamed_addr #0 {
entry:
  %tobool.not = icmp eq i32 %verbose, 0
  br i1 %tobool.not, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  tail call fastcc void @ref_format_flags_verbose(ptr noundef %str, i64 noundef %size, i32 noundef %var, ptr noundef nonnull @a_access_masks)
  br label %return

if.end:                                           ; preds = %entry
  %0 = load i8, ptr getelementptr inbounds ([19 x %struct.flagnames_struct], ptr @a_access_masks, i64 0, i64 0, i32 2), align 16, !tbaa !5
  %cmp.not25.i = icmp eq i8 %0, 0
  br i1 %cmp.not25.i, label %ref_format_flags_compact.exit, label %for.body.i

for.body.i:                                       ; preds = %if.end, %cond.end.i
  %1 = phi i8 [ %4, %cond.end.i ], [ %0, %if.end ]
  %arrayidx27.i = phi ptr [ %arrayidx.i, %cond.end.i ], [ @a_access_masks, %if.end ]
  %i.026.i = phi i64 [ %inc.i, %cond.end.i ], [ 0, %if.end ]
  %exitcond.not.i = icmp eq i64 %i.026.i, %size
  br i1 %exitcond.not.i, label %cond.false.i, label %cond.end.i

cond.false.i:                                     ; preds = %for.body.i
  tail call void @__assert(ptr noundef nonnull @__func__.ref_format_flags_compact, ptr noundef nonnull @.str.28, i32 noundef 124, ptr noundef nonnull @.str.31) #8
  unreachable

cond.end.i:                                       ; preds = %for.body.i
  %2 = load i32, ptr %arrayidx27.i, align 8, !tbaa !11
  %and.i = and i32 %2, %var
  %cmp5.i = icmp eq i32 %and.i, 0
  %spec.select.i = select i1 %cmp5.i, i8 45, i8 %1
  %3 = getelementptr inbounds i8, ptr %str, i64 %i.026.i
  store i8 %spec.select.i, ptr %3, align 1
  %inc.i = add i64 %i.026.i, 1
  %arrayidx.i = getelementptr inbounds %struct.flagnames_struct, ptr @a_access_masks, i64 %inc.i
  %letter.i = getelementptr inbounds %struct.flagnames_struct, ptr @a_access_masks, i64 %inc.i, i32 2
  %4 = load i8, ptr %letter.i, align 8, !tbaa !5
  %cmp.not.i = icmp eq i8 %4, 0
  br i1 %cmp.not.i, label %ref_format_flags_compact.exit, label %for.body.i, !llvm.loop !12

ref_format_flags_compact.exit:                    ; preds = %cond.end.i, %if.end
  %i.0.lcssa.i = phi i64 [ 0, %if.end ], [ %inc.i, %cond.end.i ]
  %arrayidx11.i = getelementptr inbounds i8, ptr %str, i64 %i.0.lcssa.i
  store i8 0, ptr %arrayidx11.i, align 1, !tbaa !14
  br label %return

return:                                           ; preds = %ref_format_flags_compact.exit, %if.then
  ret i32 0
}

; Function Attrs: nounwind uwtable
define dso_local noundef i32 @ref__nfs4_parse_flags(ptr nocapture noundef readonly %str, ptr nocapture noundef writeonly %flags) local_unnamed_addr #0 {
entry:
  %try_compact = alloca i32, align 4
  %tmpflags = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %try_compact) #9
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %tmpflags) #9
  %call = call fastcc i32 @ref_parse_flags_verbose(ptr noundef %str, ptr noundef nonnull %tmpflags, ptr noundef nonnull @a_flags, ptr noundef nonnull @.str.25, ptr noundef nonnull %try_compact), !range !18
  %tobool = icmp ne i32 %call, 0
  %0 = load i32, ptr %try_compact, align 4
  %tobool1 = icmp ne i32 %0, 0
  %or.cond = select i1 %tobool, i1 %tobool1, i1 false
  br i1 %or.cond, label %if.then, label %entry.if.end_crit_edge

entry.if.end_crit_edge:                           ; preds = %entry
  %.pre = load i32, ptr %tmpflags, align 4, !tbaa !19
  br label %if.end

if.then:                                          ; preds = %entry
  %1 = load ptr, ptr getelementptr inbounds ([8 x %struct.flagnames_struct], ptr @a_flags, i64 0, i64 0, i32 1), align 8
  %cmp12.not51.i = icmp eq ptr %1, null
  br label %for.cond.i

for.cond.i:                                       ; preds = %for.inc31.i, %if.then
  %or.i10 = phi i32 [ %or.i9, %for.inc31.i ], [ 0, %if.then ]
  %2 = phi i32 [ %7, %for.inc31.i ], [ 0, %if.then ]
  %indvars.iv62.i = phi i64 [ %indvars.iv.next63.i, %for.inc31.i ], [ 0, %if.then ]
  %arrayidx.i = getelementptr inbounds i8, ptr %str, i64 %indvars.iv62.i
  %3 = load i8, ptr %arrayidx.i, align 1, !tbaa !14
  switch i8 %3, label %for.cond9.preheader.i [
    i8 0, label %if.end
    i8 45, label %for.inc31.i
  ]

for.cond9.preheader.i:                            ; preds = %for.cond.i
  br i1 %cmp12.not51.i, label %if.then26.i, label %for.body.i

for.cond9.i:                                      ; preds = %for.body.i
  %indvars.iv.next.i = add nuw i64 %indvars.iv.i, 1
  %name.i = getelementptr inbounds %struct.flagnames_struct, ptr @a_flags, i64 %indvars.iv.next.i, i32 1
  %4 = load ptr, ptr %name.i, align 8, !tbaa !15
  %cmp12.not.i = icmp eq ptr %4, null
  br i1 %cmp12.not.i, label %if.then26.i, label %for.body.i, !llvm.loop !20

for.body.i:                                       ; preds = %for.cond9.preheader.i, %for.cond9.i
  %indvars.iv.i = phi i64 [ %indvars.iv.next.i, %for.cond9.i ], [ 0, %for.cond9.preheader.i ]
  %letter.i = getelementptr inbounds %struct.flagnames_struct, ptr @a_flags, i64 %indvars.iv.i, i32 2
  %5 = load i8, ptr %letter.i, align 8, !tbaa !5
  %cmp20.i = icmp eq i8 %5, %3
  br i1 %cmp20.i, label %if.then22.i, label %for.cond9.i

if.then22.i:                                      ; preds = %for.body.i
  %arrayidx11.le.i = getelementptr inbounds %struct.flagnames_struct, ptr @a_flags, i64 %indvars.iv.i
  %6 = load i32, ptr %arrayidx11.le.i, align 8, !tbaa !11
  %or.i = or i32 %6, %2
  br label %for.inc31.i

if.then26.i:                                      ; preds = %for.cond9.preheader.i, %for.cond9.i
  %conv29.i = sext i8 %3 to i32
  tail call void (ptr, ...) @warnx(ptr noundef nonnull @.str.34, ptr noundef nonnull @.str.25, i32 noundef %conv29.i) #9
  br label %if.end

for.inc31.i:                                      ; preds = %if.then22.i, %for.cond.i
  %or.i9 = phi i32 [ %or.i, %if.then22.i ], [ %or.i10, %for.cond.i ]
  %7 = phi i32 [ %or.i, %if.then22.i ], [ %2, %for.cond.i ]
  %indvars.iv.next63.i = add nuw i64 %indvars.iv62.i, 1
  br label %for.cond.i

if.end:                                           ; preds = %for.cond.i, %entry.if.end_crit_edge, %if.then26.i
  %8 = phi i32 [ %.pre, %entry.if.end_crit_edge ], [ %or.i10, %if.then26.i ], [ %or.i10, %for.cond.i ]
  %error.0 = phi i32 [ %call, %entry.if.end_crit_edge ], [ -1, %if.then26.i ], [ 0, %for.cond.i ]
  %conv = trunc i32 %8 to i16
  store i16 %conv, ptr %flags, align 2, !tbaa !21
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %tmpflags) #9
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %try_compact) #9
  ret i32 %error.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nounwind uwtable
define internal fastcc noundef i32 @ref_parse_flags_verbose(ptr nocapture noundef readonly %strp, ptr nocapture noundef %var, ptr nocapture noundef readonly %flags, ptr noundef %flags_name, ptr nocapture noundef writeonly %try_compact) unnamed_addr #0 {
entry:
  %str = alloca ptr, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %str) #9
  %call = tail call noalias ptr @strdup(ptr noundef %strp)
  store ptr %call, ptr %str, align 8, !tbaa !23
  store i32 0, ptr %try_compact, align 4, !tbaa !19
  store i32 0, ptr %var, align 4, !tbaa !19
  %cmp.not37 = icmp eq ptr %call, null
  br i1 %cmp.not37, label %cleanup, label %while.body.lr.ph

while.body.lr.ph:                                 ; preds = %entry
  %name30 = getelementptr inbounds %struct.flagnames_struct, ptr %flags, i64 0, i32 1
  br label %while.body

while.condthread-pre-split:                       ; preds = %for.end
  %.pr = load ptr, ptr %str, align 8, !tbaa !23
  %cmp.not = icmp eq ptr %.pr, null
  br i1 %cmp.not, label %cleanup, label %while.body

while.body:                                       ; preds = %while.body.lr.ph, %while.condthread-pre-split
  %ever_found.038 = phi i32 [ 0, %while.body.lr.ph ], [ %ever_found.2, %while.condthread-pre-split ]
  %call1 = call ptr @strsep(ptr noundef nonnull %str, ptr noundef nonnull @.str.32) #9
  %0 = load ptr, ptr %name30, align 8, !tbaa !15
  %cmp2.not31 = icmp eq ptr %0, null
  br i1 %cmp2.not31, label %if.then11, label %for.body

for.body:                                         ; preds = %while.body, %for.inc
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc ], [ 0, %while.body ]
  %1 = phi ptr [ %4, %for.inc ], [ %0, %while.body ]
  %arrayidx35 = phi ptr [ %arrayidx, %for.inc ], [ %flags, %while.body ]
  %ever_found.133 = phi i32 [ %ever_found.2, %for.inc ], [ %ever_found.038, %while.body ]
  %found.032 = phi i32 [ %found.1, %for.inc ], [ 0, %while.body ]
  %call6 = call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %1, ptr noundef nonnull dereferenceable(1) %call1) #10
  %cmp7 = icmp eq i32 %call6, 0
  br i1 %cmp7, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %2 = load i32, ptr %arrayidx35, align 8, !tbaa !11
  %3 = load i32, ptr %var, align 4, !tbaa !19
  %or = or i32 %3, %2
  store i32 %or, ptr %var, align 4, !tbaa !19
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %found.1 = phi i32 [ 1, %if.then ], [ %found.032, %for.body ]
  %ever_found.2 = phi i32 [ 1, %if.then ], [ %ever_found.133, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx = getelementptr inbounds %struct.flagnames_struct, ptr %flags, i64 %indvars.iv.next
  %name = getelementptr inbounds %struct.flagnames_struct, ptr %flags, i64 %indvars.iv.next, i32 1
  %4 = load ptr, ptr %name, align 8, !tbaa !15
  %cmp2.not = icmp eq ptr %4, null
  br i1 %cmp2.not, label %for.end, label %for.body, !llvm.loop !24

for.end:                                          ; preds = %for.inc
  %tobool.not = icmp eq i32 %found.1, 0
  br i1 %tobool.not, label %if.then11, label %while.condthread-pre-split, !llvm.loop !25

if.then11:                                        ; preds = %while.body, %for.end
  %ever_found.1.lcssa45 = phi i32 [ %ever_found.2, %for.end ], [ %ever_found.038, %while.body ]
  %tobool12.not = icmp eq i32 %ever_found.1.lcssa45, 0
  br i1 %tobool12.not, label %if.else, label %if.then13

if.then13:                                        ; preds = %if.then11
  call void (ptr, ...) @warnx(ptr noundef nonnull @.str.33, ptr noundef %flags_name, ptr noundef %call1) #9
  br label %cleanup

if.else:                                          ; preds = %if.then11
  store i32 1, ptr %try_compact, align 4, !tbaa !19
  br label %cleanup

cleanup:                                          ; preds = %while.condthread-pre-split, %entry, %if.then13, %if.else
  %retval.0 = phi i32 [ -1, %if.else ], [ -1, %if.then13 ], [ 0, %entry ], [ 0, %while.condthread-pre-split ]
  call void @free(ptr noundef %call)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %str) #9
  ret i32 %retval.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nounwind uwtable
define dso_local noundef i32 @ref__nfs4_parse_access_mask(ptr nocapture noundef readonly %str, ptr nocapture noundef writeonly %perms) local_unnamed_addr #0 {
entry:
  %try_compact = alloca i32, align 4
  %tmpperms = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %try_compact) #9
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %tmpperms) #9
  %call = call fastcc i32 @ref_parse_flags_verbose(ptr noundef %str, ptr noundef nonnull %tmpperms, ptr noundef nonnull @a_access_masks, ptr noundef nonnull @.str.26, ptr noundef nonnull %try_compact), !range !18
  %tobool = icmp ne i32 %call, 0
  %0 = load i32, ptr %try_compact, align 4
  %tobool1 = icmp ne i32 %0, 0
  %or.cond = select i1 %tobool, i1 %tobool1, i1 false
  br i1 %or.cond, label %if.then, label %entry.if.end_crit_edge

entry.if.end_crit_edge:                           ; preds = %entry
  %.pre = load i32, ptr %tmpperms, align 4, !tbaa !19
  br label %if.end

if.then:                                          ; preds = %entry
  %1 = load ptr, ptr getelementptr inbounds ([19 x %struct.flagnames_struct], ptr @a_access_masks, i64 0, i64 0, i32 1), align 8
  %cmp12.not51.i = icmp eq ptr %1, null
  br label %for.cond.i

for.cond.i:                                       ; preds = %for.inc31.i, %if.then
  %or.i10 = phi i32 [ %or.i9, %for.inc31.i ], [ 0, %if.then ]
  %2 = phi i32 [ %7, %for.inc31.i ], [ 0, %if.then ]
  %indvars.iv62.i = phi i64 [ %indvars.iv.next63.i, %for.inc31.i ], [ 0, %if.then ]
  %arrayidx.i = getelementptr inbounds i8, ptr %str, i64 %indvars.iv62.i
  %3 = load i8, ptr %arrayidx.i, align 1, !tbaa !14
  switch i8 %3, label %for.cond9.preheader.i [
    i8 0, label %if.end
    i8 45, label %for.inc31.i
  ]

for.cond9.preheader.i:                            ; preds = %for.cond.i
  br i1 %cmp12.not51.i, label %if.then26.i, label %for.body.i

for.cond9.i:                                      ; preds = %for.body.i
  %indvars.iv.next.i = add nuw i64 %indvars.iv.i, 1
  %name.i = getelementptr inbounds %struct.flagnames_struct, ptr @a_access_masks, i64 %indvars.iv.next.i, i32 1
  %4 = load ptr, ptr %name.i, align 8, !tbaa !15
  %cmp12.not.i = icmp eq ptr %4, null
  br i1 %cmp12.not.i, label %if.then26.i, label %for.body.i, !llvm.loop !20

for.body.i:                                       ; preds = %for.cond9.preheader.i, %for.cond9.i
  %indvars.iv.i = phi i64 [ %indvars.iv.next.i, %for.cond9.i ], [ 0, %for.cond9.preheader.i ]
  %letter.i = getelementptr inbounds %struct.flagnames_struct, ptr @a_access_masks, i64 %indvars.iv.i, i32 2
  %5 = load i8, ptr %letter.i, align 8, !tbaa !5
  %cmp20.i = icmp eq i8 %5, %3
  br i1 %cmp20.i, label %if.then22.i, label %for.cond9.i

if.then22.i:                                      ; preds = %for.body.i
  %arrayidx11.le.i = getelementptr inbounds %struct.flagnames_struct, ptr @a_access_masks, i64 %indvars.iv.i
  %6 = load i32, ptr %arrayidx11.le.i, align 8, !tbaa !11
  %or.i = or i32 %6, %2
  br label %for.inc31.i

if.then26.i:                                      ; preds = %for.cond9.preheader.i, %for.cond9.i
  %conv29.i = sext i8 %3 to i32
  tail call void (ptr, ...) @warnx(ptr noundef nonnull @.str.34, ptr noundef nonnull @.str.26, i32 noundef %conv29.i) #9
  br label %if.end

for.inc31.i:                                      ; preds = %if.then22.i, %for.cond.i
  %or.i9 = phi i32 [ %or.i, %if.then22.i ], [ %or.i10, %for.cond.i ]
  %7 = phi i32 [ %or.i, %if.then22.i ], [ %2, %for.cond.i ]
  %indvars.iv.next63.i = add nuw i64 %indvars.iv62.i, 1
  br label %for.cond.i

if.end:                                           ; preds = %for.cond.i, %entry.if.end_crit_edge, %if.then26.i
  %8 = phi i32 [ %.pre, %entry.if.end_crit_edge ], [ %or.i10, %if.then26.i ], [ %or.i10, %for.cond.i ]
  %error.0 = phi i32 [ %call, %entry.if.end_crit_edge ], [ -1, %if.then26.i ], [ 0, %for.cond.i ]
  store i32 %8, ptr %perms, align 4, !tbaa !19
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %tmpperms) #9
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %try_compact) #9
  ret i32 %error.0
}

; Function Attrs: nofree nounwind
declare noundef i32 @snprintf(ptr noalias nocapture noundef writeonly, i64 noundef, ptr nocapture noundef readonly, ...) local_unnamed_addr #2

; Function Attrs: noreturn
declare void @__assert(ptr noundef, ptr noundef, i32 noundef, ptr noundef) local_unnamed_addr #3

; Function Attrs: mustprogress nofree nounwind willreturn memory(argmem: readwrite, inaccessiblemem: readwrite)
declare noalias ptr @strdup(ptr nocapture noundef readonly) local_unnamed_addr #4

declare ptr @strsep(ptr noundef, ptr noundef) local_unnamed_addr #5

; Function Attrs: mustprogress nofree nounwind willreturn memory(argmem: read)
declare i32 @strcmp(ptr nocapture noundef, ptr nocapture noundef) local_unnamed_addr #6

declare void @warnx(ptr noundef, ...) local_unnamed_addr #5

; Function Attrs: mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite)
declare void @free(ptr allocptr nocapture noundef) local_unnamed_addr #7

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { nofree nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { noreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { mustprogress nofree nounwind willreturn memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { mustprogress nofree nounwind willreturn memory(argmem: read) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #7 = { mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #8 = { noreturn nounwind }
attributes #9 = { nounwind }
attributes #10 = { nounwind willreturn memory(read) }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!5 = !{!6, !8, i64 16}
!6 = !{!"flagnames_struct", !7, i64 0, !10, i64 8, !8, i64 16}
!7 = !{!"int", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{!"any pointer", !8, i64 0}
!11 = !{!6, !7, i64 0}
!12 = distinct !{!12, !13}
!13 = !{!"llvm.loop.mustprogress"}
!14 = !{!8, !8, i64 0}
!15 = !{!6, !10, i64 8}
!16 = distinct !{!16, !13}
!17 = distinct !{!17, !13}
!18 = !{i32 -1, i32 1}
!19 = !{!7, !7, i64 0}
!20 = distinct !{!20, !13}
!21 = !{!22, !22, i64 0}
!22 = !{!"short", !8, i64 0}
!23 = !{!10, !10, i64 0}
!24 = distinct !{!24, !13}
!25 = distinct !{!25, !13}
