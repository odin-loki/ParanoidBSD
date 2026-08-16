; ModuleID = '/home/odin/pbsd/pbsd/lib/libc/stdio/xprintf_float_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/libc/stdio/xprintf_float_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.printf_info = type { i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, ptr, ptr, [2 x ptr] }
%struct.lconv = type { ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8 }

@__func__.ref___printf_arginfo_float = private unnamed_addr constant [27 x i8] c"ref___printf_arginfo_float\00", align 1
@.str = private unnamed_addr constant [61 x i8] c"/home/odin/pbsd/pbsd/lib/libc/stdio/xprintf_float_m/oracle.c\00", align 1
@.str.1 = private unnamed_addr constant [6 x i8] c"n > 0\00", align 1
@__lowercase_hex = external constant [17 x i8], align 16
@__uppercase_hex = external constant [17 x i8], align 16
@__func__.ref___printf_render_float = private unnamed_addr constant [26 x i8] c"ref___printf_render_float\00", align 1
@.str.2 = private unnamed_addr constant [16 x i8] c"pi->spec == 'f'\00", align 1
@.str.3 = private unnamed_addr constant [4 x i8] c"nan\00", align 1
@.str.4 = private unnamed_addr constant [4 x i8] c"NAN\00", align 1
@.str.5 = private unnamed_addr constant [4 x i8] c"inf\00", align 1
@.str.6 = private unnamed_addr constant [4 x i8] c"INF\00", align 1
@.str.7 = private unnamed_addr constant [2 x i8] c"0\00", align 1

; Function Attrs: nounwind uwtable
define dso_local noundef i32 @ref___printf_arginfo_float(ptr nocapture noundef readonly %pi, i64 noundef %n, ptr nocapture noundef writeonly %argt) local_unnamed_addr #0 {
entry:
  %cmp.not = icmp eq i64 %n, 0
  br i1 %cmp.not, label %cond.false, label %cond.end

cond.false:                                       ; preds = %entry
  tail call void @__assert(ptr noundef nonnull @__func__.ref___printf_arginfo_float, ptr noundef nonnull @.str, i32 noundef 125, ptr noundef nonnull @.str.1) #6
  unreachable

cond.end:                                         ; preds = %entry
  store i32 128, ptr %argt, align 4, !tbaa !5
  %is_long_double = getelementptr inbounds %struct.printf_info, ptr %pi, i64 0, i32 3
  %0 = load i32, ptr %is_long_double, align 4, !tbaa !9
  %tobool.not = icmp eq i32 %0, 0
  %spec.store.select = select i1 %tobool.not, i32 128, i32 65664
  store i32 %spec.store.select, ptr %argt, align 4
  ret i32 1
}

; Function Attrs: noreturn
declare void @__assert(ptr noundef, ptr noundef, i32 noundef, ptr noundef) local_unnamed_addr #1

; Function Attrs: nounwind uwtable
define dso_local i32 @ref___printf_render_float(ptr noundef %io, ptr nocapture noundef readonly %pi, ptr nocapture noundef readonly %arg) local_unnamed_addr #0 {
entry:
  %expbuf.i = alloca [6 x i8], align 1
  %expt = alloca i32, align 4
  %signflag = alloca i32, align 4
  %dtoaend = alloca ptr, align 8
  %sign = alloca i8, align 1
  %expstr = alloca [8 x i8], align 1
  %ld = alloca x86_fp80, align 16
  %ox = alloca [2 x i8], align 1
  %thousands_sep = alloca i8, align 1
  %buf = alloca [100 x i8], align 16
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %expt) #7
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %signflag) #7
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %dtoaend) #7
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %sign) #7
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %expstr) #7
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %ld) #7
  call void @llvm.lifetime.start.p0(i64 2, ptr nonnull %ox) #7
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %thousands_sep) #7
  call void @llvm.lifetime.start.p0(i64 100, ptr nonnull %buf) #7
  %0 = load i32, ptr %pi, align 8, !tbaa !12
  %arrayidx = getelementptr inbounds [2 x i8], ptr %ox, i64 0, i64 1
  store i8 0, ptr %arrayidx, align 1, !tbaa !13
  %showsign = getelementptr inbounds %struct.printf_info, ptr %pi, i64 0, i32 10
  %1 = load i32, ptr %showsign, align 8, !tbaa !14
  %conv = trunc i32 %1 to i8
  store i8 %conv, ptr %sign, align 1, !tbaa !13
  %call = tail call ptr @localeconv() #7
  %thousands_sep2 = getelementptr inbounds %struct.lconv, ptr %call, i64 0, i32 1
  %2 = load ptr, ptr %thousands_sep2, align 8, !tbaa !15
  %3 = load i8, ptr %2, align 1, !tbaa !13
  store i8 %3, ptr %thousands_sep, align 1, !tbaa !13
  %alt = getelementptr inbounds %struct.printf_info, ptr %pi, i64 0, i32 7
  %4 = load i32, ptr %alt, align 4, !tbaa !17
  %tobool.not = icmp eq i32 %4, 0
  br i1 %tobool.not, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %call3 = tail call ptr @localeconv() #7
  %grouping4 = getelementptr inbounds %struct.lconv, ptr %call3, i64 0, i32 2
  %5 = load ptr, ptr %grouping4, align 8, !tbaa !18
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %grouping.0 = phi ptr [ %5, %if.then ], [ null, %entry ]
  %call5 = tail call ptr @localeconv() #7
  %6 = load ptr, ptr %call5, align 8, !tbaa !19
  %spec = getelementptr inbounds %struct.printf_info, ptr %pi, i64 0, i32 2
  %7 = load i32, ptr %spec, align 8, !tbaa !20
  switch i32 %7, label %cond.false [
    i32 97, label %sw.bb
    i32 65, label %sw.bb
    i32 101, label %sw.bb35
    i32 69, label %sw.bb35
    i32 102, label %sw.epilog
    i32 70, label %sw.epilog
    i32 103, label %sw.bb45
    i32 71, label %sw.bb45
  ]

sw.bb:                                            ; preds = %if.end, %if.end
  %cmp = icmp eq i32 %7, 97
  %. = select i1 %cmp, i8 120, i8 88
  %.604 = select i1 %cmp, i8 112, i8 80
  %__lowercase_hex.__uppercase_hex = select i1 %cmp, ptr @__lowercase_hex, ptr @__uppercase_hex
  store i8 %., ptr %arrayidx, align 1, !tbaa !13
  %cmp13617 = icmp sgt i32 %0, -1
  %inc = zext i1 %cmp13617 to i32
  %prec.0 = add nuw nsw i32 %0, %inc
  %is_long_double = getelementptr inbounds %struct.printf_info, ptr %pi, i64 0, i32 3
  %8 = load i32, ptr %is_long_double, align 4, !tbaa !9
  %tobool17.not = icmp eq i32 %8, 0
  %9 = load ptr, ptr %arg, align 8, !tbaa !21
  br i1 %tobool17.not, label %if.else21, label %if.then18

if.then18:                                        ; preds = %sw.bb
  %10 = load x86_fp80, ptr %9, align 16, !tbaa !22
  store x86_fp80 %10, ptr %ld, align 16, !tbaa !22
  %call20 = call ptr @__hldtoa(x86_fp80 noundef %10, ptr noundef nonnull %__lowercase_hex.__uppercase_hex, i32 noundef %prec.0, ptr noundef nonnull %expt, ptr noundef nonnull %signflag, ptr noundef nonnull %dtoaend) #7
  br label %if.end24

if.else21:                                        ; preds = %sw.bb
  %11 = load double, ptr %9, align 8, !tbaa !24
  %call23 = call ptr @__hdtoa(double noundef %11, ptr noundef nonnull %__lowercase_hex.__uppercase_hex, i32 noundef %prec.0, ptr noundef nonnull %expt, ptr noundef nonnull %signflag, ptr noundef nonnull %dtoaend) #7
  br label %if.end24

if.end24:                                         ; preds = %if.else21, %if.then18
  %dtoaresult.0 = phi ptr [ %call20, %if.then18 ], [ %call23, %if.else21 ]
  %cmp25 = icmp slt i32 %prec.0, 0
  %12 = load ptr, ptr %dtoaend, align 8
  %sub.ptr.lhs.cast = ptrtoint ptr %12 to i64
  %sub.ptr.rhs.cast = ptrtoint ptr %dtoaresult.0 to i64
  %sub.ptr.sub = sub i64 %sub.ptr.lhs.cast, %sub.ptr.rhs.cast
  %conv28 = trunc i64 %sub.ptr.sub to i32
  %prec.1 = select i1 %cmp25, i32 %conv28, i32 %prec.0
  %13 = load i32, ptr %expt, align 4, !tbaa !5
  %cmp30 = icmp eq i32 %13, 2147483647
  br i1 %cmp30, label %if.then32, label %fp_common

if.then32:                                        ; preds = %if.end24
  store i8 0, ptr %arrayidx, align 1, !tbaa !13
  br label %fp_common

sw.bb35:                                          ; preds = %if.end, %if.end
  %conv37 = trunc i32 %7 to i8
  %cmp38 = icmp slt i32 %0, 0
  %inc42 = add nuw nsw i32 %0, 1
  %spec.select612 = select i1 %cmp38, i32 7, i32 %inc42
  br label %sw.epilog

sw.bb45:                                          ; preds = %if.end, %if.end
  %14 = trunc i32 %7 to i8
  %conv47 = add nsw i8 %14, -2
  %spec.store.select = tail call i32 @llvm.umax.i32(i32 %0, i32 1)
  br label %sw.epilog

cond.false:                                       ; preds = %if.end
  tail call void @__assert(ptr noundef nonnull @__func__.ref___printf_render_float, ptr noundef nonnull @.str, i32 noundef 241, ptr noundef nonnull @.str.2) #6
  unreachable

sw.epilog:                                        ; preds = %sw.bb35, %if.end, %if.end, %sw.bb45
  %expchar.1 = phi i8 [ %conv47, %sw.bb45 ], [ 0, %if.end ], [ 0, %if.end ], [ %conv37, %sw.bb35 ]
  %prec.2 = phi i32 [ %spec.store.select, %sw.bb45 ], [ %0, %if.end ], [ %0, %if.end ], [ %spec.select612, %sw.bb35 ]
  %cmp55 = icmp slt i32 %prec.2, 0
  %spec.store.select418 = select i1 %cmp55, i32 6, i32 %prec.2
  %is_long_double59 = getelementptr inbounds %struct.printf_info, ptr %pi, i64 0, i32 3
  %15 = load i32, ptr %is_long_double59, align 4, !tbaa !9
  %tobool60.not = icmp eq i32 %15, 0
  %16 = load ptr, ptr %arg, align 8, !tbaa !21
  br i1 %tobool60.not, label %if.else66, label %if.then61

if.then61:                                        ; preds = %sw.epilog
  %17 = load x86_fp80, ptr %16, align 16, !tbaa !22
  store x86_fp80 %17, ptr %ld, align 16, !tbaa !22
  %tobool64.not = icmp eq i8 %expchar.1, 0
  %cond = select i1 %tobool64.not, i32 3, i32 2
  %call65 = call ptr @__ldtoa(ptr noundef nonnull %ld, i32 noundef %cond, i32 noundef %spec.store.select418, ptr noundef nonnull %expt, ptr noundef nonnull %signflag, ptr noundef nonnull %dtoaend) #7
  br label %fp_common

if.else66:                                        ; preds = %sw.epilog
  %18 = load double, ptr %16, align 8, !tbaa !24
  %tobool69.not = icmp eq i8 %expchar.1, 0
  %cond70 = select i1 %tobool69.not, i32 3, i32 2
  %call71 = call ptr @__dtoa(double noundef %18, i32 noundef %cond70, i32 noundef %spec.store.select418, ptr noundef nonnull %expt, ptr noundef nonnull %signflag, ptr noundef nonnull %dtoaend) #7
  %19 = load i32, ptr %expt, align 4, !tbaa !5
  %cmp72 = icmp eq i32 %19, 9999
  br i1 %cmp72, label %if.then74, label %fp_common

if.then74:                                        ; preds = %if.else66
  store i32 2147483647, ptr %expt, align 4, !tbaa !5
  br label %fp_common

fp_common:                                        ; preds = %if.then61, %if.then74, %if.else66, %if.end24, %if.then32
  %expchar.2 = phi i8 [ %expchar.1, %if.then61 ], [ %expchar.1, %if.then74 ], [ %expchar.1, %if.else66 ], [ %.604, %if.then32 ], [ %.604, %if.end24 ]
  %dtoaresult.1 = phi ptr [ %call65, %if.then61 ], [ %call71, %if.then74 ], [ %call71, %if.else66 ], [ %dtoaresult.0, %if.then32 ], [ %dtoaresult.0, %if.end24 ]
  %prec.3 = phi i32 [ %spec.store.select418, %if.then61 ], [ %spec.store.select418, %if.then74 ], [ %spec.store.select418, %if.else66 ], [ %prec.1, %if.then32 ], [ %prec.1, %if.end24 ]
  %20 = load i32, ptr %signflag, align 4, !tbaa !5
  %tobool77.not = icmp eq i32 %20, 0
  br i1 %tobool77.not, label %if.end79, label %if.then78

if.then78:                                        ; preds = %fp_common
  store i8 45, ptr %sign, align 1, !tbaa !13
  br label %if.end79

if.end79:                                         ; preds = %if.then78, %fp_common
  %21 = load i32, ptr %expt, align 4
  %cmp80.not = icmp eq i32 %21, 2147483647
  br i1 %cmp80.not, label %if.then82, label %if.end97

if.then82:                                        ; preds = %if.end79
  %22 = load i8, ptr %dtoaresult.1, align 1, !tbaa !13
  %cmp84 = icmp eq i8 %22, 78
  %23 = load i32, ptr %spec, align 8, !tbaa !20
  %cmp88 = icmp sgt i32 %23, 96
  br i1 %cmp84, label %if.then86, label %if.else91

if.then86:                                        ; preds = %if.then82
  %cond90 = select i1 %cmp88, ptr @.str.3, ptr @.str.4
  store i8 0, ptr %sign, align 1, !tbaa !13
  br label %here

if.else91:                                        ; preds = %if.then82
  %cond95 = select i1 %cmp88, ptr @.str.5, ptr @.str.6
  br label %here

if.end97:                                         ; preds = %if.end79
  %24 = load ptr, ptr %dtoaend, align 8, !tbaa !21
  %sub.ptr.lhs.cast98 = ptrtoint ptr %24 to i64
  %sub.ptr.rhs.cast99 = ptrtoint ptr %dtoaresult.1 to i64
  %sub.ptr.sub100 = sub i64 %sub.ptr.lhs.cast98, %sub.ptr.rhs.cast99
  %conv101 = trunc i64 %sub.ptr.sub100 to i32
  %25 = load i32, ptr %spec, align 8, !tbaa !20
  switch i32 %25, label %if.end131 [
    i32 103, label %if.then108
    i32 71, label %if.then108
  ]

if.then108:                                       ; preds = %if.end97, %if.end97
  %cmp109 = icmp slt i32 %21, -3
  %cmp111.not = icmp sgt i32 %21, %prec.3
  %or.cond605 = select i1 %cmp109, i1 true, i1 %cmp111.not
  %26 = load i32, ptr %alt, align 4, !tbaa !17
  %tobool127.not = icmp eq i32 %26, 0
  %spec.select = select i1 %tobool127.not, i32 %conv101, i32 %prec.3
  br i1 %or.cond605, label %if.end131, label %if.end131.thread

if.end131.thread:                                 ; preds = %if.then108
  %prec.4 = sub nsw i32 %spec.select, %21
  %spec.store.select419 = call i32 @llvm.smax.i32(i32 %prec.4, i32 0)
  br label %if.else145

if.end131:                                        ; preds = %if.then108, %if.end97
  %prec.5 = phi i32 [ %prec.3, %if.end97 ], [ %spec.select, %if.then108 ]
  %tobool132.not = icmp eq i8 %expchar.2, 0
  br i1 %tobool132.not, label %if.else145, label %if.then133

if.then133:                                       ; preds = %if.end131
  %sub134 = add nsw i32 %21, -1
  call void @llvm.lifetime.start.p0(i64 6, ptr nonnull %expbuf.i) #7
  %incdec.ptr.i = getelementptr inbounds i8, ptr %expstr, i64 1
  store i8 %expchar.2, ptr %expstr, align 1, !tbaa !13
  %cmp.i = icmp slt i32 %21, 1
  %storemerge.i = select i1 %cmp.i, i8 45, i8 43
  %expo.addr.0.i = call i32 @llvm.abs.i32(i32 %sub134, i1 true)
  %p.0.i = getelementptr inbounds i8, ptr %expstr, i64 2
  store i8 %storemerge.i, ptr %incdec.ptr.i, align 1, !tbaa !13
  %cmp4.i = icmp ugt i32 %expo.addr.0.i, 9
  br i1 %cmp4.i, label %do.body.preheader.i, label %if.else20.i

do.body.preheader.i:                              ; preds = %if.then133
  %scevgep55.i = getelementptr i8, ptr %expstr, i64 -1
  br label %do.body.i

do.body.i:                                        ; preds = %do.body.i, %do.body.preheader.i
  %indvars.iv57.i = phi i64 [ 4, %do.body.preheader.i ], [ %indvars.iv.next.i, %do.body.i ]
  %indvars.iv.i = phi ptr [ %scevgep55.i, %do.body.preheader.i ], [ %scevgep56.i, %do.body.i ]
  %indvar.i = phi i64 [ 0, %do.body.preheader.i ], [ %indvar.next.i, %do.body.i ]
  %expo.addr.1.i = phi i32 [ %expo.addr.0.i, %do.body.preheader.i ], [ %div.i, %do.body.i ]
  %t.0.idx.i = phi i64 [ 6, %do.body.preheader.i ], [ %t.0.add51.i, %do.body.i ]
  %rem.i = urem i32 %expo.addr.1.i, 10
  %27 = trunc i32 %rem.i to i8
  %conv7.i = or disjoint i8 %27, 48
  %t.0.add51.i = add nsw i64 %t.0.idx.i, -1
  %incdec.ptr8.ptr.i = getelementptr inbounds i8, ptr %expbuf.i, i64 %t.0.add51.i
  store i8 %conv7.i, ptr %incdec.ptr8.ptr.i, align 1, !tbaa !13
  %div.i = udiv i32 %expo.addr.1.i, 10
  %cmp9.i = icmp ugt i32 %expo.addr.1.i, 99
  %indvar.next.i = add i64 %indvar.i, 1
  %scevgep56.i = getelementptr i8, ptr %indvars.iv.i, i64 1
  %indvars.iv.next.i = add i64 %indvars.iv57.i, -1
  br i1 %cmp9.i, label %do.body.i, label %do.end.i, !llvm.loop !26

do.end.i:                                         ; preds = %do.body.i
  %28 = trunc i32 %div.i to i8
  %conv12.i = or disjoint i8 %28, 48
  %t.0.add.i = add nsw i64 %t.0.idx.i, -2
  %incdec.ptr13.ptr.i = getelementptr i8, ptr %expbuf.i, i64 %t.0.add.i
  store i8 %conv12.i, ptr %incdec.ptr13.ptr.i, align 1, !tbaa !13
  %cmp1652.i = icmp slt i64 %t.0.idx.i, 8
  br i1 %cmp1652.i, label %for.inc.preheader.i, label %ref_exponent.exit

for.inc.preheader.i:                              ; preds = %do.end.i
  %smax.i = call i64 @llvm.smax.i64(i64 %t.0.add.i, i64 5)
  %29 = add i64 %indvar.i, -3
  %30 = add i64 %29, %smax.i
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 1 %p.0.i, ptr nonnull align 1 %incdec.ptr13.ptr.i, i64 %30, i1 false), !tbaa !13
  %smax58.i = call i64 @llvm.smax.i64(i64 %indvars.iv57.i, i64 5)
  %scevgep59.i = getelementptr i8, ptr %indvars.iv.i, i64 %smax58.i
  br label %ref_exponent.exit

if.else20.i:                                      ; preds = %if.then133
  %31 = and i8 %expchar.2, 95
  %or.cond.i = icmp eq i8 %31, 69
  br i1 %or.cond.i, label %if.then25.i, label %if.end27.i

if.then25.i:                                      ; preds = %if.else20.i
  %incdec.ptr26.i = getelementptr inbounds i8, ptr %expstr, i64 3
  store i8 48, ptr %p.0.i, align 1, !tbaa !13
  br label %if.end27.i

if.end27.i:                                       ; preds = %if.then25.i, %if.else20.i
  %p.2.i = phi ptr [ %incdec.ptr26.i, %if.then25.i ], [ %p.0.i, %if.else20.i ]
  %32 = trunc i32 %expo.addr.0.i to i8
  %conv29.i = or disjoint i8 %32, 48
  %incdec.ptr30.i = getelementptr inbounds i8, ptr %p.2.i, i64 1
  store i8 %conv29.i, ptr %p.2.i, align 1, !tbaa !13
  br label %ref_exponent.exit

ref_exponent.exit:                                ; preds = %do.end.i, %for.inc.preheader.i, %if.end27.i
  %p.3.i = phi ptr [ %incdec.ptr30.i, %if.end27.i ], [ %p.0.i, %do.end.i ], [ %scevgep59.i, %for.inc.preheader.i ]
  %sub.ptr.lhs.cast.i = ptrtoint ptr %p.3.i to i64
  %sub.ptr.rhs.cast.i = ptrtoint ptr %expstr to i64
  %sub.ptr.sub.i = sub i64 %sub.ptr.lhs.cast.i, %sub.ptr.rhs.cast.i
  %conv32.i = trunc i64 %sub.ptr.sub.i to i32
  call void @llvm.lifetime.end.p0(i64 6, ptr nonnull %expbuf.i) #7
  %add = add nsw i32 %prec.5, %conv32.i
  %cmp137 = icmp sgt i32 %prec.5, 1
  br i1 %cmp137, label %if.then142, label %lor.lhs.false139

lor.lhs.false139:                                 ; preds = %ref_exponent.exit
  %33 = load i32, ptr %alt, align 4, !tbaa !17
  %tobool141.not = icmp eq i32 %33, 0
  br i1 %tobool141.not, label %here, label %if.then142

if.then142:                                       ; preds = %lor.lhs.false139, %ref_exponent.exit
  %inc143 = add nsw i32 %add, 1
  br label %here

if.else145:                                       ; preds = %if.end131.thread, %if.end131
  %prec.5616 = phi i32 [ %spec.store.select419, %if.end131.thread ], [ %prec.5, %if.end131 ]
  %cmp146 = icmp sgt i32 %21, 0
  %.606 = select i1 %cmp146, i32 %21, i32 1
  %tobool151.not = icmp eq i32 %prec.5616, 0
  br i1 %tobool151.not, label %lor.lhs.false152, label %if.then155

lor.lhs.false152:                                 ; preds = %if.else145
  %34 = load i32, ptr %alt, align 4, !tbaa !17
  %tobool154.not = icmp eq i32 %34, 0
  br i1 %tobool154.not, label %if.end158, label %if.then155

if.then155:                                       ; preds = %lor.lhs.false152, %if.else145
  %add156 = add nsw i32 %.606, 1
  %add157 = add i32 %add156, %prec.5616
  br label %if.end158

if.end158:                                        ; preds = %if.then155, %lor.lhs.false152
  %size.1 = phi i32 [ %add157, %if.then155 ], [ %.606, %lor.lhs.false152 ]
  %tobool159 = icmp ne ptr %grouping.0, null
  %or.cond = and i1 %tobool159, %cmp146
  br i1 %or.cond, label %while.cond.preheader, label %here

while.cond.preheader:                             ; preds = %if.end158
  %35 = load i8, ptr %grouping.0, align 1, !tbaa !13
  %conv164619 = sext i8 %35 to i32
  %cmp165.not620 = icmp ne i8 %35, 127
  %cmp168.not621 = icmp sgt i32 %21, %conv164619
  %or.cond607622 = and i1 %cmp165.not620, %cmp168.not621
  br i1 %or.cond607622, label %if.end171, label %while.end

if.end171:                                        ; preds = %while.cond.preheader, %if.end171
  %conv164627 = phi i32 [ %conv164, %if.end171 ], [ %conv164619, %while.cond.preheader ]
  %lead.0626 = phi i32 [ %sub173, %if.end171 ], [ %21, %while.cond.preheader ]
  %grouping.1625 = phi ptr [ %grouping.2, %if.end171 ], [ %grouping.0, %while.cond.preheader ]
  %nrepeats.0624 = phi i32 [ %nrepeats.1, %if.end171 ], [ 0, %while.cond.preheader ]
  %nseps.0623 = phi i32 [ %nseps.1, %if.end171 ], [ 0, %while.cond.preheader ]
  %sub173 = sub nsw i32 %lead.0626, %conv164627
  %add.ptr = getelementptr inbounds i8, ptr %grouping.1625, i64 1
  %36 = load i8, ptr %add.ptr, align 1, !tbaa !13
  %tobool174.not = icmp ne i8 %36, 0
  %inc176 = zext i1 %tobool174.not to i32
  %nseps.1 = add nuw nsw i32 %nseps.0623, %inc176
  %not.tobool174.not = xor i1 %tobool174.not, true
  %inc178 = zext i1 %not.tobool174.not to i32
  %nrepeats.1 = add nuw nsw i32 %nrepeats.0624, %inc178
  %grouping.2 = select i1 %tobool174.not, ptr %add.ptr, ptr %grouping.1625
  %37 = load i8, ptr %grouping.2, align 1, !tbaa !13
  %conv164 = sext i8 %37 to i32
  %cmp165.not = icmp ne i8 %37, 127
  %cmp168.not = icmp sgt i32 %sub173, %conv164
  %or.cond607 = select i1 %cmp165.not, i1 %cmp168.not, i1 false
  br i1 %or.cond607, label %if.end171, label %while.end, !llvm.loop !28

while.end:                                        ; preds = %if.end171, %while.cond.preheader
  %nseps.0.lcssa = phi i32 [ 0, %while.cond.preheader ], [ %nseps.1, %if.end171 ]
  %nrepeats.0.lcssa = phi i32 [ 0, %while.cond.preheader ], [ %nrepeats.1, %if.end171 ]
  %grouping.1.lcssa = phi ptr [ %grouping.0, %while.cond.preheader ], [ %grouping.2, %if.end171 ]
  %lead.0.lcssa = phi i32 [ %21, %while.cond.preheader ], [ %sub173, %if.end171 ]
  %add180 = add i32 %nseps.0.lcssa, %size.1
  %add181 = add i32 %add180, %nrepeats.0.lcssa
  br label %here

here:                                             ; preds = %if.end158, %if.then86, %if.else91, %if.then142, %lor.lhs.false139, %while.end
  %expchar.4 = phi i8 [ 1, %if.then142 ], [ 1, %lor.lhs.false139 ], [ 0, %while.end ], [ %expchar.2, %if.else91 ], [ %expchar.2, %if.then86 ], [ 0, %if.end158 ]
  %cp.3 = phi ptr [ %dtoaresult.1, %if.then142 ], [ %dtoaresult.1, %lor.lhs.false139 ], [ %dtoaresult.1, %while.end ], [ %cond95, %if.else91 ], [ %cond90, %if.then86 ], [ %dtoaresult.1, %if.end158 ]
  %size.2 = phi i32 [ %inc143, %if.then142 ], [ %add, %lor.lhs.false139 ], [ %add181, %while.end ], [ 3, %if.else91 ], [ 3, %if.then86 ], [ %size.1, %if.end158 ]
  %ndig.0 = phi i32 [ %conv101, %if.then142 ], [ %conv101, %lor.lhs.false139 ], [ %conv101, %while.end ], [ undef, %if.else91 ], [ undef, %if.then86 ], [ %conv101, %if.end158 ]
  %expsize.0 = phi i32 [ %conv32.i, %if.then142 ], [ %conv32.i, %lor.lhs.false139 ], [ undef, %while.end ], [ undef, %if.else91 ], [ undef, %if.then86 ], [ undef, %if.end158 ]
  %nseps.2 = phi i32 [ undef, %if.then142 ], [ undef, %lor.lhs.false139 ], [ %nseps.0.lcssa, %while.end ], [ undef, %if.else91 ], [ undef, %if.then86 ], [ undef, %if.end158 ]
  %nrepeats.2 = phi i32 [ undef, %if.then142 ], [ undef, %lor.lhs.false139 ], [ %nrepeats.0.lcssa, %while.end ], [ undef, %if.else91 ], [ undef, %if.then86 ], [ undef, %if.end158 ]
  %grouping.3 = phi ptr [ %grouping.0, %if.then142 ], [ %grouping.0, %lor.lhs.false139 ], [ %grouping.1.lcssa, %while.end ], [ %grouping.0, %if.else91 ], [ %grouping.0, %if.then86 ], [ %grouping.0, %if.end158 ]
  %lead.1 = phi i32 [ undef, %if.then142 ], [ undef, %lor.lhs.false139 ], [ %lead.0.lcssa, %while.end ], [ undef, %if.else91 ], [ undef, %if.then86 ], [ %21, %if.end158 ]
  %prec.6 = phi i32 [ %prec.5, %if.then142 ], [ %prec.5, %lor.lhs.false139 ], [ %prec.5616, %while.end ], [ %prec.3, %if.else91 ], [ %prec.3, %if.then86 ], [ %prec.5616, %if.end158 ]
  %cond190 = call i32 @llvm.smax.i32(i32 %size.2, i32 -1)
  %38 = load i8, ptr %sign, align 1, !tbaa !13
  %tobool191.not = icmp ne i8 %38, 0
  %inc193 = zext i1 %tobool191.not to i32
  %spec.select608 = add nsw i32 %cond190, %inc193
  %39 = load i8, ptr %arrayidx, align 1, !tbaa !13
  %tobool196.not = icmp eq i8 %39, 0
  %add198 = add nsw i32 %spec.select608, 2
  %realsz.1 = select i1 %tobool196.not, i32 %spec.select608, i32 %add198
  %pad = getelementptr inbounds %struct.printf_info, ptr %pi, i64 0, i32 14
  %40 = load i32, ptr %pad, align 8, !tbaa !29
  %cmp200.not = icmp eq i32 %40, 48
  br i1 %cmp200.not, label %if.end209, label %land.lhs.true202

land.lhs.true202:                                 ; preds = %here
  %left = getelementptr inbounds %struct.printf_info, ptr %pi, i64 0, i32 9
  %41 = load i32, ptr %left, align 4, !tbaa !30
  %cmp203 = icmp eq i32 %41, 0
  br i1 %cmp203, label %if.then205, label %if.end209

if.then205:                                       ; preds = %land.lhs.true202
  %width = getelementptr inbounds %struct.printf_info, ptr %pi, i64 0, i32 1
  %42 = load i32, ptr %width, align 4, !tbaa !31
  %sub206 = sub nsw i32 %42, %realsz.1
  %call207 = call i32 @__printf_pad(ptr noundef %io, i32 noundef %sub206, i32 noundef 0) #7
  %.pre = load i8, ptr %sign, align 1, !tbaa !13
  br label %if.end209

if.end209:                                        ; preds = %if.then205, %land.lhs.true202, %here
  %43 = phi i8 [ %.pre, %if.then205 ], [ %38, %land.lhs.true202 ], [ %38, %here ]
  %ret.0 = phi i32 [ %call207, %if.then205 ], [ 0, %land.lhs.true202 ], [ 0, %here ]
  %tobool210.not = icmp eq i8 %43, 0
  br i1 %tobool210.not, label %if.end214, label %if.then211

if.then211:                                       ; preds = %if.end209
  %call212 = call i32 @__printf_puts(ptr noundef %io, ptr noundef nonnull %sign, i32 noundef 1) #7
  %add213 = add nsw i32 %call212, %ret.0
  br label %if.end214

if.end214:                                        ; preds = %if.then211, %if.end209
  %ret.1 = phi i32 [ %add213, %if.then211 ], [ %ret.0, %if.end209 ]
  %44 = load i8, ptr %arrayidx, align 1, !tbaa !13
  %tobool216.not = icmp eq i8 %44, 0
  br i1 %tobool216.not, label %if.end222, label %if.then217

if.then217:                                       ; preds = %if.end214
  store i8 48, ptr %ox, align 1, !tbaa !13
  %call220 = call i32 @__printf_puts(ptr noundef %io, ptr noundef nonnull %ox, i32 noundef 2) #7
  %add221 = add nsw i32 %call220, %ret.1
  br label %if.end222

if.end222:                                        ; preds = %if.then217, %if.end214
  %ret.2 = phi i32 [ %add221, %if.then217 ], [ %ret.1, %if.end214 ]
  %45 = load i32, ptr %pad, align 8, !tbaa !29
  %cmp224 = icmp eq i32 %45, 48
  br i1 %cmp224, label %land.lhs.true226, label %if.end235

land.lhs.true226:                                 ; preds = %if.end222
  %left227 = getelementptr inbounds %struct.printf_info, ptr %pi, i64 0, i32 9
  %46 = load i32, ptr %left227, align 4, !tbaa !30
  %cmp228 = icmp eq i32 %46, 0
  br i1 %cmp228, label %if.then230, label %if.end235

if.then230:                                       ; preds = %land.lhs.true226
  %width231 = getelementptr inbounds %struct.printf_info, ptr %pi, i64 0, i32 1
  %47 = load i32, ptr %width231, align 4, !tbaa !31
  %sub232 = sub nsw i32 %47, %realsz.1
  %call233 = call i32 @__printf_pad(ptr noundef %io, i32 noundef %sub232, i32 noundef 1) #7
  %add234 = add nsw i32 %call233, %ret.2
  br label %if.end235

if.end235:                                        ; preds = %if.then230, %land.lhs.true226, %if.end222
  %ret.3 = phi i32 [ %add234, %if.then230 ], [ %ret.2, %land.lhs.true226 ], [ %ret.2, %if.end222 ]
  %sub236 = xor i32 %size.2, -1
  %call237 = call i32 @__printf_pad(ptr noundef %io, i32 noundef %sub236, i32 noundef 1) #7
  %add238 = add nsw i32 %call237, %ret.3
  br i1 %cmp80.not, label %if.then240, label %if.else243

if.then240:                                       ; preds = %if.end235
  %call241 = call i32 @__printf_puts(ptr noundef %io, ptr noundef %cp.3, i32 noundef %size.2) #7
  %add242 = add nsw i32 %call241, %add238
  br label %if.end405

if.else243:                                       ; preds = %if.end235
  %tobool244.not = icmp eq i8 %expchar.4, 0
  br i1 %tobool244.not, label %if.then245, label %if.else378

if.then245:                                       ; preds = %if.else243
  %48 = load i32, ptr %expt, align 4, !tbaa !5
  %cmp246 = icmp slt i32 %48, 1
  br i1 %cmp246, label %if.then248, label %do.body

if.then248:                                       ; preds = %if.then245
  %call249 = call i32 @__printf_puts(ptr noundef %io, ptr noundef nonnull @.str.7, i32 noundef 1) #7
  %add250 = add nsw i32 %call249, %add238
  %tobool251.not = icmp eq i32 %prec.6, 0
  br i1 %tobool251.not, label %lor.lhs.false252, label %if.then255

lor.lhs.false252:                                 ; preds = %if.then248
  %49 = load i32, ptr %alt, align 4, !tbaa !17
  %tobool254.not = icmp eq i32 %49, 0
  br i1 %tobool254.not, label %if.end258, label %if.then255

if.then255:                                       ; preds = %lor.lhs.false252, %if.then248
  %call256 = call i32 @__printf_puts(ptr noundef %io, ptr noundef %6, i32 noundef 1) #7
  %add257 = add nsw i32 %call256, %add250
  br label %if.end258

if.end258:                                        ; preds = %if.then255, %lor.lhs.false252
  %ret.4 = phi i32 [ %add257, %if.then255 ], [ %add250, %lor.lhs.false252 ]
  %50 = load i32, ptr %expt, align 4, !tbaa !5
  %sub259 = sub nsw i32 0, %50
  %call260 = call i32 @__printf_pad(ptr noundef %io, i32 noundef %sub259, i32 noundef 1) #7
  %add261 = add nsw i32 %call260, %ret.4
  %51 = load i32, ptr %expt, align 4, !tbaa !5
  %add262 = add nsw i32 %51, %prec.6
  br label %do.body352

do.body:                                          ; preds = %if.then245
  %52 = load ptr, ptr %dtoaend, align 8, !tbaa !21
  %sub.ptr.lhs.cast264 = ptrtoint ptr %52 to i64
  %sub.ptr.rhs.cast265 = ptrtoint ptr %cp.3 to i64
  %sub.ptr.sub266 = sub i64 %sub.ptr.lhs.cast264, %sub.ptr.rhs.cast265
  %conv267 = trunc i64 %sub.ptr.sub266 to i32
  %spec.select609 = call i32 @llvm.smin.i32(i32 %lead.1, i32 %conv267)
  %cmp272 = icmp sgt i32 %spec.select609, 0
  br i1 %cmp272, label %if.then274, label %if.end277

if.then274:                                       ; preds = %do.body
  %call275 = call i32 @__printf_puts(ptr noundef %io, ptr noundef %cp.3, i32 noundef %spec.select609) #7
  %add276 = add nsw i32 %call275, %add238
  br label %if.end277

if.end277:                                        ; preds = %if.then274, %do.body
  %ret.5 = phi i32 [ %add276, %if.then274 ], [ %add238, %do.body ]
  %cond283 = call i32 @llvm.smax.i32(i32 %spec.select609, i32 0)
  %sub284 = sub nsw i32 %lead.1, %cond283
  %call285 = call i32 @__printf_pad(ptr noundef %io, i32 noundef %sub284, i32 noundef 1) #7
  %add286 = add nsw i32 %call285, %ret.5
  %idx.ext = sext i32 %lead.1 to i64
  %add.ptr287 = getelementptr inbounds i8, ptr %cp.3, i64 %idx.ext
  %tobool288.not = icmp eq ptr %grouping.3, null
  br i1 %tobool288.not, label %if.end342, label %while.cond290.preheader

while.cond290.preheader:                          ; preds = %if.end277
  %cmp291631 = icmp sgt i32 %nseps.2, 0
  %cmp293632 = icmp sgt i32 %nrepeats.2, 0
  %53 = select i1 %cmp291631, i1 true, i1 %cmp293632
  br i1 %53, label %while.body295, label %while.end337

while.body295:                                    ; preds = %while.cond290.preheader, %if.end321
  %cmp293638 = phi i1 [ %cmp293, %if.end321 ], [ %cmp293632, %while.cond290.preheader ]
  %ret.6637 = phi i32 [ %add331, %if.end321 ], [ %add286, %while.cond290.preheader ]
  %grouping.4636 = phi ptr [ %grouping.5, %if.end321 ], [ %grouping.3, %while.cond290.preheader ]
  %nrepeats.3635 = phi i32 [ %nrepeats.4, %if.end321 ], [ %nrepeats.2, %while.cond290.preheader ]
  %nseps.3634 = phi i32 [ %nseps.4, %if.end321 ], [ %nseps.2, %while.cond290.preheader ]
  %cp.4633 = phi ptr [ %add.ptr336, %if.end321 ], [ %add.ptr287, %while.cond290.preheader ]
  %not.cmp293 = xor i1 %cmp293638, true
  %dec301 = sext i1 %not.cmp293 to i32
  %nseps.4 = add nsw i32 %nseps.3634, %dec301
  %dec = sext i1 %cmp293638 to i32
  %nrepeats.4 = add nsw i32 %nrepeats.3635, %dec
  %grouping.5.idx = sext i1 %not.cmp293 to i64
  %grouping.5 = getelementptr inbounds i8, ptr %grouping.4636, i64 %grouping.5.idx
  %call303 = call i32 @__printf_puts(ptr noundef %io, ptr noundef nonnull %thousands_sep, i32 noundef 1) #7
  %add304 = add nsw i32 %call303, %ret.6637
  %54 = load ptr, ptr %dtoaend, align 8, !tbaa !21
  %sub.ptr.lhs.cast306 = ptrtoint ptr %54 to i64
  %sub.ptr.rhs.cast307 = ptrtoint ptr %cp.4633 to i64
  %sub.ptr.sub308 = sub i64 %sub.ptr.lhs.cast306, %sub.ptr.rhs.cast307
  %conv309 = trunc i64 %sub.ptr.sub308 to i32
  %55 = load i8, ptr %grouping.5, align 1, !tbaa !13
  %conv310 = sext i8 %55 to i32
  %n2.1 = call i32 @llvm.smin.i32(i32 %conv309, i32 %conv310)
  %cmp316 = icmp sgt i32 %n2.1, 0
  br i1 %cmp316, label %if.then318, label %if.end321

if.then318:                                       ; preds = %while.body295
  %call319 = call i32 @__printf_puts(ptr noundef %io, ptr noundef %cp.4633, i32 noundef %n2.1) #7
  %add320 = add nsw i32 %call319, %add304
  %.pre641 = load i8, ptr %grouping.5, align 1, !tbaa !13
  %.pre642 = sext i8 %.pre641 to i32
  br label %if.end321

if.end321:                                        ; preds = %if.then318, %while.body295
  %conv322.pre-phi = phi i32 [ %.pre642, %if.then318 ], [ %conv310, %while.body295 ]
  %ret.7 = phi i32 [ %add320, %if.then318 ], [ %add304, %while.body295 ]
  %cond328 = call i32 @llvm.smax.i32(i32 %n2.1, i32 0)
  %sub329 = sub nsw i32 %conv322.pre-phi, %cond328
  %call330 = call i32 @__printf_pad(ptr noundef %io, i32 noundef %sub329, i32 noundef 1) #7
  %add331 = add nsw i32 %call330, %ret.7
  %56 = load i8, ptr %grouping.5, align 1, !tbaa !13
  %idx.ext335 = sext i8 %56 to i64
  %add.ptr336 = getelementptr inbounds i8, ptr %cp.4633, i64 %idx.ext335
  %cmp291 = icmp sgt i32 %nseps.4, 0
  %cmp293 = icmp sgt i32 %nrepeats.4, 0
  %57 = select i1 %cmp291, i1 true, i1 %cmp293
  br i1 %57, label %while.body295, label %while.end337, !llvm.loop !32

while.end337:                                     ; preds = %if.end321, %while.cond290.preheader
  %cp.4.lcssa = phi ptr [ %add.ptr287, %while.cond290.preheader ], [ %add.ptr336, %if.end321 ]
  %ret.6.lcssa = phi i32 [ %add286, %while.cond290.preheader ], [ %add331, %if.end321 ]
  %58 = load ptr, ptr %dtoaend, align 8, !tbaa !21
  %cmp338 = icmp ugt ptr %cp.4.lcssa, %58
  %spec.select610 = select i1 %cmp338, ptr %58, ptr %cp.4.lcssa
  br label %if.end342

if.end342:                                        ; preds = %while.end337, %if.end277
  %cp.5 = phi ptr [ %add.ptr287, %if.end277 ], [ %spec.select610, %while.end337 ]
  %ret.8 = phi i32 [ %add286, %if.end277 ], [ %ret.6.lcssa, %while.end337 ]
  %tobool343.not = icmp eq i32 %prec.6, 0
  br i1 %tobool343.not, label %lor.lhs.false344, label %if.then347

lor.lhs.false344:                                 ; preds = %if.end342
  %59 = load i32, ptr %alt, align 4, !tbaa !17
  %tobool346.not = icmp eq i32 %59, 0
  br i1 %tobool346.not, label %do.body352, label %if.then347

if.then347:                                       ; preds = %lor.lhs.false344, %if.end342
  %call348 = call i32 @__printf_puts(ptr noundef %io, ptr noundef %6, i32 noundef 1) #7
  %add349 = add nsw i32 %call348, %ret.8
  br label %do.body352

do.body352:                                       ; preds = %if.end258, %if.then347, %lor.lhs.false344
  %cp.6 = phi ptr [ %cp.3, %if.end258 ], [ %cp.5, %if.then347 ], [ %cp.5, %lor.lhs.false344 ]
  %ret.9 = phi i32 [ %add261, %if.end258 ], [ %add349, %if.then347 ], [ %ret.8, %lor.lhs.false344 ]
  %prec.7 = phi i32 [ %add262, %if.end258 ], [ %prec.6, %if.then347 ], [ 0, %lor.lhs.false344 ]
  %60 = load ptr, ptr %dtoaend, align 8, !tbaa !21
  %sub.ptr.lhs.cast353 = ptrtoint ptr %60 to i64
  %sub.ptr.rhs.cast354 = ptrtoint ptr %cp.6 to i64
  %sub.ptr.sub355 = sub i64 %sub.ptr.lhs.cast353, %sub.ptr.rhs.cast354
  %conv356 = trunc i64 %sub.ptr.sub355 to i32
  %spec.select611 = call i32 @llvm.smin.i32(i32 %prec.7, i32 %conv356)
  %cmp361 = icmp sgt i32 %spec.select611, 0
  br i1 %cmp361, label %if.then363, label %if.end366

if.then363:                                       ; preds = %do.body352
  %call364 = call i32 @__printf_puts(ptr noundef %io, ptr noundef %cp.6, i32 noundef %spec.select611) #7
  %add365 = add nsw i32 %call364, %ret.9
  br label %if.end366

if.end366:                                        ; preds = %if.then363, %do.body352
  %ret.10 = phi i32 [ %add365, %if.then363 ], [ %ret.9, %do.body352 ]
  %cond372 = call i32 @llvm.smax.i32(i32 %spec.select611, i32 0)
  %sub373 = sub nsw i32 %prec.7, %cond372
  %call374 = call i32 @__printf_pad(ptr noundef %io, i32 noundef %sub373, i32 noundef 1) #7
  %add375 = add nsw i32 %call374, %ret.10
  br label %if.end405

if.else378:                                       ; preds = %if.else243
  %cmp379 = icmp sgt i32 %prec.6, 1
  br i1 %cmp379, label %if.then384, label %lor.lhs.false381

lor.lhs.false381:                                 ; preds = %if.else378
  %61 = load i32, ptr %alt, align 4, !tbaa !17
  %tobool383.not = icmp eq i32 %61, 0
  br i1 %tobool383.not, label %if.else397, label %if.then384

if.then384:                                       ; preds = %lor.lhs.false381, %if.else378
  %incdec.ptr385 = getelementptr inbounds i8, ptr %cp.3, i64 1
  %62 = load i8, ptr %cp.3, align 1, !tbaa !13
  store i8 %62, ptr %buf, align 16, !tbaa !13
  %63 = load i8, ptr %6, align 1, !tbaa !13
  %arrayidx387 = getelementptr inbounds [100 x i8], ptr %buf, i64 0, i64 1
  store i8 %63, ptr %arrayidx387, align 1, !tbaa !13
  %call389 = call i32 @__printf_puts(ptr noundef %io, ptr noundef nonnull %buf, i32 noundef 2) #7
  %add390 = add nsw i32 %call389, %add238
  %sub391 = add nsw i32 %ndig.0, -1
  %call392 = call i32 @__printf_puts(ptr noundef %io, ptr noundef nonnull %incdec.ptr385, i32 noundef %sub391) #7
  %add393 = add nsw i32 %add390, %call392
  %sub394 = sub nsw i32 %prec.6, %ndig.0
  %call395 = call i32 @__printf_pad(ptr noundef %io, i32 noundef %sub394, i32 noundef 1) #7
  %add396 = add nsw i32 %add393, %call395
  br label %if.end400

if.else397:                                       ; preds = %lor.lhs.false381
  %call398 = call i32 @__printf_puts(ptr noundef %io, ptr noundef %cp.3, i32 noundef 1) #7
  %add399 = add nsw i32 %call398, %add238
  br label %if.end400

if.end400:                                        ; preds = %if.else397, %if.then384
  %ret.11 = phi i32 [ %add396, %if.then384 ], [ %add399, %if.else397 ]
  %call402 = call i32 @__printf_puts(ptr noundef %io, ptr noundef nonnull %expstr, i32 noundef %expsize.0) #7
  %add403 = add nsw i32 %call402, %ret.11
  br label %if.end405

if.end405:                                        ; preds = %if.end366, %if.end400, %if.then240
  %ret.12 = phi i32 [ %add242, %if.then240 ], [ %add403, %if.end400 ], [ %add375, %if.end366 ]
  %left406 = getelementptr inbounds %struct.printf_info, ptr %pi, i64 0, i32 9
  %64 = load i32, ptr %left406, align 4, !tbaa !30
  %tobool407.not = icmp eq i32 %64, 0
  br i1 %tobool407.not, label %if.end413, label %if.then408

if.then408:                                       ; preds = %if.end405
  %width409 = getelementptr inbounds %struct.printf_info, ptr %pi, i64 0, i32 1
  %65 = load i32, ptr %width409, align 4, !tbaa !31
  %sub410 = sub nsw i32 %65, %realsz.1
  %call411 = call i32 @__printf_pad(ptr noundef %io, i32 noundef %sub410, i32 noundef 0) #7
  %add412 = add nsw i32 %call411, %ret.12
  br label %if.end413

if.end413:                                        ; preds = %if.then408, %if.end405
  %ret.13 = phi i32 [ %add412, %if.then408 ], [ %ret.12, %if.end405 ]
  call void @__printf_flush(ptr noundef %io) #7
  %cmp414.not = icmp eq ptr %dtoaresult.1, null
  br i1 %cmp414.not, label %if.end417, label %if.then416

if.then416:                                       ; preds = %if.end413
  call void @__freedtoa(ptr noundef nonnull %dtoaresult.1) #7
  br label %if.end417

if.end417:                                        ; preds = %if.then416, %if.end413
  call void @llvm.lifetime.end.p0(i64 100, ptr nonnull %buf) #7
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %thousands_sep) #7
  call void @llvm.lifetime.end.p0(i64 2, ptr nonnull %ox) #7
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %ld) #7
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %expstr) #7
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %sign) #7
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %dtoaend) #7
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %signflag) #7
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %expt) #7
  ret i32 %ret.13
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #2

declare ptr @localeconv() local_unnamed_addr #3

declare ptr @__hldtoa(x86_fp80 noundef, ptr noundef, i32 noundef, ptr noundef, ptr noundef, ptr noundef) local_unnamed_addr #3

declare ptr @__hdtoa(double noundef, ptr noundef, i32 noundef, ptr noundef, ptr noundef, ptr noundef) local_unnamed_addr #3

declare ptr @__ldtoa(ptr noundef, i32 noundef, i32 noundef, ptr noundef, ptr noundef, ptr noundef) local_unnamed_addr #3

declare ptr @__dtoa(double noundef, i32 noundef, i32 noundef, ptr noundef, ptr noundef, ptr noundef) local_unnamed_addr #3

declare i32 @__printf_pad(ptr noundef, i32 noundef, i32 noundef) local_unnamed_addr #3

declare i32 @__printf_puts(ptr noundef, ptr noundef, i32 noundef) local_unnamed_addr #3

declare void @__printf_flush(ptr noundef) local_unnamed_addr #3

declare void @__freedtoa(ptr noundef) local_unnamed_addr #3

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.umax.i32(i32, i32) #4

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.smax.i32(i32, i32) #4

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.abs.i32(i32, i1 immarg) #4

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.smax.i64(i64, i64) #4

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #5

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.smin.i32(i32, i32) #4

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { noreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #3 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #5 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #6 = { noreturn nounwind }
attributes #7 = { nounwind }

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
!9 = !{!10, !6, i64 12}
!10 = !{!"printf_info", !6, i64 0, !6, i64 4, !6, i64 8, !6, i64 12, !6, i64 16, !6, i64 20, !6, i64 24, !6, i64 28, !6, i64 32, !6, i64 36, !6, i64 40, !6, i64 44, !6, i64 48, !6, i64 52, !6, i64 56, !6, i64 60, !6, i64 64, !6, i64 68, !6, i64 72, !6, i64 76, !6, i64 80, !6, i64 84, !11, i64 88, !11, i64 96, !7, i64 104}
!11 = !{!"any pointer", !7, i64 0}
!12 = !{!10, !6, i64 0}
!13 = !{!7, !7, i64 0}
!14 = !{!10, !6, i64 40}
!15 = !{!16, !11, i64 8}
!16 = !{!"lconv", !11, i64 0, !11, i64 8, !11, i64 16, !11, i64 24, !11, i64 32, !11, i64 40, !11, i64 48, !11, i64 56, !11, i64 64, !11, i64 72, !7, i64 80, !7, i64 81, !7, i64 82, !7, i64 83, !7, i64 84, !7, i64 85, !7, i64 86, !7, i64 87, !7, i64 88, !7, i64 89, !7, i64 90, !7, i64 91, !7, i64 92, !7, i64 93}
!17 = !{!10, !6, i64 28}
!18 = !{!16, !11, i64 16}
!19 = !{!16, !11, i64 0}
!20 = !{!10, !6, i64 8}
!21 = !{!11, !11, i64 0}
!22 = !{!23, !23, i64 0}
!23 = !{!"long double", !7, i64 0}
!24 = !{!25, !25, i64 0}
!25 = !{!"double", !7, i64 0}
!26 = distinct !{!26, !27}
!27 = !{!"llvm.loop.mustprogress"}
!28 = distinct !{!28, !27}
!29 = !{!10, !6, i64 56}
!30 = !{!10, !6, i64 36}
!31 = !{!10, !6, i64 4}
!32 = distinct !{!32, !27}
