; ModuleID = '/home/odin/pbsd/pbsd/lib/libc/stdio/fvwrite_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/libc/stdio/fvwrite_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

module asm ".symver __qsort_r_compat, qsort_r@FBSD_1.0"

%struct.__suio = type { ptr, i32, i32 }
%struct.__sFILE = type { ptr, i32, i32, i16, i16, %struct.__sbuf, i32, ptr, ptr, ptr, ptr, ptr, %struct.__sbuf, ptr, i32, [3 x i8], [1 x i8], %struct.__sbuf, i32, i64, ptr, ptr, i32, i32, %union.__mbstate_t, i32 }
%struct.__sbuf = type { ptr, i32 }
%union.__mbstate_t = type { i64, [120 x i8] }
%struct.__siov = type { ptr, i64 }

; Function Attrs: nounwind uwtable
define dso_local noundef i32 @ref___sfvwrite(ptr noundef %fp, ptr nocapture noundef %uio) local_unnamed_addr #0 {
entry:
  %uio_resid = getelementptr inbounds %struct.__suio, ptr %uio, i64 0, i32 2
  %0 = load i32, ptr %uio_resid, align 4, !tbaa !5
  %cmp = icmp eq i32 %0, 0
  br i1 %cmp, label %cleanup269, label %if.end

if.end:                                           ; preds = %entry
  %_flags = getelementptr inbounds %struct.__sFILE, ptr %fp, i64 0, i32 3
  %1 = load i16, ptr %_flags, align 8, !tbaa !11
  %conv443 = zext i16 %1 to i32
  %and = and i32 %conv443, 8
  %cmp1 = icmp eq i32 %and, 0
  br i1 %cmp1, label %land.rhs, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %if.end
  %_bf = getelementptr inbounds %struct.__sFILE, ptr %fp, i64 0, i32 5
  %2 = load ptr, ptr %_bf, align 8, !tbaa !16
  %cmp3 = icmp eq ptr %2, null
  %and7 = and i32 %conv443, 512
  %cmp8 = icmp eq i32 %and7, 0
  %or.cond = and i1 %cmp8, %cmp3
  br i1 %or.cond, label %land.rhs, label %if.end13

land.rhs:                                         ; preds = %lor.lhs.false, %if.end
  %call = tail call i32 @__swsetup(ptr noundef nonnull %fp) #6
  %tobool.not = icmp eq i32 %call, 0
  br i1 %tobool.not, label %land.rhs.if.end13_crit_edge, label %cleanup269

land.rhs.if.end13_crit_edge:                      ; preds = %land.rhs
  %.pre = load i16, ptr %_flags, align 8, !tbaa !11
  %.pre442444 = zext i16 %.pre to i32
  br label %if.end13

if.end13:                                         ; preds = %land.rhs.if.end13_crit_edge, %lor.lhs.false
  %conv15.pre-phi = phi i32 [ %.pre442444, %land.rhs.if.end13_crit_edge ], [ %conv443, %lor.lhs.false ]
  %3 = load ptr, ptr %uio, align 8, !tbaa !17
  %4 = load ptr, ptr %3, align 8, !tbaa !18
  %iov_len = getelementptr inbounds %struct.__siov, ptr %3, i64 0, i32 1
  %5 = load i64, ptr %iov_len, align 8, !tbaa !20
  %incdec.ptr = getelementptr inbounds %struct.__siov, ptr %3, i64 1
  %and16 = and i32 %conv15.pre-phi, 2
  %tobool17.not = icmp eq i32 %and16, 0
  br i1 %tobool17.not, label %if.else, label %do.body

do.body:                                          ; preds = %if.end13, %if.end31
  %iov.0 = phi ptr [ %iov.1.lcssa, %if.end31 ], [ %incdec.ptr, %if.end13 ]
  %p.0 = phi ptr [ %add.ptr, %if.end31 ], [ %4, %if.end13 ]
  %len.0 = phi i64 [ %sub, %if.end31 ], [ %5, %if.end13 ]
  %cmp19418 = icmp eq i64 %len.0, 0
  br i1 %cmp19418, label %while.body, label %while.end

while.body:                                       ; preds = %do.body, %while.body
  %iov.1419 = phi ptr [ %incdec.ptr23, %while.body ], [ %iov.0, %do.body ]
  %iov_len22 = getelementptr inbounds %struct.__siov, ptr %iov.1419, i64 0, i32 1
  %6 = load i64, ptr %iov_len22, align 8, !tbaa !20
  %incdec.ptr23 = getelementptr inbounds %struct.__siov, ptr %iov.1419, i64 1
  %cmp19 = icmp eq i64 %6, 0
  br i1 %cmp19, label %while.body, label %while.cond.while.end_crit_edge, !llvm.loop !21

while.cond.while.end_crit_edge:                   ; preds = %while.body
  %7 = load ptr, ptr %iov.1419, align 8, !tbaa !18
  br label %while.end

while.end:                                        ; preds = %while.cond.while.end_crit_edge, %do.body
  %iov.1.lcssa = phi ptr [ %incdec.ptr23, %while.cond.while.end_crit_edge ], [ %iov.0, %do.body ]
  %p.1.lcssa = phi ptr [ %7, %while.cond.while.end_crit_edge ], [ %p.0, %do.body ]
  %len.1.lcssa = phi i64 [ %6, %while.cond.while.end_crit_edge ], [ %len.0, %do.body ]
  %cond = tail call i64 @llvm.umin.i64(i64 %len.1.lcssa, i64 1024)
  %conv26 = trunc i64 %cond to i32
  %call27 = tail call i32 @_swrite(ptr noundef %fp, ptr noundef %p.1.lcssa, i32 noundef %conv26) #6
  %cmp28 = icmp slt i32 %call27, 1
  br i1 %cmp28, label %err, label %if.end31

if.end31:                                         ; preds = %while.end
  %idx.ext = zext nneg i32 %call27 to i64
  %add.ptr = getelementptr inbounds i8, ptr %p.1.lcssa, i64 %idx.ext
  %sub = sub i64 %len.1.lcssa, %idx.ext
  %8 = load i32, ptr %uio_resid, align 4, !tbaa !5
  %sub34 = sub nsw i32 %8, %call27
  store i32 %sub34, ptr %uio_resid, align 4, !tbaa !5
  %cmp35.not = icmp eq i32 %8, %call27
  br i1 %cmp35.not, label %cleanup269, label %do.body, !llvm.loop !23

if.else:                                          ; preds = %if.end13
  %and39 = and i32 %conv15.pre-phi, 1
  %cmp40 = icmp eq i32 %and39, 0
  %_w = getelementptr inbounds %struct.__sFILE, ptr %fp, i64 0, i32 2
  %_bf62 = getelementptr inbounds %struct.__sFILE, ptr %fp, i64 0, i32 5
  %_size = getelementptr inbounds %struct.__sFILE, ptr %fp, i64 0, i32 5, i32 1
  br i1 %cmp40, label %do.body43, label %do.body169

do.body43:                                        ; preds = %if.else, %if.end157
  %iov.2 = phi ptr [ %iov.3.lcssa, %if.end157 ], [ %incdec.ptr, %if.else ]
  %p.2 = phi ptr [ %add.ptr159, %if.end157 ], [ %4, %if.else ]
  %len.2 = phi i64 [ %sub161, %if.end157 ], [ %5, %if.else ]
  %cmp45428 = icmp eq i64 %len.2, 0
  br i1 %cmp45428, label %while.body47, label %while.end51

while.body47:                                     ; preds = %do.body43, %while.body47
  %iov.3429 = phi ptr [ %incdec.ptr50, %while.body47 ], [ %iov.2, %do.body43 ]
  %iov_len49 = getelementptr inbounds %struct.__siov, ptr %iov.3429, i64 0, i32 1
  %9 = load i64, ptr %iov_len49, align 8, !tbaa !20
  %incdec.ptr50 = getelementptr inbounds %struct.__siov, ptr %iov.3429, i64 1
  %cmp45 = icmp eq i64 %9, 0
  br i1 %cmp45, label %while.body47, label %while.cond44.while.end51_crit_edge, !llvm.loop !24

while.cond44.while.end51_crit_edge:               ; preds = %while.body47
  %10 = load ptr, ptr %iov.3429, align 8, !tbaa !18
  br label %while.end51

while.end51:                                      ; preds = %while.cond44.while.end51_crit_edge, %do.body43
  %iov.3.lcssa = phi ptr [ %incdec.ptr50, %while.cond44.while.end51_crit_edge ], [ %iov.2, %do.body43 ]
  %p.3.lcssa = phi ptr [ %10, %while.cond44.while.end51_crit_edge ], [ %p.2, %do.body43 ]
  %len.3.lcssa = phi i64 [ %9, %while.cond44.while.end51_crit_edge ], [ %len.2, %do.body43 ]
  %11 = load i16, ptr %_flags, align 8, !tbaa !11
  %12 = and i16 %11, 16896
  %cmp55 = icmp eq i16 %12, 16896
  %.pre440 = load i32, ptr %_w, align 4, !tbaa !25
  %conv58 = sext i32 %.pre440 to i64
  %cmp59 = icmp ugt i64 %len.3.lcssa, %conv58
  %or.cond462 = select i1 %cmp55, i1 %cmp59, i1 false
  br i1 %or.cond462, label %if.then61, label %if.end89

if.then61:                                        ; preds = %while.end51
  %13 = load ptr, ptr %fp, align 8, !tbaa !26
  %14 = load ptr, ptr %_bf62, align 8, !tbaa !16
  %sub.ptr.lhs.cast = ptrtoint ptr %13 to i64
  %sub.ptr.rhs.cast = ptrtoint ptr %14 to i64
  %sub.ptr.sub = sub i64 %sub.ptr.lhs.cast, %sub.ptr.rhs.cast
  %15 = trunc i64 %len.3.lcssa to i32
  %conv64 = add i32 %15, 128
  store i32 %conv64, ptr %_w, align 4, !tbaa !25
  %add66 = add i64 %sub.ptr.sub, %len.3.lcssa
  %16 = trunc i64 %add66 to i32
  %conv68 = add i32 %16, 128
  store i32 %conv68, ptr %_size, align 8, !tbaa !27
  %add74 = shl i64 %add66, 32
  %sext410 = add i64 %add74, 554050781184
  %conv75 = ashr exact i64 %sext410, 32
  %call76 = tail call ptr @reallocf(ptr noundef %14, i64 noundef %conv75) #7
  store ptr %call76, ptr %_bf62, align 8, !tbaa !16
  %cmp81 = icmp eq ptr %call76, null
  br i1 %cmp81, label %err, label %cleanup.thread

cleanup.thread:                                   ; preds = %if.then61
  %add.ptr87 = getelementptr inbounds i8, ptr %call76, i64 %sub.ptr.sub
  store ptr %add.ptr87, ptr %fp, align 8, !tbaa !26
  %.pre439 = load i32, ptr %_w, align 4, !tbaa !25
  %.pre441 = load i16, ptr %_flags, align 8, !tbaa !11
  br label %if.end89

if.end89:                                         ; preds = %cleanup.thread, %while.end51
  %17 = phi i16 [ %.pre441, %cleanup.thread ], [ %11, %while.end51 ]
  %18 = phi i32 [ %.pre439, %cleanup.thread ], [ %.pre440, %while.end51 ]
  %19 = and i16 %17, 512
  %tobool94.not = icmp eq i16 %19, 0
  br i1 %tobool94.not, label %if.else114, label %if.then95

if.then95:                                        ; preds = %if.end89
  %conv96 = sext i32 %18 to i64
  %cmp97 = icmp ult i64 %len.3.lcssa, %conv96
  %conv100 = trunc i64 %len.3.lcssa to i32
  %spec.select = select i1 %cmp97, i32 %conv100, i32 %18
  %cmp102 = icmp sgt i32 %spec.select, 0
  br i1 %cmp102, label %if.then104, label %if.end157

if.then104:                                       ; preds = %if.then95
  %20 = load ptr, ptr %fp, align 8, !tbaa !26
  %conv106 = zext nneg i32 %spec.select to i64
  tail call void @llvm.memcpy.p0.p0.i64(ptr align 1 %20, ptr align 1 %p.3.lcssa, i64 %conv106, i1 false)
  br label %if.end157.sink.split

if.else114:                                       ; preds = %if.end89
  %21 = load ptr, ptr %fp, align 8, !tbaa !26
  %22 = load ptr, ptr %_bf62, align 8, !tbaa !16
  %cmp118 = icmp ugt ptr %21, %22
  br i1 %cmp118, label %land.lhs.true120, label %if.else134

land.lhs.true120:                                 ; preds = %if.else114
  %conv121 = sext i32 %18 to i64
  %cmp122 = icmp ugt i64 %len.3.lcssa, %conv121
  br i1 %cmp122, label %if.then124, label %if.else134

if.then124:                                       ; preds = %land.lhs.true120
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 1 %21, ptr align 1 %p.3.lcssa, i64 %conv121, i1 false)
  %23 = load ptr, ptr %fp, align 8, !tbaa !26
  %add.ptr129 = getelementptr inbounds i8, ptr %23, i64 %conv121
  store ptr %add.ptr129, ptr %fp, align 8, !tbaa !26
  %call130 = tail call i32 @__fflush(ptr noundef nonnull %fp) #6
  %tobool131.not = icmp eq i32 %call130, 0
  br i1 %tobool131.not, label %if.end157, label %err

if.else134:                                       ; preds = %land.lhs.true120, %if.else114
  %24 = load i32, ptr %_size, align 8, !tbaa !27
  %conv137 = sext i32 %24 to i64
  %cmp138.not = icmp ult i64 %len.3.lcssa, %conv137
  br i1 %cmp138.not, label %if.else146, label %if.then140

if.then140:                                       ; preds = %if.else134
  %call141 = tail call i32 @_swrite(ptr noundef nonnull %fp, ptr noundef %p.3.lcssa, i32 noundef %24) #6
  %cmp142 = icmp slt i32 %call141, 1
  br i1 %cmp142, label %err, label %if.end157

if.else146:                                       ; preds = %if.else134
  %conv147 = trunc i64 %len.3.lcssa to i32
  %sext411 = shl i64 %len.3.lcssa, 32
  %conv149 = ashr exact i64 %sext411, 32
  tail call void @llvm.memcpy.p0.p0.i64(ptr align 1 %21, ptr align 1 %p.3.lcssa, i64 %conv149, i1 false)
  br label %if.end157.sink.split

if.end157.sink.split:                             ; preds = %if.else146, %if.then104
  %spec.select.sink = phi i32 [ %spec.select, %if.then104 ], [ %conv147, %if.else146 ]
  %conv106.sink = phi i64 [ %conv106, %if.then104 ], [ %conv149, %if.else146 ]
  %w.1.ph = phi i32 [ %conv100, %if.then104 ], [ %conv147, %if.else146 ]
  %25 = load i32, ptr %_w, align 4, !tbaa !25
  %sub108 = sub nsw i32 %25, %spec.select.sink
  store i32 %sub108, ptr %_w, align 4, !tbaa !25
  %26 = load ptr, ptr %fp, align 8, !tbaa !26
  %add.ptr111 = getelementptr inbounds i8, ptr %26, i64 %conv106.sink
  store ptr %add.ptr111, ptr %fp, align 8, !tbaa !26
  br label %if.end157

if.end157:                                        ; preds = %if.end157.sink.split, %if.then95, %if.then124, %if.then140
  %w.1 = phi i32 [ %18, %if.then124 ], [ %call141, %if.then140 ], [ %conv100, %if.then95 ], [ %w.1.ph, %if.end157.sink.split ]
  %idx.ext158 = sext i32 %w.1 to i64
  %add.ptr159 = getelementptr inbounds i8, ptr %p.3.lcssa, i64 %idx.ext158
  %sub161 = sub i64 %len.3.lcssa, %idx.ext158
  %27 = load i32, ptr %uio_resid, align 4, !tbaa !5
  %sub164 = sub nsw i32 %27, %w.1
  store i32 %sub164, ptr %uio_resid, align 4, !tbaa !5
  %cmp165.not = icmp eq i32 %27, %w.1
  br i1 %cmp165.not, label %cleanup269, label %do.body43, !llvm.loop !28

do.body169:                                       ; preds = %if.else, %if.end253
  %iov.4 = phi ptr [ %iov.5.lcssa450, %if.end253 ], [ %incdec.ptr, %if.else ]
  %p.4 = phi ptr [ %add.ptr255, %if.end253 ], [ %4, %if.else ]
  %len.4 = phi i64 [ %sub257, %if.end253 ], [ %5, %if.else ]
  %nlknown.0 = phi i32 [ %nlknown.3, %if.end253 ], [ 0, %if.else ]
  %nldist.0 = phi i32 [ %sub245, %if.end253 ], [ 0, %if.else ]
  %cmp171422 = icmp eq i64 %len.4, 0
  br i1 %cmp171422, label %while.body173, label %while.end177

while.body173:                                    ; preds = %do.body169, %while.body173
  %iov.5423 = phi ptr [ %incdec.ptr176, %while.body173 ], [ %iov.4, %do.body169 ]
  %iov_len175 = getelementptr inbounds %struct.__siov, ptr %iov.5423, i64 0, i32 1
  %28 = load i64, ptr %iov_len175, align 8, !tbaa !20
  %incdec.ptr176 = getelementptr inbounds %struct.__siov, ptr %iov.5423, i64 1
  %cmp171 = icmp eq i64 %28, 0
  br i1 %cmp171, label %while.body173, label %while.end177.thread, !llvm.loop !29

while.end177.thread:                              ; preds = %while.body173
  %29 = load ptr, ptr %iov.5423, align 8, !tbaa !18
  br label %if.then179

while.end177:                                     ; preds = %do.body169
  %tobool178.not = icmp eq i32 %nlknown.0, 0
  br i1 %tobool178.not, label %if.then179, label %if.end192

if.then179:                                       ; preds = %while.end177.thread, %while.end177
  %len.5.lcssa455 = phi i64 [ %28, %while.end177.thread ], [ %len.4, %while.end177 ]
  %p.5.lcssa452 = phi ptr [ %29, %while.end177.thread ], [ %p.4, %while.end177 ]
  %iov.5.lcssa451 = phi ptr [ %incdec.ptr176, %while.end177.thread ], [ %iov.4, %while.end177 ]
  %call180 = tail call ptr @memchr(ptr noundef %p.5.lcssa452, i32 noundef 10, i64 noundef %len.5.lcssa455) #8
  %tobool181.not = icmp eq ptr %call180, null
  %add.ptr183 = getelementptr inbounds i8, ptr %call180, i64 1
  %sub.ptr.lhs.cast184 = ptrtoint ptr %add.ptr183 to i64
  %sub.ptr.rhs.cast185 = ptrtoint ptr %p.5.lcssa452 to i64
  %sub.ptr.sub186 = sub i64 %sub.ptr.lhs.cast184, %sub.ptr.rhs.cast185
  %add188 = add i64 %len.5.lcssa455, 1
  %cond190 = select i1 %tobool181.not, i64 %add188, i64 %sub.ptr.sub186
  %conv191 = trunc i64 %cond190 to i32
  br label %if.end192

if.end192:                                        ; preds = %if.then179, %while.end177
  %len.5.lcssa454 = phi i64 [ %len.4, %while.end177 ], [ %len.5.lcssa455, %if.then179 ]
  %p.5.lcssa453 = phi ptr [ %p.4, %while.end177 ], [ %p.5.lcssa452, %if.then179 ]
  %iov.5.lcssa450 = phi ptr [ %iov.4, %while.end177 ], [ %iov.5.lcssa451, %if.then179 ]
  %nldist.1 = phi i32 [ %nldist.0, %while.end177 ], [ %conv191, %if.then179 ]
  %conv193 = sext i32 %nldist.1 to i64
  %cond200 = tail call i64 @llvm.umin.i64(i64 %len.5.lcssa454, i64 %conv193)
  %conv201 = trunc i64 %cond200 to i32
  %30 = load i32, ptr %_w, align 4, !tbaa !25
  %31 = load i32, ptr %_size, align 8, !tbaa !27
  %add205 = add nsw i32 %31, %30
  %32 = load ptr, ptr %fp, align 8, !tbaa !26
  %33 = load ptr, ptr %_bf62, align 8, !tbaa !16
  %cmp209 = icmp ugt ptr %32, %33
  %cmp212 = icmp slt i32 %add205, %conv201
  %or.cond412 = select i1 %cmp209, i1 %cmp212, i1 false
  br i1 %or.cond412, label %if.then214, label %if.else224

if.then214:                                       ; preds = %if.end192
  %conv216 = sext i32 %add205 to i64
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 1 %32, ptr align 1 %p.5.lcssa453, i64 %conv216, i1 false)
  %34 = load ptr, ptr %fp, align 8, !tbaa !26
  %add.ptr219 = getelementptr inbounds i8, ptr %34, i64 %conv216
  store ptr %add.ptr219, ptr %fp, align 8, !tbaa !26
  %call220 = tail call i32 @__fflush(ptr noundef nonnull %fp) #6
  %tobool221.not = icmp eq i32 %call220, 0
  br i1 %tobool221.not, label %if.end244, label %err

if.else224:                                       ; preds = %if.end192
  %cmp227.not = icmp sgt i32 %31, %conv201
  br i1 %cmp227.not, label %if.else235, label %if.then229

if.then229:                                       ; preds = %if.else224
  %call230 = tail call i32 @_swrite(ptr noundef nonnull %fp, ptr noundef %p.5.lcssa453, i32 noundef %31) #6
  %cmp231 = icmp slt i32 %call230, 1
  br i1 %cmp231, label %err, label %if.end244

if.else235:                                       ; preds = %if.else224
  %sext = shl i64 %cond200, 32
  %conv237 = ashr exact i64 %sext, 32
  tail call void @llvm.memcpy.p0.p0.i64(ptr align 1 %32, ptr align 1 %p.5.lcssa453, i64 %conv237, i1 false)
  %35 = load i32, ptr %_w, align 4, !tbaa !25
  %sub239 = sub nsw i32 %35, %conv201
  store i32 %sub239, ptr %_w, align 4, !tbaa !25
  %36 = load ptr, ptr %fp, align 8, !tbaa !26
  %add.ptr242 = getelementptr inbounds i8, ptr %36, i64 %conv237
  store ptr %add.ptr242, ptr %fp, align 8, !tbaa !26
  br label %if.end244

if.end244:                                        ; preds = %if.else235, %if.then229, %if.then214
  %w.2 = phi i32 [ %add205, %if.then214 ], [ %call230, %if.then229 ], [ %conv201, %if.else235 ]
  %sub245 = sub nsw i32 %nldist.1, %w.2
  %cmp246 = icmp eq i32 %sub245, 0
  br i1 %cmp246, label %if.then248, label %if.end253

if.then248:                                       ; preds = %if.end244
  %call249 = tail call i32 @__fflush(ptr noundef nonnull %fp) #6
  %tobool250.not = icmp eq i32 %call249, 0
  br i1 %tobool250.not, label %if.end253, label %err

if.end253:                                        ; preds = %if.then248, %if.end244
  %nlknown.3 = phi i32 [ 1, %if.end244 ], [ 0, %if.then248 ]
  %idx.ext254 = sext i32 %w.2 to i64
  %add.ptr255 = getelementptr inbounds i8, ptr %p.5.lcssa453, i64 %idx.ext254
  %sub257 = sub i64 %len.5.lcssa454, %idx.ext254
  %37 = load i32, ptr %uio_resid, align 4, !tbaa !5
  %sub260 = sub nsw i32 %37, %w.2
  store i32 %sub260, ptr %uio_resid, align 4, !tbaa !5
  %cmp261.not = icmp eq i32 %37, %w.2
  br i1 %cmp261.not, label %cleanup269, label %do.body169, !llvm.loop !30

err:                                              ; preds = %while.end, %if.then248, %if.then229, %if.then214, %if.then61, %if.then140, %if.then124
  %38 = load i16, ptr %_flags, align 8, !tbaa !11
  %39 = or i16 %38, 64
  store i16 %39, ptr %_flags, align 8, !tbaa !11
  br label %cleanup269

cleanup269:                                       ; preds = %if.end31, %if.end253, %if.end157, %land.rhs, %entry, %err
  %retval.0 = phi i32 [ -1, %err ], [ 0, %entry ], [ -1, %land.rhs ], [ 0, %if.end157 ], [ 0, %if.end253 ], [ 0, %if.end31 ]
  ret i32 %retval.0
}

declare i32 @__swsetup(ptr noundef) local_unnamed_addr #1

declare i32 @_swrite(ptr noundef, ptr noundef, i32 noundef) local_unnamed_addr #1

; Function Attrs: mustprogress nounwind willreturn allockind("realloc") allocsize(1) memory(argmem: readwrite, inaccessiblemem: readwrite)
declare noalias noundef ptr @reallocf(ptr allocptr nocapture noundef, i64 noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #3

declare i32 @__fflush(ptr noundef) local_unnamed_addr #1

; Function Attrs: mustprogress nofree nounwind willreturn memory(argmem: read)
declare ptr @memchr(ptr noundef, i32 noundef, i64 noundef) local_unnamed_addr #4

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umin.i64(i64, i64) #5

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { mustprogress nounwind willreturn allockind("realloc") allocsize(1) memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #4 = { mustprogress nofree nounwind willreturn memory(argmem: read) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #6 = { nounwind }
attributes #7 = { allocsize(1) }
attributes #8 = { nounwind willreturn memory(read) }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!5 = !{!6, !10, i64 12}
!6 = !{!"__suio", !7, i64 0, !10, i64 8, !10, i64 12}
!7 = !{!"any pointer", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{!"int", !8, i64 0}
!11 = !{!12, !13, i64 16}
!12 = !{!"__sFILE", !7, i64 0, !10, i64 8, !10, i64 12, !13, i64 16, !13, i64 18, !14, i64 24, !10, i64 40, !7, i64 48, !7, i64 56, !7, i64 64, !7, i64 72, !7, i64 80, !14, i64 88, !7, i64 104, !10, i64 112, !8, i64 116, !8, i64 119, !14, i64 120, !10, i64 136, !15, i64 144, !7, i64 152, !7, i64 160, !10, i64 168, !10, i64 172, !8, i64 176, !10, i64 304}
!13 = !{!"short", !8, i64 0}
!14 = !{!"__sbuf", !7, i64 0, !10, i64 8}
!15 = !{!"long", !8, i64 0}
!16 = !{!12, !7, i64 24}
!17 = !{!6, !7, i64 0}
!18 = !{!19, !7, i64 0}
!19 = !{!"__siov", !7, i64 0, !15, i64 8}
!20 = !{!19, !15, i64 8}
!21 = distinct !{!21, !22}
!22 = !{!"llvm.loop.mustprogress"}
!23 = distinct !{!23, !22}
!24 = distinct !{!24, !22}
!25 = !{!12, !10, i64 12}
!26 = !{!12, !7, i64 0}
!27 = !{!12, !10, i64 32}
!28 = distinct !{!28, !22}
!29 = distinct !{!29, !22}
!30 = distinct !{!30, !22}
