; ModuleID = '/home/odin/pbsd/pbsd/bin/getfacl/getfacl_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/bin/getfacl/getfacl_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

module asm ".symver __qsort_r_compat, qsort_r@FBSD_1.0"

%struct.option = type { ptr, i32, ptr, i32 }
%struct.stat = type { i64, i64, i64, i16, i16, i32, i32, i32, i64, %struct.timespec, %struct.timespec, %struct.timespec, %struct.timespec, i64, i64, i32, i32, i64, i64, [9 x i64] }
%struct.timespec = type { i64, i64 }

@.str = private unnamed_addr constant [9 x i8] c"+dhinqsv\00", align 1
@long_options = internal constant [5 x %struct.option] [%struct.option { ptr @.str.2, i32 0, ptr null, i32 100 }, %struct.option { ptr @.str.3, i32 0, ptr null, i32 110 }, %struct.option { ptr @.str.4, i32 0, ptr null, i32 113 }, %struct.option { ptr @.str.5, i32 0, ptr null, i32 115 }, %struct.option zeroinitializer], align 16
@optind = external local_unnamed_addr global i32, align 4
@.str.1 = private unnamed_addr constant [2 x i8] c"-\00", align 1
@.str.2 = private unnamed_addr constant [8 x i8] c"default\00", align 1
@.str.3 = private unnamed_addr constant [8 x i8] c"numeric\00", align 1
@.str.4 = private unnamed_addr constant [12 x i8] c"omit-header\00", align 1
@.str.5 = private unnamed_addr constant [10 x i8] c"skip-base\00", align 1
@__stderrp = external local_unnamed_addr global ptr, align 8
@.str.6 = private unnamed_addr constant [29 x i8] c"getfacl [-dhnqv] [file ...]\0A\00", align 1
@__stdinp = external local_unnamed_addr global ptr, align 8
@.str.7 = private unnamed_addr constant [18 x i8] c"%s: stat() failed\00", align 1
@.str.8 = private unnamed_addr constant [47 x i8] c"%s: there are no default entries in NFSv4 ACLs\00", align 1
@.str.9 = private unnamed_addr constant [39 x i8] c"%s: pathconf(..., _PC_ACL_NFS4) failed\00", align 1
@.str.10 = private unnamed_addr constant [3 x i8] c"%s\00", align 1
@.str.11 = private unnamed_addr constant [29 x i8] c"%s: acl_is_trivial_np failed\00", align 1
@more_than_one = internal unnamed_addr global i1 false, align 4
@.str.13 = private unnamed_addr constant [36 x i8] c"# file: %s\0A# owner: %s\0A# group: %s\0A\00", align 1
@.str.14 = private unnamed_addr constant [27 x i8] c"%s: acl_from_mode() failed\00", align 1
@.str.15 = private unnamed_addr constant [28 x i8] c"%s: acl_to_text_np() failed\00", align 1
@ref_getuname.uids = internal global [10 x i8] zeroinitializer, align 1
@.str.16 = private unnamed_addr constant [3 x i8] c"%u\00", align 1
@ref_getgname.gids = internal global [10 x i8] zeroinitializer, align 1

; Function Attrs: nounwind uwtable
define dso_local i32 @ref_main(i32 noundef %argc, ptr noundef %argv) local_unnamed_addr #0 {
entry:
  %pathname.i62 = alloca [1024 x i8], align 16
  %pathname.i = alloca [1024 x i8], align 16
  br label %while.cond.outer

while.cond.outer.loopexit:                        ; preds = %while.cond.outer135, %while.cond
  %vflag.0.lcssa139 = phi i32 [ 1, %while.cond ], [ %vflag.0.ph137, %while.cond.outer135 ]
  br label %while.cond.outer, !llvm.loop !5

while.cond.outer:                                 ; preds = %while.cond.outer.loopexit, %entry
  %type.0.ph = phi i32 [ 2, %entry ], [ 3, %while.cond.outer.loopexit ]
  %hflag.0.ph = phi i32 [ 0, %entry ], [ %hflag.0.ph114, %while.cond.outer.loopexit ]
  %iflag.0.ph = phi i32 [ 0, %entry ], [ %iflag.0.ph121, %while.cond.outer.loopexit ]
  %qflag.0.ph = phi i32 [ 0, %entry ], [ %qflag.0.ph127, %while.cond.outer.loopexit ]
  %nflag.0.ph = phi i32 [ 0, %entry ], [ %nflag.0.ph132, %while.cond.outer.loopexit ]
  %sflag.0.ph = phi i32 [ 0, %entry ], [ %sflag.0.ph136, %while.cond.outer.loopexit ]
  %vflag.0.ph = phi i32 [ 0, %entry ], [ %vflag.0.lcssa139, %while.cond.outer.loopexit ]
  br label %while.cond.outer113

while.cond.outer113:                              ; preds = %while.cond.outer, %sw.bb1
  %hflag.0.ph114 = phi i32 [ %hflag.0.ph, %while.cond.outer ], [ 1, %sw.bb1 ]
  %iflag.0.ph115 = phi i32 [ %iflag.0.ph, %while.cond.outer ], [ %iflag.0.ph121, %sw.bb1 ]
  %qflag.0.ph116 = phi i32 [ %qflag.0.ph, %while.cond.outer ], [ %qflag.0.ph127, %sw.bb1 ]
  %nflag.0.ph117 = phi i32 [ %nflag.0.ph, %while.cond.outer ], [ %nflag.0.ph132, %sw.bb1 ]
  %sflag.0.ph118 = phi i32 [ %sflag.0.ph, %while.cond.outer ], [ %sflag.0.ph136, %sw.bb1 ]
  %vflag.0.ph119 = phi i32 [ %vflag.0.ph, %while.cond.outer ], [ %vflag.0.lcssa140, %sw.bb1 ]
  br label %while.cond.outer120

while.cond.outer120:                              ; preds = %while.cond.outer113, %sw.bb2
  %iflag.0.ph121 = phi i32 [ %iflag.0.ph115, %while.cond.outer113 ], [ 1, %sw.bb2 ]
  %qflag.0.ph122 = phi i32 [ %qflag.0.ph116, %while.cond.outer113 ], [ %qflag.0.ph127, %sw.bb2 ]
  %nflag.0.ph123 = phi i32 [ %nflag.0.ph117, %while.cond.outer113 ], [ %nflag.0.ph132, %sw.bb2 ]
  %sflag.0.ph124 = phi i32 [ %sflag.0.ph118, %while.cond.outer113 ], [ %sflag.0.ph136, %sw.bb2 ]
  %vflag.0.ph125 = phi i32 [ %vflag.0.ph119, %while.cond.outer113 ], [ %vflag.0.lcssa141, %sw.bb2 ]
  br label %while.cond.outer126

while.cond.outer126:                              ; preds = %while.cond.outer120, %sw.bb4
  %qflag.0.ph127 = phi i32 [ %qflag.0.ph122, %while.cond.outer120 ], [ 1, %sw.bb4 ]
  %nflag.0.ph128 = phi i32 [ %nflag.0.ph123, %while.cond.outer120 ], [ %nflag.0.ph132, %sw.bb4 ]
  %sflag.0.ph129 = phi i32 [ %sflag.0.ph124, %while.cond.outer120 ], [ %sflag.0.ph136, %sw.bb4 ]
  %vflag.0.ph130 = phi i32 [ %vflag.0.ph125, %while.cond.outer120 ], [ %vflag.0.lcssa143, %sw.bb4 ]
  br label %while.cond.outer131

while.cond.outer131:                              ; preds = %while.cond.outer126, %sw.bb3
  %nflag.0.ph132 = phi i32 [ %nflag.0.ph128, %while.cond.outer126 ], [ 1, %sw.bb3 ]
  %sflag.0.ph133 = phi i32 [ %sflag.0.ph129, %while.cond.outer126 ], [ %sflag.0.ph136, %sw.bb3 ]
  %vflag.0.ph134 = phi i32 [ %vflag.0.ph130, %while.cond.outer126 ], [ %vflag.0.lcssa142, %sw.bb3 ]
  br label %while.cond.outer135

while.cond.outer135:                              ; preds = %while.cond.outer131, %sw.bb5
  %sflag.0.ph136 = phi i32 [ %sflag.0.ph133, %while.cond.outer131 ], [ 1, %sw.bb5 ]
  %vflag.0.ph137 = phi i32 [ %vflag.0.ph134, %while.cond.outer131 ], [ %vflag.0.lcssa144, %sw.bb5 ]
  %call.peel = tail call i32 @getopt_long(i32 noundef %argc, ptr noundef %argv, ptr noundef nonnull @.str, ptr noundef nonnull @long_options, ptr noundef null) #6
  switch i32 %call.peel, label %sw.default [
    i32 -1, label %while.end
    i32 100, label %while.cond.outer.loopexit
    i32 104, label %sw.bb1
    i32 105, label %sw.bb2
    i32 110, label %sw.bb3
    i32 113, label %sw.bb4
    i32 115, label %sw.bb5
    i32 118, label %while.cond
  ]

while.cond:                                       ; preds = %while.cond, %while.cond.outer135
  %call = tail call i32 @getopt_long(i32 noundef %argc, ptr noundef %argv, ptr noundef nonnull @.str, ptr noundef nonnull @long_options, ptr noundef null) #6
  switch i32 %call, label %sw.default [
    i32 -1, label %while.end
    i32 100, label %while.cond.outer.loopexit
    i32 104, label %sw.bb1
    i32 105, label %sw.bb2
    i32 110, label %sw.bb3
    i32 113, label %sw.bb4
    i32 115, label %sw.bb5
    i32 118, label %while.cond
  ], !llvm.loop !7

sw.bb1:                                           ; preds = %while.cond.outer135, %while.cond
  %vflag.0.lcssa140 = phi i32 [ 1, %while.cond ], [ %vflag.0.ph137, %while.cond.outer135 ]
  br label %while.cond.outer113, !llvm.loop !5

sw.bb2:                                           ; preds = %while.cond.outer135, %while.cond
  %vflag.0.lcssa141 = phi i32 [ 1, %while.cond ], [ %vflag.0.ph137, %while.cond.outer135 ]
  br label %while.cond.outer120, !llvm.loop !5

sw.bb3:                                           ; preds = %while.cond.outer135, %while.cond
  %vflag.0.lcssa142 = phi i32 [ 1, %while.cond ], [ %vflag.0.ph137, %while.cond.outer135 ]
  br label %while.cond.outer131, !llvm.loop !5

sw.bb4:                                           ; preds = %while.cond.outer135, %while.cond
  %vflag.0.lcssa143 = phi i32 [ 1, %while.cond ], [ %vflag.0.ph137, %while.cond.outer135 ]
  br label %while.cond.outer126, !llvm.loop !5

sw.bb5:                                           ; preds = %while.cond, %while.cond.outer135
  %vflag.0.lcssa144 = phi i32 [ %vflag.0.ph137, %while.cond.outer135 ], [ 1, %while.cond ]
  br label %while.cond.outer135, !llvm.loop !5

sw.default:                                       ; preds = %while.cond.outer135, %while.cond
  %0 = load ptr, ptr @__stderrp, align 8, !tbaa !9
  %1 = tail call i64 @fwrite(ptr nonnull @.str.6, i64 28, i64 1, ptr %0)
  br label %cleanup

while.end:                                        ; preds = %while.cond.outer135, %while.cond
  %vflag.0.lcssa138 = phi i32 [ 1, %while.cond ], [ %vflag.0.ph137, %while.cond.outer135 ]
  %2 = load i32, ptr @optind, align 4, !tbaa !13
  %sub = sub i32 %argc, %2
  %idx.ext = sext i32 %2 to i64
  %add.ptr = getelementptr inbounds ptr, ptr %argv, i64 %idx.ext
  %cmp7 = icmp eq i32 %2, %argc
  br i1 %cmp7, label %if.then, label %for.cond.preheader

for.cond.preheader:                               ; preds = %while.end
  %cmp988 = icmp sgt i32 %sub, 0
  br i1 %cmp988, label %for.body.preheader, label %cleanup

for.body.preheader:                               ; preds = %for.cond.preheader
  %wide.trip.count = zext nneg i32 %sub to i64
  br label %for.body

if.then:                                          ; preds = %while.end
  call void @llvm.lifetime.start.p0(i64 1024, ptr nonnull %pathname.i) #6
  %3 = load ptr, ptr @__stdinp, align 8, !tbaa !9
  %call8.i = call ptr @fgets(ptr noundef nonnull %pathname.i, i32 noundef 1024, ptr noundef %3)
  %tobool.not9.i = icmp eq ptr %call8.i, null
  br i1 %tobool.not9.i, label %ref_print_acl_from_stdin.exit, label %while.body.i

while.body.i:                                     ; preds = %if.then, %if.end.i
  %carried_error.010.i = phi i32 [ %spec.select.i, %if.end.i ], [ 0, %if.then ]
  %call2.i = call ptr @strchr(ptr noundef nonnull dereferenceable(1) %pathname.i, i32 noundef 10) #7
  %cmp.not.i = icmp eq ptr %call2.i, null
  br i1 %cmp.not.i, label %if.end.i, label %if.then.i

if.then.i:                                        ; preds = %while.body.i
  store i8 0, ptr %call2.i, align 1, !tbaa !15
  br label %if.end.i

if.end.i:                                         ; preds = %if.then.i, %while.body.i
  %call4.i = call fastcc i32 @ref_print_acl(ptr noundef nonnull %pathname.i, i32 noundef %type.0.ph, i32 noundef %hflag.0.ph114, i32 noundef %iflag.0.ph121, i32 noundef %nflag.0.ph132, i32 noundef %qflag.0.ph127, i32 noundef %vflag.0.lcssa138, i32 noundef %sflag.0.ph136), !range !16
  %cmp5.i = icmp eq i32 %call4.i, -1
  %spec.select.i = select i1 %cmp5.i, i32 -1, i32 %carried_error.010.i
  %4 = load ptr, ptr @__stdinp, align 8, !tbaa !9
  %call.i = call ptr @fgets(ptr noundef nonnull %pathname.i, i32 noundef 1024, ptr noundef %4)
  %tobool.not.i = icmp eq ptr %call.i, null
  br i1 %tobool.not.i, label %ref_print_acl_from_stdin.exit.loopexit, label %while.body.i, !llvm.loop !17

ref_print_acl_from_stdin.exit.loopexit:           ; preds = %if.end.i
  %5 = icmp ne i32 %spec.select.i, 0
  %6 = zext i1 %5 to i32
  br label %ref_print_acl_from_stdin.exit

ref_print_acl_from_stdin.exit:                    ; preds = %ref_print_acl_from_stdin.exit.loopexit, %if.then
  %carried_error.0.lcssa.i = phi i32 [ 0, %if.then ], [ %6, %ref_print_acl_from_stdin.exit.loopexit ]
  call void @llvm.lifetime.end.p0(i64 1024, ptr nonnull %pathname.i) #6
  br label %cleanup

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %carried_error.089 = phi i32 [ 0, %for.body.preheader ], [ %carried_error.1, %for.inc ]
  %arrayidx = getelementptr inbounds ptr, ptr %add.ptr, i64 %indvars.iv
  %7 = load ptr, ptr %arrayidx, align 8, !tbaa !9
  %call10 = call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %7, ptr noundef nonnull dereferenceable(2) @.str.1) #7
  %tobool11.not = icmp eq i32 %call10, 0
  br i1 %tobool11.not, label %if.then12, label %if.else

if.then12:                                        ; preds = %for.body
  call void @llvm.lifetime.start.p0(i64 1024, ptr nonnull %pathname.i62) #6
  %8 = load ptr, ptr @__stdinp, align 8, !tbaa !9
  %call8.i63 = call ptr @fgets(ptr noundef nonnull %pathname.i62, i32 noundef 1024, ptr noundef %8)
  %tobool.not9.i64 = icmp eq ptr %call8.i63, null
  br i1 %tobool.not9.i64, label %ref_print_acl_from_stdin.exit77.thread, label %while.body.i65

ref_print_acl_from_stdin.exit77.thread:           ; preds = %if.then12
  call void @llvm.lifetime.end.p0(i64 1024, ptr nonnull %pathname.i62) #6
  br label %for.inc

while.body.i65:                                   ; preds = %if.then12, %if.end.i70
  %carried_error.010.i66 = phi i32 [ %spec.select.i73, %if.end.i70 ], [ 0, %if.then12 ]
  %call2.i67 = call ptr @strchr(ptr noundef nonnull dereferenceable(1) %pathname.i62, i32 noundef 10) #7
  %cmp.not.i68 = icmp eq ptr %call2.i67, null
  br i1 %cmp.not.i68, label %if.end.i70, label %if.then.i69

if.then.i69:                                      ; preds = %while.body.i65
  store i8 0, ptr %call2.i67, align 1, !tbaa !15
  br label %if.end.i70

if.end.i70:                                       ; preds = %if.then.i69, %while.body.i65
  %call4.i71 = call fastcc i32 @ref_print_acl(ptr noundef nonnull %pathname.i62, i32 noundef %type.0.ph, i32 noundef %hflag.0.ph114, i32 noundef %iflag.0.ph121, i32 noundef %nflag.0.ph132, i32 noundef %qflag.0.ph127, i32 noundef %vflag.0.lcssa138, i32 noundef %sflag.0.ph136), !range !16
  %cmp5.i72 = icmp eq i32 %call4.i71, -1
  %spec.select.i73 = select i1 %cmp5.i72, i32 -1, i32 %carried_error.010.i66
  %9 = load ptr, ptr @__stdinp, align 8, !tbaa !9
  %call.i74 = call ptr @fgets(ptr noundef nonnull %pathname.i62, i32 noundef 1024, ptr noundef %9)
  %tobool.not.i75 = icmp eq ptr %call.i74, null
  br i1 %tobool.not.i75, label %ref_print_acl_from_stdin.exit77, label %while.body.i65, !llvm.loop !17

ref_print_acl_from_stdin.exit77:                  ; preds = %if.end.i70
  call void @llvm.lifetime.end.p0(i64 1024, ptr nonnull %pathname.i62) #6
  %cmp14 = icmp eq i32 %spec.select.i73, -1
  %spec.select = select i1 %cmp14, i32 -1, i32 %carried_error.089
  br label %for.inc

if.else:                                          ; preds = %for.body
  %call19 = call fastcc i32 @ref_print_acl(ptr noundef %7, i32 noundef %type.0.ph, i32 noundef %hflag.0.ph114, i32 noundef %iflag.0.ph121, i32 noundef %nflag.0.ph132, i32 noundef %qflag.0.ph127, i32 noundef %vflag.0.lcssa138, i32 noundef %sflag.0.ph136), !range !16
  %cmp20 = icmp eq i32 %call19, -1
  %spec.select61 = select i1 %cmp20, i32 -1, i32 %carried_error.089
  br label %for.inc

for.inc:                                          ; preds = %ref_print_acl_from_stdin.exit77, %ref_print_acl_from_stdin.exit77.thread, %if.else
  %carried_error.1 = phi i32 [ %spec.select61, %if.else ], [ %carried_error.089, %ref_print_acl_from_stdin.exit77.thread ], [ %spec.select, %ref_print_acl_from_stdin.exit77 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body, !llvm.loop !18

for.end.loopexit:                                 ; preds = %for.inc
  %10 = icmp ne i32 %carried_error.1, 0
  %11 = zext i1 %10 to i32
  br label %cleanup

cleanup:                                          ; preds = %for.cond.preheader, %for.end.loopexit, %ref_print_acl_from_stdin.exit, %sw.default
  %retval.0 = phi i32 [ -1, %sw.default ], [ %carried_error.0.lcssa.i, %ref_print_acl_from_stdin.exit ], [ 0, %for.cond.preheader ], [ %11, %for.end.loopexit ]
  ret i32 %retval.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

declare i32 @getopt_long(i32 noundef, ptr noundef, ptr noundef, ptr noundef, ptr noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nofree nounwind willreturn memory(argmem: read)
declare i32 @strcmp(ptr nocapture noundef, ptr nocapture noundef) local_unnamed_addr #3

; Function Attrs: nounwind uwtable
define internal fastcc noundef i32 @ref_print_acl(ptr noundef %path, i32 noundef %type, i32 noundef %hflag, i32 noundef %iflag, i32 noundef %nflag, i32 noundef %qflag, i32 noundef %vflag, i32 noundef %sflag) unnamed_addr #0 {
entry:
  %sb = alloca %struct.stat, align 8
  %trivial = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 224, ptr nonnull %sb) #6
  %tobool.not = icmp eq i32 %hflag, 0
  br i1 %tobool.not, label %if.end, label %if.end.thread

if.end:                                           ; preds = %entry
  %call1 = call i32 @stat(ptr noundef %path, ptr noundef nonnull %sb)
  %cmp = icmp eq i32 %call1, -1
  br i1 %cmp, label %if.then2, label %if.else7

if.end.thread:                                    ; preds = %entry
  %call = call i32 @lstat(ptr noundef %path, ptr noundef nonnull %sb)
  %cmp143 = icmp eq i32 %call, -1
  br i1 %cmp143, label %if.then2, label %if.then5

if.then2:                                         ; preds = %if.end.thread, %if.end
  tail call void (ptr, ...) @warn(ptr noundef nonnull @.str.7, ptr noundef %path) #6
  br label %cleanup97

if.then5:                                         ; preds = %if.end.thread
  %call6 = tail call i64 @lpathconf(ptr noundef %path, i32 noundef 64) #6
  br label %if.end10

if.else7:                                         ; preds = %if.end
  %call8 = tail call i64 @pathconf(ptr noundef %path, i32 noundef 64) #6
  br label %if.end10

if.end10:                                         ; preds = %if.else7, %if.then5
  %ret.0.in = phi i64 [ %call6, %if.then5 ], [ %call8, %if.else7 ]
  %ret.0 = trunc i64 %ret.0.in to i32
  %cmp11 = icmp sgt i32 %ret.0, 0
  br i1 %cmp11, label %if.then13, label %if.else18

if.then13:                                        ; preds = %if.end10
  %cmp14 = icmp eq i32 %type, 3
  br i1 %cmp14, label %if.then16, label %if.end26

if.then16:                                        ; preds = %if.then13
  tail call void (ptr, ...) @warnx(ptr noundef nonnull @.str.8, ptr noundef %path) #6
  br label %cleanup97

if.else18:                                        ; preds = %if.end10
  %cmp19 = icmp slt i32 %ret.0, 0
  br i1 %cmp19, label %land.lhs.true, label %if.end26

land.lhs.true:                                    ; preds = %if.else18
  %call21 = tail call ptr @__error() #6
  %0 = load i32, ptr %call21, align 4, !tbaa !13
  %cmp22.not = icmp eq i32 %0, 22
  br i1 %cmp22.not, label %if.end26, label %if.then24

if.then24:                                        ; preds = %land.lhs.true
  tail call void (ptr, ...) @warn(ptr noundef nonnull @.str.9, ptr noundef %path) #6
  br label %cleanup97

if.end26:                                         ; preds = %if.then13, %if.else18, %land.lhs.true
  %type.addr.0 = phi i32 [ %type, %land.lhs.true ], [ %type, %if.else18 ], [ 4, %if.then13 ]
  br i1 %tobool.not, label %if.else30, label %if.then28

if.then28:                                        ; preds = %if.end26
  %call29 = tail call ptr @acl_get_link_np(ptr noundef %path, i32 noundef %type.addr.0) #6
  br label %if.end32

if.else30:                                        ; preds = %if.end26
  %call31 = tail call ptr @acl_get_file(ptr noundef %path, i32 noundef %type.addr.0) #6
  br label %if.end32

if.end32:                                         ; preds = %if.else30, %if.then28
  %acl.0 = phi ptr [ %call29, %if.then28 ], [ %call31, %if.else30 ]
  %tobool33.not = icmp eq ptr %acl.0, null
  br i1 %tobool33.not, label %land.lhs.true34, label %if.end39.thread

land.lhs.true34:                                  ; preds = %if.end32
  %call35 = tail call ptr @__error() #6
  %1 = load i32, ptr %call35, align 4, !tbaa !13
  %cmp36.not = icmp eq i32 %1, 45
  br i1 %cmp36.not, label %if.end39, label %if.then38

if.then38:                                        ; preds = %land.lhs.true34
  tail call void (ptr, ...) @warn(ptr noundef nonnull @.str.10, ptr noundef %path) #6
  br label %cleanup97

if.end39:                                         ; preds = %land.lhs.true34
  %tobool40.not = icmp eq i32 %sflag, 0
  br i1 %tobool40.not, label %if.end55, label %if.then41

if.end39.thread:                                  ; preds = %if.end32
  %tobool40.not144 = icmp eq i32 %sflag, 0
  br i1 %tobool40.not144, label %if.end55, label %if.end44

if.then41:                                        ; preds = %if.end39
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %trivial) #6
  br label %cleanup97.critedge

if.end44:                                         ; preds = %if.end39.thread
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %trivial) #6
  %call45 = call i32 @acl_is_trivial_np(ptr noundef nonnull %acl.0, ptr noundef nonnull %trivial) #6
  %cmp46.not = icmp eq i32 %call45, 0
  br i1 %cmp46.not, label %if.else49, label %if.then48

if.then48:                                        ; preds = %if.end44
  call void (ptr, ...) @warn(ptr noundef nonnull @.str.11, ptr noundef %path) #6
  br label %cleanup

if.else49:                                        ; preds = %if.end44
  %2 = load i32, ptr %trivial, align 4, !tbaa !13
  %tobool50.not = icmp eq i32 %2, 0
  br i1 %tobool50.not, label %cleanup, label %if.then51

if.then51:                                        ; preds = %if.else49
  %call52 = call i32 @acl_free(ptr noundef nonnull %acl.0) #6
  br label %cleanup97.critedge

cleanup:                                          ; preds = %if.then48, %if.else49
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %trivial) #6
  br label %if.end55

if.end55:                                         ; preds = %if.end39.thread, %cleanup, %if.end39
  %.b = load i1, ptr @more_than_one, align 4
  br i1 %.b, label %if.then57, label %if.else59

if.then57:                                        ; preds = %if.end55
  %putchar = call i32 @putchar(i32 10)
  br label %if.end60

if.else59:                                        ; preds = %if.end55
  store i1 true, ptr @more_than_one, align 4
  br label %if.end60

if.end60:                                         ; preds = %if.else59, %if.then57
  %tobool61.not = icmp eq i32 %qflag, 0
  br i1 %tobool61.not, label %if.then62, label %if.end66

if.then62:                                        ; preds = %if.end60
  %st_uid = getelementptr inbounds %struct.stat, ptr %sb, i64 0, i32 5
  %3 = load i32, ptr %st_uid, align 4, !tbaa !19
  %call.i = call ptr @getpwuid(i32 noundef %3) #6
  %cmp.i = icmp eq ptr %call.i, null
  br i1 %cmp.i, label %if.then.i, label %if.else.i

if.then.i:                                        ; preds = %if.then62
  %call1.i = call i32 (ptr, i64, ptr, ...) @snprintf(ptr noundef nonnull dereferenceable(1) @ref_getuname.uids, i64 noundef 10, ptr noundef nonnull @.str.16, i32 noundef %3)
  br label %ref_getuname.exit

if.else.i:                                        ; preds = %if.then62
  %4 = load ptr, ptr %call.i, align 8, !tbaa !24
  br label %ref_getuname.exit

ref_getuname.exit:                                ; preds = %if.then.i, %if.else.i
  %retval.0.i = phi ptr [ @ref_getuname.uids, %if.then.i ], [ %4, %if.else.i ]
  %st_gid = getelementptr inbounds %struct.stat, ptr %sb, i64 0, i32 6
  %5 = load i32, ptr %st_gid, align 8, !tbaa !26
  %call.i136 = call ptr @getgrgid(i32 noundef %5) #6
  %cmp.i137 = icmp eq ptr %call.i136, null
  br i1 %cmp.i137, label %if.then.i140, label %if.else.i138

if.then.i140:                                     ; preds = %ref_getuname.exit
  %call1.i141 = call i32 (ptr, i64, ptr, ...) @snprintf(ptr noundef nonnull dereferenceable(1) @ref_getgname.gids, i64 noundef 10, ptr noundef nonnull @.str.16, i32 noundef %5)
  br label %ref_getgname.exit

if.else.i138:                                     ; preds = %ref_getuname.exit
  %6 = load ptr, ptr %call.i136, align 8, !tbaa !27
  br label %ref_getgname.exit

ref_getgname.exit:                                ; preds = %if.then.i140, %if.else.i138
  %retval.0.i139 = phi ptr [ @ref_getgname.gids, %if.then.i140 ], [ %6, %if.else.i138 ]
  %call65 = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.13, ptr noundef %path, ptr noundef %retval.0.i, ptr noundef %retval.0.i139)
  br label %if.end66

if.end66:                                         ; preds = %ref_getgname.exit, %if.end60
  br i1 %tobool33.not, label %if.then68, label %if.end77

if.then68:                                        ; preds = %if.end66
  %cmp69 = icmp eq i32 %type.addr.0, 3
  br i1 %cmp69, label %cleanup97, label %if.end72

if.end72:                                         ; preds = %if.then68
  %st_mode = getelementptr inbounds %struct.stat, ptr %sb, i64 0, i32 3
  %7 = load i16, ptr %st_mode, align 8, !tbaa !29
  %call73 = call ptr @acl_from_mode_np(i16 noundef zeroext %7) #6
  %tobool74.not = icmp eq ptr %call73, null
  br i1 %tobool74.not, label %if.then75, label %if.end77

if.then75:                                        ; preds = %if.end72
  call void (ptr, ...) @warn(ptr noundef nonnull @.str.14, ptr noundef %path) #6
  br label %cleanup97

if.end77:                                         ; preds = %if.end72, %if.end66
  %acl.1 = phi ptr [ %acl.0, %if.end66 ], [ %call73, %if.end72 ]
  %tobool78.not = icmp eq i32 %iflag, 0
  %spec.select = select i1 %tobool78.not, i32 0, i32 4
  %tobool81.not = icmp eq i32 %nflag, 0
  %or83 = or disjoint i32 %spec.select, 2
  %flags.1 = select i1 %tobool81.not, i32 %spec.select, i32 %or83
  %tobool85.not = icmp ne i32 %vflag, 0
  %or87 = zext i1 %tobool85.not to i32
  %flags.2 = or disjoint i32 %flags.1, %or87
  %call89 = call ptr @acl_to_text_np(ptr noundef nonnull %acl.1, ptr noundef null, i32 noundef %flags.2) #6
  %tobool90.not = icmp eq ptr %call89, null
  br i1 %tobool90.not, label %if.then91, label %if.end93

if.then91:                                        ; preds = %if.end77
  call void (ptr, ...) @warn(ptr noundef nonnull @.str.15, ptr noundef %path) #6
  %call92 = call i32 @acl_free(ptr noundef nonnull %acl.1) #6
  br label %cleanup97

if.end93:                                         ; preds = %if.end77
  %call94 = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.10, ptr noundef nonnull %call89)
  %call95 = call i32 @acl_free(ptr noundef nonnull %acl.1) #6
  %call96 = call i32 @acl_free(ptr noundef nonnull %call89) #6
  br label %cleanup97

cleanup97.critedge:                               ; preds = %if.then41, %if.then51
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %trivial) #6
  br label %cleanup97

cleanup97:                                        ; preds = %if.then68, %cleanup97.critedge, %if.end93, %if.then91, %if.then75, %if.then38, %if.then24, %if.then16, %if.then2
  %retval.1 = phi i32 [ -1, %if.then2 ], [ -1, %if.then16 ], [ 0, %if.end93 ], [ -1, %if.then91 ], [ -1, %if.then75 ], [ -1, %if.then38 ], [ -1, %if.then24 ], [ 0, %cleanup97.critedge ], [ 0, %if.then68 ]
  call void @llvm.lifetime.end.p0(i64 224, ptr nonnull %sb) #6
  ret i32 %retval.1
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nofree nounwind
declare noundef ptr @fgets(ptr noundef, i32 noundef, ptr nocapture noundef) local_unnamed_addr #4

; Function Attrs: mustprogress nofree nounwind willreturn memory(argmem: read)
declare ptr @strchr(ptr noundef, i32 noundef) local_unnamed_addr #3

; Function Attrs: nofree nounwind
declare noundef i32 @lstat(ptr nocapture noundef readonly, ptr nocapture noundef) local_unnamed_addr #4

; Function Attrs: nofree nounwind
declare noundef i32 @stat(ptr nocapture noundef readonly, ptr nocapture noundef) local_unnamed_addr #4

declare void @warn(ptr noundef, ...) local_unnamed_addr #2

declare i64 @lpathconf(ptr noundef, i32 noundef) local_unnamed_addr #2

declare i64 @pathconf(ptr noundef, i32 noundef) local_unnamed_addr #2

declare void @warnx(ptr noundef, ...) local_unnamed_addr #2

declare ptr @__error() local_unnamed_addr #2

declare ptr @acl_get_link_np(ptr noundef, i32 noundef) local_unnamed_addr #2

declare ptr @acl_get_file(ptr noundef, i32 noundef) local_unnamed_addr #2

declare i32 @acl_is_trivial_np(ptr noundef, ptr noundef) local_unnamed_addr #2

declare i32 @acl_free(ptr noundef) local_unnamed_addr #2

; Function Attrs: nofree nounwind
declare noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #4

declare ptr @acl_from_mode_np(i16 noundef zeroext) local_unnamed_addr #2

declare ptr @acl_to_text_np(ptr noundef, ptr noundef, i32 noundef) local_unnamed_addr #2

declare ptr @getpwuid(i32 noundef) local_unnamed_addr #2

; Function Attrs: nofree nounwind
declare noundef i32 @snprintf(ptr noalias nocapture noundef writeonly, i64 noundef, ptr nocapture noundef readonly, ...) local_unnamed_addr #4

declare ptr @getgrgid(i32 noundef) local_unnamed_addr #2

; Function Attrs: nofree nounwind
declare noundef i64 @fwrite(ptr nocapture noundef, i64 noundef, i64 noundef, ptr nocapture noundef) local_unnamed_addr #5

; Function Attrs: nofree nounwind
declare noundef i32 @putchar(i32 noundef) local_unnamed_addr #5

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { mustprogress nofree nounwind willreturn memory(argmem: read) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nofree nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { nofree nounwind }
attributes #6 = { nounwind }
attributes #7 = { nounwind willreturn memory(read) }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.mustprogress"}
!7 = distinct !{!7, !6, !8}
!8 = !{!"llvm.loop.peeled.count", i32 1}
!9 = !{!10, !10, i64 0}
!10 = !{!"any pointer", !11, i64 0}
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C/C++ TBAA"}
!13 = !{!14, !14, i64 0}
!14 = !{!"int", !11, i64 0}
!15 = !{!11, !11, i64 0}
!16 = !{i32 -1, i32 1}
!17 = distinct !{!17, !6}
!18 = distinct !{!18, !6}
!19 = !{!20, !14, i64 28}
!20 = !{!"stat", !21, i64 0, !21, i64 8, !21, i64 16, !22, i64 24, !22, i64 26, !14, i64 28, !14, i64 32, !14, i64 36, !21, i64 40, !23, i64 48, !23, i64 64, !23, i64 80, !23, i64 96, !21, i64 112, !21, i64 120, !14, i64 128, !14, i64 132, !21, i64 136, !21, i64 144, !11, i64 152}
!21 = !{!"long", !11, i64 0}
!22 = !{!"short", !11, i64 0}
!23 = !{!"timespec", !21, i64 0, !21, i64 8}
!24 = !{!25, !10, i64 0}
!25 = !{!"passwd", !10, i64 0, !10, i64 8, !14, i64 16, !14, i64 20, !21, i64 24, !10, i64 32, !10, i64 40, !10, i64 48, !10, i64 56, !21, i64 64, !14, i64 72}
!26 = !{!20, !14, i64 32}
!27 = !{!28, !10, i64 0}
!28 = !{!"group", !10, i64 0, !10, i64 8, !14, i64 16, !10, i64 24}
!29 = !{!20, !22, i64 24}
