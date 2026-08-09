; ModuleID = '/home/odin/pbsd/pbsd/lib/libc/gen/sysctl_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/libc/gen/sysctl_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@.str = private unnamed_addr constant [11 x i8] c"/usr/local\00", align 1
@.str.1 = private unnamed_addr constant [30 x i8] c"/usr/bin:/bin:/usr/sbin:/sbin\00", align 1

; Function Attrs: nounwind uwtable
define dso_local i32 @ref_sysctl(ptr noundef %name, i32 noundef %namelen, ptr noundef %oldp, ptr noundef %oldlenp, ptr noundef %newp, i64 noundef %newlen) local_unnamed_addr #0 {
entry:
  %cmp.not = icmp eq ptr %oldlenp, null
  br i1 %cmp.not, label %cond.end, label %cond.true

cond.true:                                        ; preds = %entry
  %0 = load i64, ptr %oldlenp, align 8, !tbaa !5
  br label %cond.end

cond.end:                                         ; preds = %entry, %cond.true
  %cond = phi i64 [ %0, %cond.true ], [ 0, %entry ]
  %call = tail call i32 @__sysctl(ptr noundef %name, i32 noundef %namelen, ptr noundef %oldp, ptr noundef %oldlenp, ptr noundef %newp, i64 noundef %newlen) #3
  %cmp2.not = icmp eq i32 %call, 0
  br i1 %cmp2.not, label %lor.lhs.false, label %cleanup

lor.lhs.false:                                    ; preds = %cond.end
  %1 = load i32, ptr %name, align 4, !tbaa !9
  %cmp3.not = icmp eq i32 %1, 8
  br i1 %cmp3.not, label %if.end, label %cleanup

if.end:                                           ; preds = %lor.lhs.false
  %cmp4.not = icmp eq i32 %namelen, 2
  br i1 %cmp4.not, label %if.end7, label %if.then5

if.then5:                                         ; preds = %if.end
  %call6 = tail call ptr @__error() #3
  store i32 22, ptr %call6, align 4, !tbaa !9
  br label %cleanup

if.end7:                                          ; preds = %if.end
  %arrayidx8 = getelementptr inbounds i32, ptr %name, i64 1
  %2 = load i32, ptr %arrayidx8, align 4, !tbaa !9
  %cond53 = icmp eq i32 %2, 21
  br i1 %cond53, label %sw.bb, label %sw.epilog

sw.bb:                                            ; preds = %if.end7
  br i1 %cmp.not, label %cleanup, label %lor.lhs.false10

lor.lhs.false10:                                  ; preds = %sw.bb
  %3 = load i64, ptr %oldlenp, align 8, !tbaa !5
  %cmp11 = icmp ugt i64 %3, 1
  br i1 %cmp11, label %cleanup, label %if.end13

if.end13:                                         ; preds = %lor.lhs.false10
  %cmp.not.i = icmp eq ptr %oldp, null
  br i1 %cmp.not.i, label %ref_set_user_str.exit, label %if.then.i

if.then.i:                                        ; preds = %if.end13
  %cmp2.i = icmp ult i64 %cond, 11
  br i1 %cmp2.i, label %if.then3.i, label %if.end.i

if.then3.i:                                       ; preds = %if.then.i
  %call.i = tail call ptr @__error() #3
  store i32 12, ptr %call.i, align 4, !tbaa !9
  br label %if.end.i

if.end.i:                                         ; preds = %if.then3.i, %if.then.i
  %len.addr.0.i = phi i64 [ %cond, %if.then3.i ], [ 11, %if.then.i ]
  %retval1.0.i = phi i32 [ -1, %if.then3.i ], [ 0, %if.then.i ]
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 1 %oldp, ptr nonnull align 1 @.str, i64 %len.addr.0.i, i1 false)
  br label %ref_set_user_str.exit

ref_set_user_str.exit:                            ; preds = %if.end13, %if.end.i
  %len.addr.1.i = phi i64 [ %len.addr.0.i, %if.end.i ], [ 11, %if.end13 ]
  %retval1.1.i = phi i32 [ %retval1.0.i, %if.end.i ], [ 0, %if.end13 ]
  store i64 %len.addr.1.i, ptr %oldlenp, align 8, !tbaa !5
  br label %cleanup

sw.epilog:                                        ; preds = %if.end7
  %cmp15.not = icmp eq ptr %newp, null
  br i1 %cmp15.not, label %if.end18, label %if.then16

if.then16:                                        ; preds = %sw.epilog
  %call17 = tail call ptr @__error() #3
  store i32 1, ptr %call17, align 4, !tbaa !9
  br label %cleanup

if.end18:                                         ; preds = %sw.epilog
  %cond54 = icmp eq i32 %2, 1
  %cmp.not.i94 = icmp eq ptr %oldp, null
  br i1 %cond54, label %sw.bb20, label %sw.epilog22

sw.bb20:                                          ; preds = %if.end18
  br i1 %cmp.not.i94, label %ref_set_user_str.exit104, label %if.then.i95

if.then.i95:                                      ; preds = %sw.bb20
  %cmp2.i96 = icmp ult i64 %cond, 30
  br i1 %cmp2.i96, label %if.then3.i102, label %if.end.i97

if.then3.i102:                                    ; preds = %if.then.i95
  %call.i103 = tail call ptr @__error() #3
  store i32 12, ptr %call.i103, align 4, !tbaa !9
  br label %if.end.i97

if.end.i97:                                       ; preds = %if.then3.i102, %if.then.i95
  %len.addr.0.i98 = phi i64 [ %cond, %if.then3.i102 ], [ 30, %if.then.i95 ]
  %retval1.0.i99 = phi i32 [ -1, %if.then3.i102 ], [ 0, %if.then.i95 ]
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 1 %oldp, ptr nonnull align 1 @.str.1, i64 %len.addr.0.i98, i1 false)
  br label %ref_set_user_str.exit104

ref_set_user_str.exit104:                         ; preds = %sw.bb20, %if.end.i97
  %len.addr.1.i100 = phi i64 [ %len.addr.0.i98, %if.end.i97 ], [ 30, %sw.bb20 ]
  %retval1.1.i101 = phi i32 [ %retval1.0.i99, %if.end.i97 ], [ 0, %sw.bb20 ]
  store i64 %len.addr.1.i100, ptr %oldlenp, align 8, !tbaa !5
  br label %cleanup

sw.epilog22:                                      ; preds = %if.end18
  br i1 %cmp.not.i94, label %if.end27.thread, label %land.lhs.true

if.end27.thread:                                  ; preds = %sw.epilog22
  store i64 4, ptr %oldlenp, align 8, !tbaa !5
  br label %cleanup

land.lhs.true:                                    ; preds = %sw.epilog22
  %4 = load i64, ptr %oldlenp, align 8, !tbaa !5
  %cmp24 = icmp ult i64 %4, 4
  br i1 %cmp24, label %if.then25, label %if.end30

if.then25:                                        ; preds = %land.lhs.true
  %call26 = tail call ptr @__error() #3
  store i32 12, ptr %call26, align 4, !tbaa !9
  br label %cleanup

if.end30:                                         ; preds = %land.lhs.true
  store i64 4, ptr %oldlenp, align 8, !tbaa !5
  switch i32 %2, label %sw.default [
    i32 2, label %sw.bb32
    i32 3, label %sw.bb33
    i32 4, label %sw.bb34
    i32 5, label %sw.bb35
    i32 6, label %sw.bb36
    i32 7, label %sw.bb37
    i32 8, label %sw.bb38
    i32 9, label %sw.bb39
    i32 10, label %sw.bb40
    i32 11, label %sw.bb41
    i32 12, label %sw.bb42
    i32 13, label %sw.bb43
    i32 14, label %sw.bb44
    i32 15, label %sw.bb45
    i32 16, label %sw.bb46
    i32 17, label %sw.bb47
    i32 18, label %sw.bb48
    i32 19, label %sw.bb49
    i32 20, label %sw.bb50
  ]

sw.bb32:                                          ; preds = %if.end30
  store i32 99, ptr %oldp, align 4, !tbaa !9
  br label %cleanup

sw.bb33:                                          ; preds = %if.end30
  store i32 2048, ptr %oldp, align 4, !tbaa !9
  br label %cleanup

sw.bb34:                                          ; preds = %if.end30
  store i32 99, ptr %oldp, align 4, !tbaa !9
  br label %cleanup

sw.bb35:                                          ; preds = %if.end30
  store i32 1000, ptr %oldp, align 4, !tbaa !9
  br label %cleanup

sw.bb36:                                          ; preds = %if.end30
  store i32 10, ptr %oldp, align 4, !tbaa !9
  br label %cleanup

sw.bb37:                                          ; preds = %if.end30
  store i32 32, ptr %oldp, align 4, !tbaa !9
  br label %cleanup

sw.bb38:                                          ; preds = %if.end30
  store i32 2048, ptr %oldp, align 4, !tbaa !9
  br label %cleanup

sw.bb39:                                          ; preds = %if.end30
  store i32 255, ptr %oldp, align 4, !tbaa !9
  br label %cleanup

sw.bb40:                                          ; preds = %if.end30
  store i32 199212, ptr %oldp, align 4, !tbaa !9
  br label %cleanup

sw.bb41:                                          ; preds = %if.end30
  store i32 0, ptr %oldp, align 4, !tbaa !9
  br label %cleanup

sw.bb42:                                          ; preds = %if.end30
  store i32 0, ptr %oldp, align 4, !tbaa !9
  br label %cleanup

sw.bb43:                                          ; preds = %if.end30
  store i32 0, ptr %oldp, align 4, !tbaa !9
  br label %cleanup

sw.bb44:                                          ; preds = %if.end30
  store i32 0, ptr %oldp, align 4, !tbaa !9
  br label %cleanup

sw.bb45:                                          ; preds = %if.end30
  store i32 0, ptr %oldp, align 4, !tbaa !9
  br label %cleanup

sw.bb46:                                          ; preds = %if.end30
  store i32 0, ptr %oldp, align 4, !tbaa !9
  br label %cleanup

sw.bb47:                                          ; preds = %if.end30
  store i32 0, ptr %oldp, align 4, !tbaa !9
  br label %cleanup

sw.bb48:                                          ; preds = %if.end30
  store i32 0, ptr %oldp, align 4, !tbaa !9
  br label %cleanup

sw.bb49:                                          ; preds = %if.end30
  store i32 20, ptr %oldp, align 4, !tbaa !9
  br label %cleanup

sw.bb50:                                          ; preds = %if.end30
  store i32 255, ptr %oldp, align 4, !tbaa !9
  br label %cleanup

sw.default:                                       ; preds = %if.end30
  %call51 = tail call ptr @__error() #3
  store i32 22, ptr %call51, align 4, !tbaa !9
  br label %cleanup

cleanup:                                          ; preds = %if.end27.thread, %sw.bb, %lor.lhs.false10, %cond.end, %lor.lhs.false, %sw.default, %sw.bb50, %sw.bb49, %sw.bb48, %sw.bb47, %sw.bb46, %sw.bb45, %sw.bb44, %sw.bb43, %sw.bb42, %sw.bb41, %sw.bb40, %sw.bb39, %sw.bb38, %sw.bb37, %sw.bb36, %sw.bb35, %sw.bb34, %sw.bb33, %sw.bb32, %if.then25, %ref_set_user_str.exit104, %if.then16, %ref_set_user_str.exit, %if.then5
  %retval.0 = phi i32 [ -1, %if.then5 ], [ %retval1.1.i, %ref_set_user_str.exit ], [ -1, %if.then16 ], [ %retval1.1.i101, %ref_set_user_str.exit104 ], [ -1, %if.then25 ], [ -1, %sw.default ], [ 0, %sw.bb50 ], [ 0, %sw.bb49 ], [ 0, %sw.bb48 ], [ 0, %sw.bb47 ], [ 0, %sw.bb46 ], [ 0, %sw.bb45 ], [ 0, %sw.bb44 ], [ 0, %sw.bb43 ], [ 0, %sw.bb42 ], [ 0, %sw.bb41 ], [ 0, %sw.bb40 ], [ 0, %sw.bb39 ], [ 0, %sw.bb38 ], [ 0, %sw.bb37 ], [ 0, %sw.bb36 ], [ 0, %sw.bb35 ], [ 0, %sw.bb34 ], [ 0, %sw.bb33 ], [ 0, %sw.bb32 ], [ 0, %lor.lhs.false ], [ %call, %cond.end ], [ 0, %lor.lhs.false10 ], [ 0, %sw.bb ], [ 0, %if.end27.thread ]
  ret i32 %retval.0
}

declare i32 @__sysctl(ptr noundef, i32 noundef, ptr noundef, ptr noundef, ptr noundef, i64 noundef) local_unnamed_addr #1

declare ptr @__error() local_unnamed_addr #1

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #2

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!5 = !{!6, !6, i64 0}
!6 = !{!"long", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{!10, !10, i64 0}
!10 = !{!"int", !7, i64 0}
