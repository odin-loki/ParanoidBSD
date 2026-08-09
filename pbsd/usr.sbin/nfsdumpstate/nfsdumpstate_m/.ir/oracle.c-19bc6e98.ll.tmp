; ModuleID = '/home/odin/pbsd/pbsd/usr.sbin/nfsdumpstate/nfsdumpstate_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/usr.sbin/nfsdumpstate/nfsdumpstate_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

module asm ".symver __qsort_r_compat, qsort_r@FBSD_1.0"

%struct.nfsd_dumpclients = type { i32, i32, i32, i32, i32, i32, i32, i8, %union.anon, %struct.nfsd_clid }
%union.anon = type { %struct.in6_addr }
%struct.in6_addr = type { %union.anon.0 }
%union.anon.0 = type { [4 x i32] }
%struct.nfsd_clid = type { i32, [1024 x i8] }
%struct.nfsd_dumplocks = type { i32, %struct.nfsv4stateid, i64, i64, %struct.nfsd_clid, i8, %union.anon.1, %struct.nfsd_clid }
%struct.nfsv4stateid = type { i32, [3 x i32] }
%union.anon.1 = type { %struct.in6_addr }
%struct.nfsd_dumplocklist = type { ptr, i32, ptr }
%struct.nfsd_dumplist = type { i32, ptr }

@.str = private unnamed_addr constant [5 x i8] c"nfsd\00", align 1
@.str.1 = private unnamed_addr constant [35 x i8] c"nfsd not loaded - self terminating\00", align 1
@.str.2 = private unnamed_addr constant [4 x i8] c"ol:\00", align 1
@optarg = external local_unnamed_addr global ptr, align 8
@.str.3 = private unnamed_addr constant [35 x i8] c"-o and -l cannot both be specified\00", align 1
@.str.4 = private unnamed_addr constant [30 x i8] c"usage: nfsdumpstate [-o] [-l]\00", align 1
@dp = internal global [10000 x %struct.nfsd_dumpclients] zeroinitializer, align 16
@.str.5 = private unnamed_addr constant [35 x i8] c"Can't perform dump clients syscall\00", align 1
@.str.6 = private unnamed_addr constant [52 x i8] c"%-13s %9.9s %9.9s %9.9s %9.9s %9.9s %9.9s %-45s %s\0A\00", align 1
@.str.7 = private unnamed_addr constant [6 x i8] c"Flags\00", align 1
@.str.8 = private unnamed_addr constant [10 x i8] c"OpenOwner\00", align 1
@.str.9 = private unnamed_addr constant [5 x i8] c"Open\00", align 1
@.str.10 = private unnamed_addr constant [10 x i8] c"LockOwner\00", align 1
@.str.11 = private unnamed_addr constant [5 x i8] c"Lock\00", align 1
@.str.12 = private unnamed_addr constant [6 x i8] c"Deleg\00", align 1
@.str.13 = private unnamed_addr constant [9 x i8] c"OldDeleg\00", align 1
@.str.14 = private unnamed_addr constant [11 x i8] c"Clientaddr\00", align 1
@.str.15 = private unnamed_addr constant [9 x i8] c"ClientID\00", align 1
@.str.16 = private unnamed_addr constant [7 x i8] c"%-13s \00", align 1
@.str.17 = private unnamed_addr constant [25 x i8] c"%9d %9d %9d %9d %9d %9d \00", align 1
@.str.18 = private unnamed_addr constant [7 x i8] c"%-45s \00", align 1
@.str.19 = private unnamed_addr constant [2 x i8] c" \00", align 1
@.str.20 = private unnamed_addr constant [5 x i8] c"%02x\00", align 1
@flag_string = internal global [20 x i8] zeroinitializer, align 16
@.str.22 = private unnamed_addr constant [4 x i8] c"NC \00", align 1
@.str.23 = private unnamed_addr constant [4 x i8] c"CB \00", align 1
@.str.24 = private unnamed_addr constant [5 x i8] c"GSS \00", align 1
@.str.25 = private unnamed_addr constant [4 x i8] c"REV\00", align 1
@lp = internal global [10000 x %struct.nfsd_dumplocks] zeroinitializer, align 16
@.str.26 = private unnamed_addr constant [25 x i8] c"Can't dump locks for %s\0A\00", align 1
@.str.27 = private unnamed_addr constant [22 x i8] c"%-11s %-36s %-45s %s\0A\00", align 1
@.str.28 = private unnamed_addr constant [10 x i8] c"Open/Lock\00", align 1
@.str.29 = private unnamed_addr constant [32 x i8] c"          Stateid or Lock Range\00", align 1
@.str.30 = private unnamed_addr constant [19 x i8] c"Owner and ClientID\00", align 1
@.str.31 = private unnamed_addr constant [26 x i8] c"%-11s %9d %08x %08x %08x \00", align 1
@.str.32 = private unnamed_addr constant [20 x i8] c"%-11s  %17jd %17jd \00", align 1
@.str.33 = private unnamed_addr constant [3 x i8] c"  \00", align 1
@.str.34 = private unnamed_addr constant [6 x i8] c"Open \00", align 1
@.str.37 = private unnamed_addr constant [7 x i8] c"Lock R\00", align 1
@.str.38 = private unnamed_addr constant [7 x i8] c"Lock W\00", align 1

; Function Attrs: noreturn nounwind uwtable
define dso_local noundef i32 @ref_main(i32 noundef %argc, ptr noundef %argv) local_unnamed_addr #0 {
entry:
  %dumplocklist.i = alloca %struct.nfsd_dumplocklist, align 8
  %dumplist.i = alloca %struct.nfsd_dumplist, align 8
  %call = tail call i32 @modfind(ptr noundef nonnull @.str) #8
  %cmp = icmp slt i32 %call, 0
  br i1 %cmp, label %if.then, label %while.cond.outer

if.then:                                          ; preds = %entry
  tail call void (i32, ptr, ...) @errx(i32 noundef 1, ptr noundef nonnull @.str.1) #9
  unreachable

while.cond:                                       ; preds = %while.cond.outer, %sw.bb3
  %lockfile.0 = phi ptr [ %0, %sw.bb3 ], [ %lockfile.0.ph, %while.cond.outer ]
  %call1 = tail call i32 @getopt(i32 noundef %argc, ptr noundef %argv, ptr noundef nonnull @.str.2) #8
  switch i32 %call1, label %sw.default [
    i32 -1, label %while.end
    i32 111, label %while.cond.outer
    i32 108, label %sw.bb3
  ], !llvm.loop !5

while.cond.outer:                                 ; preds = %while.cond, %entry
  %cmp4 = phi i1 [ true, %entry ], [ false, %while.cond ]
  %cmp7 = phi i1 [ false, %entry ], [ true, %while.cond ]
  %lockfile.0.ph = phi ptr [ null, %entry ], [ %lockfile.0, %while.cond ]
  br label %while.cond

sw.bb3:                                           ; preds = %while.cond
  %0 = load ptr, ptr @optarg, align 8, !tbaa !7
  br label %while.cond, !llvm.loop !5

sw.default:                                       ; preds = %while.cond
  tail call fastcc void @ref_usage() #10
  unreachable

while.end:                                        ; preds = %while.cond
  %cmp5 = icmp eq ptr %lockfile.0, null
  %or.cond = select i1 %cmp4, i1 %cmp5, i1 false
  br i1 %or.cond, label %if.then14, label %if.else

if.else:                                          ; preds = %while.end
  %cmp9 = icmp ne ptr %lockfile.0, null
  %or.cond17 = select i1 %cmp7, i1 %cmp9, i1 false
  br i1 %or.cond17, label %if.then10, label %if.end12

if.then10:                                        ; preds = %if.else
  tail call void (i32, ptr, ...) @errx(i32 noundef 1, ptr noundef nonnull @.str.3) #9
  unreachable

if.end12:                                         ; preds = %if.else
  br i1 %cmp4, label %if.else15, label %if.then14

if.then14:                                        ; preds = %while.end, %if.end12
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %dumplist.i) #8
  store i32 10000, ptr %dumplist.i, align 8, !tbaa !11
  %ndl_list.i = getelementptr inbounds %struct.nfsd_dumplist, ptr %dumplist.i, i64 0, i32 1
  store ptr @dp, ptr %ndl_list.i, align 8, !tbaa !14
  %call.i = call i32 @nfssvc(i32 noundef 65536, ptr noundef nonnull %dumplist.i) #8
  %cmp.i = icmp slt i32 %call.i, 0
  br i1 %cmp.i, label %if.then.i, label %if.end.i

if.then.i:                                        ; preds = %if.then14
  call void (i32, ptr, ...) @errx(i32 noundef 1, ptr noundef nonnull @.str.5) #9
  unreachable

if.end.i:                                         ; preds = %if.then14
  %call1.i = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.6, ptr noundef nonnull @.str.7, ptr noundef nonnull @.str.8, ptr noundef nonnull @.str.9, ptr noundef nonnull @.str.10, ptr noundef nonnull @.str.11, ptr noundef nonnull @.str.12, ptr noundef nonnull @.str.13, ptr noundef nonnull @.str.14, ptr noundef nonnull @.str.15)
  %1 = load i32, ptr getelementptr inbounds ([10000 x %struct.nfsd_dumpclients], ptr @dp, i64 0, i64 0, i32 9), align 16, !tbaa !15
  %cmp254.i = icmp sgt i32 %1, 0
  br i1 %cmp254.i, label %while.body.i, label %ref_dump_openstate.exit

while.body.i:                                     ; preds = %if.end.i, %for.end.i
  %indvars.iv59.i = phi i64 [ %indvars.iv.next60.i, %for.end.i ], [ 0, %if.end.i ]
  %ndcl_clid57.i = phi ptr [ %ndcl_clid.i, %for.end.i ], [ getelementptr inbounds ([10000 x %struct.nfsd_dumpclients], ptr @dp, i64 0, i64 0, i32 9), %if.end.i ]
  %arrayidx.i = getelementptr inbounds [10000 x %struct.nfsd_dumpclients], ptr @dp, i64 0, i64 %indvars.iv59.i
  %2 = load i32, ptr %arrayidx.i, align 4, !tbaa !18
  store i8 0, ptr @flag_string, align 16, !tbaa !19
  %and.i.i = and i32 %2, 1
  %tobool.not.i.i = icmp eq i32 %and.i.i, 0
  br i1 %tobool.not.i.i, label %if.end.i.i, label %if.then.i.i

if.then.i.i:                                      ; preds = %while.body.i
  %call.i.i = call i64 @strlcat(ptr noundef nonnull @flag_string, ptr noundef nonnull @.str.22, i64 noundef 20) #8
  br label %if.end.i.i

if.end.i.i:                                       ; preds = %if.then.i.i, %while.body.i
  %and1.i.i = and i32 %2, 16
  %tobool2.not.i.i = icmp eq i32 %and1.i.i, 0
  br i1 %tobool2.not.i.i, label %if.end5.i.i, label %if.then3.i.i

if.then3.i.i:                                     ; preds = %if.end.i.i
  %call4.i.i = call i64 @strlcat(ptr noundef nonnull @flag_string, ptr noundef nonnull @.str.23, i64 noundef 20) #8
  br label %if.end5.i.i

if.end5.i.i:                                      ; preds = %if.then3.i.i, %if.end.i.i
  %and6.i.i = and i32 %2, 1024
  %tobool7.not.i.i = icmp eq i32 %and6.i.i, 0
  br i1 %tobool7.not.i.i, label %if.end10.i.i, label %if.then8.i.i

if.then8.i.i:                                     ; preds = %if.end5.i.i
  %call9.i.i = call i64 @strlcat(ptr noundef nonnull @flag_string, ptr noundef nonnull @.str.24, i64 noundef 20) #8
  br label %if.end10.i.i

if.end10.i.i:                                     ; preds = %if.then8.i.i, %if.end5.i.i
  %and11.i.i = and i32 %2, 32768
  %tobool12.not.i.i = icmp eq i32 %and11.i.i, 0
  br i1 %tobool12.not.i.i, label %ref_client_flags.exit.i, label %if.then13.i.i

if.then13.i.i:                                    ; preds = %if.end10.i.i
  %call14.i.i = call i64 @strlcat(ptr noundef nonnull @flag_string, ptr noundef nonnull @.str.25, i64 noundef 20) #8
  br label %ref_client_flags.exit.i

ref_client_flags.exit.i:                          ; preds = %if.then13.i.i, %if.end10.i.i
  %call7.i = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.16, ptr noundef nonnull @flag_string)
  %ndcl_nopenowners.i = getelementptr inbounds [10000 x %struct.nfsd_dumpclients], ptr @dp, i64 0, i64 %indvars.iv59.i, i32 1
  %3 = load i32, ptr %ndcl_nopenowners.i, align 4, !tbaa !20
  %ndcl_nopens.i = getelementptr inbounds [10000 x %struct.nfsd_dumpclients], ptr @dp, i64 0, i64 %indvars.iv59.i, i32 2
  %4 = load i32, ptr %ndcl_nopens.i, align 4, !tbaa !21
  %ndcl_nlockowners.i = getelementptr inbounds [10000 x %struct.nfsd_dumpclients], ptr @dp, i64 0, i64 %indvars.iv59.i, i32 3
  %5 = load i32, ptr %ndcl_nlockowners.i, align 4, !tbaa !22
  %ndcl_nlocks.i = getelementptr inbounds [10000 x %struct.nfsd_dumpclients], ptr @dp, i64 0, i64 %indvars.iv59.i, i32 4
  %6 = load i32, ptr %ndcl_nlocks.i, align 4, !tbaa !23
  %ndcl_ndelegs.i = getelementptr inbounds [10000 x %struct.nfsd_dumpclients], ptr @dp, i64 0, i64 %indvars.iv59.i, i32 5
  %7 = load i32, ptr %ndcl_ndelegs.i, align 4, !tbaa !24
  %ndcl_nolddelegs.i = getelementptr inbounds [10000 x %struct.nfsd_dumpclients], ptr @dp, i64 0, i64 %indvars.iv59.i, i32 6
  %8 = load i32, ptr %ndcl_nolddelegs.i, align 4, !tbaa !25
  %call20.i = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.17, i32 noundef %3, i32 noundef %4, i32 noundef %5, i32 noundef %6, i32 noundef %7, i32 noundef %8)
  %call23.i = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.18, ptr noundef nonnull @.str.19)
  %9 = load i32, ptr %ndcl_clid57.i, align 4, !tbaa !15
  %cmp2852.i = icmp sgt i32 %9, 0
  br i1 %cmp2852.i, label %for.body.i, label %for.end.i

for.body.i:                                       ; preds = %ref_client_flags.exit.i, %for.body.i
  %indvars.iv.i = phi i64 [ %indvars.iv.next.i, %for.body.i ], [ 0, %ref_client_flags.exit.i ]
  %arrayidx34.i = getelementptr inbounds [10000 x %struct.nfsd_dumpclients], ptr @dp, i64 0, i64 %indvars.iv59.i, i32 9, i32 1, i64 %indvars.iv.i
  %10 = load i8, ptr %arrayidx34.i, align 1, !tbaa !19
  %conv35.i = zext i8 %10 to i32
  %call36.i = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.20, i32 noundef %conv35.i)
  %indvars.iv.next.i = add nuw nsw i64 %indvars.iv.i, 1
  %11 = load i32, ptr %ndcl_clid57.i, align 4, !tbaa !15
  %12 = sext i32 %11 to i64
  %cmp28.i = icmp slt i64 %indvars.iv.next.i, %12
  br i1 %cmp28.i, label %for.body.i, label %for.end.i, !llvm.loop !26

for.end.i:                                        ; preds = %for.body.i, %ref_client_flags.exit.i
  %putchar.i = call i32 @putchar(i32 10)
  %indvars.iv.next60.i = add nuw nsw i64 %indvars.iv59.i, 1
  %ndcl_clid.i = getelementptr inbounds [10000 x %struct.nfsd_dumpclients], ptr @dp, i64 0, i64 %indvars.iv.next60.i, i32 9
  %13 = load i32, ptr %ndcl_clid.i, align 4, !tbaa !15
  %cmp2.i = icmp sgt i32 %13, 0
  %cmp3.i = icmp ult i64 %indvars.iv59.i, 9999
  %14 = and i1 %cmp3.i, %cmp2.i
  br i1 %14, label %while.body.i, label %ref_dump_openstate.exit, !llvm.loop !27

ref_dump_openstate.exit:                          ; preds = %for.end.i, %if.end.i
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %dumplist.i) #8
  br label %if.end16

if.else15:                                        ; preds = %if.end12
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %dumplocklist.i) #8
  %ndllck_size.i = getelementptr inbounds %struct.nfsd_dumplocklist, ptr %dumplocklist.i, i64 0, i32 1
  store i32 10000, ptr %ndllck_size.i, align 8, !tbaa !28
  %ndllck_list.i = getelementptr inbounds %struct.nfsd_dumplocklist, ptr %dumplocklist.i, i64 0, i32 2
  store ptr @lp, ptr %ndllck_list.i, align 8, !tbaa !30
  store ptr %lockfile.0, ptr %dumplocklist.i, align 8, !tbaa !31
  %call.i24 = call i32 @nfssvc(i32 noundef 131072, ptr noundef nonnull %dumplocklist.i) #8
  %cmp.i25 = icmp slt i32 %call.i24, 0
  br i1 %cmp.i25, label %if.then.i44, label %if.end.i26

if.then.i44:                                      ; preds = %if.else15
  call void (i32, ptr, ...) @errx(i32 noundef 1, ptr noundef nonnull @.str.26, ptr noundef %lockfile.0) #9
  unreachable

if.end.i26:                                       ; preds = %if.else15
  %call1.i27 = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.27, ptr noundef nonnull @.str.28, ptr noundef nonnull @.str.29, ptr noundef nonnull @.str.14, ptr noundef nonnull @.str.30)
  %15 = load i32, ptr getelementptr inbounds ([10000 x %struct.nfsd_dumplocks], ptr @lp, i64 0, i64 0, i32 7), align 16, !tbaa !32
  %cmp2145.i = icmp sgt i32 %15, 0
  br i1 %cmp2145.i, label %while.body.i28, label %ref_dump_lockstate.exit

while.body.i28:                                   ; preds = %if.end.i26, %for.end104.i
  %indvars.iv153.i = phi i64 [ %indvars.iv.next154.i, %for.end104.i ], [ 0, %if.end.i26 ]
  %ndlck_clid148.i = phi ptr [ %ndlck_clid.i, %for.end104.i ], [ getelementptr inbounds ([10000 x %struct.nfsd_dumplocks], ptr @lp, i64 0, i64 0, i32 7), %if.end.i26 ]
  %arrayidx.i29 = getelementptr inbounds [10000 x %struct.nfsd_dumplocks], ptr @lp, i64 0, i64 %indvars.iv153.i
  %16 = load i32, ptr %arrayidx.i29, align 8, !tbaa !36
  %and.i = and i32 %16, 4096
  %tobool.not.i = icmp eq i32 %and.i, 0
  br i1 %tobool.not.i, label %if.else.i, label %if.then6.i

if.then6.i:                                       ; preds = %while.body.i28
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 16 dereferenceable(6) @flag_string, ptr noundef nonnull align 1 dereferenceable(6) @.str.34, i64 noundef 6, i1 false) #8
  %and.i.i30 = and i32 %16, 1
  %tobool.not.i.i31 = icmp eq i32 %and.i.i30, 0
  br i1 %tobool.not.i.i31, label %if.end.i.i33, label %if.then.i.i32

if.then.i.i32:                                    ; preds = %if.then6.i
  store i8 82, ptr getelementptr inbounds ([20 x i8], ptr @flag_string, i64 0, i64 5), align 1, !tbaa !19
  br label %if.end.i.i33

if.end.i.i33:                                     ; preds = %if.then.i.i32, %if.then6.i
  %i.0.i.i = phi i32 [ 6, %if.then.i.i32 ], [ 5, %if.then6.i ]
  %and1.i.i34 = and i32 %16, 2
  %tobool2.not.i.i35 = icmp eq i32 %and1.i.i34, 0
  br i1 %tobool2.not.i.i35, label %if.end7.i.i, label %if.then3.i.i36

if.then3.i.i36:                                   ; preds = %if.end.i.i33
  %inc4.i.i = add nuw nsw i32 %i.0.i.i, 1
  %idxprom5.i.i = zext nneg i32 %i.0.i.i to i64
  %arrayidx6.i.i = getelementptr inbounds [20 x i8], ptr @flag_string, i64 0, i64 %idxprom5.i.i
  store i8 87, ptr %arrayidx6.i.i, align 1, !tbaa !19
  br label %if.end7.i.i

if.end7.i.i:                                      ; preds = %if.then3.i.i36, %if.end.i.i33
  %i.1.i.i = phi i32 [ %inc4.i.i, %if.then3.i.i36 ], [ %i.0.i.i, %if.end.i.i33 ]
  %inc8.i.i = add nuw nsw i32 %i.1.i.i, 1
  %idxprom9.i.i = zext nneg i32 %i.1.i.i to i64
  %arrayidx10.i.i = getelementptr inbounds [20 x i8], ptr @flag_string, i64 0, i64 %idxprom9.i.i
  store i8 32, ptr %arrayidx10.i.i, align 1, !tbaa !19
  %inc11.i.i = add nuw nsw i32 %i.1.i.i, 2
  %idxprom12.i.i = zext nneg i32 %inc8.i.i to i64
  %arrayidx13.i.i = getelementptr inbounds [20 x i8], ptr @flag_string, i64 0, i64 %idxprom12.i.i
  store i8 68, ptr %arrayidx13.i.i, align 1, !tbaa !19
  %idxprom14.i.i = zext nneg i32 %inc11.i.i to i64
  %arrayidx15.i.i = getelementptr inbounds [20 x i8], ptr @flag_string, i64 0, i64 %idxprom14.i.i
  %and16.i.i = and i32 %16, 4
  %tobool17.not.i.i = icmp eq i32 %and16.i.i, 0
  %inc19.i.i = add nuw nsw i32 %i.1.i.i, 3
  %spec.select.i.i = select i1 %tobool17.not.i.i, i8 78, i8 82
  %spec.select48.i.i = select i1 %tobool17.not.i.i, i32 %inc11.i.i, i32 %inc19.i.i
  store i8 %spec.select.i.i, ptr %arrayidx15.i.i, align 1, !tbaa !19
  %and23.i.i = and i32 %16, 8
  %tobool24.not.i.i = icmp eq i32 %and23.i.i, 0
  br i1 %tobool24.not.i.i, label %ref_open_flags.exit.i, label %if.then25.i.i

if.then25.i.i:                                    ; preds = %if.end7.i.i
  %inc26.i.i = add nuw nsw i32 %spec.select48.i.i, 1
  %idxprom27.i.i = zext nneg i32 %spec.select48.i.i to i64
  %arrayidx28.i.i = getelementptr inbounds [20 x i8], ptr @flag_string, i64 0, i64 %idxprom27.i.i
  store i8 87, ptr %arrayidx28.i.i, align 1, !tbaa !19
  br label %ref_open_flags.exit.i

ref_open_flags.exit.i:                            ; preds = %if.then25.i.i, %if.end7.i.i
  %i.3.i.i = phi i32 [ %inc26.i.i, %if.then25.i.i ], [ %spec.select48.i.i, %if.end7.i.i ]
  %cmp.i.i = icmp eq i32 %i.3.i.i, %inc11.i.i
  %inc31.i.i = zext i1 %cmp.i.i to i32
  %spec.select49.i.i = add nuw nsw i32 %i.3.i.i, %inc31.i.i
  %idxprom33.i.i = zext nneg i32 %spec.select49.i.i to i64
  %arrayidx34.i.i = getelementptr inbounds [20 x i8], ptr @flag_string, i64 0, i64 %idxprom33.i.i
  store i8 0, ptr %arrayidx34.i.i, align 1, !tbaa !19
  %ndlck_stateid.i = getelementptr inbounds [10000 x %struct.nfsd_dumplocks], ptr @lp, i64 0, i64 %indvars.iv153.i, i32 1
  %17 = load i32, ptr %ndlck_stateid.i, align 4, !tbaa !37
  %other.i = getelementptr inbounds [10000 x %struct.nfsd_dumplocks], ptr @lp, i64 0, i64 %indvars.iv153.i, i32 1, i32 1
  %18 = load i32, ptr %other.i, align 8, !tbaa !38
  %arrayidx21.i = getelementptr inbounds [10000 x %struct.nfsd_dumplocks], ptr @lp, i64 0, i64 %indvars.iv153.i, i32 1, i32 1, i64 1
  %19 = load i32, ptr %arrayidx21.i, align 4, !tbaa !38
  %arrayidx26.i = getelementptr inbounds [10000 x %struct.nfsd_dumplocks], ptr @lp, i64 0, i64 %indvars.iv153.i, i32 1, i32 1, i64 2
  %20 = load i32, ptr %arrayidx26.i, align 8, !tbaa !38
  %call27.i = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.31, ptr noundef nonnull @flag_string, i32 noundef %17, i32 noundef %18, i32 noundef %19, i32 noundef %20)
  br label %sw.default.i

if.else.i:                                        ; preds = %while.body.i28
  %and31.i = and i32 %16, 1572864
  %tobool32.not.i = icmp eq i32 %and31.i, 0
  br i1 %tobool32.not.i, label %if.else58.i, label %if.then33.i

if.then33.i:                                      ; preds = %if.else.i
  %and.i135.i = and i32 %16, 524288
  %tobool.not.i136.i = icmp eq i32 %and.i135.i, 0
  %..i.i = select i1 %tobool.not.i136.i, i64 24523951429150020, i64 23116576545596740
  store i64 %..i.i, ptr @flag_string, align 16
  %ndlck_stateid40.i = getelementptr inbounds [10000 x %struct.nfsd_dumplocks], ptr @lp, i64 0, i64 %indvars.iv153.i, i32 1
  %21 = load i32, ptr %ndlck_stateid40.i, align 4, !tbaa !37
  %other45.i = getelementptr inbounds [10000 x %struct.nfsd_dumplocks], ptr @lp, i64 0, i64 %indvars.iv153.i, i32 1, i32 1
  %22 = load i32, ptr %other45.i, align 8, !tbaa !38
  %arrayidx51.i = getelementptr inbounds [10000 x %struct.nfsd_dumplocks], ptr @lp, i64 0, i64 %indvars.iv153.i, i32 1, i32 1, i64 1
  %23 = load i32, ptr %arrayidx51.i, align 4, !tbaa !38
  %arrayidx56.i = getelementptr inbounds [10000 x %struct.nfsd_dumplocks], ptr @lp, i64 0, i64 %indvars.iv153.i, i32 1, i32 1, i64 2
  %24 = load i32, ptr %arrayidx56.i, align 8, !tbaa !38
  %call57.i = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.31, ptr noundef nonnull @flag_string, i32 noundef %21, i32 noundef %22, i32 noundef %23, i32 noundef %24)
  br label %sw.default.i

if.else58.i:                                      ; preds = %if.else.i
  %and.i137.i = and i32 %16, 16
  %tobool.not.i138.i = icmp eq i32 %and.i137.i, 0
  br i1 %tobool.not.i138.i, label %if.else.i.i, label %if.then.i139.i

if.then.i139.i:                                   ; preds = %if.else58.i
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 16 dereferenceable(7) @flag_string, ptr noundef nonnull align 1 dereferenceable(7) @.str.37, i64 noundef 7, i1 false) #8
  br label %ref_lock_flags.exit.i

if.else.i.i:                                      ; preds = %if.else58.i
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 16 dereferenceable(7) @flag_string, ptr noundef nonnull align 1 dereferenceable(7) @.str.38, i64 noundef 7, i1 false) #8
  br label %ref_lock_flags.exit.i

ref_lock_flags.exit.i:                            ; preds = %if.else.i.i, %if.then.i139.i
  %ndlck_first.i = getelementptr inbounds [10000 x %struct.nfsd_dumplocks], ptr @lp, i64 0, i64 %indvars.iv153.i, i32 2
  %25 = load i64, ptr %ndlck_first.i, align 8, !tbaa !39
  %ndlck_end.i = getelementptr inbounds [10000 x %struct.nfsd_dumplocks], ptr @lp, i64 0, i64 %indvars.iv153.i, i32 3
  %26 = load i64, ptr %ndlck_end.i, align 8, !tbaa !40
  %call67.i = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.32, ptr noundef nonnull @flag_string, i64 noundef %25, i64 noundef %26)
  br label %sw.default.i

sw.default.i:                                     ; preds = %ref_lock_flags.exit.i, %if.then33.i, %ref_open_flags.exit.i
  %call72.i = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.18, ptr noundef nonnull @.str.33)
  %ndlck_owner.i = getelementptr inbounds [10000 x %struct.nfsd_dumplocks], ptr @lp, i64 0, i64 %indvars.iv153.i, i32 4
  %27 = load i32, ptr %ndlck_owner.i, align 8, !tbaa !41
  %cmp76141.i = icmp sgt i32 %27, 0
  br i1 %cmp76141.i, label %for.body.i41, label %for.end.i37

for.body.i41:                                     ; preds = %sw.default.i, %for.body.i41
  %indvars.iv.i42 = phi i64 [ %indvars.iv.next.i43, %for.body.i41 ], [ 0, %sw.default.i ]
  %arrayidx82.i = getelementptr inbounds [10000 x %struct.nfsd_dumplocks], ptr @lp, i64 0, i64 %indvars.iv153.i, i32 4, i32 1, i64 %indvars.iv.i42
  %28 = load i8, ptr %arrayidx82.i, align 1, !tbaa !19
  %conv83.i = zext i8 %28 to i32
  %call84.i = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.20, i32 noundef %conv83.i)
  %indvars.iv.next.i43 = add nuw nsw i64 %indvars.iv.i42, 1
  %29 = load i32, ptr %ndlck_owner.i, align 8, !tbaa !41
  %30 = sext i32 %29 to i64
  %cmp76.i = icmp slt i64 %indvars.iv.next.i43, %30
  br i1 %cmp76.i, label %for.body.i41, label %for.end.i37, !llvm.loop !42

for.end.i37:                                      ; preds = %for.body.i41, %sw.default.i
  %putchar.i38 = call i32 @putchar(i32 32)
  %31 = load i32, ptr %ndlck_clid148.i, align 8, !tbaa !32
  %cmp91143.i = icmp sgt i32 %31, 0
  br i1 %cmp91143.i, label %for.body93.i, label %for.end104.i

for.body93.i:                                     ; preds = %for.end.i37, %for.body93.i
  %indvars.iv150.i = phi i64 [ %indvars.iv.next151.i, %for.body93.i ], [ 0, %for.end.i37 ]
  %arrayidx99.i = getelementptr inbounds [10000 x %struct.nfsd_dumplocks], ptr @lp, i64 0, i64 %indvars.iv153.i, i32 7, i32 1, i64 %indvars.iv150.i
  %32 = load i8, ptr %arrayidx99.i, align 1, !tbaa !19
  %conv100.i = zext i8 %32 to i32
  %call101.i = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.20, i32 noundef %conv100.i)
  %indvars.iv.next151.i = add nuw nsw i64 %indvars.iv150.i, 1
  %33 = load i32, ptr %ndlck_clid148.i, align 8, !tbaa !32
  %34 = sext i32 %33 to i64
  %cmp91.i = icmp slt i64 %indvars.iv.next151.i, %34
  br i1 %cmp91.i, label %for.body93.i, label %for.end104.i, !llvm.loop !43

for.end104.i:                                     ; preds = %for.body93.i, %for.end.i37
  %putchar134.i = call i32 @putchar(i32 10)
  %indvars.iv.next154.i = add nuw nsw i64 %indvars.iv153.i, 1
  %ndlck_clid.i = getelementptr inbounds [10000 x %struct.nfsd_dumplocks], ptr @lp, i64 0, i64 %indvars.iv.next154.i, i32 7
  %35 = load i32, ptr %ndlck_clid.i, align 8, !tbaa !32
  %cmp2.i39 = icmp sgt i32 %35, 0
  %cmp3.i40 = icmp ult i64 %indvars.iv153.i, 9999
  %36 = and i1 %cmp3.i40, %cmp2.i39
  br i1 %36, label %while.body.i28, label %ref_dump_lockstate.exit, !llvm.loop !44

ref_dump_lockstate.exit:                          ; preds = %for.end104.i, %if.end.i26
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %dumplocklist.i) #8
  br label %if.end16

if.end16:                                         ; preds = %ref_dump_lockstate.exit, %ref_dump_openstate.exit
  call void @exit(i32 noundef 0) #9
  unreachable
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

declare i32 @modfind(ptr noundef) local_unnamed_addr #2

; Function Attrs: noreturn
declare void @errx(i32 noundef, ptr noundef, ...) local_unnamed_addr #3

declare i32 @getopt(i32 noundef, ptr noundef, ptr noundef) local_unnamed_addr #2

; Function Attrs: noreturn nounwind uwtable
define internal fastcc void @ref_usage() unnamed_addr #0 {
entry:
  tail call void (i32, ptr, ...) @errx(i32 noundef 1, ptr noundef nonnull @.str.4) #9
  unreachable
}

; Function Attrs: noreturn
declare void @exit(i32 noundef) local_unnamed_addr #3

declare i32 @nfssvc(i32 noundef, ptr noundef) local_unnamed_addr #2

; Function Attrs: nofree nounwind
declare noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #4

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nofree
declare i64 @strlcat(ptr noundef, ptr noundef, i64 noundef) local_unnamed_addr #5

; Function Attrs: nofree nounwind
declare noundef i32 @putchar(i32 noundef) local_unnamed_addr #6

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #7

attributes #0 = { noreturn nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { noreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nofree nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { nofree "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { nofree nounwind }
attributes #7 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #8 = { nounwind }
attributes #9 = { noreturn nounwind }
attributes #10 = { noreturn }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.mustprogress"}
!7 = !{!8, !8, i64 0}
!8 = !{!"any pointer", !9, i64 0}
!9 = !{!"omnipotent char", !10, i64 0}
!10 = !{!"Simple C/C++ TBAA"}
!11 = !{!12, !13, i64 0}
!12 = !{!"nfsd_dumplist", !13, i64 0, !8, i64 8}
!13 = !{!"int", !9, i64 0}
!14 = !{!12, !8, i64 8}
!15 = !{!16, !13, i64 48}
!16 = !{!"nfsd_dumpclients", !13, i64 0, !13, i64 4, !13, i64 8, !13, i64 12, !13, i64 16, !13, i64 20, !13, i64 24, !9, i64 28, !9, i64 32, !17, i64 48}
!17 = !{!"nfsd_clid", !13, i64 0, !9, i64 4}
!18 = !{!16, !13, i64 0}
!19 = !{!9, !9, i64 0}
!20 = !{!16, !13, i64 4}
!21 = !{!16, !13, i64 8}
!22 = !{!16, !13, i64 12}
!23 = !{!16, !13, i64 16}
!24 = !{!16, !13, i64 20}
!25 = !{!16, !13, i64 24}
!26 = distinct !{!26, !6}
!27 = distinct !{!27, !6}
!28 = !{!29, !13, i64 8}
!29 = !{!"nfsd_dumplocklist", !8, i64 0, !13, i64 8, !8, i64 16}
!30 = !{!29, !8, i64 16}
!31 = !{!29, !8, i64 0}
!32 = !{!33, !13, i64 1088}
!33 = !{!"nfsd_dumplocks", !13, i64 0, !34, i64 4, !35, i64 24, !35, i64 32, !17, i64 40, !9, i64 1068, !9, i64 1072, !17, i64 1088}
!34 = !{!"nfsv4stateid", !13, i64 0, !9, i64 4}
!35 = !{!"long", !9, i64 0}
!36 = !{!33, !13, i64 0}
!37 = !{!33, !13, i64 4}
!38 = !{!13, !13, i64 0}
!39 = !{!33, !35, i64 24}
!40 = !{!33, !35, i64 32}
!41 = !{!33, !13, i64 40}
!42 = distinct !{!42, !6}
!43 = distinct !{!43, !6}
!44 = distinct !{!44, !6}
