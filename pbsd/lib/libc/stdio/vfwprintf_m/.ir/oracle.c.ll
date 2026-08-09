; ModuleID = '/home/odin/pbsd/pbsd/lib/libc/stdio/vfwprintf_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/libc/stdio/vfwprintf_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

module asm ".symver __qsort_r_compat, qsort_r@FBSD_1.0"

%struct._xlocale = type { %struct.xlocale_refcounted, [6 x ptr], i32, i32, i32, i32, i32, i32, %struct.lconv, ptr }
%struct.xlocale_refcounted = type { i64, ptr }
%struct.lconv = type { ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8 }
%struct._pthread_cleanup_info = type { [8 x i64] }
%struct.__sFILE = type { ptr, i32, i32, i16, i16, %struct.__sbuf, i32, ptr, ptr, ptr, ptr, ptr, %struct.__sbuf, ptr, i32, [3 x i8], [1 x i8], %struct.__sbuf, i32, i64, ptr, ptr, i32, i32, %union.__mbstate_t, i32 }
%struct.__sbuf = type { ptr, i32 }
%union.__mbstate_t = type { i64, [120 x i8] }
%struct.grouping_state = type { i32, ptr, i32, i32, i32 }
%union.anon = type { x86_fp80 }
%struct.io_state = type { ptr, %struct.__suio, [8 x %struct.__siov] }
%struct.__suio = type { ptr, i32, i32 }
%struct.__siov = type { ptr, i64 }
%union.arg = type { x86_fp80 }
%struct.__va_list_tag = type { i32, i32, ptr, ptr }

@__isthreaded = external local_unnamed_addr global i32, align 4
@ref___vfwprintf.xdigs_lower = internal constant [16 x i8] c"0123456789abcdef", align 16
@ref___vfwprintf.xdigs_upper = internal constant [16 x i8] c"0123456789ABCDEF", align 16
@.str = private unnamed_addr constant [4 x i32] [i32 110, i32 97, i32 110, i32 0], align 4
@.str.1 = private unnamed_addr constant [4 x i32] [i32 78, i32 65, i32 78, i32 0], align 4
@.str.2 = private unnamed_addr constant [4 x i32] [i32 105, i32 110, i32 102, i32 0], align 4
@.str.3 = private unnamed_addr constant [4 x i32] [i32 73, i32 78, i32 70, i32 0], align 4
@.str.4 = private unnamed_addr constant [7 x i32] [i32 40, i32 110, i32 117, i32 108, i32 108, i32 41, i32 0], align 4
@blanks = internal constant [16 x i32] [i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32], align 16
@zeroes = internal constant [16 x i32] [i32 48, i32 48, i32 48, i32 48, i32 48, i32 48, i32 48, i32 48, i32 48, i32 48, i32 48, i32 48, i32 48, i32 48, i32 48, i32 48], align 16
@__xlocale_C_locale = external global %struct._xlocale, align 8
@__xlocale_global_locale = external global %struct._xlocale, align 8
@__has_thread_locale = external local_unnamed_addr global i32, align 4
@__thread_locale = external thread_local global ptr, align 8

; Function Attrs: nounwind uwtable
define dso_local i32 @ref_vfwprintf_l(ptr noalias noundef %fp, ptr noundef %locale, ptr noalias noundef %fmt0, ptr noundef %ap) local_unnamed_addr #0 {
entry:
  %__cleanup_info__ = alloca %struct._pthread_cleanup_info, align 8
  %0 = ptrtoint ptr %locale to i64
  switch i64 %0, label %sw.default.i [
    i64 0, label %get_real_locale.exit
    i64 -1, label %sw.bb1.i
  ]

sw.bb1.i:                                         ; preds = %entry
  br label %get_real_locale.exit

sw.default.i:                                     ; preds = %entry
  br label %get_real_locale.exit

get_real_locale.exit:                             ; preds = %entry, %sw.bb1.i, %sw.default.i
  %retval.0.i = phi ptr [ %locale, %sw.default.i ], [ @__xlocale_global_locale, %sw.bb1.i ], [ @__xlocale_C_locale, %entry ]
  call void @llvm.lifetime.start.p0(i64 64, ptr nonnull %__cleanup_info__) #14
  %1 = load i32, ptr @__isthreaded, align 4, !tbaa !5
  %tobool.not = icmp eq i32 %1, 0
  br i1 %tobool.not, label %if.end, label %if.then

if.then:                                          ; preds = %get_real_locale.exit
  tail call void @_flockfile(ptr noundef %fp) #14
  br label %if.end

if.end:                                           ; preds = %get_real_locale.exit, %if.then
  %.sink = phi ptr [ %fp, %if.then ], [ null, %get_real_locale.exit ]
  call void @___pthread_cleanup_push_imp(ptr noundef nonnull @__stdio_cancel_cleanup, ptr noundef %.sink, ptr noundef nonnull %__cleanup_info__) #14
  %_flags = getelementptr inbounds %struct.__sFILE, ptr %fp, i64 0, i32 3
  %2 = load i16, ptr %_flags, align 8, !tbaa !9
  %3 = and i16 %2, 26
  %cmp = icmp eq i16 %3, 10
  br i1 %cmp, label %land.lhs.true, label %if.else7

land.lhs.true:                                    ; preds = %if.end
  %_file = getelementptr inbounds %struct.__sFILE, ptr %fp, i64 0, i32 4
  %4 = load i16, ptr %_file, align 2, !tbaa !15
  %cmp3 = icmp sgt i16 %4, -1
  br i1 %cmp3, label %if.then5, label %if.else7

if.then5:                                         ; preds = %land.lhs.true
  %call6 = call fastcc i32 @ref___sbprintf(ptr noundef nonnull %fp, ptr noundef %retval.0.i, ptr noundef %fmt0, ptr noundef %ap)
  br label %if.end9

if.else7:                                         ; preds = %land.lhs.true, %if.end
  %call8 = call i32 @ref___vfwprintf(ptr noundef nonnull %fp, ptr noundef %retval.0.i, ptr noundef %fmt0, ptr noundef %ap)
  br label %if.end9

if.end9:                                          ; preds = %if.else7, %if.then5
  %ret.0 = phi i32 [ %call6, %if.then5 ], [ %call8, %if.else7 ]
  call void @___pthread_cleanup_pop_imp(i32 noundef 1) #14
  call void @llvm.lifetime.end.p0(i64 64, ptr nonnull %__cleanup_info__) #14
  ret i32 %ret.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

declare void @_flockfile(ptr noundef) local_unnamed_addr #2

declare void @___pthread_cleanup_push_imp(ptr noundef, ptr noundef, ptr noundef) local_unnamed_addr #2

declare void @__stdio_cancel_cleanup(ptr noundef) #2

; Function Attrs: noinline nounwind uwtable
define internal fastcc i32 @ref___sbprintf(ptr noundef %fp, ptr noundef %locale, ptr noundef %fmt, ptr noundef %ap) unnamed_addr #3 {
entry:
  %fake = alloca %struct.__sFILE, align 8
  %buf = alloca [1024 x i8], align 16
  call void @llvm.lifetime.start.p0(i64 312, ptr nonnull %fake) #14
  call void @llvm.lifetime.start.p0(i64 1024, ptr nonnull %buf) #14
  %_flags = getelementptr inbounds %struct.__sFILE, ptr %fp, i64 0, i32 3
  %0 = load i16, ptr %_flags, align 8, !tbaa !9
  %conv54 = zext i16 %0 to i32
  %and = and i32 %conv54, 8
  %cmp = icmp eq i32 %and, 0
  br i1 %cmp, label %land.rhs, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %entry
  %_bf = getelementptr inbounds %struct.__sFILE, ptr %fp, i64 0, i32 5
  %1 = load ptr, ptr %_bf, align 8, !tbaa !16
  %cmp2 = icmp eq ptr %1, null
  %and6 = and i32 %conv54, 512
  %cmp7 = icmp eq i32 %and6, 0
  %or.cond = and i1 %cmp7, %cmp2
  br i1 %or.cond, label %land.rhs, label %if.end

land.rhs:                                         ; preds = %lor.lhs.false, %entry
  %call = tail call i32 @__swsetup(ptr noundef nonnull %fp) #14
  %tobool.not = icmp eq i32 %call, 0
  br i1 %tobool.not, label %land.rhs.if.end_crit_edge, label %cleanup

land.rhs.if.end_crit_edge:                        ; preds = %land.rhs
  %.pre = load i16, ptr %_flags, align 8, !tbaa !9
  br label %if.end

if.end:                                           ; preds = %land.rhs.if.end_crit_edge, %lor.lhs.false
  %2 = phi i16 [ %.pre, %land.rhs.if.end_crit_edge ], [ %0, %lor.lhs.false ]
  %3 = and i16 %2, -3
  %_flags15 = getelementptr inbounds %struct.__sFILE, ptr %fake, i64 0, i32 3
  store i16 %3, ptr %_flags15, align 8, !tbaa !9
  %_file = getelementptr inbounds %struct.__sFILE, ptr %fp, i64 0, i32 4
  %4 = load i16, ptr %_file, align 2, !tbaa !15
  %_file16 = getelementptr inbounds %struct.__sFILE, ptr %fake, i64 0, i32 4
  store i16 %4, ptr %_file16, align 2, !tbaa !15
  %_cookie = getelementptr inbounds %struct.__sFILE, ptr %fp, i64 0, i32 7
  %5 = load ptr, ptr %_cookie, align 8, !tbaa !17
  %_cookie17 = getelementptr inbounds %struct.__sFILE, ptr %fake, i64 0, i32 7
  store ptr %5, ptr %_cookie17, align 8, !tbaa !17
  %_write = getelementptr inbounds %struct.__sFILE, ptr %fp, i64 0, i32 11
  %6 = load ptr, ptr %_write, align 8, !tbaa !18
  %_write18 = getelementptr inbounds %struct.__sFILE, ptr %fake, i64 0, i32 11
  store ptr %6, ptr %_write18, align 8, !tbaa !18
  %_orientation = getelementptr inbounds %struct.__sFILE, ptr %fp, i64 0, i32 23
  %7 = load i32, ptr %_orientation, align 4, !tbaa !19
  %_orientation19 = getelementptr inbounds %struct.__sFILE, ptr %fake, i64 0, i32 23
  store i32 %7, ptr %_orientation19, align 4, !tbaa !19
  %_mbstate = getelementptr inbounds %struct.__sFILE, ptr %fake, i64 0, i32 24
  %_mbstate20 = getelementptr inbounds %struct.__sFILE, ptr %fp, i64 0, i32 24
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 8 dereferenceable(128) %_mbstate, ptr noundef nonnull align 8 dereferenceable(128) %_mbstate20, i64 128, i1 false), !tbaa.struct !20
  store ptr %buf, ptr %fake, align 8, !tbaa !23
  %_bf21 = getelementptr inbounds %struct.__sFILE, ptr %fake, i64 0, i32 5
  store ptr %buf, ptr %_bf21, align 8, !tbaa !16
  %_w = getelementptr inbounds %struct.__sFILE, ptr %fake, i64 0, i32 2
  store i32 1024, ptr %_w, align 4, !tbaa !24
  %_size = getelementptr inbounds %struct.__sFILE, ptr %fake, i64 0, i32 5, i32 1
  store i32 1024, ptr %_size, align 8, !tbaa !25
  %_lbfsize = getelementptr inbounds %struct.__sFILE, ptr %fake, i64 0, i32 6
  store i32 0, ptr %_lbfsize, align 8, !tbaa !26
  %call24 = call i32 @ref___vfwprintf(ptr noundef nonnull %fake, ptr noundef %locale, ptr noundef %fmt, ptr noundef %ap)
  %cmp25 = icmp sgt i32 %call24, -1
  br i1 %cmp25, label %land.lhs.true27, label %if.end31

land.lhs.true27:                                  ; preds = %if.end
  %call28 = call i32 @__fflush(ptr noundef nonnull %fake) #14
  %tobool29.not = icmp eq i32 %call28, 0
  %spec.select = select i1 %tobool29.not, i32 %call24, i32 -1
  br label %if.end31

if.end31:                                         ; preds = %land.lhs.true27, %if.end
  %ret.0 = phi i32 [ %call24, %if.end ], [ %spec.select, %land.lhs.true27 ]
  %8 = load i16, ptr %_flags15, align 8, !tbaa !9
  %9 = and i16 %8, 64
  %tobool35.not = icmp eq i16 %9, 0
  br i1 %tobool35.not, label %cleanup, label %if.then36

if.then36:                                        ; preds = %if.end31
  %10 = load i16, ptr %_flags, align 8, !tbaa !9
  %11 = or i16 %10, 64
  store i16 %11, ptr %_flags, align 8, !tbaa !9
  br label %cleanup

cleanup:                                          ; preds = %if.end31, %if.then36, %land.rhs
  %retval.0 = phi i32 [ -1, %land.rhs ], [ %ret.0, %if.then36 ], [ %ret.0, %if.end31 ]
  call void @llvm.lifetime.end.p0(i64 1024, ptr nonnull %buf) #14
  call void @llvm.lifetime.end.p0(i64 312, ptr nonnull %fake) #14
  ret i32 %retval.0
}

declare void @___pthread_cleanup_pop_imp(i32 noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nounwind uwtable
define dso_local i32 @ref_vfwprintf(ptr noalias noundef %fp, ptr noalias noundef %fmt0, ptr noundef %ap) local_unnamed_addr #0 {
entry:
  %0 = load i32, ptr @__has_thread_locale, align 4, !tbaa !5
  %tobool.not.i = icmp eq i32 %0, 0
  br i1 %tobool.not.i, label %__get_locale.exit, label %lor.lhs.false.i

lor.lhs.false.i:                                  ; preds = %entry
  %1 = tail call align 8 ptr @llvm.threadlocal.address.p0(ptr align 8 @__thread_locale)
  %2 = load ptr, ptr %1, align 8, !tbaa !27
  %cmp.i = icmp eq ptr %2, null
  %spec.select.i = select i1 %cmp.i, ptr @__xlocale_global_locale, ptr %2
  br label %__get_locale.exit

__get_locale.exit:                                ; preds = %entry, %lor.lhs.false.i
  %retval.0.i = phi ptr [ @__xlocale_global_locale, %entry ], [ %spec.select.i, %lor.lhs.false.i ]
  %call1 = tail call i32 @ref_vfwprintf_l(ptr noundef %fp, ptr noundef nonnull %retval.0.i, ptr noundef %fmt0, ptr noundef %ap)
  ret i32 %call1
}

; Function Attrs: nounwind uwtable
define dso_local i32 @ref___vfwprintf(ptr noundef %fp, ptr noundef %locale, ptr noundef %fmt0, ptr noundef %ap) local_unnamed_addr #0 {
entry:
  %mbs.i3084 = alloca %union.__mbstate_t, align 8
  %expbuf.i = alloca [6 x i32], align 16
  %mbs.i3051 = alloca %union.__mbstate_t, align 8
  %mbs.i3048 = alloca %union.__mbstate_t, align 8
  %mbs.i = alloca %union.__mbstate_t, align 8
  %decpt.i = alloca i32, align 4
  %sign = alloca i32, align 4
  %gs = alloca %struct.grouping_state, align 8
  %decimal_point = alloca i32, align 4
  %signflag = alloca i32, align 4
  %fparg = alloca %union.anon, align 16
  %expt = alloca i32, align 4
  %dtoaend = alloca ptr, align 8
  %expstr = alloca [8 x i32], align 16
  %io = alloca %struct.io_state, align 8
  %buf = alloca [64 x i32], align 16
  %ox = alloca [2 x i32], align 4
  %argtable = alloca ptr, align 8
  %statargtable = alloca [8 x %union.arg], align 16
  %orgap = alloca [1 x %struct.__va_list_tag], align 16
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %sign) #14
  call void @llvm.lifetime.start.p0(i64 32, ptr nonnull %gs) #14
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %decimal_point) #14
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %signflag) #14
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %fparg) #14
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %expt) #14
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %dtoaend) #14
  call void @llvm.lifetime.start.p0(i64 32, ptr nonnull %expstr) #14
  call void @llvm.lifetime.start.p0(i64 152, ptr nonnull %io) #14
  call void @llvm.lifetime.start.p0(i64 256, ptr nonnull %buf) #14
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %ox) #14
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %argtable) #14
  call void @llvm.lifetime.start.p0(i64 128, ptr nonnull %statargtable) #14
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %orgap) #14
  %_flags = getelementptr inbounds %struct.__sFILE, ptr %fp, i64 0, i32 3
  %0 = load i16, ptr %_flags, align 8, !tbaa !9
  %conv4398 = zext i16 %0 to i32
  %and = and i32 %conv4398, 8
  %cmp = icmp eq i32 %and, 0
  br i1 %cmp, label %land.rhs, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %entry
  %_bf = getelementptr inbounds %struct.__sFILE, ptr %fp, i64 0, i32 5
  %1 = load ptr, ptr %_bf, align 8, !tbaa !16
  %cmp2 = icmp eq ptr %1, null
  %and6 = and i32 %conv4398, 512
  %cmp7 = icmp eq i32 %and6, 0
  %or.cond3028 = and i1 %cmp7, %cmp2
  br i1 %or.cond3028, label %land.rhs, label %if.end

land.rhs:                                         ; preds = %lor.lhs.false, %entry
  %call = tail call i32 @__swsetup(ptr noundef nonnull %fp) #14
  %tobool.not = icmp eq i32 %call, 0
  br i1 %tobool.not, label %land.rhs.if.end_crit_edge, label %if.then

land.rhs.if.end_crit_edge:                        ; preds = %land.rhs
  %.pre = load i16, ptr %_flags, align 8, !tbaa !9
  br label %if.end

if.then:                                          ; preds = %land.rhs
  %call11 = tail call ptr @__error() #14
  store i32 9, ptr %call11, align 4, !tbaa !5
  br label %cleanup2390

if.end:                                           ; preds = %land.rhs.if.end_crit_edge, %lor.lhs.false
  %2 = phi i16 [ %.pre, %land.rhs.if.end_crit_edge ], [ %0, %lor.lhs.false ]
  %3 = and i16 %2, 64
  %and17 = and i16 %2, -65
  store i16 %and17, ptr %_flags, align 8, !tbaa !9
  store ptr null, ptr %argtable, align 8, !tbaa !27
  call void @llvm.va_copy(ptr nonnull %orgap, ptr %ap)
  %iov.i = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2
  %uio.i = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 1
  store ptr %iov.i, ptr %uio.i, align 8, !tbaa !28
  %uio_resid.i = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 1, i32 2
  store i32 0, ptr %uio_resid.i, align 4, !tbaa !31
  %uio_iovcnt.i = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 1, i32 1
  store i32 0, ptr %uio_iovcnt.i, align 8, !tbaa !32
  store ptr %fp, ptr %io, align 8, !tbaa !33
  call void @llvm.lifetime.start.p0(i64 128, ptr nonnull %mbs.i) #14
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %decpt.i) #14
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(128) %mbs.i, i8 0, i64 128, i1 false)
  %call.i = call ptr @localeconv_l(ptr noundef %locale) #14
  %4 = load ptr, ptr %call.i, align 8, !tbaa !34
  %call1.i = call i32 @___mb_cur_max() #14
  %conv.i = sext i32 %call1.i to i64
  %call2.i = call i64 @mbrtowc(ptr noundef nonnull %decpt.i, ptr noundef %4, i64 noundef %conv.i, ptr noundef nonnull %mbs.i) #14
  %5 = and i64 %call2.i, 4294967294
  %or.cond.i = icmp eq i64 %5, 4294967294
  %.pre.i = load i32, ptr %decpt.i, align 4
  %6 = select i1 %or.cond.i, i32 46, i32 %.pre.i
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %decpt.i) #14
  call void @llvm.lifetime.end.p0(i64 128, ptr nonnull %mbs.i) #14
  store i32 %6, ptr %decimal_point, align 4, !tbaa !5
  %grouping = getelementptr inbounds %struct.grouping_state, ptr %gs, i64 0, i32 1
  %arrayidx = getelementptr inbounds [2 x i32], ptr %ox, i64 0, i64 1
  %overflow_arg_area_p178 = getelementptr inbounds %struct.__va_list_tag, ptr %ap, i64 0, i32 2
  %7 = getelementptr inbounds %struct.__va_list_tag, ptr %ap, i64 0, i32 3
  %fp_offset_p956 = getelementptr inbounds %struct.__va_list_tag, ptr %ap, i64 0, i32 1
  %incdec.ptr.i3079 = getelementptr inbounds i32, ptr %expstr, i64 1
  %p.0.i = getelementptr inbounds i32, ptr %expstr, i64 2
  %incdec.ptr18.i = getelementptr inbounds i32, ptr %expstr, i64 3
  %sub.ptr.rhs.cast.i = ptrtoint ptr %expstr to i64
  %add.ptr2063 = getelementptr inbounds i32, ptr %buf, i64 64
  %sub.ptr.lhs.cast2101 = ptrtoint ptr %add.ptr2063 to i64
  %arrayidx2306 = getelementptr inbounds [64 x i32], ptr %buf, i64 0, i64 1
  br label %for.cond.outer

for.cond.outer:                                   ; preds = %if.end2355, %if.end
  %ret.0.ph = phi i32 [ %add2146, %if.end2355 ], [ 0, %if.end ]
  %expchar.0.ph = phi i8 [ %expchar.5, %if.end2355 ], [ undef, %if.end ]
  %expsize.0.ph = phi i32 [ %expsize.1, %if.end2355 ], [ undef, %if.end ]
  %ndig.0.ph = phi i32 [ %ndig.2, %if.end2355 ], [ undef, %if.end ]
  %ulval.0.ph = phi i64 [ %ulval.8, %if.end2355 ], [ undef, %if.end ]
  %ujval.0.ph = phi i64 [ %ujval.8, %if.end2355 ], [ undef, %if.end ]
  %xdigs.0.ph = phi ptr [ %xdigs.6, %if.end2355 ], [ undef, %if.end ]
  %nextarg.0.ph = phi i32 [ %nextarg.64, %if.end2355 ], [ 1, %if.end ]
  %convbuf.0.ph = phi ptr [ %convbuf.5, %if.end2355 ], [ null, %if.end ]
  %fmt.0.ph = phi ptr [ %fmt.14, %if.end2355 ], [ %fmt0, %if.end ]
  br label %for.cond

for.cond:                                         ; preds = %for.cond.outer, %if.end1282
  %ret.0 = phi i32 [ %ret.1, %if.end1282 ], [ %ret.0.ph, %for.cond.outer ]
  %nextarg.0 = phi i32 [ %nextarg.39, %if.end1282 ], [ %nextarg.0.ph, %for.cond.outer ]
  %fmt.0 = phi ptr [ %fmt.3, %if.end1282 ], [ %fmt.0.ph, %for.cond.outer ]
  br label %for.cond20

for.cond20:                                       ; preds = %for.inc, %for.cond
  %fmt.1 = phi ptr [ %fmt.0, %for.cond ], [ %incdec.ptr, %for.inc ]
  %8 = load i32, ptr %fmt.1, align 4, !tbaa !5
  switch i32 %8, label %for.inc [
    i32 37, label %for.end
    i32 0, label %for.end
  ]

for.inc:                                          ; preds = %for.cond20
  %incdec.ptr = getelementptr inbounds i32, ptr %fmt.1, i64 1
  br label %for.cond20, !llvm.loop !36

for.end:                                          ; preds = %for.cond20, %for.cond20
  %sub.ptr.lhs.cast = ptrtoint ptr %fmt.1 to i64
  %sub.ptr.rhs.cast = ptrtoint ptr %fmt.0 to i64
  %sub.ptr.sub = sub i64 %sub.ptr.lhs.cast, %sub.ptr.rhs.cast
  %sub.ptr.div = lshr exact i64 %sub.ptr.sub, 2
  %conv28 = trunc i64 %sub.ptr.div to i32
  %cmp29.not = icmp eq i32 %conv28, 0
  br i1 %cmp29.not, label %if.end42, label %if.then31

if.then31:                                        ; preds = %for.end
  %add = add i32 %ret.0, %conv28
  %cmp32 = icmp slt i32 %add, 0
  br i1 %cmp32, label %if.then34, label %do.body

if.then34:                                        ; preds = %if.then31
  %call35 = call ptr @__error() #14
  store i32 84, ptr %call35, align 4, !tbaa !5
  br label %error

do.body:                                          ; preds = %if.then31
  %9 = load i32, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !38
  %idxprom.i = sext i32 %9 to i64
  %arrayidx.i = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i
  store ptr %fmt.0, ptr %arrayidx.i, align 8, !tbaa !41, !noalias !38
  %sext3540 = shl i64 %sub.ptr.sub, 30
  %conv.i3044 = ashr i64 %sext3540, 32
  %iov_len.i = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i, i32 1
  store i64 %conv.i3044, ptr %iov_len.i, align 8, !tbaa !43, !noalias !38
  %10 = load i32, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !38
  %add.i = add nsw i32 %10, %conv28
  store i32 %add.i, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !38
  %inc.i = add nsw i32 %9, 1
  store i32 %inc.i, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !38
  %cmp.i = icmp sgt i32 %9, 6
  br i1 %cmp.i, label %io_print.exit, label %if.end42

io_print.exit:                                    ; preds = %do.body
  %11 = load ptr, ptr %io, align 8, !tbaa !33, !noalias !38
  %call.i3047 = call i32 (ptr, ptr, ptr, ...) @__sprint(ptr noundef %11, ptr noundef nonnull %uio.i, ptr noundef %locale) #14
  %tobool38.not = icmp eq i32 %call.i3047, 0
  br i1 %tobool38.not, label %if.end42, label %error

if.end42:                                         ; preds = %do.body, %io_print.exit, %for.end
  %ret.1 = phi i32 [ %ret.0, %for.end ], [ %add, %io_print.exit ], [ %add, %do.body ]
  %cmp43 = icmp eq i32 %8, 0
  br i1 %cmp43, label %done, label %if.end46

if.end46:                                         ; preds = %if.end42
  %incdec.ptr47 = getelementptr inbounds i32, ptr %fmt.1, i64 1
  store ptr null, ptr %grouping, align 8, !tbaa !44
  store i32 0, ptr %sign, align 4, !tbaa !5
  store i32 0, ptr %arrayidx, align 4, !tbaa !5
  br label %rflag

rflag:                                            ; preds = %rflag.backedge, %if.end46
  %flags.0 = phi i32 [ 0, %if.end46 ], [ %flags.0.be, %rflag.backedge ]
  %width.0 = phi i32 [ 0, %if.end46 ], [ %width.0.be, %rflag.backedge ]
  %prec.0 = phi i32 [ -1, %if.end46 ], [ %prec.0.be, %rflag.backedge ]
  %nextarg.1 = phi i32 [ %nextarg.0, %if.end46 ], [ %nextarg.1.be, %rflag.backedge ]
  %fmt.2 = phi ptr [ %incdec.ptr47, %if.end46 ], [ %fmt.2.be, %rflag.backedge ]
  %incdec.ptr48 = getelementptr inbounds i32, ptr %fmt.2, i64 1
  %12 = load i32, ptr %fmt.2, align 4, !tbaa !5
  br label %reswitch.outer

reswitch.outer:                                   ; preds = %do.end209, %rflag
  %width.1.ph = phi i32 [ %add203, %do.end209 ], [ %width.0, %rflag ]
  %prec.1.ph = phi i32 [ %prec.1, %do.end209 ], [ %prec.0, %rflag ]
  %ch.0.ph = phi i32 [ %37, %do.end209 ], [ %12, %rflag ]
  %fmt.3.ph = phi ptr [ %incdec.ptr204, %do.end209 ], [ %incdec.ptr48, %rflag ]
  br label %reswitch

reswitch:                                         ; preds = %reswitch.backedge, %reswitch.outer
  %prec.1 = phi i32 [ %prec.1.ph, %reswitch.outer ], [ %prec.1.be, %reswitch.backedge ]
  %ch.0 = phi i32 [ %ch.0.ph, %reswitch.outer ], [ %ch.0.be, %reswitch.backedge ]
  %fmt.3 = phi ptr [ %fmt.3.ph, %reswitch.outer ], [ %fmt.3.be, %reswitch.backedge ]
  switch i32 %ch.0, label %sw.default [
    i32 32, label %sw.bb
    i32 35, label %sw.bb52
    i32 42, label %while.cond.preheader
    i32 45, label %sw.bb104
    i32 43, label %sw.bb106
    i32 39, label %sw.bb107
    i32 46, label %sw.bb109
    i32 48, label %sw.bb197
    i32 49, label %do.body200.preheader
    i32 50, label %do.body200.preheader
    i32 51, label %do.body200.preheader
    i32 52, label %do.body200.preheader
    i32 53, label %do.body200.preheader
    i32 54, label %do.body200.preheader
    i32 55, label %do.body200.preheader
    i32 56, label %do.body200.preheader
    i32 57, label %do.body200.preheader
    i32 76, label %sw.bb224
    i32 104, label %sw.bb226
    i32 106, label %sw.bb235
    i32 108, label %sw.bb237
    i32 113, label %sw.bb246
    i32 116, label %sw.bb248
    i32 119, label %sw.bb250
    i32 122, label %sw.bb319
    i32 66, label %sw.bb321
    i32 98, label %sw.bb321
    i32 67, label %sw.bb550
    i32 99, label %sw.bb552
    i32 68, label %sw.bb603
    i32 100, label %sw.bb605
    i32 105, label %sw.bb605
    i32 97, label %sw.bb830
    i32 65, label %sw.bb830
    i32 101, label %sw.bb903
    i32 69, label %sw.bb903
    i32 102, label %fp_begin
    i32 70, label %fp_begin
    i32 103, label %sw.bb912
    i32 71, label %sw.bb912
    i32 110, label %sw.bb1072
    i32 79, label %sw.bb1283
    i32 111, label %sw.bb1285
    i32 112, label %sw.bb1500
    i32 83, label %sw.bb1524
    i32 115, label %sw.bb1526
    i32 85, label %sw.bb1609
    i32 117, label %sw.bb1611
    i32 88, label %hex.loopexit
    i32 120, label %hex
  ]

do.body200.preheader:                             ; preds = %reswitch, %reswitch, %reswitch, %reswitch, %reswitch, %reswitch, %reswitch, %reswitch, %reswitch
  br label %do.body200

while.cond.preheader:                             ; preds = %reswitch
  %13 = load i32, ptr %fmt.3, align 4, !tbaa !5
  %sub3984 = add nsw i32 %13, -48
  %cmp543985 = icmp ult i32 %sub3984, 10
  br i1 %cmp543985, label %while.body, label %while.end

sw.bb:                                            ; preds = %reswitch
  %14 = load i32, ptr %sign, align 4, !tbaa !5
  %tobool49.not = icmp eq i32 %14, 0
  br i1 %tobool49.not, label %if.then50, label %rflag.backedge

if.then50:                                        ; preds = %sw.bb
  store i32 32, ptr %sign, align 4, !tbaa !5
  br label %rflag.backedge

sw.bb52:                                          ; preds = %reswitch
  %or = or i32 %flags.0, 1
  br label %rflag.backedge

while.body:                                       ; preds = %while.cond.preheader, %while.body
  %sub3988 = phi i32 [ %sub, %while.body ], [ %sub3984, %while.cond.preheader ]
  %n2.03987 = phi i32 [ %add57, %while.body ], [ 0, %while.cond.preheader ]
  %cp.13986 = phi ptr [ %incdec.ptr58, %while.body ], [ %fmt.3, %while.cond.preheader ]
  %mul = mul nsw i32 %n2.03987, 10
  %add57 = add nsw i32 %sub3988, %mul
  %incdec.ptr58 = getelementptr inbounds i32, ptr %cp.13986, i64 1
  %15 = load i32, ptr %incdec.ptr58, align 4, !tbaa !5
  %sub = add nsw i32 %15, -48
  %cmp54 = icmp ult i32 %sub, 10
  br i1 %cmp54, label %while.body, label %while.end, !llvm.loop !46

while.end:                                        ; preds = %while.body, %while.cond.preheader
  %cp.1.lcssa = phi ptr [ %fmt.3, %while.cond.preheader ], [ %incdec.ptr58, %while.body ]
  %n2.0.lcssa = phi i32 [ 0, %while.cond.preheader ], [ %add57, %while.body ]
  %.lcssa3740 = phi i32 [ %13, %while.cond.preheader ], [ %15, %while.body ]
  %cmp59 = icmp eq i32 %.lcssa3740, 36
  %16 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp62 = icmp eq ptr %16, null
  br i1 %cmp59, label %if.then61, label %if.else

if.then61:                                        ; preds = %while.end
  br i1 %cmp62, label %if.then64, label %cond.true

if.then64:                                        ; preds = %if.then61
  store ptr %statargtable, ptr %argtable, align 8, !tbaa !27
  %call67 = call i32 @__find_warguments(ptr noundef %fmt0, ptr noundef nonnull %orgap, ptr noundef nonnull %argtable) #14
  %tobool68.not = icmp eq i32 %call67, 0
  br i1 %tobool68.not, label %if.end71, label %error

if.end71:                                         ; preds = %if.then64
  %.pr = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp72.not = icmp eq ptr %.pr, null
  br i1 %cmp72.not, label %cond.false, label %cond.true

cond.true:                                        ; preds = %if.then61, %if.end71
  %17 = phi ptr [ %.pr, %if.end71 ], [ %16, %if.then61 ]
  %idxprom = sext i32 %n2.0.lcssa to i64
  %arrayidx74 = getelementptr inbounds %union.arg, ptr %17, i64 %idxprom
  br label %cleanup.thread

cond.false:                                       ; preds = %if.end71
  %gp_offset = load i32, ptr %ap, align 8
  %fits_in_gp = icmp ult i32 %gp_offset, 41
  br i1 %fits_in_gp, label %vaarg.in_reg, label %vaarg.in_mem

vaarg.in_reg:                                     ; preds = %cond.false
  %reg_save_area = load ptr, ptr %7, align 8
  %18 = zext nneg i32 %gp_offset to i64
  %19 = getelementptr i8, ptr %reg_save_area, i64 %18
  %20 = add nuw nsw i32 %gp_offset, 8
  store i32 %20, ptr %ap, align 8
  br label %cleanup.thread

vaarg.in_mem:                                     ; preds = %cond.false
  %overflow_arg_area = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next = getelementptr i8, ptr %overflow_arg_area, i64 8
  store ptr %overflow_arg_area.next, ptr %overflow_arg_area_p178, align 8
  br label %cleanup.thread

cleanup.thread:                                   ; preds = %cond.true, %vaarg.in_mem, %vaarg.in_reg
  %cond.in = phi ptr [ %arrayidx74, %cond.true ], [ %19, %vaarg.in_reg ], [ %overflow_arg_area, %vaarg.in_mem ]
  %incdec.ptr76 = getelementptr inbounds i32, ptr %cp.1.lcssa, i64 1
  br label %if.end98

if.else:                                          ; preds = %while.end
  br i1 %cmp62, label %cond.false83, label %cond.true79

cond.true79:                                      ; preds = %if.else
  %idxprom81 = sext i32 %nextarg.1 to i64
  %arrayidx82 = getelementptr inbounds %union.arg, ptr %16, i64 %idxprom81
  br label %cond.end96

cond.false83:                                     ; preds = %if.else
  %gp_offset86 = load i32, ptr %ap, align 8
  %fits_in_gp87 = icmp ult i32 %gp_offset86, 41
  br i1 %fits_in_gp87, label %vaarg.in_reg88, label %vaarg.in_mem90

vaarg.in_reg88:                                   ; preds = %cond.false83
  %reg_save_area89 = load ptr, ptr %7, align 8
  %21 = zext nneg i32 %gp_offset86 to i64
  %22 = getelementptr i8, ptr %reg_save_area89, i64 %21
  %23 = add nuw nsw i32 %gp_offset86, 8
  store i32 %23, ptr %ap, align 8
  br label %cond.end96

vaarg.in_mem90:                                   ; preds = %cond.false83
  %overflow_arg_area92 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next93 = getelementptr i8, ptr %overflow_arg_area92, i64 8
  store ptr %overflow_arg_area.next93, ptr %overflow_arg_area_p178, align 8
  br label %cond.end96

cond.end96:                                       ; preds = %vaarg.in_reg88, %vaarg.in_mem90, %cond.true79
  %cond97.in = phi ptr [ %arrayidx82, %cond.true79 ], [ %22, %vaarg.in_reg88 ], [ %overflow_arg_area92, %vaarg.in_mem90 ]
  %nextarg.3 = add nsw i32 %nextarg.1, 1
  br label %if.end98

if.end98:                                         ; preds = %cleanup.thread, %cond.end96
  %width.3.in = phi ptr [ %cond97.in, %cond.end96 ], [ %cond.in, %cleanup.thread ]
  %nextarg.4 = phi i32 [ %nextarg.3, %cond.end96 ], [ %nextarg.1, %cleanup.thread ]
  %fmt.5 = phi ptr [ %fmt.3, %cond.end96 ], [ %incdec.ptr76, %cleanup.thread ]
  %width.3 = load i32, ptr %width.3.in, align 4
  %cmp99 = icmp sgt i32 %width.3, -1
  br i1 %cmp99, label %rflag.backedge, label %if.end102

if.end102:                                        ; preds = %if.end98
  %sub103 = sub nsw i32 0, %width.3
  br label %sw.bb104

sw.bb104:                                         ; preds = %reswitch, %if.end102
  %width.4 = phi i32 [ %sub103, %if.end102 ], [ %width.1.ph, %reswitch ]
  %nextarg.5 = phi i32 [ %nextarg.4, %if.end102 ], [ %nextarg.1, %reswitch ]
  %fmt.6 = phi ptr [ %fmt.5, %if.end102 ], [ %fmt.3, %reswitch ]
  %or105 = or i32 %flags.0, 4
  br label %rflag.backedge

sw.bb106:                                         ; preds = %reswitch
  store i32 43, ptr %sign, align 4, !tbaa !5
  br label %rflag.backedge

sw.bb107:                                         ; preds = %reswitch
  %or108 = or i32 %flags.0, 512
  br label %rflag.backedge

sw.bb109:                                         ; preds = %reswitch
  %incdec.ptr110 = getelementptr inbounds i32, ptr %fmt.3, i64 1
  %24 = load i32, ptr %fmt.3, align 4, !tbaa !5
  %cmp111 = icmp eq i32 %24, 42
  br i1 %cmp111, label %while.cond114.preheader, label %while.cond187.preheader

while.cond114.preheader:                          ; preds = %sw.bb109
  %25 = load i32, ptr %incdec.ptr110, align 4, !tbaa !5
  %sub1153976 = add nsw i32 %25, -48
  %cmp1163977 = icmp ult i32 %sub1153976, 10
  br i1 %cmp1163977, label %while.body118, label %while.end123

while.cond187.preheader:                          ; preds = %sw.bb109
  %sub1883969 = add nsw i32 %24, -48
  %cmp1893970 = icmp ult i32 %sub1883969, 10
  br i1 %cmp1893970, label %while.body191, label %reswitch.backedge

reswitch.backedge:                                ; preds = %while.body191, %while.cond187.preheader
  %prec.1.be = phi i32 [ 0, %while.cond187.preheader ], [ %add194, %while.body191 ]
  %ch.0.be = phi i32 [ %24, %while.cond187.preheader ], [ %36, %while.body191 ]
  %fmt.3.be = phi ptr [ %incdec.ptr110, %while.cond187.preheader ], [ %incdec.ptr195, %while.body191 ]
  br label %reswitch

while.body118:                                    ; preds = %while.cond114.preheader, %while.body118
  %sub1153980 = phi i32 [ %sub115, %while.body118 ], [ %sub1153976, %while.cond114.preheader ]
  %n2.13979 = phi i32 [ %add121, %while.body118 ], [ 0, %while.cond114.preheader ]
  %cp.53978 = phi ptr [ %incdec.ptr122, %while.body118 ], [ %incdec.ptr110, %while.cond114.preheader ]
  %mul119 = mul nsw i32 %n2.13979, 10
  %add121 = add nsw i32 %sub1153980, %mul119
  %incdec.ptr122 = getelementptr inbounds i32, ptr %cp.53978, i64 1
  %26 = load i32, ptr %incdec.ptr122, align 4, !tbaa !5
  %sub115 = add nsw i32 %26, -48
  %cmp116 = icmp ult i32 %sub115, 10
  br i1 %cmp116, label %while.body118, label %while.end123, !llvm.loop !47

while.end123:                                     ; preds = %while.body118, %while.cond114.preheader
  %27 = phi ptr [ %fmt.3, %while.cond114.preheader ], [ %cp.53978, %while.body118 ]
  %n2.1.lcssa = phi i32 [ 0, %while.cond114.preheader ], [ %add121, %while.body118 ]
  %.lcssa3739 = phi i32 [ %25, %while.cond114.preheader ], [ %26, %while.body118 ]
  %cmp124 = icmp eq i32 %.lcssa3739, 36
  %28 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp128 = icmp eq ptr %28, null
  br i1 %cmp124, label %if.then126, label %if.else163

if.then126:                                       ; preds = %while.end123
  br i1 %cmp128, label %if.then130, label %cond.true140

if.then130:                                       ; preds = %if.then126
  store ptr %statargtable, ptr %argtable, align 8, !tbaa !27
  %call133 = call i32 @__find_warguments(ptr noundef %fmt0, ptr noundef nonnull %orgap, ptr noundef nonnull %argtable) #14
  %tobool134.not = icmp eq i32 %call133, 0
  br i1 %tobool134.not, label %if.end137, label %error

if.end137:                                        ; preds = %if.then130
  %.pr3449 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp138.not = icmp eq ptr %.pr3449, null
  br i1 %cmp138.not, label %cond.false144, label %cond.true140

cond.true140:                                     ; preds = %if.then126, %if.end137
  %29 = phi ptr [ %.pr3449, %if.end137 ], [ %28, %if.then126 ]
  %idxprom142 = sext i32 %n2.1.lcssa to i64
  %arrayidx143 = getelementptr inbounds %union.arg, ptr %29, i64 %idxprom142
  br label %cleanup160.thread

cond.false144:                                    ; preds = %if.end137
  %gp_offset147 = load i32, ptr %ap, align 8
  %fits_in_gp148 = icmp ult i32 %gp_offset147, 41
  br i1 %fits_in_gp148, label %vaarg.in_reg149, label %vaarg.in_mem151

vaarg.in_reg149:                                  ; preds = %cond.false144
  %reg_save_area150 = load ptr, ptr %7, align 8
  %30 = zext nneg i32 %gp_offset147 to i64
  %31 = getelementptr i8, ptr %reg_save_area150, i64 %30
  %32 = add nuw nsw i32 %gp_offset147, 8
  store i32 %32, ptr %ap, align 8
  br label %cleanup160.thread

vaarg.in_mem151:                                  ; preds = %cond.false144
  %overflow_arg_area153 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next154 = getelementptr i8, ptr %overflow_arg_area153, i64 8
  store ptr %overflow_arg_area.next154, ptr %overflow_arg_area_p178, align 8
  br label %cleanup160.thread

cleanup160.thread:                                ; preds = %cond.true140, %vaarg.in_mem151, %vaarg.in_reg149
  %cond158.in = phi ptr [ %arrayidx143, %cond.true140 ], [ %31, %vaarg.in_reg149 ], [ %overflow_arg_area153, %vaarg.in_mem151 ]
  %incdec.ptr159 = getelementptr inbounds i32, ptr %27, i64 2
  br label %if.end185

if.else163:                                       ; preds = %while.end123
  br i1 %cmp128, label %cond.false170, label %cond.true166

cond.true166:                                     ; preds = %if.else163
  %idxprom168 = sext i32 %nextarg.1 to i64
  %arrayidx169 = getelementptr inbounds %union.arg, ptr %28, i64 %idxprom168
  br label %cond.end183

cond.false170:                                    ; preds = %if.else163
  %gp_offset173 = load i32, ptr %ap, align 8
  %fits_in_gp174 = icmp ult i32 %gp_offset173, 41
  br i1 %fits_in_gp174, label %vaarg.in_reg175, label %vaarg.in_mem177

vaarg.in_reg175:                                  ; preds = %cond.false170
  %reg_save_area176 = load ptr, ptr %7, align 8
  %33 = zext nneg i32 %gp_offset173 to i64
  %34 = getelementptr i8, ptr %reg_save_area176, i64 %33
  %35 = add nuw nsw i32 %gp_offset173, 8
  store i32 %35, ptr %ap, align 8
  br label %cond.end183

vaarg.in_mem177:                                  ; preds = %cond.false170
  %overflow_arg_area179 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next180 = getelementptr i8, ptr %overflow_arg_area179, i64 8
  store ptr %overflow_arg_area.next180, ptr %overflow_arg_area_p178, align 8
  br label %cond.end183

cond.end183:                                      ; preds = %vaarg.in_reg175, %vaarg.in_mem177, %cond.true166
  %cond184.in = phi ptr [ %arrayidx169, %cond.true166 ], [ %34, %vaarg.in_reg175 ], [ %overflow_arg_area179, %vaarg.in_mem177 ]
  %nextarg.7 = add nsw i32 %nextarg.1, 1
  br label %if.end185

if.end185:                                        ; preds = %cleanup160.thread, %cond.end183
  %prec.3.in = phi ptr [ %cond184.in, %cond.end183 ], [ %cond158.in, %cleanup160.thread ]
  %nextarg.8 = phi i32 [ %nextarg.7, %cond.end183 ], [ %nextarg.1, %cleanup160.thread ]
  %fmt.8 = phi ptr [ %incdec.ptr110, %cond.end183 ], [ %incdec.ptr159, %cleanup160.thread ]
  %prec.3 = load i32, ptr %prec.3.in, align 4
  br label %rflag.backedge

while.body191:                                    ; preds = %while.cond187.preheader, %while.body191
  %sub1883973 = phi i32 [ %sub188, %while.body191 ], [ %sub1883969, %while.cond187.preheader ]
  %fmt.93972 = phi ptr [ %incdec.ptr195, %while.body191 ], [ %incdec.ptr110, %while.cond187.preheader ]
  %prec.43971 = phi i32 [ %add194, %while.body191 ], [ 0, %while.cond187.preheader ]
  %mul192 = mul nsw i32 %prec.43971, 10
  %add194 = add nsw i32 %sub1883973, %mul192
  %incdec.ptr195 = getelementptr inbounds i32, ptr %fmt.93972, i64 1
  %36 = load i32, ptr %fmt.93972, align 4, !tbaa !5
  %sub188 = add nsw i32 %36, -48
  %cmp189 = icmp ult i32 %sub188, 10
  br i1 %cmp189, label %while.body191, label %reswitch.backedge, !llvm.loop !48

sw.bb197:                                         ; preds = %reswitch
  %or198 = or i32 %flags.0, 128
  br label %rflag.backedge

do.body200:                                       ; preds = %do.body200.preheader, %do.body200
  %n.0 = phi i32 [ %add203, %do.body200 ], [ 0, %do.body200.preheader ]
  %ch.2 = phi i32 [ %37, %do.body200 ], [ %ch.0, %do.body200.preheader ]
  %fmt.10 = phi ptr [ %incdec.ptr204, %do.body200 ], [ %fmt.3, %do.body200.preheader ]
  %mul201 = mul nsw i32 %n.0, 10
  %sub202 = add nsw i32 %ch.2, -48
  %add203 = add nsw i32 %sub202, %mul201
  %incdec.ptr204 = getelementptr inbounds i32, ptr %fmt.10, i64 1
  %37 = load i32, ptr %fmt.10, align 4, !tbaa !5
  %sub206 = add nsw i32 %37, -48
  %cmp207 = icmp ult i32 %sub206, 10
  br i1 %cmp207, label %do.body200, label %do.end209, !llvm.loop !49

do.end209:                                        ; preds = %do.body200
  %cmp210 = icmp eq i32 %37, 36
  br i1 %cmp210, label %if.then212, label %reswitch.outer

if.then212:                                       ; preds = %do.end209
  %38 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp213 = icmp eq ptr %38, null
  br i1 %cmp213, label %if.then215, label %rflag.backedge

if.then215:                                       ; preds = %if.then212
  store ptr %statargtable, ptr %argtable, align 8, !tbaa !27
  %call218 = call i32 @__find_warguments(ptr noundef %fmt0, ptr noundef nonnull %orgap, ptr noundef nonnull %argtable) #14
  %tobool219.not = icmp eq i32 %call218, 0
  br i1 %tobool219.not, label %rflag.backedge, label %error

sw.bb224:                                         ; preds = %reswitch
  %or225 = or i32 %flags.0, 8
  br label %rflag.backedge

sw.bb226:                                         ; preds = %reswitch
  %and227 = and i32 %flags.0, 64
  %tobool228.not = icmp eq i32 %and227, 0
  %and230 = and i32 %flags.0, -8257
  %or231 = or disjoint i32 %and230, 8192
  %or233 = or disjoint i32 %flags.0, 64
  %flags.1 = select i1 %tobool228.not, i32 %or233, i32 %or231
  br label %rflag.backedge

sw.bb235:                                         ; preds = %reswitch
  %or236 = or i32 %flags.0, 4096
  br label %rflag.backedge

sw.bb237:                                         ; preds = %reswitch
  %and238 = and i32 %flags.0, 16
  %tobool239.not = icmp eq i32 %and238, 0
  %and241 = and i32 %flags.0, -49
  %or242 = or disjoint i32 %and241, 32
  %or244 = or disjoint i32 %flags.0, 16
  %flags.2 = select i1 %tobool239.not, i32 %or244, i32 %or242
  br label %rflag.backedge

sw.bb246:                                         ; preds = %reswitch
  %or247 = or i32 %flags.0, 32
  br label %rflag.backedge

sw.bb248:                                         ; preds = %reswitch
  %or249 = or i32 %flags.0, 2048
  br label %rflag.backedge

sw.bb250:                                         ; preds = %reswitch
  %39 = load i32, ptr %fmt.3, align 4, !tbaa !5
  %cmp253 = icmp eq i32 %39, 102
  %and251 = and i32 %flags.0, -28785
  %or256 = or disjoint i32 %and251, 16384
  %flags.3 = select i1 %cmp253, i32 %or256, i32 %and251
  %fmt.11.idx = zext i1 %cmp253 to i64
  %fmt.11 = getelementptr inbounds i32, ptr %fmt.3, i64 %fmt.11.idx
  %40 = load i32, ptr %fmt.11, align 4, !tbaa !5
  switch i32 %40, label %if.else308 [
    i32 56, label %if.then264
    i32 49, label %land.lhs.true275
    i32 51, label %land.lhs.true291
    i32 54, label %land.lhs.true301
  ]

if.then264:                                       ; preds = %sw.bb250
  %and265 = lshr i32 %flags.3, 1
  %41 = and i32 %and265, 8192
  %42 = or disjoint i32 %41, %flags.3
  %spec.select = xor i32 %42, 8192
  %add.ptr = getelementptr inbounds i32, ptr %fmt.11, i64 1
  br label %rflag.backedge

rflag.backedge:                                   ; preds = %if.then264, %if.then295, %if.then305, %if.then279, %if.then212, %if.then215, %sw.bb, %if.then50, %sw.bb52, %sw.bb104, %sw.bb106, %sw.bb107, %if.end185, %sw.bb197, %sw.bb224, %sw.bb226, %sw.bb235, %sw.bb237, %sw.bb246, %sw.bb248, %sw.bb319, %if.end98
  %flags.0.be = phi i32 [ %or320, %sw.bb319 ], [ %or249, %sw.bb248 ], [ %or247, %sw.bb246 ], [ %flags.2, %sw.bb237 ], [ %or236, %sw.bb235 ], [ %flags.1, %sw.bb226 ], [ %or225, %sw.bb224 ], [ %or198, %sw.bb197 ], [ %flags.0, %if.end185 ], [ %or108, %sw.bb107 ], [ %flags.0, %sw.bb106 ], [ %or105, %sw.bb104 ], [ %flags.0, %if.end98 ], [ %or, %sw.bb52 ], [ %flags.0, %if.then50 ], [ %flags.0, %sw.bb ], [ %flags.0, %if.then215 ], [ %flags.0, %if.then212 ], [ %spec.select3029, %if.then279 ], [ %or306, %if.then305 ], [ %flags.3, %if.then295 ], [ %spec.select, %if.then264 ]
  %width.0.be = phi i32 [ %width.1.ph, %sw.bb319 ], [ %width.1.ph, %sw.bb248 ], [ %width.1.ph, %sw.bb246 ], [ %width.1.ph, %sw.bb237 ], [ %width.1.ph, %sw.bb235 ], [ %width.1.ph, %sw.bb226 ], [ %width.1.ph, %sw.bb224 ], [ %width.1.ph, %sw.bb197 ], [ %width.1.ph, %if.end185 ], [ %width.1.ph, %sw.bb107 ], [ %width.1.ph, %sw.bb106 ], [ %width.4, %sw.bb104 ], [ %width.3, %if.end98 ], [ %width.1.ph, %sw.bb52 ], [ %width.1.ph, %if.then50 ], [ %width.1.ph, %sw.bb ], [ %width.1.ph, %if.then215 ], [ %width.1.ph, %if.then212 ], [ %width.1.ph, %if.then279 ], [ %width.1.ph, %if.then305 ], [ %width.1.ph, %if.then295 ], [ %width.1.ph, %if.then264 ]
  %prec.0.be = phi i32 [ %prec.1, %sw.bb319 ], [ %prec.1, %sw.bb248 ], [ %prec.1, %sw.bb246 ], [ %prec.1, %sw.bb237 ], [ %prec.1, %sw.bb235 ], [ %prec.1, %sw.bb226 ], [ %prec.1, %sw.bb224 ], [ %prec.1, %sw.bb197 ], [ %prec.3, %if.end185 ], [ %prec.1, %sw.bb107 ], [ %prec.1, %sw.bb106 ], [ %prec.1, %sw.bb104 ], [ %prec.1, %if.end98 ], [ %prec.1, %sw.bb52 ], [ %prec.1, %if.then50 ], [ %prec.1, %sw.bb ], [ %prec.1, %if.then215 ], [ %prec.1, %if.then212 ], [ %prec.1, %if.then279 ], [ %prec.1, %if.then305 ], [ %prec.1, %if.then295 ], [ %prec.1, %if.then264 ]
  %nextarg.1.be = phi i32 [ %nextarg.1, %sw.bb319 ], [ %nextarg.1, %sw.bb248 ], [ %nextarg.1, %sw.bb246 ], [ %nextarg.1, %sw.bb237 ], [ %nextarg.1, %sw.bb235 ], [ %nextarg.1, %sw.bb226 ], [ %nextarg.1, %sw.bb224 ], [ %nextarg.1, %sw.bb197 ], [ %nextarg.8, %if.end185 ], [ %nextarg.1, %sw.bb107 ], [ %nextarg.1, %sw.bb106 ], [ %nextarg.5, %sw.bb104 ], [ %nextarg.4, %if.end98 ], [ %nextarg.1, %sw.bb52 ], [ %nextarg.1, %if.then50 ], [ %nextarg.1, %sw.bb ], [ %add203, %if.then215 ], [ %add203, %if.then212 ], [ %nextarg.1, %if.then279 ], [ %nextarg.1, %if.then305 ], [ %nextarg.1, %if.then295 ], [ %nextarg.1, %if.then264 ]
  %fmt.2.be = phi ptr [ %fmt.3, %sw.bb319 ], [ %fmt.3, %sw.bb248 ], [ %fmt.3, %sw.bb246 ], [ %fmt.3, %sw.bb237 ], [ %fmt.3, %sw.bb235 ], [ %fmt.3, %sw.bb226 ], [ %fmt.3, %sw.bb224 ], [ %fmt.3, %sw.bb197 ], [ %fmt.8, %if.end185 ], [ %fmt.3, %sw.bb107 ], [ %fmt.3, %sw.bb106 ], [ %fmt.6, %sw.bb104 ], [ %fmt.5, %if.end98 ], [ %fmt.3, %sw.bb52 ], [ %fmt.3, %if.then50 ], [ %fmt.3, %sw.bb ], [ %incdec.ptr204, %if.then215 ], [ %incdec.ptr204, %if.then212 ], [ %add.ptr286, %if.then279 ], [ %add.ptr307, %if.then305 ], [ %add.ptr296, %if.then295 ], [ %add.ptr, %if.then264 ]
  br label %rflag

land.lhs.true275:                                 ; preds = %sw.bb250
  %arrayidx276 = getelementptr inbounds i32, ptr %fmt.11, i64 1
  %43 = load i32, ptr %arrayidx276, align 4, !tbaa !5
  %cmp277 = icmp eq i32 %43, 54
  br i1 %cmp277, label %if.then279, label %if.else308

if.then279:                                       ; preds = %land.lhs.true275
  %and280 = lshr i32 %flags.3, 8
  %44 = and i32 %and280, 64
  %45 = or disjoint i32 %44, %flags.3
  %spec.select3029 = xor i32 %45, 64
  %add.ptr286 = getelementptr inbounds i32, ptr %fmt.11, i64 2
  br label %rflag.backedge

land.lhs.true291:                                 ; preds = %sw.bb250
  %arrayidx292 = getelementptr inbounds i32, ptr %fmt.11, i64 1
  %46 = load i32, ptr %arrayidx292, align 4, !tbaa !5
  %cmp293 = icmp eq i32 %46, 50
  br i1 %cmp293, label %if.then295, label %if.else308

if.then295:                                       ; preds = %land.lhs.true291
  %add.ptr296 = getelementptr inbounds i32, ptr %fmt.11, i64 2
  br label %rflag.backedge

land.lhs.true301:                                 ; preds = %sw.bb250
  %arrayidx302 = getelementptr inbounds i32, ptr %fmt.11, i64 1
  %47 = load i32, ptr %arrayidx302, align 4, !tbaa !5
  %cmp303 = icmp eq i32 %47, 52
  br i1 %cmp303, label %if.then305, label %if.else308

if.then305:                                       ; preds = %land.lhs.true301
  %or306 = or disjoint i32 %flags.3, 32
  %add.ptr307 = getelementptr inbounds i32, ptr %fmt.11, i64 2
  br label %rflag.backedge

if.else308:                                       ; preds = %sw.bb250, %land.lhs.true275, %land.lhs.true291, %land.lhs.true301
  %and309 = and i32 %flags.3, 16384
  %tobool310.not = icmp ne i32 %and309, 0
  %spec.select3538 = select i1 %tobool310.not, i32 %and251, i32 %flags.3
  %spec.select3539.idx = sext i1 %tobool310.not to i64
  %spec.select3539 = getelementptr inbounds i32, ptr %fmt.11, i64 %spec.select3539.idx
  br label %invalid

sw.bb319:                                         ; preds = %reswitch
  %or320 = or i32 %flags.0, 1024
  br label %rflag.backedge

sw.bb321:                                         ; preds = %reswitch, %reswitch
  %and322 = and i32 %flags.0, 7200
  %tobool323.not = icmp eq i32 %and322, 0
  br i1 %tobool323.not, label %if.else427, label %if.then324

if.then324:                                       ; preds = %sw.bb321
  %and325 = and i32 %flags.0, 4096
  %tobool326.not = icmp eq i32 %and325, 0
  br i1 %tobool326.not, label %cond.false349, label %cond.true327

cond.true327:                                     ; preds = %if.then324
  %48 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp328.not = icmp eq ptr %48, null
  br i1 %cmp328.not, label %cond.false334, label %cond.true330

cond.true330:                                     ; preds = %cond.true327
  %idxprom332 = sext i32 %nextarg.1 to i64
  %arrayidx333 = getelementptr inbounds %union.arg, ptr %48, i64 %idxprom332
  br label %if.end535.thread

cond.false334:                                    ; preds = %cond.true327
  %gp_offset337 = load i32, ptr %ap, align 8
  %fits_in_gp338 = icmp ult i32 %gp_offset337, 41
  br i1 %fits_in_gp338, label %vaarg.in_reg339, label %vaarg.in_mem341

vaarg.in_reg339:                                  ; preds = %cond.false334
  %reg_save_area340 = load ptr, ptr %7, align 8
  %49 = zext nneg i32 %gp_offset337 to i64
  %50 = getelementptr i8, ptr %reg_save_area340, i64 %49
  %51 = add nuw nsw i32 %gp_offset337, 8
  store i32 %51, ptr %ap, align 8
  br label %if.end535.thread

vaarg.in_mem341:                                  ; preds = %cond.false334
  %overflow_arg_area343 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next344 = getelementptr i8, ptr %overflow_arg_area343, i64 8
  store ptr %overflow_arg_area.next344, ptr %overflow_arg_area_p178, align 8
  br label %if.end535.thread

cond.false349:                                    ; preds = %if.then324
  %and350 = and i32 %flags.0, 1024
  %tobool351.not = icmp eq i32 %and350, 0
  br i1 %tobool351.not, label %cond.false374, label %cond.true352

cond.true352:                                     ; preds = %cond.false349
  %52 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp353.not = icmp eq ptr %52, null
  br i1 %cmp353.not, label %cond.false359, label %cond.true355

cond.true355:                                     ; preds = %cond.true352
  %idxprom357 = sext i32 %nextarg.1 to i64
  %arrayidx358 = getelementptr inbounds %union.arg, ptr %52, i64 %idxprom357
  br label %if.end535.thread

cond.false359:                                    ; preds = %cond.true352
  %gp_offset362 = load i32, ptr %ap, align 8
  %fits_in_gp363 = icmp ult i32 %gp_offset362, 41
  br i1 %fits_in_gp363, label %vaarg.in_reg364, label %vaarg.in_mem366

vaarg.in_reg364:                                  ; preds = %cond.false359
  %reg_save_area365 = load ptr, ptr %7, align 8
  %53 = zext nneg i32 %gp_offset362 to i64
  %54 = getelementptr i8, ptr %reg_save_area365, i64 %53
  %55 = add nuw nsw i32 %gp_offset362, 8
  store i32 %55, ptr %ap, align 8
  br label %if.end535.thread

vaarg.in_mem366:                                  ; preds = %cond.false359
  %overflow_arg_area368 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next369 = getelementptr i8, ptr %overflow_arg_area368, i64 8
  store ptr %overflow_arg_area.next369, ptr %overflow_arg_area_p178, align 8
  br label %if.end535.thread

cond.false374:                                    ; preds = %cond.false349
  %and375 = and i32 %flags.0, 2048
  %tobool376.not = icmp eq i32 %and375, 0
  %56 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp400.not = icmp eq ptr %56, null
  br i1 %tobool376.not, label %cond.false399, label %cond.true377

cond.true377:                                     ; preds = %cond.false374
  br i1 %cmp400.not, label %cond.false384, label %cond.true380

cond.true380:                                     ; preds = %cond.true377
  %idxprom382 = sext i32 %nextarg.1 to i64
  %arrayidx383 = getelementptr inbounds %union.arg, ptr %56, i64 %idxprom382
  br label %if.end535.thread

cond.false384:                                    ; preds = %cond.true377
  %gp_offset387 = load i32, ptr %ap, align 8
  %fits_in_gp388 = icmp ult i32 %gp_offset387, 41
  br i1 %fits_in_gp388, label %vaarg.in_reg389, label %vaarg.in_mem391

vaarg.in_reg389:                                  ; preds = %cond.false384
  %reg_save_area390 = load ptr, ptr %7, align 8
  %57 = zext nneg i32 %gp_offset387 to i64
  %58 = getelementptr i8, ptr %reg_save_area390, i64 %57
  %59 = add nuw nsw i32 %gp_offset387, 8
  store i32 %59, ptr %ap, align 8
  br label %if.end535.thread

vaarg.in_mem391:                                  ; preds = %cond.false384
  %overflow_arg_area393 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next394 = getelementptr i8, ptr %overflow_arg_area393, i64 8
  store ptr %overflow_arg_area.next394, ptr %overflow_arg_area_p178, align 8
  br label %if.end535.thread

cond.false399:                                    ; preds = %cond.false374
  br i1 %cmp400.not, label %cond.false406, label %cond.true402

cond.true402:                                     ; preds = %cond.false399
  %idxprom404 = sext i32 %nextarg.1 to i64
  %arrayidx405 = getelementptr inbounds %union.arg, ptr %56, i64 %idxprom404
  br label %if.end535.thread

cond.false406:                                    ; preds = %cond.false399
  %gp_offset409 = load i32, ptr %ap, align 8
  %fits_in_gp410 = icmp ult i32 %gp_offset409, 41
  br i1 %fits_in_gp410, label %vaarg.in_reg411, label %vaarg.in_mem413

vaarg.in_reg411:                                  ; preds = %cond.false406
  %reg_save_area412 = load ptr, ptr %7, align 8
  %60 = zext nneg i32 %gp_offset409 to i64
  %61 = getelementptr i8, ptr %reg_save_area412, i64 %60
  %62 = add nuw nsw i32 %gp_offset409, 8
  store i32 %62, ptr %ap, align 8
  br label %if.end535.thread

vaarg.in_mem413:                                  ; preds = %cond.false406
  %overflow_arg_area415 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next416 = getelementptr i8, ptr %overflow_arg_area415, i64 8
  store ptr %overflow_arg_area.next416, ptr %overflow_arg_area_p178, align 8
  br label %if.end535.thread

if.else427:                                       ; preds = %sw.bb321
  %and428 = and i32 %flags.0, 16
  %tobool429.not = icmp eq i32 %and428, 0
  br i1 %tobool429.not, label %cond.false452, label %cond.true430

cond.true430:                                     ; preds = %if.else427
  %63 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp431.not = icmp eq ptr %63, null
  br i1 %cmp431.not, label %cond.false437, label %cond.true433

cond.true433:                                     ; preds = %cond.true430
  %idxprom435 = sext i32 %nextarg.1 to i64
  %arrayidx436 = getelementptr inbounds %union.arg, ptr %63, i64 %idxprom435
  %64 = load i64, ptr %arrayidx436, align 8, !tbaa !22
  br label %if.end535

cond.false437:                                    ; preds = %cond.true430
  %gp_offset440 = load i32, ptr %ap, align 8
  %fits_in_gp441 = icmp ult i32 %gp_offset440, 41
  br i1 %fits_in_gp441, label %vaarg.in_reg442, label %vaarg.in_mem444

vaarg.in_reg442:                                  ; preds = %cond.false437
  %reg_save_area443 = load ptr, ptr %7, align 8
  %65 = zext nneg i32 %gp_offset440 to i64
  %66 = getelementptr i8, ptr %reg_save_area443, i64 %65
  %67 = add nuw nsw i32 %gp_offset440, 8
  store i32 %67, ptr %ap, align 8
  br label %vaarg.end448

vaarg.in_mem444:                                  ; preds = %cond.false437
  %overflow_arg_area446 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next447 = getelementptr i8, ptr %overflow_arg_area446, i64 8
  store ptr %overflow_arg_area.next447, ptr %overflow_arg_area_p178, align 8
  br label %vaarg.end448

vaarg.end448:                                     ; preds = %vaarg.in_mem444, %vaarg.in_reg442
  %vaarg.addr449 = phi ptr [ %66, %vaarg.in_reg442 ], [ %overflow_arg_area446, %vaarg.in_mem444 ]
  %68 = load i64, ptr %vaarg.addr449, align 8
  br label %if.end535

cond.false452:                                    ; preds = %if.else427
  %and453 = and i32 %flags.0, 64
  %tobool454.not = icmp eq i32 %and453, 0
  br i1 %tobool454.not, label %cond.false479, label %cond.true455

cond.true455:                                     ; preds = %cond.false452
  %69 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp456.not = icmp eq ptr %69, null
  br i1 %cmp456.not, label %cond.false462, label %cond.true458

cond.true458:                                     ; preds = %cond.true455
  %idxprom460 = sext i32 %nextarg.1 to i64
  %arrayidx461 = getelementptr inbounds %union.arg, ptr %69, i64 %idxprom460
  br label %cond.end475

cond.false462:                                    ; preds = %cond.true455
  %gp_offset465 = load i32, ptr %ap, align 8
  %fits_in_gp466 = icmp ult i32 %gp_offset465, 41
  br i1 %fits_in_gp466, label %vaarg.in_reg467, label %vaarg.in_mem469

vaarg.in_reg467:                                  ; preds = %cond.false462
  %reg_save_area468 = load ptr, ptr %7, align 8
  %70 = zext nneg i32 %gp_offset465 to i64
  %71 = getelementptr i8, ptr %reg_save_area468, i64 %70
  %72 = add nuw nsw i32 %gp_offset465, 8
  store i32 %72, ptr %ap, align 8
  br label %cond.end475

vaarg.in_mem469:                                  ; preds = %cond.false462
  %overflow_arg_area471 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next472 = getelementptr i8, ptr %overflow_arg_area471, i64 8
  store ptr %overflow_arg_area.next472, ptr %overflow_arg_area_p178, align 8
  br label %cond.end475

cond.end475:                                      ; preds = %vaarg.in_reg467, %vaarg.in_mem469, %cond.true458
  %cond476.in = phi ptr [ %arrayidx461, %cond.true458 ], [ %71, %vaarg.in_reg467 ], [ %overflow_arg_area471, %vaarg.in_mem469 ]
  %cond476 = load i32, ptr %cond476.in, align 4
  %73 = and i32 %cond476, 65535
  %conv478 = zext nneg i32 %73 to i64
  br label %if.end535

cond.false479:                                    ; preds = %cond.false452
  %and480 = and i32 %flags.0, 8192
  %tobool481.not = icmp eq i32 %and480, 0
  %74 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp507.not = icmp eq ptr %74, null
  br i1 %tobool481.not, label %cond.false506, label %cond.true482

cond.true482:                                     ; preds = %cond.false479
  br i1 %cmp507.not, label %cond.false489, label %cond.true485

cond.true485:                                     ; preds = %cond.true482
  %idxprom487 = sext i32 %nextarg.1 to i64
  %arrayidx488 = getelementptr inbounds %union.arg, ptr %74, i64 %idxprom487
  br label %cond.end502

cond.false489:                                    ; preds = %cond.true482
  %gp_offset492 = load i32, ptr %ap, align 8
  %fits_in_gp493 = icmp ult i32 %gp_offset492, 41
  br i1 %fits_in_gp493, label %vaarg.in_reg494, label %vaarg.in_mem496

vaarg.in_reg494:                                  ; preds = %cond.false489
  %reg_save_area495 = load ptr, ptr %7, align 8
  %75 = zext nneg i32 %gp_offset492 to i64
  %76 = getelementptr i8, ptr %reg_save_area495, i64 %75
  %77 = add nuw nsw i32 %gp_offset492, 8
  store i32 %77, ptr %ap, align 8
  br label %cond.end502

vaarg.in_mem496:                                  ; preds = %cond.false489
  %overflow_arg_area498 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next499 = getelementptr i8, ptr %overflow_arg_area498, i64 8
  store ptr %overflow_arg_area.next499, ptr %overflow_arg_area_p178, align 8
  br label %cond.end502

cond.end502:                                      ; preds = %vaarg.in_reg494, %vaarg.in_mem496, %cond.true485
  %cond503.in = phi ptr [ %arrayidx488, %cond.true485 ], [ %76, %vaarg.in_reg494 ], [ %overflow_arg_area498, %vaarg.in_mem496 ]
  %cond503 = load i32, ptr %cond503.in, align 4
  %78 = and i32 %cond503, 255
  %conv505 = zext nneg i32 %78 to i64
  br label %if.end535

cond.false506:                                    ; preds = %cond.false479
  br i1 %cmp507.not, label %cond.false513, label %cond.true509

cond.true509:                                     ; preds = %cond.false506
  %idxprom511 = sext i32 %nextarg.1 to i64
  %arrayidx512 = getelementptr inbounds %union.arg, ptr %74, i64 %idxprom511
  br label %cond.end526

cond.false513:                                    ; preds = %cond.false506
  %gp_offset516 = load i32, ptr %ap, align 8
  %fits_in_gp517 = icmp ult i32 %gp_offset516, 41
  br i1 %fits_in_gp517, label %vaarg.in_reg518, label %vaarg.in_mem520

vaarg.in_reg518:                                  ; preds = %cond.false513
  %reg_save_area519 = load ptr, ptr %7, align 8
  %79 = zext nneg i32 %gp_offset516 to i64
  %80 = getelementptr i8, ptr %reg_save_area519, i64 %79
  %81 = add nuw nsw i32 %gp_offset516, 8
  store i32 %81, ptr %ap, align 8
  br label %cond.end526

vaarg.in_mem520:                                  ; preds = %cond.false513
  %overflow_arg_area522 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next523 = getelementptr i8, ptr %overflow_arg_area522, i64 8
  store ptr %overflow_arg_area.next523, ptr %overflow_arg_area_p178, align 8
  br label %cond.end526

cond.end526:                                      ; preds = %vaarg.in_reg518, %vaarg.in_mem520, %cond.true509
  %cond527.in = phi ptr [ %arrayidx512, %cond.true509 ], [ %80, %vaarg.in_reg518 ], [ %overflow_arg_area522, %vaarg.in_mem520 ]
  %cond527 = load i32, ptr %cond527.in, align 4
  %conv528 = zext i32 %cond527 to i64
  br label %if.end535

if.end535:                                        ; preds = %vaarg.end448, %cond.true433, %cond.end502, %cond.end526, %cond.end475
  %ulval.1 = phi i64 [ %64, %cond.true433 ], [ %68, %vaarg.end448 ], [ %conv478, %cond.end475 ], [ %conv505, %cond.end502 ], [ %conv528, %cond.end526 ]
  %and536 = and i32 %flags.0, 1
  %tobool537.not = icmp eq i32 %and536, 0
  br i1 %tobool537.not, label %nosign, label %cond.false544

if.end535.thread:                                 ; preds = %cond.true330, %cond.true380, %cond.true402, %cond.true355, %vaarg.in_mem341, %vaarg.in_reg339, %vaarg.in_mem366, %vaarg.in_reg364, %vaarg.in_mem391, %vaarg.in_reg389, %vaarg.in_mem413, %vaarg.in_reg411
  %cond426.in = phi ptr [ %arrayidx333, %cond.true330 ], [ %arrayidx358, %cond.true355 ], [ %arrayidx383, %cond.true380 ], [ %arrayidx405, %cond.true402 ], [ %50, %vaarg.in_reg339 ], [ %overflow_arg_area343, %vaarg.in_mem341 ], [ %54, %vaarg.in_reg364 ], [ %overflow_arg_area368, %vaarg.in_mem366 ], [ %58, %vaarg.in_reg389 ], [ %overflow_arg_area393, %vaarg.in_mem391 ], [ %61, %vaarg.in_reg411 ], [ %overflow_arg_area415, %vaarg.in_mem413 ]
  %cond426 = load i64, ptr %cond426.in, align 8
  %and5363460 = and i32 %flags.0, 1
  %tobool537.not3461 = icmp eq i32 %and5363460, 0
  br i1 %tobool537.not3461, label %nosign, label %cond.true541

cond.true541:                                     ; preds = %if.end535.thread
  %cmp542.not = icmp eq i64 %cond426, 0
  br i1 %cmp542.not, label %nosign, label %if.then547

cond.false544:                                    ; preds = %if.end535
  %cmp545.not = icmp eq i64 %ulval.1, 0
  br i1 %cmp545.not, label %nosign, label %if.then547

if.then547:                                       ; preds = %cond.false544, %cond.true541
  %ulval.134623468 = phi i64 [ %ulval.1, %cond.false544 ], [ %ulval.0.ph, %cond.true541 ]
  %ujval.134633467 = phi i64 [ %ujval.0.ph, %cond.false544 ], [ %cond426, %cond.true541 ]
  store i32 %ch.0, ptr %arrayidx, align 4, !tbaa !5
  br label %nosign

sw.bb550:                                         ; preds = %reswitch
  %or551 = or i32 %flags.0, 16
  br label %sw.bb552

sw.bb552:                                         ; preds = %reswitch, %sw.bb550
  %flags.7 = phi i32 [ %or551, %sw.bb550 ], [ %flags.0, %reswitch ]
  %and553 = and i32 %flags.7, 16
  %tobool554.not = icmp eq i32 %and553, 0
  %82 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp579.not = icmp eq ptr %82, null
  br i1 %tobool554.not, label %if.else578, label %if.then555

if.then555:                                       ; preds = %sw.bb552
  br i1 %cmp579.not, label %cond.false562, label %cond.true558

cond.true558:                                     ; preds = %if.then555
  %idxprom560 = sext i32 %nextarg.1 to i64
  %arrayidx561 = getelementptr inbounds %union.arg, ptr %82, i64 %idxprom560
  br label %cond.end575

cond.false562:                                    ; preds = %if.then555
  %gp_offset565 = load i32, ptr %ap, align 8
  %fits_in_gp566 = icmp ult i32 %gp_offset565, 41
  br i1 %fits_in_gp566, label %vaarg.in_reg567, label %vaarg.in_mem569

vaarg.in_reg567:                                  ; preds = %cond.false562
  %reg_save_area568 = load ptr, ptr %7, align 8
  %83 = zext nneg i32 %gp_offset565 to i64
  %84 = getelementptr i8, ptr %reg_save_area568, i64 %83
  %85 = add nuw nsw i32 %gp_offset565, 8
  store i32 %85, ptr %ap, align 8
  br label %cond.end575

vaarg.in_mem569:                                  ; preds = %cond.false562
  %overflow_arg_area571 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next572 = getelementptr i8, ptr %overflow_arg_area571, i64 8
  store ptr %overflow_arg_area.next572, ptr %overflow_arg_area_p178, align 8
  br label %cond.end575

cond.end575:                                      ; preds = %vaarg.in_reg567, %vaarg.in_mem569, %cond.true558
  %cond576.in = phi ptr [ %arrayidx561, %cond.true558 ], [ %84, %vaarg.in_reg567 ], [ %overflow_arg_area571, %vaarg.in_mem569 ]
  %cond576 = load i32, ptr %cond576.in, align 4
  br label %if.end602

if.else578:                                       ; preds = %sw.bb552
  br i1 %cmp579.not, label %cond.false585, label %cond.true581

cond.true581:                                     ; preds = %if.else578
  %idxprom583 = sext i32 %nextarg.1 to i64
  %arrayidx584 = getelementptr inbounds %union.arg, ptr %82, i64 %idxprom583
  br label %cond.end598

cond.false585:                                    ; preds = %if.else578
  %gp_offset588 = load i32, ptr %ap, align 8
  %fits_in_gp589 = icmp ult i32 %gp_offset588, 41
  br i1 %fits_in_gp589, label %vaarg.in_reg590, label %vaarg.in_mem592

vaarg.in_reg590:                                  ; preds = %cond.false585
  %reg_save_area591 = load ptr, ptr %7, align 8
  %86 = zext nneg i32 %gp_offset588 to i64
  %87 = getelementptr i8, ptr %reg_save_area591, i64 %86
  %88 = add nuw nsw i32 %gp_offset588, 8
  store i32 %88, ptr %ap, align 8
  br label %cond.end598

vaarg.in_mem592:                                  ; preds = %cond.false585
  %overflow_arg_area594 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next595 = getelementptr i8, ptr %overflow_arg_area594, i64 8
  store ptr %overflow_arg_area.next595, ptr %overflow_arg_area_p178, align 8
  br label %cond.end598

cond.end598:                                      ; preds = %vaarg.in_reg590, %vaarg.in_mem592, %cond.true581
  %cond599.in = phi ptr [ %arrayidx584, %cond.true581 ], [ %87, %vaarg.in_reg590 ], [ %overflow_arg_area594, %vaarg.in_mem592 ]
  %cond599 = load i32, ptr %cond599.in, align 4
  %call600 = call i32 @btowc(i32 noundef %cond599) #14
  br label %if.end602

if.end602:                                        ; preds = %cond.end598, %cond.end575
  %storemerge3026 = phi i32 [ %call600, %cond.end598 ], [ %cond576, %cond.end575 ]
  %nextarg.17 = add nsw i32 %nextarg.1, 1
  store i32 %storemerge3026, ptr %buf, align 16, !tbaa !5
  store i32 0, ptr %sign, align 4, !tbaa !5
  br label %sw.epilog

sw.bb603:                                         ; preds = %reswitch
  %or604 = or i32 %flags.0, 16
  br label %sw.bb605

sw.bb605:                                         ; preds = %reswitch, %reswitch, %sw.bb603
  %flags.8 = phi i32 [ %or604, %sw.bb603 ], [ %flags.0, %reswitch ], [ %flags.0, %reswitch ]
  %and606 = and i32 %flags.8, 7200
  %tobool607.not = icmp eq i32 %and606, 0
  br i1 %tobool607.not, label %if.else716, label %if.then608

if.then608:                                       ; preds = %sw.bb605
  %and609 = and i32 %flags.8, 4096
  %tobool610.not = icmp eq i32 %and609, 0
  br i1 %tobool610.not, label %cond.false633, label %cond.true611

cond.true611:                                     ; preds = %if.then608
  %89 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp612.not = icmp eq ptr %89, null
  br i1 %cmp612.not, label %cond.false618, label %cond.true614

cond.true614:                                     ; preds = %cond.true611
  %idxprom616 = sext i32 %nextarg.1 to i64
  %arrayidx617 = getelementptr inbounds %union.arg, ptr %89, i64 %idxprom616
  br label %cond.end709

cond.false618:                                    ; preds = %cond.true611
  %gp_offset621 = load i32, ptr %ap, align 8
  %fits_in_gp622 = icmp ult i32 %gp_offset621, 41
  br i1 %fits_in_gp622, label %vaarg.in_reg623, label %vaarg.in_mem625

vaarg.in_reg623:                                  ; preds = %cond.false618
  %reg_save_area624 = load ptr, ptr %7, align 8
  %90 = zext nneg i32 %gp_offset621 to i64
  %91 = getelementptr i8, ptr %reg_save_area624, i64 %90
  %92 = add nuw nsw i32 %gp_offset621, 8
  store i32 %92, ptr %ap, align 8
  br label %cond.end709

vaarg.in_mem625:                                  ; preds = %cond.false618
  %overflow_arg_area627 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next628 = getelementptr i8, ptr %overflow_arg_area627, i64 8
  store ptr %overflow_arg_area.next628, ptr %overflow_arg_area_p178, align 8
  br label %cond.end709

cond.false633:                                    ; preds = %if.then608
  %and634 = and i32 %flags.8, 1024
  %tobool635.not = icmp eq i32 %and634, 0
  br i1 %tobool635.not, label %cond.false658, label %cond.true636

cond.true636:                                     ; preds = %cond.false633
  %93 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp637.not = icmp eq ptr %93, null
  br i1 %cmp637.not, label %cond.false643, label %cond.true639

cond.true639:                                     ; preds = %cond.true636
  %idxprom641 = sext i32 %nextarg.1 to i64
  %arrayidx642 = getelementptr inbounds %union.arg, ptr %93, i64 %idxprom641
  br label %cond.end709

cond.false643:                                    ; preds = %cond.true636
  %gp_offset646 = load i32, ptr %ap, align 8
  %fits_in_gp647 = icmp ult i32 %gp_offset646, 41
  br i1 %fits_in_gp647, label %vaarg.in_reg648, label %vaarg.in_mem650

vaarg.in_reg648:                                  ; preds = %cond.false643
  %reg_save_area649 = load ptr, ptr %7, align 8
  %94 = zext nneg i32 %gp_offset646 to i64
  %95 = getelementptr i8, ptr %reg_save_area649, i64 %94
  %96 = add nuw nsw i32 %gp_offset646, 8
  store i32 %96, ptr %ap, align 8
  br label %cond.end709

vaarg.in_mem650:                                  ; preds = %cond.false643
  %overflow_arg_area652 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next653 = getelementptr i8, ptr %overflow_arg_area652, i64 8
  store ptr %overflow_arg_area.next653, ptr %overflow_arg_area_p178, align 8
  br label %cond.end709

cond.false658:                                    ; preds = %cond.false633
  %and659 = and i32 %flags.8, 2048
  %tobool660.not = icmp eq i32 %and659, 0
  %97 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp684.not = icmp eq ptr %97, null
  br i1 %tobool660.not, label %cond.false683, label %cond.true661

cond.true661:                                     ; preds = %cond.false658
  br i1 %cmp684.not, label %cond.false668, label %cond.true664

cond.true664:                                     ; preds = %cond.true661
  %idxprom666 = sext i32 %nextarg.1 to i64
  %arrayidx667 = getelementptr inbounds %union.arg, ptr %97, i64 %idxprom666
  br label %cond.end709

cond.false668:                                    ; preds = %cond.true661
  %gp_offset671 = load i32, ptr %ap, align 8
  %fits_in_gp672 = icmp ult i32 %gp_offset671, 41
  br i1 %fits_in_gp672, label %vaarg.in_reg673, label %vaarg.in_mem675

vaarg.in_reg673:                                  ; preds = %cond.false668
  %reg_save_area674 = load ptr, ptr %7, align 8
  %98 = zext nneg i32 %gp_offset671 to i64
  %99 = getelementptr i8, ptr %reg_save_area674, i64 %98
  %100 = add nuw nsw i32 %gp_offset671, 8
  store i32 %100, ptr %ap, align 8
  br label %cond.end709

vaarg.in_mem675:                                  ; preds = %cond.false668
  %overflow_arg_area677 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next678 = getelementptr i8, ptr %overflow_arg_area677, i64 8
  store ptr %overflow_arg_area.next678, ptr %overflow_arg_area_p178, align 8
  br label %cond.end709

cond.false683:                                    ; preds = %cond.false658
  br i1 %cmp684.not, label %cond.false690, label %cond.true686

cond.true686:                                     ; preds = %cond.false683
  %idxprom688 = sext i32 %nextarg.1 to i64
  %arrayidx689 = getelementptr inbounds %union.arg, ptr %97, i64 %idxprom688
  br label %cond.end709

cond.false690:                                    ; preds = %cond.false683
  %gp_offset693 = load i32, ptr %ap, align 8
  %fits_in_gp694 = icmp ult i32 %gp_offset693, 41
  br i1 %fits_in_gp694, label %vaarg.in_reg695, label %vaarg.in_mem697

vaarg.in_reg695:                                  ; preds = %cond.false690
  %reg_save_area696 = load ptr, ptr %7, align 8
  %101 = zext nneg i32 %gp_offset693 to i64
  %102 = getelementptr i8, ptr %reg_save_area696, i64 %101
  %103 = add nuw nsw i32 %gp_offset693, 8
  store i32 %103, ptr %ap, align 8
  br label %cond.end709

vaarg.in_mem697:                                  ; preds = %cond.false690
  %overflow_arg_area699 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next700 = getelementptr i8, ptr %overflow_arg_area699, i64 8
  store ptr %overflow_arg_area.next700, ptr %overflow_arg_area_p178, align 8
  br label %cond.end709

cond.end709:                                      ; preds = %vaarg.in_reg695, %vaarg.in_mem697, %vaarg.in_reg673, %vaarg.in_mem675, %vaarg.in_reg648, %vaarg.in_mem650, %vaarg.in_reg623, %vaarg.in_mem625, %cond.true639, %cond.true686, %cond.true664, %cond.true614
  %cond710.in = phi ptr [ %arrayidx617, %cond.true614 ], [ %arrayidx642, %cond.true639 ], [ %arrayidx667, %cond.true664 ], [ %arrayidx689, %cond.true686 ], [ %91, %vaarg.in_reg623 ], [ %overflow_arg_area627, %vaarg.in_mem625 ], [ %95, %vaarg.in_reg648 ], [ %overflow_arg_area652, %vaarg.in_mem650 ], [ %99, %vaarg.in_reg673 ], [ %overflow_arg_area677, %vaarg.in_mem675 ], [ %102, %vaarg.in_reg695 ], [ %overflow_arg_area699, %vaarg.in_mem697 ]
  %cond710 = load i64, ptr %cond710.in, align 8
  %cmp711 = icmp slt i64 %cond710, 0
  br i1 %cmp711, label %if.then713, label %number

if.then713:                                       ; preds = %cond.end709
  %sub714 = sub i64 0, %cond710
  store i32 45, ptr %sign, align 4, !tbaa !5
  br label %number

if.else716:                                       ; preds = %sw.bb605
  %and717 = and i32 %flags.8, 16
  %tobool718.not = icmp eq i32 %and717, 0
  br i1 %tobool718.not, label %cond.false741, label %cond.true719

cond.true719:                                     ; preds = %if.else716
  %104 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp720.not = icmp eq ptr %104, null
  br i1 %cmp720.not, label %cond.false726, label %cond.true722

cond.true722:                                     ; preds = %cond.true719
  %idxprom724 = sext i32 %nextarg.1 to i64
  %arrayidx725 = getelementptr inbounds %union.arg, ptr %104, i64 %idxprom724
  %105 = load i64, ptr %arrayidx725, align 8, !tbaa !22
  br label %cond.end822

cond.false726:                                    ; preds = %cond.true719
  %gp_offset729 = load i32, ptr %ap, align 8
  %fits_in_gp730 = icmp ult i32 %gp_offset729, 41
  br i1 %fits_in_gp730, label %vaarg.in_reg731, label %vaarg.in_mem733

vaarg.in_reg731:                                  ; preds = %cond.false726
  %reg_save_area732 = load ptr, ptr %7, align 8
  %106 = zext nneg i32 %gp_offset729 to i64
  %107 = getelementptr i8, ptr %reg_save_area732, i64 %106
  %108 = add nuw nsw i32 %gp_offset729, 8
  store i32 %108, ptr %ap, align 8
  br label %vaarg.end737

vaarg.in_mem733:                                  ; preds = %cond.false726
  %overflow_arg_area735 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next736 = getelementptr i8, ptr %overflow_arg_area735, i64 8
  store ptr %overflow_arg_area.next736, ptr %overflow_arg_area_p178, align 8
  br label %vaarg.end737

vaarg.end737:                                     ; preds = %vaarg.in_mem733, %vaarg.in_reg731
  %vaarg.addr738 = phi ptr [ %107, %vaarg.in_reg731 ], [ %overflow_arg_area735, %vaarg.in_mem733 ]
  %109 = load i64, ptr %vaarg.addr738, align 8
  br label %cond.end822

cond.false741:                                    ; preds = %if.else716
  %and742 = and i32 %flags.8, 64
  %tobool743.not = icmp eq i32 %and742, 0
  br i1 %tobool743.not, label %cond.false768, label %cond.true744

cond.true744:                                     ; preds = %cond.false741
  %110 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp745.not = icmp eq ptr %110, null
  br i1 %cmp745.not, label %cond.false751, label %cond.true747

cond.true747:                                     ; preds = %cond.true744
  %idxprom749 = sext i32 %nextarg.1 to i64
  %arrayidx750 = getelementptr inbounds %union.arg, ptr %110, i64 %idxprom749
  br label %cond.end764

cond.false751:                                    ; preds = %cond.true744
  %gp_offset754 = load i32, ptr %ap, align 8
  %fits_in_gp755 = icmp ult i32 %gp_offset754, 41
  br i1 %fits_in_gp755, label %vaarg.in_reg756, label %vaarg.in_mem758

vaarg.in_reg756:                                  ; preds = %cond.false751
  %reg_save_area757 = load ptr, ptr %7, align 8
  %111 = zext nneg i32 %gp_offset754 to i64
  %112 = getelementptr i8, ptr %reg_save_area757, i64 %111
  %113 = add nuw nsw i32 %gp_offset754, 8
  store i32 %113, ptr %ap, align 8
  br label %cond.end764

vaarg.in_mem758:                                  ; preds = %cond.false751
  %overflow_arg_area760 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next761 = getelementptr i8, ptr %overflow_arg_area760, i64 8
  store ptr %overflow_arg_area.next761, ptr %overflow_arg_area_p178, align 8
  br label %cond.end764

cond.end764:                                      ; preds = %vaarg.in_reg756, %vaarg.in_mem758, %cond.true747
  %cond765.in = phi ptr [ %arrayidx750, %cond.true747 ], [ %112, %vaarg.in_reg756 ], [ %overflow_arg_area760, %vaarg.in_mem758 ]
  %cond765 = load i32, ptr %cond765.in, align 4
  %conv766 = zext i32 %cond765 to i64
  %sext3025 = shl i64 %conv766, 48
  %conv767 = ashr exact i64 %sext3025, 48
  br label %cond.end822

cond.false768:                                    ; preds = %cond.false741
  %and769 = and i32 %flags.8, 8192
  %tobool770.not = icmp eq i32 %and769, 0
  %114 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp796.not = icmp eq ptr %114, null
  br i1 %tobool770.not, label %cond.false795, label %cond.true771

cond.true771:                                     ; preds = %cond.false768
  br i1 %cmp796.not, label %cond.false778, label %cond.true774

cond.true774:                                     ; preds = %cond.true771
  %idxprom776 = sext i32 %nextarg.1 to i64
  %arrayidx777 = getelementptr inbounds %union.arg, ptr %114, i64 %idxprom776
  br label %cond.end791

cond.false778:                                    ; preds = %cond.true771
  %gp_offset781 = load i32, ptr %ap, align 8
  %fits_in_gp782 = icmp ult i32 %gp_offset781, 41
  br i1 %fits_in_gp782, label %vaarg.in_reg783, label %vaarg.in_mem785

vaarg.in_reg783:                                  ; preds = %cond.false778
  %reg_save_area784 = load ptr, ptr %7, align 8
  %115 = zext nneg i32 %gp_offset781 to i64
  %116 = getelementptr i8, ptr %reg_save_area784, i64 %115
  %117 = add nuw nsw i32 %gp_offset781, 8
  store i32 %117, ptr %ap, align 8
  br label %cond.end791

vaarg.in_mem785:                                  ; preds = %cond.false778
  %overflow_arg_area787 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next788 = getelementptr i8, ptr %overflow_arg_area787, i64 8
  store ptr %overflow_arg_area.next788, ptr %overflow_arg_area_p178, align 8
  br label %cond.end791

cond.end791:                                      ; preds = %vaarg.in_reg783, %vaarg.in_mem785, %cond.true774
  %cond792.in = phi ptr [ %arrayidx777, %cond.true774 ], [ %116, %vaarg.in_reg783 ], [ %overflow_arg_area787, %vaarg.in_mem785 ]
  %cond792 = load i32, ptr %cond792.in, align 4
  %conv793 = zext i32 %cond792 to i64
  %sext = shl i64 %conv793, 56
  %conv794 = ashr exact i64 %sext, 56
  br label %cond.end822

cond.false795:                                    ; preds = %cond.false768
  br i1 %cmp796.not, label %cond.false802, label %cond.true798

cond.true798:                                     ; preds = %cond.false795
  %idxprom800 = sext i32 %nextarg.1 to i64
  %arrayidx801 = getelementptr inbounds %union.arg, ptr %114, i64 %idxprom800
  br label %cond.end815

cond.false802:                                    ; preds = %cond.false795
  %gp_offset805 = load i32, ptr %ap, align 8
  %fits_in_gp806 = icmp ult i32 %gp_offset805, 41
  br i1 %fits_in_gp806, label %vaarg.in_reg807, label %vaarg.in_mem809

vaarg.in_reg807:                                  ; preds = %cond.false802
  %reg_save_area808 = load ptr, ptr %7, align 8
  %118 = zext nneg i32 %gp_offset805 to i64
  %119 = getelementptr i8, ptr %reg_save_area808, i64 %118
  %120 = add nuw nsw i32 %gp_offset805, 8
  store i32 %120, ptr %ap, align 8
  br label %cond.end815

vaarg.in_mem809:                                  ; preds = %cond.false802
  %overflow_arg_area811 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next812 = getelementptr i8, ptr %overflow_arg_area811, i64 8
  store ptr %overflow_arg_area.next812, ptr %overflow_arg_area_p178, align 8
  br label %cond.end815

cond.end815:                                      ; preds = %vaarg.in_reg807, %vaarg.in_mem809, %cond.true798
  %cond816.in = phi ptr [ %arrayidx801, %cond.true798 ], [ %119, %vaarg.in_reg807 ], [ %overflow_arg_area811, %vaarg.in_mem809 ]
  %cond816 = load i32, ptr %cond816.in, align 4
  %conv817 = sext i32 %cond816 to i64
  br label %cond.end822

cond.end822:                                      ; preds = %cond.end764, %cond.end815, %cond.end791, %cond.true722, %vaarg.end737
  %cond823 = phi i64 [ %105, %cond.true722 ], [ %109, %vaarg.end737 ], [ %conv767, %cond.end764 ], [ %conv794, %cond.end791 ], [ %conv817, %cond.end815 ]
  %cmp824 = icmp slt i64 %cond823, 0
  br i1 %cmp824, label %if.then826, label %number

if.then826:                                       ; preds = %cond.end822
  %sub827 = sub i64 0, %cond823
  store i32 45, ptr %sign, align 4, !tbaa !5
  br label %number

sw.bb830:                                         ; preds = %reswitch, %reswitch
  %cmp831 = icmp eq i32 %ch.0, 97
  %. = select i1 %cmp831, i32 120, i32 88
  %.3030 = select i1 %cmp831, i8 112, i8 80
  %ref___vfwprintf.xdigs_lower.ref___vfwprintf.xdigs_upper = select i1 %cmp831, ptr @ref___vfwprintf.xdigs_lower, ptr @ref___vfwprintf.xdigs_upper
  store i32 %., ptr %arrayidx, align 4, !tbaa !5
  %cmp8383541 = icmp sgt i32 %prec.1, -1
  %inc841 = zext i1 %cmp8383541 to i32
  %prec.5 = add nuw nsw i32 %prec.1, %inc841
  %and843 = and i32 %flags.0, 8
  %tobool844.not = icmp eq i32 %and843, 0
  %121 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp861.not = icmp eq ptr %121, null
  br i1 %tobool844.not, label %if.else860, label %if.then845

if.then845:                                       ; preds = %sw.bb830
  br i1 %cmp861.not, label %cond.false852, label %cond.true848

cond.true848:                                     ; preds = %if.then845
  %idxprom850 = sext i32 %nextarg.1 to i64
  %arrayidx851 = getelementptr inbounds %union.arg, ptr %121, i64 %idxprom850
  br label %cond.end857

cond.false852:                                    ; preds = %if.then845
  %overflow_arg_area855 = load ptr, ptr %overflow_arg_area_p178, align 8
  %122 = getelementptr inbounds i8, ptr %overflow_arg_area855, i64 15
  %overflow_arg_area855.aligned = call align 16 ptr @llvm.ptrmask.p0.i64(ptr nonnull %122, i64 -16)
  %overflow_arg_area.next856 = getelementptr i8, ptr %overflow_arg_area855.aligned, i64 16
  store ptr %overflow_arg_area.next856, ptr %overflow_arg_area_p178, align 8
  br label %cond.end857

cond.end857:                                      ; preds = %cond.false852, %cond.true848
  %cond858.in = phi ptr [ %arrayidx851, %cond.true848 ], [ %overflow_arg_area855.aligned, %cond.false852 ]
  %cond858 = load x86_fp80, ptr %cond858.in, align 16
  store x86_fp80 %cond858, ptr %fparg, align 16, !tbaa !21
  %call859 = call ptr @__hldtoa(x86_fp80 noundef %cond858, ptr noundef nonnull %ref___vfwprintf.xdigs_lower.ref___vfwprintf.xdigs_upper, i32 noundef %prec.5, ptr noundef nonnull %expt, ptr noundef nonnull %signflag, ptr noundef nonnull %dtoaend) #14
  br label %if.end880

if.else860:                                       ; preds = %sw.bb830
  br i1 %cmp861.not, label %cond.false867, label %cond.true863

cond.true863:                                     ; preds = %if.else860
  %idxprom865 = sext i32 %nextarg.1 to i64
  %arrayidx866 = getelementptr inbounds %union.arg, ptr %121, i64 %idxprom865
  br label %cond.end877

cond.false867:                                    ; preds = %if.else860
  %fp_offset = load i32, ptr %fp_offset_p956, align 4
  %fits_in_fp = icmp ult i32 %fp_offset, 161
  br i1 %fits_in_fp, label %vaarg.in_reg869, label %vaarg.in_mem871

vaarg.in_reg869:                                  ; preds = %cond.false867
  %reg_save_area870 = load ptr, ptr %7, align 8
  %123 = zext nneg i32 %fp_offset to i64
  %124 = getelementptr i8, ptr %reg_save_area870, i64 %123
  %125 = add nuw nsw i32 %fp_offset, 16
  store i32 %125, ptr %fp_offset_p956, align 4
  br label %cond.end877

vaarg.in_mem871:                                  ; preds = %cond.false867
  %overflow_arg_area873 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next874 = getelementptr i8, ptr %overflow_arg_area873, i64 8
  store ptr %overflow_arg_area.next874, ptr %overflow_arg_area_p178, align 8
  br label %cond.end877

cond.end877:                                      ; preds = %vaarg.in_reg869, %vaarg.in_mem871, %cond.true863
  %cond878.in = phi ptr [ %arrayidx866, %cond.true863 ], [ %124, %vaarg.in_reg869 ], [ %overflow_arg_area873, %vaarg.in_mem871 ]
  %cond878 = load double, ptr %cond878.in, align 8
  store double %cond878, ptr %fparg, align 16, !tbaa !21
  %call879 = call ptr @__hdtoa(double noundef %cond878, ptr noundef nonnull %ref___vfwprintf.xdigs_lower.ref___vfwprintf.xdigs_upper, i32 noundef %prec.5, ptr noundef nonnull %expt, ptr noundef nonnull %signflag, ptr noundef nonnull %dtoaend) #14
  br label %if.end880

if.end880:                                        ; preds = %cond.end877, %cond.end857
  %dtoaresult.0 = phi ptr [ %call859, %cond.end857 ], [ %call879, %cond.end877 ]
  %cmp881 = icmp slt i32 %prec.5, 0
  %126 = load ptr, ptr %dtoaend, align 8
  %sub.ptr.lhs.cast884 = ptrtoint ptr %126 to i64
  %sub.ptr.rhs.cast885 = ptrtoint ptr %dtoaresult.0 to i64
  %sub.ptr.sub886 = sub i64 %sub.ptr.lhs.cast884, %sub.ptr.rhs.cast885
  %conv887 = trunc i64 %sub.ptr.sub886 to i32
  %prec.6 = select i1 %cmp881, i32 %conv887, i32 %prec.5
  %127 = load i32, ptr %expt, align 4, !tbaa !5
  %cmp889 = icmp eq i32 %127, 2147483647
  br i1 %cmp889, label %if.then891, label %if.end893

if.then891:                                       ; preds = %if.end880
  store i32 0, ptr %arrayidx, align 4, !tbaa !5
  br label %if.end893

if.end893:                                        ; preds = %if.then891, %if.end880
  %cmp894.not = icmp eq ptr %convbuf.0.ph, null
  br i1 %cmp894.not, label %if.end897, label %if.then896

if.then896:                                       ; preds = %if.end893
  call void @free(ptr noundef nonnull %convbuf.0.ph)
  %.pre4390 = load ptr, ptr %dtoaend, align 8, !tbaa !27
  %.pre4395 = ptrtoint ptr %.pre4390 to i64
  %.pre4396 = sub i64 %.pre4395, %sub.ptr.rhs.cast885
  br label %if.end897

if.end897:                                        ; preds = %if.then896, %if.end893
  %sub.ptr.sub900.pre-phi = phi i64 [ %.pre4396, %if.then896 ], [ %sub.ptr.sub886, %if.end893 ]
  call void @llvm.lifetime.start.p0(i64 128, ptr nonnull %mbs.i3048) #14
  %cmp.i3049 = icmp eq ptr %dtoaresult.0, null
  br i1 %cmp.i3049, label %ref___mbsconv.exit, label %if.end.i

if.end.i:                                         ; preds = %if.end897
  %call23.i = call i64 @strlen(ptr noundef nonnull dereferenceable(1) %dtoaresult.0) #15
  %add25.i = shl i64 %call23.i, 2
  %mul.i = add i64 %add25.i, 4
  %call26.i = call noalias ptr @malloc(i64 noundef %mul.i) #16
  %cmp27.i = icmp eq ptr %call26.i, null
  br i1 %cmp27.i, label %ref___mbsconv.exit, label %if.end30.i

if.end30.i:                                       ; preds = %if.end.i
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(128) %mbs.i3048, i8 0, i64 128, i1 false)
  %cmp32.not107.i = icmp eq i64 %call23.i, 0
  br i1 %cmp32.not107.i, label %if.end54.i, label %while.body34.i

while.body34.i:                                   ; preds = %if.end30.i, %if.end45.i
  %insize.2110.i = phi i64 [ %sub.i, %if.end45.i ], [ %call23.i, %if.end30.i ]
  %p.1109.i = phi ptr [ %add.ptr46.i, %if.end45.i ], [ %dtoaresult.0, %if.end30.i ]
  %wcp.0108.i = phi ptr [ %incdec.ptr.i, %if.end45.i ], [ %call26.i, %if.end30.i ]
  %call35.i = call i64 @mbrtowc(ptr noundef nonnull %wcp.0108.i, ptr noundef %p.1109.i, i64 noundef %insize.2110.i, ptr noundef nonnull %mbs.i3048) #14
  %128 = add i64 %call35.i, 2
  %or.cond64.i = icmp ult i64 %128, 3
  br i1 %or.cond64.i, label %while.end47.i, label %if.end45.i

if.end45.i:                                       ; preds = %while.body34.i
  %incdec.ptr.i = getelementptr inbounds i32, ptr %wcp.0108.i, i64 1
  %add.ptr46.i = getelementptr inbounds i8, ptr %p.1109.i, i64 %call35.i
  %sub.i = sub i64 %insize.2110.i, %call35.i
  %cmp32.not.i = icmp eq i64 %sub.i, 0
  br i1 %cmp32.not.i, label %if.end54.i, label %while.body34.i, !llvm.loop !50

while.end47.i:                                    ; preds = %while.body34.i
  %or.cond65.i = icmp ugt i64 %call35.i, -3
  br i1 %or.cond65.i, label %if.then53.i, label %if.end54.i

if.then53.i:                                      ; preds = %while.end47.i
  call void @free(ptr noundef %call26.i)
  br label %ref___mbsconv.exit

if.end54.i:                                       ; preds = %if.end45.i, %while.end47.i, %if.end30.i
  %wcp.0101.i = phi ptr [ %wcp.0108.i, %while.end47.i ], [ %call26.i, %if.end30.i ], [ %incdec.ptr.i, %if.end45.i ]
  store i32 0, ptr %wcp.0101.i, align 4, !tbaa !5
  br label %ref___mbsconv.exit

ref___mbsconv.exit:                               ; preds = %if.end897, %if.end.i, %if.then53.i, %if.end54.i
  %retval.0.i3050 = phi ptr [ null, %if.then53.i ], [ %call26.i, %if.end54.i ], [ null, %if.end897 ], [ null, %if.end.i ]
  call void @llvm.lifetime.end.p0(i64 128, ptr nonnull %mbs.i3048) #14
  br label %fp_common

sw.bb903:                                         ; preds = %reswitch, %reswitch
  %conv904 = trunc i32 %ch.0 to i8
  %cmp905 = icmp slt i32 %prec.1, 0
  %inc909 = add nuw nsw i32 %prec.1, 1
  %spec.select3041 = select i1 %cmp905, i32 7, i32 %inc909
  br label %fp_begin

sw.bb912:                                         ; preds = %reswitch, %reswitch
  %129 = trunc i32 %ch.0 to i8
  %conv914 = add nsw i8 %129, -2
  %spec.store.select = call i32 @llvm.umax.i32(i32 %prec.1, i32 1)
  br label %fp_begin

fp_begin:                                         ; preds = %reswitch, %reswitch, %sw.bb903, %sw.bb912
  %prec.7 = phi i32 [ %spec.store.select, %sw.bb912 ], [ %spec.select3041, %sw.bb903 ], [ %prec.1, %reswitch ], [ %prec.1, %reswitch ]
  %expchar.2 = phi i8 [ %conv914, %sw.bb912 ], [ %conv904, %sw.bb903 ], [ 0, %reswitch ], [ 0, %reswitch ]
  %cmp919 = icmp slt i32 %prec.7, 0
  %spec.store.select2428 = select i1 %cmp919, i32 6, i32 %prec.7
  %cmp923.not = icmp eq ptr %convbuf.0.ph, null
  br i1 %cmp923.not, label %if.end926, label %if.then925

if.then925:                                       ; preds = %fp_begin
  call void @free(ptr noundef nonnull %convbuf.0.ph)
  br label %if.end926

if.end926:                                        ; preds = %if.then925, %fp_begin
  %and927 = and i32 %flags.0, 8
  %tobool928.not = icmp eq i32 %and927, 0
  %130 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp948.not = icmp eq ptr %130, null
  br i1 %tobool928.not, label %if.else947, label %if.then929

if.then929:                                       ; preds = %if.end926
  br i1 %cmp948.not, label %cond.false936, label %cond.true932

cond.true932:                                     ; preds = %if.then929
  %idxprom934 = sext i32 %nextarg.1 to i64
  %arrayidx935 = getelementptr inbounds %union.arg, ptr %130, i64 %idxprom934
  br label %cond.end941

cond.false936:                                    ; preds = %if.then929
  %overflow_arg_area939 = load ptr, ptr %overflow_arg_area_p178, align 8
  %131 = getelementptr inbounds i8, ptr %overflow_arg_area939, i64 15
  %overflow_arg_area939.aligned = call align 16 ptr @llvm.ptrmask.p0.i64(ptr nonnull %131, i64 -16)
  %overflow_arg_area.next940 = getelementptr i8, ptr %overflow_arg_area939.aligned, i64 16
  store ptr %overflow_arg_area.next940, ptr %overflow_arg_area_p178, align 8
  br label %cond.end941

cond.end941:                                      ; preds = %cond.false936, %cond.true932
  %cond942.in = phi ptr [ %arrayidx935, %cond.true932 ], [ %overflow_arg_area939.aligned, %cond.false936 ]
  %cond942 = load x86_fp80, ptr %cond942.in, align 16
  store x86_fp80 %cond942, ptr %fparg, align 16, !tbaa !21
  %tobool944.not = icmp eq i8 %expchar.2, 0
  %cond945 = select i1 %tobool944.not, i32 3, i32 2
  %call946 = call ptr @__ldtoa(ptr noundef nonnull %fparg, i32 noundef %cond945, i32 noundef %spec.store.select2428, ptr noundef nonnull %expt, ptr noundef nonnull %signflag, ptr noundef nonnull %dtoaend) #14
  br label %if.end977

if.else947:                                       ; preds = %if.end926
  br i1 %cmp948.not, label %cond.false954, label %cond.true950

cond.true950:                                     ; preds = %if.else947
  %idxprom952 = sext i32 %nextarg.1 to i64
  %arrayidx953 = getelementptr inbounds %union.arg, ptr %130, i64 %idxprom952
  br label %cond.end967

cond.false954:                                    ; preds = %if.else947
  %fp_offset957 = load i32, ptr %fp_offset_p956, align 4
  %fits_in_fp958 = icmp ult i32 %fp_offset957, 161
  br i1 %fits_in_fp958, label %vaarg.in_reg959, label %vaarg.in_mem961

vaarg.in_reg959:                                  ; preds = %cond.false954
  %reg_save_area960 = load ptr, ptr %7, align 8
  %132 = zext nneg i32 %fp_offset957 to i64
  %133 = getelementptr i8, ptr %reg_save_area960, i64 %132
  %134 = add nuw nsw i32 %fp_offset957, 16
  store i32 %134, ptr %fp_offset_p956, align 4
  br label %cond.end967

vaarg.in_mem961:                                  ; preds = %cond.false954
  %overflow_arg_area963 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next964 = getelementptr i8, ptr %overflow_arg_area963, i64 8
  store ptr %overflow_arg_area.next964, ptr %overflow_arg_area_p178, align 8
  br label %cond.end967

cond.end967:                                      ; preds = %vaarg.in_reg959, %vaarg.in_mem961, %cond.true950
  %cond968.in = phi ptr [ %arrayidx953, %cond.true950 ], [ %133, %vaarg.in_reg959 ], [ %overflow_arg_area963, %vaarg.in_mem961 ]
  %cond968 = load double, ptr %cond968.in, align 8
  store double %cond968, ptr %fparg, align 16, !tbaa !21
  %tobool970.not = icmp eq i8 %expchar.2, 0
  %cond971 = select i1 %tobool970.not, i32 3, i32 2
  %call972 = call ptr @__dtoa(double noundef %cond968, i32 noundef %cond971, i32 noundef %spec.store.select2428, ptr noundef nonnull %expt, ptr noundef nonnull %signflag, ptr noundef nonnull %dtoaend) #14
  %135 = load i32, ptr %expt, align 4, !tbaa !5
  %cmp973 = icmp eq i32 %135, 9999
  br i1 %cmp973, label %if.then975, label %if.end977

if.then975:                                       ; preds = %cond.end967
  store i32 2147483647, ptr %expt, align 4, !tbaa !5
  br label %if.end977

if.end977:                                        ; preds = %cond.end967, %if.then975, %cond.end941
  %dtoaresult.1 = phi ptr [ %call946, %cond.end941 ], [ %call972, %if.then975 ], [ %call972, %cond.end967 ]
  %136 = load ptr, ptr %dtoaend, align 8, !tbaa !27
  %sub.ptr.lhs.cast978 = ptrtoint ptr %136 to i64
  %sub.ptr.rhs.cast979 = ptrtoint ptr %dtoaresult.1 to i64
  %sub.ptr.sub980 = sub i64 %sub.ptr.lhs.cast978, %sub.ptr.rhs.cast979
  call void @llvm.lifetime.start.p0(i64 128, ptr nonnull %mbs.i3051) #14
  %cmp.i3052 = icmp eq ptr %dtoaresult.1, null
  br i1 %cmp.i3052, label %ref___mbsconv.exit3078, label %if.end.i3053

if.end.i3053:                                     ; preds = %if.end977
  %call23.i3054 = call i64 @strlen(ptr noundef nonnull dereferenceable(1) %dtoaresult.1) #15
  %add25.i3055 = shl i64 %call23.i3054, 2
  %mul.i3056 = add i64 %add25.i3055, 4
  %call26.i3057 = call noalias ptr @malloc(i64 noundef %mul.i3056) #16
  %cmp27.i3058 = icmp eq ptr %call26.i3057, null
  br i1 %cmp27.i3058, label %ref___mbsconv.exit3078, label %if.end30.i3059

if.end30.i3059:                                   ; preds = %if.end.i3053
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(128) %mbs.i3051, i8 0, i64 128, i1 false)
  %cmp32.not107.i3060 = icmp eq i64 %call23.i3054, 0
  br i1 %cmp32.not107.i3060, label %if.end54.i3072, label %while.body34.i3061

while.body34.i3061:                               ; preds = %if.end30.i3059, %if.end45.i3067
  %insize.2110.i3062 = phi i64 [ %sub.i3070, %if.end45.i3067 ], [ %call23.i3054, %if.end30.i3059 ]
  %p.1109.i3063 = phi ptr [ %add.ptr46.i3069, %if.end45.i3067 ], [ %dtoaresult.1, %if.end30.i3059 ]
  %wcp.0108.i3064 = phi ptr [ %incdec.ptr.i3068, %if.end45.i3067 ], [ %call26.i3057, %if.end30.i3059 ]
  %call35.i3065 = call i64 @mbrtowc(ptr noundef nonnull %wcp.0108.i3064, ptr noundef %p.1109.i3063, i64 noundef %insize.2110.i3062, ptr noundef nonnull %mbs.i3051) #14
  %137 = add i64 %call35.i3065, 2
  %or.cond64.i3066 = icmp ult i64 %137, 3
  br i1 %or.cond64.i3066, label %while.end47.i3075, label %if.end45.i3067

if.end45.i3067:                                   ; preds = %while.body34.i3061
  %incdec.ptr.i3068 = getelementptr inbounds i32, ptr %wcp.0108.i3064, i64 1
  %add.ptr46.i3069 = getelementptr inbounds i8, ptr %p.1109.i3063, i64 %call35.i3065
  %sub.i3070 = sub i64 %insize.2110.i3062, %call35.i3065
  %cmp32.not.i3071 = icmp eq i64 %sub.i3070, 0
  br i1 %cmp32.not.i3071, label %if.end54.i3072, label %while.body34.i3061, !llvm.loop !50

while.end47.i3075:                                ; preds = %while.body34.i3061
  %or.cond65.i3076 = icmp ugt i64 %call35.i3065, -3
  br i1 %or.cond65.i3076, label %if.then53.i3077, label %if.end54.i3072

if.then53.i3077:                                  ; preds = %while.end47.i3075
  call void @free(ptr noundef %call26.i3057)
  br label %ref___mbsconv.exit3078

if.end54.i3072:                                   ; preds = %if.end45.i3067, %while.end47.i3075, %if.end30.i3059
  %wcp.0101.i3073 = phi ptr [ %wcp.0108.i3064, %while.end47.i3075 ], [ %call26.i3057, %if.end30.i3059 ], [ %incdec.ptr.i3068, %if.end45.i3067 ]
  store i32 0, ptr %wcp.0101.i3073, align 4, !tbaa !5
  br label %ref___mbsconv.exit3078

ref___mbsconv.exit3078:                           ; preds = %if.end977, %if.end.i3053, %if.then53.i3077, %if.end54.i3072
  %retval.0.i3074 = phi ptr [ null, %if.then53.i3077 ], [ %call26.i3057, %if.end54.i3072 ], [ null, %if.end977 ], [ null, %if.end.i3053 ]
  call void @llvm.lifetime.end.p0(i64 128, ptr nonnull %mbs.i3051) #14
  br label %fp_common

fp_common:                                        ; preds = %ref___mbsconv.exit3078, %ref___mbsconv.exit
  %dtoaresult.1.sink = phi ptr [ %dtoaresult.1, %ref___mbsconv.exit3078 ], [ %dtoaresult.0, %ref___mbsconv.exit ]
  %prec.8 = phi i32 [ %spec.store.select2428, %ref___mbsconv.exit3078 ], [ %prec.6, %ref___mbsconv.exit ]
  %expchar.3 = phi i8 [ %expchar.2, %ref___mbsconv.exit3078 ], [ %.3030, %ref___mbsconv.exit ]
  %ndig.1.in = phi i64 [ %sub.ptr.sub980, %ref___mbsconv.exit3078 ], [ %sub.ptr.sub900.pre-phi, %ref___mbsconv.exit ]
  %xdigs.2 = phi ptr [ %xdigs.0.ph, %ref___mbsconv.exit3078 ], [ %ref___vfwprintf.xdigs_lower.ref___vfwprintf.xdigs_upper, %ref___mbsconv.exit ]
  %convbuf.1 = phi ptr [ %retval.0.i3074, %ref___mbsconv.exit3078 ], [ %retval.0.i3050, %ref___mbsconv.exit ]
  call void @__freedtoa(ptr noundef %dtoaresult.1.sink) #14
  %nextarg.30 = add nsw i32 %nextarg.1, 1
  %ndig.1 = trunc i64 %ndig.1.in to i32
  %138 = load i32, ptr %signflag, align 4, !tbaa !5
  %tobool983.not = icmp eq i32 %138, 0
  br i1 %tobool983.not, label %if.end985, label %if.then984

if.then984:                                       ; preds = %fp_common
  store i32 45, ptr %sign, align 4, !tbaa !5
  br label %if.end985

if.end985:                                        ; preds = %if.then984, %fp_common
  %139 = load i32, ptr %expt, align 4
  %cmp986 = icmp eq i32 %139, 2147483647
  br i1 %cmp986, label %if.then988, label %if.end1001

if.then988:                                       ; preds = %if.end985
  %140 = load i32, ptr %convbuf.1, align 4, !tbaa !5
  %cmp989 = icmp eq i32 %140, 78
  %cmp992 = icmp sgt i32 %ch.0, 96
  br i1 %cmp989, label %if.then991, label %if.else995

if.then991:                                       ; preds = %if.then988
  %cond994 = select i1 %cmp992, ptr @.str, ptr @.str.1
  store i32 0, ptr %sign, align 4, !tbaa !5
  br label %if.end999

if.else995:                                       ; preds = %if.then988
  %cond998 = select i1 %cmp992, ptr @.str.2, ptr @.str.3
  br label %if.end999

if.end999:                                        ; preds = %if.else995, %if.then991
  %cp.10 = phi ptr [ %cond994, %if.then991 ], [ %cond998, %if.else995 ]
  %and1000 = and i32 %flags.0, -129
  br label %sw.epilog

if.end1001:                                       ; preds = %if.end985
  %or1002 = or i32 %flags.0, 256
  %141 = and i32 %ch.0, -33
  %or.cond = icmp eq i32 %141, 71
  br i1 %or.cond, label %if.then1008, label %if.end1032

if.then1008:                                      ; preds = %if.end1001
  %cmp1009 = icmp slt i32 %139, -3
  %cmp1012.not = icmp sgt i32 %139, %prec.8
  %or.cond3031 = select i1 %cmp1009, i1 true, i1 %cmp1012.not
  %and1027 = and i32 %flags.0, 1
  %tobool1028.not = icmp eq i32 %and1027, 0
  %spec.select3032 = select i1 %tobool1028.not, i32 %ndig.1, i32 %prec.8
  br i1 %or.cond3031, label %if.end1032, label %if.end1032.thread

if.end1032.thread:                                ; preds = %if.then1008
  %prec.9 = sub nsw i32 %spec.select3032, %139
  %spec.store.select2429 = call i32 @llvm.smax.i32(i32 %prec.9, i32 0)
  br label %if.else1048

if.end1032:                                       ; preds = %if.then1008, %if.end1001
  %prec.10 = phi i32 [ %prec.8, %if.end1001 ], [ %spec.select3032, %if.then1008 ]
  %tobool1033.not = icmp eq i8 %expchar.3, 0
  br i1 %tobool1033.not, label %if.end1032.if.else1048_crit_edge, label %if.then1034

if.end1032.if.else1048_crit_edge:                 ; preds = %if.end1032
  %.pre4397 = and i32 %flags.0, 1
  br label %if.else1048

if.then1034:                                      ; preds = %if.end1032
  %sub1036 = add nsw i32 %139, -1
  %conv1037 = sext i8 %expchar.3 to i32
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %expbuf.i) #14
  store i32 %conv1037, ptr %expstr, align 16, !tbaa !5
  %cmp.i3080 = icmp slt i32 %139, 1
  %storemerge.i = select i1 %cmp.i3080, i32 45, i32 43
  %exp.addr.0.i = call i32 @llvm.abs.i32(i32 %sub1036, i1 true)
  store i32 %storemerge.i, ptr %incdec.ptr.i3079, align 4, !tbaa !5
  %cmp3.i = icmp ugt i32 %exp.addr.0.i, 9
  br i1 %cmp3.i, label %do.body.i, label %if.else14.i

do.body.i:                                        ; preds = %if.then1034, %do.body.i
  %indvars.iv47.i = phi i64 [ %indvars.iv.next48.i, %do.body.i ], [ -13, %if.then1034 ]
  %indvars.iv.i = phi i64 [ %indvars.iv.next.i, %do.body.i ], [ 16, %if.then1034 ]
  %indvar.i = phi i64 [ %indvar.next.i, %do.body.i ], [ 0, %if.then1034 ]
  %exp.addr.1.i = phi i32 [ %div.i, %do.body.i ], [ %exp.addr.0.i, %if.then1034 ]
  %t.0.idx.i = phi i64 [ %t.0.add41.i, %do.body.i ], [ 24, %if.then1034 ]
  %rem.i = urem i32 %exp.addr.1.i, 10
  %add.i3083 = or disjoint i32 %rem.i, 48
  %t.0.add41.i = add nsw i64 %t.0.idx.i, -4
  %incdec.ptr5.ptr.i = getelementptr inbounds i8, ptr %expbuf.i, i64 %t.0.add41.i
  store i32 %add.i3083, ptr %incdec.ptr5.ptr.i, align 4, !tbaa !5
  %div.i = udiv i32 %exp.addr.1.i, 10
  %cmp6.i = icmp ugt i32 %exp.addr.1.i, 99
  %indvar.next.i = add i64 %indvar.i, 1
  %indvars.iv.next.i = add i64 %indvars.iv.i, -4
  %indvars.iv.next48.i = add i64 %indvars.iv47.i, 4
  br i1 %cmp6.i, label %do.body.i, label %do.end.i, !llvm.loop !51

do.end.i:                                         ; preds = %do.body.i
  %add7.i = or disjoint i32 %div.i, 48
  %t.0.add.i = add nsw i64 %t.0.idx.i, -8
  %incdec.ptr8.ptr.i = getelementptr i8, ptr %expbuf.i, i64 %t.0.add.i
  store i32 %add7.i, ptr %incdec.ptr8.ptr.i, align 4, !tbaa !5
  %cmp1142.i = icmp slt i64 %t.0.idx.i, 32
  br i1 %cmp1142.i, label %for.inc.preheader.i, label %exponent.exit

for.inc.preheader.i:                              ; preds = %do.end.i
  %smax.i = call i64 @llvm.smax.i64(i64 %t.0.add.i, i64 20)
  %142 = shl i64 %indvar.i, 2
  %143 = add i64 %142, -13
  %144 = add i64 %143, %smax.i
  %145 = and i64 %144, -4
  %146 = add i64 %145, 4
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 8 %p.0.i, ptr nonnull align 4 %incdec.ptr8.ptr.i, i64 %146, i1 false), !tbaa !5
  %smax46.i = call i64 @llvm.smax.i64(i64 %indvars.iv.i, i64 20)
  %147 = add i64 %smax46.i, %indvars.iv47.i
  %148 = and i64 %147, -4
  %scevgep49.i = getelementptr i8, ptr %incdec.ptr18.i, i64 %148
  br label %exponent.exit

if.else14.i:                                      ; preds = %if.then1034
  %149 = and i32 %conv1037, -33
  %or.cond.i3081 = icmp eq i32 %149, 69
  br i1 %or.cond.i3081, label %if.then17.i, label %if.end19.i

if.then17.i:                                      ; preds = %if.else14.i
  store i32 48, ptr %p.0.i, align 8, !tbaa !5
  br label %if.end19.i

if.end19.i:                                       ; preds = %if.then17.i, %if.else14.i
  %p.2.i = phi ptr [ %incdec.ptr18.i, %if.then17.i ], [ %p.0.i, %if.else14.i ]
  %add20.i = or disjoint i32 %exp.addr.0.i, 48
  %incdec.ptr21.i = getelementptr inbounds i32, ptr %p.2.i, i64 1
  store i32 %add20.i, ptr %p.2.i, align 4, !tbaa !5
  br label %exponent.exit

exponent.exit:                                    ; preds = %do.end.i, %for.inc.preheader.i, %if.end19.i
  %p.3.i = phi ptr [ %incdec.ptr21.i, %if.end19.i ], [ %p.0.i, %do.end.i ], [ %scevgep49.i, %for.inc.preheader.i ]
  %sub.ptr.lhs.cast.i = ptrtoint ptr %p.3.i to i64
  %sub.ptr.sub.i = sub i64 %sub.ptr.lhs.cast.i, %sub.ptr.rhs.cast.i
  %sub.ptr.div.i = lshr exact i64 %sub.ptr.sub.i, 2
  %conv.i3082 = trunc i64 %sub.ptr.div.i to i32
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %expbuf.i) #14
  %cmp1040 = icmp sgt i32 %prec.10, 1
  %and1043 = and i32 %flags.0, 1
  %tobool1044.not = icmp ne i32 %and1043, 0
  %or.cond3033.not = select i1 %cmp1040, i1 true, i1 %tobool1044.not
  %inc1046 = zext i1 %or.cond3033.not to i32
  %add1039 = add i32 %prec.10, %inc1046
  %spec.select3042 = add i32 %add1039, %conv.i3082
  br label %sw.epilog

if.else1048:                                      ; preds = %if.end1032.if.else1048_crit_edge, %if.end1032.thread
  %and1056.pre-phi = phi i32 [ %.pre4397, %if.end1032.if.else1048_crit_edge ], [ %and1027, %if.end1032.thread ]
  %prec.103473 = phi i32 [ %prec.10, %if.end1032.if.else1048_crit_edge ], [ %spec.store.select2429, %if.end1032.thread ]
  %cmp1049 = icmp sgt i32 %139, 0
  %.3034 = select i1 %cmp1049, i32 %139, i32 1
  %tobool1054.not = icmp eq i32 %prec.103473, 0
  %tobool1057.not = icmp eq i32 %and1056.pre-phi, 0
  %or.cond3035 = select i1 %tobool1054.not, i1 %tobool1057.not, i1 false
  %add1059 = add nsw i32 %prec.103473, 1
  %add1060 = select i1 %or.cond3035, i32 0, i32 %add1059
  %size.1 = add nsw i32 %add1060, %.3034
  %and1062 = and i32 %flags.0, 512
  %tobool1063 = icmp ne i32 %and1062, 0
  %or.cond2430 = and i1 %tobool1063, %cmp1049
  br i1 %or.cond2430, label %if.then1067, label %sw.epilog

if.then1067:                                      ; preds = %if.else1048
  %call1068 = call fastcc i32 @ref_grouping_init(ptr noundef nonnull %gs, i32 noundef %139, ptr noundef %locale)
  %add1069 = add nsw i32 %call1068, %size.1
  br label %sw.epilog

sw.bb1072:                                        ; preds = %reswitch
  %and1073 = and i32 %flags.0, 32
  %tobool1074.not = icmp eq i32 %and1073, 0
  br i1 %tobool1074.not, label %if.else1098, label %if.then1075

if.then1075:                                      ; preds = %sw.bb1072
  %conv1076 = sext i32 %ret.1 to i64
  %150 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp1077.not = icmp eq ptr %150, null
  br i1 %cmp1077.not, label %cond.false1083, label %cond.true1079

cond.true1079:                                    ; preds = %if.then1075
  %idxprom1081 = sext i32 %nextarg.1 to i64
  %arrayidx1082 = getelementptr inbounds %union.arg, ptr %150, i64 %idxprom1081
  br label %cond.end1096

cond.false1083:                                   ; preds = %if.then1075
  %gp_offset1086 = load i32, ptr %ap, align 8
  %fits_in_gp1087 = icmp ult i32 %gp_offset1086, 41
  br i1 %fits_in_gp1087, label %vaarg.in_reg1088, label %vaarg.in_mem1090

vaarg.in_reg1088:                                 ; preds = %cond.false1083
  %reg_save_area1089 = load ptr, ptr %7, align 8
  %151 = zext nneg i32 %gp_offset1086 to i64
  %152 = getelementptr i8, ptr %reg_save_area1089, i64 %151
  %153 = add nuw nsw i32 %gp_offset1086, 8
  store i32 %153, ptr %ap, align 8
  br label %cond.end1096

vaarg.in_mem1090:                                 ; preds = %cond.false1083
  %overflow_arg_area1092 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1093 = getelementptr i8, ptr %overflow_arg_area1092, i64 8
  store ptr %overflow_arg_area.next1093, ptr %overflow_arg_area_p178, align 8
  br label %cond.end1096

cond.end1096:                                     ; preds = %vaarg.in_reg1088, %vaarg.in_mem1090, %cond.true1079
  %cond1097.in = phi ptr [ %arrayidx1082, %cond.true1079 ], [ %152, %vaarg.in_reg1088 ], [ %overflow_arg_area1092, %vaarg.in_mem1090 ]
  %cond1097 = load ptr, ptr %cond1097.in, align 8
  store i64 %conv1076, ptr %cond1097, align 8, !tbaa !52
  br label %if.end1282

if.else1098:                                      ; preds = %sw.bb1072
  %and1099 = and i32 %flags.0, 1024
  %tobool1100.not = icmp eq i32 %and1099, 0
  br i1 %tobool1100.not, label %if.else1124, label %if.then1101

if.then1101:                                      ; preds = %if.else1098
  %conv1102 = sext i32 %ret.1 to i64
  %154 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp1103.not = icmp eq ptr %154, null
  br i1 %cmp1103.not, label %cond.false1109, label %cond.true1105

cond.true1105:                                    ; preds = %if.then1101
  %idxprom1107 = sext i32 %nextarg.1 to i64
  %arrayidx1108 = getelementptr inbounds %union.arg, ptr %154, i64 %idxprom1107
  br label %cond.end1122

cond.false1109:                                   ; preds = %if.then1101
  %gp_offset1112 = load i32, ptr %ap, align 8
  %fits_in_gp1113 = icmp ult i32 %gp_offset1112, 41
  br i1 %fits_in_gp1113, label %vaarg.in_reg1114, label %vaarg.in_mem1116

vaarg.in_reg1114:                                 ; preds = %cond.false1109
  %reg_save_area1115 = load ptr, ptr %7, align 8
  %155 = zext nneg i32 %gp_offset1112 to i64
  %156 = getelementptr i8, ptr %reg_save_area1115, i64 %155
  %157 = add nuw nsw i32 %gp_offset1112, 8
  store i32 %157, ptr %ap, align 8
  br label %cond.end1122

vaarg.in_mem1116:                                 ; preds = %cond.false1109
  %overflow_arg_area1118 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1119 = getelementptr i8, ptr %overflow_arg_area1118, i64 8
  store ptr %overflow_arg_area.next1119, ptr %overflow_arg_area_p178, align 8
  br label %cond.end1122

cond.end1122:                                     ; preds = %vaarg.in_reg1114, %vaarg.in_mem1116, %cond.true1105
  %cond1123.in = phi ptr [ %arrayidx1108, %cond.true1105 ], [ %156, %vaarg.in_reg1114 ], [ %overflow_arg_area1118, %vaarg.in_mem1116 ]
  %cond1123 = load ptr, ptr %cond1123.in, align 8
  store i64 %conv1102, ptr %cond1123, align 8, !tbaa !22
  br label %if.end1282

if.else1124:                                      ; preds = %if.else1098
  %and1125 = and i32 %flags.0, 2048
  %tobool1126.not = icmp eq i32 %and1125, 0
  br i1 %tobool1126.not, label %if.else1150, label %if.then1127

if.then1127:                                      ; preds = %if.else1124
  %conv1128 = sext i32 %ret.1 to i64
  %158 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp1129.not = icmp eq ptr %158, null
  br i1 %cmp1129.not, label %cond.false1135, label %cond.true1131

cond.true1131:                                    ; preds = %if.then1127
  %idxprom1133 = sext i32 %nextarg.1 to i64
  %arrayidx1134 = getelementptr inbounds %union.arg, ptr %158, i64 %idxprom1133
  br label %cond.end1148

cond.false1135:                                   ; preds = %if.then1127
  %gp_offset1138 = load i32, ptr %ap, align 8
  %fits_in_gp1139 = icmp ult i32 %gp_offset1138, 41
  br i1 %fits_in_gp1139, label %vaarg.in_reg1140, label %vaarg.in_mem1142

vaarg.in_reg1140:                                 ; preds = %cond.false1135
  %reg_save_area1141 = load ptr, ptr %7, align 8
  %159 = zext nneg i32 %gp_offset1138 to i64
  %160 = getelementptr i8, ptr %reg_save_area1141, i64 %159
  %161 = add nuw nsw i32 %gp_offset1138, 8
  store i32 %161, ptr %ap, align 8
  br label %cond.end1148

vaarg.in_mem1142:                                 ; preds = %cond.false1135
  %overflow_arg_area1144 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1145 = getelementptr i8, ptr %overflow_arg_area1144, i64 8
  store ptr %overflow_arg_area.next1145, ptr %overflow_arg_area_p178, align 8
  br label %cond.end1148

cond.end1148:                                     ; preds = %vaarg.in_reg1140, %vaarg.in_mem1142, %cond.true1131
  %cond1149.in = phi ptr [ %arrayidx1134, %cond.true1131 ], [ %160, %vaarg.in_reg1140 ], [ %overflow_arg_area1144, %vaarg.in_mem1142 ]
  %cond1149 = load ptr, ptr %cond1149.in, align 8
  store i64 %conv1128, ptr %cond1149, align 8, !tbaa !22
  br label %if.end1282

if.else1150:                                      ; preds = %if.else1124
  %and1151 = and i32 %flags.0, 4096
  %tobool1152.not = icmp eq i32 %and1151, 0
  br i1 %tobool1152.not, label %if.else1176, label %if.then1153

if.then1153:                                      ; preds = %if.else1150
  %conv1154 = sext i32 %ret.1 to i64
  %162 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp1155.not = icmp eq ptr %162, null
  br i1 %cmp1155.not, label %cond.false1161, label %cond.true1157

cond.true1157:                                    ; preds = %if.then1153
  %idxprom1159 = sext i32 %nextarg.1 to i64
  %arrayidx1160 = getelementptr inbounds %union.arg, ptr %162, i64 %idxprom1159
  br label %cond.end1174

cond.false1161:                                   ; preds = %if.then1153
  %gp_offset1164 = load i32, ptr %ap, align 8
  %fits_in_gp1165 = icmp ult i32 %gp_offset1164, 41
  br i1 %fits_in_gp1165, label %vaarg.in_reg1166, label %vaarg.in_mem1168

vaarg.in_reg1166:                                 ; preds = %cond.false1161
  %reg_save_area1167 = load ptr, ptr %7, align 8
  %163 = zext nneg i32 %gp_offset1164 to i64
  %164 = getelementptr i8, ptr %reg_save_area1167, i64 %163
  %165 = add nuw nsw i32 %gp_offset1164, 8
  store i32 %165, ptr %ap, align 8
  br label %cond.end1174

vaarg.in_mem1168:                                 ; preds = %cond.false1161
  %overflow_arg_area1170 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1171 = getelementptr i8, ptr %overflow_arg_area1170, i64 8
  store ptr %overflow_arg_area.next1171, ptr %overflow_arg_area_p178, align 8
  br label %cond.end1174

cond.end1174:                                     ; preds = %vaarg.in_reg1166, %vaarg.in_mem1168, %cond.true1157
  %cond1175.in = phi ptr [ %arrayidx1160, %cond.true1157 ], [ %164, %vaarg.in_reg1166 ], [ %overflow_arg_area1170, %vaarg.in_mem1168 ]
  %cond1175 = load ptr, ptr %cond1175.in, align 8
  store i64 %conv1154, ptr %cond1175, align 8, !tbaa !22
  br label %if.end1282

if.else1176:                                      ; preds = %if.else1150
  %and1177 = and i32 %flags.0, 16
  %tobool1178.not = icmp eq i32 %and1177, 0
  br i1 %tobool1178.not, label %if.else1202, label %if.then1179

if.then1179:                                      ; preds = %if.else1176
  %conv1180 = sext i32 %ret.1 to i64
  %166 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp1181.not = icmp eq ptr %166, null
  br i1 %cmp1181.not, label %cond.false1187, label %cond.true1183

cond.true1183:                                    ; preds = %if.then1179
  %idxprom1185 = sext i32 %nextarg.1 to i64
  %arrayidx1186 = getelementptr inbounds %union.arg, ptr %166, i64 %idxprom1185
  br label %cond.end1200

cond.false1187:                                   ; preds = %if.then1179
  %gp_offset1190 = load i32, ptr %ap, align 8
  %fits_in_gp1191 = icmp ult i32 %gp_offset1190, 41
  br i1 %fits_in_gp1191, label %vaarg.in_reg1192, label %vaarg.in_mem1194

vaarg.in_reg1192:                                 ; preds = %cond.false1187
  %reg_save_area1193 = load ptr, ptr %7, align 8
  %167 = zext nneg i32 %gp_offset1190 to i64
  %168 = getelementptr i8, ptr %reg_save_area1193, i64 %167
  %169 = add nuw nsw i32 %gp_offset1190, 8
  store i32 %169, ptr %ap, align 8
  br label %cond.end1200

vaarg.in_mem1194:                                 ; preds = %cond.false1187
  %overflow_arg_area1196 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1197 = getelementptr i8, ptr %overflow_arg_area1196, i64 8
  store ptr %overflow_arg_area.next1197, ptr %overflow_arg_area_p178, align 8
  br label %cond.end1200

cond.end1200:                                     ; preds = %vaarg.in_reg1192, %vaarg.in_mem1194, %cond.true1183
  %cond1201.in = phi ptr [ %arrayidx1186, %cond.true1183 ], [ %168, %vaarg.in_reg1192 ], [ %overflow_arg_area1196, %vaarg.in_mem1194 ]
  %cond1201 = load ptr, ptr %cond1201.in, align 8
  store i64 %conv1180, ptr %cond1201, align 8, !tbaa !22
  br label %if.end1282

if.else1202:                                      ; preds = %if.else1176
  %and1203 = and i32 %flags.0, 64
  %tobool1204.not = icmp eq i32 %and1203, 0
  br i1 %tobool1204.not, label %if.else1228, label %if.then1205

if.then1205:                                      ; preds = %if.else1202
  %conv1206 = trunc i32 %ret.1 to i16
  %170 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp1207.not = icmp eq ptr %170, null
  br i1 %cmp1207.not, label %cond.false1213, label %cond.true1209

cond.true1209:                                    ; preds = %if.then1205
  %idxprom1211 = sext i32 %nextarg.1 to i64
  %arrayidx1212 = getelementptr inbounds %union.arg, ptr %170, i64 %idxprom1211
  br label %cond.end1226

cond.false1213:                                   ; preds = %if.then1205
  %gp_offset1216 = load i32, ptr %ap, align 8
  %fits_in_gp1217 = icmp ult i32 %gp_offset1216, 41
  br i1 %fits_in_gp1217, label %vaarg.in_reg1218, label %vaarg.in_mem1220

vaarg.in_reg1218:                                 ; preds = %cond.false1213
  %reg_save_area1219 = load ptr, ptr %7, align 8
  %171 = zext nneg i32 %gp_offset1216 to i64
  %172 = getelementptr i8, ptr %reg_save_area1219, i64 %171
  %173 = add nuw nsw i32 %gp_offset1216, 8
  store i32 %173, ptr %ap, align 8
  br label %cond.end1226

vaarg.in_mem1220:                                 ; preds = %cond.false1213
  %overflow_arg_area1222 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1223 = getelementptr i8, ptr %overflow_arg_area1222, i64 8
  store ptr %overflow_arg_area.next1223, ptr %overflow_arg_area_p178, align 8
  br label %cond.end1226

cond.end1226:                                     ; preds = %vaarg.in_reg1218, %vaarg.in_mem1220, %cond.true1209
  %cond1227.in = phi ptr [ %arrayidx1212, %cond.true1209 ], [ %172, %vaarg.in_reg1218 ], [ %overflow_arg_area1222, %vaarg.in_mem1220 ]
  %cond1227 = load ptr, ptr %cond1227.in, align 8
  store i16 %conv1206, ptr %cond1227, align 2, !tbaa !54
  br label %if.end1282

if.else1228:                                      ; preds = %if.else1202
  %and1229 = and i32 %flags.0, 8192
  %tobool1230.not = icmp eq i32 %and1229, 0
  br i1 %tobool1230.not, label %if.else1254, label %if.then1231

if.then1231:                                      ; preds = %if.else1228
  %conv1232 = trunc i32 %ret.1 to i8
  %174 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp1233.not = icmp eq ptr %174, null
  br i1 %cmp1233.not, label %cond.false1239, label %cond.true1235

cond.true1235:                                    ; preds = %if.then1231
  %idxprom1237 = sext i32 %nextarg.1 to i64
  %arrayidx1238 = getelementptr inbounds %union.arg, ptr %174, i64 %idxprom1237
  br label %cond.end1252

cond.false1239:                                   ; preds = %if.then1231
  %gp_offset1242 = load i32, ptr %ap, align 8
  %fits_in_gp1243 = icmp ult i32 %gp_offset1242, 41
  br i1 %fits_in_gp1243, label %vaarg.in_reg1244, label %vaarg.in_mem1246

vaarg.in_reg1244:                                 ; preds = %cond.false1239
  %reg_save_area1245 = load ptr, ptr %7, align 8
  %175 = zext nneg i32 %gp_offset1242 to i64
  %176 = getelementptr i8, ptr %reg_save_area1245, i64 %175
  %177 = add nuw nsw i32 %gp_offset1242, 8
  store i32 %177, ptr %ap, align 8
  br label %cond.end1252

vaarg.in_mem1246:                                 ; preds = %cond.false1239
  %overflow_arg_area1248 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1249 = getelementptr i8, ptr %overflow_arg_area1248, i64 8
  store ptr %overflow_arg_area.next1249, ptr %overflow_arg_area_p178, align 8
  br label %cond.end1252

cond.end1252:                                     ; preds = %vaarg.in_reg1244, %vaarg.in_mem1246, %cond.true1235
  %cond1253.in = phi ptr [ %arrayidx1238, %cond.true1235 ], [ %176, %vaarg.in_reg1244 ], [ %overflow_arg_area1248, %vaarg.in_mem1246 ]
  %cond1253 = load ptr, ptr %cond1253.in, align 8
  store i8 %conv1232, ptr %cond1253, align 1, !tbaa !21
  br label %if.end1282

if.else1254:                                      ; preds = %if.else1228
  %178 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp1255.not = icmp eq ptr %178, null
  br i1 %cmp1255.not, label %cond.false1261, label %cond.true1257

cond.true1257:                                    ; preds = %if.else1254
  %idxprom1259 = sext i32 %nextarg.1 to i64
  %arrayidx1260 = getelementptr inbounds %union.arg, ptr %178, i64 %idxprom1259
  br label %cond.end1274

cond.false1261:                                   ; preds = %if.else1254
  %gp_offset1264 = load i32, ptr %ap, align 8
  %fits_in_gp1265 = icmp ult i32 %gp_offset1264, 41
  br i1 %fits_in_gp1265, label %vaarg.in_reg1266, label %vaarg.in_mem1268

vaarg.in_reg1266:                                 ; preds = %cond.false1261
  %reg_save_area1267 = load ptr, ptr %7, align 8
  %179 = zext nneg i32 %gp_offset1264 to i64
  %180 = getelementptr i8, ptr %reg_save_area1267, i64 %179
  %181 = add nuw nsw i32 %gp_offset1264, 8
  store i32 %181, ptr %ap, align 8
  br label %cond.end1274

vaarg.in_mem1268:                                 ; preds = %cond.false1261
  %overflow_arg_area1270 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1271 = getelementptr i8, ptr %overflow_arg_area1270, i64 8
  store ptr %overflow_arg_area.next1271, ptr %overflow_arg_area_p178, align 8
  br label %cond.end1274

cond.end1274:                                     ; preds = %vaarg.in_reg1266, %vaarg.in_mem1268, %cond.true1257
  %cond1275.in = phi ptr [ %arrayidx1260, %cond.true1257 ], [ %180, %vaarg.in_reg1266 ], [ %overflow_arg_area1270, %vaarg.in_mem1268 ]
  %cond1275 = load ptr, ptr %cond1275.in, align 8
  store i32 %ret.1, ptr %cond1275, align 4, !tbaa !5
  br label %if.end1282

if.end1282:                                       ; preds = %cond.end1122, %cond.end1174, %cond.end1226, %cond.end1274, %cond.end1252, %cond.end1200, %cond.end1148, %cond.end1096
  %nextarg.39 = add nsw i32 %nextarg.1, 1
  br label %for.cond

sw.bb1283:                                        ; preds = %reswitch
  %or1284 = or i32 %flags.0, 16
  br label %sw.bb1285

sw.bb1285:                                        ; preds = %reswitch, %sw.bb1283
  %flags.9 = phi i32 [ %or1284, %sw.bb1283 ], [ %flags.0, %reswitch ]
  %and1286 = and i32 %flags.9, 7200
  %tobool1287.not = icmp eq i32 %and1286, 0
  br i1 %tobool1287.not, label %if.else1391, label %if.then1288

if.then1288:                                      ; preds = %sw.bb1285
  %and1289 = and i32 %flags.9, 4096
  %tobool1290.not = icmp eq i32 %and1289, 0
  br i1 %tobool1290.not, label %cond.false1313, label %cond.true1291

cond.true1291:                                    ; preds = %if.then1288
  %182 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp1292.not = icmp eq ptr %182, null
  br i1 %cmp1292.not, label %cond.false1298, label %cond.true1294

cond.true1294:                                    ; preds = %cond.true1291
  %idxprom1296 = sext i32 %nextarg.1 to i64
  %arrayidx1297 = getelementptr inbounds %union.arg, ptr %182, i64 %idxprom1296
  br label %cond.end1389

cond.false1298:                                   ; preds = %cond.true1291
  %gp_offset1301 = load i32, ptr %ap, align 8
  %fits_in_gp1302 = icmp ult i32 %gp_offset1301, 41
  br i1 %fits_in_gp1302, label %vaarg.in_reg1303, label %vaarg.in_mem1305

vaarg.in_reg1303:                                 ; preds = %cond.false1298
  %reg_save_area1304 = load ptr, ptr %7, align 8
  %183 = zext nneg i32 %gp_offset1301 to i64
  %184 = getelementptr i8, ptr %reg_save_area1304, i64 %183
  %185 = add nuw nsw i32 %gp_offset1301, 8
  store i32 %185, ptr %ap, align 8
  br label %cond.end1389

vaarg.in_mem1305:                                 ; preds = %cond.false1298
  %overflow_arg_area1307 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1308 = getelementptr i8, ptr %overflow_arg_area1307, i64 8
  store ptr %overflow_arg_area.next1308, ptr %overflow_arg_area_p178, align 8
  br label %cond.end1389

cond.false1313:                                   ; preds = %if.then1288
  %and1314 = and i32 %flags.9, 1024
  %tobool1315.not = icmp eq i32 %and1314, 0
  br i1 %tobool1315.not, label %cond.false1338, label %cond.true1316

cond.true1316:                                    ; preds = %cond.false1313
  %186 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp1317.not = icmp eq ptr %186, null
  br i1 %cmp1317.not, label %cond.false1323, label %cond.true1319

cond.true1319:                                    ; preds = %cond.true1316
  %idxprom1321 = sext i32 %nextarg.1 to i64
  %arrayidx1322 = getelementptr inbounds %union.arg, ptr %186, i64 %idxprom1321
  br label %cond.end1389

cond.false1323:                                   ; preds = %cond.true1316
  %gp_offset1326 = load i32, ptr %ap, align 8
  %fits_in_gp1327 = icmp ult i32 %gp_offset1326, 41
  br i1 %fits_in_gp1327, label %vaarg.in_reg1328, label %vaarg.in_mem1330

vaarg.in_reg1328:                                 ; preds = %cond.false1323
  %reg_save_area1329 = load ptr, ptr %7, align 8
  %187 = zext nneg i32 %gp_offset1326 to i64
  %188 = getelementptr i8, ptr %reg_save_area1329, i64 %187
  %189 = add nuw nsw i32 %gp_offset1326, 8
  store i32 %189, ptr %ap, align 8
  br label %cond.end1389

vaarg.in_mem1330:                                 ; preds = %cond.false1323
  %overflow_arg_area1332 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1333 = getelementptr i8, ptr %overflow_arg_area1332, i64 8
  store ptr %overflow_arg_area.next1333, ptr %overflow_arg_area_p178, align 8
  br label %cond.end1389

cond.false1338:                                   ; preds = %cond.false1313
  %and1339 = and i32 %flags.9, 2048
  %tobool1340.not = icmp eq i32 %and1339, 0
  %190 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp1364.not = icmp eq ptr %190, null
  br i1 %tobool1340.not, label %cond.false1363, label %cond.true1341

cond.true1341:                                    ; preds = %cond.false1338
  br i1 %cmp1364.not, label %cond.false1348, label %cond.true1344

cond.true1344:                                    ; preds = %cond.true1341
  %idxprom1346 = sext i32 %nextarg.1 to i64
  %arrayidx1347 = getelementptr inbounds %union.arg, ptr %190, i64 %idxprom1346
  br label %cond.end1389

cond.false1348:                                   ; preds = %cond.true1341
  %gp_offset1351 = load i32, ptr %ap, align 8
  %fits_in_gp1352 = icmp ult i32 %gp_offset1351, 41
  br i1 %fits_in_gp1352, label %vaarg.in_reg1353, label %vaarg.in_mem1355

vaarg.in_reg1353:                                 ; preds = %cond.false1348
  %reg_save_area1354 = load ptr, ptr %7, align 8
  %191 = zext nneg i32 %gp_offset1351 to i64
  %192 = getelementptr i8, ptr %reg_save_area1354, i64 %191
  %193 = add nuw nsw i32 %gp_offset1351, 8
  store i32 %193, ptr %ap, align 8
  br label %cond.end1389

vaarg.in_mem1355:                                 ; preds = %cond.false1348
  %overflow_arg_area1357 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1358 = getelementptr i8, ptr %overflow_arg_area1357, i64 8
  store ptr %overflow_arg_area.next1358, ptr %overflow_arg_area_p178, align 8
  br label %cond.end1389

cond.false1363:                                   ; preds = %cond.false1338
  br i1 %cmp1364.not, label %cond.false1370, label %cond.true1366

cond.true1366:                                    ; preds = %cond.false1363
  %idxprom1368 = sext i32 %nextarg.1 to i64
  %arrayidx1369 = getelementptr inbounds %union.arg, ptr %190, i64 %idxprom1368
  br label %cond.end1389

cond.false1370:                                   ; preds = %cond.false1363
  %gp_offset1373 = load i32, ptr %ap, align 8
  %fits_in_gp1374 = icmp ult i32 %gp_offset1373, 41
  br i1 %fits_in_gp1374, label %vaarg.in_reg1375, label %vaarg.in_mem1377

vaarg.in_reg1375:                                 ; preds = %cond.false1370
  %reg_save_area1376 = load ptr, ptr %7, align 8
  %194 = zext nneg i32 %gp_offset1373 to i64
  %195 = getelementptr i8, ptr %reg_save_area1376, i64 %194
  %196 = add nuw nsw i32 %gp_offset1373, 8
  store i32 %196, ptr %ap, align 8
  br label %cond.end1389

vaarg.in_mem1377:                                 ; preds = %cond.false1370
  %overflow_arg_area1379 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1380 = getelementptr i8, ptr %overflow_arg_area1379, i64 8
  store ptr %overflow_arg_area.next1380, ptr %overflow_arg_area_p178, align 8
  br label %cond.end1389

cond.end1389:                                     ; preds = %vaarg.in_reg1375, %vaarg.in_mem1377, %vaarg.in_reg1353, %vaarg.in_mem1355, %vaarg.in_reg1328, %vaarg.in_mem1330, %vaarg.in_reg1303, %vaarg.in_mem1305, %cond.true1319, %cond.true1366, %cond.true1344, %cond.true1294
  %cond1390.in = phi ptr [ %arrayidx1297, %cond.true1294 ], [ %arrayidx1322, %cond.true1319 ], [ %arrayidx1347, %cond.true1344 ], [ %arrayidx1369, %cond.true1366 ], [ %184, %vaarg.in_reg1303 ], [ %overflow_arg_area1307, %vaarg.in_mem1305 ], [ %188, %vaarg.in_reg1328 ], [ %overflow_arg_area1332, %vaarg.in_mem1330 ], [ %192, %vaarg.in_reg1353 ], [ %overflow_arg_area1357, %vaarg.in_mem1355 ], [ %195, %vaarg.in_reg1375 ], [ %overflow_arg_area1379, %vaarg.in_mem1377 ]
  %cond1390 = load i64, ptr %cond1390.in, align 8
  br label %nosign

if.else1391:                                      ; preds = %sw.bb1285
  %and1392 = and i32 %flags.9, 16
  %tobool1393.not = icmp eq i32 %and1392, 0
  br i1 %tobool1393.not, label %cond.false1416, label %cond.true1394

cond.true1394:                                    ; preds = %if.else1391
  %197 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp1395.not = icmp eq ptr %197, null
  br i1 %cmp1395.not, label %cond.false1401, label %cond.true1397

cond.true1397:                                    ; preds = %cond.true1394
  %idxprom1399 = sext i32 %nextarg.1 to i64
  %arrayidx1400 = getelementptr inbounds %union.arg, ptr %197, i64 %idxprom1399
  %198 = load i64, ptr %arrayidx1400, align 8, !tbaa !22
  br label %nosign

cond.false1401:                                   ; preds = %cond.true1394
  %gp_offset1404 = load i32, ptr %ap, align 8
  %fits_in_gp1405 = icmp ult i32 %gp_offset1404, 41
  br i1 %fits_in_gp1405, label %vaarg.in_reg1406, label %vaarg.in_mem1408

vaarg.in_reg1406:                                 ; preds = %cond.false1401
  %reg_save_area1407 = load ptr, ptr %7, align 8
  %199 = zext nneg i32 %gp_offset1404 to i64
  %200 = getelementptr i8, ptr %reg_save_area1407, i64 %199
  %201 = add nuw nsw i32 %gp_offset1404, 8
  store i32 %201, ptr %ap, align 8
  br label %vaarg.end1412

vaarg.in_mem1408:                                 ; preds = %cond.false1401
  %overflow_arg_area1410 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1411 = getelementptr i8, ptr %overflow_arg_area1410, i64 8
  store ptr %overflow_arg_area.next1411, ptr %overflow_arg_area_p178, align 8
  br label %vaarg.end1412

vaarg.end1412:                                    ; preds = %vaarg.in_mem1408, %vaarg.in_reg1406
  %vaarg.addr1413 = phi ptr [ %200, %vaarg.in_reg1406 ], [ %overflow_arg_area1410, %vaarg.in_mem1408 ]
  %202 = load i64, ptr %vaarg.addr1413, align 8
  br label %nosign

cond.false1416:                                   ; preds = %if.else1391
  %and1417 = and i32 %flags.9, 64
  %tobool1418.not = icmp eq i32 %and1417, 0
  br i1 %tobool1418.not, label %cond.false1443, label %cond.true1419

cond.true1419:                                    ; preds = %cond.false1416
  %203 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp1420.not = icmp eq ptr %203, null
  br i1 %cmp1420.not, label %cond.false1426, label %cond.true1422

cond.true1422:                                    ; preds = %cond.true1419
  %idxprom1424 = sext i32 %nextarg.1 to i64
  %arrayidx1425 = getelementptr inbounds %union.arg, ptr %203, i64 %idxprom1424
  br label %cond.end1439

cond.false1426:                                   ; preds = %cond.true1419
  %gp_offset1429 = load i32, ptr %ap, align 8
  %fits_in_gp1430 = icmp ult i32 %gp_offset1429, 41
  br i1 %fits_in_gp1430, label %vaarg.in_reg1431, label %vaarg.in_mem1433

vaarg.in_reg1431:                                 ; preds = %cond.false1426
  %reg_save_area1432 = load ptr, ptr %7, align 8
  %204 = zext nneg i32 %gp_offset1429 to i64
  %205 = getelementptr i8, ptr %reg_save_area1432, i64 %204
  %206 = add nuw nsw i32 %gp_offset1429, 8
  store i32 %206, ptr %ap, align 8
  br label %cond.end1439

vaarg.in_mem1433:                                 ; preds = %cond.false1426
  %overflow_arg_area1435 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1436 = getelementptr i8, ptr %overflow_arg_area1435, i64 8
  store ptr %overflow_arg_area.next1436, ptr %overflow_arg_area_p178, align 8
  br label %cond.end1439

cond.end1439:                                     ; preds = %vaarg.in_reg1431, %vaarg.in_mem1433, %cond.true1422
  %cond1440.in = phi ptr [ %arrayidx1425, %cond.true1422 ], [ %205, %vaarg.in_reg1431 ], [ %overflow_arg_area1435, %vaarg.in_mem1433 ]
  %cond1440 = load i32, ptr %cond1440.in, align 4
  %207 = and i32 %cond1440, 65535
  %conv1442 = zext nneg i32 %207 to i64
  br label %nosign

cond.false1443:                                   ; preds = %cond.false1416
  %and1444 = and i32 %flags.9, 8192
  %tobool1445.not = icmp eq i32 %and1444, 0
  %208 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp1471.not = icmp eq ptr %208, null
  br i1 %tobool1445.not, label %cond.false1470, label %cond.true1446

cond.true1446:                                    ; preds = %cond.false1443
  br i1 %cmp1471.not, label %cond.false1453, label %cond.true1449

cond.true1449:                                    ; preds = %cond.true1446
  %idxprom1451 = sext i32 %nextarg.1 to i64
  %arrayidx1452 = getelementptr inbounds %union.arg, ptr %208, i64 %idxprom1451
  br label %cond.end1466

cond.false1453:                                   ; preds = %cond.true1446
  %gp_offset1456 = load i32, ptr %ap, align 8
  %fits_in_gp1457 = icmp ult i32 %gp_offset1456, 41
  br i1 %fits_in_gp1457, label %vaarg.in_reg1458, label %vaarg.in_mem1460

vaarg.in_reg1458:                                 ; preds = %cond.false1453
  %reg_save_area1459 = load ptr, ptr %7, align 8
  %209 = zext nneg i32 %gp_offset1456 to i64
  %210 = getelementptr i8, ptr %reg_save_area1459, i64 %209
  %211 = add nuw nsw i32 %gp_offset1456, 8
  store i32 %211, ptr %ap, align 8
  br label %cond.end1466

vaarg.in_mem1460:                                 ; preds = %cond.false1453
  %overflow_arg_area1462 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1463 = getelementptr i8, ptr %overflow_arg_area1462, i64 8
  store ptr %overflow_arg_area.next1463, ptr %overflow_arg_area_p178, align 8
  br label %cond.end1466

cond.end1466:                                     ; preds = %vaarg.in_reg1458, %vaarg.in_mem1460, %cond.true1449
  %cond1467.in = phi ptr [ %arrayidx1452, %cond.true1449 ], [ %210, %vaarg.in_reg1458 ], [ %overflow_arg_area1462, %vaarg.in_mem1460 ]
  %cond1467 = load i32, ptr %cond1467.in, align 4
  %212 = and i32 %cond1467, 255
  %conv1469 = zext nneg i32 %212 to i64
  br label %nosign

cond.false1470:                                   ; preds = %cond.false1443
  br i1 %cmp1471.not, label %cond.false1477, label %cond.true1473

cond.true1473:                                    ; preds = %cond.false1470
  %idxprom1475 = sext i32 %nextarg.1 to i64
  %arrayidx1476 = getelementptr inbounds %union.arg, ptr %208, i64 %idxprom1475
  br label %cond.end1490

cond.false1477:                                   ; preds = %cond.false1470
  %gp_offset1480 = load i32, ptr %ap, align 8
  %fits_in_gp1481 = icmp ult i32 %gp_offset1480, 41
  br i1 %fits_in_gp1481, label %vaarg.in_reg1482, label %vaarg.in_mem1484

vaarg.in_reg1482:                                 ; preds = %cond.false1477
  %reg_save_area1483 = load ptr, ptr %7, align 8
  %213 = zext nneg i32 %gp_offset1480 to i64
  %214 = getelementptr i8, ptr %reg_save_area1483, i64 %213
  %215 = add nuw nsw i32 %gp_offset1480, 8
  store i32 %215, ptr %ap, align 8
  br label %cond.end1490

vaarg.in_mem1484:                                 ; preds = %cond.false1477
  %overflow_arg_area1486 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1487 = getelementptr i8, ptr %overflow_arg_area1486, i64 8
  store ptr %overflow_arg_area.next1487, ptr %overflow_arg_area_p178, align 8
  br label %cond.end1490

cond.end1490:                                     ; preds = %vaarg.in_reg1482, %vaarg.in_mem1484, %cond.true1473
  %cond1491.in = phi ptr [ %arrayidx1476, %cond.true1473 ], [ %214, %vaarg.in_reg1482 ], [ %overflow_arg_area1486, %vaarg.in_mem1484 ]
  %cond1491 = load i32, ptr %cond1491.in, align 4
  %conv1492 = zext i32 %cond1491 to i64
  br label %nosign

sw.bb1500:                                        ; preds = %reswitch
  %216 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp1501.not = icmp eq ptr %216, null
  br i1 %cmp1501.not, label %cond.false1507, label %cond.true1503

cond.true1503:                                    ; preds = %sw.bb1500
  %idxprom1505 = sext i32 %nextarg.1 to i64
  %arrayidx1506 = getelementptr inbounds %union.arg, ptr %216, i64 %idxprom1505
  br label %cond.end1520

cond.false1507:                                   ; preds = %sw.bb1500
  %gp_offset1510 = load i32, ptr %ap, align 8
  %fits_in_gp1511 = icmp ult i32 %gp_offset1510, 41
  br i1 %fits_in_gp1511, label %vaarg.in_reg1512, label %vaarg.in_mem1514

vaarg.in_reg1512:                                 ; preds = %cond.false1507
  %reg_save_area1513 = load ptr, ptr %7, align 8
  %217 = zext nneg i32 %gp_offset1510 to i64
  %218 = getelementptr i8, ptr %reg_save_area1513, i64 %217
  %219 = add nuw nsw i32 %gp_offset1510, 8
  store i32 %219, ptr %ap, align 8
  br label %cond.end1520

vaarg.in_mem1514:                                 ; preds = %cond.false1507
  %overflow_arg_area1516 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1517 = getelementptr i8, ptr %overflow_arg_area1516, i64 8
  store ptr %overflow_arg_area.next1517, ptr %overflow_arg_area_p178, align 8
  br label %cond.end1520

cond.end1520:                                     ; preds = %vaarg.in_reg1512, %vaarg.in_mem1514, %cond.true1503
  %cond1521.in = phi ptr [ %arrayidx1506, %cond.true1503 ], [ %218, %vaarg.in_reg1512 ], [ %overflow_arg_area1516, %vaarg.in_mem1514 ]
  %cond1521 = load ptr, ptr %cond1521.in, align 8
  %220 = ptrtoint ptr %cond1521 to i64
  %or1522 = or i32 %flags.0, 4096
  store i32 120, ptr %arrayidx, align 4, !tbaa !5
  br label %nosign

sw.bb1524:                                        ; preds = %reswitch
  %or1525 = or i32 %flags.0, 16
  br label %sw.bb1526

sw.bb1526:                                        ; preds = %reswitch, %sw.bb1524
  %flags.10 = phi i32 [ %or1525, %sw.bb1524 ], [ %flags.0, %reswitch ]
  %and1527 = and i32 %flags.10, 16
  %tobool1528.not = icmp eq i32 %and1527, 0
  br i1 %tobool1528.not, label %if.else1555, label %if.then1529

if.then1529:                                      ; preds = %sw.bb1526
  %221 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp1530.not = icmp eq ptr %221, null
  br i1 %cmp1530.not, label %cond.false1536, label %cond.true1532

cond.true1532:                                    ; preds = %if.then1529
  %idxprom1534 = sext i32 %nextarg.1 to i64
  %arrayidx1535 = getelementptr inbounds %union.arg, ptr %221, i64 %idxprom1534
  br label %cond.end1549

cond.false1536:                                   ; preds = %if.then1529
  %gp_offset1539 = load i32, ptr %ap, align 8
  %fits_in_gp1540 = icmp ult i32 %gp_offset1539, 41
  br i1 %fits_in_gp1540, label %vaarg.in_reg1541, label %vaarg.in_mem1543

vaarg.in_reg1541:                                 ; preds = %cond.false1536
  %reg_save_area1542 = load ptr, ptr %7, align 8
  %222 = zext nneg i32 %gp_offset1539 to i64
  %223 = getelementptr i8, ptr %reg_save_area1542, i64 %222
  %224 = add nuw nsw i32 %gp_offset1539, 8
  store i32 %224, ptr %ap, align 8
  br label %cond.end1549

vaarg.in_mem1543:                                 ; preds = %cond.false1536
  %overflow_arg_area1545 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1546 = getelementptr i8, ptr %overflow_arg_area1545, i64 8
  store ptr %overflow_arg_area.next1546, ptr %overflow_arg_area_p178, align 8
  br label %cond.end1549

cond.end1549:                                     ; preds = %vaarg.in_reg1541, %vaarg.in_mem1543, %cond.true1532
  %cond1550.in = phi ptr [ %arrayidx1535, %cond.true1532 ], [ %223, %vaarg.in_reg1541 ], [ %overflow_arg_area1545, %vaarg.in_mem1543 ]
  %cond1550 = load ptr, ptr %cond1550.in, align 8
  %cmp1551 = icmp eq ptr %cond1550, null
  %spec.store.select2431 = select i1 %cmp1551, ptr @.str.4, ptr %cond1550
  br label %if.end1598

if.else1555:                                      ; preds = %sw.bb1526
  %cmp1556.not = icmp eq ptr %convbuf.0.ph, null
  br i1 %cmp1556.not, label %if.end1559, label %if.then1558

if.then1558:                                      ; preds = %if.else1555
  call void @free(ptr noundef nonnull %convbuf.0.ph)
  br label %if.end1559

if.end1559:                                       ; preds = %if.then1558, %if.else1555
  %225 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp1560.not = icmp eq ptr %225, null
  br i1 %cmp1560.not, label %cond.false1566, label %cond.true1562

cond.true1562:                                    ; preds = %if.end1559
  %idxprom1564 = sext i32 %nextarg.1 to i64
  %arrayidx1565 = getelementptr inbounds %union.arg, ptr %225, i64 %idxprom1564
  br label %cond.end1579

cond.false1566:                                   ; preds = %if.end1559
  %gp_offset1569 = load i32, ptr %ap, align 8
  %fits_in_gp1570 = icmp ult i32 %gp_offset1569, 41
  br i1 %fits_in_gp1570, label %vaarg.in_reg1571, label %vaarg.in_mem1573

vaarg.in_reg1571:                                 ; preds = %cond.false1566
  %reg_save_area1572 = load ptr, ptr %7, align 8
  %226 = zext nneg i32 %gp_offset1569 to i64
  %227 = getelementptr i8, ptr %reg_save_area1572, i64 %226
  %228 = add nuw nsw i32 %gp_offset1569, 8
  store i32 %228, ptr %ap, align 8
  br label %cond.end1579

vaarg.in_mem1573:                                 ; preds = %cond.false1566
  %overflow_arg_area1575 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1576 = getelementptr i8, ptr %overflow_arg_area1575, i64 8
  store ptr %overflow_arg_area.next1576, ptr %overflow_arg_area_p178, align 8
  br label %cond.end1579

cond.end1579:                                     ; preds = %vaarg.in_reg1571, %vaarg.in_mem1573, %cond.true1562
  %cond1580.in = phi ptr [ %arrayidx1565, %cond.true1562 ], [ %227, %vaarg.in_reg1571 ], [ %overflow_arg_area1575, %vaarg.in_mem1573 ]
  %cond1580 = load ptr, ptr %cond1580.in, align 8
  %cmp1581 = icmp eq ptr %cond1580, null
  br i1 %cmp1581, label %if.end1598, label %if.end.i3086

if.end.i3086:                                     ; preds = %cond.end1579
  call void @llvm.lifetime.start.p0(i64 128, ptr nonnull %mbs.i3084) #14
  %cmp1.i = icmp sgt i32 %prec.1, -1
  br i1 %cmp1.i, label %if.then2.i, label %if.else.i

if.then2.i:                                       ; preds = %if.end.i3086
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(128) %mbs.i3084, i8 0, i64 128, i1 false)
  %conv.i3111 = zext nneg i32 %prec.1 to i64
  %cmp3.not103.i = icmp eq i32 %prec.1, 0
  br i1 %cmp3.not103.i, label %if.end24.i, label %while.body.i

while.body.i:                                     ; preds = %if.then2.i, %if.end15.i
  %nchars.0106.i = phi i64 [ %inc.i3113, %if.end15.i ], [ 0, %if.then2.i ]
  %insize.0105.i = phi i64 [ %add.i3114, %if.end15.i ], [ 0, %if.then2.i ]
  %p.0104.i = phi ptr [ %add.ptr.i, %if.end15.i ], [ %cond1580, %if.then2.i ]
  %call.i3112 = call i32 @___mb_cur_max() #14
  %conv5.i = sext i32 %call.i3112 to i64
  %call6.i = call i64 @mbrlen(ptr noundef %p.0104.i, i64 noundef %conv5.i, ptr noundef nonnull %mbs.i3084) #14
  %229 = add i64 %call6.i, 2
  %or.cond61.i = icmp ult i64 %229, 3
  br i1 %or.cond61.i, label %while.end.i, label %if.end15.i

if.end15.i:                                       ; preds = %while.body.i
  %add.ptr.i = getelementptr inbounds i8, ptr %p.0104.i, i64 %call6.i
  %inc.i3113 = add nuw nsw i64 %nchars.0106.i, 1
  %add.i3114 = add i64 %call6.i, %insize.0105.i
  %cmp3.not.i = icmp eq i64 %inc.i3113, %conv.i3111
  br i1 %cmp3.not.i, label %if.end24.i, label %while.body.i, !llvm.loop !55

while.end.i:                                      ; preds = %while.body.i
  %or.cond62.i = icmp ugt i64 %call6.i, -3
  br i1 %or.cond62.i, label %ref___mbsconv.exit3115.thread, label %if.end24.i

if.else.i:                                        ; preds = %if.end.i3086
  %call23.i3087 = call i64 @strlen(ptr noundef nonnull dereferenceable(1) %cond1580) #15
  br label %if.end24.i

if.end24.i:                                       ; preds = %if.end15.i, %if.else.i, %while.end.i, %if.then2.i
  %insize.1.i = phi i64 [ %insize.0105.i, %while.end.i ], [ %call23.i3087, %if.else.i ], [ 0, %if.then2.i ], [ %add.i3114, %if.end15.i ]
  %add25.i3088 = shl i64 %insize.1.i, 2
  %mul.i3089 = add i64 %add25.i3088, 4
  %call26.i3090 = call noalias ptr @malloc(i64 noundef %mul.i3089) #16
  %cmp27.i3091 = icmp eq ptr %call26.i3090, null
  br i1 %cmp27.i3091, label %ref___mbsconv.exit3115.thread, label %if.end30.i3092

if.end30.i3092:                                   ; preds = %if.end24.i
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(128) %mbs.i3084, i8 0, i64 128, i1 false)
  %cmp32.not107.i3093 = icmp eq i64 %insize.1.i, 0
  br i1 %cmp32.not107.i3093, label %ref___mbsconv.exit3115.thread3476, label %while.body34.i3094

ref___mbsconv.exit3115.thread3476:                ; preds = %if.end30.i3092
  store i32 0, ptr %call26.i3090, align 4, !tbaa !5
  call void @llvm.lifetime.end.p0(i64 128, ptr nonnull %mbs.i3084) #14
  br label %if.end1598

while.body34.i3094:                               ; preds = %if.end30.i3092, %if.end45.i3100
  %insize.2110.i3095 = phi i64 [ %sub.i3103, %if.end45.i3100 ], [ %insize.1.i, %if.end30.i3092 ]
  %p.1109.i3096 = phi ptr [ %add.ptr46.i3102, %if.end45.i3100 ], [ %cond1580, %if.end30.i3092 ]
  %wcp.0108.i3097 = phi ptr [ %incdec.ptr.i3101, %if.end45.i3100 ], [ %call26.i3090, %if.end30.i3092 ]
  %call35.i3098 = call i64 @mbrtowc(ptr noundef nonnull %wcp.0108.i3097, ptr noundef %p.1109.i3096, i64 noundef %insize.2110.i3095, ptr noundef nonnull %mbs.i3084) #14
  %230 = add i64 %call35.i3098, 2
  %or.cond64.i3099 = icmp ult i64 %230, 3
  br i1 %or.cond64.i3099, label %while.end47.i3108, label %if.end45.i3100

if.end45.i3100:                                   ; preds = %while.body34.i3094
  %incdec.ptr.i3101 = getelementptr inbounds i32, ptr %wcp.0108.i3097, i64 1
  %add.ptr46.i3102 = getelementptr inbounds i8, ptr %p.1109.i3096, i64 %call35.i3098
  %sub.i3103 = sub i64 %insize.2110.i3095, %call35.i3098
  %cmp32.not.i3104 = icmp eq i64 %sub.i3103, 0
  br i1 %cmp32.not.i3104, label %ref___mbsconv.exit3115, label %while.body34.i3094, !llvm.loop !50

while.end47.i3108:                                ; preds = %while.body34.i3094
  %or.cond65.i3109 = icmp ugt i64 %call35.i3098, -3
  br i1 %or.cond65.i3109, label %if.then53.i3110, label %ref___mbsconv.exit3115

if.then53.i3110:                                  ; preds = %while.end47.i3108
  call void @free(ptr noundef %call26.i3090)
  br label %ref___mbsconv.exit3115.thread

ref___mbsconv.exit3115.thread:                    ; preds = %while.end.i, %if.end24.i, %if.then53.i3110
  call void @llvm.lifetime.end.p0(i64 128, ptr nonnull %mbs.i3084) #14
  %231 = load i16, ptr %_flags, align 8, !tbaa !9
  %232 = or i16 %231, 64
  store i16 %232, ptr %_flags, align 8, !tbaa !9
  call void @llvm.va_end(ptr nonnull %orgap)
  br label %if.end2369

ref___mbsconv.exit3115:                           ; preds = %if.end45.i3100, %while.end47.i3108
  %wcp.0101.i3106 = phi ptr [ %wcp.0108.i3097, %while.end47.i3108 ], [ %incdec.ptr.i3101, %if.end45.i3100 ]
  store i32 0, ptr %wcp.0101.i3106, align 4, !tbaa !5
  call void @llvm.lifetime.end.p0(i64 128, ptr nonnull %mbs.i3084) #14
  br label %if.end1598

if.end1598:                                       ; preds = %ref___mbsconv.exit3115, %ref___mbsconv.exit3115.thread3476, %cond.end1579, %cond.end1549
  %cp.13 = phi ptr [ %spec.store.select2431, %cond.end1549 ], [ %call26.i3090, %ref___mbsconv.exit3115.thread3476 ], [ %call26.i3090, %ref___mbsconv.exit3115 ], [ @.str.4, %cond.end1579 ]
  %convbuf.4 = phi ptr [ %convbuf.0.ph, %cond.end1549 ], [ %call26.i3090, %ref___mbsconv.exit3115.thread3476 ], [ %call26.i3090, %ref___mbsconv.exit3115 ], [ %convbuf.0.ph, %cond.end1579 ]
  %nextarg.49 = add nsw i32 %nextarg.1, 1
  %cmp1599 = icmp sgt i32 %prec.1, -1
  br i1 %cmp1599, label %cond.true1601, label %cond.false1604

cond.true1601:                                    ; preds = %if.end1598
  %conv1602 = zext nneg i32 %prec.1 to i64
  %call1603 = call i64 @wcsnlen(ptr noundef nonnull %cp.13, i64 noundef %conv1602) #15
  br label %cond.end1606

cond.false1604:                                   ; preds = %if.end1598
  %call1605 = call i64 @wcslen(ptr noundef nonnull %cp.13) #15
  br label %cond.end1606

cond.end1606:                                     ; preds = %cond.false1604, %cond.true1601
  %cond1607 = phi i64 [ %call1603, %cond.true1601 ], [ %call1605, %cond.false1604 ]
  %conv1608 = trunc i64 %cond1607 to i32
  store i32 0, ptr %sign, align 4, !tbaa !5
  br label %sw.epilog

sw.bb1609:                                        ; preds = %reswitch
  %or1610 = or i32 %flags.0, 16
  br label %sw.bb1611

sw.bb1611:                                        ; preds = %reswitch, %sw.bb1609
  %flags.11 = phi i32 [ %or1610, %sw.bb1609 ], [ %flags.0, %reswitch ]
  %and1612 = and i32 %flags.11, 7200
  %tobool1613.not = icmp eq i32 %and1612, 0
  br i1 %tobool1613.not, label %if.else1717, label %if.then1614

if.then1614:                                      ; preds = %sw.bb1611
  %and1615 = and i32 %flags.11, 4096
  %tobool1616.not = icmp eq i32 %and1615, 0
  br i1 %tobool1616.not, label %cond.false1639, label %cond.true1617

cond.true1617:                                    ; preds = %if.then1614
  %233 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp1618.not = icmp eq ptr %233, null
  br i1 %cmp1618.not, label %cond.false1624, label %cond.true1620

cond.true1620:                                    ; preds = %cond.true1617
  %idxprom1622 = sext i32 %nextarg.1 to i64
  %arrayidx1623 = getelementptr inbounds %union.arg, ptr %233, i64 %idxprom1622
  br label %cond.end1715

cond.false1624:                                   ; preds = %cond.true1617
  %gp_offset1627 = load i32, ptr %ap, align 8
  %fits_in_gp1628 = icmp ult i32 %gp_offset1627, 41
  br i1 %fits_in_gp1628, label %vaarg.in_reg1629, label %vaarg.in_mem1631

vaarg.in_reg1629:                                 ; preds = %cond.false1624
  %reg_save_area1630 = load ptr, ptr %7, align 8
  %234 = zext nneg i32 %gp_offset1627 to i64
  %235 = getelementptr i8, ptr %reg_save_area1630, i64 %234
  %236 = add nuw nsw i32 %gp_offset1627, 8
  store i32 %236, ptr %ap, align 8
  br label %cond.end1715

vaarg.in_mem1631:                                 ; preds = %cond.false1624
  %overflow_arg_area1633 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1634 = getelementptr i8, ptr %overflow_arg_area1633, i64 8
  store ptr %overflow_arg_area.next1634, ptr %overflow_arg_area_p178, align 8
  br label %cond.end1715

cond.false1639:                                   ; preds = %if.then1614
  %and1640 = and i32 %flags.11, 1024
  %tobool1641.not = icmp eq i32 %and1640, 0
  br i1 %tobool1641.not, label %cond.false1664, label %cond.true1642

cond.true1642:                                    ; preds = %cond.false1639
  %237 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp1643.not = icmp eq ptr %237, null
  br i1 %cmp1643.not, label %cond.false1649, label %cond.true1645

cond.true1645:                                    ; preds = %cond.true1642
  %idxprom1647 = sext i32 %nextarg.1 to i64
  %arrayidx1648 = getelementptr inbounds %union.arg, ptr %237, i64 %idxprom1647
  br label %cond.end1715

cond.false1649:                                   ; preds = %cond.true1642
  %gp_offset1652 = load i32, ptr %ap, align 8
  %fits_in_gp1653 = icmp ult i32 %gp_offset1652, 41
  br i1 %fits_in_gp1653, label %vaarg.in_reg1654, label %vaarg.in_mem1656

vaarg.in_reg1654:                                 ; preds = %cond.false1649
  %reg_save_area1655 = load ptr, ptr %7, align 8
  %238 = zext nneg i32 %gp_offset1652 to i64
  %239 = getelementptr i8, ptr %reg_save_area1655, i64 %238
  %240 = add nuw nsw i32 %gp_offset1652, 8
  store i32 %240, ptr %ap, align 8
  br label %cond.end1715

vaarg.in_mem1656:                                 ; preds = %cond.false1649
  %overflow_arg_area1658 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1659 = getelementptr i8, ptr %overflow_arg_area1658, i64 8
  store ptr %overflow_arg_area.next1659, ptr %overflow_arg_area_p178, align 8
  br label %cond.end1715

cond.false1664:                                   ; preds = %cond.false1639
  %and1665 = and i32 %flags.11, 2048
  %tobool1666.not = icmp eq i32 %and1665, 0
  %241 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp1690.not = icmp eq ptr %241, null
  br i1 %tobool1666.not, label %cond.false1689, label %cond.true1667

cond.true1667:                                    ; preds = %cond.false1664
  br i1 %cmp1690.not, label %cond.false1674, label %cond.true1670

cond.true1670:                                    ; preds = %cond.true1667
  %idxprom1672 = sext i32 %nextarg.1 to i64
  %arrayidx1673 = getelementptr inbounds %union.arg, ptr %241, i64 %idxprom1672
  br label %cond.end1715

cond.false1674:                                   ; preds = %cond.true1667
  %gp_offset1677 = load i32, ptr %ap, align 8
  %fits_in_gp1678 = icmp ult i32 %gp_offset1677, 41
  br i1 %fits_in_gp1678, label %vaarg.in_reg1679, label %vaarg.in_mem1681

vaarg.in_reg1679:                                 ; preds = %cond.false1674
  %reg_save_area1680 = load ptr, ptr %7, align 8
  %242 = zext nneg i32 %gp_offset1677 to i64
  %243 = getelementptr i8, ptr %reg_save_area1680, i64 %242
  %244 = add nuw nsw i32 %gp_offset1677, 8
  store i32 %244, ptr %ap, align 8
  br label %cond.end1715

vaarg.in_mem1681:                                 ; preds = %cond.false1674
  %overflow_arg_area1683 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1684 = getelementptr i8, ptr %overflow_arg_area1683, i64 8
  store ptr %overflow_arg_area.next1684, ptr %overflow_arg_area_p178, align 8
  br label %cond.end1715

cond.false1689:                                   ; preds = %cond.false1664
  br i1 %cmp1690.not, label %cond.false1696, label %cond.true1692

cond.true1692:                                    ; preds = %cond.false1689
  %idxprom1694 = sext i32 %nextarg.1 to i64
  %arrayidx1695 = getelementptr inbounds %union.arg, ptr %241, i64 %idxprom1694
  br label %cond.end1715

cond.false1696:                                   ; preds = %cond.false1689
  %gp_offset1699 = load i32, ptr %ap, align 8
  %fits_in_gp1700 = icmp ult i32 %gp_offset1699, 41
  br i1 %fits_in_gp1700, label %vaarg.in_reg1701, label %vaarg.in_mem1703

vaarg.in_reg1701:                                 ; preds = %cond.false1696
  %reg_save_area1702 = load ptr, ptr %7, align 8
  %245 = zext nneg i32 %gp_offset1699 to i64
  %246 = getelementptr i8, ptr %reg_save_area1702, i64 %245
  %247 = add nuw nsw i32 %gp_offset1699, 8
  store i32 %247, ptr %ap, align 8
  br label %cond.end1715

vaarg.in_mem1703:                                 ; preds = %cond.false1696
  %overflow_arg_area1705 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1706 = getelementptr i8, ptr %overflow_arg_area1705, i64 8
  store ptr %overflow_arg_area.next1706, ptr %overflow_arg_area_p178, align 8
  br label %cond.end1715

cond.end1715:                                     ; preds = %vaarg.in_reg1701, %vaarg.in_mem1703, %vaarg.in_reg1679, %vaarg.in_mem1681, %vaarg.in_reg1654, %vaarg.in_mem1656, %vaarg.in_reg1629, %vaarg.in_mem1631, %cond.true1645, %cond.true1692, %cond.true1670, %cond.true1620
  %cond1716.in = phi ptr [ %arrayidx1623, %cond.true1620 ], [ %arrayidx1648, %cond.true1645 ], [ %arrayidx1673, %cond.true1670 ], [ %arrayidx1695, %cond.true1692 ], [ %235, %vaarg.in_reg1629 ], [ %overflow_arg_area1633, %vaarg.in_mem1631 ], [ %239, %vaarg.in_reg1654 ], [ %overflow_arg_area1658, %vaarg.in_mem1656 ], [ %243, %vaarg.in_reg1679 ], [ %overflow_arg_area1683, %vaarg.in_mem1681 ], [ %246, %vaarg.in_reg1701 ], [ %overflow_arg_area1705, %vaarg.in_mem1703 ]
  %cond1716 = load i64, ptr %cond1716.in, align 8
  br label %nosign

if.else1717:                                      ; preds = %sw.bb1611
  %and1718 = and i32 %flags.11, 16
  %tobool1719.not = icmp eq i32 %and1718, 0
  br i1 %tobool1719.not, label %cond.false1742, label %cond.true1720

cond.true1720:                                    ; preds = %if.else1717
  %248 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp1721.not = icmp eq ptr %248, null
  br i1 %cmp1721.not, label %cond.false1727, label %cond.true1723

cond.true1723:                                    ; preds = %cond.true1720
  %idxprom1725 = sext i32 %nextarg.1 to i64
  %arrayidx1726 = getelementptr inbounds %union.arg, ptr %248, i64 %idxprom1725
  %249 = load i64, ptr %arrayidx1726, align 8, !tbaa !22
  br label %nosign

cond.false1727:                                   ; preds = %cond.true1720
  %gp_offset1730 = load i32, ptr %ap, align 8
  %fits_in_gp1731 = icmp ult i32 %gp_offset1730, 41
  br i1 %fits_in_gp1731, label %vaarg.in_reg1732, label %vaarg.in_mem1734

vaarg.in_reg1732:                                 ; preds = %cond.false1727
  %reg_save_area1733 = load ptr, ptr %7, align 8
  %250 = zext nneg i32 %gp_offset1730 to i64
  %251 = getelementptr i8, ptr %reg_save_area1733, i64 %250
  %252 = add nuw nsw i32 %gp_offset1730, 8
  store i32 %252, ptr %ap, align 8
  br label %vaarg.end1738

vaarg.in_mem1734:                                 ; preds = %cond.false1727
  %overflow_arg_area1736 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1737 = getelementptr i8, ptr %overflow_arg_area1736, i64 8
  store ptr %overflow_arg_area.next1737, ptr %overflow_arg_area_p178, align 8
  br label %vaarg.end1738

vaarg.end1738:                                    ; preds = %vaarg.in_mem1734, %vaarg.in_reg1732
  %vaarg.addr1739 = phi ptr [ %251, %vaarg.in_reg1732 ], [ %overflow_arg_area1736, %vaarg.in_mem1734 ]
  %253 = load i64, ptr %vaarg.addr1739, align 8
  br label %nosign

cond.false1742:                                   ; preds = %if.else1717
  %and1743 = and i32 %flags.11, 64
  %tobool1744.not = icmp eq i32 %and1743, 0
  br i1 %tobool1744.not, label %cond.false1769, label %cond.true1745

cond.true1745:                                    ; preds = %cond.false1742
  %254 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp1746.not = icmp eq ptr %254, null
  br i1 %cmp1746.not, label %cond.false1752, label %cond.true1748

cond.true1748:                                    ; preds = %cond.true1745
  %idxprom1750 = sext i32 %nextarg.1 to i64
  %arrayidx1751 = getelementptr inbounds %union.arg, ptr %254, i64 %idxprom1750
  br label %cond.end1765

cond.false1752:                                   ; preds = %cond.true1745
  %gp_offset1755 = load i32, ptr %ap, align 8
  %fits_in_gp1756 = icmp ult i32 %gp_offset1755, 41
  br i1 %fits_in_gp1756, label %vaarg.in_reg1757, label %vaarg.in_mem1759

vaarg.in_reg1757:                                 ; preds = %cond.false1752
  %reg_save_area1758 = load ptr, ptr %7, align 8
  %255 = zext nneg i32 %gp_offset1755 to i64
  %256 = getelementptr i8, ptr %reg_save_area1758, i64 %255
  %257 = add nuw nsw i32 %gp_offset1755, 8
  store i32 %257, ptr %ap, align 8
  br label %cond.end1765

vaarg.in_mem1759:                                 ; preds = %cond.false1752
  %overflow_arg_area1761 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1762 = getelementptr i8, ptr %overflow_arg_area1761, i64 8
  store ptr %overflow_arg_area.next1762, ptr %overflow_arg_area_p178, align 8
  br label %cond.end1765

cond.end1765:                                     ; preds = %vaarg.in_reg1757, %vaarg.in_mem1759, %cond.true1748
  %cond1766.in = phi ptr [ %arrayidx1751, %cond.true1748 ], [ %256, %vaarg.in_reg1757 ], [ %overflow_arg_area1761, %vaarg.in_mem1759 ]
  %cond1766 = load i32, ptr %cond1766.in, align 4
  %258 = and i32 %cond1766, 65535
  %conv1768 = zext nneg i32 %258 to i64
  br label %nosign

cond.false1769:                                   ; preds = %cond.false1742
  %and1770 = and i32 %flags.11, 8192
  %tobool1771.not = icmp eq i32 %and1770, 0
  %259 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp1797.not = icmp eq ptr %259, null
  br i1 %tobool1771.not, label %cond.false1796, label %cond.true1772

cond.true1772:                                    ; preds = %cond.false1769
  br i1 %cmp1797.not, label %cond.false1779, label %cond.true1775

cond.true1775:                                    ; preds = %cond.true1772
  %idxprom1777 = sext i32 %nextarg.1 to i64
  %arrayidx1778 = getelementptr inbounds %union.arg, ptr %259, i64 %idxprom1777
  br label %cond.end1792

cond.false1779:                                   ; preds = %cond.true1772
  %gp_offset1782 = load i32, ptr %ap, align 8
  %fits_in_gp1783 = icmp ult i32 %gp_offset1782, 41
  br i1 %fits_in_gp1783, label %vaarg.in_reg1784, label %vaarg.in_mem1786

vaarg.in_reg1784:                                 ; preds = %cond.false1779
  %reg_save_area1785 = load ptr, ptr %7, align 8
  %260 = zext nneg i32 %gp_offset1782 to i64
  %261 = getelementptr i8, ptr %reg_save_area1785, i64 %260
  %262 = add nuw nsw i32 %gp_offset1782, 8
  store i32 %262, ptr %ap, align 8
  br label %cond.end1792

vaarg.in_mem1786:                                 ; preds = %cond.false1779
  %overflow_arg_area1788 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1789 = getelementptr i8, ptr %overflow_arg_area1788, i64 8
  store ptr %overflow_arg_area.next1789, ptr %overflow_arg_area_p178, align 8
  br label %cond.end1792

cond.end1792:                                     ; preds = %vaarg.in_reg1784, %vaarg.in_mem1786, %cond.true1775
  %cond1793.in = phi ptr [ %arrayidx1778, %cond.true1775 ], [ %261, %vaarg.in_reg1784 ], [ %overflow_arg_area1788, %vaarg.in_mem1786 ]
  %cond1793 = load i32, ptr %cond1793.in, align 4
  %263 = and i32 %cond1793, 255
  %conv1795 = zext nneg i32 %263 to i64
  br label %nosign

cond.false1796:                                   ; preds = %cond.false1769
  br i1 %cmp1797.not, label %cond.false1803, label %cond.true1799

cond.true1799:                                    ; preds = %cond.false1796
  %idxprom1801 = sext i32 %nextarg.1 to i64
  %arrayidx1802 = getelementptr inbounds %union.arg, ptr %259, i64 %idxprom1801
  br label %cond.end1816

cond.false1803:                                   ; preds = %cond.false1796
  %gp_offset1806 = load i32, ptr %ap, align 8
  %fits_in_gp1807 = icmp ult i32 %gp_offset1806, 41
  br i1 %fits_in_gp1807, label %vaarg.in_reg1808, label %vaarg.in_mem1810

vaarg.in_reg1808:                                 ; preds = %cond.false1803
  %reg_save_area1809 = load ptr, ptr %7, align 8
  %264 = zext nneg i32 %gp_offset1806 to i64
  %265 = getelementptr i8, ptr %reg_save_area1809, i64 %264
  %266 = add nuw nsw i32 %gp_offset1806, 8
  store i32 %266, ptr %ap, align 8
  br label %cond.end1816

vaarg.in_mem1810:                                 ; preds = %cond.false1803
  %overflow_arg_area1812 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1813 = getelementptr i8, ptr %overflow_arg_area1812, i64 8
  store ptr %overflow_arg_area.next1813, ptr %overflow_arg_area_p178, align 8
  br label %cond.end1816

cond.end1816:                                     ; preds = %vaarg.in_reg1808, %vaarg.in_mem1810, %cond.true1799
  %cond1817.in = phi ptr [ %arrayidx1802, %cond.true1799 ], [ %265, %vaarg.in_reg1808 ], [ %overflow_arg_area1812, %vaarg.in_mem1810 ]
  %cond1817 = load i32, ptr %cond1817.in, align 4
  %conv1818 = zext i32 %cond1817 to i64
  br label %nosign

hex.loopexit:                                     ; preds = %reswitch
  br label %hex

hex:                                              ; preds = %reswitch, %hex.loopexit
  %xdigs.3 = phi ptr [ @ref___vfwprintf.xdigs_upper, %hex.loopexit ], [ @ref___vfwprintf.xdigs_lower, %reswitch ]
  %and1828 = and i32 %flags.0, 7200
  %tobool1829.not = icmp eq i32 %and1828, 0
  br i1 %tobool1829.not, label %if.else1933, label %if.then1830

if.then1830:                                      ; preds = %hex
  %and1831 = and i32 %flags.0, 4096
  %tobool1832.not = icmp eq i32 %and1831, 0
  br i1 %tobool1832.not, label %cond.false1855, label %cond.true1833

cond.true1833:                                    ; preds = %if.then1830
  %267 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp1834.not = icmp eq ptr %267, null
  br i1 %cmp1834.not, label %cond.false1840, label %cond.true1836

cond.true1836:                                    ; preds = %cond.true1833
  %idxprom1838 = sext i32 %nextarg.1 to i64
  %arrayidx1839 = getelementptr inbounds %union.arg, ptr %267, i64 %idxprom1838
  br label %if.end2041.thread

cond.false1840:                                   ; preds = %cond.true1833
  %gp_offset1843 = load i32, ptr %ap, align 8
  %fits_in_gp1844 = icmp ult i32 %gp_offset1843, 41
  br i1 %fits_in_gp1844, label %vaarg.in_reg1845, label %vaarg.in_mem1847

vaarg.in_reg1845:                                 ; preds = %cond.false1840
  %reg_save_area1846 = load ptr, ptr %7, align 8
  %268 = zext nneg i32 %gp_offset1843 to i64
  %269 = getelementptr i8, ptr %reg_save_area1846, i64 %268
  %270 = add nuw nsw i32 %gp_offset1843, 8
  store i32 %270, ptr %ap, align 8
  br label %if.end2041.thread

vaarg.in_mem1847:                                 ; preds = %cond.false1840
  %overflow_arg_area1849 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1850 = getelementptr i8, ptr %overflow_arg_area1849, i64 8
  store ptr %overflow_arg_area.next1850, ptr %overflow_arg_area_p178, align 8
  br label %if.end2041.thread

cond.false1855:                                   ; preds = %if.then1830
  %and1856 = and i32 %flags.0, 1024
  %tobool1857.not = icmp eq i32 %and1856, 0
  br i1 %tobool1857.not, label %cond.false1880, label %cond.true1858

cond.true1858:                                    ; preds = %cond.false1855
  %271 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp1859.not = icmp eq ptr %271, null
  br i1 %cmp1859.not, label %cond.false1865, label %cond.true1861

cond.true1861:                                    ; preds = %cond.true1858
  %idxprom1863 = sext i32 %nextarg.1 to i64
  %arrayidx1864 = getelementptr inbounds %union.arg, ptr %271, i64 %idxprom1863
  br label %if.end2041.thread

cond.false1865:                                   ; preds = %cond.true1858
  %gp_offset1868 = load i32, ptr %ap, align 8
  %fits_in_gp1869 = icmp ult i32 %gp_offset1868, 41
  br i1 %fits_in_gp1869, label %vaarg.in_reg1870, label %vaarg.in_mem1872

vaarg.in_reg1870:                                 ; preds = %cond.false1865
  %reg_save_area1871 = load ptr, ptr %7, align 8
  %272 = zext nneg i32 %gp_offset1868 to i64
  %273 = getelementptr i8, ptr %reg_save_area1871, i64 %272
  %274 = add nuw nsw i32 %gp_offset1868, 8
  store i32 %274, ptr %ap, align 8
  br label %if.end2041.thread

vaarg.in_mem1872:                                 ; preds = %cond.false1865
  %overflow_arg_area1874 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1875 = getelementptr i8, ptr %overflow_arg_area1874, i64 8
  store ptr %overflow_arg_area.next1875, ptr %overflow_arg_area_p178, align 8
  br label %if.end2041.thread

cond.false1880:                                   ; preds = %cond.false1855
  %and1881 = and i32 %flags.0, 2048
  %tobool1882.not = icmp eq i32 %and1881, 0
  %275 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp1906.not = icmp eq ptr %275, null
  br i1 %tobool1882.not, label %cond.false1905, label %cond.true1883

cond.true1883:                                    ; preds = %cond.false1880
  br i1 %cmp1906.not, label %cond.false1890, label %cond.true1886

cond.true1886:                                    ; preds = %cond.true1883
  %idxprom1888 = sext i32 %nextarg.1 to i64
  %arrayidx1889 = getelementptr inbounds %union.arg, ptr %275, i64 %idxprom1888
  br label %if.end2041.thread

cond.false1890:                                   ; preds = %cond.true1883
  %gp_offset1893 = load i32, ptr %ap, align 8
  %fits_in_gp1894 = icmp ult i32 %gp_offset1893, 41
  br i1 %fits_in_gp1894, label %vaarg.in_reg1895, label %vaarg.in_mem1897

vaarg.in_reg1895:                                 ; preds = %cond.false1890
  %reg_save_area1896 = load ptr, ptr %7, align 8
  %276 = zext nneg i32 %gp_offset1893 to i64
  %277 = getelementptr i8, ptr %reg_save_area1896, i64 %276
  %278 = add nuw nsw i32 %gp_offset1893, 8
  store i32 %278, ptr %ap, align 8
  br label %if.end2041.thread

vaarg.in_mem1897:                                 ; preds = %cond.false1890
  %overflow_arg_area1899 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1900 = getelementptr i8, ptr %overflow_arg_area1899, i64 8
  store ptr %overflow_arg_area.next1900, ptr %overflow_arg_area_p178, align 8
  br label %if.end2041.thread

cond.false1905:                                   ; preds = %cond.false1880
  br i1 %cmp1906.not, label %cond.false1912, label %cond.true1908

cond.true1908:                                    ; preds = %cond.false1905
  %idxprom1910 = sext i32 %nextarg.1 to i64
  %arrayidx1911 = getelementptr inbounds %union.arg, ptr %275, i64 %idxprom1910
  br label %if.end2041.thread

cond.false1912:                                   ; preds = %cond.false1905
  %gp_offset1915 = load i32, ptr %ap, align 8
  %fits_in_gp1916 = icmp ult i32 %gp_offset1915, 41
  br i1 %fits_in_gp1916, label %vaarg.in_reg1917, label %vaarg.in_mem1919

vaarg.in_reg1917:                                 ; preds = %cond.false1912
  %reg_save_area1918 = load ptr, ptr %7, align 8
  %279 = zext nneg i32 %gp_offset1915 to i64
  %280 = getelementptr i8, ptr %reg_save_area1918, i64 %279
  %281 = add nuw nsw i32 %gp_offset1915, 8
  store i32 %281, ptr %ap, align 8
  br label %if.end2041.thread

vaarg.in_mem1919:                                 ; preds = %cond.false1912
  %overflow_arg_area1921 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1922 = getelementptr i8, ptr %overflow_arg_area1921, i64 8
  store ptr %overflow_arg_area.next1922, ptr %overflow_arg_area_p178, align 8
  br label %if.end2041.thread

if.else1933:                                      ; preds = %hex
  %and1934 = and i32 %flags.0, 16
  %tobool1935.not = icmp eq i32 %and1934, 0
  br i1 %tobool1935.not, label %cond.false1958, label %cond.true1936

cond.true1936:                                    ; preds = %if.else1933
  %282 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp1937.not = icmp eq ptr %282, null
  br i1 %cmp1937.not, label %cond.false1943, label %cond.true1939

cond.true1939:                                    ; preds = %cond.true1936
  %idxprom1941 = sext i32 %nextarg.1 to i64
  %arrayidx1942 = getelementptr inbounds %union.arg, ptr %282, i64 %idxprom1941
  %283 = load i64, ptr %arrayidx1942, align 8, !tbaa !22
  br label %if.end2041

cond.false1943:                                   ; preds = %cond.true1936
  %gp_offset1946 = load i32, ptr %ap, align 8
  %fits_in_gp1947 = icmp ult i32 %gp_offset1946, 41
  br i1 %fits_in_gp1947, label %vaarg.in_reg1948, label %vaarg.in_mem1950

vaarg.in_reg1948:                                 ; preds = %cond.false1943
  %reg_save_area1949 = load ptr, ptr %7, align 8
  %284 = zext nneg i32 %gp_offset1946 to i64
  %285 = getelementptr i8, ptr %reg_save_area1949, i64 %284
  %286 = add nuw nsw i32 %gp_offset1946, 8
  store i32 %286, ptr %ap, align 8
  br label %vaarg.end1954

vaarg.in_mem1950:                                 ; preds = %cond.false1943
  %overflow_arg_area1952 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1953 = getelementptr i8, ptr %overflow_arg_area1952, i64 8
  store ptr %overflow_arg_area.next1953, ptr %overflow_arg_area_p178, align 8
  br label %vaarg.end1954

vaarg.end1954:                                    ; preds = %vaarg.in_mem1950, %vaarg.in_reg1948
  %vaarg.addr1955 = phi ptr [ %285, %vaarg.in_reg1948 ], [ %overflow_arg_area1952, %vaarg.in_mem1950 ]
  %287 = load i64, ptr %vaarg.addr1955, align 8
  br label %if.end2041

cond.false1958:                                   ; preds = %if.else1933
  %and1959 = and i32 %flags.0, 64
  %tobool1960.not = icmp eq i32 %and1959, 0
  br i1 %tobool1960.not, label %cond.false1985, label %cond.true1961

cond.true1961:                                    ; preds = %cond.false1958
  %288 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp1962.not = icmp eq ptr %288, null
  br i1 %cmp1962.not, label %cond.false1968, label %cond.true1964

cond.true1964:                                    ; preds = %cond.true1961
  %idxprom1966 = sext i32 %nextarg.1 to i64
  %arrayidx1967 = getelementptr inbounds %union.arg, ptr %288, i64 %idxprom1966
  br label %cond.end1981

cond.false1968:                                   ; preds = %cond.true1961
  %gp_offset1971 = load i32, ptr %ap, align 8
  %fits_in_gp1972 = icmp ult i32 %gp_offset1971, 41
  br i1 %fits_in_gp1972, label %vaarg.in_reg1973, label %vaarg.in_mem1975

vaarg.in_reg1973:                                 ; preds = %cond.false1968
  %reg_save_area1974 = load ptr, ptr %7, align 8
  %289 = zext nneg i32 %gp_offset1971 to i64
  %290 = getelementptr i8, ptr %reg_save_area1974, i64 %289
  %291 = add nuw nsw i32 %gp_offset1971, 8
  store i32 %291, ptr %ap, align 8
  br label %cond.end1981

vaarg.in_mem1975:                                 ; preds = %cond.false1968
  %overflow_arg_area1977 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next1978 = getelementptr i8, ptr %overflow_arg_area1977, i64 8
  store ptr %overflow_arg_area.next1978, ptr %overflow_arg_area_p178, align 8
  br label %cond.end1981

cond.end1981:                                     ; preds = %vaarg.in_reg1973, %vaarg.in_mem1975, %cond.true1964
  %cond1982.in = phi ptr [ %arrayidx1967, %cond.true1964 ], [ %290, %vaarg.in_reg1973 ], [ %overflow_arg_area1977, %vaarg.in_mem1975 ]
  %cond1982 = load i32, ptr %cond1982.in, align 4
  %292 = and i32 %cond1982, 65535
  %conv1984 = zext nneg i32 %292 to i64
  br label %if.end2041

cond.false1985:                                   ; preds = %cond.false1958
  %and1986 = and i32 %flags.0, 8192
  %tobool1987.not = icmp eq i32 %and1986, 0
  %293 = load ptr, ptr %argtable, align 8, !tbaa !27
  %cmp2013.not = icmp eq ptr %293, null
  br i1 %tobool1987.not, label %cond.false2012, label %cond.true1988

cond.true1988:                                    ; preds = %cond.false1985
  br i1 %cmp2013.not, label %cond.false1995, label %cond.true1991

cond.true1991:                                    ; preds = %cond.true1988
  %idxprom1993 = sext i32 %nextarg.1 to i64
  %arrayidx1994 = getelementptr inbounds %union.arg, ptr %293, i64 %idxprom1993
  br label %cond.end2008

cond.false1995:                                   ; preds = %cond.true1988
  %gp_offset1998 = load i32, ptr %ap, align 8
  %fits_in_gp1999 = icmp ult i32 %gp_offset1998, 41
  br i1 %fits_in_gp1999, label %vaarg.in_reg2000, label %vaarg.in_mem2002

vaarg.in_reg2000:                                 ; preds = %cond.false1995
  %reg_save_area2001 = load ptr, ptr %7, align 8
  %294 = zext nneg i32 %gp_offset1998 to i64
  %295 = getelementptr i8, ptr %reg_save_area2001, i64 %294
  %296 = add nuw nsw i32 %gp_offset1998, 8
  store i32 %296, ptr %ap, align 8
  br label %cond.end2008

vaarg.in_mem2002:                                 ; preds = %cond.false1995
  %overflow_arg_area2004 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next2005 = getelementptr i8, ptr %overflow_arg_area2004, i64 8
  store ptr %overflow_arg_area.next2005, ptr %overflow_arg_area_p178, align 8
  br label %cond.end2008

cond.end2008:                                     ; preds = %vaarg.in_reg2000, %vaarg.in_mem2002, %cond.true1991
  %cond2009.in = phi ptr [ %arrayidx1994, %cond.true1991 ], [ %295, %vaarg.in_reg2000 ], [ %overflow_arg_area2004, %vaarg.in_mem2002 ]
  %cond2009 = load i32, ptr %cond2009.in, align 4
  %297 = and i32 %cond2009, 255
  %conv2011 = zext nneg i32 %297 to i64
  br label %if.end2041

cond.false2012:                                   ; preds = %cond.false1985
  br i1 %cmp2013.not, label %cond.false2019, label %cond.true2015

cond.true2015:                                    ; preds = %cond.false2012
  %idxprom2017 = sext i32 %nextarg.1 to i64
  %arrayidx2018 = getelementptr inbounds %union.arg, ptr %293, i64 %idxprom2017
  br label %cond.end2032

cond.false2019:                                   ; preds = %cond.false2012
  %gp_offset2022 = load i32, ptr %ap, align 8
  %fits_in_gp2023 = icmp ult i32 %gp_offset2022, 41
  br i1 %fits_in_gp2023, label %vaarg.in_reg2024, label %vaarg.in_mem2026

vaarg.in_reg2024:                                 ; preds = %cond.false2019
  %reg_save_area2025 = load ptr, ptr %7, align 8
  %298 = zext nneg i32 %gp_offset2022 to i64
  %299 = getelementptr i8, ptr %reg_save_area2025, i64 %298
  %300 = add nuw nsw i32 %gp_offset2022, 8
  store i32 %300, ptr %ap, align 8
  br label %cond.end2032

vaarg.in_mem2026:                                 ; preds = %cond.false2019
  %overflow_arg_area2028 = load ptr, ptr %overflow_arg_area_p178, align 8
  %overflow_arg_area.next2029 = getelementptr i8, ptr %overflow_arg_area2028, i64 8
  store ptr %overflow_arg_area.next2029, ptr %overflow_arg_area_p178, align 8
  br label %cond.end2032

cond.end2032:                                     ; preds = %vaarg.in_reg2024, %vaarg.in_mem2026, %cond.true2015
  %cond2033.in = phi ptr [ %arrayidx2018, %cond.true2015 ], [ %299, %vaarg.in_reg2024 ], [ %overflow_arg_area2028, %vaarg.in_mem2026 ]
  %cond2033 = load i32, ptr %cond2033.in, align 4
  %conv2034 = zext i32 %cond2033 to i64
  br label %if.end2041

if.end2041:                                       ; preds = %vaarg.end1954, %cond.true1939, %cond.end2008, %cond.end2032, %cond.end1981
  %ulval.5 = phi i64 [ %283, %cond.true1939 ], [ %287, %vaarg.end1954 ], [ %conv1984, %cond.end1981 ], [ %conv2011, %cond.end2008 ], [ %conv2034, %cond.end2032 ]
  %and2042 = and i32 %flags.0, 1
  %tobool2043.not = icmp eq i32 %and2042, 0
  br i1 %tobool2043.not, label %if.end2055, label %cond.false2050

if.end2041.thread:                                ; preds = %cond.true1836, %cond.true1886, %cond.true1908, %cond.true1861, %vaarg.in_mem1847, %vaarg.in_reg1845, %vaarg.in_mem1872, %vaarg.in_reg1870, %vaarg.in_mem1897, %vaarg.in_reg1895, %vaarg.in_mem1919, %vaarg.in_reg1917
  %cond1932.in = phi ptr [ %arrayidx1839, %cond.true1836 ], [ %arrayidx1864, %cond.true1861 ], [ %arrayidx1889, %cond.true1886 ], [ %arrayidx1911, %cond.true1908 ], [ %269, %vaarg.in_reg1845 ], [ %overflow_arg_area1849, %vaarg.in_mem1847 ], [ %273, %vaarg.in_reg1870 ], [ %overflow_arg_area1874, %vaarg.in_mem1872 ], [ %277, %vaarg.in_reg1895 ], [ %overflow_arg_area1899, %vaarg.in_mem1897 ], [ %280, %vaarg.in_reg1917 ], [ %overflow_arg_area1921, %vaarg.in_mem1919 ]
  %cond1932 = load i64, ptr %cond1932.in, align 8
  %and20423484 = and i32 %flags.0, 1
  %tobool2043.not3485 = icmp eq i32 %and20423484, 0
  br i1 %tobool2043.not3485, label %if.end2055, label %cond.true2047

cond.true2047:                                    ; preds = %if.end2041.thread
  %cmp2048.not = icmp eq i64 %cond1932, 0
  br i1 %cmp2048.not, label %if.end2055, label %if.then2053

cond.false2050:                                   ; preds = %if.end2041
  %cmp2051.not = icmp eq i64 %ulval.5, 0
  br i1 %cmp2051.not, label %if.end2055, label %if.then2053

if.then2053:                                      ; preds = %cond.false2050, %cond.true2047
  %ulval.534863494 = phi i64 [ %ulval.5, %cond.false2050 ], [ %ulval.0.ph, %cond.true2047 ]
  %ujval.534883493 = phi i64 [ %ujval.0.ph, %cond.false2050 ], [ %cond1932, %cond.true2047 ]
  store i32 %ch.0, ptr %arrayidx, align 4, !tbaa !5
  br label %if.end2055

if.end2055:                                       ; preds = %if.end2041.thread, %if.then2053, %cond.false2050, %cond.true2047, %if.end2041
  %ujval.53489 = phi i64 [ %cond1932, %if.end2041.thread ], [ %ujval.534883493, %if.then2053 ], [ %ujval.0.ph, %cond.false2050 ], [ 0, %cond.true2047 ], [ %ujval.0.ph, %if.end2041 ]
  %ulval.53487 = phi i64 [ %ulval.0.ph, %if.end2041.thread ], [ %ulval.534863494, %if.then2053 ], [ 0, %cond.false2050 ], [ %ulval.0.ph, %cond.true2047 ], [ %ulval.5, %if.end2041 ]
  %and2056 = and i32 %flags.0, -513
  br label %nosign

nosign:                                           ; preds = %if.end535.thread, %cond.end1715, %cond.end1765, %cond.end1816, %cond.end1792, %cond.true1723, %vaarg.end1738, %cond.end1389, %cond.end1439, %cond.end1490, %cond.end1466, %cond.true1397, %vaarg.end1412, %if.end535, %cond.true541, %cond.false544, %if.then547, %if.end2055, %cond.end1520
  %flags.12 = phi i32 [ %and2056, %if.end2055 ], [ %or1522, %cond.end1520 ], [ %flags.0, %if.then547 ], [ %flags.0, %cond.true541 ], [ %flags.0, %cond.false544 ], [ %flags.0, %if.end535 ], [ %flags.9, %vaarg.end1412 ], [ %flags.9, %cond.true1397 ], [ %flags.9, %cond.end1466 ], [ %flags.9, %cond.end1490 ], [ %flags.9, %cond.end1439 ], [ %flags.9, %cond.end1389 ], [ %flags.11, %vaarg.end1738 ], [ %flags.11, %cond.true1723 ], [ %flags.11, %cond.end1792 ], [ %flags.11, %cond.end1816 ], [ %flags.11, %cond.end1765 ], [ %flags.11, %cond.end1715 ], [ %flags.0, %if.end535.thread ]
  %ulval.6 = phi i64 [ %ulval.53487, %if.end2055 ], [ %ulval.0.ph, %cond.end1520 ], [ %ulval.134623468, %if.then547 ], [ %ulval.0.ph, %cond.true541 ], [ 0, %cond.false544 ], [ %ulval.1, %if.end535 ], [ %202, %vaarg.end1412 ], [ %198, %cond.true1397 ], [ %conv1469, %cond.end1466 ], [ %conv1492, %cond.end1490 ], [ %conv1442, %cond.end1439 ], [ %ulval.0.ph, %cond.end1389 ], [ %253, %vaarg.end1738 ], [ %249, %cond.true1723 ], [ %conv1795, %cond.end1792 ], [ %conv1818, %cond.end1816 ], [ %conv1768, %cond.end1765 ], [ %ulval.0.ph, %cond.end1715 ], [ %ulval.0.ph, %if.end535.thread ]
  %ujval.6 = phi i64 [ %ujval.53489, %if.end2055 ], [ %220, %cond.end1520 ], [ %ujval.134633467, %if.then547 ], [ 0, %cond.true541 ], [ %ujval.0.ph, %cond.false544 ], [ %ujval.0.ph, %if.end535 ], [ %ujval.0.ph, %vaarg.end1412 ], [ %ujval.0.ph, %cond.true1397 ], [ %ujval.0.ph, %cond.end1466 ], [ %ujval.0.ph, %cond.end1490 ], [ %ujval.0.ph, %cond.end1439 ], [ %cond1390, %cond.end1389 ], [ %ujval.0.ph, %vaarg.end1738 ], [ %ujval.0.ph, %cond.true1723 ], [ %ujval.0.ph, %cond.end1792 ], [ %ujval.0.ph, %cond.end1816 ], [ %ujval.0.ph, %cond.end1765 ], [ %cond1716, %cond.end1715 ], [ %cond426, %if.end535.thread ]
  %base.0 = phi i32 [ 16, %if.end2055 ], [ 16, %cond.end1520 ], [ 2, %if.then547 ], [ 2, %cond.true541 ], [ 2, %cond.false544 ], [ 2, %if.end535 ], [ 8, %vaarg.end1412 ], [ 8, %cond.true1397 ], [ 8, %cond.end1466 ], [ 8, %cond.end1490 ], [ 8, %cond.end1439 ], [ 8, %cond.end1389 ], [ 10, %vaarg.end1738 ], [ 10, %cond.true1723 ], [ 10, %cond.end1792 ], [ 10, %cond.end1816 ], [ 10, %cond.end1765 ], [ 10, %cond.end1715 ], [ 2, %if.end535.thread ]
  %xdigs.4 = phi ptr [ %xdigs.3, %if.end2055 ], [ @ref___vfwprintf.xdigs_lower, %cond.end1520 ], [ %xdigs.0.ph, %if.then547 ], [ %xdigs.0.ph, %cond.true541 ], [ %xdigs.0.ph, %cond.false544 ], [ %xdigs.0.ph, %if.end535 ], [ %xdigs.0.ph, %vaarg.end1412 ], [ %xdigs.0.ph, %cond.true1397 ], [ %xdigs.0.ph, %cond.end1466 ], [ %xdigs.0.ph, %cond.end1490 ], [ %xdigs.0.ph, %cond.end1439 ], [ %xdigs.0.ph, %cond.end1389 ], [ %xdigs.0.ph, %vaarg.end1738 ], [ %xdigs.0.ph, %cond.true1723 ], [ %xdigs.0.ph, %cond.end1792 ], [ %xdigs.0.ph, %cond.end1816 ], [ %xdigs.0.ph, %cond.end1765 ], [ %xdigs.0.ph, %cond.end1715 ], [ %xdigs.0.ph, %if.end535.thread ]
  store i32 0, ptr %sign, align 4, !tbaa !5
  br label %number

number:                                           ; preds = %if.then713, %cond.end709, %if.then826, %cond.end822, %nosign
  %flags.13 = phi i32 [ %flags.12, %nosign ], [ %flags.8, %cond.end822 ], [ %flags.8, %if.then826 ], [ %flags.8, %cond.end709 ], [ %flags.8, %if.then713 ]
  %ulval.7 = phi i64 [ %ulval.6, %nosign ], [ %cond823, %cond.end822 ], [ %sub827, %if.then826 ], [ %ulval.0.ph, %cond.end709 ], [ %ulval.0.ph, %if.then713 ]
  %ujval.7 = phi i64 [ %ujval.6, %nosign ], [ %ujval.0.ph, %cond.end822 ], [ %ujval.0.ph, %if.then826 ], [ %cond710, %cond.end709 ], [ %sub714, %if.then713 ]
  %base.1 = phi i32 [ %base.0, %nosign ], [ 10, %cond.end822 ], [ 10, %if.then826 ], [ 10, %cond.end709 ], [ 10, %if.then713 ]
  %xdigs.5 = phi ptr [ %xdigs.4, %nosign ], [ %xdigs.0.ph, %cond.end822 ], [ %xdigs.0.ph, %if.then826 ], [ %xdigs.0.ph, %cond.end709 ], [ %xdigs.0.ph, %if.then713 ]
  %nextarg.63 = add nsw i32 %nextarg.1, 1
  %and2060 = and i32 %flags.13, -129
  %cmp20573542 = icmp slt i32 %prec.1, 0
  %spec.select3036 = select i1 %cmp20573542, i32 %flags.13, i32 %and2060
  %and2064 = and i32 %spec.select3036, 7200
  %tobool2065.not = icmp eq i32 %and2064, 0
  %cmp2086 = icmp ne i32 %prec.1, 0
  br i1 %tobool2065.not, label %if.else2082, label %if.then2066

if.then2066:                                      ; preds = %number
  %cmp2067 = icmp ne i64 %ujval.7, 0
  %or.cond2432 = select i1 %cmp2067, i1 true, i1 %cmp2086
  br i1 %or.cond2432, label %if.then2078, label %lor.lhs.false2072

lor.lhs.false2072:                                ; preds = %if.then2066
  %and2073 = and i32 %flags.13, 1
  %tobool2074 = icmp ne i32 %and2073, 0
  %cmp2076 = icmp eq i32 %base.1, 8
  %or.cond2433 = and i1 %tobool2074, %cmp2076
  br i1 %or.cond2433, label %if.then2078, label %if.end2098

if.then2078:                                      ; preds = %lor.lhs.false2072, %if.then2066
  %and2079 = and i32 %spec.select3036, 1
  %call.i3116 = call fastcc noundef nonnull ptr @__ultoa(i64 noundef %ujval.7, ptr noundef nonnull %add.ptr2063, i32 noundef %base.1, i32 noundef %and2079, ptr noundef %xdigs.5)
  br label %if.end2098

if.else2082:                                      ; preds = %number
  %cmp2083 = icmp ne i64 %ulval.7, 0
  %or.cond2434 = select i1 %cmp2083, i1 true, i1 %cmp2086
  br i1 %or.cond2434, label %if.then2094, label %lor.lhs.false2088

lor.lhs.false2088:                                ; preds = %if.else2082
  %and2089 = and i32 %flags.13, 1
  %tobool2090 = icmp ne i32 %and2089, 0
  %cmp2092 = icmp eq i32 %base.1, 8
  %or.cond2435 = and i1 %tobool2090, %cmp2092
  br i1 %or.cond2435, label %if.then2094, label %if.end2098

if.then2094:                                      ; preds = %lor.lhs.false2088, %if.else2082
  %and2095 = and i32 %spec.select3036, 1
  %call2096 = call fastcc ptr @__ultoa(i64 noundef %ulval.7, ptr noundef nonnull %add.ptr2063, i32 noundef %base.1, i32 noundef %and2095, ptr noundef %xdigs.5)
  br label %if.end2098

if.end2098:                                       ; preds = %lor.lhs.false2088, %if.then2094, %lor.lhs.false2072, %if.then2078
  %cp.14 = phi ptr [ %call.i3116, %if.then2078 ], [ %add.ptr2063, %lor.lhs.false2072 ], [ %call2096, %if.then2094 ], [ %add.ptr2063, %lor.lhs.false2088 ]
  %sub.ptr.rhs.cast2102 = ptrtoint ptr %cp.14 to i64
  %sub.ptr.sub2103 = sub i64 %sub.ptr.lhs.cast2101, %sub.ptr.rhs.cast2102
  %sub.ptr.div2104 = lshr exact i64 %sub.ptr.sub2103, 2
  %conv2105 = trunc i64 %sub.ptr.div2104 to i32
  %sext3027 = shl i64 %sub.ptr.sub2103, 30
  %cmp2107 = icmp ugt i64 %sext3027, 279172874239
  br i1 %cmp2107, label %if.then2109, label %if.end2110

if.then2109:                                      ; preds = %if.end2098
  call void @abort() #17
  unreachable

if.end2110:                                       ; preds = %if.end2098
  %and2111 = and i32 %spec.select3036, 512
  %tobool2112 = icmp ne i32 %and2111, 0
  %cmp2114 = icmp ne i32 %conv2105, 0
  %or.cond2436 = select i1 %tobool2112, i1 %cmp2114, i1 false
  br i1 %or.cond2436, label %if.then2116, label %sw.epilog

if.then2116:                                      ; preds = %if.end2110
  %call2117 = call fastcc i32 @ref_grouping_init(ptr noundef nonnull %gs, i32 noundef %conv2105, ptr noundef %locale)
  %add2118 = add nsw i32 %call2117, %conv2105
  br label %sw.epilog

sw.default:                                       ; preds = %reswitch
  %cmp2120 = icmp eq i32 %ch.0, 0
  br i1 %cmp2120, label %done, label %invalid

invalid:                                          ; preds = %if.else308, %sw.default
  %flags.15 = phi i32 [ %flags.0, %sw.default ], [ %spec.select3538, %if.else308 ]
  %fmt.13 = phi ptr [ %fmt.3, %sw.default ], [ %spec.select3539, %if.else308 ]
  store i32 %ch.0, ptr %buf, align 16, !tbaa !5
  store i32 0, ptr %sign, align 4, !tbaa !5
  br label %sw.epilog

sw.epilog:                                        ; preds = %exponent.exit, %if.end2110, %if.then2116, %if.then1067, %if.else1048, %invalid, %cond.end1606, %if.end999, %if.end602
  %cp.15 = phi ptr [ %buf, %invalid ], [ %cp.14, %if.then2116 ], [ %cp.14, %if.end2110 ], [ %cp.13, %cond.end1606 ], [ %cp.10, %if.end999 ], [ %convbuf.1, %if.then1067 ], [ %convbuf.1, %if.else1048 ], [ %buf, %if.end602 ], [ %convbuf.1, %exponent.exit ]
  %flags.16 = phi i32 [ %flags.15, %invalid ], [ %spec.select3036, %if.then2116 ], [ %spec.select3036, %if.end2110 ], [ %flags.10, %cond.end1606 ], [ %and1000, %if.end999 ], [ %or1002, %if.then1067 ], [ %or1002, %if.else1048 ], [ %flags.7, %if.end602 ], [ %or1002, %exponent.exit ]
  %prec.11 = phi i32 [ %prec.1, %invalid ], [ %prec.1, %if.then2116 ], [ %prec.1, %if.end2110 ], [ %prec.1, %cond.end1606 ], [ %prec.8, %if.end999 ], [ %prec.103473, %if.then1067 ], [ %prec.103473, %if.else1048 ], [ %prec.1, %if.end602 ], [ %prec.10, %exponent.exit ]
  %expchar.5 = phi i8 [ %expchar.0.ph, %invalid ], [ %expchar.0.ph, %if.then2116 ], [ %expchar.0.ph, %if.end2110 ], [ %expchar.0.ph, %cond.end1606 ], [ %expchar.3, %if.end999 ], [ 0, %if.then1067 ], [ 0, %if.else1048 ], [ %expchar.0.ph, %if.end602 ], [ %expchar.3, %exponent.exit ]
  %expsize.1 = phi i32 [ %expsize.0.ph, %invalid ], [ %expsize.0.ph, %if.then2116 ], [ %expsize.0.ph, %if.end2110 ], [ %expsize.0.ph, %cond.end1606 ], [ %expsize.0.ph, %if.end999 ], [ %expsize.0.ph, %if.then1067 ], [ %expsize.0.ph, %if.else1048 ], [ %expsize.0.ph, %if.end602 ], [ %conv.i3082, %exponent.exit ]
  %ndig.2 = phi i32 [ %ndig.0.ph, %invalid ], [ %ndig.0.ph, %if.then2116 ], [ %ndig.0.ph, %if.end2110 ], [ %ndig.0.ph, %cond.end1606 ], [ %ndig.1, %if.end999 ], [ %ndig.1, %if.then1067 ], [ %ndig.1, %if.else1048 ], [ %ndig.0.ph, %if.end602 ], [ %ndig.1, %exponent.exit ]
  %ulval.8 = phi i64 [ %ulval.0.ph, %invalid ], [ %ulval.7, %if.then2116 ], [ %ulval.7, %if.end2110 ], [ %ulval.0.ph, %cond.end1606 ], [ %ulval.0.ph, %if.end999 ], [ %ulval.0.ph, %if.then1067 ], [ %ulval.0.ph, %if.else1048 ], [ %ulval.0.ph, %if.end602 ], [ %ulval.0.ph, %exponent.exit ]
  %ujval.8 = phi i64 [ %ujval.0.ph, %invalid ], [ %ujval.7, %if.then2116 ], [ %ujval.7, %if.end2110 ], [ %ujval.0.ph, %cond.end1606 ], [ %ujval.0.ph, %if.end999 ], [ %ujval.0.ph, %if.then1067 ], [ %ujval.0.ph, %if.else1048 ], [ %ujval.0.ph, %if.end602 ], [ %ujval.0.ph, %exponent.exit ]
  %dprec.0 = phi i32 [ 0, %invalid ], [ %prec.1, %if.then2116 ], [ %prec.1, %if.end2110 ], [ 0, %cond.end1606 ], [ 0, %if.end999 ], [ 0, %if.then1067 ], [ 0, %if.else1048 ], [ 0, %if.end602 ], [ 0, %exponent.exit ]
  %size.2 = phi i32 [ 1, %invalid ], [ %add2118, %if.then2116 ], [ %conv2105, %if.end2110 ], [ %conv1608, %cond.end1606 ], [ 3, %if.end999 ], [ %add1069, %if.then1067 ], [ %size.1, %if.else1048 ], [ 1, %if.end602 ], [ %spec.select3042, %exponent.exit ]
  %xdigs.6 = phi ptr [ %xdigs.0.ph, %invalid ], [ %xdigs.5, %if.then2116 ], [ %xdigs.5, %if.end2110 ], [ %xdigs.0.ph, %cond.end1606 ], [ %xdigs.2, %if.end999 ], [ %xdigs.2, %if.then1067 ], [ %xdigs.2, %if.else1048 ], [ %xdigs.0.ph, %if.end602 ], [ %xdigs.2, %exponent.exit ]
  %nextarg.64 = phi i32 [ %nextarg.1, %invalid ], [ %nextarg.63, %if.then2116 ], [ %nextarg.63, %if.end2110 ], [ %nextarg.49, %cond.end1606 ], [ %nextarg.30, %if.end999 ], [ %nextarg.30, %if.then1067 ], [ %nextarg.30, %if.else1048 ], [ %nextarg.17, %if.end602 ], [ %nextarg.30, %exponent.exit ]
  %convbuf.5 = phi ptr [ %convbuf.0.ph, %invalid ], [ %convbuf.0.ph, %if.then2116 ], [ %convbuf.0.ph, %if.end2110 ], [ %convbuf.4, %cond.end1606 ], [ %convbuf.1, %if.end999 ], [ %convbuf.1, %if.then1067 ], [ %convbuf.1, %if.else1048 ], [ %convbuf.0.ph, %if.end602 ], [ %convbuf.1, %exponent.exit ]
  %fmt.14 = phi ptr [ %fmt.13, %invalid ], [ %fmt.3, %if.then2116 ], [ %fmt.3, %if.end2110 ], [ %fmt.3, %cond.end1606 ], [ %fmt.3, %if.end999 ], [ %fmt.3, %if.then1067 ], [ %fmt.3, %if.else1048 ], [ %fmt.3, %if.end602 ], [ %fmt.3, %exponent.exit ]
  %cond2130 = call i32 @llvm.smax.i32(i32 %dprec.0, i32 %size.2)
  %301 = load i32, ptr %sign, align 4, !tbaa !5
  %tobool2131.not = icmp ne i32 %301, 0
  %inc2133 = zext i1 %tobool2131.not to i32
  %spec.select3037 = add nsw i32 %cond2130, %inc2133
  %302 = load i32, ptr %arrayidx, align 4, !tbaa !5
  %tobool2136.not = icmp eq i32 %302, 0
  %add2138 = add nsw i32 %spec.select3037, 2
  %realsz.1 = select i1 %tobool2136.not, i32 %spec.select3037, i32 %add2138
  %cond2145 = call i32 @llvm.smax.i32(i32 %width.1.ph, i32 %realsz.1)
  %add2146 = add i32 %cond2145, %ret.1
  %cmp2147 = icmp slt i32 %add2146, 0
  br i1 %cmp2147, label %if.then2149, label %if.end2151

if.then2149:                                      ; preds = %sw.epilog
  %call2150 = call ptr @__error() #14
  store i32 84, ptr %call2150, align 4, !tbaa !5
  br label %error

if.end2151:                                       ; preds = %sw.epilog
  %and2152 = and i32 %flags.16, 132
  %cmp2153 = icmp eq i32 %and2152, 0
  %sub2156 = sub nsw i32 %width.1.ph, %realsz.1
  %cmp8.i = icmp sgt i32 %sub2156, 0
  %or.cond3543 = select i1 %cmp2153, i1 %cmp8.i, i1 false
  br i1 %or.cond3543, label %while.body.i3118, label %if.end2161

while.body.i3118:                                 ; preds = %if.end2151, %if.end.i3119
  %howmany.addr.09.i = phi i32 [ %sub.i3120, %if.end.i3119 ], [ %sub2156, %if.end2151 ]
  %cond.i = call i32 @llvm.smin.i32(i32 %howmany.addr.09.i, i32 16)
  %303 = load i32, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !56
  %idxprom.i.i = sext i32 %303 to i64
  %arrayidx.i.i = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i.i
  store ptr @blanks, ptr %arrayidx.i.i, align 8, !tbaa !41, !noalias !56
  %conv.i.i = zext nneg i32 %cond.i to i64
  %iov_len.i.i = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i.i, i32 1
  store i64 %conv.i.i, ptr %iov_len.i.i, align 8, !tbaa !43, !noalias !56
  %304 = load i32, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !56
  %add.i.i = add nsw i32 %304, %cond.i
  store i32 %add.i.i, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !56
  %inc.i.i = add nsw i32 %303, 1
  store i32 %inc.i.i, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !56
  %cmp.i.i = icmp sgt i32 %303, 6
  br i1 %cmp.i.i, label %io_print.exit.i, label %if.end.i3119

io_print.exit.i:                                  ; preds = %while.body.i3118
  %305 = load ptr, ptr %io, align 8, !tbaa !33, !noalias !56
  %call.i.i = call i32 (ptr, ptr, ptr, ...) @__sprint(ptr noundef %305, ptr noundef nonnull %uio.i, ptr noundef %locale) #14
  %tobool.not.i = icmp eq i32 %call.i.i, 0
  br i1 %tobool.not.i, label %if.end.i3119, label %error

if.end.i3119:                                     ; preds = %io_print.exit.i, %while.body.i3118
  %sub.i3120 = sub nsw i32 %howmany.addr.09.i, %cond.i
  %cmp.i3121 = icmp sgt i32 %sub.i3120, 0
  br i1 %cmp.i3121, label %while.body.i3118, label %if.end2161.loopexit, !llvm.loop !61

if.end2161.loopexit:                              ; preds = %if.end.i3119
  %.pre4391 = load i32, ptr %sign, align 4, !tbaa !5
  br label %if.end2161

if.end2161:                                       ; preds = %if.end2161.loopexit, %if.end2151
  %306 = phi i32 [ %.pre4391, %if.end2161.loopexit ], [ %301, %if.end2151 ]
  %tobool2162.not = icmp eq i32 %306, 0
  br i1 %tobool2162.not, label %if.end2171, label %do.body2164

do.body2164:                                      ; preds = %if.end2161
  %307 = load <2 x i32>, ptr %uio_iovcnt.i, align 8, !tbaa !5, !noalias !62
  %308 = extractelement <2 x i32> %307, i64 0
  %idxprom.i3123 = sext i32 %308 to i64
  %arrayidx.i3124 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i3123
  store ptr %sign, ptr %arrayidx.i3124, align 8, !tbaa !41, !noalias !62
  %iov_len.i3125 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i3123, i32 1
  store i64 1, ptr %iov_len.i3125, align 8, !tbaa !43, !noalias !62
  %309 = add nsw <2 x i32> %307, <i32 1, i32 1>
  store <2 x i32> %309, ptr %uio_iovcnt.i, align 8, !tbaa !5, !noalias !62
  %cmp.i3129 = icmp sgt i32 %308, 6
  br i1 %cmp.i3129, label %io_print.exit3134, label %if.end2171

io_print.exit3134:                                ; preds = %do.body2164
  %310 = load ptr, ptr %io, align 8, !tbaa !33, !noalias !62
  %call.i3133 = call i32 (ptr, ptr, ptr, ...) @__sprint(ptr noundef %310, ptr noundef nonnull %uio.i, ptr noundef %locale) #14
  %tobool2166.not = icmp eq i32 %call.i3133, 0
  br i1 %tobool2166.not, label %if.end2171, label %error

if.end2171:                                       ; preds = %do.body2164, %io_print.exit3134, %if.end2161
  %311 = load i32, ptr %arrayidx, align 4, !tbaa !5
  %tobool2173.not = icmp eq i32 %311, 0
  br i1 %tobool2173.not, label %if.end2184, label %if.then2174

if.then2174:                                      ; preds = %if.end2171
  store i32 48, ptr %ox, align 4, !tbaa !5
  %312 = load <2 x i32>, ptr %uio_iovcnt.i, align 8, !tbaa !5, !noalias !65
  %313 = extractelement <2 x i32> %312, i64 0
  %idxprom.i3136 = sext i32 %313 to i64
  %arrayidx.i3137 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i3136
  store ptr %ox, ptr %arrayidx.i3137, align 8, !tbaa !41, !noalias !65
  %iov_len.i3138 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i3136, i32 1
  store i64 2, ptr %iov_len.i3138, align 8, !tbaa !43, !noalias !65
  %314 = add nsw <2 x i32> %312, <i32 1, i32 2>
  store <2 x i32> %314, ptr %uio_iovcnt.i, align 8, !tbaa !5, !noalias !65
  %cmp.i3142 = icmp sgt i32 %313, 6
  br i1 %cmp.i3142, label %io_print.exit3147, label %if.end2184

io_print.exit3147:                                ; preds = %if.then2174
  %315 = load ptr, ptr %io, align 8, !tbaa !33, !noalias !65
  %call.i3146 = call i32 (ptr, ptr, ptr, ...) @__sprint(ptr noundef %315, ptr noundef nonnull %uio.i, ptr noundef %locale) #14
  %tobool2179.not = icmp eq i32 %call.i3146, 0
  br i1 %tobool2179.not, label %if.end2184, label %error

if.end2184:                                       ; preds = %if.then2174, %io_print.exit3147, %if.end2171
  %cmp2186 = icmp eq i32 %and2152, 128
  %or.cond3544 = select i1 %cmp2186, i1 %cmp8.i, i1 false
  br i1 %or.cond3544, label %while.body.i3154, label %if.end2194

while.body.i3154:                                 ; preds = %if.end2184, %if.end.i3164
  %howmany.addr.09.i3155 = phi i32 [ %sub.i3165, %if.end.i3164 ], [ %sub2156, %if.end2184 ]
  %cond.i3156 = call i32 @llvm.smin.i32(i32 %howmany.addr.09.i3155, i32 16)
  %316 = load i32, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !68
  %idxprom.i.i3157 = sext i32 %316 to i64
  %arrayidx.i.i3158 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i.i3157
  store ptr @zeroes, ptr %arrayidx.i.i3158, align 8, !tbaa !41, !noalias !68
  %conv.i.i3159 = zext nneg i32 %cond.i3156 to i64
  %iov_len.i.i3160 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i.i3157, i32 1
  store i64 %conv.i.i3159, ptr %iov_len.i.i3160, align 8, !tbaa !43, !noalias !68
  %317 = load i32, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !68
  %add.i.i3161 = add nsw i32 %317, %cond.i3156
  store i32 %add.i.i3161, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !68
  %inc.i.i3162 = add nsw i32 %316, 1
  store i32 %inc.i.i3162, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !68
  %cmp.i.i3163 = icmp sgt i32 %316, 6
  br i1 %cmp.i.i3163, label %io_print.exit.i3167, label %if.end.i3164

io_print.exit.i3167:                              ; preds = %while.body.i3154
  %318 = load ptr, ptr %io, align 8, !tbaa !33, !noalias !68
  %call.i.i3168 = call i32 (ptr, ptr, ptr, ...) @__sprint(ptr noundef %318, ptr noundef nonnull %uio.i, ptr noundef %locale) #14
  %tobool.not.i3169 = icmp eq i32 %call.i.i3168, 0
  br i1 %tobool.not.i3169, label %if.end.i3164, label %error

if.end.i3164:                                     ; preds = %io_print.exit.i3167, %while.body.i3154
  %sub.i3165 = sub nsw i32 %howmany.addr.09.i3155, %cond.i3156
  %cmp.i3166 = icmp sgt i32 %sub.i3165, 0
  br i1 %cmp.i3166, label %while.body.i3154, label %if.end2194, !llvm.loop !61

if.end2194:                                       ; preds = %if.end.i3164, %if.end2184
  %and2195 = and i32 %flags.16, 256
  %cmp2196 = icmp eq i32 %and2195, 0
  br i1 %cmp2196, label %if.then2198, label %if.else2223

if.then2198:                                      ; preds = %if.end2194
  %sub2199 = sub nsw i32 %dprec.0, %size.2
  %cmp8.i3172 = icmp sgt i32 %sub2199, 0
  br i1 %cmp8.i3172, label %while.body.i3177, label %if.end2203

while.body.i3177:                                 ; preds = %if.then2198, %if.end.i3187
  %howmany.addr.09.i3178 = phi i32 [ %sub.i3188, %if.end.i3187 ], [ %sub2199, %if.then2198 ]
  %cond.i3179 = call i32 @llvm.smin.i32(i32 %howmany.addr.09.i3178, i32 16)
  %319 = load i32, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !73
  %idxprom.i.i3180 = sext i32 %319 to i64
  %arrayidx.i.i3181 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i.i3180
  store ptr @zeroes, ptr %arrayidx.i.i3181, align 8, !tbaa !41, !noalias !73
  %conv.i.i3182 = zext nneg i32 %cond.i3179 to i64
  %iov_len.i.i3183 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i.i3180, i32 1
  store i64 %conv.i.i3182, ptr %iov_len.i.i3183, align 8, !tbaa !43, !noalias !73
  %320 = load i32, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !73
  %add.i.i3184 = add nsw i32 %320, %cond.i3179
  store i32 %add.i.i3184, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !73
  %inc.i.i3185 = add nsw i32 %319, 1
  store i32 %inc.i.i3185, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !73
  %cmp.i.i3186 = icmp sgt i32 %319, 6
  br i1 %cmp.i.i3186, label %io_print.exit.i3190, label %if.end.i3187

io_print.exit.i3190:                              ; preds = %while.body.i3177
  %321 = load ptr, ptr %io, align 8, !tbaa !33, !noalias !73
  %call.i.i3191 = call i32 (ptr, ptr, ptr, ...) @__sprint(ptr noundef %321, ptr noundef nonnull %uio.i, ptr noundef %locale) #14
  %tobool.not.i3192 = icmp eq i32 %call.i.i3191, 0
  br i1 %tobool.not.i3192, label %if.end.i3187, label %error

if.end.i3187:                                     ; preds = %io_print.exit.i3190, %while.body.i3177
  %sub.i3188 = sub nsw i32 %howmany.addr.09.i3178, %cond.i3179
  %cmp.i3189 = icmp sgt i32 %sub.i3188, 0
  br i1 %cmp.i3189, label %while.body.i3177, label %if.end2203, !llvm.loop !61

if.end2203:                                       ; preds = %if.end.i3187, %if.then2198
  %322 = load ptr, ptr %grouping, align 8, !tbaa !44
  %tobool2205.not = icmp eq ptr %322, null
  br i1 %tobool2205.not, label %do.body2215, label %if.then2206

if.then2206:                                      ; preds = %if.end2203
  %call2209 = call fastcc i32 @ref_grouping_print(ptr noundef nonnull %gs, ptr noundef nonnull %io, ptr noundef %cp.15, ptr noundef nonnull %add.ptr2063, ptr noundef %locale)
  %cmp2210 = icmp slt i32 %call2209, 0
  br i1 %cmp2210, label %error, label %if.end2346

do.body2215:                                      ; preds = %if.end2203
  %323 = load i32, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !78
  %idxprom.i3195 = sext i32 %323 to i64
  %arrayidx.i3196 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i3195
  store ptr %cp.15, ptr %arrayidx.i3196, align 8, !tbaa !41, !noalias !78
  %conv.i3197 = sext i32 %size.2 to i64
  %iov_len.i3198 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i3195, i32 1
  store i64 %conv.i3197, ptr %iov_len.i3198, align 8, !tbaa !43, !noalias !78
  %324 = load i32, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !78
  %add.i3200 = add nsw i32 %324, %size.2
  store i32 %add.i3200, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !78
  %inc.i3201 = add nsw i32 %323, 1
  store i32 %inc.i3201, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !78
  %cmp.i3202 = icmp sgt i32 %323, 6
  br i1 %cmp.i3202, label %io_print.exit3207, label %if.end2346

io_print.exit3207:                                ; preds = %do.body2215
  %325 = load ptr, ptr %io, align 8, !tbaa !33, !noalias !78
  %call.i3206 = call i32 (ptr, ptr, ptr, ...) @__sprint(ptr noundef %325, ptr noundef nonnull %uio.i, ptr noundef %locale) #14
  %tobool2217.not = icmp eq i32 %call.i3206, 0
  br i1 %tobool2217.not, label %if.end2346, label %error

if.else2223:                                      ; preds = %if.end2194
  %tobool2224.not = icmp eq i8 %expchar.5, 0
  br i1 %tobool2224.not, label %if.then2225, label %if.else2297

if.then2225:                                      ; preds = %if.else2223
  %326 = load i32, ptr %expt, align 4, !tbaa !5
  %cmp2226 = icmp slt i32 %326, 1
  br i1 %cmp2226, label %do.body2229, label %if.else2255

do.body2229:                                      ; preds = %if.then2225
  %327 = load <2 x i32>, ptr %uio_iovcnt.i, align 8, !tbaa !5, !noalias !81
  %328 = extractelement <2 x i32> %327, i64 0
  %idxprom.i3209 = sext i32 %328 to i64
  %arrayidx.i3210 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i3209
  store ptr @zeroes, ptr %arrayidx.i3210, align 8, !tbaa !41, !noalias !81
  %iov_len.i3211 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i3209, i32 1
  store i64 1, ptr %iov_len.i3211, align 8, !tbaa !43, !noalias !81
  %329 = add nsw <2 x i32> %327, <i32 1, i32 1>
  store <2 x i32> %329, ptr %uio_iovcnt.i, align 8, !tbaa !5, !noalias !81
  %cmp.i3215 = icmp sgt i32 %328, 6
  br i1 %cmp.i3215, label %io_print.exit3220, label %do.end2235

io_print.exit3220:                                ; preds = %do.body2229
  %330 = load ptr, ptr %io, align 8, !tbaa !33, !noalias !81
  %call.i3219 = call i32 (ptr, ptr, ptr, ...) @__sprint(ptr noundef %330, ptr noundef nonnull %uio.i, ptr noundef %locale) #14
  %tobool2231.not = icmp eq i32 %call.i3219, 0
  br i1 %tobool2231.not, label %do.end2235, label %error

do.end2235:                                       ; preds = %do.body2229, %io_print.exit3220
  %tobool2236.not = icmp eq i32 %prec.11, 0
  %and2238 = and i32 %flags.16, 1
  %tobool2239.not = icmp eq i32 %and2238, 0
  %or.cond3038 = select i1 %tobool2236.not, i1 %tobool2239.not, i1 false
  br i1 %or.cond3038, label %if.end2248, label %do.body2241

do.body2241:                                      ; preds = %do.end2235
  %331 = load <2 x i32>, ptr %uio_iovcnt.i, align 8, !tbaa !5, !noalias !84
  %332 = extractelement <2 x i32> %331, i64 0
  %idxprom.i3222 = sext i32 %332 to i64
  %arrayidx.i3223 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i3222
  store ptr %decimal_point, ptr %arrayidx.i3223, align 8, !tbaa !41, !noalias !84
  %iov_len.i3224 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i3222, i32 1
  store i64 1, ptr %iov_len.i3224, align 8, !tbaa !43, !noalias !84
  %333 = add nsw <2 x i32> %331, <i32 1, i32 1>
  store <2 x i32> %333, ptr %uio_iovcnt.i, align 8, !tbaa !5, !noalias !84
  %cmp.i3228 = icmp sgt i32 %332, 6
  br i1 %cmp.i3228, label %io_print.exit3233, label %if.end2248

io_print.exit3233:                                ; preds = %do.body2241
  %334 = load ptr, ptr %io, align 8, !tbaa !33, !noalias !84
  %call.i3232 = call i32 (ptr, ptr, ptr, ...) @__sprint(ptr noundef %334, ptr noundef nonnull %uio.i, ptr noundef %locale) #14
  %tobool2243.not = icmp eq i32 %call.i3232, 0
  br i1 %tobool2243.not, label %if.end2248, label %error

if.end2248:                                       ; preds = %do.body2241, %do.end2235, %io_print.exit3233
  %335 = load i32, ptr %expt, align 4, !tbaa !5
  %cmp8.i3235 = icmp slt i32 %335, 0
  br i1 %cmp8.i3235, label %while.body.i3240.preheader, label %if.end2253

while.body.i3240.preheader:                       ; preds = %if.end2248
  %sub2249 = sub nsw i32 0, %335
  br label %while.body.i3240

while.body.i3240:                                 ; preds = %while.body.i3240.preheader, %if.end.i3250
  %howmany.addr.09.i3241 = phi i32 [ %sub.i3251, %if.end.i3250 ], [ %sub2249, %while.body.i3240.preheader ]
  %cond.i3242 = call i32 @llvm.smin.i32(i32 %howmany.addr.09.i3241, i32 16)
  %336 = load i32, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !87
  %idxprom.i.i3243 = sext i32 %336 to i64
  %arrayidx.i.i3244 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i.i3243
  store ptr @zeroes, ptr %arrayidx.i.i3244, align 8, !tbaa !41, !noalias !87
  %conv.i.i3245 = zext nneg i32 %cond.i3242 to i64
  %iov_len.i.i3246 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i.i3243, i32 1
  store i64 %conv.i.i3245, ptr %iov_len.i.i3246, align 8, !tbaa !43, !noalias !87
  %337 = load i32, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !87
  %add.i.i3247 = add nsw i32 %337, %cond.i3242
  store i32 %add.i.i3247, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !87
  %inc.i.i3248 = add nsw i32 %336, 1
  store i32 %inc.i.i3248, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !87
  %cmp.i.i3249 = icmp sgt i32 %336, 6
  br i1 %cmp.i.i3249, label %io_print.exit.i3253, label %if.end.i3250

io_print.exit.i3253:                              ; preds = %while.body.i3240
  %338 = load ptr, ptr %io, align 8, !tbaa !33, !noalias !87
  %call.i.i3254 = call i32 (ptr, ptr, ptr, ...) @__sprint(ptr noundef %338, ptr noundef nonnull %uio.i, ptr noundef %locale) #14
  %tobool.not.i3255 = icmp eq i32 %call.i.i3254, 0
  br i1 %tobool.not.i3255, label %if.end.i3250, label %error

if.end.i3250:                                     ; preds = %io_print.exit.i3253, %while.body.i3240
  %sub.i3251 = sub nsw i32 %howmany.addr.09.i3241, %cond.i3242
  %cmp.i3252 = icmp sgt i32 %sub.i3251, 0
  br i1 %cmp.i3252, label %while.body.i3240, label %if.end2253.loopexit, !llvm.loop !61

if.end2253.loopexit:                              ; preds = %if.end.i3250
  %.pre4394 = load i32, ptr %expt, align 4, !tbaa !5
  br label %if.end2253

if.end2253:                                       ; preds = %if.end2253.loopexit, %if.end2248
  %339 = phi i32 [ %.pre4394, %if.end2253.loopexit ], [ %335, %if.end2248 ]
  %add2254 = add nsw i32 %339, %prec.11
  br label %if.end2290

if.else2255:                                      ; preds = %if.then2225
  %340 = load ptr, ptr %grouping, align 8, !tbaa !44
  %tobool2257.not = icmp eq ptr %340, null
  %idx.ext2268 = sext i32 %ndig.2 to i64
  %add.ptr2269 = getelementptr inbounds i32, ptr %convbuf.5, i64 %idx.ext2268
  br i1 %tobool2257.not, label %if.else2267, label %if.then2258

if.then2258:                                      ; preds = %if.else2255
  %call2260 = call fastcc i32 @ref_grouping_print(ptr noundef nonnull %gs, ptr noundef nonnull %io, ptr noundef %cp.15, ptr noundef %add.ptr2269, ptr noundef %locale)
  %cmp2261 = icmp slt i32 %call2260, 0
  br i1 %cmp2261, label %error, label %if.end2264

if.end2264:                                       ; preds = %if.then2258
  %idx.ext2265 = zext nneg i32 %call2260 to i64
  br label %if.end2276

if.else2267:                                      ; preds = %if.else2255
  %sub.ptr.lhs.cast.i3257 = ptrtoint ptr %add.ptr2269 to i64
  %sub.ptr.rhs.cast.i3258 = ptrtoint ptr %cp.15 to i64
  %sub.ptr.sub.i3259 = sub i64 %sub.ptr.lhs.cast.i3257, %sub.ptr.rhs.cast.i3258
  %sub.ptr.div.i3260 = lshr exact i64 %sub.ptr.sub.i3259, 2
  %conv.i3261 = trunc i64 %sub.ptr.div.i3260 to i32
  %spec.select.i = call i32 @llvm.smin.i32(i32 %conv.i3261, i32 %326)
  %cmp2.i = icmp sgt i32 %conv.i3261, 0
  br i1 %cmp2.i, label %if.then4.i, label %if.end7.i

if.then4.i:                                       ; preds = %if.else2267
  %341 = load i32, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !92
  %idxprom.i.i3265 = sext i32 %341 to i64
  %arrayidx.i.i3266 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i.i3265
  store ptr %cp.15, ptr %arrayidx.i.i3266, align 8, !tbaa !41, !noalias !92
  %conv.i.i3267 = zext nneg i32 %spec.select.i to i64
  %iov_len.i.i3268 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i.i3265, i32 1
  store i64 %conv.i.i3267, ptr %iov_len.i.i3268, align 8, !tbaa !43, !noalias !92
  %342 = load i32, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !92
  %add.i.i3270 = add nsw i32 %342, %spec.select.i
  store i32 %add.i.i3270, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !92
  %inc.i.i3271 = add nsw i32 %341, 1
  store i32 %inc.i.i3271, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !92
  %cmp.i.i3272 = icmp sgt i32 %341, 6
  br i1 %cmp.i.i3272, label %io_print.exit.i3273, label %if.end7.i

io_print.exit.i3273:                              ; preds = %if.then4.i
  %343 = load ptr, ptr %io, align 8, !tbaa !33, !noalias !92
  %call.i.i3275 = call i32 (ptr, ptr, ptr, ...) @__sprint(ptr noundef %343, ptr noundef nonnull %uio.i, ptr noundef %locale) #14
  %tobool.not.i3276 = icmp eq i32 %call.i.i3275, 0
  br i1 %tobool.not.i3276, label %if.end7.i, label %error

if.end7.i:                                        ; preds = %io_print.exit.i3273, %if.then4.i, %if.else2267
  %p_len.1.i = phi i32 [ %spec.select.i, %io_print.exit.i3273 ], [ 0, %if.else2267 ], [ %spec.select.i, %if.then4.i ]
  %sub.i3262 = sub nsw i32 %326, %p_len.1.i
  %cmp8.i.i = icmp sgt i32 %sub.i3262, 0
  br i1 %cmp8.i.i, label %while.body.i.i, label %if.end2273

while.body.i.i:                                   ; preds = %if.end7.i, %if.end.i.i
  %howmany.addr.09.i.i = phi i32 [ %sub.i.i, %if.end.i.i ], [ %sub.i3262, %if.end7.i ]
  %cond.i.i = call i32 @llvm.smin.i32(i32 %howmany.addr.09.i.i, i32 16)
  %344 = load i32, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !95
  %idxprom.i.i.i = sext i32 %344 to i64
  %arrayidx.i.i.i = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i.i.i
  store ptr @zeroes, ptr %arrayidx.i.i.i, align 8, !tbaa !41, !noalias !95
  %conv.i.i.i = zext nneg i32 %cond.i.i to i64
  %iov_len.i.i.i = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i.i.i, i32 1
  store i64 %conv.i.i.i, ptr %iov_len.i.i.i, align 8, !tbaa !43, !noalias !95
  %345 = load i32, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !95
  %add.i.i.i = add nsw i32 %345, %cond.i.i
  store i32 %add.i.i.i, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !95
  %inc.i.i.i = add nsw i32 %344, 1
  store i32 %inc.i.i.i, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !95
  %cmp.i.i.i = icmp sgt i32 %344, 6
  br i1 %cmp.i.i.i, label %io_print.exit.i.i, label %if.end.i.i

io_print.exit.i.i:                                ; preds = %while.body.i.i
  %346 = load ptr, ptr %io, align 8, !tbaa !33, !noalias !95
  %call.i.i.i = call i32 (ptr, ptr, ptr, ...) @__sprint(ptr noundef %346, ptr noundef nonnull %uio.i, ptr noundef %locale) #14
  %tobool.not.i.i = icmp eq i32 %call.i.i.i, 0
  br i1 %tobool.not.i.i, label %if.end.i.i, label %error

if.end.i.i:                                       ; preds = %io_print.exit.i.i, %while.body.i.i
  %sub.i.i = sub nsw i32 %howmany.addr.09.i.i, %cond.i.i
  %cmp.i18.i = icmp sgt i32 %sub.i.i, 0
  br i1 %cmp.i18.i, label %while.body.i.i, label %if.end2273, !llvm.loop !61

if.end2273:                                       ; preds = %if.end.i.i, %if.end7.i
  %347 = load i32, ptr %expt, align 4, !tbaa !5
  %idx.ext2274 = sext i32 %347 to i64
  br label %if.end2276

if.end2276:                                       ; preds = %if.end2273, %if.end2264
  %idx.ext2265.pn = phi i64 [ %idx.ext2265, %if.end2264 ], [ %idx.ext2274, %if.end2273 ]
  %cp.16 = getelementptr inbounds i32, ptr %cp.15, i64 %idx.ext2265.pn
  %tobool2277.not = icmp eq i32 %prec.11, 0
  %and2279 = and i32 %flags.16, 1
  %tobool2280.not = icmp eq i32 %and2279, 0
  %or.cond3039 = select i1 %tobool2277.not, i1 %tobool2280.not, i1 false
  br i1 %or.cond3039, label %if.end2290, label %do.body2282

do.body2282:                                      ; preds = %if.end2276
  %348 = load <2 x i32>, ptr %uio_iovcnt.i, align 8, !tbaa !5, !noalias !100
  %349 = extractelement <2 x i32> %348, i64 0
  %idxprom.i3278 = sext i32 %349 to i64
  %arrayidx.i3279 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i3278
  store ptr %decimal_point, ptr %arrayidx.i3279, align 8, !tbaa !41, !noalias !100
  %iov_len.i3280 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i3278, i32 1
  store i64 1, ptr %iov_len.i3280, align 8, !tbaa !43, !noalias !100
  %350 = add nsw <2 x i32> %348, <i32 1, i32 1>
  store <2 x i32> %350, ptr %uio_iovcnt.i, align 8, !tbaa !5, !noalias !100
  %cmp.i3284 = icmp sgt i32 %349, 6
  br i1 %cmp.i3284, label %io_print.exit3289, label %if.end2290

io_print.exit3289:                                ; preds = %do.body2282
  %351 = load ptr, ptr %io, align 8, !tbaa !33, !noalias !100
  %call.i3288 = call i32 (ptr, ptr, ptr, ...) @__sprint(ptr noundef %351, ptr noundef nonnull %uio.i, ptr noundef %locale) #14
  %tobool2284.not = icmp eq i32 %call.i3288, 0
  br i1 %tobool2284.not, label %if.end2290, label %error

if.end2290:                                       ; preds = %do.body2282, %if.end2276, %io_print.exit3289, %if.end2253
  %cp.17 = phi ptr [ %cp.15, %if.end2253 ], [ %cp.16, %io_print.exit3289 ], [ %cp.16, %if.end2276 ], [ %cp.16, %do.body2282 ]
  %prec.12 = phi i32 [ %add2254, %if.end2253 ], [ %prec.11, %io_print.exit3289 ], [ 0, %if.end2276 ], [ %prec.11, %do.body2282 ]
  %idx.ext2291 = sext i32 %ndig.2 to i64
  %add.ptr2292 = getelementptr inbounds i32, ptr %convbuf.5, i64 %idx.ext2291
  %sub.ptr.lhs.cast.i3290 = ptrtoint ptr %add.ptr2292 to i64
  %sub.ptr.rhs.cast.i3291 = ptrtoint ptr %cp.17 to i64
  %sub.ptr.sub.i3292 = sub i64 %sub.ptr.lhs.cast.i3290, %sub.ptr.rhs.cast.i3291
  %sub.ptr.div.i3293 = lshr exact i64 %sub.ptr.sub.i3292, 2
  %conv.i3294 = trunc i64 %sub.ptr.div.i3293 to i32
  %spec.select.i3295 = call i32 @llvm.smin.i32(i32 %conv.i3294, i32 %prec.12)
  %cmp2.i3296 = icmp sgt i32 %spec.select.i3295, 0
  br i1 %cmp2.i3296, label %if.then4.i3322, label %if.end7.i3297

if.then4.i3322:                                   ; preds = %if.end2290
  %352 = load i32, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !103
  %idxprom.i.i3324 = sext i32 %352 to i64
  %arrayidx.i.i3325 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i.i3324
  store ptr %cp.17, ptr %arrayidx.i.i3325, align 8, !tbaa !41, !noalias !103
  %conv.i.i3326 = zext nneg i32 %spec.select.i3295 to i64
  %iov_len.i.i3327 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i.i3324, i32 1
  store i64 %conv.i.i3326, ptr %iov_len.i.i3327, align 8, !tbaa !43, !noalias !103
  %353 = load i32, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !103
  %add.i.i3329 = add nsw i32 %353, %spec.select.i3295
  store i32 %add.i.i3329, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !103
  %inc.i.i3330 = add nsw i32 %352, 1
  store i32 %inc.i.i3330, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !103
  %cmp.i.i3331 = icmp sgt i32 %352, 6
  br i1 %cmp.i.i3331, label %io_print.exit.i3332, label %if.end7.i3297

io_print.exit.i3332:                              ; preds = %if.then4.i3322
  %354 = load ptr, ptr %io, align 8, !tbaa !33, !noalias !103
  %call.i.i3334 = call i32 (ptr, ptr, ptr, ...) @__sprint(ptr noundef %354, ptr noundef nonnull %uio.i, ptr noundef %locale) #14
  %tobool.not.i3335 = icmp eq i32 %call.i.i3334, 0
  br i1 %tobool.not.i3335, label %if.end7.i3297, label %error

if.end7.i3297:                                    ; preds = %io_print.exit.i3332, %if.then4.i3322, %if.end2290
  %p_len.1.i3298 = phi i32 [ %spec.select.i3295, %io_print.exit.i3332 ], [ 0, %if.end2290 ], [ %spec.select.i3295, %if.then4.i3322 ]
  %sub.i3299 = sub nsw i32 %prec.12, %p_len.1.i3298
  %cmp8.i.i3301 = icmp sgt i32 %sub.i3299, 0
  br i1 %cmp8.i.i3301, label %while.body.i.i3306, label %if.end2346

while.body.i.i3306:                               ; preds = %if.end7.i3297, %if.end.i.i3316
  %howmany.addr.09.i.i3307 = phi i32 [ %sub.i.i3317, %if.end.i.i3316 ], [ %sub.i3299, %if.end7.i3297 ]
  %cond.i.i3308 = call i32 @llvm.smin.i32(i32 %howmany.addr.09.i.i3307, i32 16)
  %355 = load i32, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !106
  %idxprom.i.i.i3309 = sext i32 %355 to i64
  %arrayidx.i.i.i3310 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i.i.i3309
  store ptr @zeroes, ptr %arrayidx.i.i.i3310, align 8, !tbaa !41, !noalias !106
  %conv.i.i.i3311 = zext nneg i32 %cond.i.i3308 to i64
  %iov_len.i.i.i3312 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i.i.i3309, i32 1
  store i64 %conv.i.i.i3311, ptr %iov_len.i.i.i3312, align 8, !tbaa !43, !noalias !106
  %356 = load i32, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !106
  %add.i.i.i3313 = add nsw i32 %356, %cond.i.i3308
  store i32 %add.i.i.i3313, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !106
  %inc.i.i.i3314 = add nsw i32 %355, 1
  store i32 %inc.i.i.i3314, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !106
  %cmp.i.i.i3315 = icmp sgt i32 %355, 6
  br i1 %cmp.i.i.i3315, label %io_print.exit.i.i3319, label %if.end.i.i3316

io_print.exit.i.i3319:                            ; preds = %while.body.i.i3306
  %357 = load ptr, ptr %io, align 8, !tbaa !33, !noalias !106
  %call.i.i.i3320 = call i32 (ptr, ptr, ptr, ...) @__sprint(ptr noundef %357, ptr noundef nonnull %uio.i, ptr noundef %locale) #14
  %tobool.not.i.i3321 = icmp eq i32 %call.i.i.i3320, 0
  br i1 %tobool.not.i.i3321, label %if.end.i.i3316, label %error

if.end.i.i3316:                                   ; preds = %io_print.exit.i.i3319, %while.body.i.i3306
  %sub.i.i3317 = sub nsw i32 %howmany.addr.09.i.i3307, %cond.i.i3308
  %cmp.i18.i3318 = icmp sgt i32 %sub.i.i3317, 0
  br i1 %cmp.i18.i3318, label %while.body.i.i3306, label %if.end2346, !llvm.loop !61

if.else2297:                                      ; preds = %if.else2223
  %cmp2298 = icmp slt i32 %prec.11, 2
  %and2301 = and i32 %flags.16, 1
  %tobool2302.not = icmp eq i32 %and2301, 0
  %or.cond3040 = select i1 %cmp2298, i1 %tobool2302.not, i1 false
  br i1 %or.cond3040, label %do.body2329, label %if.then2303

if.then2303:                                      ; preds = %if.else2297
  %358 = load i32, ptr %cp.15, align 4, !tbaa !5
  store i32 %358, ptr %buf, align 16, !tbaa !5
  %359 = load i32, ptr %decimal_point, align 4, !tbaa !5
  store i32 %359, ptr %arrayidx2306, align 4, !tbaa !5
  %360 = load i32, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !111
  %idxprom.i3338 = sext i32 %360 to i64
  %arrayidx.i3339 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i3338
  store ptr %buf, ptr %arrayidx.i3339, align 8, !tbaa !41, !noalias !111
  %iov_len.i3340 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i3338, i32 1
  store i64 2, ptr %iov_len.i3340, align 8, !tbaa !43, !noalias !111
  %361 = load i32, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !111
  %add.i3342 = add nsw i32 %361, 2
  store i32 %add.i3342, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !111
  %inc.i3343 = add nsw i32 %360, 1
  store i32 %inc.i3343, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !111
  %cmp.i3344 = icmp sgt i32 %360, 6
  br i1 %cmp.i3344, label %io_print.exit3349, label %do.body2315

io_print.exit3349:                                ; preds = %if.then2303
  %362 = load ptr, ptr %io, align 8, !tbaa !33, !noalias !111
  %call.i3348 = call i32 (ptr, ptr, ptr, ...) @__sprint(ptr noundef %362, ptr noundef nonnull %uio.i, ptr noundef %locale) #14
  %tobool2310.not = icmp eq i32 %call.i3348, 0
  br i1 %tobool2310.not, label %do.body2315thread-pre-split, label %error

do.body2315thread-pre-split:                      ; preds = %io_print.exit3349
  %.pr3522 = load i32, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !114
  %.pre4392 = load i32, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !114
  br label %do.body2315

do.body2315:                                      ; preds = %if.then2303, %do.body2315thread-pre-split
  %363 = phi i32 [ %.pre4392, %do.body2315thread-pre-split ], [ %add.i3342, %if.then2303 ]
  %364 = phi i32 [ %.pr3522, %do.body2315thread-pre-split ], [ %inc.i3343, %if.then2303 ]
  %incdec.ptr2304 = getelementptr inbounds i32, ptr %cp.15, i64 1
  %sub2316 = add nsw i32 %ndig.2, -1
  %idxprom.i3351 = sext i32 %364 to i64
  %arrayidx.i3352 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i3351
  store ptr %incdec.ptr2304, ptr %arrayidx.i3352, align 8, !tbaa !41, !noalias !114
  %conv.i3353 = sext i32 %sub2316 to i64
  %iov_len.i3354 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i3351, i32 1
  store i64 %conv.i3353, ptr %iov_len.i3354, align 8, !tbaa !43, !noalias !114
  %add.i3356 = add nsw i32 %363, %sub2316
  store i32 %add.i3356, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !114
  %inc.i3357 = add nsw i32 %364, 1
  store i32 %inc.i3357, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !114
  %cmp.i3358 = icmp sgt i32 %364, 6
  br i1 %cmp.i3358, label %io_print.exit3363, label %do.end2322

io_print.exit3363:                                ; preds = %do.body2315
  %365 = load ptr, ptr %io, align 8, !tbaa !33, !noalias !114
  %call.i3362 = call i32 (ptr, ptr, ptr, ...) @__sprint(ptr noundef %365, ptr noundef nonnull %uio.i, ptr noundef %locale) #14
  %tobool2318.not = icmp eq i32 %call.i3362, 0
  br i1 %tobool2318.not, label %do.end2322, label %error

do.end2322:                                       ; preds = %do.body2315, %io_print.exit3363
  %sub2323 = sub nsw i32 %prec.11, %ndig.2
  %cmp8.i3365 = icmp sgt i32 %sub2323, 0
  br i1 %cmp8.i3365, label %while.body.i3370, label %do.body2337thread-pre-split

while.body.i3370:                                 ; preds = %do.end2322, %if.end.i3380
  %howmany.addr.09.i3371 = phi i32 [ %sub.i3381, %if.end.i3380 ], [ %sub2323, %do.end2322 ]
  %cond.i3372 = call i32 @llvm.smin.i32(i32 %howmany.addr.09.i3371, i32 16)
  %366 = load i32, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !117
  %idxprom.i.i3373 = sext i32 %366 to i64
  %arrayidx.i.i3374 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i.i3373
  store ptr @zeroes, ptr %arrayidx.i.i3374, align 8, !tbaa !41, !noalias !117
  %conv.i.i3375 = zext nneg i32 %cond.i3372 to i64
  %iov_len.i.i3376 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i.i3373, i32 1
  store i64 %conv.i.i3375, ptr %iov_len.i.i3376, align 8, !tbaa !43, !noalias !117
  %367 = load i32, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !117
  %add.i.i3377 = add nsw i32 %367, %cond.i3372
  store i32 %add.i.i3377, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !117
  %inc.i.i3378 = add nsw i32 %366, 1
  store i32 %inc.i.i3378, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !117
  %cmp.i.i3379 = icmp sgt i32 %366, 6
  br i1 %cmp.i.i3379, label %io_print.exit.i3383, label %if.end.i3380

io_print.exit.i3383:                              ; preds = %while.body.i3370
  %368 = load ptr, ptr %io, align 8, !tbaa !33, !noalias !117
  %call.i.i3384 = call i32 (ptr, ptr, ptr, ...) @__sprint(ptr noundef %368, ptr noundef nonnull %uio.i, ptr noundef %locale) #14
  %tobool.not.i3385 = icmp eq i32 %call.i.i3384, 0
  br i1 %tobool.not.i3385, label %if.end.i3380, label %error

if.end.i3380:                                     ; preds = %io_print.exit.i3383, %while.body.i3370
  %sub.i3381 = sub nsw i32 %howmany.addr.09.i3371, %cond.i3372
  %cmp.i3382 = icmp sgt i32 %sub.i3381, 0
  br i1 %cmp.i3382, label %while.body.i3370, label %do.body2337thread-pre-split, !llvm.loop !61

do.body2329:                                      ; preds = %if.else2297
  %369 = load i32, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !122
  %idxprom.i3388 = sext i32 %369 to i64
  %arrayidx.i3389 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i3388
  store ptr %cp.15, ptr %arrayidx.i3389, align 8, !tbaa !41, !noalias !122
  %iov_len.i3390 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i3388, i32 1
  store i64 1, ptr %iov_len.i3390, align 8, !tbaa !43, !noalias !122
  %370 = load i32, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !122
  %add.i3392 = add nsw i32 %370, 1
  store i32 %add.i3392, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !122
  %inc.i3393 = add nsw i32 %369, 1
  store i32 %inc.i3393, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !122
  %cmp.i3394 = icmp sgt i32 %369, 6
  br i1 %cmp.i3394, label %io_print.exit3399, label %do.body2337

io_print.exit3399:                                ; preds = %do.body2329
  %371 = load ptr, ptr %io, align 8, !tbaa !33, !noalias !122
  %call.i3398 = call i32 (ptr, ptr, ptr, ...) @__sprint(ptr noundef %371, ptr noundef nonnull %uio.i, ptr noundef %locale) #14
  %tobool2331.not = icmp eq i32 %call.i3398, 0
  br i1 %tobool2331.not, label %do.body2337thread-pre-split, label %error

do.body2337thread-pre-split:                      ; preds = %if.end.i3380, %do.end2322, %io_print.exit3399
  %.pr3529 = load i32, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !125
  %.pre4393 = load i32, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !125
  br label %do.body2337

do.body2337:                                      ; preds = %do.body2329, %do.body2337thread-pre-split
  %372 = phi i32 [ %.pre4393, %do.body2337thread-pre-split ], [ %add.i3392, %do.body2329 ]
  %373 = phi i32 [ %.pr3529, %do.body2337thread-pre-split ], [ %inc.i3393, %do.body2329 ]
  %idxprom.i3401 = sext i32 %373 to i64
  %arrayidx.i3402 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i3401
  store ptr %expstr, ptr %arrayidx.i3402, align 8, !tbaa !41, !noalias !125
  %conv.i3403 = sext i32 %expsize.1 to i64
  %iov_len.i3404 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i3401, i32 1
  store i64 %conv.i3403, ptr %iov_len.i3404, align 8, !tbaa !43, !noalias !125
  %add.i3406 = add nsw i32 %372, %expsize.1
  store i32 %add.i3406, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !125
  %inc.i3407 = add nsw i32 %373, 1
  store i32 %inc.i3407, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !125
  %cmp.i3408 = icmp sgt i32 %373, 6
  br i1 %cmp.i3408, label %io_print.exit3413, label %if.end2346

io_print.exit3413:                                ; preds = %do.body2337
  %374 = load ptr, ptr %io, align 8, !tbaa !33, !noalias !125
  %call.i3412 = call i32 (ptr, ptr, ptr, ...) @__sprint(ptr noundef %374, ptr noundef nonnull %uio.i, ptr noundef %locale) #14
  %tobool2340.not = icmp eq i32 %call.i3412, 0
  br i1 %tobool2340.not, label %if.end2346, label %error

if.end2346:                                       ; preds = %if.end.i.i3316, %do.body2337, %if.end7.i3297, %do.body2215, %io_print.exit3413, %if.then2206, %io_print.exit3207
  %and2347 = and i32 %flags.16, 4
  %tobool2348.not = icmp ne i32 %and2347, 0
  %or.cond3545 = select i1 %tobool2348.not, i1 %cmp8.i, i1 false
  br i1 %or.cond3545, label %while.body.i3420, label %if.end2355

while.body.i3420:                                 ; preds = %if.end2346, %if.end.i3430
  %howmany.addr.09.i3421 = phi i32 [ %sub.i3431, %if.end.i3430 ], [ %sub2156, %if.end2346 ]
  %cond.i3422 = call i32 @llvm.smin.i32(i32 %howmany.addr.09.i3421, i32 16)
  %375 = load i32, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !128
  %idxprom.i.i3423 = sext i32 %375 to i64
  %arrayidx.i.i3424 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i.i3423
  store ptr @blanks, ptr %arrayidx.i.i3424, align 8, !tbaa !41, !noalias !128
  %conv.i.i3425 = zext nneg i32 %cond.i3422 to i64
  %iov_len.i.i3426 = getelementptr inbounds %struct.io_state, ptr %io, i64 0, i32 2, i64 %idxprom.i.i3423, i32 1
  store i64 %conv.i.i3425, ptr %iov_len.i.i3426, align 8, !tbaa !43, !noalias !128
  %376 = load i32, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !128
  %add.i.i3427 = add nsw i32 %376, %cond.i3422
  store i32 %add.i.i3427, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !128
  %inc.i.i3428 = add nsw i32 %375, 1
  store i32 %inc.i.i3428, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !128
  %cmp.i.i3429 = icmp sgt i32 %375, 6
  br i1 %cmp.i.i3429, label %io_print.exit.i3433, label %if.end.i3430

io_print.exit.i3433:                              ; preds = %while.body.i3420
  %377 = load ptr, ptr %io, align 8, !tbaa !33, !noalias !128
  %call.i.i3434 = call i32 (ptr, ptr, ptr, ...) @__sprint(ptr noundef %377, ptr noundef nonnull %uio.i, ptr noundef %locale) #14
  %tobool.not.i3435 = icmp eq i32 %call.i.i3434, 0
  br i1 %tobool.not.i3435, label %if.end.i3430, label %error

if.end.i3430:                                     ; preds = %io_print.exit.i3433, %while.body.i3420
  %sub.i3431 = sub nsw i32 %howmany.addr.09.i3421, %cond.i3422
  %cmp.i3432 = icmp sgt i32 %sub.i3431, 0
  br i1 %cmp.i3432, label %while.body.i3420, label %if.end2355, !llvm.loop !61

if.end2355:                                       ; preds = %if.end.i3430, %if.end2346
  %378 = load ptr, ptr %io, align 8, !tbaa !33
  %call.i3438 = call i32 (ptr, ptr, ptr, ...) @__sprint(ptr noundef %378, ptr noundef nonnull %uio.i, ptr noundef %locale) #14
  %tobool2358.not = icmp eq i32 %call.i3438, 0
  br i1 %tobool2358.not, label %for.cond.outer, label %error

done:                                             ; preds = %sw.default, %if.end42
  %379 = load ptr, ptr %io, align 8, !tbaa !33
  %call.i3440 = call i32 (ptr, ptr, ptr, ...) @__sprint(ptr noundef %379, ptr noundef nonnull %uio.i, ptr noundef %locale) #14
  br label %error

error:                                            ; preds = %io_print.exit.i3332, %io_print.exit.i3273, %if.end2355, %io_print.exit3413, %io_print.exit3399, %io_print.exit3363, %io_print.exit3349, %io_print.exit3289, %if.then2258, %io_print.exit3233, %io_print.exit3220, %io_print.exit3207, %if.then2206, %io_print.exit3147, %io_print.exit3134, %io_print.exit, %io_print.exit.i, %io_print.exit.i3167, %io_print.exit.i3383, %io_print.exit.i.i, %io_print.exit.i3253, %io_print.exit.i.i3319, %io_print.exit.i3190, %io_print.exit.i3433, %if.then130, %if.then64, %if.then215, %done, %if.then2149, %if.then34
  %ret.9 = phi i32 [ -1, %if.then34 ], [ %ret.1, %done ], [ -1, %if.then2149 ], [ -1, %if.then215 ], [ -1, %if.then64 ], [ -1, %if.then130 ], [ %ret.1, %io_print.exit.i3433 ], [ %ret.1, %io_print.exit.i3190 ], [ %ret.1, %io_print.exit.i.i3319 ], [ %ret.1, %io_print.exit.i3253 ], [ %ret.1, %io_print.exit.i.i ], [ %ret.1, %io_print.exit.i3383 ], [ %ret.1, %io_print.exit.i3167 ], [ %ret.1, %io_print.exit.i ], [ %ret.0, %io_print.exit ], [ %ret.1, %io_print.exit3134 ], [ %ret.1, %io_print.exit3147 ], [ %ret.1, %if.then2206 ], [ %add2146, %if.end2355 ], [ %ret.1, %io_print.exit3207 ], [ %ret.1, %io_print.exit3349 ], [ %ret.1, %io_print.exit3363 ], [ %ret.1, %io_print.exit3413 ], [ %ret.1, %io_print.exit3399 ], [ %ret.1, %io_print.exit3220 ], [ %ret.1, %io_print.exit3233 ], [ %ret.1, %if.then2258 ], [ %ret.1, %io_print.exit3289 ], [ %ret.1, %io_print.exit.i3273 ], [ %ret.1, %io_print.exit.i3332 ]
  %convbuf.6 = phi ptr [ %convbuf.0.ph, %if.then34 ], [ %convbuf.0.ph, %done ], [ %convbuf.5, %if.then2149 ], [ %convbuf.0.ph, %if.then215 ], [ %convbuf.0.ph, %if.then64 ], [ %convbuf.0.ph, %if.then130 ], [ %convbuf.5, %io_print.exit.i3433 ], [ %convbuf.5, %io_print.exit.i3190 ], [ %convbuf.5, %io_print.exit.i.i3319 ], [ %convbuf.5, %io_print.exit.i3253 ], [ %convbuf.5, %io_print.exit.i.i ], [ %convbuf.5, %io_print.exit.i3383 ], [ %convbuf.5, %io_print.exit.i3167 ], [ %convbuf.5, %io_print.exit.i ], [ %convbuf.0.ph, %io_print.exit ], [ %convbuf.5, %io_print.exit3134 ], [ %convbuf.5, %io_print.exit3147 ], [ %convbuf.5, %if.then2206 ], [ %convbuf.5, %io_print.exit3207 ], [ %convbuf.5, %io_print.exit3220 ], [ %convbuf.5, %io_print.exit3233 ], [ %convbuf.5, %if.then2258 ], [ %convbuf.5, %io_print.exit3289 ], [ %convbuf.5, %io_print.exit3349 ], [ %convbuf.5, %io_print.exit3363 ], [ %convbuf.5, %io_print.exit3399 ], [ %convbuf.5, %io_print.exit3413 ], [ %convbuf.5, %if.end2355 ], [ %convbuf.5, %io_print.exit.i3273 ], [ %convbuf.5, %io_print.exit.i3332 ]
  call void @llvm.va_end(ptr nonnull %orgap)
  %cmp2366.not = icmp eq ptr %convbuf.6, null
  br i1 %cmp2366.not, label %if.end2369, label %if.then2368

if.then2368:                                      ; preds = %error
  call void @free(ptr noundef nonnull %convbuf.6)
  br label %if.end2369

if.end2369:                                       ; preds = %ref___mbsconv.exit3115.thread, %if.then2368, %error
  %ret.93537 = phi i32 [ %ret.1, %ref___mbsconv.exit3115.thread ], [ %ret.9, %if.then2368 ], [ %ret.9, %error ]
  %380 = load i16, ptr %_flags, align 8, !tbaa !9
  %381 = and i16 %380, 64
  %cmp2373.not = icmp eq i16 %381, 0
  br i1 %cmp2373.not, label %if.else2376, label %if.end2381

if.else2376:                                      ; preds = %if.end2369
  %or2379 = or disjoint i16 %380, %3
  store i16 %or2379, ptr %_flags, align 8, !tbaa !9
  br label %if.end2381

if.end2381:                                       ; preds = %if.end2369, %if.else2376
  %ret.10 = phi i32 [ %ret.93537, %if.else2376 ], [ -1, %if.end2369 ]
  %382 = load ptr, ptr %argtable, align 8
  %cmp2382 = icmp ne ptr %382, null
  %cmp2386 = icmp ne ptr %382, %statargtable
  %or.cond2437 = and i1 %cmp2382, %cmp2386
  br i1 %or.cond2437, label %if.then2388, label %cleanup2390

if.then2388:                                      ; preds = %if.end2381
  call void @free(ptr noundef nonnull %382)
  br label %cleanup2390

cleanup2390:                                      ; preds = %if.end2381, %if.then2388, %if.then
  %retval.0 = phi i32 [ -1, %if.then ], [ %ret.10, %if.then2388 ], [ %ret.10, %if.end2381 ]
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %orgap) #14
  call void @llvm.lifetime.end.p0(i64 128, ptr nonnull %statargtable) #14
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %argtable) #14
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %ox) #14
  call void @llvm.lifetime.end.p0(i64 256, ptr nonnull %buf) #14
  call void @llvm.lifetime.end.p0(i64 152, ptr nonnull %io) #14
  call void @llvm.lifetime.end.p0(i64 32, ptr nonnull %expstr) #14
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %dtoaend) #14
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %expt) #14
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %fparg) #14
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %signflag) #14
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %decimal_point) #14
  call void @llvm.lifetime.end.p0(i64 32, ptr nonnull %gs) #14
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %sign) #14
  ret i32 %retval.0
}

declare i32 @__swsetup(ptr noundef) local_unnamed_addr #2

declare ptr @__error() local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.va_copy(ptr, ptr) #4

declare i32 @__find_warguments(ptr noundef, ptr noundef, ptr noundef) local_unnamed_addr #2

declare i32 @btowc(i32 noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.ptrmask.p0.i64(ptr, i64) #5

declare ptr @__hldtoa(x86_fp80 noundef, ptr noundef, i32 noundef, ptr noundef, ptr noundef, ptr noundef) local_unnamed_addr #2

declare ptr @__hdtoa(double noundef, ptr noundef, i32 noundef, ptr noundef, ptr noundef, ptr noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite)
declare void @free(ptr allocptr nocapture noundef) local_unnamed_addr #6

declare void @__freedtoa(ptr noundef) local_unnamed_addr #2

declare ptr @__ldtoa(ptr noundef, i32 noundef, i32 noundef, ptr noundef, ptr noundef, ptr noundef) local_unnamed_addr #2

declare ptr @__dtoa(double noundef, i32 noundef, i32 noundef, ptr noundef, ptr noundef, ptr noundef) local_unnamed_addr #2

; Function Attrs: nounwind uwtable
define internal fastcc i32 @ref_grouping_init(ptr nocapture noundef %gs, i32 noundef %ndigits, ptr noundef %locale) unnamed_addr #0 {
entry:
  %mbs.i = alloca %union.__mbstate_t, align 8
  %thousep.i = alloca i32, align 4
  %call = tail call ptr @localeconv_l(ptr noundef %locale) #14
  %grouping = getelementptr inbounds %struct.lconv, ptr %call, i64 0, i32 2
  %0 = load ptr, ptr %grouping, align 8, !tbaa !133
  %grouping1 = getelementptr inbounds %struct.grouping_state, ptr %gs, i64 0, i32 1
  store ptr %0, ptr %grouping1, align 8, !tbaa !44
  call void @llvm.lifetime.start.p0(i64 128, ptr nonnull %mbs.i) #14
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %thousep.i) #14
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(128) %mbs.i, i8 0, i64 128, i1 false)
  %call.i = tail call ptr @localeconv_l(ptr noundef %locale) #14
  %thousands_sep.i = getelementptr inbounds %struct.lconv, ptr %call.i, i64 0, i32 1
  %1 = load ptr, ptr %thousands_sep.i, align 8, !tbaa !134
  %call1.i = tail call i32 @___mb_cur_max() #14
  %conv.i = sext i32 %call1.i to i64
  %call2.i = call i64 @mbrtowc(ptr noundef nonnull %thousep.i, ptr noundef %1, i64 noundef %conv.i, ptr noundef nonnull %mbs.i) #14
  %2 = and i64 %call2.i, 4294967294
  %or.cond.i = icmp eq i64 %2, 4294967294
  %.pre.i = load i32, ptr %thousep.i, align 4
  %3 = select i1 %or.cond.i, i32 0, i32 %.pre.i
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %thousep.i) #14
  call void @llvm.lifetime.end.p0(i64 128, ptr nonnull %mbs.i) #14
  store i32 %3, ptr %gs, align 8, !tbaa !135
  %nrepeats = getelementptr inbounds %struct.grouping_state, ptr %gs, i64 0, i32 4
  store i32 0, ptr %nrepeats, align 8, !tbaa !136
  %nseps = getelementptr inbounds %struct.grouping_state, ptr %gs, i64 0, i32 3
  store i32 0, ptr %nseps, align 4, !tbaa !137
  %lead = getelementptr inbounds %struct.grouping_state, ptr %gs, i64 0, i32 2
  store i32 %ndigits, ptr %lead, align 8, !tbaa !138
  %grouping1.promoted = load ptr, ptr %grouping1, align 8, !tbaa !44
  %4 = load i8, ptr %grouping1.promoted, align 1, !tbaa !21
  %cmp.not41 = icmp eq i8 %4, 127
  br i1 %cmp.not41, label %while.end, label %while.body

while.body:                                       ; preds = %entry, %if.end19
  %5 = phi i32 [ %8, %if.end19 ], [ 0, %entry ]
  %6 = phi i32 [ %9, %if.end19 ], [ 0, %entry ]
  %sub44 = phi i32 [ %sub, %if.end19 ], [ %ndigits, %entry ]
  %conv43.in = phi i8 [ %10, %if.end19 ], [ %4, %entry ]
  %add.ptr3942 = phi ptr [ %add.ptr38, %if.end19 ], [ %grouping1.promoted, %entry ]
  %conv43 = sext i8 %conv43.in to i32
  %cmp8.not = icmp sgt i32 %sub44, %conv43
  br i1 %cmp8.not, label %if.end, label %while.end.loopexit

if.end:                                           ; preds = %while.body
  %sub = sub nsw i32 %sub44, %conv43
  store i32 %sub, ptr %lead, align 8, !tbaa !138
  %add.ptr = getelementptr inbounds i8, ptr %add.ptr3942, i64 1
  %7 = load i8, ptr %add.ptr, align 1, !tbaa !21
  %tobool.not = icmp eq i8 %7, 0
  br i1 %tobool.not, label %if.else, label %if.then14

if.then14:                                        ; preds = %if.end
  %inc = add nsw i32 %6, 1
  store i32 %inc, ptr %nseps, align 4, !tbaa !137
  store ptr %add.ptr, ptr %grouping1, align 8, !tbaa !44
  br label %if.end19

if.else:                                          ; preds = %if.end
  %inc18 = add nsw i32 %5, 1
  store i32 %inc18, ptr %nrepeats, align 8, !tbaa !136
  br label %if.end19

if.end19:                                         ; preds = %if.else, %if.then14
  %8 = phi i32 [ %inc18, %if.else ], [ %5, %if.then14 ]
  %9 = phi i32 [ %6, %if.else ], [ %inc, %if.then14 ]
  %add.ptr38 = phi ptr [ %add.ptr3942, %if.else ], [ %add.ptr, %if.then14 ]
  %10 = load i8, ptr %add.ptr38, align 1, !tbaa !21
  %cmp.not = icmp eq i8 %10, 127
  br i1 %cmp.not, label %while.end.loopexit, label %while.body, !llvm.loop !139

while.end.loopexit:                               ; preds = %while.body, %if.end19
  %11 = phi i32 [ %5, %while.body ], [ %8, %if.end19 ]
  %12 = phi i32 [ %6, %while.body ], [ %9, %if.end19 ]
  %13 = add nsw i32 %11, %12
  br label %while.end

while.end:                                        ; preds = %while.end.loopexit, %entry
  %add = phi i32 [ %13, %while.end.loopexit ], [ 0, %entry ]
  ret i32 %add
}

; Function Attrs: mustprogress nofree nounwind willreturn memory(read)
declare i64 @wcsnlen(ptr noundef, i64 noundef) local_unnamed_addr #7

; Function Attrs: mustprogress nofree nounwind willreturn memory(argmem: read)
declare i64 @wcslen(ptr nocapture noundef) local_unnamed_addr #8

; Function Attrs: nounwind uwtable
define internal fastcc noundef nonnull ptr @__ultoa(i64 noundef %val, ptr noundef writeonly %endp, i32 noundef %base, i32 noundef %octzero, ptr nocapture noundef readonly %xdigs) unnamed_addr #0 {
entry:
  %0 = add nsw i32 %base, -2
  %1 = tail call i32 @llvm.fshl.i32(i32 %0, i32 %0, i32 31)
  switch i32 %1, label %sw.default [
    i32 4, label %sw.bb
    i32 0, label %do.body16
    i32 3, label %do.body23
    i32 7, label %do.body39
  ]

sw.bb:                                            ; preds = %entry
  %cmp = icmp ult i64 %val, 10
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %sw.bb
  %2 = trunc i64 %val to i32
  %conv = or disjoint i32 %2, 48
  %incdec.ptr = getelementptr inbounds i32, ptr %endp, i64 -1
  store i32 %conv, ptr %incdec.ptr, align 4, !tbaa !5
  br label %cleanup

if.end:                                           ; preds = %sw.bb
  %cmp1 = icmp slt i64 %val, 0
  br i1 %cmp1, label %if.then3, label %do.body.preheader

if.then3:                                         ; preds = %if.end
  %rem = urem i64 %val, 10
  %3 = trunc i64 %rem to i32
  %conv5 = or disjoint i32 %3, 48
  %incdec.ptr6 = getelementptr inbounds i32, ptr %endp, i64 -1
  store i32 %conv5, ptr %incdec.ptr6, align 4, !tbaa !5
  %div = udiv i64 %val, 10
  br label %do.body.preheader

do.body.preheader:                                ; preds = %if.end, %if.then3
  %cp.1.ph = phi ptr [ %endp, %if.end ], [ %incdec.ptr6, %if.then3 ]
  %sval.1.ph = phi i64 [ %val, %if.end ], [ %div, %if.then3 ]
  br label %do.body

do.body:                                          ; preds = %do.body.preheader, %do.body
  %cp.1 = phi ptr [ %incdec.ptr11, %do.body ], [ %cp.1.ph, %do.body.preheader ]
  %sval.1 = phi i64 [ %div12, %do.body ], [ %sval.1.ph, %do.body.preheader ]
  %rem8 = srem i64 %sval.1, 10
  %4 = trunc i64 %rem8 to i32
  %conv10 = add nsw i32 %4, 48
  %incdec.ptr11 = getelementptr inbounds i32, ptr %cp.1, i64 -1
  store i32 %conv10, ptr %incdec.ptr11, align 4, !tbaa !5
  %div12 = sdiv i64 %sval.1, 10
  %sval.1.off = add i64 %sval.1, 9
  %cmp13.not = icmp ult i64 %sval.1.off, 19
  br i1 %cmp13.not, label %cleanup, label %do.body, !llvm.loop !140

do.body16:                                        ; preds = %entry, %do.body16
  %val.addr.0 = phi i64 [ %shr, %do.body16 ], [ %val, %entry ]
  %cp.2 = phi ptr [ %incdec.ptr19, %do.body16 ], [ %endp, %entry ]
  %5 = trunc i64 %val.addr.0 to i32
  %6 = and i32 %5, 1
  %conv18 = or disjoint i32 %6, 48
  %incdec.ptr19 = getelementptr inbounds i32, ptr %cp.2, i64 -1
  store i32 %conv18, ptr %incdec.ptr19, align 4, !tbaa !5
  %shr = lshr i64 %val.addr.0, 1
  %tobool.not = icmp ult i64 %val.addr.0, 2
  br i1 %tobool.not, label %cleanup, label %do.body16, !llvm.loop !141

do.body23:                                        ; preds = %entry, %do.body23
  %val.addr.1 = phi i64 [ %shr28, %do.body23 ], [ %val, %entry ]
  %cp.3 = phi ptr [ %incdec.ptr27, %do.body23 ], [ %endp, %entry ]
  %7 = trunc i64 %val.addr.1 to i32
  %8 = and i32 %7, 7
  %conv26 = or disjoint i32 %8, 48
  %incdec.ptr27 = getelementptr inbounds i32, ptr %cp.3, i64 -1
  store i32 %conv26, ptr %incdec.ptr27, align 4, !tbaa !5
  %shr28 = lshr i64 %val.addr.1, 3
  %tobool30.not = icmp ult i64 %val.addr.1, 8
  br i1 %tobool30.not, label %do.end31, label %do.body23, !llvm.loop !142

do.end31:                                         ; preds = %do.body23
  %tobool32.not = icmp eq i32 %octzero, 0
  %cmp33.not = icmp eq i32 %8, 0
  %or.cond = or i1 %tobool32.not, %cmp33.not
  br i1 %or.cond, label %cleanup, label %if.then35

if.then35:                                        ; preds = %do.end31
  %incdec.ptr36 = getelementptr inbounds i32, ptr %cp.3, i64 -2
  store i32 48, ptr %incdec.ptr36, align 4, !tbaa !5
  br label %cleanup

do.body39:                                        ; preds = %entry, %do.body39
  %val.addr.2 = phi i64 [ %shr43, %do.body39 ], [ %val, %entry ]
  %cp.4 = phi ptr [ %incdec.ptr42, %do.body39 ], [ %endp, %entry ]
  %and40 = and i64 %val.addr.2, 15
  %arrayidx = getelementptr inbounds i8, ptr %xdigs, i64 %and40
  %9 = load i8, ptr %arrayidx, align 1, !tbaa !21
  %conv41 = sext i8 %9 to i32
  %incdec.ptr42 = getelementptr inbounds i32, ptr %cp.4, i64 -1
  store i32 %conv41, ptr %incdec.ptr42, align 4, !tbaa !5
  %shr43 = lshr i64 %val.addr.2, 4
  %tobool45.not = icmp ult i64 %val.addr.2, 16
  br i1 %tobool45.not, label %cleanup, label %do.body39, !llvm.loop !143

sw.default:                                       ; preds = %entry
  tail call void @abort() #17
  unreachable

cleanup:                                          ; preds = %do.body39, %do.body16, %do.body, %if.then35, %do.end31, %if.then
  %retval.0 = phi ptr [ %incdec.ptr, %if.then ], [ %incdec.ptr36, %if.then35 ], [ %incdec.ptr27, %do.end31 ], [ %incdec.ptr11, %do.body ], [ %incdec.ptr19, %do.body16 ], [ %incdec.ptr42, %do.body39 ]
  ret ptr %retval.0
}

; Function Attrs: noreturn
declare void @abort() local_unnamed_addr #9

; Function Attrs: nounwind uwtable
define internal fastcc i32 @ref_grouping_print(ptr noundef %gs, ptr noundef %iop, ptr noundef %cp, ptr noundef %ep, ptr noundef %locale) unnamed_addr #0 {
entry:
  %lead = getelementptr inbounds %struct.grouping_state, ptr %gs, i64 0, i32 2
  %0 = load i32, ptr %lead, align 8, !tbaa !138
  %sub.ptr.lhs.cast.i = ptrtoint ptr %ep to i64
  %sub.ptr.rhs.cast.i = ptrtoint ptr %cp to i64
  %sub.ptr.sub.i = sub i64 %sub.ptr.lhs.cast.i, %sub.ptr.rhs.cast.i
  %sub.ptr.div.i = lshr exact i64 %sub.ptr.sub.i, 2
  %conv.i = trunc i64 %sub.ptr.div.i to i32
  %spec.select.i = tail call i32 @llvm.smin.i32(i32 %conv.i, i32 %0)
  %cmp2.i = icmp sgt i32 %spec.select.i, 0
  br i1 %cmp2.i, label %if.then4.i, label %if.end7.i

if.then4.i:                                       ; preds = %entry
  %uio_iovcnt.i.i = getelementptr inbounds %struct.io_state, ptr %iop, i64 0, i32 1, i32 1
  %1 = load i32, ptr %uio_iovcnt.i.i, align 8, !tbaa !32, !noalias !144
  %idxprom.i.i = sext i32 %1 to i64
  %arrayidx.i.i = getelementptr inbounds %struct.io_state, ptr %iop, i64 0, i32 2, i64 %idxprom.i.i
  store ptr %cp, ptr %arrayidx.i.i, align 8, !tbaa !41, !noalias !144
  %conv.i.i = zext nneg i32 %spec.select.i to i64
  %iov_len.i.i = getelementptr inbounds %struct.io_state, ptr %iop, i64 0, i32 2, i64 %idxprom.i.i, i32 1
  store i64 %conv.i.i, ptr %iov_len.i.i, align 8, !tbaa !43, !noalias !144
  %uio_resid.i.i = getelementptr inbounds %struct.io_state, ptr %iop, i64 0, i32 1, i32 2
  %2 = load i32, ptr %uio_resid.i.i, align 4, !tbaa !31, !noalias !144
  %add.i.i = add nsw i32 %2, %spec.select.i
  store i32 %add.i.i, ptr %uio_resid.i.i, align 4, !tbaa !31, !noalias !144
  %inc.i.i = add nsw i32 %1, 1
  store i32 %inc.i.i, ptr %uio_iovcnt.i.i, align 8, !tbaa !32, !noalias !144
  %cmp.i.i = icmp sgt i32 %1, 6
  br i1 %cmp.i.i, label %io_print.exit.i, label %if.end7.i

io_print.exit.i:                                  ; preds = %if.then4.i
  %uio.i.i = getelementptr inbounds %struct.io_state, ptr %iop, i64 0, i32 1
  %3 = load ptr, ptr %iop, align 8, !tbaa !33, !noalias !144
  %call.i.i = tail call i32 (ptr, ptr, ptr, ...) @__sprint(ptr noundef %3, ptr noundef nonnull %uio.i.i, ptr noundef %locale) #14
  %tobool.not.i = icmp eq i32 %call.i.i, 0
  br i1 %tobool.not.i, label %if.end7.i, label %cleanup

if.end7.i:                                        ; preds = %io_print.exit.i, %if.then4.i, %entry
  %p_len.1.i = phi i32 [ %spec.select.i, %io_print.exit.i ], [ 0, %entry ], [ %spec.select.i, %if.then4.i ]
  %sub.i = sub nsw i32 %0, %p_len.1.i
  %uio.i.i.i = getelementptr inbounds %struct.io_state, ptr %iop, i64 0, i32 1
  %cmp8.i.i = icmp sgt i32 %sub.i, 0
  br i1 %cmp8.i.i, label %while.body.lr.ph.i.i, label %if.end

while.body.lr.ph.i.i:                             ; preds = %if.end7.i
  %uio_iovcnt.i.i.i = getelementptr inbounds %struct.io_state, ptr %iop, i64 0, i32 1, i32 1
  %uio_resid.i.i.i = getelementptr inbounds %struct.io_state, ptr %iop, i64 0, i32 1, i32 2
  br label %while.body.i.i

while.body.i.i:                                   ; preds = %if.end.i.i, %while.body.lr.ph.i.i
  %howmany.addr.09.i.i = phi i32 [ %sub.i, %while.body.lr.ph.i.i ], [ %sub.i.i, %if.end.i.i ]
  %cond.i.i = tail call i32 @llvm.smin.i32(i32 %howmany.addr.09.i.i, i32 16)
  %4 = load i32, ptr %uio_iovcnt.i.i.i, align 8, !tbaa !32, !noalias !147
  %idxprom.i.i.i = sext i32 %4 to i64
  %arrayidx.i.i.i = getelementptr inbounds %struct.io_state, ptr %iop, i64 0, i32 2, i64 %idxprom.i.i.i
  store ptr @zeroes, ptr %arrayidx.i.i.i, align 8, !tbaa !41, !noalias !147
  %conv.i.i.i = zext nneg i32 %cond.i.i to i64
  %iov_len.i.i.i = getelementptr inbounds %struct.io_state, ptr %iop, i64 0, i32 2, i64 %idxprom.i.i.i, i32 1
  store i64 %conv.i.i.i, ptr %iov_len.i.i.i, align 8, !tbaa !43, !noalias !147
  %5 = load i32, ptr %uio_resid.i.i.i, align 4, !tbaa !31, !noalias !147
  %add.i.i.i = add nsw i32 %5, %cond.i.i
  store i32 %add.i.i.i, ptr %uio_resid.i.i.i, align 4, !tbaa !31, !noalias !147
  %inc.i.i.i = add nsw i32 %4, 1
  store i32 %inc.i.i.i, ptr %uio_iovcnt.i.i.i, align 8, !tbaa !32, !noalias !147
  %cmp.i.i.i = icmp sgt i32 %4, 6
  br i1 %cmp.i.i.i, label %io_print.exit.i.i, label %if.end.i.i

io_print.exit.i.i:                                ; preds = %while.body.i.i
  %6 = load ptr, ptr %iop, align 8, !tbaa !33, !noalias !147
  %call.i.i.i = tail call i32 (ptr, ptr, ptr, ...) @__sprint(ptr noundef %6, ptr noundef nonnull %uio.i.i.i, ptr noundef %locale) #14
  %tobool.not.i.i = icmp eq i32 %call.i.i.i, 0
  br i1 %tobool.not.i.i, label %if.end.i.i, label %cleanup

if.end.i.i:                                       ; preds = %io_print.exit.i.i, %while.body.i.i
  %sub.i.i = sub nsw i32 %howmany.addr.09.i.i, %cond.i.i
  %cmp.i18.i = icmp sgt i32 %sub.i.i, 0
  br i1 %cmp.i18.i, label %while.body.i.i, label %if.end, !llvm.loop !61

if.end:                                           ; preds = %if.end.i.i, %if.end7.i
  %7 = load i32, ptr %lead, align 8, !tbaa !138
  %idx.ext = sext i32 %7 to i64
  %add.ptr = getelementptr inbounds i32, ptr %cp, i64 %idx.ext
  %nseps = getelementptr inbounds %struct.grouping_state, ptr %gs, i64 0, i32 3
  %nrepeats = getelementptr inbounds %struct.grouping_state, ptr %gs, i64 0, i32 4
  %grouping = getelementptr inbounds %struct.grouping_state, ptr %gs, i64 0, i32 1
  %uio_iovcnt.i = getelementptr inbounds %struct.io_state, ptr %iop, i64 0, i32 1, i32 1
  %uio_resid.i = getelementptr inbounds %struct.io_state, ptr %iop, i64 0, i32 1, i32 2
  br label %while.cond

while.cond:                                       ; preds = %if.end18, %if.end
  %cp.addr.0 = phi ptr [ %add.ptr, %if.end ], [ %add.ptr22, %if.end18 ]
  %8 = load i32, ptr %nseps, align 4, !tbaa !137
  %cmp = icmp sgt i32 %8, 0
  %.pre = load i32, ptr %nrepeats, align 8, !tbaa !136
  %cmp4 = icmp sgt i32 %.pre, 0
  br i1 %cmp, label %while.body, label %lor.rhs

lor.rhs:                                          ; preds = %while.cond
  br i1 %cmp4, label %if.then5, label %while.end

while.body:                                       ; preds = %while.cond
  br i1 %cmp4, label %if.then5, label %if.else

if.then5:                                         ; preds = %lor.rhs, %while.body
  %dec = add nsw i32 %.pre, -1
  store i32 %dec, ptr %nrepeats, align 8, !tbaa !136
  br label %if.end9

if.else:                                          ; preds = %while.body
  %9 = load ptr, ptr %grouping, align 8, !tbaa !44
  %incdec.ptr = getelementptr inbounds i8, ptr %9, i64 -1
  store ptr %incdec.ptr, ptr %grouping, align 8, !tbaa !44
  %dec8 = add nsw i32 %8, -1
  store i32 %dec8, ptr %nseps, align 4, !tbaa !137
  br label %if.end9

if.end9:                                          ; preds = %if.else, %if.then5
  %10 = load <2 x i32>, ptr %uio_iovcnt.i, align 8, !tbaa !5, !noalias !152
  %11 = extractelement <2 x i32> %10, i64 0
  %idxprom.i = sext i32 %11 to i64
  %arrayidx.i = getelementptr inbounds %struct.io_state, ptr %iop, i64 0, i32 2, i64 %idxprom.i
  store ptr %gs, ptr %arrayidx.i, align 8, !tbaa !41, !noalias !152
  %iov_len.i = getelementptr inbounds %struct.io_state, ptr %iop, i64 0, i32 2, i64 %idxprom.i, i32 1
  store i64 1, ptr %iov_len.i, align 8, !tbaa !43, !noalias !152
  %12 = add nsw <2 x i32> %10, <i32 1, i32 1>
  store <2 x i32> %12, ptr %uio_iovcnt.i, align 8, !tbaa !5, !noalias !152
  %cmp.i = icmp sgt i32 %11, 6
  br i1 %cmp.i, label %io_print.exit, label %if.end13

io_print.exit:                                    ; preds = %if.end9
  %13 = load ptr, ptr %iop, align 8, !tbaa !33, !noalias !152
  %call.i = tail call i32 (ptr, ptr, ptr, ...) @__sprint(ptr noundef %13, ptr noundef nonnull %uio.i.i.i, ptr noundef %locale) #14
  %tobool11.not = icmp eq i32 %call.i, 0
  br i1 %tobool11.not, label %if.end13, label %cleanup

if.end13:                                         ; preds = %if.end9, %io_print.exit
  %14 = load ptr, ptr %grouping, align 8, !tbaa !44
  %15 = load i8, ptr %14, align 1, !tbaa !21
  %conv = sext i8 %15 to i32
  %sub.ptr.rhs.cast.i53 = ptrtoint ptr %cp.addr.0 to i64
  %sub.ptr.sub.i54 = sub i64 %sub.ptr.lhs.cast.i, %sub.ptr.rhs.cast.i53
  %sub.ptr.div.i55 = lshr exact i64 %sub.ptr.sub.i54, 2
  %conv.i56 = trunc i64 %sub.ptr.div.i55 to i32
  %spec.select.i57 = tail call i32 @llvm.smin.i32(i32 %conv.i56, i32 %conv)
  %cmp2.i58 = icmp sgt i32 %spec.select.i57, 0
  br i1 %cmp2.i58, label %if.then4.i84, label %if.end7.i59

if.then4.i84:                                     ; preds = %if.end13
  %16 = load i32, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !155
  %idxprom.i.i86 = sext i32 %16 to i64
  %arrayidx.i.i87 = getelementptr inbounds %struct.io_state, ptr %iop, i64 0, i32 2, i64 %idxprom.i.i86
  store ptr %cp.addr.0, ptr %arrayidx.i.i87, align 8, !tbaa !41, !noalias !155
  %conv.i.i88 = zext nneg i32 %spec.select.i57 to i64
  %iov_len.i.i89 = getelementptr inbounds %struct.io_state, ptr %iop, i64 0, i32 2, i64 %idxprom.i.i86, i32 1
  store i64 %conv.i.i88, ptr %iov_len.i.i89, align 8, !tbaa !43, !noalias !155
  %17 = load i32, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !155
  %add.i.i91 = add nsw i32 %17, %spec.select.i57
  store i32 %add.i.i91, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !155
  %inc.i.i92 = add nsw i32 %16, 1
  store i32 %inc.i.i92, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !155
  %cmp.i.i93 = icmp sgt i32 %16, 6
  br i1 %cmp.i.i93, label %io_print.exit.i94, label %if.end7.i59

io_print.exit.i94:                                ; preds = %if.then4.i84
  %18 = load ptr, ptr %iop, align 8, !tbaa !33, !noalias !155
  %call.i.i96 = tail call i32 (ptr, ptr, ptr, ...) @__sprint(ptr noundef %18, ptr noundef nonnull %uio.i.i.i, ptr noundef %locale) #14
  %tobool.not.i97 = icmp eq i32 %call.i.i96, 0
  br i1 %tobool.not.i97, label %if.end7.i59, label %cleanup

if.end7.i59:                                      ; preds = %io_print.exit.i94, %if.then4.i84, %if.end13
  %p_len.1.i60 = phi i32 [ %spec.select.i57, %io_print.exit.i94 ], [ 0, %if.end13 ], [ %spec.select.i57, %if.then4.i84 ]
  %sub.i61 = sub nsw i32 %conv, %p_len.1.i60
  %cmp8.i.i63 = icmp sgt i32 %sub.i61, 0
  br i1 %cmp8.i.i63, label %while.body.i.i68, label %if.end18

while.body.i.i68:                                 ; preds = %if.end7.i59, %if.end.i.i78
  %howmany.addr.09.i.i69 = phi i32 [ %sub.i.i79, %if.end.i.i78 ], [ %sub.i61, %if.end7.i59 ]
  %cond.i.i70 = tail call i32 @llvm.smin.i32(i32 %howmany.addr.09.i.i69, i32 16)
  %19 = load i32, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !158
  %idxprom.i.i.i71 = sext i32 %19 to i64
  %arrayidx.i.i.i72 = getelementptr inbounds %struct.io_state, ptr %iop, i64 0, i32 2, i64 %idxprom.i.i.i71
  store ptr @zeroes, ptr %arrayidx.i.i.i72, align 8, !tbaa !41, !noalias !158
  %conv.i.i.i73 = zext nneg i32 %cond.i.i70 to i64
  %iov_len.i.i.i74 = getelementptr inbounds %struct.io_state, ptr %iop, i64 0, i32 2, i64 %idxprom.i.i.i71, i32 1
  store i64 %conv.i.i.i73, ptr %iov_len.i.i.i74, align 8, !tbaa !43, !noalias !158
  %20 = load i32, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !158
  %add.i.i.i75 = add nsw i32 %20, %cond.i.i70
  store i32 %add.i.i.i75, ptr %uio_resid.i, align 4, !tbaa !31, !noalias !158
  %inc.i.i.i76 = add nsw i32 %19, 1
  store i32 %inc.i.i.i76, ptr %uio_iovcnt.i, align 8, !tbaa !32, !noalias !158
  %cmp.i.i.i77 = icmp sgt i32 %19, 6
  br i1 %cmp.i.i.i77, label %io_print.exit.i.i81, label %if.end.i.i78

io_print.exit.i.i81:                              ; preds = %while.body.i.i68
  %21 = load ptr, ptr %iop, align 8, !tbaa !33, !noalias !158
  %call.i.i.i82 = tail call i32 (ptr, ptr, ptr, ...) @__sprint(ptr noundef %21, ptr noundef nonnull %uio.i.i.i, ptr noundef %locale) #14
  %tobool.not.i.i83 = icmp eq i32 %call.i.i.i82, 0
  br i1 %tobool.not.i.i83, label %if.end.i.i78, label %cleanup

if.end.i.i78:                                     ; preds = %io_print.exit.i.i81, %while.body.i.i68
  %sub.i.i79 = sub nsw i32 %howmany.addr.09.i.i69, %cond.i.i70
  %cmp.i18.i80 = icmp sgt i32 %sub.i.i79, 0
  br i1 %cmp.i18.i80, label %while.body.i.i68, label %if.end18, !llvm.loop !61

if.end18:                                         ; preds = %if.end.i.i78, %if.end7.i59
  %22 = load ptr, ptr %grouping, align 8, !tbaa !44
  %23 = load i8, ptr %22, align 1, !tbaa !21
  %idx.ext21 = sext i8 %23 to i64
  %add.ptr22 = getelementptr inbounds i32, ptr %cp.addr.0, i64 %idx.ext21
  br label %while.cond, !llvm.loop !163

while.end:                                        ; preds = %lor.rhs
  %cmp23 = icmp ugt ptr %cp.addr.0, %ep
  %spec.select = select i1 %cmp23, ptr %ep, ptr %cp.addr.0
  %sub.ptr.lhs.cast = ptrtoint ptr %spec.select to i64
  %sub.ptr.sub = sub i64 %sub.ptr.lhs.cast, %sub.ptr.rhs.cast.i
  %sub.ptr.div = lshr exact i64 %sub.ptr.sub, 2
  %conv27 = trunc i64 %sub.ptr.div to i32
  br label %cleanup

cleanup:                                          ; preds = %io_print.exit.i.i, %io_print.exit.i94, %io_print.exit, %io_print.exit.i.i81, %io_print.exit.i, %while.end
  %retval.0 = phi i32 [ %conv27, %while.end ], [ -1, %io_print.exit.i ], [ -1, %io_print.exit.i.i81 ], [ -1, %io_print.exit ], [ -1, %io_print.exit.i94 ], [ -1, %io_print.exit.i.i ]
  ret i32 %retval.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.va_end(ptr) #4

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #10

declare i32 @__fflush(ptr noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare nonnull ptr @llvm.threadlocal.address.p0(ptr nonnull) #5

declare i64 @mbrtowc(ptr noundef, ptr noundef, i64 noundef, ptr noundef) local_unnamed_addr #2

declare ptr @localeconv_l(ptr noundef) local_unnamed_addr #2

declare i32 @___mb_cur_max() local_unnamed_addr #2

declare i32 @__sprint(...) local_unnamed_addr #2

declare i64 @mbrlen(ptr noundef, i64 noundef, ptr noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nofree nounwind willreturn memory(argmem: read)
declare i64 @strlen(ptr nocapture noundef) local_unnamed_addr #8

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #11

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.fshl.i32(i32, i32, i32) #12

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.umax.i32(i32, i32) #12

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.smax.i32(i32, i32) #12

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.smin.i32(i32, i32) #12

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #13

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.abs.i32(i32, i1 immarg) #12

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.smax.i64(i64, i64) #12

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { noinline nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { mustprogress nocallback nofree nosync nounwind willreturn }
attributes #5 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #6 = { mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #7 = { mustprogress nofree nounwind willreturn memory(read) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #8 = { mustprogress nofree nounwind willreturn memory(argmem: read) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #9 = { noreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #10 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #11 = { mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #12 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #13 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #14 = { nounwind }
attributes #15 = { nounwind willreturn memory(read) }
attributes #16 = { allocsize(0) }
attributes #17 = { noreturn nounwind }

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
!9 = !{!10, !12, i64 16}
!10 = !{!"__sFILE", !11, i64 0, !6, i64 8, !6, i64 12, !12, i64 16, !12, i64 18, !13, i64 24, !6, i64 40, !11, i64 48, !11, i64 56, !11, i64 64, !11, i64 72, !11, i64 80, !13, i64 88, !11, i64 104, !6, i64 112, !7, i64 116, !7, i64 119, !13, i64 120, !6, i64 136, !14, i64 144, !11, i64 152, !11, i64 160, !6, i64 168, !6, i64 172, !7, i64 176, !6, i64 304}
!11 = !{!"any pointer", !7, i64 0}
!12 = !{!"short", !7, i64 0}
!13 = !{!"__sbuf", !11, i64 0, !6, i64 8}
!14 = !{!"long", !7, i64 0}
!15 = !{!10, !12, i64 18}
!16 = !{!10, !11, i64 24}
!17 = !{!10, !11, i64 48}
!18 = !{!10, !11, i64 80}
!19 = !{!10, !6, i64 172}
!20 = !{i64 0, i64 128, !21, i64 0, i64 8, !22}
!21 = !{!7, !7, i64 0}
!22 = !{!14, !14, i64 0}
!23 = !{!10, !11, i64 0}
!24 = !{!10, !6, i64 12}
!25 = !{!10, !6, i64 32}
!26 = !{!10, !6, i64 40}
!27 = !{!11, !11, i64 0}
!28 = !{!29, !11, i64 8}
!29 = !{!"io_state", !11, i64 0, !30, i64 8, !7, i64 24}
!30 = !{!"__suio", !11, i64 0, !6, i64 8, !6, i64 12}
!31 = !{!29, !6, i64 20}
!32 = !{!29, !6, i64 16}
!33 = !{!29, !11, i64 0}
!34 = !{!35, !11, i64 0}
!35 = !{!"lconv", !11, i64 0, !11, i64 8, !11, i64 16, !11, i64 24, !11, i64 32, !11, i64 40, !11, i64 48, !11, i64 56, !11, i64 64, !11, i64 72, !7, i64 80, !7, i64 81, !7, i64 82, !7, i64 83, !7, i64 84, !7, i64 85, !7, i64 86, !7, i64 87, !7, i64 88, !7, i64 89, !7, i64 90, !7, i64 91, !7, i64 92, !7, i64 93}
!36 = distinct !{!36, !37}
!37 = !{!"llvm.loop.mustprogress"}
!38 = !{!39}
!39 = distinct !{!39, !40, !"io_print: %ptr"}
!40 = distinct !{!40, !"io_print"}
!41 = !{!42, !11, i64 0}
!42 = !{!"__siov", !11, i64 0, !14, i64 8}
!43 = !{!42, !14, i64 8}
!44 = !{!45, !11, i64 8}
!45 = !{!"grouping_state", !6, i64 0, !11, i64 8, !6, i64 16, !6, i64 20, !6, i64 24}
!46 = distinct !{!46, !37}
!47 = distinct !{!47, !37}
!48 = distinct !{!48, !37}
!49 = distinct !{!49, !37}
!50 = distinct !{!50, !37}
!51 = distinct !{!51, !37}
!52 = !{!53, !53, i64 0}
!53 = !{!"long long", !7, i64 0}
!54 = !{!12, !12, i64 0}
!55 = distinct !{!55, !37}
!56 = !{!57, !59}
!57 = distinct !{!57, !58, !"io_print: %ptr"}
!58 = distinct !{!58, !"io_print"}
!59 = distinct !{!59, !60, !"io_pad: %with"}
!60 = distinct !{!60, !"io_pad"}
!61 = distinct !{!61, !37}
!62 = !{!63}
!63 = distinct !{!63, !64, !"io_print: %ptr"}
!64 = distinct !{!64, !"io_print"}
!65 = !{!66}
!66 = distinct !{!66, !67, !"io_print: %ptr"}
!67 = distinct !{!67, !"io_print"}
!68 = !{!69, !71}
!69 = distinct !{!69, !70, !"io_print: %ptr"}
!70 = distinct !{!70, !"io_print"}
!71 = distinct !{!71, !72, !"io_pad: %with"}
!72 = distinct !{!72, !"io_pad"}
!73 = !{!74, !76}
!74 = distinct !{!74, !75, !"io_print: %ptr"}
!75 = distinct !{!75, !"io_print"}
!76 = distinct !{!76, !77, !"io_pad: %with"}
!77 = distinct !{!77, !"io_pad"}
!78 = !{!79}
!79 = distinct !{!79, !80, !"io_print: %ptr"}
!80 = distinct !{!80, !"io_print"}
!81 = !{!82}
!82 = distinct !{!82, !83, !"io_print: %ptr"}
!83 = distinct !{!83, !"io_print"}
!84 = !{!85}
!85 = distinct !{!85, !86, !"io_print: %ptr"}
!86 = distinct !{!86, !"io_print"}
!87 = !{!88, !90}
!88 = distinct !{!88, !89, !"io_print: %ptr"}
!89 = distinct !{!89, !"io_print"}
!90 = distinct !{!90, !91, !"io_pad: %with"}
!91 = distinct !{!91, !"io_pad"}
!92 = !{!93}
!93 = distinct !{!93, !94, !"io_print: %ptr"}
!94 = distinct !{!94, !"io_print"}
!95 = !{!96, !98}
!96 = distinct !{!96, !97, !"io_print: %ptr"}
!97 = distinct !{!97, !"io_print"}
!98 = distinct !{!98, !99, !"io_pad: %with"}
!99 = distinct !{!99, !"io_pad"}
!100 = !{!101}
!101 = distinct !{!101, !102, !"io_print: %ptr"}
!102 = distinct !{!102, !"io_print"}
!103 = !{!104}
!104 = distinct !{!104, !105, !"io_print: %ptr"}
!105 = distinct !{!105, !"io_print"}
!106 = !{!107, !109}
!107 = distinct !{!107, !108, !"io_print: %ptr"}
!108 = distinct !{!108, !"io_print"}
!109 = distinct !{!109, !110, !"io_pad: %with"}
!110 = distinct !{!110, !"io_pad"}
!111 = !{!112}
!112 = distinct !{!112, !113, !"io_print: %ptr"}
!113 = distinct !{!113, !"io_print"}
!114 = !{!115}
!115 = distinct !{!115, !116, !"io_print: %ptr"}
!116 = distinct !{!116, !"io_print"}
!117 = !{!118, !120}
!118 = distinct !{!118, !119, !"io_print: %ptr"}
!119 = distinct !{!119, !"io_print"}
!120 = distinct !{!120, !121, !"io_pad: %with"}
!121 = distinct !{!121, !"io_pad"}
!122 = !{!123}
!123 = distinct !{!123, !124, !"io_print: %ptr"}
!124 = distinct !{!124, !"io_print"}
!125 = !{!126}
!126 = distinct !{!126, !127, !"io_print: %ptr"}
!127 = distinct !{!127, !"io_print"}
!128 = !{!129, !131}
!129 = distinct !{!129, !130, !"io_print: %ptr"}
!130 = distinct !{!130, !"io_print"}
!131 = distinct !{!131, !132, !"io_pad: %with"}
!132 = distinct !{!132, !"io_pad"}
!133 = !{!35, !11, i64 16}
!134 = !{!35, !11, i64 8}
!135 = !{!45, !6, i64 0}
!136 = !{!45, !6, i64 24}
!137 = !{!45, !6, i64 20}
!138 = !{!45, !6, i64 16}
!139 = distinct !{!139, !37}
!140 = distinct !{!140, !37}
!141 = distinct !{!141, !37}
!142 = distinct !{!142, !37}
!143 = distinct !{!143, !37}
!144 = !{!145}
!145 = distinct !{!145, !146, !"io_print: %ptr"}
!146 = distinct !{!146, !"io_print"}
!147 = !{!148, !150}
!148 = distinct !{!148, !149, !"io_print: %ptr"}
!149 = distinct !{!149, !"io_print"}
!150 = distinct !{!150, !151, !"io_pad: %with"}
!151 = distinct !{!151, !"io_pad"}
!152 = !{!153}
!153 = distinct !{!153, !154, !"io_print: %ptr"}
!154 = distinct !{!154, !"io_print"}
!155 = !{!156}
!156 = distinct !{!156, !157, !"io_print: %ptr"}
!157 = distinct !{!157, !"io_print"}
!158 = !{!159, !161}
!159 = distinct !{!159, !160, !"io_print: %ptr"}
!160 = distinct !{!160, !"io_print"}
!161 = distinct !{!161, !162, !"io_pad: %with"}
!162 = distinct !{!162, !"io_pad"}
!163 = distinct !{!163, !37}
