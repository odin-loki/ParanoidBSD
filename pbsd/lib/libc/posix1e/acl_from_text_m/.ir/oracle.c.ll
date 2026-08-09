; ModuleID = '/home/odin/pbsd/pbsd/lib/libc/posix1e/acl_from_text_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/libc/posix1e/acl_from_text_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

module asm ".symver __qsort_r_compat, qsort_r@FBSD_1.0"

%struct.passwd = type { ptr, ptr, i32, i32, i64, ptr, ptr, ptr, ptr, i64, i32 }
%struct.group = type { ptr, ptr, i32, ptr }

@.str = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@.str.1 = private unnamed_addr constant [2 x i8] c"#\00", align 1
@.str.2 = private unnamed_addr constant [2 x i8] c",\00", align 1
@__func__.ref__text_is_nfs4_entry = private unnamed_addr constant [24 x i8] c"ref__text_is_nfs4_entry\00", align 1
@.str.3 = private unnamed_addr constant [63 x i8] c"/home/odin/pbsd/pbsd/lib/libc/posix1e/acl_from_text_m/oracle.c\00", align 1
@.str.4 = private unnamed_addr constant [18 x i8] c"strlen(entry) > 0\00", align 1
@__func__.ref__posix1e_acl_entry_from_text = private unnamed_addr constant [33 x i8] c"ref__posix1e_acl_entry_from_text\00", align 1
@.str.5 = private unnamed_addr constant [36 x i8] c"_acl_brand(aclp) == ACL_BRAND_POSIX\00", align 1
@.str.6 = private unnamed_addr constant [2 x i8] c":\00", align 1
@.str.7 = private unnamed_addr constant [5 x i8] c"user\00", align 1
@.str.8 = private unnamed_addr constant [2 x i8] c"u\00", align 1
@.str.9 = private unnamed_addr constant [6 x i8] c"group\00", align 1
@.str.10 = private unnamed_addr constant [2 x i8] c"g\00", align 1
@.str.11 = private unnamed_addr constant [5 x i8] c"mask\00", align 1
@.str.12 = private unnamed_addr constant [2 x i8] c"m\00", align 1
@.str.13 = private unnamed_addr constant [6 x i8] c"other\00", align 1
@.str.14 = private unnamed_addr constant [2 x i8] c"o\00", align 1

; Function Attrs: nounwind uwtable
define dso_local ptr @ref_acl_from_text(ptr nocapture noundef readonly %buf_p) local_unnamed_addr #0 {
entry:
  %entry.addr.i = alloca ptr, align 8
  %p.i = alloca i32, align 4
  %id.i = alloca i32, align 4
  %cur = alloca ptr, align 8
  %notcomment = alloca ptr, align 8
  %comment = alloca ptr, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %cur) #9
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %notcomment) #9
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %comment) #9
  %call = tail call noalias ptr @strdup(ptr noundef %buf_p)
  %cmp = icmp eq ptr %call, null
  br i1 %cmp, label %cleanup, label %if.end

if.end:                                           ; preds = %entry
  %call2 = tail call ptr @acl_init(i32 noundef 3) #9
  %cmp3 = icmp eq ptr %call2, null
  br i1 %cmp3, label %if.then4, label %if.end5

if.then4:                                         ; preds = %if.end
  tail call void @free(ptr noundef nonnull %call)
  br label %cleanup

if.end5:                                          ; preds = %if.end
  store ptr %call, ptr %cur, align 8, !tbaa !5
  %call665 = call ptr @strsep(ptr noundef nonnull %cur, ptr noundef nonnull @.str) #9
  %tobool.not66 = icmp eq ptr %call665, null
  br i1 %tobool.not66, label %while.end32, label %while.body

while.cond.loopexit:                              ; preds = %while.cond8.backedge, %while.body
  %call6 = call ptr @strsep(ptr noundef nonnull %cur, ptr noundef nonnull @.str) #9
  %tobool.not = icmp eq ptr %call6, null
  br i1 %tobool.not, label %while.end32, label %while.body, !llvm.loop !9

while.body:                                       ; preds = %if.end5, %while.cond.loopexit
  %call667 = phi ptr [ %call6, %while.cond.loopexit ], [ %call665, %if.end5 ]
  store ptr %call667, ptr %comment, align 8, !tbaa !5
  %call7 = call ptr @strsep(ptr noundef nonnull %comment, ptr noundef nonnull @.str.1) #9
  store ptr %call7, ptr %notcomment, align 8, !tbaa !5
  %call962 = call ptr @strsep(ptr noundef nonnull %notcomment, ptr noundef nonnull @.str.2) #9
  %tobool10.not63 = icmp eq ptr %call962, null
  br i1 %tobool10.not63, label %while.cond.loopexit, label %while.body11

while.body11:                                     ; preds = %while.body, %while.cond8.backedge
  %call964 = phi ptr [ %call9, %while.cond8.backedge ], [ %call962, %while.body ]
  %call12 = call ptr @string_skip_whitespace(ptr noundef nonnull %call964) #9
  %char0 = load i8, ptr %call12, align 1
  %cmp14 = icmp eq i8 %char0, 0
  br i1 %cmp14, label %while.cond8.backedge, label %if.end16

while.cond8.backedge:                             ; preds = %while.body11, %sw.epilog
  %call9 = call ptr @strsep(ptr noundef nonnull %notcomment, ptr noundef nonnull @.str.2) #9
  %tobool10.not = icmp eq ptr %call9, null
  br i1 %tobool10.not, label %while.cond.loopexit, label %while.body11, !llvm.loop !11

if.end16:                                         ; preds = %while.body11
  %call17 = call i32 @_acl_brand(ptr noundef nonnull %call2) #9
  %cmp18 = icmp eq i32 %call17, 0
  br i1 %cmp18, label %if.then19, label %if.end24

if.then19:                                        ; preds = %if.end16
  %char0.i = load i8, ptr %call964, align 1
  %cmp.not.i = icmp eq i8 %char0.i, 0
  br i1 %cmp.not.i, label %cond.false.i, label %while.cond.i

cond.false.i:                                     ; preds = %if.then19
  call void @__assert(ptr noundef nonnull @__func__.ref__text_is_nfs4_entry, ptr noundef nonnull @.str.3, i32 noundef 171, ptr noundef nonnull @.str.4) #10
  unreachable

while.cond.i:                                     ; preds = %if.then19, %if.end.i
  %0 = phi i8 [ %.pr.i, %if.end.i ], [ %char0.i, %if.then19 ]
  %entry.addr.0.i = phi ptr [ %incdec.ptr.i, %if.end.i ], [ %call964, %if.then19 ]
  %count.0.i = phi i32 [ %count.1.i, %if.end.i ], [ 0, %if.then19 ]
  switch i8 %0, label %if.end.i [
    i8 0, label %ref__text_is_nfs4_entry.exit
    i8 58, label %if.then.i
    i8 64, label %if.then.i
  ]

if.then.i:                                        ; preds = %while.cond.i, %while.cond.i
  %inc.i = add nsw i32 %count.0.i, 1
  br label %if.end.i

if.end.i:                                         ; preds = %if.then.i, %while.cond.i
  %count.1.i = phi i32 [ %inc.i, %if.then.i ], [ %count.0.i, %while.cond.i ]
  %incdec.ptr.i = getelementptr inbounds i8, ptr %entry.addr.0.i, i64 1
  %.pr.i = load i8, ptr %incdec.ptr.i, align 1, !tbaa !12
  br label %while.cond.i, !llvm.loop !13

ref__text_is_nfs4_entry.exit:                     ; preds = %while.cond.i
  %cmp10.i.inv = icmp sgt i32 %count.0.i, 2
  %. = select i1 %cmp10.i.inv, i32 2, i32 1
  call void @_acl_brand_as(ptr noundef nonnull %call2, i32 noundef %.) #9
  br label %if.end24

if.end24:                                         ; preds = %ref__text_is_nfs4_entry.exit, %if.end16
  %call25 = call i32 @_acl_brand(ptr noundef nonnull %call2) #9
  switch i32 %call25, label %error_label [
    i32 2, label %sw.bb
    i32 1, label %sw.bb27
  ]

sw.bb:                                            ; preds = %if.end24
  %call26 = call i32 @_nfs4_acl_entry_from_text(ptr noundef nonnull %call2, ptr noundef nonnull %call964) #9
  br label %sw.epilog

sw.bb27:                                          ; preds = %if.end24
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %entry.addr.i)
  store ptr %call964, ptr %entry.addr.i, align 8, !tbaa !5
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %p.i) #9
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %id.i) #9
  %call.i = call i32 @_acl_brand(ptr noundef nonnull %call2) #9
  %cmp.i = icmp eq i32 %call.i, 1
  br i1 %cmp.i, label %cond.end.i, label %cond.false.i56

cond.false.i56:                                   ; preds = %sw.bb27
  call void @__assert(ptr noundef nonnull @__func__.ref__posix1e_acl_entry_from_text, ptr noundef nonnull @.str.3, i32 noundef 89, ptr noundef nonnull @.str.5) #10
  unreachable

cond.end.i:                                       ; preds = %sw.bb27
  %call2.i = call ptr @strsep(ptr noundef nonnull %entry.addr.i, ptr noundef nonnull @.str.6) #9
  %cmp3.i = icmp eq ptr %call2.i, null
  br i1 %cmp3.i, label %if.then.i59, label %if.end.i57

if.then.i59:                                      ; preds = %cond.end.i
  %call4.i = call ptr @__error() #9
  store i32 22, ptr %call4.i, align 4, !tbaa !14
  br label %ref__posix1e_acl_entry_from_text.exit

if.end.i57:                                       ; preds = %cond.end.i
  %call5.i = call ptr @string_skip_whitespace(ptr noundef nonnull %call2.i) #9
  %1 = load i8, ptr %call5.i, align 1, !tbaa !12
  %cmp6.i = icmp ne i8 %1, 0
  %2 = load ptr, ptr %entry.addr.i, align 8
  %tobool.i = icmp ne ptr %2, null
  %or.cond.i = select i1 %cmp6.i, i1 true, i1 %tobool.i
  br i1 %or.cond.i, label %if.end9.i, label %ref__posix1e_acl_entry_from_text.exit

if.end9.i:                                        ; preds = %if.end.i57
  call void @string_trim_trailing_whitespace(ptr noundef nonnull %call5.i) #9
  %call10.i = call ptr @strsep(ptr noundef nonnull %entry.addr.i, ptr noundef nonnull @.str.6) #9
  %cmp11.i = icmp eq ptr %call10.i, null
  br i1 %cmp11.i, label %if.then13.i, label %if.end15.i

if.then13.i:                                      ; preds = %if.end9.i
  %call14.i = call ptr @__error() #9
  store i32 22, ptr %call14.i, align 4, !tbaa !14
  br label %ref__posix1e_acl_entry_from_text.exit

if.end15.i:                                       ; preds = %if.end9.i
  %call16.i = call ptr @string_skip_whitespace(ptr noundef nonnull %call10.i) #9
  call void @string_trim_trailing_whitespace(ptr noundef %call16.i) #9
  %call17.i = call ptr @strsep(ptr noundef nonnull %entry.addr.i, ptr noundef nonnull @.str.6) #9
  %cmp18.i = icmp eq ptr %call17.i, null
  %3 = load ptr, ptr %entry.addr.i, align 8
  %tobool20.i = icmp ne ptr %3, null
  %or.cond61.i = select i1 %cmp18.i, i1 true, i1 %tobool20.i
  br i1 %or.cond61.i, label %if.then21.i, label %if.end23.i

if.then21.i:                                      ; preds = %if.end15.i
  %call22.i = call ptr @__error() #9
  store i32 22, ptr %call22.i, align 4, !tbaa !14
  br label %ref__posix1e_acl_entry_from_text.exit

if.end23.i:                                       ; preds = %if.end15.i
  %call24.i = call ptr @string_skip_whitespace(ptr noundef nonnull %call17.i) #9
  call void @string_trim_trailing_whitespace(ptr noundef %call24.i) #9
  %4 = load i8, ptr %call16.i, align 1, !tbaa !12
  %cmp.i.i = icmp eq i8 %4, 0
  %call.i.i = call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %call5.i, ptr noundef nonnull dereferenceable(5) @.str.7) #11
  %tobool.not.i.i = icmp eq i32 %call.i.i, 0
  br i1 %cmp.i.i, label %if.then.i.i, label %if.else26.i.i

if.then.i.i:                                      ; preds = %if.end23.i
  br i1 %tobool.not.i.i, label %if.end30.i, label %lor.lhs.false.i.i

lor.lhs.false.i.i:                                ; preds = %if.then.i.i
  %call2.i.i = call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %call5.i, ptr noundef nonnull dereferenceable(2) @.str.8) #11
  %tobool3.not.i.i = icmp eq i32 %call2.i.i, 0
  br i1 %tobool3.not.i.i, label %if.end30.i, label %if.else.i.i

if.else.i.i:                                      ; preds = %lor.lhs.false.i.i
  %call5.i.i = call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %call5.i, ptr noundef nonnull dereferenceable(6) @.str.9) #11
  %tobool6.not.i.i = icmp eq i32 %call5.i.i, 0
  br i1 %tobool6.not.i.i, label %if.end30.i, label %lor.lhs.false7.i.i

lor.lhs.false7.i.i:                               ; preds = %if.else.i.i
  %call8.i.i = call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %call5.i, ptr noundef nonnull dereferenceable(2) @.str.10) #11
  %tobool9.not.i.i = icmp eq i32 %call8.i.i, 0
  br i1 %tobool9.not.i.i, label %if.end30.i, label %if.else11.i.i

if.else11.i.i:                                    ; preds = %lor.lhs.false7.i.i
  %call12.i.i = call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %call5.i, ptr noundef nonnull dereferenceable(5) @.str.11) #11
  %tobool13.not.i.i = icmp eq i32 %call12.i.i, 0
  br i1 %tobool13.not.i.i, label %if.end30.i, label %lor.lhs.false14.i.i

lor.lhs.false14.i.i:                              ; preds = %if.else11.i.i
  %call15.i.i = call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %call5.i, ptr noundef nonnull dereferenceable(2) @.str.12) #11
  %tobool16.not.i.i = icmp eq i32 %call15.i.i, 0
  br i1 %tobool16.not.i.i, label %if.end30.i, label %if.else18.i.i

if.else18.i.i:                                    ; preds = %lor.lhs.false14.i.i
  %call19.i.i = call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %call5.i, ptr noundef nonnull dereferenceable(6) @.str.13) #11
  %tobool20.not.i.i = icmp eq i32 %call19.i.i, 0
  br i1 %tobool20.not.i.i, label %if.end30.i, label %return.sink.split.i.i

if.else26.i.i:                                    ; preds = %if.end23.i
  br i1 %tobool.not.i.i, label %if.end30.i, label %lor.lhs.false29.i.i

lor.lhs.false29.i.i:                              ; preds = %if.else26.i.i
  %call30.i.i = call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %call5.i, ptr noundef nonnull dereferenceable(2) @.str.8) #11
  %tobool31.not.i.i = icmp eq i32 %call30.i.i, 0
  br i1 %tobool31.not.i.i, label %if.end30.i, label %if.else33.i.i

if.else33.i.i:                                    ; preds = %lor.lhs.false29.i.i
  %call34.i.i = call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %call5.i, ptr noundef nonnull dereferenceable(6) @.str.9) #11
  %tobool35.not.i.i = icmp eq i32 %call34.i.i, 0
  br i1 %tobool35.not.i.i, label %if.end30.i, label %return.sink.split.i.i

return.sink.split.i.i:                            ; preds = %if.else33.i.i, %if.else18.i.i
  %.str.10.sink.i.i = phi ptr [ @.str.14, %if.else18.i.i ], [ @.str.10, %if.else33.i.i ]
  %.sink.i.i = phi i32 [ 32, %if.else18.i.i ], [ 8, %if.else33.i.i ]
  %call37.i.i = call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %call5.i, ptr noundef nonnull dereferenceable(2) %.str.10.sink.i.i) #11
  %tobool38.not.i.i = icmp eq i32 %call37.i.i, 0
  br i1 %tobool38.not.i.i, label %if.end30.i, label %if.then28.i

if.then28.i:                                      ; preds = %return.sink.split.i.i
  %call29.i = call ptr @__error() #9
  store i32 22, ptr %call29.i, align 4, !tbaa !14
  br label %ref__posix1e_acl_entry_from_text.exit

if.end30.i:                                       ; preds = %return.sink.split.i.i, %if.else33.i.i, %lor.lhs.false29.i.i, %if.else26.i.i, %if.else18.i.i, %lor.lhs.false14.i.i, %if.else11.i.i, %lor.lhs.false7.i.i, %if.else.i.i, %lor.lhs.false.i.i, %if.then.i.i
  %retval.0.i.ph.i = phi i32 [ 8, %if.else33.i.i ], [ 2, %if.else26.i.i ], [ 2, %lor.lhs.false29.i.i ], [ 32, %if.else18.i.i ], [ 16, %if.else11.i.i ], [ 16, %lor.lhs.false14.i.i ], [ 4, %if.else.i.i ], [ 4, %lor.lhs.false7.i.i ], [ 1, %if.then.i.i ], [ 1, %lor.lhs.false.i.i ], [ %.sink.i.i, %return.sink.split.i.i ]
  %call31.i = call i32 @_posix1e_acl_string_to_perm(ptr noundef %call24.i, ptr noundef nonnull %p.i) #9
  %cmp32.i = icmp eq i32 %call31.i, -1
  br i1 %cmp32.i, label %if.then34.i, label %if.end36.i

if.then34.i:                                      ; preds = %if.end30.i
  %call35.i = call ptr @__error() #9
  store i32 22, ptr %call35.i, align 4, !tbaa !14
  br label %ref__posix1e_acl_entry_from_text.exit

if.end36.i:                                       ; preds = %if.end30.i
  switch i32 %retval.0.i.ph.i, label %sw.default.i [
    i32 1, label %sw.bb.i
    i32 4, label %sw.bb.i
    i32 16, label %sw.bb.i
    i32 32, label %sw.bb.i
    i32 2, label %sw.bb43.i
    i32 8, label %sw.bb43.i
  ]

sw.bb.i:                                          ; preds = %if.end36.i, %if.end36.i, %if.end36.i, %if.end36.i
  %5 = load i8, ptr %call16.i, align 1, !tbaa !12
  %cmp38.not.i = icmp eq i8 %5, 0
  br i1 %cmp38.not.i, label %sw.epilog.i, label %if.then40.i

if.then40.i:                                      ; preds = %sw.bb.i
  %call41.i = call ptr @__error() #9
  store i32 22, ptr %call41.i, align 4, !tbaa !14
  br label %ref__posix1e_acl_entry_from_text.exit

sw.bb43.i:                                        ; preds = %if.end36.i, %if.end36.i
  %call44.i = call i32 @ref__acl_name_to_id(i32 noundef %retval.0.i.ph.i, ptr noundef nonnull %call16.i, ptr noundef nonnull %id.i), !range !16
  %cmp45.i = icmp eq i32 %call44.i, -1
  br i1 %cmp45.i, label %ref__posix1e_acl_entry_from_text.exit, label %sw.bb43.sw.epilog_crit_edge.i

sw.bb43.sw.epilog_crit_edge.i:                    ; preds = %sw.bb43.i
  %.pre.i = load i32, ptr %id.i, align 4, !tbaa !14
  br label %sw.epilog.i

sw.default.i:                                     ; preds = %if.end36.i
  %call49.i = call ptr @__error() #9
  store i32 22, ptr %call49.i, align 4, !tbaa !14
  br label %ref__posix1e_acl_entry_from_text.exit

sw.epilog.i:                                      ; preds = %sw.bb43.sw.epilog_crit_edge.i, %sw.bb.i
  %6 = phi i32 [ %.pre.i, %sw.bb43.sw.epilog_crit_edge.i ], [ 0, %sw.bb.i ]
  %7 = load i32, ptr %p.i, align 4, !tbaa !14
  %call50.i = call i32 @_posix1e_acl_add_entry(ptr noundef nonnull %call2, i32 noundef %retval.0.i.ph.i, i32 noundef %6, i32 noundef %7) #9
  %cmp51.i = icmp eq i32 %call50.i, -1
  %..i58 = sext i1 %cmp51.i to i32
  br label %ref__posix1e_acl_entry_from_text.exit

ref__posix1e_acl_entry_from_text.exit:            ; preds = %if.then.i59, %if.end.i57, %if.then13.i, %if.then21.i, %if.then28.i, %if.then34.i, %if.then40.i, %sw.bb43.i, %sw.default.i, %sw.epilog.i
  %retval.0.i = phi i32 [ -1, %if.then.i59 ], [ -1, %if.then13.i ], [ -1, %if.then21.i ], [ -1, %if.then28.i ], [ -1, %if.then34.i ], [ -1, %sw.default.i ], [ -1, %if.then40.i ], [ 0, %if.end.i57 ], [ -1, %sw.bb43.i ], [ %..i58, %sw.epilog.i ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %id.i) #9
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %p.i) #9
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %entry.addr.i)
  br label %sw.epilog

sw.epilog:                                        ; preds = %ref__posix1e_acl_entry_from_text.exit, %sw.bb
  %error.0 = phi i32 [ %retval.0.i, %ref__posix1e_acl_entry_from_text.exit ], [ %call26, %sw.bb ]
  %tobool29.not = icmp eq i32 %error.0, 0
  br i1 %tobool29.not, label %while.cond8.backedge, label %error_label

while.end32:                                      ; preds = %while.cond.loopexit, %if.end5
  call void @free(ptr noundef %call)
  br label %cleanup

error_label:                                      ; preds = %if.end24, %sw.epilog
  %call33 = call i32 @acl_free(ptr noundef nonnull %call2) #9
  call void @free(ptr noundef %call)
  br label %cleanup

cleanup:                                          ; preds = %entry, %error_label, %while.end32, %if.then4
  %retval.0 = phi ptr [ null, %if.then4 ], [ null, %error_label ], [ %call2, %while.end32 ], [ null, %entry ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %comment) #9
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %notcomment) #9
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %cur) #9
  ret ptr %retval.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nofree nounwind willreturn memory(argmem: readwrite, inaccessiblemem: readwrite)
declare noalias ptr @strdup(ptr nocapture noundef readonly) local_unnamed_addr #2

declare ptr @acl_init(i32 noundef) local_unnamed_addr #3

; Function Attrs: mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite)
declare void @free(ptr allocptr nocapture noundef) local_unnamed_addr #4

declare ptr @strsep(ptr noundef, ptr noundef) local_unnamed_addr #3

declare ptr @string_skip_whitespace(ptr noundef) local_unnamed_addr #3

declare i32 @_acl_brand(ptr noundef) local_unnamed_addr #3

declare void @_acl_brand_as(ptr noundef, i32 noundef) local_unnamed_addr #3

declare i32 @_nfs4_acl_entry_from_text(ptr noundef, ptr noundef) local_unnamed_addr #3

declare i32 @acl_free(ptr noundef) local_unnamed_addr #3

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nounwind uwtable
define dso_local noundef i32 @ref__acl_name_to_id(i32 noundef %tag, ptr noundef %name, ptr nocapture noundef writeonly %id) local_unnamed_addr #0 {
entry:
  %endp = alloca ptr, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %endp) #9
  switch i32 %tag, label %cleanup [
    i32 2, label %sw.bb
    i32 8, label %sw.bb12
  ]

sw.bb:                                            ; preds = %entry
  %call = tail call ptr @getpwnam(ptr noundef %name)
  %cmp = icmp eq ptr %call, null
  br i1 %cmp, label %if.then, label %if.end11

if.then:                                          ; preds = %sw.bb
  %call1 = call i64 @strtoul(ptr noundef %name, ptr noundef nonnull %endp, i32 noundef 0)
  %0 = load ptr, ptr %endp, align 8, !tbaa !5
  %1 = load i8, ptr %0, align 1, !tbaa !12
  %cmp2.not = icmp eq i8 %1, 0
  %cmp6.not = icmp ult i64 %call1, 4294967296
  %or.cond = select i1 %cmp2.not, i1 %cmp6.not, i1 false
  br i1 %or.cond, label %if.end, label %if.then8

if.then8:                                         ; preds = %if.then
  %call9 = tail call ptr @__error() #9
  store i32 22, ptr %call9, align 4, !tbaa !14
  br label %cleanup

if.end:                                           ; preds = %if.then
  %conv4 = trunc i64 %call1 to i32
  store i32 %conv4, ptr %id, align 4, !tbaa !14
  br label %cleanup

if.end11:                                         ; preds = %sw.bb
  %pw_uid = getelementptr inbounds %struct.passwd, ptr %call, i64 0, i32 2
  %2 = load i32, ptr %pw_uid, align 8, !tbaa !17
  store i32 %2, ptr %id, align 4, !tbaa !14
  br label %cleanup

sw.bb12:                                          ; preds = %entry
  %call13 = tail call ptr @getgrnam(ptr noundef %name) #9
  %cmp14 = icmp eq ptr %call13, null
  br i1 %cmp14, label %if.then16, label %if.end30

if.then16:                                        ; preds = %sw.bb12
  %call17 = call i64 @strtoul(ptr noundef %name, ptr noundef nonnull %endp, i32 noundef 0)
  %3 = load ptr, ptr %endp, align 8, !tbaa !5
  %4 = load i8, ptr %3, align 1, !tbaa !12
  %cmp19.not = icmp eq i8 %4, 0
  %cmp24.not = icmp ult i64 %call17, 4294967296
  %or.cond47 = select i1 %cmp19.not, i1 %cmp24.not, i1 false
  br i1 %or.cond47, label %if.end28, label %if.then26

if.then26:                                        ; preds = %if.then16
  %call27 = tail call ptr @__error() #9
  store i32 22, ptr %call27, align 4, !tbaa !14
  br label %cleanup

if.end28:                                         ; preds = %if.then16
  %conv22 = trunc i64 %call17 to i32
  store i32 %conv22, ptr %id, align 4, !tbaa !14
  br label %cleanup

if.end30:                                         ; preds = %sw.bb12
  %gr_gid = getelementptr inbounds %struct.group, ptr %call13, i64 0, i32 2
  %5 = load i32, ptr %gr_gid, align 8, !tbaa !20
  store i32 %5, ptr %id, align 4, !tbaa !14
  br label %cleanup

cleanup:                                          ; preds = %entry, %if.end30, %if.end28, %if.then26, %if.end11, %if.end, %if.then8
  %retval.0 = phi i32 [ -1, %if.then26 ], [ 0, %if.end28 ], [ 0, %if.end30 ], [ -1, %if.then8 ], [ 0, %if.end ], [ 0, %if.end11 ], [ 22, %entry ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %endp) #9
  ret i32 %retval.0
}

; Function Attrs: nofree nounwind
declare noundef ptr @getpwnam(ptr nocapture noundef readonly) local_unnamed_addr #5

; Function Attrs: mustprogress nofree nounwind willreturn
declare i64 @strtoul(ptr noundef readonly, ptr nocapture noundef, i32 noundef) local_unnamed_addr #6

declare ptr @__error() local_unnamed_addr #3

declare ptr @getgrnam(ptr noundef) local_unnamed_addr #3

; Function Attrs: noreturn
declare void @__assert(ptr noundef, ptr noundef, i32 noundef, ptr noundef) local_unnamed_addr #7

declare void @string_trim_trailing_whitespace(ptr noundef) local_unnamed_addr #3

declare i32 @_posix1e_acl_string_to_perm(ptr noundef, ptr noundef) local_unnamed_addr #3

declare i32 @_posix1e_acl_add_entry(ptr noundef, i32 noundef, i32 noundef, i32 noundef) local_unnamed_addr #3

; Function Attrs: mustprogress nofree nounwind willreturn memory(argmem: read)
declare i32 @strcmp(ptr nocapture noundef, ptr nocapture noundef) local_unnamed_addr #8

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { mustprogress nofree nounwind willreturn memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { nofree nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { mustprogress nofree nounwind willreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #7 = { noreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #8 = { mustprogress nofree nounwind willreturn memory(argmem: read) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #9 = { nounwind }
attributes #10 = { noreturn nounwind }
attributes #11 = { nounwind willreturn memory(read) }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!5 = !{!6, !6, i64 0}
!6 = !{!"any pointer", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.mustprogress"}
!11 = distinct !{!11, !10}
!12 = !{!7, !7, i64 0}
!13 = distinct !{!13, !10}
!14 = !{!15, !15, i64 0}
!15 = !{!"int", !7, i64 0}
!16 = !{i32 -1, i32 23}
!17 = !{!18, !15, i64 16}
!18 = !{!"passwd", !6, i64 0, !6, i64 8, !15, i64 16, !15, i64 20, !19, i64 24, !6, i64 32, !6, i64 40, !6, i64 48, !6, i64 56, !19, i64 64, !15, i64 72}
!19 = !{!"long", !7, i64 0}
!20 = !{!21, !15, i64 16}
!21 = !{!"group", !6, i64 0, !6, i64 8, !15, i64 16, !6, i64 24}
