; ModuleID = '/home/odin/pbsd/pbsd/lib/libc/xdr/xdr_float_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/libc/xdr/xdr_float_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.XDR = type { i32, ptr, ptr, ptr, ptr, i32 }
%struct.xdr_ops = type { ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr }

; Function Attrs: nounwind uwtable
define dso_local i32 @ref_xdr_float(ptr noundef %xdrs, ptr nocapture noundef %fp) local_unnamed_addr #0 {
entry:
  %l.i7 = alloca i64, align 8
  %l.i = alloca i64, align 8
  %0 = load i32, ptr %xdrs, align 8, !tbaa !5
  switch i32 %0, label %sw.epilog [
    i32 0, label %sw.bb
    i32 1, label %sw.bb1
    i32 2, label %return
  ]

sw.bb:                                            ; preds = %entry
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %l.i) #2
  %1 = load i32, ptr %fp, align 4, !tbaa !11
  %conv.i = sext i32 %1 to i64
  store i64 %conv.i, ptr %l.i, align 8, !tbaa !12
  %x_ops.i = getelementptr inbounds %struct.XDR, ptr %xdrs, i64 0, i32 1
  %2 = load ptr, ptr %x_ops.i, align 8, !tbaa !14
  %x_putlong.i = getelementptr inbounds %struct.xdr_ops, ptr %2, i64 0, i32 1
  %3 = load ptr, ptr %x_putlong.i, align 8, !tbaa !15
  %call.i = call i32 %3(ptr noundef nonnull %xdrs, ptr noundef nonnull %l.i) #2
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %l.i) #2
  br label %return

sw.bb1:                                           ; preds = %entry
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %l.i7) #2
  %x_ops.i8 = getelementptr inbounds %struct.XDR, ptr %xdrs, i64 0, i32 1
  %4 = load ptr, ptr %x_ops.i8, align 8, !tbaa !14
  %5 = load ptr, ptr %4, align 8, !tbaa !17
  %call.i9 = call i32 %5(ptr noundef nonnull %xdrs, ptr noundef nonnull %l.i7) #2
  %tobool.not.i = icmp eq i32 %call.i9, 0
  br i1 %tobool.not.i, label %xdr_getint32.exit, label %if.end.i

if.end.i:                                         ; preds = %sw.bb1
  %6 = load i64, ptr %l.i7, align 8, !tbaa !12
  %conv.i10 = trunc i64 %6 to i32
  store i32 %conv.i10, ptr %fp, align 4, !tbaa !11
  br label %xdr_getint32.exit

xdr_getint32.exit:                                ; preds = %sw.bb1, %if.end.i
  %retval.0.i = phi i32 [ 1, %if.end.i ], [ 0, %sw.bb1 ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %l.i7) #2
  br label %return

sw.epilog:                                        ; preds = %entry
  br label %return

return:                                           ; preds = %entry, %sw.epilog, %xdr_getint32.exit, %sw.bb
  %retval.0 = phi i32 [ 0, %sw.epilog ], [ %retval.0.i, %xdr_getint32.exit ], [ %call.i, %sw.bb ], [ 1, %entry ]
  ret i32 %retval.0
}

; Function Attrs: nounwind uwtable
define dso_local i32 @ref_xdr_double(ptr noundef %xdrs, ptr nocapture noundef %dp) local_unnamed_addr #0 {
entry:
  %l.i33 = alloca i64, align 8
  %l.i29 = alloca i64, align 8
  %l.i24 = alloca i64, align 8
  %l.i = alloca i64, align 8
  %0 = load i32, ptr %xdrs, align 8, !tbaa !5
  switch i32 %0, label %sw.epilog [
    i32 0, label %sw.bb
    i32 1, label %sw.bb2
    i32 2, label %cleanup
  ]

sw.bb:                                            ; preds = %entry
  %add.ptr = getelementptr inbounds i32, ptr %dp, i64 1
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %l.i) #2
  %1 = load i32, ptr %add.ptr, align 4, !tbaa !11
  %conv.i = sext i32 %1 to i64
  store i64 %conv.i, ptr %l.i, align 8, !tbaa !12
  %x_ops.i = getelementptr inbounds %struct.XDR, ptr %xdrs, i64 0, i32 1
  %2 = load ptr, ptr %x_ops.i, align 8, !tbaa !14
  %x_putlong.i = getelementptr inbounds %struct.xdr_ops, ptr %2, i64 0, i32 1
  %3 = load ptr, ptr %x_putlong.i, align 8, !tbaa !15
  %call.i = call i32 %3(ptr noundef nonnull %xdrs, ptr noundef nonnull %l.i) #2
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %l.i) #2
  %tobool.not = icmp eq i32 %call.i, 0
  br i1 %tobool.not, label %cleanup, label %if.end

if.end:                                           ; preds = %sw.bb
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %l.i24) #2
  %4 = load i32, ptr %dp, align 4, !tbaa !11
  %conv.i25 = sext i32 %4 to i64
  store i64 %conv.i25, ptr %l.i24, align 8, !tbaa !12
  %5 = load ptr, ptr %x_ops.i, align 8, !tbaa !14
  %x_putlong.i27 = getelementptr inbounds %struct.xdr_ops, ptr %5, i64 0, i32 1
  %6 = load ptr, ptr %x_putlong.i27, align 8, !tbaa !15
  %call.i28 = call i32 %6(ptr noundef nonnull %xdrs, ptr noundef nonnull %l.i24) #2
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %l.i24) #2
  br label %cleanup

sw.bb2:                                           ; preds = %entry
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %l.i29) #2
  %x_ops.i30 = getelementptr inbounds %struct.XDR, ptr %xdrs, i64 0, i32 1
  %7 = load ptr, ptr %x_ops.i30, align 8, !tbaa !14
  %8 = load ptr, ptr %7, align 8, !tbaa !17
  %call.i31 = call i32 %8(ptr noundef nonnull %xdrs, ptr noundef nonnull %l.i29) #2
  %tobool.not.i = icmp eq i32 %call.i31, 0
  br i1 %tobool.not.i, label %xdr_getint32.exit.thread, label %if.end7

xdr_getint32.exit.thread:                         ; preds = %sw.bb2
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %l.i29) #2
  br label %cleanup

if.end7:                                          ; preds = %sw.bb2
  %add.ptr3 = getelementptr inbounds i32, ptr %dp, i64 1
  %9 = load i64, ptr %l.i29, align 8, !tbaa !12
  %conv.i32 = trunc i64 %9 to i32
  store i32 %conv.i32, ptr %add.ptr3, align 4, !tbaa !11
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %l.i29) #2
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %l.i33) #2
  %10 = load ptr, ptr %x_ops.i30, align 8, !tbaa !14
  %11 = load ptr, ptr %10, align 8, !tbaa !17
  %call.i35 = call i32 %11(ptr noundef nonnull %xdrs, ptr noundef nonnull %l.i33) #2
  %tobool.not.i36 = icmp eq i32 %call.i35, 0
  br i1 %tobool.not.i36, label %xdr_getint32.exit40, label %if.end.i37

if.end.i37:                                       ; preds = %if.end7
  %12 = load i64, ptr %l.i33, align 8, !tbaa !12
  %conv.i38 = trunc i64 %12 to i32
  store i32 %conv.i38, ptr %dp, align 4, !tbaa !11
  br label %xdr_getint32.exit40

xdr_getint32.exit40:                              ; preds = %if.end7, %if.end.i37
  %retval.0.i39 = phi i32 [ 1, %if.end.i37 ], [ 0, %if.end7 ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %l.i33) #2
  br label %cleanup

sw.epilog:                                        ; preds = %entry
  br label %cleanup

cleanup:                                          ; preds = %xdr_getint32.exit.thread, %entry, %sw.bb, %sw.epilog, %xdr_getint32.exit40, %if.end
  %retval.0 = phi i32 [ 0, %sw.epilog ], [ %retval.0.i39, %xdr_getint32.exit40 ], [ %call.i28, %if.end ], [ 0, %sw.bb ], [ 1, %entry ], [ 0, %xdr_getint32.exit.thread ]
  ret i32 %retval.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!5 = !{!6, !7, i64 0}
!6 = !{!"XDR", !7, i64 0, !10, i64 8, !10, i64 16, !10, i64 24, !10, i64 32, !7, i64 40}
!7 = !{!"int", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{!"any pointer", !8, i64 0}
!11 = !{!7, !7, i64 0}
!12 = !{!13, !13, i64 0}
!13 = !{!"long", !8, i64 0}
!14 = !{!6, !10, i64 8}
!15 = !{!16, !10, i64 8}
!16 = !{!"xdr_ops", !10, i64 0, !10, i64 8, !10, i64 16, !10, i64 24, !10, i64 32, !10, i64 40, !10, i64 48, !10, i64 56, !10, i64 64}
!17 = !{!16, !10, i64 0}
