; ModuleID = '/home/odin/pbsd/pbsd/lib/libc/posix1e/acl_from_text_nfs4_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/libc/posix1e/acl_from_text_nfs4_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

module asm ".symver __qsort_r_compat, qsort_r@FBSD_1.0"

@__func__.ref__nfs4_acl_entry_from_text = private unnamed_addr constant [30 x i8] c"ref__nfs4_acl_entry_from_text\00", align 1
@.str = private unnamed_addr constant [68 x i8] c"/home/odin/pbsd/pbsd/lib/libc/posix1e/acl_from_text_nfs4_m/oracle.c\00", align 1
@.str.1 = private unnamed_addr constant [38 x i8] c"_entry_brand(entry) == ACL_BRAND_NFS4\00", align 1
@.str.2 = private unnamed_addr constant [2 x i8] c":\00", align 1
@.str.3 = private unnamed_addr constant [47 x i8] c"malformed ACL: unknown user or group name \22%s\22\00", align 1
@.str.4 = private unnamed_addr constant [23 x i8] c"need_qualifier != NULL\00", align 1
@.str.5 = private unnamed_addr constant [7 x i8] c"owner@\00", align 1
@.str.6 = private unnamed_addr constant [7 x i8] c"group@\00", align 1
@.str.7 = private unnamed_addr constant [10 x i8] c"everyone@\00", align 1
@.str.8 = private unnamed_addr constant [5 x i8] c"user\00", align 1
@.str.9 = private unnamed_addr constant [2 x i8] c"u\00", align 1
@.str.10 = private unnamed_addr constant [6 x i8] c"group\00", align 1
@.str.11 = private unnamed_addr constant [2 x i8] c"g\00", align 1
@.str.12 = private unnamed_addr constant [35 x i8] c"malformed ACL: invalid \22tag\22 field\00", align 1
@__func__.ref_parse_qualifier = private unnamed_addr constant [20 x i8] c"ref_parse_qualifier\00", align 1
@.str.13 = private unnamed_addr constant [39 x i8] c"malformed ACL: empty \22qualifier\22 field\00", align 1
@.str.14 = private unnamed_addr constant [6 x i8] c"allow\00", align 1
@.str.15 = private unnamed_addr constant [5 x i8] c"deny\00", align 1
@.str.16 = private unnamed_addr constant [6 x i8] c"audit\00", align 1
@.str.17 = private unnamed_addr constant [6 x i8] c"alarm\00", align 1
@.str.18 = private unnamed_addr constant [36 x i8] c"malformed ACL: invalid \22type\22 field\00", align 1
@.str.19 = private unnamed_addr constant [54 x i8] c"malformed ACL: \22appended id\22 field present, but empty\00", align 1
@.str.20 = private unnamed_addr constant [43 x i8] c"malformed ACL: appended id is not a number\00", align 1

; Function Attrs: nounwind uwtable
define dso_local i32 @ref__nfs4_acl_entry_from_text(ptr noundef %aclp, ptr noundef %str) local_unnamed_addr #0 {
entry:
  %aclp.addr = alloca ptr, align 8
  %str.addr = alloca ptr, align 8
  %need_qualifier = alloca i32, align 4
  %entry1 = alloca ptr, align 8
  store ptr %aclp, ptr %aclp.addr, align 8, !tbaa !5
  store ptr %str, ptr %str.addr, align 8, !tbaa !5
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %need_qualifier) #6
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %entry1) #6
  %call = call i32 @acl_create_entry(ptr noundef nonnull %aclp.addr, ptr noundef nonnull %entry1) #6
  %tobool.not = icmp eq i32 %call, 0
  br i1 %tobool.not, label %if.end, label %cleanup

if.end:                                           ; preds = %entry
  %0 = load ptr, ptr %entry1, align 8, !tbaa !5
  %call2 = call i32 @_entry_brand(ptr noundef %0) #6
  %cmp = icmp eq i32 %call2, 2
  br i1 %cmp, label %cond.end, label %cond.false

cond.false:                                       ; preds = %if.end
  call void @__assert(ptr noundef nonnull @__func__.ref__nfs4_acl_entry_from_text, ptr noundef nonnull @.str, i32 noundef 210, ptr noundef nonnull @.str.1) #7
  unreachable

cond.end:                                         ; preds = %if.end
  %1 = load ptr, ptr %str.addr, align 8, !tbaa !5
  %cmp3 = icmp eq ptr %1, null
  br i1 %cmp3, label %malformed_field, label %if.end5

if.end5:                                          ; preds = %cond.end
  %call6 = call ptr @strsep(ptr noundef nonnull %str.addr, ptr noundef nonnull @.str.2) #6
  %call7 = call ptr @string_skip_whitespace(ptr noundef %call6) #6
  %2 = load i8, ptr %call7, align 1, !tbaa !9
  %cmp8 = icmp ne i8 %2, 0
  %3 = load ptr, ptr %str.addr, align 8
  %tobool10 = icmp ne ptr %3, null
  %or.cond = select i1 %cmp8, i1 true, i1 %tobool10
  br i1 %or.cond, label %if.end12, label %cleanup

if.end12:                                         ; preds = %if.end5
  %4 = load ptr, ptr %entry1, align 8, !tbaa !5
  store i32 0, ptr %need_qualifier, align 4, !tbaa !10
  %call.i = call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %call7, ptr noundef nonnull dereferenceable(7) @.str.5) #8
  %cmp2.i = icmp eq i32 %call.i, 0
  br i1 %cmp2.i, label %ref_parse_tag.exit, label %if.end.i

if.end.i:                                         ; preds = %if.end12
  %call4.i = call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %call7, ptr noundef nonnull dereferenceable(7) @.str.6) #8
  %cmp5.i = icmp eq i32 %call4.i, 0
  br i1 %cmp5.i, label %ref_parse_tag.exit, label %if.end8.i

if.end8.i:                                        ; preds = %if.end.i
  %call9.i = call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %call7, ptr noundef nonnull dereferenceable(10) @.str.7) #8
  %cmp10.i = icmp eq i32 %call9.i, 0
  br i1 %cmp10.i, label %ref_parse_tag.exit, label %if.end13.i

if.end13.i:                                       ; preds = %if.end8.i
  store i32 1, ptr %need_qualifier, align 4, !tbaa !10
  %call14.i = call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %call7, ptr noundef nonnull dereferenceable(5) @.str.8) #8
  %cmp15.i = icmp eq i32 %call14.i, 0
  br i1 %cmp15.i, label %ref_parse_tag.exit, label %lor.lhs.false.i

lor.lhs.false.i:                                  ; preds = %if.end13.i
  %call16.i = call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %call7, ptr noundef nonnull dereferenceable(2) @.str.9) #8
  %cmp17.i = icmp eq i32 %call16.i, 0
  br i1 %cmp17.i, label %ref_parse_tag.exit, label %if.end20.i

if.end20.i:                                       ; preds = %lor.lhs.false.i
  %call21.i = call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %call7, ptr noundef nonnull dereferenceable(6) @.str.10) #8
  %cmp22.i = icmp eq i32 %call21.i, 0
  br i1 %cmp22.i, label %ref_parse_tag.exit, label %lor.lhs.false23.i

lor.lhs.false23.i:                                ; preds = %if.end20.i
  %call24.i = call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %call7, ptr noundef nonnull dereferenceable(2) @.str.11) #8
  %cmp25.i = icmp eq i32 %call24.i, 0
  br i1 %cmp25.i, label %ref_parse_tag.exit, label %ref_parse_tag.exit.thread

ref_parse_tag.exit.thread:                        ; preds = %lor.lhs.false23.i
  call void (ptr, ...) @warnx(ptr noundef nonnull @.str.12) #6
  br label %malformed_field

ref_parse_tag.exit:                               ; preds = %if.end20.i, %lor.lhs.false23.i, %if.end13.i, %lor.lhs.false.i, %if.end8.i, %if.end.i, %if.end12
  %.sink = phi i32 [ 1, %if.end12 ], [ 4, %if.end.i ], [ 64, %if.end8.i ], [ 2, %lor.lhs.false.i ], [ 2, %if.end13.i ], [ 8, %lor.lhs.false23.i ], [ 8, %if.end20.i ]
  %tobool17.not = phi i1 [ true, %if.end12 ], [ true, %if.end.i ], [ true, %if.end8.i ], [ false, %lor.lhs.false.i ], [ false, %if.end13.i ], [ false, %lor.lhs.false23.i ], [ false, %if.end20.i ]
  %call3.i = call i32 @acl_set_tag_type(ptr noundef %4, i32 noundef %.sink) #6
  %tobool14.not = icmp eq i32 %call3.i, 0
  br i1 %tobool14.not, label %if.end16, label %malformed_field

if.end16:                                         ; preds = %ref_parse_tag.exit
  %.old = load ptr, ptr %str.addr, align 8, !tbaa !5
  %cmp29.old = icmp eq ptr %.old, null
  br i1 %tobool17.not, label %if.end28, label %if.then18

if.then18:                                        ; preds = %if.end16
  br i1 %cmp29.old, label %malformed_field, label %if.end22

if.end22:                                         ; preds = %if.then18
  %call23 = call ptr @strsep(ptr noundef nonnull %str.addr, ptr noundef nonnull @.str.2) #6
  %5 = load ptr, ptr %entry1, align 8, !tbaa !5
  %call24 = call fastcc i32 @ref_parse_qualifier(ptr noundef %call23, ptr noundef %5, ptr noundef nonnull %need_qualifier)
  %tobool25 = icmp ne i32 %call24, 0
  %6 = load ptr, ptr %str.addr, align 8
  %cmp29 = icmp eq ptr %6, null
  %or.cond78 = select i1 %tobool25, i1 true, i1 %cmp29
  br i1 %or.cond78, label %malformed_field, label %if.end32

if.end28:                                         ; preds = %if.end16
  br i1 %cmp29.old, label %malformed_field, label %if.end32

if.end32:                                         ; preds = %if.end22, %if.end28
  %qualifier_field.0 = phi ptr [ %call23, %if.end22 ], [ undef, %if.end28 ]
  %call33 = call ptr @strsep(ptr noundef nonnull %str.addr, ptr noundef nonnull @.str.2) #6
  %7 = load ptr, ptr %entry1, align 8, !tbaa !5
  %call34 = call fastcc i32 @ref_parse_access_mask(ptr noundef %call33, ptr noundef %7)
  %tobool35 = icmp ne i32 %call34, 0
  %8 = load ptr, ptr %str.addr, align 8
  %cmp38 = icmp eq ptr %8, null
  %or.cond80 = select i1 %tobool35, i1 true, i1 %cmp38
  br i1 %or.cond80, label %malformed_field, label %while.cond.i

while.cond.i:                                     ; preds = %if.end32, %if.end.i98
  %str.addr.0.i = phi ptr [ %incdec.ptr.i, %if.end.i98 ], [ %8, %if.end32 ]
  %count.0.i = phi i32 [ %count.1.i, %if.end.i98 ], [ 0, %if.end32 ]
  %9 = load i8, ptr %str.addr.0.i, align 1, !tbaa !9
  switch i8 %9, label %if.end.i98 [
    i8 0, label %ref_number_of_colons.exit
    i8 58, label %if.then.i97
  ]

if.then.i97:                                      ; preds = %while.cond.i
  %inc.i = add nsw i32 %count.0.i, 1
  br label %if.end.i98

if.end.i98:                                       ; preds = %if.then.i97, %while.cond.i
  %count.1.i = phi i32 [ %inc.i, %if.then.i97 ], [ %count.0.i, %while.cond.i ]
  %incdec.ptr.i = getelementptr inbounds i8, ptr %str.addr.0.i, i64 1
  br label %while.cond.i, !llvm.loop !12

ref_number_of_colons.exit:                        ; preds = %while.cond.i
  %cmp43 = icmp sgt i32 %count.0.i, 0
  br i1 %cmp43, label %if.then45, label %if.end55

if.then45:                                        ; preds = %ref_number_of_colons.exit
  %call46 = call ptr @strsep(ptr noundef nonnull %str.addr, ptr noundef nonnull @.str.2) #6
  %10 = load ptr, ptr %entry1, align 8, !tbaa !5
  %call47 = call fastcc i32 @ref_parse_flags(ptr noundef %call46, ptr noundef %10)
  %tobool48 = icmp ne i32 %call47, 0
  %11 = load ptr, ptr %str.addr, align 8
  %cmp52 = icmp eq ptr %11, null
  %or.cond82 = select i1 %tobool48, i1 true, i1 %cmp52
  br i1 %or.cond82, label %malformed_field, label %if.end55

if.end55:                                         ; preds = %ref_number_of_colons.exit, %if.then45
  %call56 = call ptr @strsep(ptr noundef nonnull %str.addr, ptr noundef nonnull @.str.2) #6
  %12 = load ptr, ptr %entry1, align 8, !tbaa !5
  %call57 = call fastcc i32 @ref_parse_entry_type(ptr noundef %call56, ptr noundef %12)
  %tobool58.not = icmp eq i32 %call57, 0
  br i1 %tobool58.not, label %if.end60, label %malformed_field

if.end60:                                         ; preds = %if.end55
  %13 = load i32, ptr %need_qualifier, align 4, !tbaa !10
  %tobool61.not = icmp eq i32 %13, 0
  br i1 %tobool61.not, label %cleanup, label %if.then62

if.then62:                                        ; preds = %if.end60
  %14 = load ptr, ptr %str.addr, align 8, !tbaa !5
  %cmp63 = icmp eq ptr %14, null
  br i1 %cmp63, label %if.then65, label %if.end66

if.then65:                                        ; preds = %if.then62
  call void (ptr, ...) @warnx(ptr noundef nonnull @.str.3, ptr noundef %qualifier_field.0) #6
  br label %malformed_field

if.end66:                                         ; preds = %if.then62
  %15 = load ptr, ptr %entry1, align 8, !tbaa !5
  %call67 = call fastcc i32 @ref_parse_appended_id(ptr noundef nonnull %14, ptr noundef %15)
  %tobool68.not = icmp eq i32 %call67, 0
  br i1 %tobool68.not, label %cleanup, label %malformed_field

malformed_field:                                  ; preds = %ref_parse_tag.exit.thread, %if.then65, %cond.end, %if.then18, %if.end28, %if.end66, %if.end55, %if.then45, %if.end32, %if.end22, %ref_parse_tag.exit
  %16 = load ptr, ptr %aclp.addr, align 8, !tbaa !5
  %17 = load ptr, ptr %entry1, align 8, !tbaa !5
  %call72 = call i32 @acl_delete_entry(ptr noundef %16, ptr noundef %17) #6
  %call73 = call ptr @__error() #6
  store i32 22, ptr %call73, align 4, !tbaa !10
  br label %cleanup

cleanup:                                          ; preds = %if.end60, %if.end66, %if.end5, %entry, %malformed_field
  %retval.0 = phi i32 [ -1, %malformed_field ], [ %call, %entry ], [ 0, %if.end5 ], [ 0, %if.end66 ], [ 0, %if.end60 ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %entry1) #6
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %need_qualifier) #6
  ret i32 %retval.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

declare i32 @acl_create_entry(ptr noundef, ptr noundef) local_unnamed_addr #2

declare i32 @_entry_brand(ptr noundef) local_unnamed_addr #2

; Function Attrs: noreturn
declare void @__assert(ptr noundef, ptr noundef, i32 noundef, ptr noundef) local_unnamed_addr #3

declare ptr @strsep(ptr noundef, ptr noundef) local_unnamed_addr #2

declare ptr @string_skip_whitespace(ptr noundef) local_unnamed_addr #2

; Function Attrs: nounwind uwtable
define internal fastcc i32 @ref_parse_qualifier(ptr noundef %str, ptr noundef %entry1, ptr noundef writeonly %need_qualifier) unnamed_addr #0 {
entry:
  %id = alloca i32, align 4
  %tag = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %id) #6
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %tag) #6
  %cmp.not = icmp eq ptr %need_qualifier, null
  br i1 %cmp.not, label %cond.false, label %cond.end

cond.false:                                       ; preds = %entry
  tail call void @__assert(ptr noundef nonnull @__func__.ref_parse_qualifier, ptr noundef nonnull @.str, i32 noundef 90, ptr noundef nonnull @.str.4) #7
  unreachable

cond.end:                                         ; preds = %entry
  store i32 0, ptr %need_qualifier, align 4, !tbaa !10
  %call = tail call i64 @strlen(ptr noundef nonnull dereferenceable(1) %str) #8
  %0 = and i64 %call, 4294967295
  %cmp2 = icmp eq i64 %0, 0
  br i1 %cmp2, label %if.then, label %if.end

if.then:                                          ; preds = %cond.end
  tail call void (ptr, ...) @warnx(ptr noundef nonnull @.str.13) #6
  br label %cleanup

if.end:                                           ; preds = %cond.end
  %call4 = call i32 @acl_get_tag_type(ptr noundef %entry1, ptr noundef nonnull %tag) #6
  %tobool.not = icmp eq i32 %call4, 0
  br i1 %tobool.not, label %if.end6, label %cleanup

if.end6:                                          ; preds = %if.end
  %1 = load i32, ptr %tag, align 4, !tbaa !10
  %call7 = call i32 @_acl_name_to_id(i32 noundef %1, ptr noundef %str, ptr noundef nonnull %id) #6
  %tobool8.not = icmp eq i32 %call7, 0
  br i1 %tobool8.not, label %if.end10, label %if.then9

if.then9:                                         ; preds = %if.end6
  store i32 1, ptr %need_qualifier, align 4, !tbaa !10
  br label %cleanup

if.end10:                                         ; preds = %if.end6
  %call11 = call i32 @acl_set_qualifier(ptr noundef %entry1, ptr noundef nonnull %id) #6
  br label %cleanup

cleanup:                                          ; preds = %if.end, %if.end10, %if.then9, %if.then
  %retval.0 = phi i32 [ -1, %if.then ], [ 0, %if.then9 ], [ %call11, %if.end10 ], [ %call4, %if.end ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %tag) #6
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %id) #6
  ret i32 %retval.0
}

; Function Attrs: nounwind uwtable
define internal fastcc i32 @ref_parse_access_mask(ptr noundef %str, ptr noundef %entry1) unnamed_addr #0 {
entry:
  %perm = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %perm) #6
  %call = call i32 @_nfs4_parse_access_mask(ptr noundef %str, ptr noundef nonnull %perm) #6
  %tobool.not = icmp eq i32 %call, 0
  br i1 %tobool.not, label %if.end, label %cleanup

if.end:                                           ; preds = %entry
  %call2 = call i32 @acl_set_permset(ptr noundef %entry1, ptr noundef nonnull %perm) #6
  br label %cleanup

cleanup:                                          ; preds = %entry, %if.end
  %retval.0 = phi i32 [ %call2, %if.end ], [ %call, %entry ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %perm) #6
  ret i32 %retval.0
}

; Function Attrs: nounwind uwtable
define internal fastcc i32 @ref_parse_flags(ptr noundef %str, ptr noundef %entry1) unnamed_addr #0 {
entry:
  %flags = alloca i16, align 2
  call void @llvm.lifetime.start.p0(i64 2, ptr nonnull %flags) #6
  %call = call i32 @_nfs4_parse_flags(ptr noundef %str, ptr noundef nonnull %flags) #6
  %tobool.not = icmp eq i32 %call, 0
  br i1 %tobool.not, label %if.end, label %cleanup

if.end:                                           ; preds = %entry
  %call2 = call i32 @acl_set_flagset_np(ptr noundef %entry1, ptr noundef nonnull %flags) #6
  br label %cleanup

cleanup:                                          ; preds = %entry, %if.end
  %retval.0 = phi i32 [ %call2, %if.end ], [ %call, %entry ]
  call void @llvm.lifetime.end.p0(i64 2, ptr nonnull %flags) #6
  ret i32 %retval.0
}

; Function Attrs: nounwind uwtable
define internal fastcc i32 @ref_parse_entry_type(ptr nocapture noundef readonly %str, ptr noundef %entry1) unnamed_addr #0 {
entry:
  %call = tail call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %str, ptr noundef nonnull dereferenceable(6) @.str.14) #8
  %cmp = icmp eq i32 %call, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %call2 = tail call i32 @acl_set_entry_type_np(ptr noundef %entry1, i16 noundef zeroext 256) #6
  br label %return

if.end:                                           ; preds = %entry
  %call3 = tail call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %str, ptr noundef nonnull dereferenceable(5) @.str.15) #8
  %cmp4 = icmp eq i32 %call3, 0
  br i1 %cmp4, label %if.then5, label %if.end7

if.then5:                                         ; preds = %if.end
  %call6 = tail call i32 @acl_set_entry_type_np(ptr noundef %entry1, i16 noundef zeroext 512) #6
  br label %return

if.end7:                                          ; preds = %if.end
  %call8 = tail call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %str, ptr noundef nonnull dereferenceable(6) @.str.16) #8
  %cmp9 = icmp eq i32 %call8, 0
  br i1 %cmp9, label %if.then10, label %if.end12

if.then10:                                        ; preds = %if.end7
  %call11 = tail call i32 @acl_set_entry_type_np(ptr noundef %entry1, i16 noundef zeroext 1024) #6
  br label %return

if.end12:                                         ; preds = %if.end7
  %call13 = tail call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %str, ptr noundef nonnull dereferenceable(6) @.str.17) #8
  %cmp14 = icmp eq i32 %call13, 0
  br i1 %cmp14, label %if.then15, label %if.end17

if.then15:                                        ; preds = %if.end12
  %call16 = tail call i32 @acl_set_entry_type_np(ptr noundef %entry1, i16 noundef zeroext 2048) #6
  br label %return

if.end17:                                         ; preds = %if.end12
  tail call void (ptr, ...) @warnx(ptr noundef nonnull @.str.18) #6
  br label %return

return:                                           ; preds = %if.end17, %if.then15, %if.then10, %if.then5, %if.then
  %retval.0 = phi i32 [ %call2, %if.then ], [ %call6, %if.then5 ], [ %call11, %if.then10 ], [ %call16, %if.then15 ], [ -1, %if.end17 ]
  ret i32 %retval.0
}

declare void @warnx(ptr noundef, ...) local_unnamed_addr #2

; Function Attrs: nounwind uwtable
define internal fastcc i32 @ref_parse_appended_id(ptr noundef %str, ptr noundef %entry1) unnamed_addr #0 {
entry:
  %end = alloca ptr, align 8
  %id = alloca i64, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %end) #6
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %id) #6
  %call = tail call i64 @strlen(ptr noundef nonnull dereferenceable(1) %str) #8
  %0 = and i64 %call, 4294967295
  %cmp = icmp eq i64 %0, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  tail call void (ptr, ...) @warnx(ptr noundef nonnull @.str.19) #6
  br label %cleanup

if.end:                                           ; preds = %entry
  %call3 = call double @strtod(ptr noundef %str, ptr noundef nonnull %end)
  %conv4 = fptosi double %call3 to i64
  store i64 %conv4, ptr %id, align 8, !tbaa !14
  %1 = load ptr, ptr %end, align 8, !tbaa !5
  %sub.ptr.lhs.cast = ptrtoint ptr %1 to i64
  %sub.ptr.rhs.cast = ptrtoint ptr %str to i64
  %sub.ptr.sub = sub i64 %sub.ptr.lhs.cast, %sub.ptr.rhs.cast
  %sext = shl i64 %call, 32
  %conv5 = ashr exact i64 %sext, 32
  %cmp6.not = icmp eq i64 %sub.ptr.sub, %conv5
  br i1 %cmp6.not, label %if.end9, label %if.then8

if.then8:                                         ; preds = %if.end
  tail call void (ptr, ...) @warnx(ptr noundef nonnull @.str.20) #6
  br label %cleanup

if.end9:                                          ; preds = %if.end
  %call10 = call i32 @acl_set_qualifier(ptr noundef %entry1, ptr noundef nonnull %id) #6
  br label %cleanup

cleanup:                                          ; preds = %if.end9, %if.then8, %if.then
  %retval.0 = phi i32 [ -1, %if.then ], [ -1, %if.then8 ], [ %call10, %if.end9 ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %id) #6
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %end) #6
  ret i32 %retval.0
}

declare i32 @acl_delete_entry(ptr noundef, ptr noundef) local_unnamed_addr #2

declare ptr @__error() local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nofree nounwind willreturn memory(argmem: read)
declare i32 @strcmp(ptr nocapture noundef, ptr nocapture noundef) local_unnamed_addr #4

declare i32 @acl_set_tag_type(ptr noundef, i32 noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nofree nounwind willreturn memory(argmem: read)
declare i64 @strlen(ptr nocapture noundef) local_unnamed_addr #4

declare i32 @acl_get_tag_type(ptr noundef, ptr noundef) local_unnamed_addr #2

declare i32 @_acl_name_to_id(i32 noundef, ptr noundef, ptr noundef) local_unnamed_addr #2

declare i32 @acl_set_qualifier(ptr noundef, ptr noundef) local_unnamed_addr #2

declare i32 @_nfs4_parse_access_mask(ptr noundef, ptr noundef) local_unnamed_addr #2

declare i32 @acl_set_permset(ptr noundef, ptr noundef) local_unnamed_addr #2

declare i32 @_nfs4_parse_flags(ptr noundef, ptr noundef) local_unnamed_addr #2

declare i32 @acl_set_flagset_np(ptr noundef, ptr noundef) local_unnamed_addr #2

declare i32 @acl_set_entry_type_np(ptr noundef, i16 noundef zeroext) local_unnamed_addr #2

; Function Attrs: mustprogress nofree nounwind willreturn
declare double @strtod(ptr noundef readonly, ptr nocapture noundef) local_unnamed_addr #5

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { noreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { mustprogress nofree nounwind willreturn memory(argmem: read) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { mustprogress nofree nounwind willreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { nounwind }
attributes #7 = { noreturn nounwind }
attributes #8 = { nounwind willreturn memory(read) }

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
!9 = !{!7, !7, i64 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"int", !7, i64 0}
!12 = distinct !{!12, !13}
!13 = !{!"llvm.loop.mustprogress"}
!14 = !{!15, !15, i64 0}
!15 = !{!"long", !7, i64 0}
