; ModuleID = '/home/odin/pbsd/pbsd/usr.bin/proccontrol/proccontrol_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/usr.bin/proccontrol/proccontrol_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

module asm ".symver __qsort_r_compat, qsort_r@FBSD_1.0"

%struct.anon = type { i32, ptr }

@.str = private unnamed_addr constant [8 x i8] c"m:qs:p:\00", align 1
@optarg = external local_unnamed_addr global ptr, align 8
@modes = internal unnamed_addr constant [11 x %struct.anon] [%struct.anon { i32 1, ptr @.str.37 }, %struct.anon { i32 2, ptr @.str.38 }, %struct.anon { i32 3, ptr @.str.39 }, %struct.anon { i32 4, ptr @.str.40 }, %struct.anon { i32 5, ptr @.str.41 }, %struct.anon { i32 6, ptr @.str.42 }, %struct.anon { i32 7, ptr @.str.43 }, %struct.anon { i32 8, ptr @.str.44 }, %struct.anon { i32 9, ptr @.str.45 }, %struct.anon { i32 10, ptr @.str.46 }, %struct.anon { i32 11, ptr @.str.47 }], align 16
@.str.1 = private unnamed_addr constant [7 x i8] c"enable\00", align 1
@.str.2 = private unnamed_addr constant [8 x i8] c"disable\00", align 1
@optind = external local_unnamed_addr global i32, align 4
@.str.3 = private unnamed_addr constant [15 x i8] c"procctl status\00", align 1
@.str.11 = private unnamed_addr constant [23 x i8] c"enabled, traced by %d\0A\00", align 1
@.str.17 = private unnamed_addr constant [19 x i8] c", disabled on exec\00", align 1
@.str.18 = private unnamed_addr constant [17 x i8] c", wxorx enforced\00", align 1
@.str.20 = private unnamed_addr constant [13 x i8] c"la48 on exec\00", align 1
@.str.21 = private unnamed_addr constant [13 x i8] c"la57 on exec\00", align 1
@.str.22 = private unnamed_addr constant [16 x i8] c"default on exec\00", align 1
@.str.28 = private unnamed_addr constant [12 x i8] c"procctl ctl\00", align 1
@.str.29 = private unnamed_addr constant [5 x i8] c"exec\00", align 1
@__stderrp = external local_unnamed_addr global ptr, align 8
@.str.30 = private unnamed_addr constant [8 x i8] c"Usage:\0A\00", align 1
@.str.31 = private unnamed_addr constant [64 x i8] c"    proccontrol -m mode -s (enable|disable) (-p pid | command)\0A\00", align 1
@.str.32 = private unnamed_addr constant [37 x i8] c"    proccontrol -m mode -q [-p pid]\0A\00", align 1
@.str.33 = private unnamed_addr constant [8 x i8] c"Modes: \00", align 1
@.str.34 = private unnamed_addr constant [5 x i8] c"%s%s\00", align 1
@.str.35 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@.str.36 = private unnamed_addr constant [2 x i8] c"|\00", align 1
@.str.37 = private unnamed_addr constant [5 x i8] c"aslr\00", align 1
@.str.38 = private unnamed_addr constant [6 x i8] c"trace\00", align 1
@.str.39 = private unnamed_addr constant [8 x i8] c"trapcap\00", align 1
@.str.40 = private unnamed_addr constant [8 x i8] c"protmax\00", align 1
@.str.41 = private unnamed_addr constant [9 x i8] c"stackgap\00", align 1
@.str.42 = private unnamed_addr constant [11 x i8] c"nonewprivs\00", align 1
@.str.43 = private unnamed_addr constant [6 x i8] c"wxmap\00", align 1
@.str.44 = private unnamed_addr constant [5 x i8] c"kpti\00", align 1
@.str.45 = private unnamed_addr constant [5 x i8] c"la57\00", align 1
@.str.46 = private unnamed_addr constant [5 x i8] c"la48\00", align 1
@.str.47 = private unnamed_addr constant [11 x i8] c"logsigexit\00", align 1
@.str.48 = private unnamed_addr constant [16 x i8] c"non-numeric pid\00", align 1
@str = private unnamed_addr constant [15 x i8] c"force disabled\00", align 1
@str.49 = private unnamed_addr constant [14 x i8] c"force enabled\00", align 1
@str.50 = private unnamed_addr constant [11 x i8] c"not forced\00", align 1
@str.51 = private unnamed_addr constant [14 x i8] c", la57 active\00", align 1
@str.52 = private unnamed_addr constant [14 x i8] c", la48 active\00", align 1
@str.59 = private unnamed_addr constant [20 x i8] c"disabled after exec\00", align 1
@str.60 = private unnamed_addr constant [19 x i8] c"enabled after exec\00", align 1
@str.64 = private unnamed_addr constant [8 x i8] c"enabled\00", align 1
@str.65 = private unnamed_addr constant [21 x i8] c"enabled, no debugger\00", align 1
@str.66 = private unnamed_addr constant [9 x i8] c"disabled\00", align 1
@str.67 = private unnamed_addr constant [13 x i8] c", not active\00", align 1
@str.68 = private unnamed_addr constant [9 x i8] c", active\00", align 1
@switch.table.ref_main = private unnamed_addr constant [11 x i32] [i32 14, i32 8, i32 10, i32 16, i32 18, i32 20, i32 22, i32 268435457, i32 268435459, i32 268435459, i32 24], align 4
@switch.table.ref_main.70 = private unnamed_addr constant [3 x ptr] [ptr @str.49, ptr @str, ptr @str.50], align 8
@reltable.ref_main = private unnamed_addr constant [3 x i32] [i32 trunc (i64 sub (i64 ptrtoint (ptr @.str.20 to i64), i64 ptrtoint (ptr @reltable.ref_main to i64)) to i32), i32 trunc (i64 sub (i64 ptrtoint (ptr @.str.21 to i64), i64 ptrtoint (ptr @reltable.ref_main to i64)) to i32), i32 trunc (i64 sub (i64 ptrtoint (ptr @.str.22 to i64), i64 ptrtoint (ptr @reltable.ref_main to i64)) to i32)], align 4

; Function Attrs: noreturn nounwind uwtable
define dso_local noundef i32 @ref_main(i32 noundef %argc, ptr noundef %argv) local_unnamed_addr #0 {
entry:
  %tail.i = alloca ptr, align 8
  %arg = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %arg) #10
  br label %while.cond.outer

while.cond.outer:                                 ; preds = %cleanup, %entry
  %mode.0.ph = phi i32 [ %1, %cleanup ], [ 0, %entry ]
  %pid.0.ph = phi i32 [ %pid.0.ph471, %cleanup ], [ -1, %entry ]
  %enable.0.ph = phi i8 [ %enable.0.ph475, %cleanup ], [ 1, %entry ]
  %query.0.ph = phi i8 [ %query.0.lcssa478, %cleanup ], [ 0, %entry ]
  br label %while.cond.outer470

while.cond.outer470:                              ; preds = %while.cond.outer, %ref_str2pid.exit
  %pid.0.ph471 = phi i32 [ %pid.0.ph, %while.cond.outer ], [ %retval.0.i, %ref_str2pid.exit ]
  %enable.0.ph472 = phi i8 [ %enable.0.ph, %while.cond.outer ], [ %enable.0.ph475, %ref_str2pid.exit ]
  %query.0.ph473 = phi i8 [ %query.0.ph, %while.cond.outer ], [ %query.0.lcssa480, %ref_str2pid.exit ]
  br label %while.cond.outer474

while.cond.outer474:                              ; preds = %while.cond.outer474.backedge, %while.cond.outer470
  %enable.0.ph475 = phi i8 [ %enable.0.ph472, %while.cond.outer470 ], [ %enable.0.ph475.be, %while.cond.outer474.backedge ]
  %query.0.ph476 = phi i8 [ %query.0.ph473, %while.cond.outer470 ], [ %query.0.lcssa479, %while.cond.outer474.backedge ]
  %call.peel = tail call i32 @getopt(i32 noundef %argc, ptr noundef %argv, ptr noundef nonnull @.str) #10
  switch i32 %call.peel, label %sw.default [
    i32 -1, label %while.end
    i32 109, label %sw.bb
    i32 115, label %sw.bb12
    i32 112, label %sw.bb22
    i32 113, label %while.cond
  ]

while.cond:                                       ; preds = %while.cond.outer474, %while.cond
  %call = tail call i32 @getopt(i32 noundef %argc, ptr noundef %argv, ptr noundef nonnull @.str) #10
  switch i32 %call, label %sw.default [
    i32 -1, label %while.end
    i32 109, label %sw.bb
    i32 115, label %sw.bb12
    i32 112, label %sw.bb22
    i32 113, label %while.cond
  ], !llvm.loop !5

sw.bb:                                            ; preds = %while.cond.outer474, %while.cond
  %query.0.lcssa478 = phi i8 [ 1, %while.cond ], [ %query.0.ph476, %while.cond.outer474 ]
  %cmp1.not = icmp eq i32 %mode.0.ph, 0
  br i1 %cmp1.not, label %for.cond.preheader, label %if.then

for.cond.preheader:                               ; preds = %sw.bb
  %0 = load ptr, ptr @optarg, align 8, !tbaa !8
  %call3 = tail call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %0, ptr noundef nonnull dereferenceable(5) @.str.37) #11
  %cmp4 = icmp eq i32 %call3, 0
  br i1 %cmp4, label %cleanup, label %for.inc

if.then:                                          ; preds = %sw.bb
  tail call fastcc void @ref_usage() #12
  unreachable

for.inc:                                          ; preds = %for.cond.preheader
  %call3.1 = tail call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %0, ptr noundef nonnull dereferenceable(6) @.str.38) #11
  %cmp4.1 = icmp eq i32 %call3.1, 0
  br i1 %cmp4.1, label %cleanup, label %for.inc.1

for.inc.1:                                        ; preds = %for.inc
  %call3.2 = tail call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %0, ptr noundef nonnull dereferenceable(8) @.str.39) #11
  %cmp4.2 = icmp eq i32 %call3.2, 0
  br i1 %cmp4.2, label %cleanup, label %for.inc.2

for.inc.2:                                        ; preds = %for.inc.1
  %call3.3 = tail call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %0, ptr noundef nonnull dereferenceable(8) @.str.40) #11
  %cmp4.3 = icmp eq i32 %call3.3, 0
  br i1 %cmp4.3, label %cleanup, label %for.inc.3

for.inc.3:                                        ; preds = %for.inc.2
  %call3.4 = tail call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %0, ptr noundef nonnull dereferenceable(9) @.str.41) #11
  %cmp4.4 = icmp eq i32 %call3.4, 0
  br i1 %cmp4.4, label %cleanup, label %for.inc.4

for.inc.4:                                        ; preds = %for.inc.3
  %call3.5 = tail call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %0, ptr noundef nonnull dereferenceable(11) @.str.42) #11
  %cmp4.5 = icmp eq i32 %call3.5, 0
  br i1 %cmp4.5, label %cleanup, label %for.inc.5

for.inc.5:                                        ; preds = %for.inc.4
  %call3.6 = tail call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %0, ptr noundef nonnull dereferenceable(6) @.str.43) #11
  %cmp4.6 = icmp eq i32 %call3.6, 0
  br i1 %cmp4.6, label %cleanup, label %for.inc.6

for.inc.6:                                        ; preds = %for.inc.5
  %call3.7 = tail call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %0, ptr noundef nonnull dereferenceable(5) @.str.44) #11
  %cmp4.7 = icmp eq i32 %call3.7, 0
  br i1 %cmp4.7, label %cleanup, label %for.inc.7

for.inc.7:                                        ; preds = %for.inc.6
  %call3.8 = tail call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %0, ptr noundef nonnull dereferenceable(5) @.str.45) #11
  %cmp4.8 = icmp eq i32 %call3.8, 0
  br i1 %cmp4.8, label %cleanup, label %for.inc.8

for.inc.8:                                        ; preds = %for.inc.7
  %call3.9 = tail call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %0, ptr noundef nonnull dereferenceable(5) @.str.46) #11
  %cmp4.9 = icmp eq i32 %call3.9, 0
  br i1 %cmp4.9, label %cleanup, label %for.inc.9

for.inc.9:                                        ; preds = %for.inc.8
  %call3.10 = tail call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %0, ptr noundef nonnull dereferenceable(11) @.str.47) #11
  %cmp4.10 = icmp eq i32 %call3.10, 0
  br i1 %cmp4.10, label %cleanup, label %if.then10

cleanup:                                          ; preds = %for.inc.9, %for.inc.8, %for.inc.7, %for.inc.6, %for.inc.5, %for.inc.4, %for.inc.3, %for.inc.2, %for.inc.1, %for.inc, %for.cond.preheader
  %i.0392.lcssa = phi i64 [ 0, %for.cond.preheader ], [ 1, %for.inc ], [ 2, %for.inc.1 ], [ 3, %for.inc.2 ], [ 4, %for.inc.3 ], [ 5, %for.inc.4 ], [ 6, %for.inc.5 ], [ 7, %for.inc.6 ], [ 8, %for.inc.7 ], [ 9, %for.inc.8 ], [ 10, %for.inc.9 ]
  %arrayidx = getelementptr inbounds [11 x %struct.anon], ptr @modes, i64 0, i64 %i.0392.lcssa
  %1 = load i32, ptr %arrayidx, align 16, !tbaa !12
  %cmp9 = icmp eq i32 %1, 0
  br i1 %cmp9, label %if.then10, label %while.cond.outer, !llvm.loop !15

if.then10:                                        ; preds = %cleanup, %for.inc.9
  tail call fastcc void @ref_usage() #12
  unreachable

sw.bb12:                                          ; preds = %while.cond, %while.cond.outer474
  %query.0.lcssa479 = phi i8 [ %query.0.ph476, %while.cond.outer474 ], [ 1, %while.cond ]
  %2 = load ptr, ptr @optarg, align 8, !tbaa !8
  %call13 = tail call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %2, ptr noundef nonnull dereferenceable(7) @.str.1) #11
  %cmp14 = icmp eq i32 %call13, 0
  br i1 %cmp14, label %while.cond.outer474.backedge, label %if.else

while.cond.outer474.backedge:                     ; preds = %sw.bb12, %if.else
  %enable.0.ph475.be = phi i8 [ 0, %if.else ], [ 1, %sw.bb12 ]
  br label %while.cond.outer474, !llvm.loop !15

if.else:                                          ; preds = %sw.bb12
  %call16 = tail call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %2, ptr noundef nonnull dereferenceable(8) @.str.2) #11
  %cmp17 = icmp eq i32 %call16, 0
  br i1 %cmp17, label %while.cond.outer474.backedge, label %if.else19

if.else19:                                        ; preds = %if.else
  tail call fastcc void @ref_usage() #12
  unreachable

sw.bb22:                                          ; preds = %while.cond.outer474, %while.cond
  %query.0.lcssa480 = phi i8 [ 1, %while.cond ], [ %query.0.ph476, %while.cond.outer474 ]
  %3 = load ptr, ptr @optarg, align 8, !tbaa !8
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %tail.i) #10
  %call.i = call i64 @strtol(ptr noundef %3, ptr noundef nonnull %tail.i, i32 noundef 0)
  %4 = load ptr, ptr %tail.i, align 8, !tbaa !8
  %5 = load i8, ptr %4, align 1, !tbaa !16
  %cmp.not.i = icmp eq i8 %5, 0
  br i1 %cmp.not.i, label %if.end.i, label %if.then.i

if.then.i:                                        ; preds = %sw.bb22
  tail call void (ptr, ...) @warnx(ptr noundef nonnull @.str.48) #10
  br label %ref_str2pid.exit

if.end.i:                                         ; preds = %sw.bb22
  %conv.i = trunc i64 %call.i to i32
  br label %ref_str2pid.exit

ref_str2pid.exit:                                 ; preds = %if.then.i, %if.end.i
  %retval.0.i = phi i32 [ -1, %if.then.i ], [ %conv.i, %if.end.i ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %tail.i) #10
  br label %while.cond.outer470, !llvm.loop !15

sw.default:                                       ; preds = %while.cond.outer474, %while.cond
  tail call fastcc void @ref_usage() #12
  unreachable

while.end:                                        ; preds = %while.cond.outer474, %while.cond
  %query.0.lcssa477 = phi i8 [ 1, %while.cond ], [ %query.0.ph476, %while.cond.outer474 ]
  %6 = load i32, ptr @optind, align 4, !tbaa !17
  %cmp26.not = icmp eq i32 %6, %argc
  %cmp34 = icmp eq i32 %pid.0.ph471, -1
  %tobool36.not = icmp eq i8 %query.0.lcssa477, 0
  br i1 %cmp26.not, label %if.else33, label %if.then27

if.then27:                                        ; preds = %while.end
  %or.cond = select i1 %cmp34, i1 %tobool36.not, i1 false
  br i1 %or.cond, label %if.end41.thread, label %if.then30

if.then30:                                        ; preds = %if.then27
  tail call fastcc void @ref_usage() #12
  unreachable

if.end41.thread:                                  ; preds = %if.then27
  %call32 = tail call i32 @getpid() #10
  br label %if.else219

if.else33:                                        ; preds = %while.end
  br i1 %cmp34, label %if.then35, label %if.end41

if.then35:                                        ; preds = %if.else33
  br i1 %tobool36.not, label %if.then37, label %if.end41.thread419

if.then37:                                        ; preds = %if.then35
  tail call fastcc void @ref_usage() #12
  unreachable

if.end41.thread419:                               ; preds = %if.then35
  %call39 = tail call i32 @getpid() #10
  br label %if.then43

if.end41:                                         ; preds = %if.else33
  br i1 %tobool36.not, label %if.else219, label %if.then43

if.then43:                                        ; preds = %if.end41.thread419, %if.end41
  %pid.2423 = phi i32 [ %call39, %if.end41.thread419 ], [ %pid.0.ph471, %if.end41 ]
  %switch.tableidx = add i32 %mode.0.ph, -1
  %7 = icmp ult i32 %switch.tableidx, 11
  br i1 %7, label %switch.lookup, label %sw.default73

sw.default73:                                     ; preds = %if.then43
  tail call fastcc void @ref_usage() #12
  unreachable

switch.lookup:                                    ; preds = %if.then43
  %8 = zext nneg i32 %switch.tableidx to i64
  %switch.gep = getelementptr inbounds [11 x i32], ptr @switch.table.ref_main, i64 0, i64 %8
  %switch.load = load i32, ptr %switch.gep, align 4
  %conv71 = sext i32 %pid.2423 to i64
  %call72 = call i32 @procctl(i32 noundef 0, i64 noundef %conv71, i32 noundef %switch.load, ptr noundef nonnull %arg) #10
  %cmp75.not = icmp eq i32 %call72, 0
  br i1 %cmp75.not, label %if.end78, label %if.then77

if.then77:                                        ; preds = %switch.lookup
  call void (i32, ptr, ...) @err(i32 noundef 1, ptr noundef nonnull @.str.3) #13
  unreachable

if.end78:                                         ; preds = %switch.lookup
  switch i32 %mode.0.ph, label %if.end296 [
    i32 1, label %sw.bb79
    i32 2, label %sw.bb95
    i32 3, label %sw.bb109
    i32 4, label %sw.bb115
    i32 5, label %sw.bb132
    i32 6, label %sw.bb145
    i32 7, label %sw.bb151
    i32 8, label %sw.bb173
    i32 9, label %sw.bb188
    i32 10, label %sw.bb188
    i32 11, label %sw.bb210
  ]

sw.bb79:                                          ; preds = %if.end78
  %9 = load i32, ptr %arg, align 4, !tbaa !17
  %and = and i32 %9, 2147483647
  %switch.tableidx459 = add nsw i32 %and, -1
  %10 = icmp ult i32 %switch.tableidx459, 3
  br i1 %10, label %switch.lookup458, label %sw.epilog86

switch.lookup458:                                 ; preds = %sw.bb79
  %11 = zext nneg i32 %switch.tableidx459 to i64
  %switch.gep460 = getelementptr inbounds [3 x ptr], ptr @switch.table.ref_main.70, i64 0, i64 %11
  %switch.load461 = load ptr, ptr %switch.gep460, align 8
  %call81 = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) %switch.load461)
  %.pr = load i32, ptr %arg, align 4, !tbaa !17
  br label %sw.epilog86

sw.epilog86:                                      ; preds = %sw.bb79, %switch.lookup458
  %12 = phi i32 [ %.pr, %switch.lookup458 ], [ %9, %sw.bb79 ]
  %cmp88.not = icmp sgt i32 %12, -1
  br i1 %cmp88.not, label %if.else92, label %if.then90

if.then90:                                        ; preds = %sw.epilog86
  %puts364 = call i32 @puts(ptr nonnull dereferenceable(1) @str.68)
  br label %if.end296

if.else92:                                        ; preds = %sw.epilog86
  %puts363 = call i32 @puts(ptr nonnull dereferenceable(1) @str.67)
  br label %if.end296

sw.bb95:                                          ; preds = %if.end78
  %13 = load i32, ptr %arg, align 4, !tbaa !17
  switch i32 %13, label %if.else105 [
    i32 -1, label %if.then98
    i32 0, label %if.then103
  ]

if.then98:                                        ; preds = %sw.bb95
  %puts362 = call i32 @puts(ptr nonnull dereferenceable(1) @str.66)
  br label %if.end296

if.then103:                                       ; preds = %sw.bb95
  %puts361 = call i32 @puts(ptr nonnull dereferenceable(1) @str.65)
  br label %if.end296

if.else105:                                       ; preds = %sw.bb95
  %call106 = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.11, i32 noundef %13)
  br label %if.end296

sw.bb109:                                         ; preds = %if.end78
  %14 = load i32, ptr %arg, align 4, !tbaa !17
  switch i32 %14, label %if.end296 [
    i32 1, label %sw.bb110
    i32 2, label %sw.bb112
  ]

sw.bb110:                                         ; preds = %sw.bb109
  %puts360 = call i32 @puts(ptr nonnull dereferenceable(1) @str.64)
  br label %if.end296

sw.bb112:                                         ; preds = %sw.bb109
  %puts359 = call i32 @puts(ptr nonnull dereferenceable(1) @str.66)
  br label %if.end296

sw.bb115:                                         ; preds = %if.end78
  %15 = load i32, ptr %arg, align 4, !tbaa !17
  %and116 = and i32 %15, 2147483647
  %switch.tableidx463 = add nsw i32 %and116, -1
  %16 = icmp ult i32 %switch.tableidx463, 3
  br i1 %16, label %switch.lookup462, label %sw.epilog123

switch.lookup462:                                 ; preds = %sw.bb115
  %17 = zext nneg i32 %switch.tableidx463 to i64
  %switch.gep464 = getelementptr inbounds [3 x ptr], ptr @switch.table.ref_main.70, i64 0, i64 %17
  %switch.load465 = load ptr, ptr %switch.gep464, align 8
  %call118 = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) %switch.load465)
  %.pr367 = load i32, ptr %arg, align 4, !tbaa !17
  br label %sw.epilog123

sw.epilog123:                                     ; preds = %sw.bb115, %switch.lookup462
  %18 = phi i32 [ %.pr367, %switch.lookup462 ], [ %15, %sw.bb115 ]
  %cmp125.not = icmp sgt i32 %18, -1
  br i1 %cmp125.not, label %if.else129, label %if.then127

if.then127:                                       ; preds = %sw.epilog123
  %puts358 = call i32 @puts(ptr nonnull dereferenceable(1) @str.68)
  br label %if.end296

if.else129:                                       ; preds = %sw.epilog123
  %puts357 = call i32 @puts(ptr nonnull dereferenceable(1) @str.67)
  br label %if.end296

sw.bb132:                                         ; preds = %if.end78
  %19 = load i32, ptr %arg, align 4, !tbaa !17
  %and133 = and i32 %19, 3
  switch i32 %and133, label %sw.epilog138 [
    i32 1, label %sw.epilog138.sink.split
    i32 2, label %sw.bb136
  ]

sw.bb136:                                         ; preds = %sw.bb132
  br label %sw.epilog138.sink.split

sw.epilog138.sink.split:                          ; preds = %sw.bb132, %sw.bb136
  %str.57.sink = phi ptr [ @str.66, %sw.bb136 ], [ @str.64, %sw.bb132 ]
  %puts353 = call i32 @puts(ptr nonnull dereferenceable(1) %str.57.sink)
  br label %sw.epilog138

sw.epilog138:                                     ; preds = %sw.epilog138.sink.split, %sw.bb132
  %20 = load i32, ptr %arg, align 4, !tbaa !17
  %and139 = and i32 %20, 12
  switch i32 %and139, label %if.end296 [
    i32 4, label %sw.bb140
    i32 8, label %sw.bb142
  ]

sw.bb140:                                         ; preds = %sw.epilog138
  %puts356 = call i32 @puts(ptr nonnull dereferenceable(1) @str.60)
  br label %if.end296

sw.bb142:                                         ; preds = %sw.epilog138
  %puts355 = call i32 @puts(ptr nonnull dereferenceable(1) @str.59)
  br label %if.end296

sw.bb145:                                         ; preds = %if.end78
  %21 = load i32, ptr %arg, align 4, !tbaa !17
  switch i32 %21, label %if.end296 [
    i32 1, label %sw.bb146
    i32 2, label %sw.bb148
  ]

sw.bb146:                                         ; preds = %sw.bb145
  %puts352 = call i32 @puts(ptr nonnull dereferenceable(1) @str.64)
  br label %if.end296

sw.bb148:                                         ; preds = %sw.bb145
  %puts351 = call i32 @puts(ptr nonnull dereferenceable(1) @str.66)
  br label %if.end296

sw.bb151:                                         ; preds = %if.end78
  %22 = load i32, ptr %arg, align 4, !tbaa !17
  %and152 = and i32 %22, 1
  %cmp153.not = icmp eq i32 %and152, 0
  %.str.16..str.15 = select i1 %cmp153.not, ptr @str.66, ptr @str.64
  %call158 = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) %.str.16..str.15)
  %23 = load i32, ptr %arg, align 4, !tbaa !17
  %and160 = and i32 %23, 2
  %cmp161.not = icmp eq i32 %and160, 0
  br i1 %cmp161.not, label %if.end165, label %if.then163

if.then163:                                       ; preds = %sw.bb151
  %call164 = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.17)
  %.pr368 = load i32, ptr %arg, align 4, !tbaa !17
  br label %if.end165

if.end165:                                        ; preds = %if.then163, %sw.bb151
  %24 = phi i32 [ %.pr368, %if.then163 ], [ %23, %sw.bb151 ]
  %cmp167.not = icmp sgt i32 %24, -1
  br i1 %cmp167.not, label %if.end171, label %if.then169

if.then169:                                       ; preds = %if.end165
  %call170 = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.18)
  br label %if.end171

if.end171:                                        ; preds = %if.then169, %if.end165
  %putchar = call i32 @putchar(i32 10)
  br label %if.end296

sw.bb173:                                         ; preds = %if.end78
  %25 = load i32, ptr %arg, align 4, !tbaa !17
  %and174 = and i32 %25, 2147483647
  switch i32 %and174, label %sw.epilog179 [
    i32 1, label %sw.epilog179thread-pre-split
    i32 2, label %sw.bb177
  ]

sw.bb177:                                         ; preds = %sw.bb173
  br label %sw.epilog179thread-pre-split

sw.epilog179thread-pre-split:                     ; preds = %sw.bb173, %sw.bb177
  %.str.15.sink = phi ptr [ @str.66, %sw.bb177 ], [ @str.64, %sw.bb173 ]
  %call176 = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) %.str.15.sink)
  %.pr369 = load i32, ptr %arg, align 4, !tbaa !17
  br label %sw.epilog179

sw.epilog179:                                     ; preds = %sw.epilog179thread-pre-split, %sw.bb173
  %26 = phi i32 [ %.pr369, %sw.epilog179thread-pre-split ], [ %25, %sw.bb173 ]
  %cmp181.not = icmp sgt i32 %26, -1
  br i1 %cmp181.not, label %if.else185, label %if.then183

if.then183:                                       ; preds = %sw.epilog179
  %puts350 = call i32 @puts(ptr nonnull dereferenceable(1) @str.68)
  br label %if.end296

if.else185:                                       ; preds = %sw.epilog179
  %puts349 = call i32 @puts(ptr nonnull dereferenceable(1) @str.67)
  br label %if.end296

sw.bb188:                                         ; preds = %if.end78, %if.end78
  %27 = load i32, ptr %arg, align 4, !tbaa !17
  %and189 = and i32 %27, -50331649
  %switch.tableidx467 = add i32 %and189, -1
  %28 = icmp ult i32 %switch.tableidx467, 3
  br i1 %28, label %switch.lookup466, label %sw.epilog196

switch.lookup466:                                 ; preds = %sw.bb188
  %29 = zext nneg i32 %switch.tableidx467 to i64
  %reltable.shift = shl i64 %29, 2
  %reltable.intrinsic = call ptr @llvm.load.relative.i64(ptr @reltable.ref_main, i64 %reltable.shift)
  %call195 = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) %reltable.intrinsic)
  br label %sw.epilog196

sw.epilog196:                                     ; preds = %sw.bb188, %switch.lookup466
  %30 = load i32, ptr %arg, align 4, !tbaa !17
  %and197 = and i32 %30, 16777216
  %cmp198.not = icmp eq i32 %and197, 0
  br i1 %cmp198.not, label %if.else202, label %if.then200

if.then200:                                       ; preds = %sw.epilog196
  %puts348 = call i32 @puts(ptr nonnull dereferenceable(1) @str.52)
  br label %if.end296

if.else202:                                       ; preds = %sw.epilog196
  %and203 = and i32 %30, 33554432
  %cmp204.not = icmp eq i32 %and203, 0
  br i1 %cmp204.not, label %if.end296, label %if.then206

if.then206:                                       ; preds = %if.else202
  %puts347 = call i32 @puts(ptr nonnull dereferenceable(1) @str.51)
  br label %if.end296

sw.bb210:                                         ; preds = %if.end78
  %31 = load i32, ptr %arg, align 4, !tbaa !17
  switch i32 %31, label %if.end296 [
    i32 1, label %sw.bb211
    i32 2, label %sw.bb213
    i32 3, label %sw.bb215
  ]

sw.bb211:                                         ; preds = %sw.bb210
  %puts346 = call i32 @puts(ptr nonnull dereferenceable(1) @str.50)
  br label %if.end296

sw.bb213:                                         ; preds = %sw.bb210
  %puts345 = call i32 @puts(ptr nonnull dereferenceable(1) @str.49)
  br label %if.end296

sw.bb215:                                         ; preds = %sw.bb210
  %puts = call i32 @puts(ptr nonnull dereferenceable(1) @str)
  br label %if.end296

if.else219:                                       ; preds = %if.end41.thread, %if.end41
  %pid.2418 = phi i32 [ %call32, %if.end41.thread ], [ %pid.0.ph471, %if.end41 ]
  switch i32 %mode.0.ph, label %sw.default285 [
    i32 1, label %sw.bb220
    i32 2, label %sw.bb225
    i32 3, label %sw.bb231
    i32 4, label %sw.bb237
    i32 5, label %sw.bb243
    i32 6, label %sw.bb249
    i32 7, label %sw.bb255
    i32 8, label %sw.bb261
    i32 9, label %sw.bb267
    i32 10, label %sw.bb273
    i32 11, label %sw.bb279
  ]

sw.bb220:                                         ; preds = %if.else219
  %tobool221.not = icmp eq i8 %enable.0.ph475, 0
  %cond = select i1 %tobool221.not, i32 2, i32 1
  store i32 %cond, ptr %arg, align 4, !tbaa !17
  br label %sw.epilog286

sw.bb225:                                         ; preds = %if.else219
  %tobool226.not = icmp eq i8 %enable.0.ph475, 0
  %cond228 = select i1 %tobool226.not, i32 2, i32 1
  store i32 %cond228, ptr %arg, align 4, !tbaa !17
  br label %sw.epilog286

sw.bb231:                                         ; preds = %if.else219
  %tobool232.not = icmp eq i8 %enable.0.ph475, 0
  %cond234 = select i1 %tobool232.not, i32 2, i32 1
  store i32 %cond234, ptr %arg, align 4, !tbaa !17
  br label %sw.epilog286

sw.bb237:                                         ; preds = %if.else219
  %tobool238.not = icmp eq i8 %enable.0.ph475, 0
  %cond240 = select i1 %tobool238.not, i32 2, i32 1
  store i32 %cond240, ptr %arg, align 4, !tbaa !17
  br label %sw.epilog286

sw.bb243:                                         ; preds = %if.else219
  %tobool244.not = icmp eq i8 %enable.0.ph475, 0
  %cond246 = select i1 %tobool244.not, i32 10, i32 4
  store i32 %cond246, ptr %arg, align 4, !tbaa !17
  br label %sw.epilog286

sw.bb249:                                         ; preds = %if.else219
  %tobool250.not = icmp eq i8 %enable.0.ph475, 0
  %cond252 = select i1 %tobool250.not, i32 2, i32 1
  store i32 %cond252, ptr %arg, align 4, !tbaa !17
  br label %sw.epilog286

sw.bb255:                                         ; preds = %if.else219
  %tobool256.not = icmp eq i8 %enable.0.ph475, 0
  %cond258 = select i1 %tobool256.not, i32 2, i32 1
  store i32 %cond258, ptr %arg, align 4, !tbaa !17
  br label %sw.epilog286

sw.bb261:                                         ; preds = %if.else219
  %tobool262.not = icmp eq i8 %enable.0.ph475, 0
  %cond264 = select i1 %tobool262.not, i32 2, i32 1
  store i32 %cond264, ptr %arg, align 4, !tbaa !17
  br label %sw.epilog286

sw.bb267:                                         ; preds = %if.else219
  %tobool268.not = icmp eq i8 %enable.0.ph475, 0
  %cond270 = select i1 %tobool268.not, i32 3, i32 2
  store i32 %cond270, ptr %arg, align 4, !tbaa !17
  br label %sw.epilog286

sw.bb273:                                         ; preds = %if.else219
  %tobool274.not = icmp eq i8 %enable.0.ph475, 0
  %cond276 = select i1 %tobool274.not, i32 3, i32 1
  store i32 %cond276, ptr %arg, align 4, !tbaa !17
  br label %sw.epilog286

sw.bb279:                                         ; preds = %if.else219
  %tobool280.not = icmp eq i8 %enable.0.ph475, 0
  %cond282 = select i1 %tobool280.not, i32 3, i32 2
  store i32 %cond282, ptr %arg, align 4, !tbaa !17
  br label %sw.epilog286

sw.default285:                                    ; preds = %if.else219
  tail call fastcc void @ref_usage() #12
  unreachable

sw.epilog286:                                     ; preds = %sw.bb279, %sw.bb273, %sw.bb267, %sw.bb261, %sw.bb255, %sw.bb249, %sw.bb243, %sw.bb237, %sw.bb231, %sw.bb225, %sw.bb220
  %.sink441 = phi i32 [ 23, %sw.bb279 ], [ 268435458, %sw.bb273 ], [ 268435458, %sw.bb267 ], [ 268435456, %sw.bb261 ], [ 21, %sw.bb255 ], [ 19, %sw.bb249 ], [ 17, %sw.bb243 ], [ 15, %sw.bb237 ], [ 9, %sw.bb231 ], [ 7, %sw.bb225 ], [ 13, %sw.bb220 ]
  %conv283 = sext i32 %pid.2418 to i64
  %call284 = call i32 @procctl(i32 noundef 0, i64 noundef %conv283, i32 noundef %.sink441, ptr noundef nonnull %arg) #10
  %cmp287.not = icmp eq i32 %call284, 0
  br i1 %cmp287.not, label %if.end290, label %if.then289

if.then289:                                       ; preds = %sw.epilog286
  call void (i32, ptr, ...) @err(i32 noundef 1, ptr noundef nonnull @.str.28) #13
  unreachable

if.end290:                                        ; preds = %sw.epilog286
  br i1 %cmp26.not, label %if.end296, label %if.then292

if.then292:                                       ; preds = %if.end290
  %idx.ext = sext i32 %6 to i64
  %add.ptr = getelementptr inbounds ptr, ptr %argv, i64 %idx.ext
  %32 = load ptr, ptr %add.ptr, align 8, !tbaa !8
  %call294 = call i32 @execvp(ptr noundef %32, ptr noundef nonnull %add.ptr) #10
  call void (i32, ptr, ...) @err(i32 noundef 1, ptr noundef nonnull @.str.29) #13
  unreachable

if.end296:                                        ; preds = %if.end290, %if.end171, %if.end78, %if.else92, %if.then90, %if.then103, %if.else105, %if.then98, %sw.bb109, %sw.bb112, %sw.bb110, %if.else129, %if.then127, %sw.epilog138, %sw.bb142, %sw.bb140, %sw.bb145, %sw.bb148, %sw.bb146, %if.else185, %if.then183, %if.else202, %if.then206, %if.then200, %sw.bb210, %sw.bb215, %sw.bb213, %sw.bb211
  call void @exit(i32 noundef 0) #13
  unreachable
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

declare i32 @getopt(i32 noundef, ptr noundef, ptr noundef) local_unnamed_addr #2

; Function Attrs: noreturn nounwind uwtable
define internal fastcc void @ref_usage() unnamed_addr #0 {
entry:
  %0 = load ptr, ptr @__stderrp, align 8, !tbaa !8
  %1 = tail call i64 @fwrite(ptr nonnull @.str.30, i64 7, i64 1, ptr %0)
  %2 = load ptr, ptr @__stderrp, align 8, !tbaa !8
  %3 = tail call i64 @fwrite(ptr nonnull @.str.31, i64 63, i64 1, ptr %2)
  %4 = load ptr, ptr @__stderrp, align 8, !tbaa !8
  %5 = tail call i64 @fwrite(ptr nonnull @.str.32, i64 36, i64 1, ptr %4)
  %6 = load ptr, ptr @__stderrp, align 8, !tbaa !8
  %7 = tail call i64 @fwrite(ptr nonnull @.str.33, i64 7, i64 1, ptr %6)
  %8 = load ptr, ptr @__stderrp, align 8, !tbaa !8
  %call5 = tail call i32 (ptr, ptr, ...) @fprintf(ptr noundef %8, ptr noundef nonnull @.str.34, ptr noundef nonnull @.str.35, ptr noundef nonnull @.str.37)
  %9 = load ptr, ptr @__stderrp, align 8, !tbaa !8
  %call5.1 = tail call i32 (ptr, ptr, ...) @fprintf(ptr noundef %9, ptr noundef nonnull @.str.34, ptr noundef nonnull @.str.36, ptr noundef nonnull @.str.38)
  %10 = load ptr, ptr @__stderrp, align 8, !tbaa !8
  %call5.2 = tail call i32 (ptr, ptr, ...) @fprintf(ptr noundef %10, ptr noundef nonnull @.str.34, ptr noundef nonnull @.str.36, ptr noundef nonnull @.str.39)
  %11 = load ptr, ptr @__stderrp, align 8, !tbaa !8
  %call5.3 = tail call i32 (ptr, ptr, ...) @fprintf(ptr noundef %11, ptr noundef nonnull @.str.34, ptr noundef nonnull @.str.36, ptr noundef nonnull @.str.40)
  %12 = load ptr, ptr @__stderrp, align 8, !tbaa !8
  %call5.4 = tail call i32 (ptr, ptr, ...) @fprintf(ptr noundef %12, ptr noundef nonnull @.str.34, ptr noundef nonnull @.str.36, ptr noundef nonnull @.str.41)
  %13 = load ptr, ptr @__stderrp, align 8, !tbaa !8
  %call5.5 = tail call i32 (ptr, ptr, ...) @fprintf(ptr noundef %13, ptr noundef nonnull @.str.34, ptr noundef nonnull @.str.36, ptr noundef nonnull @.str.42)
  %14 = load ptr, ptr @__stderrp, align 8, !tbaa !8
  %call5.6 = tail call i32 (ptr, ptr, ...) @fprintf(ptr noundef %14, ptr noundef nonnull @.str.34, ptr noundef nonnull @.str.36, ptr noundef nonnull @.str.43)
  %15 = load ptr, ptr @__stderrp, align 8, !tbaa !8
  %call5.7 = tail call i32 (ptr, ptr, ...) @fprintf(ptr noundef %15, ptr noundef nonnull @.str.34, ptr noundef nonnull @.str.36, ptr noundef nonnull @.str.44)
  %16 = load ptr, ptr @__stderrp, align 8, !tbaa !8
  %call5.8 = tail call i32 (ptr, ptr, ...) @fprintf(ptr noundef %16, ptr noundef nonnull @.str.34, ptr noundef nonnull @.str.36, ptr noundef nonnull @.str.45)
  %17 = load ptr, ptr @__stderrp, align 8, !tbaa !8
  %call5.9 = tail call i32 (ptr, ptr, ...) @fprintf(ptr noundef %17, ptr noundef nonnull @.str.34, ptr noundef nonnull @.str.36, ptr noundef nonnull @.str.46)
  %18 = load ptr, ptr @__stderrp, align 8, !tbaa !8
  %call5.10 = tail call i32 (ptr, ptr, ...) @fprintf(ptr noundef %18, ptr noundef nonnull @.str.34, ptr noundef nonnull @.str.36, ptr noundef nonnull @.str.47)
  %19 = load ptr, ptr @__stderrp, align 8, !tbaa !8
  %fputc = tail call i32 @fputc(i32 10, ptr %19)
  tail call void @exit(i32 noundef 1) #13
  unreachable
}

; Function Attrs: mustprogress nofree nounwind willreturn memory(argmem: read)
declare i32 @strcmp(ptr nocapture noundef, ptr nocapture noundef) local_unnamed_addr #3

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

declare i32 @getpid() local_unnamed_addr #2

declare i32 @procctl(i32 noundef, i64 noundef, i32 noundef, ptr noundef) local_unnamed_addr #2

; Function Attrs: noreturn
declare void @err(i32 noundef, ptr noundef, ...) local_unnamed_addr #4

; Function Attrs: nofree nounwind
declare noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #5

; Function Attrs: nofree
declare i32 @execvp(ptr noundef, ptr noundef) local_unnamed_addr #6

; Function Attrs: noreturn
declare void @exit(i32 noundef) local_unnamed_addr #4

; Function Attrs: nofree nounwind
declare noundef i32 @fprintf(ptr nocapture noundef, ptr nocapture noundef readonly, ...) local_unnamed_addr #5

; Function Attrs: mustprogress nofree nounwind willreturn
declare i64 @strtol(ptr noundef readonly, ptr nocapture noundef, i32 noundef) local_unnamed_addr #7

declare void @warnx(ptr noundef, ...) local_unnamed_addr #2

; Function Attrs: nofree nounwind
declare noundef i32 @puts(ptr nocapture noundef readonly) local_unnamed_addr #8

; Function Attrs: nofree nounwind
declare noundef i32 @putchar(i32 noundef) local_unnamed_addr #8

; Function Attrs: nofree nounwind
declare noundef i64 @fwrite(ptr nocapture noundef, i64 noundef, i64 noundef, ptr nocapture noundef) local_unnamed_addr #8

; Function Attrs: nofree nounwind
declare noundef i32 @fputc(i32 noundef, ptr nocapture noundef) local_unnamed_addr #8

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: read)
declare ptr @llvm.load.relative.i64(ptr, i64) #9

attributes #0 = { noreturn nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { mustprogress nofree nounwind willreturn memory(argmem: read) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { noreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { nofree nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { nofree "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #7 = { mustprogress nofree nounwind willreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #8 = { nofree nounwind }
attributes #9 = { nocallback nofree nosync nounwind willreturn memory(argmem: read) }
attributes #10 = { nounwind }
attributes #11 = { nounwind willreturn memory(read) }
attributes #12 = { noreturn }
attributes #13 = { noreturn nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!5 = distinct !{!5, !6, !7}
!6 = !{!"llvm.loop.mustprogress"}
!7 = !{!"llvm.loop.peeled.count", i32 1}
!8 = !{!9, !9, i64 0}
!9 = !{!"any pointer", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C/C++ TBAA"}
!12 = !{!13, !14, i64 0}
!13 = !{!"", !14, i64 0, !9, i64 8}
!14 = !{!"int", !10, i64 0}
!15 = distinct !{!15, !6}
!16 = !{!10, !10, i64 0}
!17 = !{!14, !14, i64 0}
