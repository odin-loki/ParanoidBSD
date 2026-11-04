; ModuleID = '/home/odin/pbsd/pbsd/lib/libthread_db/libpthread_db_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/libthread_db/libpthread_db_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

module asm ".symver __qsort_r_compat, qsort_r@FBSD_1.0"
module asm ".weak __start_set___ta_ops"
module asm ".weak __stop_set___ta_ops"

%struct.ta_ops = type { ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr }
%struct.td_thragent = type { ptr, %struct.anon, ptr, i64, i64, i64, i64, i64, i64, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, ptr, i32 }
%struct.anon = type { ptr, ptr }
%struct.reg = type { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i32, i16, i16, i32, i16, i16, i64, i64, i64, i64, i64 }
%struct.pt_map = type { i32, %union.anon }
%union.anon = type { i64 }
%struct.td_thrhandle_t = type { ptr, i64, i64 }
%struct.ptrace_lwpinfo = type { i32, i32, i32, %struct.__sigset, %struct.__sigset, %struct.__siginfo, [20 x i8], i32, i32, i32 }
%struct.__sigset = type { [4 x i32] }
%struct.__siginfo = type { i32, i32, i32, i32, i32, i32, ptr, %union.sigval, %union.anon.0 }
%union.sigval = type { ptr }
%union.anon.0 = type { %struct.anon.6 }
%struct.anon.6 = type { i64, [7 x i32] }
%struct.td_old_thrinfo_t = type { ptr, i64, i64, i32, i32, i32, i32, i32, i8, i8, %struct.__sigset, %struct.__sigset, i64, i64, i64, i64 }
%struct.td_thrinfo_t = type { ptr, i64, i64, i32, i32, i32, i32, i32, i8, i8, %struct.__sigset, %struct.__sigset, i64, i64, i64, i64, %struct.__siginfo }
%struct.kse_thr_mailbox = type { %struct.__ucontext, i32, ptr, ptr, i32, i32, %struct.__siginfo, i32, i32, [6 x i32] }
%struct.__ucontext = type { %struct.__sigset, %struct.__mcontext, ptr, %struct.sigaltstack, i32, [4 x i32], [12 x i8] }
%struct.__mcontext = type { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i32, i16, i16, i64, i32, i16, i16, i64, i64, i64, i64, i64, i64, i64, i64, i64, [64 x i64], i64, i64, i64, i64, i64, [3 x i64] }
%struct.sigaltstack = type { ptr, i64, i32 }

@libpthread_db_ops = internal global %struct.ta_ops { ptr @ref_pt_init, ptr @ref_pt_ta_clear_event, ptr @ref_pt_ta_delete, ptr @ref_pt_ta_event_addr, ptr @ref_pt_ta_event_getmsg, ptr @ref_pt_ta_map_id2thr, ptr @ref_pt_ta_map_lwp2thr, ptr @ref_pt_ta_new, ptr @ref_pt_ta_set_event, ptr @ref_pt_ta_thr_iter, ptr @ref_pt_ta_tsd_iter, ptr @ref_pt_thr_clear_event, ptr @ref_pt_thr_dbresume, ptr @ref_pt_thr_dbsuspend, ptr @ref_pt_thr_event_enable, ptr @ref_pt_thr_event_getmsg, ptr @ref_pt_thr_old_get_info, ptr @ref_pt_thr_get_info, ptr @ref_pt_thr_getfpregs, ptr @ref_pt_thr_getgregs, ptr @ref_pt_thr_set_event, ptr @ref_pt_thr_setfpregs, ptr @ref_pt_thr_setgregs, ptr @ref_pt_thr_validate, ptr @ref_pt_thr_tls_get_addr, ptr @ref_pt_thr_sstep }, align 8
@__set___ta_ops_sym_libpthread_db_ops = internal constant ptr @libpthread_db_ops, section "set___ta_ops", align 8
@.str = private unnamed_addr constant [14 x i8] c"_libkse_debug\00", align 1
@.str.1 = private unnamed_addr constant [13 x i8] c"_thread_list\00", align 1
@.str.2 = private unnamed_addr constant [18 x i8] c"_thread_activated\00", align 1
@.str.3 = private unnamed_addr constant [23 x i8] c"_thread_active_threads\00", align 1
@.str.4 = private unnamed_addr constant [17 x i8] c"_thread_keytable\00", align 1
@.str.5 = private unnamed_addr constant [16 x i8] c"_thread_off_dtv\00", align 1
@.str.6 = private unnamed_addr constant [26 x i8] c"_thread_off_kse_locklevel\00", align 1
@.str.7 = private unnamed_addr constant [16 x i8] c"_thread_off_kse\00", align 1
@.str.8 = private unnamed_addr constant [21 x i8] c"_thread_off_tlsindex\00", align 1
@.str.9 = private unnamed_addr constant [23 x i8] c"_thread_off_attr_flags\00", align 1
@.str.10 = private unnamed_addr constant [17 x i8] c"_thread_size_key\00", align 1
@.str.11 = private unnamed_addr constant [16 x i8] c"_thread_off_tcb\00", align 1
@.str.12 = private unnamed_addr constant [20 x i8] c"_thread_off_linkmap\00", align 1
@.str.13 = private unnamed_addr constant [17 x i8] c"_thread_off_tmbx\00", align 1
@.str.14 = private unnamed_addr constant [26 x i8] c"_thread_off_thr_locklevel\00", align 1
@.str.15 = private unnamed_addr constant [17 x i8] c"_thread_off_next\00", align 1
@.str.16 = private unnamed_addr constant [18 x i8] c"_thread_off_state\00", align 1
@.str.17 = private unnamed_addr constant [17 x i8] c"_thread_max_keys\00", align 1
@.str.18 = private unnamed_addr constant [26 x i8] c"_thread_off_key_allocated\00", align 1
@.str.19 = private unnamed_addr constant [27 x i8] c"_thread_off_key_destructor\00", align 1
@.str.20 = private unnamed_addr constant [22 x i8] c"_thread_state_running\00", align 1
@.str.21 = private unnamed_addr constant [22 x i8] c"_thread_state_zoombie\00", align 1
@.str.22 = private unnamed_addr constant [20 x i8] c"_thread_off_sigmask\00", align 1
@.str.23 = private unnamed_addr constant [20 x i8] c"_thread_off_sigpend\00", align 1
@llvm.compiler.used = appending global [1 x ptr] [ptr @__set___ta_ops_sym_libpthread_db_ops], section "llvm.metadata"

; Function Attrs: nounwind uwtable
define internal noundef i32 @ref_pt_init() #0 {
entry:
  tail call void @pt_md_init() #11
  ret i32 0
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define internal noundef i32 @ref_pt_ta_clear_event(ptr nocapture readnone %ta, ptr nocapture readnone %events) #1 {
entry:
  ret i32 0
}

; Function Attrs: nounwind uwtable
define internal noundef i32 @ref_pt_ta_delete(ptr nocapture noundef %ta) #0 {
entry:
  %dbg = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %dbg) #11
  store i32 0, ptr %dbg, align 4, !tbaa !5
  %ph = getelementptr inbounds %struct.td_thragent, ptr %ta, i64 0, i32 2
  %0 = load ptr, ptr %ph, align 8, !tbaa !9
  %libkse_debug_addr = getelementptr inbounds %struct.td_thragent, ptr %ta, i64 0, i32 3
  %1 = load i64, ptr %libkse_debug_addr, align 8, !tbaa !14
  %call = call i32 @ps_pwrite(ptr noundef %0, i64 noundef %1, ptr noundef nonnull %dbg, i64 noundef 4) #11
  %map = getelementptr inbounds %struct.td_thragent, ptr %ta, i64 0, i32 29
  %2 = load ptr, ptr %map, align 8, !tbaa !15
  %tobool.not = icmp eq ptr %2, null
  br i1 %tobool.not, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  call void @free(ptr noundef nonnull %2)
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  call void @free(ptr noundef nonnull %ta)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %dbg) #11
  ret i32 0
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define internal noundef i32 @ref_pt_ta_event_addr(ptr nocapture readnone %ta, i32 %event, ptr nocapture readnone %ptr) #1 {
entry:
  ret i32 -1
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define internal noundef i32 @ref_pt_ta_event_getmsg(ptr nocapture readnone %ta, ptr nocapture readnone %msg) #1 {
entry:
  ret i32 14
}

; Function Attrs: nounwind uwtable
define internal noundef i32 @ref_pt_ta_map_id2thr(ptr noundef %ta, i64 noundef %id, ptr nocapture noundef writeonly %th) #0 {
entry:
  %gregs = alloca [1 x %struct.reg], align 16
  %pt = alloca i64, align 8
  %tcb_addr = alloca i64, align 8
  %lwp = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 176, ptr nonnull %gregs) #11
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %pt) #11
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %tcb_addr) #11
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %lwp) #11
  %cmp = icmp slt i64 %id, 0
  br i1 %cmp, label %cleanup, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %entry
  %map_len = getelementptr inbounds %struct.td_thragent, ptr %ta, i64 0, i32 30
  %0 = load i32, ptr %map_len, align 8, !tbaa !16
  %conv = zext i32 %0 to i64
  %cmp1.not = icmp ugt i64 %conv, %id
  br i1 %cmp1.not, label %lor.lhs.false3, label %cleanup

lor.lhs.false3:                                   ; preds = %lor.lhs.false
  %map = getelementptr inbounds %struct.td_thragent, ptr %ta, i64 0, i32 29
  %1 = load ptr, ptr %map, align 8, !tbaa !15
  %arrayidx = getelementptr inbounds %struct.pt_map, ptr %1, i64 %id
  %2 = load i32, ptr %arrayidx, align 8, !tbaa !17
  %cmp4 = icmp eq i32 %2, 0
  br i1 %cmp4, label %cleanup, label %if.end

if.end:                                           ; preds = %lor.lhs.false3
  %thread_list_addr = getelementptr inbounds %struct.td_thragent, ptr %ta, i64 0, i32 4
  %3 = load i64, ptr %thread_list_addr, align 8, !tbaa !19
  %call = call i32 @thr_pread_ptr(ptr noundef nonnull %ta, i64 noundef %3, ptr noundef nonnull %pt) #11
  %cmp6.not = icmp eq i32 %call, 0
  br i1 %cmp6.not, label %if.end9, label %cleanup

if.end9:                                          ; preds = %if.end
  %4 = load ptr, ptr %map, align 8, !tbaa !15
  %arrayidx11 = getelementptr inbounds %struct.pt_map, ptr %4, i64 %id
  %5 = load i32, ptr %arrayidx11, align 8, !tbaa !17
  %cmp13 = icmp eq i32 %5, 2
  %thread_off_tcb = getelementptr inbounds %struct.td_thragent, ptr %ta, i64 0, i32 16
  br i1 %cmp13, label %while.cond.preheader, label %while.cond58.preheader

while.cond58.preheader:                           ; preds = %if.end9
  %thread_off_next74 = getelementptr inbounds %struct.td_thragent, ptr %ta, i64 0, i32 20
  br label %while.cond58

while.cond.preheader:                             ; preds = %if.end9
  %thread_off_tmbx = getelementptr inbounds %struct.td_thragent, ptr %ta, i64 0, i32 18
  %thread_off_next = getelementptr inbounds %struct.td_thragent, ptr %ta, i64 0, i32 20
  br label %while.cond

while.cond:                                       ; preds = %while.cond.preheader, %if.end40
  %6 = load i64, ptr %pt, align 8, !tbaa !20
  %cmp16.not = icmp eq i64 %6, 0
  br i1 %cmp16.not, label %while.end, label %while.body

while.body:                                       ; preds = %while.cond
  %7 = load i32, ptr %thread_off_tcb, align 4, !tbaa !21
  %conv18 = sext i32 %7 to i64
  %add = add i64 %6, %conv18
  %call19 = call i32 @thr_pread_ptr(ptr noundef nonnull %ta, i64 noundef %add, ptr noundef nonnull %tcb_addr) #11
  %cmp20.not = icmp eq i32 %call19, 0
  br i1 %cmp20.not, label %if.end23, label %cleanup

if.end23:                                         ; preds = %while.body
  %8 = load i64, ptr %tcb_addr, align 8, !tbaa !20
  %9 = load i32, ptr %thread_off_tmbx, align 4, !tbaa !22
  %conv24 = sext i32 %9 to i64
  %add25 = add i64 %8, 996
  %add26 = add i64 %add25, %conv24
  %call27 = call i32 @thr_pread_int(ptr noundef nonnull %ta, i64 noundef %add26, ptr noundef nonnull %lwp) #11
  %cmp28.not = icmp eq i32 %call27, 0
  br i1 %cmp28.not, label %if.end31, label %cleanup

if.end31:                                         ; preds = %if.end23
  %10 = load i32, ptr %lwp, align 4, !tbaa !5
  %11 = load ptr, ptr %map, align 8, !tbaa !15
  %12 = getelementptr inbounds %struct.pt_map, ptr %11, i64 %id, i32 1
  %13 = load i32, ptr %12, align 8, !tbaa !23
  %cmp34 = icmp eq i32 %10, %13
  br i1 %cmp34, label %if.then36, label %if.end40

if.then36:                                        ; preds = %if.end31
  %arrayidx33 = getelementptr inbounds %struct.pt_map, ptr %11, i64 %id
  store i32 0, ptr %arrayidx33, align 8, !tbaa !17
  br label %cleanup

if.end40:                                         ; preds = %if.end31
  %14 = load i64, ptr %pt, align 8, !tbaa !20
  %15 = load i32, ptr %thread_off_next, align 4, !tbaa !24
  %conv41 = sext i32 %15 to i64
  %add42 = add i64 %14, %conv41
  %call43 = call i32 @thr_pread_ptr(ptr noundef nonnull %ta, i64 noundef %add42, ptr noundef nonnull %pt) #11
  %cmp44.not = icmp eq i32 %call43, 0
  br i1 %cmp44.not, label %while.cond, label %cleanup, !llvm.loop !25

while.end:                                        ; preds = %while.cond
  %ph = getelementptr inbounds %struct.td_thragent, ptr %ta, i64 0, i32 2
  %16 = load ptr, ptr %ph, align 8, !tbaa !9
  %17 = load ptr, ptr %map, align 8, !tbaa !15
  %18 = getelementptr inbounds %struct.pt_map, ptr %17, i64 %id, i32 1
  %19 = load i32, ptr %18, align 8, !tbaa !23
  %call50 = call i32 @ps_lgetregs(ptr noundef %16, i32 noundef %19, ptr noundef nonnull %gregs) #11
  %cmp51.not = icmp eq i32 %call50, 0
  br i1 %cmp51.not, label %while.end.if.end90_crit_edge, label %if.then53

while.end.if.end90_crit_edge:                     ; preds = %while.end
  %.pre = load i64, ptr %pt, align 8, !tbaa !20
  br label %if.end90

if.then53:                                        ; preds = %while.end
  %20 = load ptr, ptr %map, align 8, !tbaa !15
  %arrayidx55 = getelementptr inbounds %struct.pt_map, ptr %20, i64 %id
  store i32 0, ptr %arrayidx55, align 8, !tbaa !17
  br label %cleanup

while.cond58:                                     ; preds = %while.cond58.preheader, %if.end73
  %21 = load i64, ptr %pt, align 8, !tbaa !20
  %cmp59.not = icmp eq i64 %21, 0
  %22 = load ptr, ptr %map, align 8, !tbaa !15
  br i1 %cmp59.not, label %if.then85, label %land.rhs

land.rhs:                                         ; preds = %while.cond58
  %23 = getelementptr inbounds %struct.pt_map, ptr %22, i64 %id, i32 1
  %24 = load i64, ptr %23, align 8, !tbaa !23
  %cmp63.not = icmp eq i64 %24, %21
  br i1 %cmp63.not, label %if.end90, label %while.body65

while.body65:                                     ; preds = %land.rhs
  %25 = load i32, ptr %thread_off_tcb, align 4, !tbaa !21
  %conv67 = sext i32 %25 to i64
  %add68 = add i64 %21, %conv67
  %call69 = call i32 @thr_pread_ptr(ptr noundef nonnull %ta, i64 noundef %add68, ptr noundef nonnull %tcb_addr) #11
  %cmp70.not = icmp eq i32 %call69, 0
  br i1 %cmp70.not, label %if.end73, label %cleanup

if.end73:                                         ; preds = %while.body65
  %26 = load i64, ptr %pt, align 8, !tbaa !20
  %27 = load i32, ptr %thread_off_next74, align 4, !tbaa !24
  %conv75 = sext i32 %27 to i64
  %add76 = add i64 %26, %conv75
  %call77 = call i32 @thr_pread_ptr(ptr noundef nonnull %ta, i64 noundef %add76, ptr noundef nonnull %pt) #11
  %cmp78.not = icmp eq i32 %call77, 0
  br i1 %cmp78.not, label %while.cond58, label %cleanup, !llvm.loop !27

if.then85:                                        ; preds = %while.cond58
  %arrayidx87 = getelementptr inbounds %struct.pt_map, ptr %22, i64 %id
  store i32 0, ptr %arrayidx87, align 8, !tbaa !17
  br label %cleanup

if.end90:                                         ; preds = %land.rhs, %while.end.if.end90_crit_edge
  %28 = phi i64 [ %.pre, %while.end.if.end90_crit_edge ], [ %21, %land.rhs ]
  store ptr %ta, ptr %th, align 8, !tbaa !28
  %th_tid = getelementptr inbounds %struct.td_thrhandle_t, ptr %th, i64 0, i32 2
  store i64 %id, ptr %th_tid, align 8, !tbaa !30
  %th_thread = getelementptr inbounds %struct.td_thrhandle_t, ptr %th, i64 0, i32 1
  store i64 %28, ptr %th_thread, align 8, !tbaa !31
  br label %cleanup

cleanup:                                          ; preds = %if.end73, %while.body65, %if.end40, %if.end23, %while.body, %if.end, %entry, %lor.lhs.false, %lor.lhs.false3, %if.end90, %if.then85, %if.then53, %if.then36
  %retval.0 = phi i32 [ 16, %if.then36 ], [ 16, %if.then53 ], [ 0, %if.end90 ], [ 16, %if.then85 ], [ 16, %lor.lhs.false3 ], [ 16, %lor.lhs.false ], [ 16, %entry ], [ -1, %if.end ], [ -1, %while.body ], [ -1, %if.end23 ], [ -1, %if.end40 ], [ -1, %while.body65 ], [ -1, %if.end73 ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %lwp) #11
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %tcb_addr) #11
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %pt) #11
  call void @llvm.lifetime.end.p0(i64 176, ptr nonnull %gregs) #11
  ret i32 %retval.0
}

; Function Attrs: nounwind uwtable
define internal noundef i32 @ref_pt_ta_map_lwp2thr(ptr noundef %ta, i32 noundef %lwp, ptr nocapture noundef writeonly %th) #0 {
entry:
  %pt = alloca i64, align 8
  %tcb_addr = alloca i64, align 8
  %lwp1 = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %pt) #11
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %tcb_addr) #11
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %lwp1) #11
  %thread_list_addr = getelementptr inbounds %struct.td_thragent, ptr %ta, i64 0, i32 4
  %0 = load i64, ptr %thread_list_addr, align 8, !tbaa !19
  %call = call i32 @thr_pread_ptr(ptr noundef %ta, i64 noundef %0, ptr noundef nonnull %pt) #11
  %cmp.not = icmp eq i32 %call, 0
  br i1 %cmp.not, label %while.cond.preheader, label %cleanup

while.cond.preheader:                             ; preds = %entry
  %thread_off_tcb = getelementptr inbounds %struct.td_thragent, ptr %ta, i64 0, i32 16
  %thread_off_tmbx = getelementptr inbounds %struct.td_thragent, ptr %ta, i64 0, i32 18
  %thread_off_next = getelementptr inbounds %struct.td_thragent, ptr %ta, i64 0, i32 20
  br label %while.cond

while.cond:                                       ; preds = %while.cond.preheader, %if.end24
  %1 = load i64, ptr %pt, align 8, !tbaa !20
  %cmp1.not = icmp eq i64 %1, 0
  br i1 %cmp1.not, label %cleanup, label %while.body

while.body:                                       ; preds = %while.cond
  %2 = load i32, ptr %thread_off_tcb, align 4, !tbaa !21
  %conv = sext i32 %2 to i64
  %add = add i64 %1, %conv
  %call2 = call i32 @thr_pread_ptr(ptr noundef nonnull %ta, i64 noundef %add, ptr noundef nonnull %tcb_addr) #11
  %cmp3.not = icmp eq i32 %call2, 0
  br i1 %cmp3.not, label %if.end6, label %cleanup

if.end6:                                          ; preds = %while.body
  %3 = load i64, ptr %tcb_addr, align 8, !tbaa !20
  %4 = load i32, ptr %thread_off_tmbx, align 4, !tbaa !22
  %conv7 = sext i32 %4 to i64
  %add8 = add i64 %3, 996
  %add9 = add i64 %add8, %conv7
  %call10 = call i32 @thr_pread_int(ptr noundef nonnull %ta, i64 noundef %add9, ptr noundef nonnull %lwp1) #11
  %cmp11.not = icmp eq i32 %call10, 0
  br i1 %cmp11.not, label %if.end14, label %cleanup

if.end14:                                         ; preds = %if.end6
  %5 = load i32, ptr %lwp1, align 4, !tbaa !5
  %cmp15 = icmp eq i32 %5, %lwp
  br i1 %cmp15, label %if.then17, label %if.end24

if.then17:                                        ; preds = %if.end14
  store ptr %ta, ptr %th, align 8, !tbaa !28
  %6 = load i64, ptr %pt, align 8, !tbaa !20
  %call18 = call fastcc i64 @ref_pt_map_thread(ptr noundef nonnull %ta, i64 noundef %6)
  %th_tid = getelementptr inbounds %struct.td_thrhandle_t, ptr %th, i64 0, i32 2
  store i64 %call18, ptr %th_tid, align 8, !tbaa !30
  %cmp20 = icmp eq i64 %call18, -1
  br i1 %cmp20, label %cleanup, label %if.end23

if.end23:                                         ; preds = %if.then17
  %map_len.i = getelementptr inbounds %struct.td_thragent, ptr %ta, i64 0, i32 30
  %7 = load i32, ptr %map_len.i, align 8, !tbaa !16
  %cmp17.not.i = icmp eq i32 %7, 0
  br i1 %cmp17.not.i, label %ref_pt_unmap_lwp.exit, label %for.body.lr.ph.i

for.body.lr.ph.i:                                 ; preds = %if.end23
  %map.i = getelementptr inbounds %struct.td_thragent, ptr %ta, i64 0, i32 29
  %8 = load ptr, ptr %map.i, align 8, !tbaa !15
  %wide.trip.count.i = zext i32 %7 to i64
  br label %for.body.i

for.body.i:                                       ; preds = %for.inc.i, %for.body.lr.ph.i
  %indvars.iv.i = phi i64 [ 0, %for.body.lr.ph.i ], [ %indvars.iv.next.i, %for.inc.i ]
  %arrayidx.i = getelementptr inbounds %struct.pt_map, ptr %8, i64 %indvars.iv.i
  %9 = load i32, ptr %arrayidx.i, align 8, !tbaa !17
  %cmp1.i = icmp eq i32 %9, 2
  br i1 %cmp1.i, label %land.lhs.true.i, label %for.inc.i

land.lhs.true.i:                                  ; preds = %for.body.i
  %10 = getelementptr inbounds %struct.pt_map, ptr %8, i64 %indvars.iv.i, i32 1
  %11 = load i32, ptr %10, align 8, !tbaa !23
  %cmp5.i = icmp eq i32 %11, %lwp
  br i1 %cmp5.i, label %if.then.i, label %for.inc.i

if.then.i:                                        ; preds = %land.lhs.true.i
  store i32 0, ptr %arrayidx.i, align 8, !tbaa !17
  br label %ref_pt_unmap_lwp.exit

for.inc.i:                                        ; preds = %land.lhs.true.i, %for.body.i
  %indvars.iv.next.i = add nuw nsw i64 %indvars.iv.i, 1
  %exitcond.not.i = icmp eq i64 %indvars.iv.next.i, %wide.trip.count.i
  br i1 %exitcond.not.i, label %ref_pt_unmap_lwp.exit, label %for.body.i, !llvm.loop !32

ref_pt_unmap_lwp.exit:                            ; preds = %for.inc.i, %if.end23, %if.then.i
  %12 = load i64, ptr %pt, align 8, !tbaa !20
  %th_thread = getelementptr inbounds %struct.td_thrhandle_t, ptr %th, i64 0, i32 1
  store i64 %12, ptr %th_thread, align 8, !tbaa !31
  br label %cleanup

if.end24:                                         ; preds = %if.end14
  %13 = load i64, ptr %pt, align 8, !tbaa !20
  %14 = load i32, ptr %thread_off_next, align 4, !tbaa !24
  %conv25 = sext i32 %14 to i64
  %add26 = add i64 %13, %conv25
  %call27 = call i32 @thr_pread_ptr(ptr noundef nonnull %ta, i64 noundef %add26, ptr noundef nonnull %pt) #11
  %cmp28.not = icmp eq i32 %call27, 0
  br i1 %cmp28.not, label %while.cond, label %cleanup, !llvm.loop !33

cleanup:                                          ; preds = %while.cond, %if.end24, %if.end6, %while.body, %if.then17, %entry, %ref_pt_unmap_lwp.exit
  %retval.0 = phi i32 [ 0, %ref_pt_unmap_lwp.exit ], [ -1, %entry ], [ 7, %if.then17 ], [ 16, %while.cond ], [ -1, %if.end24 ], [ -1, %if.end6 ], [ -1, %while.body ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %lwp1) #11
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %tcb_addr) #11
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %pt) #11
  ret i32 %retval.0
}

; Function Attrs: nounwind uwtable
define internal noundef i32 @ref_pt_ta_new(ptr noundef %ph, ptr nocapture noundef writeonly %pta) #0 {
entry:
  %vaddr = alloca i64, align 8
  %dbg = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %vaddr) #11
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %dbg) #11
  %call = tail call noalias dereferenceable_or_null(176) ptr @malloc(i64 noundef 176) #12
  %cmp = icmp eq ptr %call, null
  br i1 %cmp, label %cleanup, label %if.end

if.end:                                           ; preds = %entry
  %ph1 = getelementptr inbounds %struct.td_thragent, ptr %call, i64 0, i32 2
  store ptr %ph, ptr %ph1, align 8, !tbaa !9
  %thread_activated = getelementptr inbounds %struct.td_thragent, ptr %call, i64 0, i32 9
  store i32 0, ptr %thread_activated, align 8, !tbaa !34
  %map = getelementptr inbounds %struct.td_thragent, ptr %call, i64 0, i32 29
  store ptr null, ptr %map, align 8, !tbaa !15
  %map_len = getelementptr inbounds %struct.td_thragent, ptr %call, i64 0, i32 30
  store i32 0, ptr %map_len, align 8, !tbaa !16
  %libkse_debug_addr = getelementptr inbounds %struct.td_thragent, ptr %call, i64 0, i32 3
  %call2 = tail call i32 @ps_pglobal_lookup(ptr noundef %ph, ptr noundef null, ptr noundef nonnull @.str, ptr noundef nonnull %libkse_debug_addr) #11
  %cmp3.not = icmp eq i32 %call2, 0
  br i1 %cmp3.not, label %if.end5, label %error

if.end5:                                          ; preds = %if.end
  %thread_list_addr = getelementptr inbounds %struct.td_thragent, ptr %call, i64 0, i32 4
  %call6 = tail call i32 @ps_pglobal_lookup(ptr noundef %ph, ptr noundef null, ptr noundef nonnull @.str.1, ptr noundef nonnull %thread_list_addr) #11
  %cmp7.not = icmp eq i32 %call6, 0
  br i1 %cmp7.not, label %if.end9, label %error

if.end9:                                          ; preds = %if.end5
  %thread_activated_addr = getelementptr inbounds %struct.td_thragent, ptr %call, i64 0, i32 6
  %call10 = tail call i32 @ps_pglobal_lookup(ptr noundef %ph, ptr noundef null, ptr noundef nonnull @.str.2, ptr noundef nonnull %thread_activated_addr) #11
  %cmp11.not = icmp eq i32 %call10, 0
  br i1 %cmp11.not, label %if.end13, label %error

if.end13:                                         ; preds = %if.end9
  %thread_active_threads_addr = getelementptr inbounds %struct.td_thragent, ptr %call, i64 0, i32 7
  %call14 = tail call i32 @ps_pglobal_lookup(ptr noundef %ph, ptr noundef null, ptr noundef nonnull @.str.3, ptr noundef nonnull %thread_active_threads_addr) #11
  %cmp15.not = icmp eq i32 %call14, 0
  br i1 %cmp15.not, label %if.end17, label %error

if.end17:                                         ; preds = %if.end13
  %thread_keytable_addr = getelementptr inbounds %struct.td_thragent, ptr %call, i64 0, i32 8
  %call18 = tail call i32 @ps_pglobal_lookup(ptr noundef %ph, ptr noundef null, ptr noundef nonnull @.str.4, ptr noundef nonnull %thread_keytable_addr) #11
  %cmp19.not = icmp eq i32 %call18, 0
  br i1 %cmp19.not, label %if.end21, label %error

if.end21:                                         ; preds = %if.end17
  %call22 = call i32 @ps_pglobal_lookup(ptr noundef %ph, ptr noundef null, ptr noundef nonnull @.str.5, ptr noundef nonnull %vaddr) #11
  %cmp23.not = icmp eq i32 %call22, 0
  br i1 %cmp23.not, label %if.end25, label %error

if.end25:                                         ; preds = %if.end21
  %0 = load i64, ptr %vaddr, align 8, !tbaa !20
  %thread_off_dtv = getelementptr inbounds %struct.td_thragent, ptr %call, i64 0, i32 10
  %call26 = call i32 @ps_pread(ptr noundef %ph, i64 noundef %0, ptr noundef nonnull %thread_off_dtv, i64 noundef 4) #11
  %cmp27.not = icmp eq i32 %call26, 0
  br i1 %cmp27.not, label %if.end29, label %error

if.end29:                                         ; preds = %if.end25
  %call30 = call i32 @ps_pglobal_lookup(ptr noundef %ph, ptr noundef null, ptr noundef nonnull @.str.6, ptr noundef nonnull %vaddr) #11
  %cmp31.not = icmp eq i32 %call30, 0
  br i1 %cmp31.not, label %if.end33, label %error

if.end33:                                         ; preds = %if.end29
  %1 = load i64, ptr %vaddr, align 8, !tbaa !20
  %thread_off_kse_locklevel = getelementptr inbounds %struct.td_thragent, ptr %call, i64 0, i32 11
  %call34 = call i32 @ps_pread(ptr noundef %ph, i64 noundef %1, ptr noundef nonnull %thread_off_kse_locklevel, i64 noundef 4) #11
  %cmp35.not = icmp eq i32 %call34, 0
  br i1 %cmp35.not, label %if.end37, label %error

if.end37:                                         ; preds = %if.end33
  %call38 = call i32 @ps_pglobal_lookup(ptr noundef %ph, ptr noundef null, ptr noundef nonnull @.str.7, ptr noundef nonnull %vaddr) #11
  %cmp39.not = icmp eq i32 %call38, 0
  br i1 %cmp39.not, label %if.end41, label %error

if.end41:                                         ; preds = %if.end37
  %2 = load i64, ptr %vaddr, align 8, !tbaa !20
  %thread_off_kse = getelementptr inbounds %struct.td_thragent, ptr %call, i64 0, i32 12
  %call42 = call i32 @ps_pread(ptr noundef %ph, i64 noundef %2, ptr noundef nonnull %thread_off_kse, i64 noundef 4) #11
  %cmp43.not = icmp eq i32 %call42, 0
  br i1 %cmp43.not, label %if.end45, label %error

if.end45:                                         ; preds = %if.end41
  %call46 = call i32 @ps_pglobal_lookup(ptr noundef %ph, ptr noundef null, ptr noundef nonnull @.str.8, ptr noundef nonnull %vaddr) #11
  %cmp47.not = icmp eq i32 %call46, 0
  br i1 %cmp47.not, label %if.end49, label %error

if.end49:                                         ; preds = %if.end45
  %3 = load i64, ptr %vaddr, align 8, !tbaa !20
  %thread_off_tlsindex = getelementptr inbounds %struct.td_thragent, ptr %call, i64 0, i32 13
  %call50 = call i32 @ps_pread(ptr noundef %ph, i64 noundef %3, ptr noundef nonnull %thread_off_tlsindex, i64 noundef 4) #11
  %cmp51.not = icmp eq i32 %call50, 0
  br i1 %cmp51.not, label %if.end53, label %error

if.end53:                                         ; preds = %if.end49
  %call54 = call i32 @ps_pglobal_lookup(ptr noundef %ph, ptr noundef null, ptr noundef nonnull @.str.9, ptr noundef nonnull %vaddr) #11
  %cmp55.not = icmp eq i32 %call54, 0
  br i1 %cmp55.not, label %if.end57, label %error

if.end57:                                         ; preds = %if.end53
  %4 = load i64, ptr %vaddr, align 8, !tbaa !20
  %thread_off_attr_flags = getelementptr inbounds %struct.td_thragent, ptr %call, i64 0, i32 14
  %call58 = call i32 @ps_pread(ptr noundef %ph, i64 noundef %4, ptr noundef nonnull %thread_off_attr_flags, i64 noundef 4) #11
  %cmp59.not = icmp eq i32 %call58, 0
  br i1 %cmp59.not, label %if.end61, label %error

if.end61:                                         ; preds = %if.end57
  %call62 = call i32 @ps_pglobal_lookup(ptr noundef %ph, ptr noundef null, ptr noundef nonnull @.str.10, ptr noundef nonnull %vaddr) #11
  %cmp63.not = icmp eq i32 %call62, 0
  br i1 %cmp63.not, label %if.end65, label %error

if.end65:                                         ; preds = %if.end61
  %5 = load i64, ptr %vaddr, align 8, !tbaa !20
  %thread_size_key = getelementptr inbounds %struct.td_thragent, ptr %call, i64 0, i32 15
  %call66 = call i32 @ps_pread(ptr noundef %ph, i64 noundef %5, ptr noundef nonnull %thread_size_key, i64 noundef 4) #11
  %cmp67.not = icmp eq i32 %call66, 0
  br i1 %cmp67.not, label %if.end69, label %error

if.end69:                                         ; preds = %if.end65
  %call70 = call i32 @ps_pglobal_lookup(ptr noundef %ph, ptr noundef null, ptr noundef nonnull @.str.11, ptr noundef nonnull %vaddr) #11
  %cmp71.not = icmp eq i32 %call70, 0
  br i1 %cmp71.not, label %if.end73, label %error

if.end73:                                         ; preds = %if.end69
  %6 = load i64, ptr %vaddr, align 8, !tbaa !20
  %thread_off_tcb = getelementptr inbounds %struct.td_thragent, ptr %call, i64 0, i32 16
  %call74 = call i32 @ps_pread(ptr noundef %ph, i64 noundef %6, ptr noundef nonnull %thread_off_tcb, i64 noundef 4) #11
  %cmp75.not = icmp eq i32 %call74, 0
  br i1 %cmp75.not, label %if.end77, label %error

if.end77:                                         ; preds = %if.end73
  %call78 = call i32 @ps_pglobal_lookup(ptr noundef %ph, ptr noundef null, ptr noundef nonnull @.str.12, ptr noundef nonnull %vaddr) #11
  %cmp79.not = icmp eq i32 %call78, 0
  br i1 %cmp79.not, label %if.end81, label %error

if.end81:                                         ; preds = %if.end77
  %7 = load i64, ptr %vaddr, align 8, !tbaa !20
  %thread_off_linkmap = getelementptr inbounds %struct.td_thragent, ptr %call, i64 0, i32 17
  %call82 = call i32 @ps_pread(ptr noundef %ph, i64 noundef %7, ptr noundef nonnull %thread_off_linkmap, i64 noundef 4) #11
  %cmp83.not = icmp eq i32 %call82, 0
  br i1 %cmp83.not, label %if.end85, label %error

if.end85:                                         ; preds = %if.end81
  %call86 = call i32 @ps_pglobal_lookup(ptr noundef %ph, ptr noundef null, ptr noundef nonnull @.str.13, ptr noundef nonnull %vaddr) #11
  %cmp87.not = icmp eq i32 %call86, 0
  br i1 %cmp87.not, label %if.end89, label %error

if.end89:                                         ; preds = %if.end85
  %8 = load i64, ptr %vaddr, align 8, !tbaa !20
  %thread_off_tmbx = getelementptr inbounds %struct.td_thragent, ptr %call, i64 0, i32 18
  %call90 = call i32 @ps_pread(ptr noundef %ph, i64 noundef %8, ptr noundef nonnull %thread_off_tmbx, i64 noundef 4) #11
  %cmp91.not = icmp eq i32 %call90, 0
  br i1 %cmp91.not, label %if.end93, label %error

if.end93:                                         ; preds = %if.end89
  %call94 = call i32 @ps_pglobal_lookup(ptr noundef %ph, ptr noundef null, ptr noundef nonnull @.str.14, ptr noundef nonnull %vaddr) #11
  %cmp95.not = icmp eq i32 %call94, 0
  br i1 %cmp95.not, label %if.end97, label %error

if.end97:                                         ; preds = %if.end93
  %9 = load i64, ptr %vaddr, align 8, !tbaa !20
  %thread_off_thr_locklevel = getelementptr inbounds %struct.td_thragent, ptr %call, i64 0, i32 19
  %call98 = call i32 @ps_pread(ptr noundef %ph, i64 noundef %9, ptr noundef nonnull %thread_off_thr_locklevel, i64 noundef 4) #11
  %cmp99.not = icmp eq i32 %call98, 0
  br i1 %cmp99.not, label %if.end101, label %error

if.end101:                                        ; preds = %if.end97
  %call102 = call i32 @ps_pglobal_lookup(ptr noundef %ph, ptr noundef null, ptr noundef nonnull @.str.15, ptr noundef nonnull %vaddr) #11
  %cmp103.not = icmp eq i32 %call102, 0
  br i1 %cmp103.not, label %if.end105, label %error

if.end105:                                        ; preds = %if.end101
  %10 = load i64, ptr %vaddr, align 8, !tbaa !20
  %thread_off_next = getelementptr inbounds %struct.td_thragent, ptr %call, i64 0, i32 20
  %call106 = call i32 @ps_pread(ptr noundef %ph, i64 noundef %10, ptr noundef nonnull %thread_off_next, i64 noundef 4) #11
  %cmp107.not = icmp eq i32 %call106, 0
  br i1 %cmp107.not, label %if.end109, label %error

if.end109:                                        ; preds = %if.end105
  %call110 = call i32 @ps_pglobal_lookup(ptr noundef %ph, ptr noundef null, ptr noundef nonnull @.str.16, ptr noundef nonnull %vaddr) #11
  %cmp111.not = icmp eq i32 %call110, 0
  br i1 %cmp111.not, label %if.end113, label %error

if.end113:                                        ; preds = %if.end109
  %11 = load i64, ptr %vaddr, align 8, !tbaa !20
  %thread_off_state = getelementptr inbounds %struct.td_thragent, ptr %call, i64 0, i32 21
  %call114 = call i32 @ps_pread(ptr noundef %ph, i64 noundef %11, ptr noundef nonnull %thread_off_state, i64 noundef 4) #11
  %cmp115.not = icmp eq i32 %call114, 0
  br i1 %cmp115.not, label %if.end117, label %error

if.end117:                                        ; preds = %if.end113
  %call118 = call i32 @ps_pglobal_lookup(ptr noundef %ph, ptr noundef null, ptr noundef nonnull @.str.17, ptr noundef nonnull %vaddr) #11
  %cmp119.not = icmp eq i32 %call118, 0
  br i1 %cmp119.not, label %if.end121, label %error

if.end121:                                        ; preds = %if.end117
  %12 = load i64, ptr %vaddr, align 8, !tbaa !20
  %thread_max_keys = getelementptr inbounds %struct.td_thragent, ptr %call, i64 0, i32 22
  %call122 = call i32 @ps_pread(ptr noundef %ph, i64 noundef %12, ptr noundef nonnull %thread_max_keys, i64 noundef 4) #11
  %cmp123.not = icmp eq i32 %call122, 0
  br i1 %cmp123.not, label %if.end125, label %error

if.end125:                                        ; preds = %if.end121
  %call126 = call i32 @ps_pglobal_lookup(ptr noundef %ph, ptr noundef null, ptr noundef nonnull @.str.18, ptr noundef nonnull %vaddr) #11
  %cmp127.not = icmp eq i32 %call126, 0
  br i1 %cmp127.not, label %if.end129, label %error

if.end129:                                        ; preds = %if.end125
  %13 = load i64, ptr %vaddr, align 8, !tbaa !20
  %thread_off_key_allocated = getelementptr inbounds %struct.td_thragent, ptr %call, i64 0, i32 23
  %call130 = call i32 @ps_pread(ptr noundef %ph, i64 noundef %13, ptr noundef nonnull %thread_off_key_allocated, i64 noundef 4) #11
  %cmp131.not = icmp eq i32 %call130, 0
  br i1 %cmp131.not, label %if.end133, label %error

if.end133:                                        ; preds = %if.end129
  %call134 = call i32 @ps_pglobal_lookup(ptr noundef %ph, ptr noundef null, ptr noundef nonnull @.str.19, ptr noundef nonnull %vaddr) #11
  %cmp135.not = icmp eq i32 %call134, 0
  br i1 %cmp135.not, label %if.end137, label %error

if.end137:                                        ; preds = %if.end133
  %14 = load i64, ptr %vaddr, align 8, !tbaa !20
  %thread_off_key_destructor = getelementptr inbounds %struct.td_thragent, ptr %call, i64 0, i32 24
  %call138 = call i32 @ps_pread(ptr noundef %ph, i64 noundef %14, ptr noundef nonnull %thread_off_key_destructor, i64 noundef 4) #11
  %cmp139.not = icmp eq i32 %call138, 0
  br i1 %cmp139.not, label %if.end141, label %error

if.end141:                                        ; preds = %if.end137
  %call142 = call i32 @ps_pglobal_lookup(ptr noundef %ph, ptr noundef null, ptr noundef nonnull @.str.20, ptr noundef nonnull %vaddr) #11
  %cmp143.not = icmp eq i32 %call142, 0
  br i1 %cmp143.not, label %if.end145, label %error

if.end145:                                        ; preds = %if.end141
  %15 = load i64, ptr %vaddr, align 8, !tbaa !20
  %thread_state_running = getelementptr inbounds %struct.td_thragent, ptr %call, i64 0, i32 26
  %call146 = call i32 @ps_pread(ptr noundef %ph, i64 noundef %15, ptr noundef nonnull %thread_state_running, i64 noundef 4) #11
  %cmp147.not = icmp eq i32 %call146, 0
  br i1 %cmp147.not, label %if.end149, label %error

if.end149:                                        ; preds = %if.end145
  %call150 = call i32 @ps_pglobal_lookup(ptr noundef %ph, ptr noundef null, ptr noundef nonnull @.str.21, ptr noundef nonnull %vaddr) #11
  %cmp151.not = icmp eq i32 %call150, 0
  br i1 %cmp151.not, label %if.end153, label %error

if.end153:                                        ; preds = %if.end149
  %16 = load i64, ptr %vaddr, align 8, !tbaa !20
  %thread_state_zoombie = getelementptr inbounds %struct.td_thragent, ptr %call, i64 0, i32 25
  %call154 = call i32 @ps_pread(ptr noundef %ph, i64 noundef %16, ptr noundef nonnull %thread_state_zoombie, i64 noundef 4) #11
  %cmp155.not = icmp eq i32 %call154, 0
  br i1 %cmp155.not, label %if.end157, label %error

if.end157:                                        ; preds = %if.end153
  %call158 = call i32 @ps_pglobal_lookup(ptr noundef %ph, ptr noundef null, ptr noundef nonnull @.str.22, ptr noundef nonnull %vaddr) #11
  %cmp159.not = icmp eq i32 %call158, 0
  br i1 %cmp159.not, label %if.end161, label %error

if.end161:                                        ; preds = %if.end157
  %17 = load i64, ptr %vaddr, align 8, !tbaa !20
  %thread_off_sigmask = getelementptr inbounds %struct.td_thragent, ptr %call, i64 0, i32 27
  %call162 = call i32 @ps_pread(ptr noundef %ph, i64 noundef %17, ptr noundef nonnull %thread_off_sigmask, i64 noundef 4) #11
  %cmp163.not = icmp eq i32 %call162, 0
  br i1 %cmp163.not, label %if.end165, label %error

if.end165:                                        ; preds = %if.end161
  %call166 = call i32 @ps_pglobal_lookup(ptr noundef %ph, ptr noundef null, ptr noundef nonnull @.str.23, ptr noundef nonnull %vaddr) #11
  %cmp167.not = icmp eq i32 %call166, 0
  br i1 %cmp167.not, label %if.end169, label %error

if.end169:                                        ; preds = %if.end165
  %18 = load i64, ptr %vaddr, align 8, !tbaa !20
  %thread_off_sigpend = getelementptr inbounds %struct.td_thragent, ptr %call, i64 0, i32 28
  %call170 = call i32 @ps_pread(ptr noundef %ph, i64 noundef %18, ptr noundef nonnull %thread_off_sigpend, i64 noundef 4) #11
  %cmp171.not = icmp eq i32 %call170, 0
  br i1 %cmp171.not, label %if.end173, label %error

if.end173:                                        ; preds = %if.end169
  %call174 = call i32 @getpid() #11
  store i32 %call174, ptr %dbg, align 4, !tbaa !5
  %19 = load i64, ptr %libkse_debug_addr, align 8, !tbaa !14
  %call176 = call i32 @ps_pwrite(ptr noundef %ph, i64 noundef %19, ptr noundef nonnull %dbg, i64 noundef 4) #11
  store ptr %call, ptr %pta, align 8, !tbaa !35
  br label %cleanup

error:                                            ; preds = %if.end169, %if.end165, %if.end161, %if.end157, %if.end153, %if.end149, %if.end145, %if.end141, %if.end137, %if.end133, %if.end129, %if.end125, %if.end121, %if.end117, %if.end113, %if.end109, %if.end105, %if.end101, %if.end97, %if.end93, %if.end89, %if.end85, %if.end81, %if.end77, %if.end73, %if.end69, %if.end65, %if.end61, %if.end57, %if.end53, %if.end49, %if.end45, %if.end41, %if.end37, %if.end33, %if.end29, %if.end25, %if.end21, %if.end17, %if.end13, %if.end9, %if.end5, %if.end
  call void @free(ptr noundef nonnull %call)
  br label %cleanup

cleanup:                                          ; preds = %entry, %error, %if.end173
  %retval.0 = phi i32 [ 12, %error ], [ 0, %if.end173 ], [ 7, %entry ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %dbg) #11
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %vaddr) #11
  ret i32 %retval.0
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define internal noundef i32 @ref_pt_ta_set_event(ptr nocapture readnone %ta, ptr nocapture readnone %events) #1 {
entry:
  ret i32 0
}

; Function Attrs: nounwind uwtable
define internal i32 @ref_pt_ta_thr_iter(ptr noundef %ta, ptr nocapture noundef readonly %callback, ptr noundef %cbdata_p, i32 %state, i32 %ti_pri, ptr nocapture readnone %ti_sigmask_p, i32 %ti_user_flags) #0 {
entry:
  %th = alloca %struct.td_thrhandle_t, align 8
  %pt = alloca i64, align 8
  %activated = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %th) #11
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %pt) #11
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %activated) #11
  %ph = getelementptr inbounds %struct.td_thragent, ptr %ta, i64 0, i32 2
  %0 = load ptr, ptr %ph, align 8, !tbaa !9
  %thread_activated_addr = getelementptr inbounds %struct.td_thragent, ptr %ta, i64 0, i32 6
  %1 = load i64, ptr %thread_activated_addr, align 8, !tbaa !36
  %call = call i32 @ps_pread(ptr noundef %0, i64 noundef %1, ptr noundef nonnull %activated, i64 noundef 4) #11
  %cmp.not = icmp eq i32 %call, 0
  br i1 %cmp.not, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %call1 = call i32 (i32, ...) @ps2td(i32 noundef %call) #11
  br label %cleanup

if.end:                                           ; preds = %entry
  %2 = load i32, ptr %activated, align 4, !tbaa !5
  %tobool.not = icmp eq i32 %2, 0
  br i1 %tobool.not, label %cleanup, label %if.end3

if.end3:                                          ; preds = %if.end
  %thread_list_addr = getelementptr inbounds %struct.td_thragent, ptr %ta, i64 0, i32 4
  %3 = load i64, ptr %thread_list_addr, align 8, !tbaa !19
  %call4 = call i32 @thr_pread_ptr(ptr noundef nonnull %ta, i64 noundef %3, ptr noundef nonnull %pt) #11
  %cmp5.not = icmp eq i32 %call4, 0
  br i1 %cmp5.not, label %while.cond.preheader, label %cleanup

while.cond.preheader:                             ; preds = %if.end3
  %th_tid = getelementptr inbounds %struct.td_thrhandle_t, ptr %th, i64 0, i32 2
  %th_thread = getelementptr inbounds %struct.td_thrhandle_t, ptr %th, i64 0, i32 1
  %thread_off_next = getelementptr inbounds %struct.td_thragent, ptr %ta, i64 0, i32 20
  br label %while.cond

while.cond:                                       ; preds = %while.cond.preheader, %if.end17
  %4 = load i64, ptr %pt, align 8, !tbaa !20
  %cmp8.not = icmp eq i64 %4, 0
  br i1 %cmp8.not, label %cleanup, label %while.body

while.body:                                       ; preds = %while.cond
  store ptr %ta, ptr %th, align 8, !tbaa !28
  %call9 = call fastcc i64 @ref_pt_map_thread(ptr noundef nonnull %ta, i64 noundef %4)
  store i64 %call9, ptr %th_tid, align 8, !tbaa !30
  %5 = load i64, ptr %pt, align 8, !tbaa !20
  store i64 %5, ptr %th_thread, align 8, !tbaa !31
  %cmp11 = icmp eq i64 %call9, -1
  br i1 %cmp11, label %cleanup, label %if.end13

if.end13:                                         ; preds = %while.body
  %call14 = call i32 %callback(ptr noundef nonnull %th, ptr noundef %cbdata_p) #11
  %tobool15.not = icmp eq i32 %call14, 0
  br i1 %tobool15.not, label %if.end17, label %cleanup

if.end17:                                         ; preds = %if.end13
  %6 = load i64, ptr %pt, align 8, !tbaa !20
  %7 = load i32, ptr %thread_off_next, align 4, !tbaa !24
  %conv = sext i32 %7 to i64
  %add = add i64 %6, %conv
  %call18 = call i32 @thr_pread_ptr(ptr noundef nonnull %ta, i64 noundef %add, ptr noundef nonnull %pt) #11
  %cmp19.not = icmp eq i32 %call18, 0
  br i1 %cmp19.not, label %while.cond, label %cleanup, !llvm.loop !37

cleanup:                                          ; preds = %while.cond, %if.end17, %if.end13, %while.body, %if.end3, %if.end, %if.then
  %retval.0 = phi i32 [ %call1, %if.then ], [ 0, %if.end ], [ -1, %if.end3 ], [ 0, %while.cond ], [ -1, %if.end17 ], [ 6, %if.end13 ], [ 7, %while.body ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %activated) #11
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %pt) #11
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %th) #11
  ret i32 %retval.0
}

; Function Attrs: nounwind uwtable
define internal i32 @ref_pt_ta_tsd_iter(ptr nocapture noundef readonly %ta, ptr nocapture noundef readonly %ki, ptr noundef %arg) #0 {
entry:
  %thread_max_keys = getelementptr inbounds %struct.td_thragent, ptr %ta, i64 0, i32 22
  %0 = load i32, ptr %thread_max_keys, align 4, !tbaa !38
  %thread_size_key = getelementptr inbounds %struct.td_thragent, ptr %ta, i64 0, i32 15
  %1 = load i32, ptr %thread_size_key, align 8, !tbaa !39
  %mul = mul nsw i32 %1, %0
  %conv = sext i32 %mul to i64
  %call = tail call noalias ptr @malloc(i64 noundef %conv) #12
  %cmp = icmp eq ptr %call, null
  br i1 %cmp, label %cleanup, label %if.end

if.end:                                           ; preds = %entry
  %ph = getelementptr inbounds %struct.td_thragent, ptr %ta, i64 0, i32 2
  %2 = load ptr, ptr %ph, align 8, !tbaa !9
  %thread_keytable_addr = getelementptr inbounds %struct.td_thragent, ptr %ta, i64 0, i32 8
  %3 = load i64, ptr %thread_keytable_addr, align 8, !tbaa !40
  %call6 = tail call i32 @ps_pread(ptr noundef %2, i64 noundef %3, ptr noundef nonnull %call, i64 noundef %conv) #11
  %cmp7.not = icmp eq i32 %call6, 0
  br i1 %cmp7.not, label %for.cond.preheader, label %if.then9

for.cond.preheader:                               ; preds = %if.end
  %4 = load i32, ptr %thread_max_keys, align 4, !tbaa !38
  %cmp1359 = icmp sgt i32 %4, 0
  br i1 %cmp1359, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %for.cond.preheader
  %5 = ptrtoint ptr %call to i64
  %thread_off_key_allocated = getelementptr inbounds %struct.td_thragent, ptr %ta, i64 0, i32 23
  %thread_off_key_destructor = getelementptr inbounds %struct.td_thragent, ptr %ta, i64 0, i32 24
  br label %for.body

if.then9:                                         ; preds = %if.end
  tail call void @free(ptr noundef nonnull %call)
  %call10 = tail call i32 (i32, ...) @ps2td(i32 noundef %call6) #11
  br label %cleanup

for.body:                                         ; preds = %for.body.lr.ph, %for.inc
  %6 = phi i32 [ %4, %for.body.lr.ph ], [ %14, %for.inc ]
  %i.060 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.inc ]
  %7 = load i32, ptr %thread_size_key, align 8, !tbaa !39
  %mul16 = mul nsw i32 %7, %i.060
  %conv17 = sext i32 %mul16 to i64
  %add = add i64 %conv17, %5
  %8 = load i32, ptr %thread_off_key_allocated, align 8, !tbaa !41
  %conv18 = sext i32 %8 to i64
  %add19 = add i64 %add, %conv18
  %9 = inttoptr i64 %add19 to ptr
  %10 = load i32, ptr %9, align 4, !tbaa !5
  %tobool.not = icmp eq i32 %10, 0
  br i1 %tobool.not, label %for.inc, label %if.then26

if.then26:                                        ; preds = %for.body
  %11 = load i32, ptr %thread_off_key_destructor, align 4, !tbaa !42
  %conv24 = sext i32 %11 to i64
  %add25 = add i64 %add, %conv24
  %12 = inttoptr i64 %add25 to ptr
  %13 = load ptr, ptr %12, align 8, !tbaa !35
  %call27 = tail call i32 %ki(i32 noundef %i.060, ptr noundef %13, ptr noundef %arg) #11
  %cmp28.not = icmp eq i32 %call27, 0
  br i1 %cmp28.not, label %if.then26.for.inc_crit_edge, label %if.then30

if.then26.for.inc_crit_edge:                      ; preds = %if.then26
  %.pre = load i32, ptr %thread_max_keys, align 4, !tbaa !38
  br label %for.inc

if.then30:                                        ; preds = %if.then26
  tail call void @free(ptr noundef %call)
  br label %cleanup

for.inc:                                          ; preds = %if.then26.for.inc_crit_edge, %for.body
  %14 = phi i32 [ %.pre, %if.then26.for.inc_crit_edge ], [ %6, %for.body ]
  %inc = add nuw nsw i32 %i.060, 1
  %cmp13 = icmp slt i32 %inc, %14
  br i1 %cmp13, label %for.body, label %for.end, !llvm.loop !43

for.end:                                          ; preds = %for.inc, %for.cond.preheader
  tail call void @free(ptr noundef %call)
  br label %cleanup

cleanup:                                          ; preds = %entry, %for.end, %if.then30, %if.then9
  %retval.0 = phi i32 [ %call10, %if.then9 ], [ 6, %if.then30 ], [ 0, %for.end ], [ 7, %entry ]
  ret i32 %retval.0
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define internal noundef i32 @ref_pt_thr_clear_event(ptr nocapture readnone %th, ptr nocapture readnone %setp) #1 {
entry:
  ret i32 0
}

; Function Attrs: nounwind uwtable
define internal i32 @ref_pt_thr_dbresume(ptr nocapture noundef readonly %th) #0 {
entry:
  %call = tail call fastcc i32 @ref_pt_dbsuspend(ptr noundef %th, i32 noundef 0)
  ret i32 %call
}

; Function Attrs: nounwind uwtable
define internal i32 @ref_pt_thr_dbsuspend(ptr nocapture noundef readonly %th) #0 {
entry:
  %call = tail call fastcc i32 @ref_pt_dbsuspend(ptr noundef %th, i32 noundef 1)
  ret i32 %call
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define internal noundef i32 @ref_pt_thr_event_enable(ptr nocapture readnone %th, i32 %en) #1 {
entry:
  ret i32 0
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define internal noundef i32 @ref_pt_thr_event_getmsg(ptr nocapture readnone %th, ptr nocapture readnone %msg) #1 {
entry:
  ret i32 14
}

; Function Attrs: nounwind uwtable
define internal i32 @ref_pt_thr_old_get_info(ptr nocapture noundef readonly %th, ptr noundef %info) #0 {
entry:
  %linfo = alloca %struct.ptrace_lwpinfo, align 8
  %tcb_addr = alloca i64, align 8
  %dflags = alloca i32, align 4
  %lwp = alloca i32, align 4
  %state = alloca i32, align 4
  %attrflags = alloca i32, align 4
  %0 = load ptr, ptr %th, align 8, !tbaa !28
  call void @llvm.lifetime.start.p0(i64 160, ptr nonnull %linfo) #11
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %tcb_addr) #11
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %dflags) #11
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %lwp) #11
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %state) #11
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %attrflags) #11
  tail call void @bzero(ptr noundef %info, i64 noundef 112)
  %th_tid.i = getelementptr inbounds %struct.td_thrhandle_t, ptr %th, i64 0, i32 2
  %1 = load i64, ptr %th_tid.i, align 8, !tbaa !30
  %cmp.i = icmp slt i64 %1, 0
  br i1 %cmp.i, label %cleanup, label %lor.lhs.false.i

lor.lhs.false.i:                                  ; preds = %entry
  %2 = load ptr, ptr %th, align 8, !tbaa !28
  %map_len.i = getelementptr inbounds %struct.td_thragent, ptr %2, i64 0, i32 30
  %3 = load i32, ptr %map_len.i, align 8, !tbaa !16
  %conv.i = zext i32 %3 to i64
  %cmp2.not.i = icmp ult i64 %1, %conv.i
  br i1 %cmp2.not.i, label %lor.lhs.false4.i, label %cleanup

lor.lhs.false4.i:                                 ; preds = %lor.lhs.false.i
  %map.i = getelementptr inbounds %struct.td_thragent, ptr %2, i64 0, i32 29
  %4 = load ptr, ptr %map.i, align 8, !tbaa !15
  %arrayidx.i = getelementptr inbounds %struct.pt_map, ptr %4, i64 %1
  %5 = load i32, ptr %arrayidx.i, align 8, !tbaa !17
  %cmp7.i = icmp eq i32 %5, 0
  br i1 %cmp7.i, label %cleanup, label %if.end

if.end:                                           ; preds = %lor.lhs.false4.i
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(112) %info, i8 0, i64 112, i1 false)
  %map = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 29
  %6 = load ptr, ptr %map, align 8, !tbaa !15
  %7 = load i64, ptr %th_tid.i, align 8, !tbaa !30
  %arrayidx = getelementptr inbounds %struct.pt_map, ptr %6, i64 %7
  %8 = load i32, ptr %arrayidx, align 8, !tbaa !17
  %cmp = icmp eq i32 %8, 2
  %9 = getelementptr inbounds %struct.pt_map, ptr %6, i64 %7, i32 1
  br i1 %cmp, label %if.then1, label %if.end7

if.then1:                                         ; preds = %if.end
  %ti_type = getelementptr inbounds %struct.td_old_thrinfo_t, ptr %info, i64 0, i32 4
  store i32 1, ptr %ti_type, align 4, !tbaa !44
  %10 = load i32, ptr %9, align 8, !tbaa !23
  %ti_lid = getelementptr inbounds %struct.td_old_thrinfo_t, ptr %info, i64 0, i32 7
  store i32 %10, ptr %ti_lid, align 8, !tbaa !47
  %ti_tid = getelementptr inbounds %struct.td_old_thrinfo_t, ptr %info, i64 0, i32 1
  store i64 %7, ptr %ti_tid, align 8, !tbaa !48
  %ti_state = getelementptr inbounds %struct.td_old_thrinfo_t, ptr %info, i64 0, i32 3
  store i32 2, ptr %ti_state, align 8, !tbaa !49
  store i32 1, ptr %ti_type, align 4, !tbaa !44
  br label %cleanup

if.end7:                                          ; preds = %if.end
  %ph = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 2
  %11 = load ptr, ptr %ph, align 8, !tbaa !9
  %12 = load i64, ptr %9, align 8, !tbaa !23
  %thread_off_attr_flags = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 14
  %13 = load i32, ptr %thread_off_attr_flags, align 4, !tbaa !50
  %conv = sext i32 %13 to i64
  %add = add i64 %12, %conv
  %call11 = call i32 @ps_pread(ptr noundef %11, i64 noundef %add, ptr noundef nonnull %attrflags, i64 noundef 4) #11
  %cmp12.not = icmp eq i32 %call11, 0
  br i1 %cmp12.not, label %if.end16, label %if.then14

if.then14:                                        ; preds = %if.end7
  %call15 = call i32 (i32, ...) @ps2td(i32 noundef %call11) #11
  br label %cleanup

if.end16:                                         ; preds = %if.end7
  %14 = load ptr, ptr %ph, align 8, !tbaa !9
  %15 = load ptr, ptr %map, align 8, !tbaa !15
  %16 = load i64, ptr %th_tid.i, align 8, !tbaa !30
  %17 = getelementptr inbounds %struct.pt_map, ptr %15, i64 %16, i32 1
  %18 = load i64, ptr %17, align 8, !tbaa !23
  %thread_off_tcb = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 16
  %19 = load i32, ptr %thread_off_tcb, align 4, !tbaa !21
  %conv21 = sext i32 %19 to i64
  %add22 = add i64 %18, %conv21
  %call23 = call i32 @ps_pread(ptr noundef %14, i64 noundef %add22, ptr noundef nonnull %tcb_addr, i64 noundef 8) #11
  %cmp24.not = icmp eq i32 %call23, 0
  br i1 %cmp24.not, label %if.end28, label %if.then26

if.then26:                                        ; preds = %if.end16
  %call27 = call i32 (i32, ...) @ps2td(i32 noundef %call23) #11
  br label %cleanup

if.end28:                                         ; preds = %if.end16
  %20 = load ptr, ptr %ph, align 8, !tbaa !9
  %21 = load ptr, ptr %map, align 8, !tbaa !15
  %22 = load i64, ptr %th_tid.i, align 8, !tbaa !30
  %23 = getelementptr inbounds %struct.pt_map, ptr %21, i64 %22, i32 1
  %24 = load i64, ptr %23, align 8, !tbaa !23
  %thread_off_state = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 21
  %25 = load i32, ptr %thread_off_state, align 8, !tbaa !51
  %conv33 = sext i32 %25 to i64
  %add34 = add i64 %24, %conv33
  %call35 = call i32 @ps_pread(ptr noundef %20, i64 noundef %add34, ptr noundef nonnull %state, i64 noundef 4) #11
  %26 = load ptr, ptr %ph, align 8, !tbaa !9
  %27 = load i64, ptr %tcb_addr, align 8, !tbaa !20
  %thread_off_tmbx = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 18
  %28 = load i32, ptr %thread_off_tmbx, align 4, !tbaa !22
  %conv37 = sext i32 %28 to i64
  %add38 = add i64 %27, 996
  %add39 = add i64 %add38, %conv37
  %ti_lid40 = getelementptr inbounds %struct.td_old_thrinfo_t, ptr %info, i64 0, i32 7
  %call41 = call i32 @ps_pread(ptr noundef %26, i64 noundef %add39, ptr noundef nonnull %ti_lid40, i64 noundef 4) #11
  %cmp42.not = icmp eq i32 %call41, 0
  br i1 %cmp42.not, label %if.end46, label %if.then44

if.then44:                                        ; preds = %if.end28
  %call45 = call i32 (i32, ...) @ps2td(i32 noundef %call41) #11
  br label %cleanup

if.end46:                                         ; preds = %if.end28
  %29 = load ptr, ptr %ph, align 8, !tbaa !9
  %30 = load i64, ptr %tcb_addr, align 8, !tbaa !20
  %31 = load i32, ptr %thread_off_tmbx, align 4, !tbaa !22
  %conv49 = sext i32 %31 to i64
  %add50 = add i64 %30, 992
  %add51 = add i64 %add50, %conv49
  %call52 = call i32 @ps_pread(ptr noundef %29, i64 noundef %add51, ptr noundef nonnull %dflags, i64 noundef 4) #11
  %cmp53.not = icmp eq i32 %call52, 0
  br i1 %cmp53.not, label %if.end57, label %if.then55

if.then55:                                        ; preds = %if.end46
  %call56 = call i32 (i32, ...) @ps2td(i32 noundef %call52) #11
  br label %cleanup

if.end57:                                         ; preds = %if.end46
  %32 = load ptr, ptr %ph, align 8, !tbaa !9
  %33 = load i64, ptr %tcb_addr, align 8, !tbaa !20
  %34 = load i32, ptr %thread_off_tmbx, align 4, !tbaa !22
  %conv60 = sext i32 %34 to i64
  %add61 = add i64 %33, 996
  %add62 = add i64 %add61, %conv60
  %call63 = call i32 @ps_pread(ptr noundef %32, i64 noundef %add62, ptr noundef nonnull %lwp, i64 noundef 4) #11
  %cmp64.not = icmp eq i32 %call63, 0
  br i1 %cmp64.not, label %if.end68, label %if.then66

if.then66:                                        ; preds = %if.end57
  %call67 = call i32 (i32, ...) @ps2td(i32 noundef %call63) #11
  br label %cleanup

if.end68:                                         ; preds = %if.end57
  %35 = load ptr, ptr %th, align 8, !tbaa !28
  store ptr %35, ptr %info, align 8, !tbaa !52
  %36 = load i64, ptr %th_tid.i, align 8, !tbaa !30
  %ti_tid71 = getelementptr inbounds %struct.td_old_thrinfo_t, ptr %info, i64 0, i32 1
  store i64 %36, ptr %ti_tid71, align 8, !tbaa !48
  %37 = load i32, ptr %attrflags, align 4, !tbaa !5
  %and = and i32 %37, 2
  %tobool72.not = icmp eq i32 %and, 0
  %38 = load ptr, ptr %ph, align 8, !tbaa !9
  br i1 %tobool72.not, label %if.else80, label %if.then73

if.then73:                                        ; preds = %if.end68
  %39 = load i32, ptr %lwp, align 4, !tbaa !5
  %call75 = call i32 @ps_linfo(ptr noundef %38, i32 noundef %39, ptr noundef nonnull %linfo) #11
  %cmp76 = icmp eq i32 %call75, 0
  br i1 %cmp76, label %if.then78, label %cleanup

if.then78:                                        ; preds = %if.then73
  %ti_sigmask = getelementptr inbounds %struct.td_old_thrinfo_t, ptr %info, i64 0, i32 10
  %pl_sigmask = getelementptr inbounds %struct.ptrace_lwpinfo, ptr %linfo, i64 0, i32 3
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %ti_sigmask, ptr noundef nonnull align 4 dereferenceable(16) %pl_sigmask, i64 16, i1 false), !tbaa.struct !53
  %ti_pending = getelementptr inbounds %struct.td_old_thrinfo_t, ptr %info, i64 0, i32 11
  %pl_siglist = getelementptr inbounds %struct.ptrace_lwpinfo, ptr %linfo, i64 0, i32 4
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %ti_pending, ptr noundef nonnull align 4 dereferenceable(16) %pl_siglist, i64 16, i1 false), !tbaa.struct !53
  br label %if.end103

if.else80:                                        ; preds = %if.end68
  %40 = load ptr, ptr %map, align 8, !tbaa !15
  %41 = getelementptr inbounds %struct.pt_map, ptr %40, i64 %36, i32 1
  %42 = load i64, ptr %41, align 8, !tbaa !23
  %thread_off_sigmask = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 27
  %43 = load i32, ptr %thread_off_sigmask, align 8, !tbaa !54
  %conv85 = sext i32 %43 to i64
  %add86 = add i64 %42, %conv85
  %ti_sigmask87 = getelementptr inbounds %struct.td_old_thrinfo_t, ptr %info, i64 0, i32 10
  %call88 = call i32 @ps_pread(ptr noundef %38, i64 noundef %add86, ptr noundef nonnull %ti_sigmask87, i64 noundef 16) #11
  %tobool89.not = icmp eq i32 %call88, 0
  br i1 %tobool89.not, label %if.end91, label %cleanup

if.end91:                                         ; preds = %if.else80
  %44 = load ptr, ptr %ph, align 8, !tbaa !9
  %45 = load ptr, ptr %map, align 8, !tbaa !15
  %46 = load i64, ptr %th_tid.i, align 8, !tbaa !30
  %47 = getelementptr inbounds %struct.pt_map, ptr %45, i64 %46, i32 1
  %48 = load i64, ptr %47, align 8, !tbaa !23
  %thread_off_sigpend = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 28
  %49 = load i32, ptr %thread_off_sigpend, align 4, !tbaa !55
  %conv96 = sext i32 %49 to i64
  %add97 = add i64 %48, %conv96
  %ti_pending98 = getelementptr inbounds %struct.td_old_thrinfo_t, ptr %info, i64 0, i32 11
  %call99 = call i32 @ps_pread(ptr noundef %44, i64 noundef %add97, ptr noundef nonnull %ti_pending98, i64 noundef 16) #11
  %tobool100.not = icmp eq i32 %call99, 0
  br i1 %tobool100.not, label %if.end103, label %cleanup

if.end103:                                        ; preds = %if.end91, %if.then78
  %50 = load i32, ptr %state, align 4, !tbaa !5
  %thread_state_running = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 26
  %51 = load i32, ptr %thread_state_running, align 4, !tbaa !56
  %cmp104 = icmp eq i32 %50, %51
  br i1 %cmp104, label %if.then106, label %if.else108

if.then106:                                       ; preds = %if.end103
  %ti_state107 = getelementptr inbounds %struct.td_old_thrinfo_t, ptr %info, i64 0, i32 3
  store i32 2, ptr %ti_state107, align 8, !tbaa !49
  br label %if.end116

if.else108:                                       ; preds = %if.end103
  %thread_state_zoombie = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 25
  %52 = load i32, ptr %thread_state_zoombie, align 8, !tbaa !57
  %cmp109 = icmp eq i32 %50, %52
  %ti_state112 = getelementptr inbounds %struct.td_old_thrinfo_t, ptr %info, i64 0, i32 3
  br i1 %cmp109, label %if.then111, label %if.else113

if.then111:                                       ; preds = %if.else108
  store i32 6, ptr %ti_state112, align 8, !tbaa !49
  br label %if.end116

if.else113:                                       ; preds = %if.else108
  store i32 3, ptr %ti_state112, align 8, !tbaa !49
  br label %if.end116

if.end116:                                        ; preds = %if.then111, %if.else113, %if.then106
  %53 = load i32, ptr %dflags, align 4, !tbaa !5
  %54 = trunc i32 %53 to i8
  %55 = lshr i8 %54, 1
  %conv120 = and i8 %55, 1
  %ti_db_suspended = getelementptr inbounds %struct.td_old_thrinfo_t, ptr %info, i64 0, i32 8
  store i8 %conv120, ptr %ti_db_suspended, align 4, !tbaa !58
  %ti_type121 = getelementptr inbounds %struct.td_old_thrinfo_t, ptr %info, i64 0, i32 4
  store i32 2, ptr %ti_type121, align 4, !tbaa !44
  br label %cleanup

cleanup:                                          ; preds = %lor.lhs.false4.i, %entry, %lor.lhs.false.i, %if.end91, %if.else80, %if.then73, %if.end116, %if.then66, %if.then55, %if.then44, %if.then26, %if.then14, %if.then1
  %retval.0 = phi i32 [ 0, %if.then1 ], [ %call15, %if.then14 ], [ %call27, %if.then26 ], [ %call45, %if.then44 ], [ %call56, %if.then55 ], [ %call67, %if.then66 ], [ 0, %if.end116 ], [ %call75, %if.then73 ], [ %call88, %if.else80 ], [ %call99, %if.end91 ], [ 16, %lor.lhs.false.i ], [ 16, %entry ], [ 16, %lor.lhs.false4.i ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %attrflags) #11
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %state) #11
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %lwp) #11
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %dflags) #11
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %tcb_addr) #11
  call void @llvm.lifetime.end.p0(i64 160, ptr nonnull %linfo) #11
  ret i32 %retval.0
}

; Function Attrs: nounwind uwtable
define internal i32 @ref_pt_thr_get_info(ptr nocapture noundef readonly %th, ptr noundef %info) #0 {
entry:
  %call = tail call i32 @ref_pt_thr_old_get_info(ptr noundef %th, ptr noundef %info)
  %ti_siginfo = getelementptr inbounds %struct.td_thrinfo_t, ptr %info, i64 0, i32 16
  tail call void @bzero(ptr noundef nonnull %ti_siginfo, i64 noundef 80)
  ret i32 %call
}

; Function Attrs: nounwind uwtable
define internal i32 @ref_pt_thr_getfpregs(ptr nocapture noundef readonly %th, ptr noundef %fpregs) #0 {
entry:
  %tmbx = alloca %struct.kse_thr_mailbox, align 16
  %tcb_addr = alloca i64, align 8
  %lwp = alloca i32, align 4
  %0 = load ptr, ptr %th, align 8, !tbaa !28
  call void @llvm.lifetime.start.p0(i64 1024, ptr nonnull %tmbx) #11
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %tcb_addr) #11
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %lwp) #11
  %th_tid.i = getelementptr inbounds %struct.td_thrhandle_t, ptr %th, i64 0, i32 2
  %1 = load i64, ptr %th_tid.i, align 8, !tbaa !30
  %cmp.i = icmp slt i64 %1, 0
  br i1 %cmp.i, label %cleanup, label %lor.lhs.false.i

lor.lhs.false.i:                                  ; preds = %entry
  %map_len.i = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 30
  %2 = load i32, ptr %map_len.i, align 8, !tbaa !16
  %conv.i = zext i32 %2 to i64
  %cmp2.not.i = icmp ult i64 %1, %conv.i
  br i1 %cmp2.not.i, label %lor.lhs.false4.i, label %cleanup

lor.lhs.false4.i:                                 ; preds = %lor.lhs.false.i
  %map.i = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 29
  %3 = load ptr, ptr %map.i, align 8, !tbaa !15
  %arrayidx.i = getelementptr inbounds %struct.pt_map, ptr %3, i64 %1
  %4 = load i32, ptr %arrayidx.i, align 8, !tbaa !17
  switch i32 %4, label %if.end7 [
    i32 0, label %cleanup
    i32 2, label %if.then1
  ]

if.then1:                                         ; preds = %lor.lhs.false4.i
  %ph = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 2
  %5 = load ptr, ptr %ph, align 8, !tbaa !9
  %6 = getelementptr inbounds %struct.pt_map, ptr %3, i64 %1, i32 1
  %7 = load i32, ptr %6, align 8, !tbaa !23
  %call5 = tail call i32 @ps_lgetfpregs(ptr noundef %5, i32 noundef %7, ptr noundef %fpregs) #11
  %call6 = tail call i32 (i32, ...) @ps2td(i32 noundef %call5) #11
  br label %cleanup

if.end7:                                          ; preds = %lor.lhs.false4.i
  %ph8 = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 2
  %8 = load ptr, ptr %ph8, align 8, !tbaa !9
  %9 = getelementptr inbounds %struct.pt_map, ptr %3, i64 %1, i32 1
  %10 = load i64, ptr %9, align 8, !tbaa !23
  %thread_off_tcb = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 16
  %11 = load i32, ptr %thread_off_tcb, align 4, !tbaa !21
  %conv = sext i32 %11 to i64
  %add = add i64 %10, %conv
  %call12 = call i32 @ps_pread(ptr noundef %8, i64 noundef %add, ptr noundef nonnull %tcb_addr, i64 noundef 8) #11
  %cmp13.not = icmp eq i32 %call12, 0
  br i1 %cmp13.not, label %if.end17, label %if.then15

if.then15:                                        ; preds = %if.end7
  %call16 = call i32 (i32, ...) @ps2td(i32 noundef %call12) #11
  br label %cleanup

if.end17:                                         ; preds = %if.end7
  %12 = load i64, ptr %tcb_addr, align 8, !tbaa !20
  %thread_off_tmbx = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 18
  %13 = load i32, ptr %thread_off_tmbx, align 4, !tbaa !22
  %conv18 = sext i32 %13 to i64
  %add19 = add i64 %12, %conv18
  %add20 = add i64 %add19, 996
  %14 = load ptr, ptr %ph8, align 8, !tbaa !9
  %call22 = call i32 @ps_pread(ptr noundef %14, i64 noundef %add20, ptr noundef nonnull %lwp, i64 noundef 4) #11
  %cmp23.not = icmp eq i32 %call22, 0
  br i1 %cmp23.not, label %if.end27, label %if.then25

if.then25:                                        ; preds = %if.end17
  %call26 = call i32 (i32, ...) @ps2td(i32 noundef %call22) #11
  br label %cleanup

if.end27:                                         ; preds = %if.end17
  %15 = load i32, ptr %lwp, align 4, !tbaa !5
  %cmp28.not = icmp eq i32 %15, 0
  %16 = load ptr, ptr %ph8, align 8, !tbaa !9
  br i1 %cmp28.not, label %if.end34, label %if.then30

if.then30:                                        ; preds = %if.end27
  %call32 = call i32 @ps_lgetfpregs(ptr noundef %16, i32 noundef %15, ptr noundef %fpregs) #11
  %call33 = call i32 (i32, ...) @ps2td(i32 noundef %call32) #11
  br label %cleanup

if.end34:                                         ; preds = %if.end27
  %call36 = call i32 @ps_pread(ptr noundef %16, i64 noundef %add19, ptr noundef nonnull %tmbx, i64 noundef 1024) #11
  %cmp37.not = icmp eq i32 %call36, 0
  br i1 %cmp37.not, label %if.end41, label %if.then39

if.then39:                                        ; preds = %if.end34
  %call40 = call i32 (i32, ...) @ps2td(i32 noundef %call36) #11
  br label %cleanup

if.end41:                                         ; preds = %if.end34
  call void @pt_ucontext_to_fpreg(ptr noundef nonnull %tmbx, ptr noundef %fpregs) #11
  br label %cleanup

cleanup:                                          ; preds = %lor.lhs.false4.i, %entry, %lor.lhs.false.i, %if.end41, %if.then39, %if.then30, %if.then25, %if.then15, %if.then1
  %retval.0 = phi i32 [ %call6, %if.then1 ], [ %call16, %if.then15 ], [ %call26, %if.then25 ], [ %call33, %if.then30 ], [ %call40, %if.then39 ], [ 0, %if.end41 ], [ 16, %lor.lhs.false.i ], [ 16, %entry ], [ 16, %lor.lhs.false4.i ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %lwp) #11
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %tcb_addr) #11
  call void @llvm.lifetime.end.p0(i64 1024, ptr nonnull %tmbx) #11
  ret i32 %retval.0
}

; Function Attrs: nounwind uwtable
define internal i32 @ref_pt_thr_getgregs(ptr nocapture noundef readonly %th, ptr noundef %gregs) #0 {
entry:
  %tmbx = alloca %struct.kse_thr_mailbox, align 16
  %tcb_addr = alloca i64, align 8
  %lwp = alloca i32, align 4
  %0 = load ptr, ptr %th, align 8, !tbaa !28
  call void @llvm.lifetime.start.p0(i64 1024, ptr nonnull %tmbx) #11
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %tcb_addr) #11
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %lwp) #11
  %th_tid.i = getelementptr inbounds %struct.td_thrhandle_t, ptr %th, i64 0, i32 2
  %1 = load i64, ptr %th_tid.i, align 8, !tbaa !30
  %cmp.i = icmp slt i64 %1, 0
  br i1 %cmp.i, label %cleanup, label %lor.lhs.false.i

lor.lhs.false.i:                                  ; preds = %entry
  %map_len.i = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 30
  %2 = load i32, ptr %map_len.i, align 8, !tbaa !16
  %conv.i = zext i32 %2 to i64
  %cmp2.not.i = icmp ult i64 %1, %conv.i
  br i1 %cmp2.not.i, label %lor.lhs.false4.i, label %cleanup

lor.lhs.false4.i:                                 ; preds = %lor.lhs.false.i
  %map.i = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 29
  %3 = load ptr, ptr %map.i, align 8, !tbaa !15
  %arrayidx.i = getelementptr inbounds %struct.pt_map, ptr %3, i64 %1
  %4 = load i32, ptr %arrayidx.i, align 8, !tbaa !17
  switch i32 %4, label %if.end7 [
    i32 0, label %cleanup
    i32 2, label %if.then1
  ]

if.then1:                                         ; preds = %lor.lhs.false4.i
  %ph = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 2
  %5 = load ptr, ptr %ph, align 8, !tbaa !9
  %6 = getelementptr inbounds %struct.pt_map, ptr %3, i64 %1, i32 1
  %7 = load i32, ptr %6, align 8, !tbaa !23
  %call5 = tail call i32 @ps_lgetregs(ptr noundef %5, i32 noundef %7, ptr noundef %gregs) #11
  %call6 = tail call i32 (i32, ...) @ps2td(i32 noundef %call5) #11
  br label %cleanup

if.end7:                                          ; preds = %lor.lhs.false4.i
  %ph8 = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 2
  %8 = load ptr, ptr %ph8, align 8, !tbaa !9
  %9 = getelementptr inbounds %struct.pt_map, ptr %3, i64 %1, i32 1
  %10 = load i64, ptr %9, align 8, !tbaa !23
  %thread_off_tcb = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 16
  %11 = load i32, ptr %thread_off_tcb, align 4, !tbaa !21
  %conv = sext i32 %11 to i64
  %add = add i64 %10, %conv
  %call12 = call i32 @ps_pread(ptr noundef %8, i64 noundef %add, ptr noundef nonnull %tcb_addr, i64 noundef 8) #11
  %cmp13.not = icmp eq i32 %call12, 0
  br i1 %cmp13.not, label %if.end17, label %if.then15

if.then15:                                        ; preds = %if.end7
  %call16 = call i32 (i32, ...) @ps2td(i32 noundef %call12) #11
  br label %cleanup

if.end17:                                         ; preds = %if.end7
  %12 = load i64, ptr %tcb_addr, align 8, !tbaa !20
  %thread_off_tmbx = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 18
  %13 = load i32, ptr %thread_off_tmbx, align 4, !tbaa !22
  %conv18 = sext i32 %13 to i64
  %add19 = add i64 %12, %conv18
  %add20 = add i64 %add19, 996
  %14 = load ptr, ptr %ph8, align 8, !tbaa !9
  %call22 = call i32 @ps_pread(ptr noundef %14, i64 noundef %add20, ptr noundef nonnull %lwp, i64 noundef 4) #11
  %cmp23.not = icmp eq i32 %call22, 0
  br i1 %cmp23.not, label %if.end27, label %if.then25

if.then25:                                        ; preds = %if.end17
  %call26 = call i32 (i32, ...) @ps2td(i32 noundef %call22) #11
  br label %cleanup

if.end27:                                         ; preds = %if.end17
  %15 = load i32, ptr %lwp, align 4, !tbaa !5
  %cmp28.not = icmp eq i32 %15, 0
  %16 = load ptr, ptr %ph8, align 8, !tbaa !9
  br i1 %cmp28.not, label %if.end34, label %if.then30

if.then30:                                        ; preds = %if.end27
  %call32 = call i32 @ps_lgetregs(ptr noundef %16, i32 noundef %15, ptr noundef %gregs) #11
  %call33 = call i32 (i32, ...) @ps2td(i32 noundef %call32) #11
  br label %cleanup

if.end34:                                         ; preds = %if.end27
  %call36 = call i32 @ps_pread(ptr noundef %16, i64 noundef %add19, ptr noundef nonnull %tmbx, i64 noundef 1024) #11
  %cmp37.not = icmp eq i32 %call36, 0
  br i1 %cmp37.not, label %if.end41, label %if.then39

if.then39:                                        ; preds = %if.end34
  %call40 = call i32 (i32, ...) @ps2td(i32 noundef %call36) #11
  br label %cleanup

if.end41:                                         ; preds = %if.end34
  call void @pt_ucontext_to_reg(ptr noundef nonnull %tmbx, ptr noundef %gregs) #11
  br label %cleanup

cleanup:                                          ; preds = %lor.lhs.false4.i, %entry, %lor.lhs.false.i, %if.end41, %if.then39, %if.then30, %if.then25, %if.then15, %if.then1
  %retval.0 = phi i32 [ %call6, %if.then1 ], [ %call16, %if.then15 ], [ %call26, %if.then25 ], [ %call33, %if.then30 ], [ %call40, %if.then39 ], [ 0, %if.end41 ], [ 16, %lor.lhs.false.i ], [ 16, %entry ], [ 16, %lor.lhs.false4.i ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %lwp) #11
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %tcb_addr) #11
  call void @llvm.lifetime.end.p0(i64 1024, ptr nonnull %tmbx) #11
  ret i32 %retval.0
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define internal noundef i32 @ref_pt_thr_set_event(ptr nocapture readnone %th, ptr nocapture readnone %setp) #1 {
entry:
  ret i32 0
}

; Function Attrs: nounwind uwtable
define internal i32 @ref_pt_thr_setfpregs(ptr nocapture noundef readonly %th, ptr noundef %fpregs) #0 {
entry:
  %tmbx = alloca %struct.kse_thr_mailbox, align 16
  %tcb_addr = alloca i64, align 8
  %lwp = alloca i32, align 4
  %0 = load ptr, ptr %th, align 8, !tbaa !28
  call void @llvm.lifetime.start.p0(i64 1024, ptr nonnull %tmbx) #11
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %tcb_addr) #11
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %lwp) #11
  %th_tid.i = getelementptr inbounds %struct.td_thrhandle_t, ptr %th, i64 0, i32 2
  %1 = load i64, ptr %th_tid.i, align 8, !tbaa !30
  %cmp.i = icmp slt i64 %1, 0
  br i1 %cmp.i, label %cleanup, label %lor.lhs.false.i

lor.lhs.false.i:                                  ; preds = %entry
  %map_len.i = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 30
  %2 = load i32, ptr %map_len.i, align 8, !tbaa !16
  %conv.i = zext i32 %2 to i64
  %cmp2.not.i = icmp ult i64 %1, %conv.i
  br i1 %cmp2.not.i, label %lor.lhs.false4.i, label %cleanup

lor.lhs.false4.i:                                 ; preds = %lor.lhs.false.i
  %map.i = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 29
  %3 = load ptr, ptr %map.i, align 8, !tbaa !15
  %arrayidx.i = getelementptr inbounds %struct.pt_map, ptr %3, i64 %1
  %4 = load i32, ptr %arrayidx.i, align 8, !tbaa !17
  switch i32 %4, label %if.end7 [
    i32 0, label %cleanup
    i32 2, label %if.then1
  ]

if.then1:                                         ; preds = %lor.lhs.false4.i
  %ph = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 2
  %5 = load ptr, ptr %ph, align 8, !tbaa !9
  %6 = getelementptr inbounds %struct.pt_map, ptr %3, i64 %1, i32 1
  %7 = load i32, ptr %6, align 8, !tbaa !23
  %call5 = tail call i32 @ps_lsetfpregs(ptr noundef %5, i32 noundef %7, ptr noundef %fpregs) #11
  %call6 = tail call i32 (i32, ...) @ps2td(i32 noundef %call5) #11
  br label %cleanup

if.end7:                                          ; preds = %lor.lhs.false4.i
  %ph8 = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 2
  %8 = load ptr, ptr %ph8, align 8, !tbaa !9
  %9 = getelementptr inbounds %struct.pt_map, ptr %3, i64 %1, i32 1
  %10 = load i64, ptr %9, align 8, !tbaa !23
  %thread_off_tcb = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 16
  %11 = load i32, ptr %thread_off_tcb, align 4, !tbaa !21
  %conv = sext i32 %11 to i64
  %add = add i64 %10, %conv
  %call12 = call i32 @ps_pread(ptr noundef %8, i64 noundef %add, ptr noundef nonnull %tcb_addr, i64 noundef 8) #11
  %cmp13.not = icmp eq i32 %call12, 0
  br i1 %cmp13.not, label %if.end17, label %if.then15

if.then15:                                        ; preds = %if.end7
  %call16 = call i32 (i32, ...) @ps2td(i32 noundef %call12) #11
  br label %cleanup

if.end17:                                         ; preds = %if.end7
  %12 = load i64, ptr %tcb_addr, align 8, !tbaa !20
  %thread_off_tmbx = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 18
  %13 = load i32, ptr %thread_off_tmbx, align 4, !tbaa !22
  %conv18 = sext i32 %13 to i64
  %add19 = add i64 %12, %conv18
  %add20 = add i64 %add19, 996
  %14 = load ptr, ptr %ph8, align 8, !tbaa !9
  %call22 = call i32 @ps_pread(ptr noundef %14, i64 noundef %add20, ptr noundef nonnull %lwp, i64 noundef 4) #11
  %cmp23.not = icmp eq i32 %call22, 0
  br i1 %cmp23.not, label %if.end27, label %if.then25

if.then25:                                        ; preds = %if.end17
  %call26 = call i32 (i32, ...) @ps2td(i32 noundef %call22) #11
  br label %cleanup

if.end27:                                         ; preds = %if.end17
  %15 = load i32, ptr %lwp, align 4, !tbaa !5
  %cmp28.not = icmp eq i32 %15, 0
  %16 = load ptr, ptr %ph8, align 8, !tbaa !9
  br i1 %cmp28.not, label %if.end34, label %if.then30

if.then30:                                        ; preds = %if.end27
  %call32 = call i32 @ps_lsetfpregs(ptr noundef %16, i32 noundef %15, ptr noundef %fpregs) #11
  %call33 = call i32 (i32, ...) @ps2td(i32 noundef %call32) #11
  br label %cleanup

if.end34:                                         ; preds = %if.end27
  %call36 = call i32 @ps_pread(ptr noundef %16, i64 noundef %add19, ptr noundef nonnull %tmbx, i64 noundef 1024) #11
  %cmp37.not = icmp eq i32 %call36, 0
  br i1 %cmp37.not, label %if.end41, label %if.then39

if.then39:                                        ; preds = %if.end34
  %call40 = call i32 (i32, ...) @ps2td(i32 noundef %call36) #11
  br label %cleanup

if.end41:                                         ; preds = %if.end34
  call void @pt_fpreg_to_ucontext(ptr noundef %fpregs, ptr noundef nonnull %tmbx) #11
  %17 = load ptr, ptr %ph8, align 8, !tbaa !9
  %call43 = call i32 @ps_pwrite(ptr noundef %17, i64 noundef %add19, ptr noundef nonnull %tmbx, i64 noundef 1024) #11
  %call44 = call i32 (i32, ...) @ps2td(i32 noundef %call43) #11
  br label %cleanup

cleanup:                                          ; preds = %lor.lhs.false4.i, %entry, %lor.lhs.false.i, %if.end41, %if.then39, %if.then30, %if.then25, %if.then15, %if.then1
  %retval.0 = phi i32 [ %call6, %if.then1 ], [ %call16, %if.then15 ], [ %call26, %if.then25 ], [ %call33, %if.then30 ], [ %call40, %if.then39 ], [ %call44, %if.end41 ], [ 16, %lor.lhs.false.i ], [ 16, %entry ], [ 16, %lor.lhs.false4.i ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %lwp) #11
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %tcb_addr) #11
  call void @llvm.lifetime.end.p0(i64 1024, ptr nonnull %tmbx) #11
  ret i32 %retval.0
}

; Function Attrs: nounwind uwtable
define internal i32 @ref_pt_thr_setgregs(ptr nocapture noundef readonly %th, ptr noundef %gregs) #0 {
entry:
  %tmbx = alloca %struct.kse_thr_mailbox, align 16
  %tcb_addr = alloca i64, align 8
  %lwp = alloca i32, align 4
  %0 = load ptr, ptr %th, align 8, !tbaa !28
  call void @llvm.lifetime.start.p0(i64 1024, ptr nonnull %tmbx) #11
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %tcb_addr) #11
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %lwp) #11
  %th_tid.i = getelementptr inbounds %struct.td_thrhandle_t, ptr %th, i64 0, i32 2
  %1 = load i64, ptr %th_tid.i, align 8, !tbaa !30
  %cmp.i = icmp slt i64 %1, 0
  br i1 %cmp.i, label %cleanup, label %lor.lhs.false.i

lor.lhs.false.i:                                  ; preds = %entry
  %map_len.i = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 30
  %2 = load i32, ptr %map_len.i, align 8, !tbaa !16
  %conv.i = zext i32 %2 to i64
  %cmp2.not.i = icmp ult i64 %1, %conv.i
  br i1 %cmp2.not.i, label %lor.lhs.false4.i, label %cleanup

lor.lhs.false4.i:                                 ; preds = %lor.lhs.false.i
  %map.i = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 29
  %3 = load ptr, ptr %map.i, align 8, !tbaa !15
  %arrayidx.i = getelementptr inbounds %struct.pt_map, ptr %3, i64 %1
  %4 = load i32, ptr %arrayidx.i, align 8, !tbaa !17
  switch i32 %4, label %if.end7 [
    i32 0, label %cleanup
    i32 2, label %if.then1
  ]

if.then1:                                         ; preds = %lor.lhs.false4.i
  %ph = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 2
  %5 = load ptr, ptr %ph, align 8, !tbaa !9
  %6 = getelementptr inbounds %struct.pt_map, ptr %3, i64 %1, i32 1
  %7 = load i32, ptr %6, align 8, !tbaa !23
  %call5 = tail call i32 @ps_lsetregs(ptr noundef %5, i32 noundef %7, ptr noundef %gregs) #11
  %call6 = tail call i32 (i32, ...) @ps2td(i32 noundef %call5) #11
  br label %cleanup

if.end7:                                          ; preds = %lor.lhs.false4.i
  %ph8 = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 2
  %8 = load ptr, ptr %ph8, align 8, !tbaa !9
  %9 = getelementptr inbounds %struct.pt_map, ptr %3, i64 %1, i32 1
  %10 = load i64, ptr %9, align 8, !tbaa !23
  %thread_off_tcb = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 16
  %11 = load i32, ptr %thread_off_tcb, align 4, !tbaa !21
  %conv = sext i32 %11 to i64
  %add = add i64 %10, %conv
  %call12 = call i32 @ps_pread(ptr noundef %8, i64 noundef %add, ptr noundef nonnull %tcb_addr, i64 noundef 8) #11
  %cmp13.not = icmp eq i32 %call12, 0
  br i1 %cmp13.not, label %if.end17, label %if.then15

if.then15:                                        ; preds = %if.end7
  %call16 = call i32 (i32, ...) @ps2td(i32 noundef %call12) #11
  br label %cleanup

if.end17:                                         ; preds = %if.end7
  %12 = load i64, ptr %tcb_addr, align 8, !tbaa !20
  %thread_off_tmbx = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 18
  %13 = load i32, ptr %thread_off_tmbx, align 4, !tbaa !22
  %conv18 = sext i32 %13 to i64
  %add19 = add i64 %12, %conv18
  %add20 = add i64 %add19, 996
  %14 = load ptr, ptr %ph8, align 8, !tbaa !9
  %call22 = call i32 @ps_pread(ptr noundef %14, i64 noundef %add20, ptr noundef nonnull %lwp, i64 noundef 4) #11
  %cmp23.not = icmp eq i32 %call22, 0
  br i1 %cmp23.not, label %if.end27, label %if.then25

if.then25:                                        ; preds = %if.end17
  %call26 = call i32 (i32, ...) @ps2td(i32 noundef %call22) #11
  br label %cleanup

if.end27:                                         ; preds = %if.end17
  %15 = load i32, ptr %lwp, align 4, !tbaa !5
  %cmp28.not = icmp eq i32 %15, 0
  %16 = load ptr, ptr %ph8, align 8, !tbaa !9
  br i1 %cmp28.not, label %if.end34, label %if.then30

if.then30:                                        ; preds = %if.end27
  %call32 = call i32 @ps_lsetregs(ptr noundef %16, i32 noundef %15, ptr noundef %gregs) #11
  %call33 = call i32 (i32, ...) @ps2td(i32 noundef %call32) #11
  br label %cleanup

if.end34:                                         ; preds = %if.end27
  %call36 = call i32 @ps_pread(ptr noundef %16, i64 noundef %add19, ptr noundef nonnull %tmbx, i64 noundef 1024) #11
  %cmp37.not = icmp eq i32 %call36, 0
  br i1 %cmp37.not, label %if.end41, label %if.then39

if.then39:                                        ; preds = %if.end34
  %call40 = call i32 (i32, ...) @ps2td(i32 noundef %call36) #11
  br label %cleanup

if.end41:                                         ; preds = %if.end34
  call void @pt_reg_to_ucontext(ptr noundef %gregs, ptr noundef nonnull %tmbx) #11
  %17 = load ptr, ptr %ph8, align 8, !tbaa !9
  %call43 = call i32 @ps_pwrite(ptr noundef %17, i64 noundef %add19, ptr noundef nonnull %tmbx, i64 noundef 1024) #11
  %call44 = call i32 (i32, ...) @ps2td(i32 noundef %call43) #11
  br label %cleanup

cleanup:                                          ; preds = %lor.lhs.false4.i, %entry, %lor.lhs.false.i, %if.end41, %if.then39, %if.then30, %if.then25, %if.then15, %if.then1
  %retval.0 = phi i32 [ %call6, %if.then1 ], [ %call16, %if.then15 ], [ %call26, %if.then25 ], [ %call33, %if.then30 ], [ %call40, %if.then39 ], [ %call44, %if.end41 ], [ 16, %lor.lhs.false.i ], [ 16, %entry ], [ 16, %lor.lhs.false4.i ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %lwp) #11
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %tcb_addr) #11
  call void @llvm.lifetime.end.p0(i64 1024, ptr nonnull %tmbx) #11
  ret i32 %retval.0
}

; Function Attrs: nounwind uwtable
define internal noundef i32 @ref_pt_thr_validate(ptr nocapture noundef readonly %th) #0 {
entry:
  %temp = alloca %struct.td_thrhandle_t, align 8
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %temp) #11
  %0 = load ptr, ptr %th, align 8, !tbaa !28
  %th_tid = getelementptr inbounds %struct.td_thrhandle_t, ptr %th, i64 0, i32 2
  %1 = load i64, ptr %th_tid, align 8, !tbaa !30
  %call = call i32 @ref_pt_ta_map_id2thr(ptr noundef %0, i64 noundef %1, ptr noundef nonnull %temp), !range !59
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %temp) #11
  ret i32 %call
}

; Function Attrs: nounwind uwtable
define internal i32 @ref_pt_thr_tls_get_addr(ptr nocapture noundef readonly %th, i64 noundef %_linkmap, i64 noundef %offset, ptr noundef %address) #0 {
entry:
  %dtv_addr = alloca i64, align 8
  %tcb_addr = alloca i64, align 8
  %tls_index = alloca i32, align 4
  %0 = load ptr, ptr %th, align 8, !tbaa !28
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %dtv_addr) #11
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %tcb_addr) #11
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %tls_index) #11
  %thread_off_linkmap = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 17
  %1 = load i32, ptr %thread_off_linkmap, align 8, !tbaa !60
  %conv = sext i32 %1 to i64
  %sub = sub i64 %_linkmap, %conv
  %ph = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 2
  %2 = load ptr, ptr %ph, align 8, !tbaa !9
  %thread_off_tlsindex = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 13
  %3 = load i32, ptr %thread_off_tlsindex, align 8, !tbaa !61
  %conv1 = sext i32 %3 to i64
  %add = add i64 %sub, %conv1
  %call = call i32 @ps_pread(ptr noundef %2, i64 noundef %add, ptr noundef nonnull %tls_index, i64 noundef 4) #11
  %cmp.not = icmp eq i32 %call, 0
  br i1 %cmp.not, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %call3 = call i32 (i32, ...) @ps2td(i32 noundef %call) #11
  br label %cleanup

if.end:                                           ; preds = %entry
  %4 = load ptr, ptr %ph, align 8, !tbaa !9
  %map = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 29
  %5 = load ptr, ptr %map, align 8, !tbaa !15
  %th_tid = getelementptr inbounds %struct.td_thrhandle_t, ptr %th, i64 0, i32 2
  %6 = load i64, ptr %th_tid, align 8, !tbaa !30
  %7 = getelementptr inbounds %struct.pt_map, ptr %5, i64 %6, i32 1
  %8 = load i64, ptr %7, align 8, !tbaa !23
  %thread_off_tcb = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 16
  %9 = load i32, ptr %thread_off_tcb, align 4, !tbaa !21
  %conv5 = sext i32 %9 to i64
  %add6 = add i64 %8, %conv5
  %call7 = call i32 @ps_pread(ptr noundef %4, i64 noundef %add6, ptr noundef nonnull %tcb_addr, i64 noundef 8) #11
  %cmp8.not = icmp eq i32 %call7, 0
  br i1 %cmp8.not, label %if.end12, label %if.then10

if.then10:                                        ; preds = %if.end
  %call11 = call i32 (i32, ...) @ps2td(i32 noundef %call7) #11
  br label %cleanup

if.end12:                                         ; preds = %if.end
  %10 = load ptr, ptr %ph, align 8, !tbaa !9
  %11 = load i64, ptr %tcb_addr, align 8, !tbaa !20
  %thread_off_dtv = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 10
  %12 = load i32, ptr %thread_off_dtv, align 4, !tbaa !62
  %conv14 = sext i32 %12 to i64
  %add15 = add i64 %11, %conv14
  %call16 = call i32 @ps_pread(ptr noundef %10, i64 noundef %add15, ptr noundef nonnull %dtv_addr, i64 noundef 8) #11
  %cmp17.not = icmp eq i32 %call16, 0
  br i1 %cmp17.not, label %if.end21, label %if.then19

if.then19:                                        ; preds = %if.end12
  %call20 = call i32 (i32, ...) @ps2td(i32 noundef %call16) #11
  br label %cleanup

if.end21:                                         ; preds = %if.end12
  %13 = load ptr, ptr %ph, align 8, !tbaa !9
  %14 = load i64, ptr %dtv_addr, align 8, !tbaa !20
  %15 = load i32, ptr %tls_index, align 4, !tbaa !5
  %add23 = add nsw i32 %15, 1
  %conv24 = sext i32 %add23 to i64
  %mul = shl nsw i64 %conv24, 3
  %add25 = add i64 %mul, %14
  %call26 = call i32 @ps_pread(ptr noundef %13, i64 noundef %add25, ptr noundef %address, i64 noundef 8) #11
  %cmp27.not = icmp eq i32 %call26, 0
  br i1 %cmp27.not, label %if.end31, label %if.then29

if.then29:                                        ; preds = %if.end21
  %call30 = call i32 (i32, ...) @ps2td(i32 noundef %call26) #11
  br label %cleanup

if.end31:                                         ; preds = %if.end21
  %16 = load i64, ptr %address, align 8, !tbaa !20
  %add32 = add i64 %16, %offset
  store i64 %add32, ptr %address, align 8, !tbaa !20
  br label %cleanup

cleanup:                                          ; preds = %if.end31, %if.then29, %if.then19, %if.then10, %if.then
  %retval.0 = phi i32 [ %call3, %if.then ], [ %call11, %if.then10 ], [ %call20, %if.then19 ], [ %call30, %if.then29 ], [ 0, %if.end31 ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %tls_index) #11
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %tcb_addr) #11
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %dtv_addr) #11
  ret i32 %retval.0
}

; Function Attrs: nounwind uwtable
define internal i32 @ref_pt_thr_sstep(ptr nocapture noundef readonly %th, i32 noundef %step) #0 {
entry:
  %tmbx = alloca %struct.kse_thr_mailbox, align 16
  %regs = alloca %struct.reg, align 8
  %tcb_addr = alloca i64, align 8
  %dflags = alloca i32, align 4
  %lwp = alloca i32, align 4
  %0 = load ptr, ptr %th, align 8, !tbaa !28
  call void @llvm.lifetime.start.p0(i64 1024, ptr nonnull %tmbx) #11
  call void @llvm.lifetime.start.p0(i64 176, ptr nonnull %regs) #11
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %tcb_addr) #11
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %dflags) #11
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %lwp) #11
  %th_tid.i = getelementptr inbounds %struct.td_thrhandle_t, ptr %th, i64 0, i32 2
  %1 = load i64, ptr %th_tid.i, align 8, !tbaa !30
  %cmp.i = icmp slt i64 %1, 0
  br i1 %cmp.i, label %cleanup, label %lor.lhs.false.i

lor.lhs.false.i:                                  ; preds = %entry
  %map_len.i = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 30
  %2 = load i32, ptr %map_len.i, align 8, !tbaa !16
  %conv.i = zext i32 %2 to i64
  %cmp2.not.i = icmp ult i64 %1, %conv.i
  br i1 %cmp2.not.i, label %lor.lhs.false4.i, label %cleanup

lor.lhs.false4.i:                                 ; preds = %lor.lhs.false.i
  %map.i = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 29
  %3 = load ptr, ptr %map.i, align 8, !tbaa !15
  %arrayidx.i = getelementptr inbounds %struct.pt_map, ptr %3, i64 %1
  %4 = load i32, ptr %arrayidx.i, align 8, !tbaa !17
  switch i32 %4, label %if.end2 [
    i32 0, label %cleanup
    i32 2, label %cleanup.fold.split
  ]

if.end2:                                          ; preds = %lor.lhs.false4.i
  %ph = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 2
  %5 = load ptr, ptr %ph, align 8, !tbaa !9
  %6 = getelementptr inbounds %struct.pt_map, ptr %3, i64 %1, i32 1
  %7 = load i64, ptr %6, align 8, !tbaa !23
  %thread_off_tcb = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 16
  %8 = load i32, ptr %thread_off_tcb, align 4, !tbaa !21
  %conv = sext i32 %8 to i64
  %add = add i64 %7, %conv
  %call6 = call i32 @ps_pread(ptr noundef %5, i64 noundef %add, ptr noundef nonnull %tcb_addr, i64 noundef 8) #11
  %cmp7.not = icmp eq i32 %call6, 0
  br i1 %cmp7.not, label %if.end11, label %if.then9

if.then9:                                         ; preds = %if.end2
  %call10 = call i32 (i32, ...) @ps2td(i32 noundef %call6) #11
  br label %cleanup

if.end11:                                         ; preds = %if.end2
  %9 = load ptr, ptr %ph, align 8, !tbaa !9
  %10 = load i64, ptr %tcb_addr, align 8, !tbaa !20
  %thread_off_tmbx = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 18
  %11 = load i32, ptr %thread_off_tmbx, align 4, !tbaa !22
  %conv13 = sext i32 %11 to i64
  %add14 = add i64 %10, 992
  %add15 = add i64 %add14, %conv13
  %call16 = call i32 @ps_pread(ptr noundef %9, i64 noundef %add15, ptr noundef nonnull %dflags, i64 noundef 4) #11
  %cmp17.not = icmp eq i32 %call16, 0
  br i1 %cmp17.not, label %if.end21, label %if.then19

if.then19:                                        ; preds = %if.end11
  %call20 = call i32 (i32, ...) @ps2td(i32 noundef %call16) #11
  br label %cleanup

if.end21:                                         ; preds = %if.end11
  %cmp22.not = icmp ne i32 %step, 0
  %12 = load i32, ptr %dflags, align 4
  %and = and i32 %12, -2
  %masksel = zext i1 %cmp22.not to i32
  %storemerge = or disjoint i32 %and, %masksel
  store i32 %storemerge, ptr %dflags, align 4, !tbaa !5
  %13 = load ptr, ptr %ph, align 8, !tbaa !9
  %14 = load i64, ptr %tcb_addr, align 8, !tbaa !20
  %15 = load i32, ptr %thread_off_tmbx, align 4, !tbaa !22
  %conv28 = sext i32 %15 to i64
  %add29 = add i64 %14, 992
  %add30 = add i64 %add29, %conv28
  %call31 = call i32 @ps_pwrite(ptr noundef %13, i64 noundef %add30, ptr noundef nonnull %dflags, i64 noundef 4) #11
  %cmp32.not = icmp eq i32 %call31, 0
  br i1 %cmp32.not, label %if.end36, label %if.then34

if.then34:                                        ; preds = %if.end21
  %call35 = call i32 (i32, ...) @ps2td(i32 noundef %call31) #11
  br label %cleanup

if.end36:                                         ; preds = %if.end21
  %16 = load ptr, ptr %ph, align 8, !tbaa !9
  %17 = load i64, ptr %tcb_addr, align 8, !tbaa !20
  %18 = load i32, ptr %thread_off_tmbx, align 4, !tbaa !22
  %conv39 = sext i32 %18 to i64
  %add40 = add i64 %17, 996
  %add41 = add i64 %add40, %conv39
  %call42 = call i32 @ps_pread(ptr noundef %16, i64 noundef %add41, ptr noundef nonnull %lwp, i64 noundef 4) #11
  %cmp43.not = icmp eq i32 %call42, 0
  br i1 %cmp43.not, label %if.end47, label %if.then45

if.then45:                                        ; preds = %if.end36
  %call46 = call i32 (i32, ...) @ps2td(i32 noundef %call42) #11
  br label %cleanup

if.end47:                                         ; preds = %if.end36
  %19 = load i32, ptr %lwp, align 4, !tbaa !5
  %cmp48.not = icmp eq i32 %19, 0
  br i1 %cmp48.not, label %if.end51, label %cleanup

if.end51:                                         ; preds = %if.end47
  %20 = load i64, ptr %tcb_addr, align 8, !tbaa !20
  %21 = load i32, ptr %thread_off_tmbx, align 4, !tbaa !22
  %conv53 = sext i32 %21 to i64
  %add54 = add i64 %20, %conv53
  %22 = load ptr, ptr %ph, align 8, !tbaa !9
  %call56 = call i32 @ps_pread(ptr noundef %22, i64 noundef %add54, ptr noundef nonnull %tmbx, i64 noundef 1024) #11
  %cmp57 = icmp eq i32 %call56, 0
  br i1 %cmp57, label %if.then59, label %if.end68

if.then59:                                        ; preds = %if.end51
  call void @pt_ucontext_to_reg(ptr noundef nonnull %tmbx, ptr noundef nonnull %regs) #11
  %call60 = call i32 @pt_reg_sstep(ptr noundef nonnull %regs, i32 noundef %step) #11
  %cmp61.not = icmp eq i32 %call60, 0
  br i1 %cmp61.not, label %if.end68, label %if.then63

if.then63:                                        ; preds = %if.then59
  call void @pt_reg_to_ucontext(ptr noundef nonnull %regs, ptr noundef nonnull %tmbx) #11
  %23 = load ptr, ptr %ph, align 8, !tbaa !9
  %call66 = call i32 @ps_pwrite(ptr noundef %23, i64 noundef %add54, ptr noundef nonnull %tmbx, i64 noundef 1024) #11
  br label %if.end68

if.end68:                                         ; preds = %if.then59, %if.then63, %if.end51
  %ret.0 = phi i32 [ %call66, %if.then63 ], [ 0, %if.then59 ], [ %call56, %if.end51 ]
  %call69 = call i32 (i32, ...) @ps2td(i32 noundef %ret.0) #11
  br label %cleanup

cleanup.fold.split:                               ; preds = %lor.lhs.false4.i
  br label %cleanup

cleanup:                                          ; preds = %lor.lhs.false4.i, %cleanup.fold.split, %entry, %lor.lhs.false.i, %if.end47, %if.end68, %if.then45, %if.then34, %if.then19, %if.then9
  %retval.0 = phi i32 [ %call10, %if.then9 ], [ %call20, %if.then19 ], [ %call35, %if.then34 ], [ %call46, %if.then45 ], [ %call69, %if.end68 ], [ 0, %if.end47 ], [ 16, %lor.lhs.false.i ], [ 16, %entry ], [ 16, %lor.lhs.false4.i ], [ 5, %cleanup.fold.split ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %lwp) #11
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %dflags) #11
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %tcb_addr) #11
  call void @llvm.lifetime.end.p0(i64 176, ptr nonnull %regs) #11
  call void @llvm.lifetime.end.p0(i64 1024, ptr nonnull %tmbx) #11
  ret i32 %retval.0
}

declare void @pt_md_init() local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #3

declare i32 @ps_pwrite(ptr noundef, i64 noundef, ptr noundef, i64 noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite)
declare void @free(ptr allocptr nocapture noundef) local_unnamed_addr #4

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #3

declare i32 @thr_pread_ptr(ptr noundef, i64 noundef, ptr noundef) local_unnamed_addr #2

declare i32 @thr_pread_int(ptr noundef, i64 noundef, ptr noundef) local_unnamed_addr #2

declare i32 @ps_lgetregs(ptr noundef, i32 noundef, ptr noundef) local_unnamed_addr #2

; Function Attrs: nounwind uwtable
define internal fastcc i64 @ref_pt_map_thread(ptr nocapture noundef %const_ta, i64 noundef %pt) unnamed_addr #0 {
entry:
  %map_len = getelementptr inbounds %struct.td_thragent, ptr %const_ta, i64 0, i32 30
  %0 = load i32, ptr %map_len, align 8, !tbaa !16
  %cmp94 = icmp ugt i32 %0, 1
  br i1 %cmp94, label %for.body.lr.ph, label %if.then19

for.body.lr.ph:                                   ; preds = %entry
  %map = getelementptr inbounds %struct.td_thragent, ptr %const_ta, i64 0, i32 29
  %1 = load ptr, ptr %map, align 8, !tbaa !15
  %wide.trip.count = zext i32 %0 to i64
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc
  %indvars.iv = phi i64 [ 1, %for.body.lr.ph ], [ %indvars.iv.next, %for.inc ]
  %first.095 = phi i32 [ -1, %for.body.lr.ph ], [ %first.1, %for.inc ]
  %arrayidx = getelementptr inbounds %struct.pt_map, ptr %1, i64 %indvars.iv
  %2 = load i32, ptr %arrayidx, align 8, !tbaa !17
  switch i32 %2, label %for.inc [
    i32 0, label %if.then
    i32 1, label %land.lhs.true
  ]

if.then:                                          ; preds = %for.body
  %cmp3 = icmp eq i32 %first.095, -1
  %3 = trunc i64 %indvars.iv to i32
  %spec.select = select i1 %cmp3, i32 %3, i32 %first.095
  br label %for.inc

land.lhs.true:                                    ; preds = %for.body
  %4 = getelementptr inbounds %struct.pt_map, ptr %1, i64 %indvars.iv, i32 1
  %5 = load i64, ptr %4, align 8, !tbaa !23
  %cmp13 = icmp eq i64 %5, %pt
  br i1 %cmp13, label %cleanup, label %for.inc

for.inc:                                          ; preds = %if.then, %for.body, %land.lhs.true
  %first.1 = phi i32 [ %first.095, %land.lhs.true ], [ %spec.select, %if.then ], [ %first.095, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.end, label %for.body, !llvm.loop !63

for.end:                                          ; preds = %for.inc
  %cmp17 = icmp eq i32 %first.1, -1
  br i1 %cmp17, label %if.then19, label %for.end.if.end48_crit_edge

for.end.if.end48_crit_edge:                       ; preds = %for.end
  %map49.phi.trans.insert = getelementptr inbounds %struct.td_thragent, ptr %const_ta, i64 0, i32 29
  %.pre = load ptr, ptr %map49.phi.trans.insert, align 8, !tbaa !15
  br label %if.end48

if.then19:                                        ; preds = %entry, %for.end
  %cmp21 = icmp eq i32 %0, 0
  br i1 %cmp21, label %if.then23, label %if.else31

if.then23:                                        ; preds = %if.then19
  %call = tail call noalias dereferenceable_or_null(320) ptr @calloc(i64 noundef 20, i64 noundef 16) #13
  %map24 = getelementptr inbounds %struct.td_thragent, ptr %const_ta, i64 0, i32 29
  store ptr %call, ptr %map24, align 8, !tbaa !15
  %cmp26 = icmp eq ptr %call, null
  br i1 %cmp26, label %cleanup, label %if.end29

if.end29:                                         ; preds = %if.then23
  store i32 20, ptr %map_len, align 8, !tbaa !16
  br label %if.end48

if.else31:                                        ; preds = %if.then19
  %map32 = getelementptr inbounds %struct.td_thragent, ptr %const_ta, i64 0, i32 29
  %6 = load ptr, ptr %map32, align 8, !tbaa !15
  %conv34 = zext i32 %0 to i64
  %call35 = tail call ptr @reallocarray(ptr noundef %6, i64 noundef %conv34, i64 noundef 32) #14
  %cmp36 = icmp eq ptr %call35, null
  br i1 %cmp36, label %cleanup, label %if.end39

if.end39:                                         ; preds = %if.else31
  %7 = load i32, ptr %map_len, align 8, !tbaa !16
  %idx.ext = zext i32 %7 to i64
  %add.ptr = getelementptr inbounds %struct.pt_map, ptr %call35, i64 %idx.ext
  %mul = shl nuw nsw i64 %idx.ext, 4
  tail call void @llvm.memset.p0.i64(ptr nonnull align 8 %add.ptr, i8 0, i64 %mul, i1 false)
  %8 = load i32, ptr %map_len, align 8, !tbaa !16
  store ptr %call35, ptr %map32, align 8, !tbaa !15
  %mul46 = shl i32 %8, 1
  store i32 %mul46, ptr %map_len, align 8, !tbaa !16
  br label %if.end48

if.end48:                                         ; preds = %for.end.if.end48_crit_edge, %if.end29, %if.end39
  %9 = phi ptr [ %call, %if.end29 ], [ %call35, %if.end39 ], [ %.pre, %for.end.if.end48_crit_edge ]
  %first.2 = phi i32 [ 1, %if.end29 ], [ %8, %if.end39 ], [ %first.1, %for.end.if.end48_crit_edge ]
  %idxprom50 = sext i32 %first.2 to i64
  %arrayidx51 = getelementptr inbounds %struct.pt_map, ptr %9, i64 %idxprom50
  store i32 1, ptr %arrayidx51, align 8, !tbaa !17
  %10 = getelementptr inbounds %struct.pt_map, ptr %9, i64 %idxprom50, i32 1
  store i64 %pt, ptr %10, align 8, !tbaa !23
  br label %cleanup

cleanup:                                          ; preds = %land.lhs.true, %if.else31, %if.then23, %if.end48
  %retval.0 = phi i64 [ %idxprom50, %if.end48 ], [ -1, %if.then23 ], [ -1, %if.else31 ], [ %indvars.iv, %land.lhs.true ]
  ret i64 %retval.0
}

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,zeroed") allocsize(0,1) memory(inaccessiblemem: readwrite)
declare noalias noundef ptr @calloc(i64 noundef, i64 noundef) local_unnamed_addr #5

; Function Attrs: allocsize(1,2)
declare ptr @reallocarray(ptr noundef, i64 noundef, i64 noundef) local_unnamed_addr #6

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #7

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #8

declare i32 @ps_pglobal_lookup(ptr noundef, ptr noundef, ptr noundef, ptr noundef) local_unnamed_addr #2

declare i32 @ps_pread(ptr noundef, i64 noundef, ptr noundef, i64 noundef) local_unnamed_addr #2

declare i32 @getpid() local_unnamed_addr #2

declare i32 @ps2td(...) local_unnamed_addr #2

; Function Attrs: nounwind uwtable
define internal fastcc i32 @ref_pt_dbsuspend(ptr nocapture noundef readonly %th, i32 noundef %suspend) unnamed_addr #0 {
entry:
  %tcb_addr = alloca i64, align 8
  %ptr = alloca i64, align 8
  %lwp = alloca i32, align 4
  %dflags = alloca i32, align 4
  %attrflags = alloca i32, align 4
  %locklevel = alloca i32, align 4
  %pl = alloca %struct.ptrace_lwpinfo, align 8
  %0 = load ptr, ptr %th, align 8, !tbaa !28
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %tcb_addr) #11
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %ptr) #11
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %lwp) #11
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %dflags) #11
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %attrflags) #11
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %locklevel) #11
  %th_tid.i = getelementptr inbounds %struct.td_thrhandle_t, ptr %th, i64 0, i32 2
  %1 = load i64, ptr %th_tid.i, align 8, !tbaa !30
  %cmp.i = icmp slt i64 %1, 0
  br i1 %cmp.i, label %cleanup153, label %lor.lhs.false.i

lor.lhs.false.i:                                  ; preds = %entry
  %map_len.i = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 30
  %2 = load i32, ptr %map_len.i, align 8, !tbaa !16
  %conv.i = zext i32 %2 to i64
  %cmp2.not.i = icmp ult i64 %1, %conv.i
  br i1 %cmp2.not.i, label %lor.lhs.false4.i, label %cleanup153

lor.lhs.false4.i:                                 ; preds = %lor.lhs.false.i
  %map.i = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 29
  %3 = load ptr, ptr %map.i, align 8, !tbaa !15
  %arrayidx.i = getelementptr inbounds %struct.pt_map, ptr %3, i64 %1
  %4 = load i32, ptr %arrayidx.i, align 8, !tbaa !17
  switch i32 %4, label %if.end15 [
    i32 0, label %cleanup153
    i32 2, label %if.then1
  ]

if.then1:                                         ; preds = %lor.lhs.false4.i
  %tobool2.not = icmp eq i32 %suspend, 0
  %ph8 = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 2
  %5 = load ptr, ptr %ph8, align 8, !tbaa !9
  %6 = getelementptr inbounds %struct.pt_map, ptr %3, i64 %1, i32 1
  %7 = load i32, ptr %6, align 8, !tbaa !23
  br i1 %tobool2.not, label %if.else, label %if.then3

if.then3:                                         ; preds = %if.then1
  %call7 = tail call i32 @ps_lstop(ptr noundef %5, i32 noundef %7) #11
  br label %if.end13

if.else:                                          ; preds = %if.then1
  %call12 = tail call i32 @ps_lcontinue(ptr noundef %5, i32 noundef %7) #11
  br label %if.end13

if.end13:                                         ; preds = %if.else, %if.then3
  %ret.0 = phi i32 [ %call7, %if.then3 ], [ %call12, %if.else ]
  %call14 = tail call i32 (i32, ...) @ps2td(i32 noundef %ret.0) #11
  br label %cleanup153

if.end15:                                         ; preds = %lor.lhs.false4.i
  %ph16 = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 2
  %8 = load ptr, ptr %ph16, align 8, !tbaa !9
  %9 = getelementptr inbounds %struct.pt_map, ptr %3, i64 %1, i32 1
  %10 = load i64, ptr %9, align 8, !tbaa !23
  %thread_off_attr_flags = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 14
  %11 = load i32, ptr %thread_off_attr_flags, align 4, !tbaa !50
  %conv = sext i32 %11 to i64
  %add = add i64 %10, %conv
  %call20 = call i32 @ps_pread(ptr noundef %8, i64 noundef %add, ptr noundef nonnull %attrflags, i64 noundef 4) #11
  %cmp21.not = icmp eq i32 %call20, 0
  br i1 %cmp21.not, label %if.end25, label %if.then23

if.then23:                                        ; preds = %if.end15
  %call24 = call i32 (i32, ...) @ps2td(i32 noundef %call20) #11
  br label %cleanup153

if.end25:                                         ; preds = %if.end15
  %12 = load ptr, ptr %ph16, align 8, !tbaa !9
  %13 = load ptr, ptr %map.i, align 8, !tbaa !15
  %14 = load i64, ptr %th_tid.i, align 8, !tbaa !30
  %15 = getelementptr inbounds %struct.pt_map, ptr %13, i64 %14, i32 1
  %16 = load i64, ptr %15, align 8, !tbaa !23
  %thread_off_tcb = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 16
  %17 = load i32, ptr %thread_off_tcb, align 4, !tbaa !21
  %conv30 = sext i32 %17 to i64
  %add31 = add i64 %16, %conv30
  %call32 = call i32 @ps_pread(ptr noundef %12, i64 noundef %add31, ptr noundef nonnull %tcb_addr, i64 noundef 8) #11
  %cmp33.not = icmp eq i32 %call32, 0
  br i1 %cmp33.not, label %if.end37, label %if.then35

if.then35:                                        ; preds = %if.end25
  %call36 = call i32 (i32, ...) @ps2td(i32 noundef %call32) #11
  br label %cleanup153

if.end37:                                         ; preds = %if.end25
  %18 = load i64, ptr %tcb_addr, align 8, !tbaa !20
  %thread_off_tmbx = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 18
  %19 = load i32, ptr %thread_off_tmbx, align 4, !tbaa !22
  %conv38 = sext i32 %19 to i64
  %add39 = add i64 %18, %conv38
  %add40 = add i64 %add39, 996
  store i64 %add40, ptr %ptr, align 8, !tbaa !20
  %20 = load ptr, ptr %ph16, align 8, !tbaa !9
  %call42 = call i32 @ps_pread(ptr noundef %20, i64 noundef %add40, ptr noundef nonnull %lwp, i64 noundef 4) #11
  %cmp43.not = icmp eq i32 %call42, 0
  br i1 %cmp43.not, label %if.end47, label %if.then45

if.then45:                                        ; preds = %if.end37
  %call46 = call i32 (i32, ...) @ps2td(i32 noundef %call42) #11
  br label %cleanup153

if.end47:                                         ; preds = %if.end37
  %21 = load i32, ptr %lwp, align 4, !tbaa !5
  %cmp48.not = icmp eq i32 %21, 0
  br i1 %cmp48.not, label %if.end135, label %if.then50

if.then50:                                        ; preds = %if.end47
  %22 = load i32, ptr %attrflags, align 4, !tbaa !5
  %and = and i32 %22, 512
  %tobool51.not = icmp eq i32 %and, 0
  br i1 %tobool51.not, label %if.end53, label %cleanup153

if.end53:                                         ; preds = %if.then50
  %and54 = and i32 %22, 2
  %tobool55.not = icmp eq i32 %and54, 0
  br i1 %tobool55.not, label %if.else111, label %if.then56

if.then56:                                        ; preds = %if.end53
  %23 = load ptr, ptr %map.i, align 8, !tbaa !15
  %24 = load i64, ptr %th_tid.i, align 8, !tbaa !30
  %25 = getelementptr inbounds %struct.pt_map, ptr %23, i64 %24, i32 1
  %26 = load i64, ptr %25, align 8, !tbaa !23
  %thread_off_kse = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 12
  %27 = load i32, ptr %thread_off_kse, align 4, !tbaa !64
  %conv60 = sext i32 %27 to i64
  %add61 = add i64 %26, %conv60
  store i64 %add61, ptr %ptr, align 8, !tbaa !20
  %28 = load ptr, ptr %ph16, align 8, !tbaa !9
  %call63 = call i32 @ps_pread(ptr noundef %28, i64 noundef %add61, ptr noundef nonnull %ptr, i64 noundef 8) #11
  %cmp64.not = icmp eq i32 %call63, 0
  br i1 %cmp64.not, label %if.end68, label %if.then66

if.then66:                                        ; preds = %if.then56
  %call67 = call i32 (i32, ...) @ps2td(i32 noundef %call63) #11
  br label %cleanup153

if.end68:                                         ; preds = %if.then56
  %29 = load ptr, ptr %ph16, align 8, !tbaa !9
  %30 = load i64, ptr %ptr, align 8, !tbaa !20
  %thread_off_kse_locklevel = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 11
  %31 = load i32, ptr %thread_off_kse_locklevel, align 8, !tbaa !65
  %conv70 = sext i32 %31 to i64
  %add71 = add i64 %30, %conv70
  %call72 = call i32 @ps_pread(ptr noundef %29, i64 noundef %add71, ptr noundef nonnull %locklevel, i64 noundef 4) #11
  %cmp73.not = icmp eq i32 %call72, 0
  br i1 %cmp73.not, label %if.end77, label %if.then75

if.then75:                                        ; preds = %if.end68
  %call76 = call i32 (i32, ...) @ps2td(i32 noundef %call72) #11
  br label %cleanup153

if.end77:                                         ; preds = %if.end68
  %32 = load i32, ptr %locklevel, align 4, !tbaa !5
  %cmp78 = icmp slt i32 %32, 1
  br i1 %cmp78, label %if.then80, label %if.end93

if.then80:                                        ; preds = %if.end77
  %33 = load ptr, ptr %map.i, align 8, !tbaa !15
  %34 = load i64, ptr %th_tid.i, align 8, !tbaa !30
  %35 = getelementptr inbounds %struct.pt_map, ptr %33, i64 %34, i32 1
  %36 = load i64, ptr %35, align 8, !tbaa !23
  %thread_off_thr_locklevel = getelementptr inbounds %struct.td_thragent, ptr %0, i64 0, i32 19
  %37 = load i32, ptr %thread_off_thr_locklevel, align 8, !tbaa !66
  %conv84 = sext i32 %37 to i64
  %add85 = add i64 %36, %conv84
  store i64 %add85, ptr %ptr, align 8, !tbaa !20
  %38 = load ptr, ptr %ph16, align 8, !tbaa !9
  %call87 = call i32 @ps_pread(ptr noundef %38, i64 noundef %add85, ptr noundef nonnull %locklevel, i64 noundef 4) #11
  %cmp88.not = icmp eq i32 %call87, 0
  br i1 %cmp88.not, label %if.end93, label %if.then90

if.then90:                                        ; preds = %if.then80
  %call91 = call i32 (i32, ...) @ps2td(i32 noundef %call87) #11
  br label %cleanup153

if.end93:                                         ; preds = %if.then80, %if.end77
  %tobool94.not = icmp eq i32 %suspend, 0
  br i1 %tobool94.not, label %if.else102, label %if.then95

if.then95:                                        ; preds = %if.end93
  %39 = load i32, ptr %locklevel, align 4, !tbaa !5
  %cmp96 = icmp slt i32 %39, 1
  br i1 %cmp96, label %if.then98, label %if.end135

if.then98:                                        ; preds = %if.then95
  %40 = load ptr, ptr %ph16, align 8, !tbaa !9
  %41 = load i32, ptr %lwp, align 4, !tbaa !5
  %call100 = call i32 @ps_lstop(ptr noundef %40, i32 noundef %41) #11
  br label %if.end105

if.else102:                                       ; preds = %if.end93
  %42 = load ptr, ptr %ph16, align 8, !tbaa !9
  %43 = load i32, ptr %lwp, align 4, !tbaa !5
  %call104 = call i32 @ps_lcontinue(ptr noundef %42, i32 noundef %43) #11
  br label %if.end105

if.end105:                                        ; preds = %if.then98, %if.else102
  %ret.2 = phi i32 [ %call100, %if.then98 ], [ %call104, %if.else102 ]
  %cmp106.not = icmp eq i32 %ret.2, 0
  br i1 %cmp106.not, label %if.end135, label %if.then108

if.then108:                                       ; preds = %if.end105
  %call109 = call i32 (i32, ...) @ps2td(i32 noundef %ret.2) #11
  br label %cleanup153

if.else111:                                       ; preds = %if.end53
  call void @llvm.lifetime.start.p0(i64 160, ptr nonnull %pl) #11
  %44 = load ptr, ptr %ph16, align 8, !tbaa !9
  %call113 = call i32 @ps_linfo(ptr noundef %44, i32 noundef %21, ptr noundef nonnull %pl) #11
  %tobool114.not = icmp eq i32 %call113, 0
  br i1 %tobool114.not, label %if.end116, label %cleanup

if.end116:                                        ; preds = %if.else111
  %tobool117.not = icmp eq i32 %suspend, 0
  br i1 %tobool117.not, label %if.else125, label %if.then118

if.then118:                                       ; preds = %if.end116
  %pl_flags = getelementptr inbounds %struct.ptrace_lwpinfo, ptr %pl, i64 0, i32 2
  %45 = load i32, ptr %pl_flags, align 8, !tbaa !67
  %and119 = and i32 %45, 2
  %tobool120.not = icmp eq i32 %and119, 0
  br i1 %tobool120.not, label %if.then121, label %cleanup.thread

if.then121:                                       ; preds = %if.then118
  %46 = load ptr, ptr %ph16, align 8, !tbaa !9
  %47 = load i32, ptr %lwp, align 4, !tbaa !5
  %call123 = call i32 @ps_lstop(ptr noundef %46, i32 noundef %47) #11
  br label %if.end128

if.else125:                                       ; preds = %if.end116
  %48 = load ptr, ptr %ph16, align 8, !tbaa !9
  %49 = load i32, ptr %lwp, align 4, !tbaa !5
  %call127 = call i32 @ps_lcontinue(ptr noundef %48, i32 noundef %49) #11
  br label %if.end128

if.end128:                                        ; preds = %if.then121, %if.else125
  %ret.3 = phi i32 [ %call123, %if.then121 ], [ %call127, %if.else125 ]
  %cmp129.not = icmp eq i32 %ret.3, 0
  br i1 %cmp129.not, label %cleanup.thread, label %if.then131

if.then131:                                       ; preds = %if.end128
  %call132 = call i32 (i32, ...) @ps2td(i32 noundef %ret.3) #11
  br label %cleanup

cleanup.thread:                                   ; preds = %if.end128, %if.then118
  call void @llvm.lifetime.end.p0(i64 160, ptr nonnull %pl) #11
  br label %if.end135

cleanup:                                          ; preds = %if.else111, %if.then131
  %retval.0 = phi i32 [ %call132, %if.then131 ], [ -1, %if.else111 ]
  call void @llvm.lifetime.end.p0(i64 160, ptr nonnull %pl) #11
  br label %cleanup153

if.end135:                                        ; preds = %if.then95, %cleanup.thread, %if.end105, %if.end47
  %50 = load ptr, ptr %ph16, align 8, !tbaa !9
  %add137 = add i64 %add39, 992
  %call138 = call i32 @ps_pread(ptr noundef %50, i64 noundef %add137, ptr noundef nonnull %dflags, i64 noundef 4) #11
  %cmp139.not = icmp eq i32 %call138, 0
  br i1 %cmp139.not, label %if.end143, label %if.then141

if.then141:                                       ; preds = %if.end135
  %call142 = call i32 (i32, ...) @ps2td(i32 noundef %call138) #11
  br label %cleanup153

if.end143:                                        ; preds = %if.end135
  %tobool144.not = icmp eq i32 %suspend, 0
  %51 = load i32, ptr %dflags, align 4
  %and147 = and i32 %51, -3
  %masksel = select i1 %tobool144.not, i32 0, i32 2
  %storemerge = or disjoint i32 %and147, %masksel
  store i32 %storemerge, ptr %dflags, align 4, !tbaa !5
  %52 = load ptr, ptr %ph16, align 8, !tbaa !9
  %call151 = call i32 @ps_pwrite(ptr noundef %52, i64 noundef %add137, ptr noundef nonnull %dflags, i64 noundef 4) #11
  %call152 = call i32 (i32, ...) @ps2td(i32 noundef %call151) #11
  br label %cleanup153

cleanup153:                                       ; preds = %lor.lhs.false4.i, %entry, %lor.lhs.false.i, %cleanup, %if.then50, %if.end143, %if.then141, %if.then108, %if.then90, %if.then75, %if.then66, %if.then45, %if.then35, %if.then23, %if.end13
  %retval.1 = phi i32 [ %call14, %if.end13 ], [ %call24, %if.then23 ], [ %call36, %if.then35 ], [ %call46, %if.then45 ], [ %call67, %if.then66 ], [ %call76, %if.then75 ], [ %call91, %if.then90 ], [ %call109, %if.then108 ], [ %call142, %if.then141 ], [ %call152, %if.end143 ], [ %retval.0, %cleanup ], [ 0, %if.then50 ], [ 16, %lor.lhs.false.i ], [ 16, %entry ], [ 16, %lor.lhs.false4.i ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %locklevel) #11
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %attrflags) #11
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %dflags) #11
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %lwp) #11
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %ptr) #11
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %tcb_addr) #11
  ret i32 %retval.1
}

declare i32 @ps_lstop(ptr noundef, i32 noundef) local_unnamed_addr #2

declare i32 @ps_lcontinue(ptr noundef, i32 noundef) local_unnamed_addr #2

declare i32 @ps_linfo(ptr noundef, i32 noundef, ptr noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nofree nounwind willreturn memory(argmem: readwrite)
declare void @bzero(ptr nocapture noundef writeonly, i64 noundef) local_unnamed_addr #9

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #10

declare i32 @ps_lgetfpregs(ptr noundef, i32 noundef, ptr noundef) local_unnamed_addr #2

declare void @pt_ucontext_to_fpreg(ptr noundef, ptr noundef) local_unnamed_addr #2

declare void @pt_ucontext_to_reg(ptr noundef, ptr noundef) local_unnamed_addr #2

declare i32 @ps_lsetfpregs(ptr noundef, i32 noundef, ptr noundef) local_unnamed_addr #2

declare void @pt_fpreg_to_ucontext(ptr noundef, ptr noundef) local_unnamed_addr #2

declare i32 @ps_lsetregs(ptr noundef, i32 noundef, ptr noundef) local_unnamed_addr #2

declare void @pt_reg_to_ucontext(ptr noundef, ptr noundef) local_unnamed_addr #2

declare i32 @pt_reg_sstep(ptr noundef, i32 noundef) local_unnamed_addr #2

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #4 = { mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { mustprogress nofree nounwind willreturn allockind("alloc,zeroed") allocsize(0,1) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { allocsize(1,2) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #7 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #8 = { mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #9 = { mustprogress nofree nounwind willreturn memory(argmem: readwrite) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #10 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #11 = { nounwind }
attributes #12 = { allocsize(0) }
attributes #13 = { allocsize(0,1) }
attributes #14 = { nounwind allocsize(1,2) }

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
!9 = !{!10, !11, i64 24}
!10 = !{!"td_thragent", !11, i64 0, !12, i64 8, !11, i64 24, !13, i64 32, !13, i64 40, !13, i64 48, !13, i64 56, !13, i64 64, !13, i64 72, !6, i64 80, !6, i64 84, !6, i64 88, !6, i64 92, !6, i64 96, !6, i64 100, !6, i64 104, !6, i64 108, !6, i64 112, !6, i64 116, !6, i64 120, !6, i64 124, !6, i64 128, !6, i64 132, !6, i64 136, !6, i64 140, !6, i64 144, !6, i64 148, !6, i64 152, !6, i64 156, !11, i64 160, !6, i64 168}
!11 = !{!"any pointer", !7, i64 0}
!12 = !{!"", !11, i64 0, !11, i64 8}
!13 = !{!"long", !7, i64 0}
!14 = !{!10, !13, i64 32}
!15 = !{!10, !11, i64 160}
!16 = !{!10, !6, i64 168}
!17 = !{!18, !6, i64 0}
!18 = !{!"pt_map", !6, i64 0, !7, i64 8}
!19 = !{!10, !13, i64 40}
!20 = !{!13, !13, i64 0}
!21 = !{!10, !6, i64 108}
!22 = !{!10, !6, i64 116}
!23 = !{!7, !7, i64 0}
!24 = !{!10, !6, i64 124}
!25 = distinct !{!25, !26}
!26 = !{!"llvm.loop.mustprogress"}
!27 = distinct !{!27, !26}
!28 = !{!29, !11, i64 0}
!29 = !{!"", !11, i64 0, !13, i64 8, !13, i64 16}
!30 = !{!29, !13, i64 16}
!31 = !{!29, !13, i64 8}
!32 = distinct !{!32, !26}
!33 = distinct !{!33, !26}
!34 = !{!10, !6, i64 80}
!35 = !{!11, !11, i64 0}
!36 = !{!10, !13, i64 56}
!37 = distinct !{!37, !26}
!38 = !{!10, !6, i64 132}
!39 = !{!10, !6, i64 104}
!40 = !{!10, !13, i64 72}
!41 = !{!10, !6, i64 136}
!42 = !{!10, !6, i64 140}
!43 = distinct !{!43, !26}
!44 = !{!45, !6, i64 28}
!45 = !{!"", !11, i64 0, !13, i64 8, !13, i64 16, !6, i64 24, !6, i64 28, !6, i64 32, !6, i64 36, !6, i64 40, !7, i64 44, !7, i64 45, !46, i64 48, !46, i64 64, !13, i64 80, !13, i64 88, !13, i64 96, !13, i64 104}
!46 = !{!"__sigset", !7, i64 0}
!47 = !{!45, !6, i64 40}
!48 = !{!45, !13, i64 8}
!49 = !{!45, !6, i64 24}
!50 = !{!10, !6, i64 100}
!51 = !{!10, !6, i64 128}
!52 = !{!45, !11, i64 0}
!53 = !{i64 0, i64 16, !23}
!54 = !{!10, !6, i64 152}
!55 = !{!10, !6, i64 156}
!56 = !{!10, !6, i64 148}
!57 = !{!10, !6, i64 144}
!58 = !{!45, !7, i64 44}
!59 = !{i32 -1, i32 17}
!60 = !{!10, !6, i64 112}
!61 = !{!10, !6, i64 96}
!62 = !{!10, !6, i64 84}
!63 = distinct !{!63, !26}
!64 = !{!10, !6, i64 92}
!65 = !{!10, !6, i64 88}
!66 = !{!10, !6, i64 120}
!67 = !{!68, !6, i64 8}
!68 = !{!"ptrace_lwpinfo", !6, i64 0, !6, i64 4, !6, i64 8, !46, i64 12, !46, i64 28, !69, i64 48, !7, i64 128, !6, i64 148, !6, i64 152, !6, i64 156}
!69 = !{!"__siginfo", !6, i64 0, !6, i64 4, !6, i64 8, !6, i64 12, !6, i64 16, !6, i64 20, !11, i64 24, !7, i64 32, !7, i64 40}
