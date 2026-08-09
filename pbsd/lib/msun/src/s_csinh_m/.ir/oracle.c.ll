; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/s_csinh_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/s_csinh_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local { double, double } @ref_csinh(double noundef %z.coerce0, double noundef %z.coerce1) local_unnamed_addr #0 {
entry:
  %c = alloca double, align 8
  %s = alloca double, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %c) #5
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %s) #5
  %0 = bitcast double %z.coerce0 to i64
  %ew_u.sroa.0.4.extract.shift = lshr i64 %0, 32
  %ew_u.sroa.0.4.extract.trunc = trunc i64 %ew_u.sroa.0.4.extract.shift to i32
  %ew_u.sroa.0.0.extract.trunc = trunc i64 %0 to i32
  %1 = bitcast double %z.coerce1 to i64
  %ew_u6.sroa.0.4.extract.shift = lshr i64 %1, 32
  %ew_u6.sroa.0.4.extract.trunc = trunc i64 %ew_u6.sroa.0.4.extract.shift to i32
  %ew_u6.sroa.0.0.extract.trunc = trunc i64 %1 to i32
  %and = and i32 %ew_u.sroa.0.4.extract.trunc, 2147483647
  %and11 = and i32 %ew_u6.sroa.0.4.extract.trunc, 2147483647
  %cmp = icmp ult i32 %and, 2146435072
  %cmp12 = icmp ult i32 %and11, 2146435072
  %or.cond = and i1 %cmp, %cmp12
  br i1 %or.cond, label %if.then, label %if.end70

if.then:                                          ; preds = %entry
  %or = or i32 %and11, %ew_u6.sroa.0.0.extract.trunc
  %cmp13 = icmp eq i32 %or, 0
  br i1 %cmp13, label %if.then14, label %if.end

if.then14:                                        ; preds = %if.then
  %call = tail call double @sinh(double noundef %z.coerce0) #5
  br label %cleanup

if.end:                                           ; preds = %if.then
  call void @sincos(double noundef %z.coerce1, ptr noundef nonnull %s, ptr noundef nonnull %c) #5
  %cmp17 = icmp ult i32 %and, 1077280768
  br i1 %cmp17, label %if.then18, label %if.end27

if.then18:                                        ; preds = %if.end
  %call20 = call double @sinh(double noundef %z.coerce0) #5
  %2 = load double, ptr %c, align 8, !tbaa !5
  %mul = fmul double %call20, %2
  %call21 = call double @cosh(double noundef %z.coerce0) #5
  %3 = load double, ptr %s, align 8, !tbaa !5
  %mul22 = fmul double %call21, %3
  br label %cleanup

if.end27:                                         ; preds = %if.end
  %cmp28 = icmp ult i32 %and, 1082535490
  br i1 %cmp28, label %if.then29, label %if.else

if.then29:                                        ; preds = %if.end27
  %4 = call double @llvm.fabs.f64(double %z.coerce0)
  %call30 = call double @exp(double noundef %4) #5
  %div = fmul double %call30, 5.000000e-01
  %5 = call double @llvm.copysign.f64(double %div, double %z.coerce0)
  %6 = load double, ptr %c, align 8, !tbaa !5
  %mul32 = fmul double %6, %5
  %7 = load double, ptr %s, align 8, !tbaa !5
  %mul33 = fmul double %div, %7
  br label %cleanup

if.else:                                          ; preds = %if.end27
  %cmp38 = icmp ult i32 %and, 1083620266
  br i1 %cmp38, label %if.then39, label %if.else60

if.then39:                                        ; preds = %if.else
  %8 = call double @llvm.fabs.f64(double %z.coerce0)
  %call43 = call { double, double } @__ldexp_cexp(double noundef %8, double noundef %z.coerce1, i32 noundef -1) #5
  %9 = extractvalue { double, double } %call43, 0
  %10 = extractvalue { double, double } %call43, 1
  %11 = call double @llvm.copysign.f64(double 1.000000e+00, double %z.coerce0)
  %mul51 = fmul double %11, %9
  br label %cleanup

if.else60:                                        ; preds = %if.else
  %mul61 = fmul double %z.coerce0, 0x7FE0000000000000
  %12 = load double, ptr %c, align 8, !tbaa !5
  %mul63 = fmul double %mul61, %12
  %mul64 = fmul double %mul61, %mul61
  %13 = load double, ptr %s, align 8, !tbaa !5
  %mul65 = fmul double %mul64, %13
  br label %cleanup

if.end70:                                         ; preds = %entry
  %or71 = or i32 %and, %ew_u.sroa.0.0.extract.trunc
  %cmp72 = icmp eq i32 %or71, 0
  br i1 %cmp72, label %if.then73, label %if.end79

if.then73:                                        ; preds = %if.end70
  %sub = fsub double %z.coerce1, %z.coerce1
  br label %cleanup

if.end79:                                         ; preds = %if.end70
  %or80 = or i32 %and11, %ew_u6.sroa.0.0.extract.trunc
  %cmp81 = icmp eq i32 %or80, 0
  br i1 %cmp81, label %if.then82, label %if.end88

if.then82:                                        ; preds = %if.end79
  %add = fadd double %z.coerce0, %z.coerce0
  br label %cleanup

if.end88:                                         ; preds = %if.end79
  br i1 %cmp, label %if.then90, label %if.end98

if.then90:                                        ; preds = %if.end88
  %sub92 = fsub double %z.coerce1, %z.coerce1
  br label %cleanup

if.end98:                                         ; preds = %if.end88
  %cmp99 = icmp eq i32 %and, 2146435072
  %cmp101 = icmp eq i32 %ew_u.sroa.0.0.extract.trunc, 0
  %or.cond148 = and i1 %cmp101, %cmp99
  br i1 %or.cond148, label %if.then102, label %if.end119

if.then102:                                       ; preds = %if.end98
  %cmp103 = icmp ugt i32 %and11, 2146435071
  br i1 %cmp103, label %if.then104, label %if.end111

if.then104:                                       ; preds = %if.then102
  %sub106 = fsub double %z.coerce1, %z.coerce1
  br label %cleanup

if.end111:                                        ; preds = %if.then102
  call void @sincos(double noundef %z.coerce1, ptr noundef nonnull %s, ptr noundef nonnull %c) #5
  %14 = load double, ptr %c, align 8, !tbaa !5
  %mul113 = fmul double %14, %z.coerce0
  %15 = load double, ptr %s, align 8, !tbaa !5
  %mul114 = fmul double %15, 0x7FF0000000000000
  br label %cleanup

if.end119:                                        ; preds = %if.end98
  %conv = fpext double %z.coerce0 to x86_fp80
  %add122 = fadd x86_fp80 %conv, %conv
  %sub123 = fsub double %z.coerce1, %z.coerce1
  %conv124 = fpext double %sub123 to x86_fp80
  %mul125 = fmul x86_fp80 %add122, %conv124
  %conv126 = fptrunc x86_fp80 %mul125 to double
  %mul129 = fmul x86_fp80 %conv, %conv
  %mul132 = fmul x86_fp80 %mul129, %conv124
  %conv133 = fptrunc x86_fp80 %mul132 to double
  br label %cleanup

cleanup:                                          ; preds = %if.end119, %if.end111, %if.then104, %if.then90, %if.then82, %if.then73, %if.else60, %if.then39, %if.then29, %if.then18, %if.then14
  %retval.sroa.0.0 = phi double [ %call, %if.then14 ], [ %mul, %if.then18 ], [ %mul32, %if.then29 ], [ %mul51, %if.then39 ], [ %mul63, %if.else60 ], [ %z.coerce0, %if.then73 ], [ %add, %if.then82 ], [ %sub92, %if.then90 ], [ %z.coerce0, %if.then104 ], [ %mul113, %if.end111 ], [ %conv126, %if.end119 ]
  %retval.sroa.12.0 = phi double [ %z.coerce1, %if.then14 ], [ %mul22, %if.then18 ], [ %mul33, %if.then29 ], [ %10, %if.then39 ], [ %mul65, %if.else60 ], [ %sub, %if.then73 ], [ %z.coerce1, %if.then82 ], [ %sub92, %if.then90 ], [ %sub106, %if.then104 ], [ %mul114, %if.end111 ], [ %conv133, %if.end119 ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %s) #5
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %c) #5
  %.fca.0.insert = insertvalue { double, double } poison, double %retval.sroa.0.0, 0
  %.fca.1.insert = insertvalue { double, double } %.fca.0.insert, double %retval.sroa.12.0, 1
  ret { double, double } %.fca.1.insert
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare double @sinh(double noundef) local_unnamed_addr #2

declare void @sincos(double noundef, ptr noundef, ptr noundef) local_unnamed_addr #3

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare double @cosh(double noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare double @exp(double noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fabs.f64(double) #4

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.copysign.f64(double, double) #4

declare { double, double } @__ldexp_cexp(double noundef, double noundef, i32 noundef) local_unnamed_addr #3

; Function Attrs: nounwind uwtable
define dso_local { double, double } @ref_csin(double noundef %z.coerce0, double noundef %z.coerce1) local_unnamed_addr #0 {
entry:
  %call = tail call { double, double } @ref_csinh(double noundef %z.coerce1, double noundef %z.coerce0)
  %0 = extractvalue { double, double } %call, 0
  %1 = extractvalue { double, double } %call, 1
  %.fca.0.insert = insertvalue { double, double } poison, double %1, 0
  %.fca.1.insert = insertvalue { double, double } %.fca.0.insert, double %0, 1
  ret { double, double } %.fca.1.insert
}

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { mustprogress nofree nounwind willreturn memory(write) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #5 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!5 = !{!6, !6, i64 0}
!6 = !{!"double", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
