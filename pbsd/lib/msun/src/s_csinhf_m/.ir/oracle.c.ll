; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/s_csinhf_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/s_csinhf_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local <2 x float> @ref_csinhf(<2 x float> noundef %z.coerce) local_unnamed_addr #0 {
entry:
  %c = alloca float, align 4
  %s = alloca float, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %c) #5
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %s) #5
  %z.sroa.0.0.vec.extract = extractelement <2 x float> %z.coerce, i64 0
  %z.sroa.0.4.vec.extract197 = extractelement <2 x float> %z.coerce, i64 1
  %0 = tail call float @llvm.fabs.f32(float %z.sroa.0.0.vec.extract)
  %and = bitcast float %0 to i32
  %1 = tail call float @llvm.fabs.f32(float %z.sroa.0.4.vec.extract197)
  %and9 = bitcast float %1 to i32
  %cmp = icmp slt i32 %and, 2139095040
  %cmp10 = icmp slt i32 %and9, 2139095040
  %or.cond = and i1 %cmp, %cmp10
  br i1 %or.cond, label %if.then, label %if.end69

if.then:                                          ; preds = %entry
  %cmp11 = icmp eq i32 %and9, 0
  br i1 %cmp11, label %if.then12, label %if.end

if.then12:                                        ; preds = %if.then
  %call = tail call float @sinhf(float noundef %z.sroa.0.0.vec.extract) #5
  %retval.sroa.0.4.vec.insert = insertelement <2 x float> %z.coerce, float %call, i64 0
  br label %cleanup

if.end:                                           ; preds = %if.then
  call void @sincosf(float noundef %z.sroa.0.4.vec.extract197, ptr noundef nonnull %s, ptr noundef nonnull %c) #5
  %cmp15 = icmp slt i32 %and, 1091567616
  br i1 %cmp15, label %if.then16, label %if.end25

if.then16:                                        ; preds = %if.end
  %call18 = call float @sinhf(float noundef %z.sroa.0.0.vec.extract) #5
  %2 = load float, ptr %c, align 4, !tbaa !5
  %call19 = call float @coshf(float noundef %z.sroa.0.0.vec.extract) #5
  %3 = load float, ptr %s, align 4, !tbaa !5
  %4 = insertelement <2 x float> poison, float %call18, i64 0
  %5 = insertelement <2 x float> %4, float %call19, i64 1
  %6 = insertelement <2 x float> poison, float %2, i64 0
  %7 = insertelement <2 x float> %6, float %3, i64 1
  %8 = fmul <2 x float> %5, %7
  br label %cleanup

if.end25:                                         ; preds = %if.end
  %cmp26 = icmp ult i32 %and, 1118925336
  br i1 %cmp26, label %if.then27, label %if.else

if.then27:                                        ; preds = %if.end25
  %call28 = call float @expf(float noundef %0) #5
  %div = fmul float %call28, 5.000000e-01
  %9 = call float @llvm.copysign.f32(float %div, float %z.sroa.0.0.vec.extract)
  %10 = load float, ptr %c, align 4, !tbaa !5
  %11 = load float, ptr %s, align 4, !tbaa !5
  %12 = insertelement <2 x float> poison, float %10, i64 0
  %13 = insertelement <2 x float> %12, float %div, i64 1
  %14 = insertelement <2 x float> poison, float %9, i64 0
  %15 = insertelement <2 x float> %14, float %11, i64 1
  %16 = fmul <2 x float> %13, %15
  br label %cleanup

if.else:                                          ; preds = %if.end25
  %cmp36 = icmp ult i32 %and, 1128313319
  br i1 %cmp36, label %if.then37, label %if.else59

if.then37:                                        ; preds = %if.else
  %coerce.sroa.0.4.vec.insert = insertelement <2 x float> %z.coerce, float %0, i64 0
  %call41 = call <2 x float> @__ldexp_cexpf(<2 x float> noundef %coerce.sroa.0.4.vec.insert, i32 noundef -1) #5
  %coerce42.sroa.0.0.vec.extract = extractelement <2 x float> %call41, i64 0
  %17 = call float @llvm.copysign.f32(float 1.000000e+00, float %z.sroa.0.0.vec.extract)
  %mul50 = fmul float %17, %coerce42.sroa.0.0.vec.extract
  %retval.sroa.0.4.vec.insert229 = insertelement <2 x float> %call41, float %mul50, i64 0
  br label %cleanup

if.else59:                                        ; preds = %if.else
  %18 = load float, ptr %c, align 4, !tbaa !5
  %19 = load float, ptr %s, align 4, !tbaa !5
  %mul60 = fmul float %z.sroa.0.0.vec.extract, 0x47E0000000000000
  %mul63 = fmul float %mul60, %mul60
  %20 = insertelement <2 x float> poison, float %mul60, i64 0
  %21 = insertelement <2 x float> %20, float %mul63, i64 1
  %22 = insertelement <2 x float> poison, float %18, i64 0
  %23 = insertelement <2 x float> %22, float %19, i64 1
  %24 = fmul <2 x float> %21, %23
  br label %cleanup

if.end69:                                         ; preds = %entry
  %cmp70 = icmp eq i32 %and, 0
  br i1 %cmp70, label %if.then71, label %if.end77

if.then71:                                        ; preds = %if.end69
  %sub = fsub float %z.sroa.0.4.vec.extract197, %z.sroa.0.4.vec.extract197
  %retval.sroa.0.4.vec.insert233 = insertelement <2 x float> %z.coerce, float %sub, i64 1
  br label %cleanup

if.end77:                                         ; preds = %if.end69
  %cmp78 = icmp eq i32 %and9, 0
  br i1 %cmp78, label %if.then79, label %if.end85

if.then79:                                        ; preds = %if.end77
  %25 = fadd <2 x float> %z.coerce, %z.coerce
  %retval.sroa.0.4.vec.insert235 = shufflevector <2 x float> %25, <2 x float> %z.coerce, <2 x i32> <i32 0, i32 3>
  br label %cleanup

if.end85:                                         ; preds = %if.end77
  br i1 %cmp, label %if.then87, label %if.end95

if.then87:                                        ; preds = %if.end85
  %sub89 = fsub float %z.sroa.0.4.vec.extract197, %z.sroa.0.4.vec.extract197
  %retval.sroa.0.0.vec.insert217 = insertelement <2 x float> poison, float %sub89, i64 0
  %retval.sroa.0.4.vec.insert237 = shufflevector <2 x float> %retval.sroa.0.0.vec.insert217, <2 x float> poison, <2 x i32> zeroinitializer
  br label %cleanup

if.end95:                                         ; preds = %if.end85
  %cmp96 = icmp eq i32 %and, 2139095040
  br i1 %cmp96, label %if.then97, label %if.end114

if.then97:                                        ; preds = %if.end95
  %cmp98 = icmp sgt i32 %and9, 2139095039
  br i1 %cmp98, label %if.then99, label %if.end106

if.then99:                                        ; preds = %if.then97
  %sub101 = fsub float %z.sroa.0.4.vec.extract197, %z.sroa.0.4.vec.extract197
  %retval.sroa.0.4.vec.insert239 = insertelement <2 x float> %z.coerce, float %sub101, i64 1
  br label %cleanup

if.end106:                                        ; preds = %if.then97
  call void @sincosf(float noundef %z.sroa.0.4.vec.extract197, ptr noundef nonnull %s, ptr noundef nonnull %c) #5
  %26 = load float, ptr %c, align 4, !tbaa !5
  %mul108 = fmul float %z.sroa.0.0.vec.extract, %26
  %27 = load float, ptr %s, align 4, !tbaa !5
  %mul109 = fmul float %27, 0x7FF0000000000000
  %retval.sroa.0.0.vec.insert221 = insertelement <2 x float> poison, float %mul108, i64 0
  %retval.sroa.0.4.vec.insert241 = insertelement <2 x float> %retval.sroa.0.0.vec.insert221, float %mul109, i64 1
  br label %cleanup

if.end114:                                        ; preds = %if.end95
  %conv = fpext float %z.sroa.0.0.vec.extract to x86_fp80
  %add117 = fadd x86_fp80 %conv, %conv
  %sub118 = fsub float %z.sroa.0.4.vec.extract197, %z.sroa.0.4.vec.extract197
  %conv119 = fpext float %sub118 to x86_fp80
  %mul120 = fmul x86_fp80 %add117, %conv119
  %conv121 = fptrunc x86_fp80 %mul120 to float
  %mul124 = fmul x86_fp80 %conv, %conv
  %mul127 = fmul x86_fp80 %mul124, %conv119
  %conv128 = fptrunc x86_fp80 %mul127 to float
  %retval.sroa.0.0.vec.insert223 = insertelement <2 x float> poison, float %conv121, i64 0
  %retval.sroa.0.4.vec.insert243 = insertelement <2 x float> %retval.sroa.0.0.vec.insert223, float %conv128, i64 1
  br label %cleanup

cleanup:                                          ; preds = %if.end114, %if.end106, %if.then99, %if.then87, %if.then79, %if.then71, %if.else59, %if.then37, %if.then27, %if.then16, %if.then12
  %retval.sroa.0.0 = phi <2 x float> [ %retval.sroa.0.4.vec.insert, %if.then12 ], [ %8, %if.then16 ], [ %16, %if.then27 ], [ %retval.sroa.0.4.vec.insert229, %if.then37 ], [ %24, %if.else59 ], [ %retval.sroa.0.4.vec.insert233, %if.then71 ], [ %retval.sroa.0.4.vec.insert235, %if.then79 ], [ %retval.sroa.0.4.vec.insert237, %if.then87 ], [ %retval.sroa.0.4.vec.insert239, %if.then99 ], [ %retval.sroa.0.4.vec.insert241, %if.end106 ], [ %retval.sroa.0.4.vec.insert243, %if.end114 ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %s) #5
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %c) #5
  ret <2 x float> %retval.sroa.0.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare float @sinhf(float noundef) local_unnamed_addr #2

declare void @sincosf(float noundef, ptr noundef, ptr noundef) local_unnamed_addr #3

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare float @coshf(float noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare float @expf(float noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fabs.f32(float) #4

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.copysign.f32(float, float) #4

declare <2 x float> @__ldexp_cexpf(<2 x float> noundef, i32 noundef) local_unnamed_addr #3

; Function Attrs: nounwind uwtable
define dso_local <2 x float> @ref_csinf(<2 x float> noundef %z.coerce) local_unnamed_addr #0 {
entry:
  %coerce.sroa.0.4.vec.insert = shufflevector <2 x float> %z.coerce, <2 x float> poison, <2 x i32> <i32 1, i32 0>
  %call = tail call <2 x float> @ref_csinhf(<2 x float> noundef %coerce.sroa.0.4.vec.insert)
  %retval.sroa.0.4.vec.insert = shufflevector <2 x float> %call, <2 x float> poison, <2 x i32> <i32 1, i32 0>
  ret <2 x float> %retval.sroa.0.4.vec.insert
}

attributes #0 = { nounwind uwtable "min-legal-vector-width"="64" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
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
!6 = !{!"float", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
