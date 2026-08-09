; ModuleID = '/home/odin/pbsd/pbsd/lib/libcrypt/crypt_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/libcrypt/crypt_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.crypt_format = type { ptr, ptr, ptr }
%struct.crypt_data = type { i32, [256 x i8] }

@crypt_format = internal unnamed_addr global ptr getelementptr inbounds ([5 x %struct.crypt_format], ptr @crypt_formats, i64 0, i64 3, i32 0), align 8
@crypt_formats = internal constant [5 x %struct.crypt_format] [%struct.crypt_format { ptr @.str, ptr @crypt_md5, ptr @.str.1 }, %struct.crypt_format { ptr @.str.2, ptr @crypt_nthash, ptr @.str.3 }, %struct.crypt_format { ptr @.str.4, ptr @crypt_sha256, ptr @.str.5 }, %struct.crypt_format { ptr @.str.6, ptr @crypt_sha512, ptr @.str.7 }, %struct.crypt_format zeroinitializer], align 16
@ref_crypt.data = internal global %struct.crypt_data zeroinitializer, align 4
@.str = private unnamed_addr constant [4 x i8] c"md5\00", align 1
@.str.1 = private unnamed_addr constant [4 x i8] c"$1$\00", align 1
@.str.2 = private unnamed_addr constant [4 x i8] c"nth\00", align 1
@.str.3 = private unnamed_addr constant [4 x i8] c"$3$\00", align 1
@.str.4 = private unnamed_addr constant [7 x i8] c"sha256\00", align 1
@.str.5 = private unnamed_addr constant [4 x i8] c"$5$\00", align 1
@.str.6 = private unnamed_addr constant [7 x i8] c"sha512\00", align 1
@.str.7 = private unnamed_addr constant [4 x i8] c"$6$\00", align 1

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(read, inaccessiblemem: none) uwtable
define dso_local ptr @ref_crypt_get_format() local_unnamed_addr #0 {
entry:
  %0 = load ptr, ptr @crypt_format, align 8, !tbaa !5
  %1 = load ptr, ptr %0, align 8, !tbaa !9
  ret ptr %1
}

; Function Attrs: mustprogress nofree nounwind willreturn memory(readwrite, argmem: read, inaccessiblemem: read) uwtable
define dso_local noundef i32 @ref_crypt_set_format(ptr nocapture noundef readonly %format) local_unnamed_addr #1 {
entry:
  %call = tail call i32 @strcasecmp(ptr noundef nonnull @.str, ptr noundef %format) #6
  %cmp2 = icmp eq i32 %call, 0
  br i1 %cmp2, label %if.then, label %for.inc

if.then:                                          ; preds = %for.inc.2, %for.inc.1, %for.inc, %entry
  %cf.07.lcssa = phi ptr [ @crypt_formats, %entry ], [ getelementptr inbounds ([5 x %struct.crypt_format], ptr @crypt_formats, i64 0, i64 1), %for.inc ], [ getelementptr inbounds ([5 x %struct.crypt_format], ptr @crypt_formats, i64 0, i64 2), %for.inc.1 ], [ getelementptr inbounds ([5 x %struct.crypt_format], ptr @crypt_formats, i64 0, i64 3), %for.inc.2 ]
  store ptr %cf.07.lcssa, ptr @crypt_format, align 8, !tbaa !5
  br label %cleanup

for.inc:                                          ; preds = %entry
  %call.1 = tail call i32 @strcasecmp(ptr noundef nonnull @.str.2, ptr noundef %format) #6
  %cmp2.1 = icmp eq i32 %call.1, 0
  br i1 %cmp2.1, label %if.then, label %for.inc.1

for.inc.1:                                        ; preds = %for.inc
  %call.2 = tail call i32 @strcasecmp(ptr noundef nonnull @.str.4, ptr noundef %format) #6
  %cmp2.2 = icmp eq i32 %call.2, 0
  br i1 %cmp2.2, label %if.then, label %for.inc.2

for.inc.2:                                        ; preds = %for.inc.1
  %call.3 = tail call i32 @strcasecmp(ptr noundef nonnull @.str.6, ptr noundef %format) #6
  %cmp2.3 = icmp eq i32 %call.3, 0
  br i1 %cmp2.3, label %if.then, label %cleanup

cleanup:                                          ; preds = %for.inc.2, %if.then
  %retval.0 = phi i32 [ 1, %if.then ], [ 0, %for.inc.2 ]
  ret i32 %retval.0
}

; Function Attrs: mustprogress nofree nounwind willreturn memory(read)
declare i32 @strcasecmp(ptr nocapture noundef, ptr nocapture noundef) local_unnamed_addr #2

; Function Attrs: nounwind uwtable
define dso_local ptr @ref_crypt_r(ptr noundef %passwd, ptr noundef %salt, ptr noundef %data) local_unnamed_addr #3 {
land.lhs.true:
  %strncmp = tail call i32 @strncmp(ptr noundef nonnull dereferenceable(1) %salt, ptr noundef nonnull dereferenceable(4) @.str.1, i64 3)
  %cmp = icmp eq i32 %strncmp, 0
  br i1 %cmp, label %match, label %land.lhs.true.1

land.lhs.true.1:                                  ; preds = %land.lhs.true
  %strncmp.1 = tail call i32 @strncmp(ptr noundef nonnull dereferenceable(1) %salt, ptr noundef nonnull dereferenceable(4) @.str.3, i64 3)
  %cmp.1 = icmp eq i32 %strncmp.1, 0
  br i1 %cmp.1, label %match, label %land.lhs.true.2

land.lhs.true.2:                                  ; preds = %land.lhs.true.1
  %strncmp.2 = tail call i32 @strncmp(ptr noundef nonnull dereferenceable(1) %salt, ptr noundef nonnull dereferenceable(4) @.str.5, i64 3)
  %cmp.2 = icmp eq i32 %strncmp.2, 0
  br i1 %cmp.2, label %match, label %land.lhs.true.3

land.lhs.true.3:                                  ; preds = %land.lhs.true.2
  %strncmp.3 = tail call i32 @strncmp(ptr noundef nonnull dereferenceable(1) %salt, ptr noundef nonnull dereferenceable(4) @.str.7, i64 3)
  %cmp.3 = icmp eq i32 %strncmp.3, 0
  br i1 %cmp.3, label %match, label %for.inc.3

for.inc.3:                                        ; preds = %land.lhs.true.3
  %0 = load ptr, ptr @crypt_format, align 8, !tbaa !5
  br label %match

match:                                            ; preds = %land.lhs.true, %land.lhs.true.1, %land.lhs.true.2, %land.lhs.true.3, %for.inc.3
  %cf.0.pn = phi ptr [ %0, %for.inc.3 ], [ @crypt_formats, %land.lhs.true ], [ getelementptr inbounds ([5 x %struct.crypt_format], ptr @crypt_formats, i64 0, i64 1), %land.lhs.true.1 ], [ getelementptr inbounds ([5 x %struct.crypt_format], ptr @crypt_formats, i64 0, i64 2), %land.lhs.true.2 ], [ getelementptr inbounds ([5 x %struct.crypt_format], ptr @crypt_formats, i64 0, i64 3), %land.lhs.true.3 ]
  %func.0.in = getelementptr inbounds %struct.crypt_format, ptr %cf.0.pn, i64 0, i32 1
  %func.0 = load ptr, ptr %func.0.in, align 8, !tbaa !11
  %__buf = getelementptr inbounds %struct.crypt_data, ptr %data, i64 0, i32 1
  %call6 = tail call i32 %func.0(ptr noundef %passwd, ptr noundef %salt, ptr noundef nonnull %__buf) #7
  %cmp7.not = icmp eq i32 %call6, 0
  %__buf. = select i1 %cmp7.not, ptr %__buf, ptr null
  ret ptr %__buf.
}

; Function Attrs: nounwind uwtable
define dso_local ptr @ref_crypt(ptr noundef %passwd, ptr noundef %salt) local_unnamed_addr #3 {
entry:
  %strncmp.i = tail call i32 @strncmp(ptr noundef nonnull dereferenceable(1) %salt, ptr noundef nonnull dereferenceable(4) @.str.1, i64 3)
  %cmp.i = icmp eq i32 %strncmp.i, 0
  br i1 %cmp.i, label %ref_crypt_r.exit, label %land.lhs.true.1.i

land.lhs.true.1.i:                                ; preds = %entry
  %strncmp.1.i = tail call i32 @strncmp(ptr noundef nonnull dereferenceable(1) %salt, ptr noundef nonnull dereferenceable(4) @.str.3, i64 3)
  %cmp.1.i = icmp eq i32 %strncmp.1.i, 0
  br i1 %cmp.1.i, label %ref_crypt_r.exit, label %land.lhs.true.2.i

land.lhs.true.2.i:                                ; preds = %land.lhs.true.1.i
  %strncmp.2.i = tail call i32 @strncmp(ptr noundef nonnull dereferenceable(1) %salt, ptr noundef nonnull dereferenceable(4) @.str.5, i64 3)
  %cmp.2.i = icmp eq i32 %strncmp.2.i, 0
  br i1 %cmp.2.i, label %ref_crypt_r.exit, label %land.lhs.true.3.i

land.lhs.true.3.i:                                ; preds = %land.lhs.true.2.i
  %strncmp.3.i = tail call i32 @strncmp(ptr noundef nonnull dereferenceable(1) %salt, ptr noundef nonnull dereferenceable(4) @.str.7, i64 3)
  %cmp.3.i = icmp eq i32 %strncmp.3.i, 0
  br i1 %cmp.3.i, label %ref_crypt_r.exit, label %for.inc.3.i

for.inc.3.i:                                      ; preds = %land.lhs.true.3.i
  %0 = load ptr, ptr @crypt_format, align 8, !tbaa !5
  br label %ref_crypt_r.exit

ref_crypt_r.exit:                                 ; preds = %entry, %land.lhs.true.1.i, %land.lhs.true.2.i, %land.lhs.true.3.i, %for.inc.3.i
  %cf.0.pn.i = phi ptr [ %0, %for.inc.3.i ], [ @crypt_formats, %entry ], [ getelementptr inbounds ([5 x %struct.crypt_format], ptr @crypt_formats, i64 0, i64 1), %land.lhs.true.1.i ], [ getelementptr inbounds ([5 x %struct.crypt_format], ptr @crypt_formats, i64 0, i64 2), %land.lhs.true.2.i ], [ getelementptr inbounds ([5 x %struct.crypt_format], ptr @crypt_formats, i64 0, i64 3), %land.lhs.true.3.i ]
  %func.0.in.i = getelementptr inbounds %struct.crypt_format, ptr %cf.0.pn.i, i64 0, i32 1
  %func.0.i = load ptr, ptr %func.0.in.i, align 8, !tbaa !11
  %call6.i = tail call i32 %func.0.i(ptr noundef %passwd, ptr noundef %salt, ptr noundef nonnull getelementptr inbounds (%struct.crypt_data, ptr @ref_crypt.data, i64 0, i32 1)) #7
  %cmp7.not.i = icmp eq i32 %call6.i, 0
  %__buf..i = select i1 %cmp7.not.i, ptr getelementptr inbounds (%struct.crypt_data, ptr @ref_crypt.data, i64 0, i32 1), ptr null
  ret ptr %__buf..i
}

declare i32 @crypt_md5(ptr noundef, ptr noundef, ptr noundef) #4

declare i32 @crypt_nthash(ptr noundef, ptr noundef, ptr noundef) #4

declare i32 @crypt_sha256(ptr noundef, ptr noundef, ptr noundef) #4

declare i32 @crypt_sha512(ptr noundef, ptr noundef, ptr noundef) #4

; Function Attrs: nofree nounwind willreturn memory(argmem: read)
declare i32 @strncmp(ptr nocapture, ptr nocapture, i64) local_unnamed_addr #5

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(read, inaccessiblemem: none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nofree nounwind willreturn memory(readwrite, argmem: read, inaccessiblemem: read) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { mustprogress nofree nounwind willreturn memory(read) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { nofree nounwind willreturn memory(argmem: read) }
attributes #6 = { nounwind willreturn memory(read) }
attributes #7 = { nounwind }

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
!9 = !{!10, !6, i64 0}
!10 = !{!"crypt_format", !6, i64 0, !6, i64 8, !6, i64 16}
!11 = !{!10, !6, i64 8}
