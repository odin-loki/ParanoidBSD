; ModuleID = '/home/odin/pbsd/pbsd/usr.sbin/setpmac/setpmac_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/usr.sbin/setpmac/setpmac_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

module asm ".symver __qsort_r_compat, qsort_r@FBSD_1.0"

@.str = private unnamed_addr constant [14 x i8] c"mac_from_text\00", align 1
@.str.1 = private unnamed_addr constant [3 x i8] c"%s\00", align 1
@__stderrp = external local_unnamed_addr global ptr, align 8
@.str.5 = private unnamed_addr constant [38 x i8] c"setpmac [label] [command] [args ...]\0A\00", align 1

; Function Attrs: nounwind uwtable
define dso_local noundef i32 @ref_main(i32 noundef %argc, ptr noundef %argv) local_unnamed_addr #0 {
entry:
  %label = alloca ptr, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %label) #7
  %cmp = icmp slt i32 %argc, 3
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %0 = load ptr, ptr @__stderrp, align 8, !tbaa !5
  %1 = tail call i64 @fwrite(ptr nonnull @.str.5, i64 37, i64 1, ptr %0)
  tail call void @exit(i32 noundef 64) #8
  unreachable

if.end:                                           ; preds = %entry
  %arrayidx = getelementptr inbounds ptr, ptr %argv, i64 1
  %2 = load ptr, ptr %arrayidx, align 8, !tbaa !5
  %call = call i32 @mac_from_text(ptr noundef nonnull %label, ptr noundef %2) #7
  %cmp1.not = icmp eq i32 %call, 0
  br i1 %cmp1.not, label %if.end3, label %if.then2

if.then2:                                         ; preds = %if.end
  call void @perror(ptr noundef nonnull @.str) #9
  br label %cleanup

if.end3:                                          ; preds = %if.end
  %3 = load ptr, ptr %label, align 8, !tbaa !5
  %call4 = call i32 @mac_set_proc(ptr noundef %3) #7
  %cmp5.not = icmp eq i32 %call4, 0
  br i1 %cmp5.not, label %if.then11, label %if.then6

if.then6:                                         ; preds = %if.end3
  %4 = load ptr, ptr %arrayidx, align 8, !tbaa !5
  call void @perror(ptr noundef %4) #9
  br label %cleanup

if.then11:                                        ; preds = %if.end3
  %5 = load ptr, ptr %label, align 8, !tbaa !5
  %call9 = call i32 @mac_free(ptr noundef %5) #7
  %arrayidx12 = getelementptr inbounds ptr, ptr %argv, i64 2
  %6 = load ptr, ptr %arrayidx12, align 8, !tbaa !5
  %call13 = call i32 @execvp(ptr noundef %6, ptr noundef nonnull %arrayidx12) #7
  %7 = load ptr, ptr %arrayidx12, align 8, !tbaa !5
  call void (i32, ptr, ...) @err(i32 noundef 1, ptr noundef nonnull @.str.1, ptr noundef %7) #8
  unreachable

cleanup:                                          ; preds = %if.then6, %if.then2
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %label) #7
  ret i32 -1
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

declare i32 @mac_from_text(ptr noundef, ptr noundef) local_unnamed_addr #2

; Function Attrs: nofree nounwind
declare void @perror(ptr nocapture noundef readonly) local_unnamed_addr #3

declare i32 @mac_set_proc(ptr noundef) local_unnamed_addr #2

declare i32 @mac_free(ptr noundef) local_unnamed_addr #2

; Function Attrs: nofree
declare i32 @execvp(ptr noundef, ptr noundef) local_unnamed_addr #4

; Function Attrs: noreturn
declare void @err(i32 noundef, ptr noundef, ...) local_unnamed_addr #5

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: noreturn
declare void @exit(i32 noundef) local_unnamed_addr #5

; Function Attrs: nofree nounwind
declare noundef i64 @fwrite(ptr nocapture noundef, i64 noundef, i64 noundef, ptr nocapture noundef) local_unnamed_addr #6

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nofree nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nofree "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { noreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { nofree nounwind }
attributes #7 = { nounwind }
attributes #8 = { noreturn nounwind }
attributes #9 = { cold }

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
