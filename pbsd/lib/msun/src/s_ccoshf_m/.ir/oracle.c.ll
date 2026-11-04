; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/s_ccoshf_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/s_ccoshf_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local <2 x float> @ref_ccoshf(<2 x float> noundef %z.coerce) local_unnamed_addr #0 {
entry:
  %0 = extractelement <2 x float> %z.coerce, i64 1
  %1 = extractelement <2 x float> %z.coerce, i64 0
  %c = alloca float, align 4
  %s = alloca float, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %c) #5
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %s) #5
  %2 = tail call float @llvm.fabs.f32(float %1)
  %and = bitcast float %2 to i32
  %3 = tail call float @llvm.fabs.f32(float %0)
  %and9 = bitcast float %3 to i32
  %cmp = icmp slt i32 %and, 2139095040
  %cmp10 = icmp slt i32 %and9, 2139095040
  %or.cond = and i1 %cmp, %cmp10
  br i1 %or.cond, label %if.then, label %if.end70

if.then:                                          ; preds = %entry
  %cmp11 = icmp eq i32 %and9, 0
  br i1 %cmp11, label %if.then12, label %if.end

if.then12:                                        ; preds = %if.then
  %call = tail call float @coshf(float noundef %1) #5
  %mul = fmul float %1, %0
  %retval.sroa.0.0.vec.insert = insertelement <2 x float> poison, float %call, i64 0
  %retval.sroa.0.4.vec.insert = insertelement <2 x float> %retval.sroa.0.0.vec.insert, float %mul, i64 1
  br label %cleanup

if.end:                                           ; preds = %if.then
  call void @sincosf(float noundef %0, ptr noundef nonnull %s, ptr noundef nonnull %c) #5
  %cmp15 = icmp slt i32 %and, 1091567616
  br i1 %cmp15, label %if.then16, label %if.end26

if.then16:                                        ; preds = %if.end
  %call18 = call float @coshf(float noundef %1) #5
  %4 = load float, ptr %c, align 4, !tbaa !5
  %call20 = call float @sinhf(float noundef %1) #5
  %5 = load float, ptr %s, align 4, !tbaa !5
  %6 = insertelement <2 x float> poison, float %call18, i64 0
  %7 = insertelement <2 x float> %6, float %call20, i64 1
  %8 = insertelement <2 x float> poison, float %4, i64 0
  %9 = insertelement <2 x float> %8, float %5, i64 1
  %10 = fmul <2 x float> %7, %9
  br label %cleanup

if.end26:                                         ; preds = %if.end
  %cmp27 = icmp ult i32 %and, 1118925336
  br i1 %cmp27, label %if.then28, label %if.else

if.then28:                                        ; preds = %if.end26
  %call29 = call float @expf(float noundef %2) #5
  %div = fmul float %call29, 5.000000e-01
  %11 = load float, ptr %c, align 4, !tbaa !5
  %12 = call float @llvm.copysign.f32(float %div, float %1)
  %13 = load float, ptr %s, align 4, !tbaa !5
  %14 = insertelement <2 x float> poison, float %div, i64 0
  %15 = insertelement <2 x float> %14, float %12, i64 1
  %16 = insertelement <2 x float> poison, float %11, i64 0
  %17 = insertelement <2 x float> %16, float %13, i64 1
  %18 = fmul <2 x float> %15, %17
  br label %cleanup

if.else:                                          ; preds = %if.end26
  %cmp37 = icmp ult i32 %and, 1128313319
  br i1 %cmp37, label %if.then38, label %if.else60

if.then38:                                        ; preds = %if.else
  %coerce.sroa.0.4.vec.insert = insertelement <2 x float> %z.coerce, float %2, i64 0
  %call42 = call <2 x float> @__ldexp_cexpf(<2 x float> noundef %coerce.sroa.0.4.vec.insert, i32 noundef -1) #5
  %coerce43.sroa.0.4.vec.extract = extractelement <2 x float> %call42, i64 1
  %19 = call float @llvm.copysign.f32(float 1.000000e+00, float %1)
  %mul55 = fmul float %19, %coerce43.sroa.0.4.vec.extract
  %retval.sroa.0.4.vec.insert238 = insertelement <2 x float> %call42, float %mul55, i64 1
  br label %cleanup

if.else60:                                        ; preds = %if.else
  %20 = load float, ptr %c, align 4, !tbaa !5
  %21 = load float, ptr %s, align 4, !tbaa !5
  %mul61 = fmul float %1, 0x47E0000000000000
  %mul63 = fmul float %mul61, %mul61
  %22 = insertelement <2 x float> poison, float %mul63, i64 0
  %23 = insertelement <2 x float> %22, float %mul61, i64 1
  %24 = insertelement <2 x float> poison, float %20, i64 0
  %25 = insertelement <2 x float> %24, float %21, i64 1
  %26 = fmul <2 x float> %23, %25
  br label %cleanup

if.end70:                                         ; preds = %entry
  %cmp71 = icmp eq i32 %and, 0
  br i1 %cmp71, label %if.then72, label %if.end79

if.then72:                                        ; preds = %if.end70
  %sub = fsub float %0, %0
  %27 = tail call float @llvm.copysign.f32(float 0.000000e+00, float %0)
  %mul74 = fmul float %1, %27
  %retval.sroa.0.0.vec.insert222 = insertelement <2 x float> poison, float %sub, i64 0
  %retval.sroa.0.4.vec.insert242 = insertelement <2 x float> %retval.sroa.0.0.vec.insert222, float %mul74, i64 1
  br label %cleanup

if.end79:                                         ; preds = %if.end70
  %cmp80 = icmp eq i32 %and9, 0
  br i1 %cmp80, label %if.then81, label %if.end89

if.then81:                                        ; preds = %if.end79
  %28 = tail call float @llvm.copysign.f32(float 0.000000e+00, float %1)
  %29 = insertelement <2 x float> %z.coerce, float %28, i64 1
  %30 = fmul <2 x float> %29, %z.coerce
  br label %cleanup

if.end89:                                         ; preds = %if.end79
  br i1 %cmp, label %if.then91, label %if.end100

if.then91:                                        ; preds = %if.end89
  %sub93 = fsub float %0, %0
  %mul95 = fmul float %1, %sub93
  %retval.sroa.0.0.vec.insert226 = insertelement <2 x float> poison, float %sub93, i64 0
  %retval.sroa.0.4.vec.insert246 = insertelement <2 x float> %retval.sroa.0.0.vec.insert226, float %mul95, i64 1
  br label %cleanup

if.end100:                                        ; preds = %if.end89
  %cmp101 = icmp eq i32 %and, 2139095040
  br i1 %cmp101, label %if.then102, label %if.end120

if.then102:                                       ; preds = %if.end100
  %cmp103 = icmp sgt i32 %and9, 2139095039
  br i1 %cmp103, label %if.then104, label %if.end112

if.then104:                                       ; preds = %if.then102
  %sub106 = fsub float %0, %0
  %mul107 = fmul float %1, %sub106
  %retval.sroa.0.4.vec.insert248 = insertelement <2 x float> <float 0x7FF0000000000000, float poison>, float %mul107, i64 1
  br label %cleanup

if.end112:                                        ; preds = %if.then102
  call void @sincosf(float noundef %0, ptr noundef nonnull %s, ptr noundef nonnull %c) #5
  %31 = load float, ptr %c, align 4, !tbaa !5
  %mul114 = fmul float %31, 0x7FF0000000000000
  %32 = load float, ptr %s, align 4, !tbaa !5
  %mul115 = fmul float %1, %32
  %retval.sroa.0.0.vec.insert230 = insertelement <2 x float> poison, float %mul114, i64 0
  %retval.sroa.0.4.vec.insert250 = insertelement <2 x float> %retval.sroa.0.0.vec.insert230, float %mul115, i64 1
  br label %cleanup

if.end120:                                        ; preds = %if.end100
  %conv = fpext float %1 to x86_fp80
  %mul123 = fmul x86_fp80 %conv, %conv
  %sub124 = fsub float %0, %0
  %conv125 = fpext float %sub124 to x86_fp80
  %mul126 = fmul x86_fp80 %mul123, %conv125
  %conv127 = fptrunc x86_fp80 %mul126 to float
  %add = fadd x86_fp80 %conv, %conv
  %mul132 = fmul x86_fp80 %add, %conv125
  %conv133 = fptrunc x86_fp80 %mul132 to float
  %retval.sroa.0.0.vec.insert232 = insertelement <2 x float> poison, float %conv127, i64 0
  %retval.sroa.0.4.vec.insert252 = insertelement <2 x float> %retval.sroa.0.0.vec.insert232, float %conv133, i64 1
  br label %cleanup

cleanup:                                          ; preds = %if.end120, %if.end112, %if.then104, %if.then91, %if.then81, %if.then72, %if.else60, %if.then38, %if.then28, %if.then16, %if.then12
  %retval.sroa.0.0 = phi <2 x float> [ %retval.sroa.0.4.vec.insert, %if.then12 ], [ %10, %if.then16 ], [ %18, %if.then28 ], [ %retval.sroa.0.4.vec.insert238, %if.then38 ], [ %26, %if.else60 ], [ %retval.sroa.0.4.vec.insert242, %if.then72 ], [ %30, %if.then81 ], [ %retval.sroa.0.4.vec.insert246, %if.then91 ], [ %retval.sroa.0.4.vec.insert248, %if.then104 ], [ %retval.sroa.0.4.vec.insert250, %if.end112 ], [ %retval.sroa.0.4.vec.insert252, %if.end120 ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %s) #5
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %c) #5
  ret <2 x float> %retval.sroa.0.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare float @coshf(float noundef) local_unnamed_addr #2

declare void @sincosf(float noundef, ptr noundef, ptr noundef) local_unnamed_addr #3

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare float @sinhf(float noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare float @expf(float noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fabs.f32(float) #4

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.copysign.f32(float, float) #4

declare <2 x float> @__ldexp_cexpf(<2 x float> noundef, i32 noundef) local_unnamed_addr #3

; Function Attrs: nounwind uwtable
define dso_local <2 x float> @ref_ccosf(<2 x float> noundef %z.coerce) local_unnamed_addr #0 {
entry:
  %z.sroa.0.4.vec.extract = extractelement <2 x float> %z.coerce, i64 1
  %fneg = fneg float %z.sroa.0.4.vec.extract
  %coerce.sroa.0.0.vec.insert = insertelement <2 x float> poison, float %fneg, i64 0
  %coerce.sroa.0.4.vec.insert = shufflevector <2 x float> %coerce.sroa.0.0.vec.insert, <2 x float> %z.coerce, <2 x i32> <i32 0, i32 2>
  %call = tail call <2 x float> @ref_ccoshf(<2 x float> noundef %coerce.sroa.0.4.vec.insert)
  ret <2 x float> %call
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
