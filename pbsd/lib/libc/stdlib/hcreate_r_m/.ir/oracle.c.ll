; ModuleID = '/home/odin/pbsd/pbsd/lib/libc/stdlib/hcreate_r_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/libc/stdlib/hcreate_r_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

module asm ".symver __qsort_r_compat, qsort_r@FBSD_1.0"

%struct.__hsearch = type { i64, i64, i64, ptr }

; Function Attrs: nounwind uwtable
define dso_local noundef i32 @ref_hcreate_r(i64 noundef %nel, ptr nocapture noundef writeonly %htab) local_unnamed_addr #0 {
entry:
  %call = tail call noalias dereferenceable_or_null(32) ptr @malloc(i64 noundef 32) #5
  %cmp = icmp eq ptr %call, null
  br i1 %cmp, label %cleanup, label %if.end

if.end:                                           ; preds = %entry
  %call1 = tail call noalias dereferenceable_or_null(256) ptr @calloc(i64 noundef 16, i64 noundef 16) #6
  %entries = getelementptr inbounds %struct.__hsearch, ptr %call, i64 0, i32 3
  store ptr %call1, ptr %entries, align 8, !tbaa !5
  %cmp3 = icmp eq ptr %call1, null
  br i1 %cmp3, label %if.then4, label %if.end5

if.then4:                                         ; preds = %if.end
  tail call void @free(ptr noundef nonnull %call)
  br label %cleanup

if.end5:                                          ; preds = %if.end
  tail call void @arc4random_buf(ptr noundef nonnull %call, i64 noundef 8) #7
  %index_mask = getelementptr inbounds %struct.__hsearch, ptr %call, i64 0, i32 1
  store i64 15, ptr %index_mask, align 8, !tbaa !11
  %entries_used = getelementptr inbounds %struct.__hsearch, ptr %call, i64 0, i32 2
  store i64 0, ptr %entries_used, align 8, !tbaa !12
  store ptr %call, ptr %htab, align 8, !tbaa !13
  br label %cleanup

cleanup:                                          ; preds = %entry, %if.end5, %if.then4
  %retval.0 = phi i32 [ 0, %if.then4 ], [ 1, %if.end5 ], [ 0, %entry ]
  ret i32 %retval.0
}

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #1

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,zeroed") allocsize(0,1) memory(inaccessiblemem: readwrite)
declare noalias noundef ptr @calloc(i64 noundef, i64 noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite)
declare void @free(ptr allocptr nocapture noundef) local_unnamed_addr #3

declare void @arc4random_buf(ptr noundef, i64 noundef) local_unnamed_addr #4

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { mustprogress nofree nounwind willreturn allockind("alloc,zeroed") allocsize(0,1) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { allocsize(0) }
attributes #6 = { allocsize(0,1) }
attributes #7 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!5 = !{!6, !10, i64 24}
!6 = !{!"__hsearch", !7, i64 0, !7, i64 8, !7, i64 16, !10, i64 24}
!7 = !{!"long", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{!"any pointer", !8, i64 0}
!11 = !{!6, !7, i64 8}
!12 = !{!6, !7, i64 16}
!13 = !{!14, !10, i64 0}
!14 = !{!"hsearch_data", !10, i64 0}
