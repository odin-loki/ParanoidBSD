; ModuleID = '/home/odin/pbsd/pbsd/lib/libc/xdr/xdr_sizeof_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/libc/xdr/xdr_sizeof_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

module asm ".symver __qsort_r_compat, qsort_r@FBSD_1.0"

%struct.XDR = type { i32, ptr, ptr, ptr, ptr, i32 }
%struct.xdr_ops = type { ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr }

; Function Attrs: nounwind uwtable
define dso_local i64 @ref_xdr_sizeof(ptr nocapture noundef readonly %func, ptr noundef %data) local_unnamed_addr #0 {
entry:
  %x = alloca %struct.XDR, align 8
  %ops = alloca %struct.xdr_ops, align 8
  call void @llvm.lifetime.start.p0(i64 48, ptr nonnull %x) #9
  call void @llvm.lifetime.start.p0(i64 72, ptr nonnull %ops) #9
  %x_putlong = getelementptr inbounds %struct.xdr_ops, ptr %ops, i64 0, i32 1
  store ptr @ref_x_putlong, ptr %x_putlong, align 8, !tbaa !5
  %x_putbytes = getelementptr inbounds %struct.xdr_ops, ptr %ops, i64 0, i32 3
  store ptr @ref_x_putbytes, ptr %x_putbytes, align 8, !tbaa !10
  %x_inline = getelementptr inbounds %struct.xdr_ops, ptr %ops, i64 0, i32 6
  store ptr @ref_x_inline, ptr %x_inline, align 8, !tbaa !11
  %x_getpostn = getelementptr inbounds %struct.xdr_ops, ptr %ops, i64 0, i32 4
  store ptr @ref_x_getpostn, ptr %x_getpostn, align 8, !tbaa !12
  %x_setpostn = getelementptr inbounds %struct.xdr_ops, ptr %ops, i64 0, i32 5
  store ptr @ref_x_setpostn, ptr %x_setpostn, align 8, !tbaa !13
  %x_destroy = getelementptr inbounds %struct.xdr_ops, ptr %ops, i64 0, i32 7
  store ptr @ref_x_destroy, ptr %x_destroy, align 8, !tbaa !14
  store ptr @ref_harmless, ptr %ops, align 8, !tbaa !15
  %x_getbytes = getelementptr inbounds %struct.xdr_ops, ptr %ops, i64 0, i32 2
  store ptr @ref_harmless, ptr %x_getbytes, align 8, !tbaa !16
  store i32 0, ptr %x, align 8, !tbaa !17
  %x_ops = getelementptr inbounds %struct.XDR, ptr %x, i64 0, i32 1
  store ptr %ops, ptr %x_ops, align 8, !tbaa !20
  %x_private = getelementptr inbounds %struct.XDR, ptr %x, i64 0, i32 3
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(20) %x_private, i8 0, i64 20, i1 false)
  %call = call i32 (ptr, ...) %func(ptr noundef nonnull %x, ptr noundef %data) #9
  %0 = load ptr, ptr %x_private, align 8, !tbaa !21
  %tobool.not = icmp eq ptr %0, null
  br i1 %tobool.not, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  call void @free(ptr noundef nonnull %0)
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %x_handy = getelementptr inbounds %struct.XDR, ptr %x, i64 0, i32 5
  %cmp = icmp eq i32 %call, 1
  %1 = load i32, ptr %x_handy, align 8
  %cond = select i1 %cmp, i32 %1, i32 0
  %conv = zext i32 %cond to i64
  call void @llvm.lifetime.end.p0(i64 72, ptr nonnull %ops) #9
  call void @llvm.lifetime.end.p0(i64 48, ptr nonnull %x) #9
  ret i64 %conv
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: readwrite) uwtable
define internal noundef i32 @ref_x_putlong(ptr nocapture noundef %xdrs, ptr nocapture readnone %longp) #2 {
entry:
  %x_handy = getelementptr inbounds %struct.XDR, ptr %xdrs, i64 0, i32 5
  %0 = load i32, ptr %x_handy, align 8, !tbaa !22
  %add = add i32 %0, 4
  store i32 %add, ptr %x_handy, align 8, !tbaa !22
  ret i32 1
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: readwrite) uwtable
define internal noundef i32 @ref_x_putbytes(ptr nocapture noundef %xdrs, ptr nocapture readnone %bp, i32 noundef %len) #2 {
entry:
  %x_handy = getelementptr inbounds %struct.XDR, ptr %xdrs, i64 0, i32 5
  %0 = load i32, ptr %x_handy, align 8, !tbaa !22
  %add = add i32 %0, %len
  store i32 %add, ptr %x_handy, align 8, !tbaa !22
  ret i32 1
}

; Function Attrs: mustprogress nounwind willreturn uwtable
define internal ptr @ref_x_inline(ptr nocapture noundef %xdrs, i32 noundef %len) #3 {
entry:
  %cmp = icmp eq i32 %len, 0
  br i1 %cmp, label %return, label %if.end

if.end:                                           ; preds = %entry
  %0 = load i32, ptr %xdrs, align 8, !tbaa !17
  %cmp1.not = icmp eq i32 %0, 0
  br i1 %cmp1.not, label %if.end3, label %return

if.end3:                                          ; preds = %if.end
  %x_base = getelementptr inbounds %struct.XDR, ptr %xdrs, i64 0, i32 4
  %1 = load ptr, ptr %x_base, align 8, !tbaa !23
  %2 = ptrtoint ptr %1 to i64
  %conv = trunc i64 %2 to i32
  %cmp4 = icmp ugt i32 %conv, %len
  br i1 %cmp4, label %if.then6, label %if.else

if.then6:                                         ; preds = %if.end3
  %x_handy = getelementptr inbounds %struct.XDR, ptr %xdrs, i64 0, i32 5
  %3 = load i32, ptr %x_handy, align 8, !tbaa !22
  %add = add i32 %3, %len
  store i32 %add, ptr %x_handy, align 8, !tbaa !22
  %x_private = getelementptr inbounds %struct.XDR, ptr %xdrs, i64 0, i32 3
  %4 = load ptr, ptr %x_private, align 8, !tbaa !21
  br label %return

if.else:                                          ; preds = %if.end3
  %x_private7 = getelementptr inbounds %struct.XDR, ptr %xdrs, i64 0, i32 3
  %5 = load ptr, ptr %x_private7, align 8, !tbaa !21
  %tobool.not = icmp eq ptr %5, null
  br i1 %tobool.not, label %if.end10, label %if.then8

if.then8:                                         ; preds = %if.else
  tail call void @free(ptr noundef nonnull %5)
  br label %if.end10

if.end10:                                         ; preds = %if.then8, %if.else
  %conv11 = zext i32 %len to i64
  %call = tail call noalias ptr @malloc(i64 noundef %conv11) #10
  store ptr %call, ptr %x_private7, align 8, !tbaa !21
  %cmp13 = icmp eq ptr %call, null
  br i1 %cmp13, label %if.then15, label %if.end17

if.then15:                                        ; preds = %if.end10
  store ptr null, ptr %x_base, align 8, !tbaa !23
  br label %return

if.end17:                                         ; preds = %if.end10
  %6 = inttoptr i64 %conv11 to ptr
  store ptr %6, ptr %x_base, align 8, !tbaa !23
  %x_handy20 = getelementptr inbounds %struct.XDR, ptr %xdrs, i64 0, i32 5
  %7 = load i32, ptr %x_handy20, align 8, !tbaa !22
  %add21 = add i32 %7, %len
  store i32 %add21, ptr %x_handy20, align 8, !tbaa !22
  br label %return

return:                                           ; preds = %if.end, %entry, %if.end17, %if.then15, %if.then6
  %retval.0 = phi ptr [ %4, %if.then6 ], [ null, %if.then15 ], [ %call, %if.end17 ], [ null, %entry ], [ null, %if.end ]
  ret ptr %retval.0
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable
define internal i32 @ref_x_getpostn(ptr nocapture noundef readonly %xdrs) #4 {
entry:
  %x_handy = getelementptr inbounds %struct.XDR, ptr %xdrs, i64 0, i32 5
  %0 = load i32, ptr %x_handy, align 8, !tbaa !22
  ret i32 %0
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define internal noundef i32 @ref_x_setpostn(ptr nocapture readnone %xdrs, i32 %pos) #5 {
entry:
  ret i32 0
}

; Function Attrs: mustprogress nounwind willreturn uwtable
define internal void @ref_x_destroy(ptr nocapture noundef %xdrs) #3 {
entry:
  %x_handy = getelementptr inbounds %struct.XDR, ptr %xdrs, i64 0, i32 5
  store i32 0, ptr %x_handy, align 8, !tbaa !22
  %x_base = getelementptr inbounds %struct.XDR, ptr %xdrs, i64 0, i32 4
  store ptr null, ptr %x_base, align 8, !tbaa !23
  %x_private = getelementptr inbounds %struct.XDR, ptr %xdrs, i64 0, i32 3
  %0 = load ptr, ptr %x_private, align 8, !tbaa !21
  %tobool.not = icmp eq ptr %0, null
  br i1 %tobool.not, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  tail call void @free(ptr noundef nonnull %0)
  store ptr null, ptr %x_private, align 8, !tbaa !21
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  ret void
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define internal noundef i32 @ref_harmless() #5 {
entry:
  ret i32 0
}

; Function Attrs: mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite)
declare void @free(ptr allocptr nocapture noundef) local_unnamed_addr #6

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #7

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #8

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: readwrite) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { mustprogress nounwind willreturn uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #7 = { mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #8 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #9 = { nounwind }
attributes #10 = { allocsize(0) }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!5 = !{!6, !7, i64 8}
!6 = !{!"xdr_ops", !7, i64 0, !7, i64 8, !7, i64 16, !7, i64 24, !7, i64 32, !7, i64 40, !7, i64 48, !7, i64 56, !7, i64 64}
!7 = !{!"any pointer", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{!6, !7, i64 24}
!11 = !{!6, !7, i64 48}
!12 = !{!6, !7, i64 32}
!13 = !{!6, !7, i64 40}
!14 = !{!6, !7, i64 56}
!15 = !{!6, !7, i64 0}
!16 = !{!6, !7, i64 16}
!17 = !{!18, !19, i64 0}
!18 = !{!"XDR", !19, i64 0, !7, i64 8, !7, i64 16, !7, i64 24, !7, i64 32, !19, i64 40}
!19 = !{!"int", !8, i64 0}
!20 = !{!18, !7, i64 8}
!21 = !{!18, !7, i64 24}
!22 = !{!18, !19, i64 40}
!23 = !{!18, !7, i64 32}
