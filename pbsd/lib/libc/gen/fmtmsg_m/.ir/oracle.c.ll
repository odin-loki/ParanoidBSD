; ModuleID = '/home/odin/pbsd/pbsd/lib/libc/gen/fmtmsg_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/libc/gen/fmtmsg_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

module asm ".symver __qsort_r_compat, qsort_r@FBSD_1.0"

@.str = private unnamed_addr constant [8 x i8] c"MSGVERB\00", align 1
@.str.1 = private unnamed_addr constant [31 x i8] c"label:severity:text:action:tag\00", align 1
@__stderrp = external local_unnamed_addr global ptr, align 8
@.str.3 = private unnamed_addr constant [13 x i8] c"/dev/console\00", align 1
@.str.4 = private unnamed_addr constant [3 x i8] c"ae\00", align 1
@.str.5 = private unnamed_addr constant [6 x i8] c"label\00", align 1
@.str.6 = private unnamed_addr constant [3 x i8] c": \00", align 1
@.str.7 = private unnamed_addr constant [9 x i8] c"severity\00", align 1
@.str.8 = private unnamed_addr constant [5 x i8] c"text\00", align 1
@.str.9 = private unnamed_addr constant [7 x i8] c"action\00", align 1
@.str.10 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@.str.11 = private unnamed_addr constant [9 x i8] c"TO FIX: \00", align 1
@.str.12 = private unnamed_addr constant [4 x i8] c"tag\00", align 1
@.str.13 = private unnamed_addr constant [2 x i8] c" \00", align 1
@.str.14 = private unnamed_addr constant [5 x i8] c"HALT\00", align 1
@.str.15 = private unnamed_addr constant [6 x i8] c"ERROR\00", align 1
@.str.16 = private unnamed_addr constant [8 x i8] c"WARNING\00", align 1
@.str.17 = private unnamed_addr constant [5 x i8] c"INFO\00", align 1
@ref_nextcomp.lmsgverb = internal global [31 x i8] zeroinitializer, align 16
@ref_nextcomp.state = internal global ptr null, align 8
@.str.18 = private unnamed_addr constant [2 x i8] c":\00", align 1
@.str.19 = private unnamed_addr constant [32 x i8] c"label\00severity\00text\00action\00tag\00\00", align 1
@switch.table.ref_printfmt.20 = private unnamed_addr constant [4 x ptr] [ptr @.str.14, ptr @.str.15, ptr @.str.16, ptr @.str.17], align 8

; Function Attrs: nounwind uwtable
define dso_local noundef i32 @ref_fmtmsg(i64 noundef %class, ptr noundef %label, i32 noundef %sev, ptr noundef %text, ptr noundef %action, ptr noundef %tag) local_unnamed_addr #0 {
entry:
  %and = and i64 %class, 256
  %tobool.not = icmp eq i64 %and, 0
  br i1 %tobool.not, label %if.end35, label %if.then

if.then:                                          ; preds = %entry
  %call = tail call ptr @getenv(ptr noundef nonnull @.str)
  %cmp.not = icmp eq ptr %call, null
  br i1 %cmp.not, label %def, label %land.lhs.true

land.lhs.true:                                    ; preds = %if.then
  %0 = load i8, ptr %call, align 1, !tbaa !5
  %cmp1.not = icmp eq i8 %0, 0
  br i1 %cmp1.not, label %def, label %land.lhs.true3

land.lhs.true3:                                   ; preds = %land.lhs.true
  %call4 = tail call i64 @strlen(ptr noundef nonnull dereferenceable(1) %call) #12
  %cmp5 = icmp ult i64 %call4, 31
  br i1 %cmp5, label %if.then7, label %def

if.then7:                                         ; preds = %land.lhs.true3
  %call8 = tail call noalias ptr @strdup(ptr noundef nonnull %call)
  %cmp9 = icmp eq ptr %call8, null
  br i1 %cmp9, label %cleanup, label %while.cond.i

while.cond.loopexit.i:                            ; preds = %for.inc.i
  br label %while.cond.i, !llvm.loop !8

while.cond.i:                                     ; preds = %if.then7, %while.cond.loopexit.i
  %equality.0.i = phi i32 [ %equality.2.i, %while.cond.loopexit.i ], [ 0, %if.then7 ]
  %1 = load i8, ptr @ref_nextcomp.lmsgverb, align 16, !tbaa !5
  %cmp.i.i = icmp eq i8 %1, 0
  br i1 %cmp.i.i, label %if.then.i.i, label %if.else.i.i

if.then.i.i:                                      ; preds = %while.cond.i
  %call.i.i = tail call i64 @strlcpy(ptr noundef nonnull dereferenceable(1) @ref_nextcomp.lmsgverb, ptr noundef nonnull dereferenceable(1) %call8, i64 noundef 31) #13
  %call3.i.i = tail call ptr @strtok_r(ptr noundef nonnull @ref_nextcomp.lmsgverb, ptr noundef nonnull @.str.18, ptr noundef nonnull @ref_nextcomp.state)
  br label %if.end.i.i

if.else.i.i:                                      ; preds = %while.cond.i
  %call4.i.i = tail call ptr @strtok_r(ptr noundef null, ptr noundef nonnull @.str.18, ptr noundef nonnull @ref_nextcomp.state)
  br label %if.end.i.i

if.end.i.i:                                       ; preds = %if.else.i.i, %if.then.i.i
  %retval1.0.i.i = phi ptr [ %call3.i.i, %if.then.i.i ], [ %call4.i.i, %if.else.i.i ]
  %cmp5.i.i = icmp eq ptr %retval1.0.i.i, null
  br i1 %cmp5.i.i, label %ref_validmsgverb.exit, label %for.body.preheader.i

for.body.preheader.i:                             ; preds = %if.end.i.i
  %call1.i = tail call i64 @strlen(ptr noundef nonnull dereferenceable(1) %retval1.0.i.i) #12
  %dec.i = add nsw i32 %equality.0.i, -1
  br label %for.body.i

for.body.i:                                       ; preds = %for.inc.i, %for.body.preheader.i
  %call218.i = phi i64 [ %call2.i, %for.inc.i ], [ 5, %for.body.preheader.i ]
  %equality.117.i = phi i32 [ %equality.2.i, %for.inc.i ], [ %dec.i, %for.body.preheader.i ]
  %p.016.i = phi ptr [ %add.ptr.i, %for.inc.i ], [ @.str.19, %for.body.preheader.i ]
  %cmp4.i = icmp eq i64 %call1.i, %call218.i
  br i1 %cmp4.i, label %land.lhs.true.i, label %for.inc.i

land.lhs.true.i:                                  ; preds = %for.body.i
  %bcmp.i = tail call i32 @bcmp(ptr nonnull %retval1.0.i.i, ptr %p.016.i, i64 %call1.i)
  %cmp6.i = icmp eq i32 %bcmp.i, 0
  %inc.i = zext i1 %cmp6.i to i32
  %spec.select.i = add nsw i32 %equality.117.i, %inc.i
  br label %for.inc.i

for.inc.i:                                        ; preds = %land.lhs.true.i, %for.body.i
  %equality.2.i = phi i32 [ %equality.117.i, %for.body.i ], [ %spec.select.i, %land.lhs.true.i ]
  %add.i = add i64 %call218.i, 1
  %add.ptr.i = getelementptr inbounds i8, ptr %p.016.i, i64 %add.i
  %call2.i = tail call i64 @strlen(ptr noundef nonnull dereferenceable(1) %add.ptr.i) #12
  %cmp3.not.i = icmp eq i64 %call2.i, 0
  br i1 %cmp3.not.i, label %while.cond.loopexit.i, label %for.body.i, !llvm.loop !10

ref_validmsgverb.exit:                            ; preds = %if.end.i.i
  store i8 0, ptr @ref_nextcomp.lmsgverb, align 16, !tbaa !5
  %tobool.not.i.not = icmp eq i32 %equality.0.i, 0
  br i1 %tobool.not.i.not, label %if.end23, label %if.then15

if.then15:                                        ; preds = %ref_validmsgverb.exit
  tail call void @free(ptr noundef %call8)
  br label %def

def:                                              ; preds = %if.then, %land.lhs.true, %land.lhs.true3, %if.then15
  %call18 = tail call noalias dereferenceable_or_null(31) ptr @strdup(ptr noundef nonnull @.str.1)
  %cmp19 = icmp eq ptr %call18, null
  br i1 %cmp19, label %cleanup, label %if.end23

if.end23:                                         ; preds = %def, %ref_validmsgverb.exit
  %msgverb.0 = phi ptr [ %call18, %def ], [ %call8, %ref_validmsgverb.exit ]
  %call24 = tail call fastcc ptr @ref_printfmt(ptr noundef nonnull %msgverb.0, ptr noundef %label, i32 noundef %sev, ptr noundef %text, ptr noundef %action, ptr noundef %tag)
  %cmp25 = icmp eq ptr %call24, null
  br i1 %cmp25, label %if.then27, label %if.end28

if.then27:                                        ; preds = %if.end23
  tail call void @free(ptr noundef %msgverb.0)
  br label %cleanup

if.end28:                                         ; preds = %if.end23
  %2 = load i8, ptr %call24, align 1, !tbaa !5
  %cmp30.not = icmp eq i8 %2, 0
  br i1 %cmp30.not, label %if.end34, label %if.then32

if.then32:                                        ; preds = %if.end28
  %3 = load ptr, ptr @__stderrp, align 8, !tbaa !11
  %fputs = tail call i32 @fputs(ptr nonnull %call24, ptr %3)
  br label %if.end34

if.end34:                                         ; preds = %if.then32, %if.end28
  tail call void @free(ptr noundef %msgverb.0)
  tail call void @free(ptr noundef nonnull %call24)
  br label %if.end35

if.end35:                                         ; preds = %if.end34, %entry
  %and36 = and i64 %class, 512
  %tobool37.not = icmp eq i64 %and36, 0
  br i1 %tobool37.not, label %cleanup, label %if.then38

if.then38:                                        ; preds = %if.end35
  %call39 = tail call fastcc ptr @ref_printfmt(ptr noundef nonnull @.str.1, ptr noundef %label, i32 noundef %sev, ptr noundef %text, ptr noundef %action, ptr noundef %tag)
  %cmp40 = icmp eq ptr %call39, null
  br i1 %cmp40, label %cleanup, label %if.end43

if.end43:                                         ; preds = %if.then38
  %4 = load i8, ptr %call39, align 1, !tbaa !5
  %cmp45.not = icmp eq i8 %4, 0
  br i1 %cmp45.not, label %if.end55, label %if.then47

if.then47:                                        ; preds = %if.end43
  %call48 = tail call ptr @fopen(ptr noundef nonnull @.str.3, ptr noundef nonnull @.str.4)
  %cmp49 = icmp eq ptr %call48, null
  br i1 %cmp49, label %if.then51, label %if.end52

if.then51:                                        ; preds = %if.then47
  tail call void @free(ptr noundef nonnull %call39)
  br label %cleanup

if.end52:                                         ; preds = %if.then47
  %fputs83 = tail call i32 @fputs(ptr nonnull %call39, ptr nonnull %call48)
  %call54 = tail call i32 @fclose(ptr noundef nonnull %call48)
  br label %if.end55

if.end55:                                         ; preds = %if.end52, %if.end43
  tail call void @free(ptr noundef nonnull %call39)
  br label %cleanup

cleanup:                                          ; preds = %if.end35, %if.end55, %if.then38, %def, %if.then7, %if.then51, %if.then27
  %retval.0 = phi i32 [ 3, %if.then27 ], [ 2, %if.then51 ], [ 3, %if.then7 ], [ 3, %def ], [ 2, %if.then38 ], [ 0, %if.end55 ], [ 0, %if.end35 ]
  ret i32 %retval.0
}

; Function Attrs: nofree nounwind memory(read)
declare noundef ptr @getenv(ptr nocapture noundef) local_unnamed_addr #1

; Function Attrs: mustprogress nofree nounwind willreturn memory(argmem: read)
declare i64 @strlen(ptr nocapture noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nofree nounwind willreturn memory(argmem: readwrite, inaccessiblemem: readwrite)
declare noalias ptr @strdup(ptr nocapture noundef readonly) local_unnamed_addr #3

; Function Attrs: mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite)
declare void @free(ptr allocptr nocapture noundef) local_unnamed_addr #4

; Function Attrs: nofree nounwind uwtable
define internal fastcc noundef ptr @ref_printfmt(ptr noundef %msgverb, ptr noundef %label, i32 noundef %sev, ptr noundef %text, ptr noundef %act, ptr noundef %tag) unnamed_addr #5 {
entry:
  %cmp = icmp ne ptr %label, null
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %call = tail call i64 @strlen(ptr noundef nonnull dereferenceable(1) %label) #12
  %add = add i64 %call, 32
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %size.0 = phi i64 [ %add, %if.then ], [ 32, %entry ]
  %switch.tableidx = add i32 %sev, -1
  %0 = icmp ult i32 %switch.tableidx, 4
  br i1 %0, label %switch.lookup, label %if.end6

switch.lookup:                                    ; preds = %if.end
  %1 = zext nneg i32 %switch.tableidx to i64
  %switch.gep = getelementptr inbounds [4 x ptr], ptr @switch.table.ref_printfmt.20, i64 0, i64 %1
  %switch.load = load ptr, ptr %switch.gep, align 8
  %call4 = tail call i64 @strlen(ptr noundef nonnull dereferenceable(1) %switch.load) #12
  %add5 = add i64 %call4, %size.0
  br label %if.end6

if.end6:                                          ; preds = %if.end, %switch.lookup
  %size.1 = phi i64 [ %add5, %switch.lookup ], [ %size.0, %if.end ]
  %cmp7 = icmp ne ptr %text, null
  br i1 %cmp7, label %if.then8, label %if.end11

if.then8:                                         ; preds = %if.end6
  %call9 = tail call i64 @strlen(ptr noundef nonnull dereferenceable(1) %text) #12
  %add10 = add i64 %call9, %size.1
  br label %if.end11

if.end11:                                         ; preds = %if.then8, %if.end6
  %size.2 = phi i64 [ %add10, %if.then8 ], [ %size.1, %if.end6 ]
  %cmp12 = icmp ne ptr %act, null
  br i1 %cmp12, label %if.then13, label %if.end16

if.then13:                                        ; preds = %if.end11
  %call14 = tail call i64 @strlen(ptr noundef nonnull dereferenceable(1) %act) #12
  %add15 = add i64 %call14, %size.2
  br label %if.end16

if.end16:                                         ; preds = %if.then13, %if.end11
  %size.3 = phi i64 [ %add15, %if.then13 ], [ %size.2, %if.end11 ]
  %cmp17 = icmp ne ptr %tag, null
  br i1 %cmp17, label %if.then18, label %if.end21

if.then18:                                        ; preds = %if.end16
  %call19 = tail call i64 @strlen(ptr noundef nonnull dereferenceable(1) %tag) #12
  %add20 = add i64 %call19, %size.3
  br label %if.end21

if.end21:                                         ; preds = %if.then18, %if.end16
  %size.4 = phi i64 [ %add20, %if.then18 ], [ %size.3, %if.end16 ]
  %call22 = tail call noalias ptr @malloc(i64 noundef %size.4) #14
  %cmp23 = icmp eq ptr %call22, null
  br i1 %cmp23, label %cleanup, label %if.end25

if.end25:                                         ; preds = %if.end21
  store i8 0, ptr %call22, align 1, !tbaa !5
  %switch.tableidx2 = add i32 %sev, -1
  %2 = icmp ult i32 %switch.tableidx2, 4
  %3 = zext nneg i32 %switch.tableidx2 to i64
  %switch.gep3 = getelementptr inbounds [4 x ptr], ptr @switch.table.ref_printfmt.20, i64 0, i64 %3
  br label %while.cond

while.cond:                                       ; preds = %while.cond.backedge, %if.end25
  %4 = load i8, ptr @ref_nextcomp.lmsgverb, align 16, !tbaa !5
  %cmp.i = icmp eq i8 %4, 0
  br i1 %cmp.i, label %if.then.i, label %if.else.i

if.then.i:                                        ; preds = %while.cond
  %call.i = tail call i64 @strlcpy(ptr noundef nonnull dereferenceable(1) @ref_nextcomp.lmsgverb, ptr noundef nonnull dereferenceable(1) %msgverb, i64 noundef 31) #13
  %call3.i = tail call ptr @strtok_r(ptr noundef nonnull @ref_nextcomp.lmsgverb, ptr noundef nonnull @.str.18, ptr noundef nonnull @ref_nextcomp.state)
  br label %if.end.i

if.else.i:                                        ; preds = %while.cond
  %call4.i = tail call ptr @strtok_r(ptr noundef null, ptr noundef nonnull @.str.18, ptr noundef nonnull @ref_nextcomp.state)
  br label %if.end.i

if.end.i:                                         ; preds = %if.else.i, %if.then.i
  %retval1.0.i = phi ptr [ %call3.i, %if.then.i ], [ %call4.i, %if.else.i ]
  %cmp5.i = icmp eq ptr %retval1.0.i, null
  br i1 %cmp5.i, label %while.end, label %while.body

while.body:                                       ; preds = %if.end.i
  %call28 = tail call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %retval1.0.i, ptr noundef nonnull dereferenceable(6) @.str.5) #12
  %cmp29 = icmp eq i32 %call28, 0
  %or.cond = and i1 %cmp, %cmp29
  br i1 %or.cond, label %if.then31, label %if.else

if.then31:                                        ; preds = %while.body
  %5 = load i8, ptr %call22, align 1, !tbaa !5
  %cmp32.not = icmp eq i8 %5, 0
  br i1 %cmp32.not, label %if.end36, label %if.then34

if.then34:                                        ; preds = %if.then31
  %call35 = tail call i64 @strlcat(ptr noundef nonnull %call22, ptr noundef nonnull @.str.6, i64 noundef %size.4) #13
  br label %if.end36

if.end36:                                         ; preds = %if.then34, %if.then31
  %call37 = tail call i64 @strlcat(ptr noundef nonnull %call22, ptr noundef nonnull %label, i64 noundef %size.4) #13
  br label %while.cond.backedge

if.else:                                          ; preds = %while.body
  %call38 = tail call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %retval1.0.i, ptr noundef nonnull dereferenceable(9) @.str.7) #12
  %cmp39 = icmp eq i32 %call38, 0
  %or.cond113 = and i1 %0, %cmp39
  br i1 %or.cond113, label %if.then44, label %if.else53

if.then44:                                        ; preds = %if.else
  %6 = load i8, ptr %call22, align 1, !tbaa !5
  %cmp46.not = icmp eq i8 %6, 0
  br i1 %cmp46.not, label %if.end50, label %if.then48

if.then48:                                        ; preds = %if.then44
  %call49 = tail call i64 @strlcat(ptr noundef nonnull %call22, ptr noundef nonnull @.str.6, i64 noundef %size.4) #13
  br label %if.end50

if.end50:                                         ; preds = %if.then48, %if.then44
  br i1 %2, label %switch.lookup1, label %ref_sevinfo.exit176

switch.lookup1:                                   ; preds = %if.end50
  %switch.load4 = load ptr, ptr %switch.gep3, align 8
  br label %ref_sevinfo.exit176

ref_sevinfo.exit176:                              ; preds = %if.end50, %switch.lookup1
  %retval.0.i172 = phi ptr [ %switch.load4, %switch.lookup1 ], [ null, %if.end50 ]
  %call52 = tail call i64 @strlcat(ptr noundef nonnull %call22, ptr noundef %retval.0.i172, i64 noundef %size.4) #13
  br label %while.cond.backedge

while.cond.backedge:                              ; preds = %ref_sevinfo.exit176, %if.end81, %if.end97, %if.else84, %if.end66, %if.end36
  br label %while.cond, !llvm.loop !13

if.else53:                                        ; preds = %if.else
  %call54 = tail call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %retval1.0.i, ptr noundef nonnull dereferenceable(5) @.str.8) #12
  %cmp55 = icmp eq i32 %call54, 0
  %or.cond114 = and i1 %cmp7, %cmp55
  br i1 %or.cond114, label %if.then60, label %if.else68

if.then60:                                        ; preds = %if.else53
  %7 = load i8, ptr %call22, align 1, !tbaa !5
  %cmp62.not = icmp eq i8 %7, 0
  br i1 %cmp62.not, label %if.end66, label %if.then64

if.then64:                                        ; preds = %if.then60
  %call65 = tail call i64 @strlcat(ptr noundef nonnull %call22, ptr noundef nonnull @.str.6, i64 noundef %size.4) #13
  br label %if.end66

if.end66:                                         ; preds = %if.then64, %if.then60
  %call67 = tail call i64 @strlcat(ptr noundef nonnull %call22, ptr noundef nonnull %text, i64 noundef %size.4) #13
  br label %while.cond.backedge

if.else68:                                        ; preds = %if.else53
  %call69 = tail call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %retval1.0.i, ptr noundef nonnull dereferenceable(7) @.str.9) #12
  %cmp70 = icmp eq i32 %call69, 0
  %or.cond115 = and i1 %cmp12, %cmp70
  br i1 %or.cond115, label %if.then75, label %if.else84

if.then75:                                        ; preds = %if.else68
  %8 = load i8, ptr %call22, align 1, !tbaa !5
  %cmp77.not = icmp eq i8 %8, 0
  br i1 %cmp77.not, label %if.end81, label %if.then79

if.then79:                                        ; preds = %if.then75
  %call80 = tail call i64 @strlcat(ptr noundef nonnull %call22, ptr noundef nonnull @.str.10, i64 noundef %size.4) #13
  br label %if.end81

if.end81:                                         ; preds = %if.then79, %if.then75
  %call82 = tail call i64 @strlcat(ptr noundef nonnull %call22, ptr noundef nonnull @.str.11, i64 noundef %size.4) #13
  %call83 = tail call i64 @strlcat(ptr noundef nonnull %call22, ptr noundef nonnull %act, i64 noundef %size.4) #13
  br label %while.cond.backedge

if.else84:                                        ; preds = %if.else68
  %call85 = tail call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %retval1.0.i, ptr noundef nonnull dereferenceable(4) @.str.12) #12
  %cmp86 = icmp eq i32 %call85, 0
  %or.cond116 = and i1 %cmp17, %cmp86
  br i1 %or.cond116, label %if.then91, label %while.cond.backedge

if.then91:                                        ; preds = %if.else84
  %9 = load i8, ptr %call22, align 1, !tbaa !5
  %cmp93.not = icmp eq i8 %9, 0
  br i1 %cmp93.not, label %if.end97, label %if.then95

if.then95:                                        ; preds = %if.then91
  %call96 = tail call i64 @strlcat(ptr noundef nonnull %call22, ptr noundef nonnull @.str.13, i64 noundef %size.4) #13
  br label %if.end97

if.end97:                                         ; preds = %if.then95, %if.then91
  %call98 = tail call i64 @strlcat(ptr noundef nonnull %call22, ptr noundef nonnull %tag, i64 noundef %size.4) #13
  br label %while.cond.backedge

while.end:                                        ; preds = %if.end.i
  store i8 0, ptr @ref_nextcomp.lmsgverb, align 16, !tbaa !5
  %10 = load i8, ptr %call22, align 1, !tbaa !5
  %cmp105.not = icmp eq i8 %10, 0
  br i1 %cmp105.not, label %cleanup, label %if.then107

if.then107:                                       ; preds = %while.end
  %call108 = tail call i64 @strlcat(ptr noundef nonnull %call22, ptr noundef nonnull @.str.10, i64 noundef %size.4) #13
  br label %cleanup

cleanup:                                          ; preds = %while.end, %if.then107, %if.end21
  ret ptr %call22
}

; Function Attrs: nofree nounwind
declare noalias noundef ptr @fopen(ptr nocapture noundef readonly, ptr nocapture noundef readonly) local_unnamed_addr #6

; Function Attrs: nofree nounwind
declare noundef i32 @fclose(ptr nocapture noundef) local_unnamed_addr #6

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #7

; Function Attrs: mustprogress nofree nounwind willreturn memory(argmem: read)
declare i32 @strcmp(ptr nocapture noundef, ptr nocapture noundef) local_unnamed_addr #2

; Function Attrs: nofree
declare i64 @strlcat(ptr noundef, ptr noundef, i64 noundef) local_unnamed_addr #8

; Function Attrs: nofree
declare i64 @strlcpy(ptr noundef, ptr noundef, i64 noundef) local_unnamed_addr #8

; Function Attrs: mustprogress nofree nounwind willreturn
declare ptr @strtok_r(ptr noundef, ptr nocapture noundef readonly, ptr noundef) local_unnamed_addr #9

; Function Attrs: nofree nounwind
declare noundef i32 @fputs(ptr nocapture noundef readonly, ptr nocapture noundef) local_unnamed_addr #10

; Function Attrs: nofree nounwind willreturn memory(argmem: read)
declare i32 @bcmp(ptr nocapture, ptr nocapture, i64) local_unnamed_addr #11

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nofree nounwind memory(read) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { mustprogress nofree nounwind willreturn memory(argmem: read) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { mustprogress nofree nounwind willreturn memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { nofree nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { nofree nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #7 = { mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #8 = { nofree "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #9 = { mustprogress nofree nounwind willreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #10 = { nofree nounwind }
attributes #11 = { nofree nounwind willreturn memory(argmem: read) }
attributes #12 = { nounwind willreturn memory(read) }
attributes #13 = { nounwind }
attributes #14 = { allocsize(0) }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!5 = !{!6, !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}
!10 = distinct !{!10, !9}
!11 = !{!12, !12, i64 0}
!12 = !{!"any pointer", !6, i64 0}
!13 = distinct !{!13, !9}
