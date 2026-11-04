; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/s_erff_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/s_erff_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@tiny = internal constant float 0x39B4484C00000000, align 4

; Function Attrs: mustprogress nofree nounwind willreturn memory(write, inaccessiblemem: readwrite) uwtable
define dso_local float @ref_erff(float noundef %x) local_unnamed_addr #0 {
entry:
  %0 = bitcast float %x to i32
  %1 = tail call float @llvm.fabs.f32(float %x)
  %and = bitcast float %1 to i32
  %cmp = icmp sgt i32 %and, 2139095039
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %2 = lshr i32 %0, 30
  %shl = and i32 %2, 2
  %sub = sub nsw i32 1, %shl
  %conv = sitofp i32 %sub to float
  %div = fdiv float 1.000000e+00, %x
  %add = fadd float %div, %conv
  br label %cleanup

if.end:                                           ; preds = %entry
  %cmp1 = icmp slt i32 %and, 1062731776
  br i1 %cmp1, label %if.then3, label %if.end21

if.then3:                                         ; preds = %if.end
  %cmp4 = icmp slt i32 %and, 947912704
  br i1 %cmp4, label %if.then6, label %if.end13

if.then6:                                         ; preds = %if.then3
  %cmp7 = icmp slt i32 %and, 67108864
  br i1 %cmp7, label %if.then9, label %if.end12

if.then9:                                         ; preds = %if.then6
  %mul10 = fmul float %x, 0x3FF06EBA80000000
  %3 = tail call float @llvm.fmuladd.f32(float %x, float 8.000000e+00, float %mul10)
  %div11 = fmul float %3, 1.250000e-01
  br label %cleanup

if.end12:                                         ; preds = %if.then6
  %4 = tail call float @llvm.fmuladd.f32(float %x, float 0x3FC06EBA80000000, float %x)
  br label %cleanup

if.end13:                                         ; preds = %if.then3
  %mul = fmul float %x, %x
  %5 = tail call float @llvm.fmuladd.f32(float %mul, float 0xBF604A6220000000, float 0x3F96202800000000)
  %6 = insertelement <2 x float> poison, float %mul, i64 0
  %7 = shufflevector <2 x float> %6, <2 x float> poison, <2 x i32> zeroinitializer
  %8 = insertelement <2 x float> <float 0xBF5E845E80000000, float poison>, float %5, i64 1
  %9 = tail call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %7, <2 x float> %8, <2 x float> <float 0xBFD58185A0000000, float 0x3FD3FD1F20000000>)
  %10 = tail call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %7, <2 x float> %9, <2 x float> <float 0x3FC06EBA80000000, float 1.000000e+00>)
  %11 = extractelement <2 x float> %10, i64 0
  %12 = extractelement <2 x float> %10, i64 1
  %div19 = fdiv float %11, %12
  %13 = tail call float @llvm.fmuladd.f32(float %x, float %div19, float %x)
  br label %cleanup

if.end21:                                         ; preds = %if.end
  %cmp22 = icmp ult i32 %and, 1067450368
  br i1 %cmp22, label %if.then24, label %if.end39

if.then24:                                        ; preds = %if.end21
  %sub25 = fadd float %1, -1.000000e+00
  %14 = tail call float @llvm.fmuladd.f32(float %sub25, float 0x3FB6366960000000, float 0xBFCACD7AA0000000)
  %15 = tail call float @llvm.fmuladd.f32(float %sub25, float %14, float 0x3FDA9126A0000000)
  %16 = tail call float @llvm.fmuladd.f32(float %sub25, float %15, float 0x3ECE9F3260000000)
  %17 = tail call float @llvm.fmuladd.f32(float %sub25, float 0x3FA4184CE0000000, float 0x3FD7C28920000000)
  %18 = tail call float @llvm.fmuladd.f32(float %sub25, float %17, float 0x3FDFB74560000000)
  %19 = tail call float @llvm.fmuladd.f32(float %sub25, float %18, float 1.000000e+00)
  %cmp32 = icmp sgt i32 %0, -1
  %div35 = fdiv float %16, %19
  br i1 %cmp32, label %if.then34, label %if.else

if.then34:                                        ; preds = %if.then24
  %add36 = fadd float %div35, 0x3FEAF76000000000
  br label %cleanup

if.else:                                          ; preds = %if.then24
  %sub38 = fsub float 0xBFEAF76000000000, %div35
  br label %cleanup

if.end39:                                         ; preds = %if.end21
  %cmp40 = icmp ugt i32 %and, 1082130431
  br i1 %cmp40, label %if.then42, label %if.end49

if.then42:                                        ; preds = %if.end39
  %cmp43 = icmp sgt i32 %0, -1
  %20 = load volatile float, ptr @tiny, align 4, !tbaa !5
  br i1 %cmp43, label %if.then45, label %if.else47

if.then45:                                        ; preds = %if.then42
  %sub46 = fsub float 1.000000e+00, %20
  br label %cleanup

if.else47:                                        ; preds = %if.then42
  %sub48 = fadd float %20, -1.000000e+00
  br label %cleanup

if.end49:                                         ; preds = %if.end39
  %mul50 = fmul float %x, %x
  %div51 = fdiv float 1.000000e+00, %mul50
  %cmp52 = icmp ult i32 %and, 1077336972
  br i1 %cmp52, label %if.then54, label %if.else61

if.then54:                                        ; preds = %if.end49
  %21 = tail call float @llvm.fmuladd.f32(float %div51, float 0xBFE6213640000000, float 0xBFFAB149A0000000)
  %22 = insertelement <2 x float> poison, float %div51, i64 0
  %23 = shufflevector <2 x float> %22, <2 x float> poison, <2 x i32> zeroinitializer
  %24 = insertelement <2 x float> <float 0x3FE1B92E80000000, float poison>, float %21, i64 1
  %25 = tail call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %23, <2 x float> %24, <2 x float> <float 0x40106E9560000000, float 0xBFE165A640000000>)
  %26 = tail call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %23, <2 x float> %25, <2 x float> <float 0x4011F179A0000000, float 0xBF843CC980000000>)
  br label %do.body68

if.else61:                                        ; preds = %if.end49
  %27 = insertelement <2 x float> poison, float %div51, i64 0
  %28 = shufflevector <2 x float> %27, <2 x float> poison, <2 x i32> zeroinitializer
  %29 = tail call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %28, <2 x float> <float 0xBFE8617D80000000, float 0xBFFD7560E0000000>, <2 x float> <float 0x4008660BC0000000, float 0xBFE1899FC0000000>)
  %30 = tail call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %28, <2 x float> %29, <2 x float> <float 0x40137C3D40000000, float 0xBF843415C0000000>)
  br label %do.body68

do.body68:                                        ; preds = %if.then54, %if.else61
  %31 = phi <2 x float> [ %26, %if.then54 ], [ %30, %if.else61 ]
  %32 = extractelement <2 x float> %31, i64 0
  %33 = tail call float @llvm.fmuladd.f32(float %div51, float %32, float 1.000000e+00)
  %and69 = and i32 %0, -8192
  %34 = bitcast i32 %and69 to float
  %fneg = fneg float %34
  %35 = tail call float @llvm.fmuladd.f32(float %fneg, float %34, float -5.625000e-01)
  %call = tail call float @expf(float noundef %35) #4
  %sub73 = fsub float %34, %1
  %add74 = fadd float %1, %34
  %36 = extractelement <2 x float> %31, i64 1
  %div76 = fdiv float %36, %33
  %37 = tail call float @llvm.fmuladd.f32(float %sub73, float %add74, float %div76)
  %call77 = tail call float @expf(float noundef %37) #4
  %mul78 = fmul float %call, %call77
  %cmp79 = icmp sgt i32 %0, -1
  %div82 = fdiv float %mul78, %1
  br i1 %cmp79, label %if.then81, label %if.else84

if.then81:                                        ; preds = %do.body68
  %sub83 = fsub float 1.000000e+00, %div82
  br label %cleanup

if.else84:                                        ; preds = %do.body68
  %sub86 = fadd float %div82, -1.000000e+00
  br label %cleanup

cleanup:                                          ; preds = %if.else84, %if.then81, %if.else47, %if.then45, %if.else, %if.then34, %if.end13, %if.end12, %if.then9, %if.then
  %retval.0 = phi float [ %add, %if.then ], [ %div11, %if.then9 ], [ %4, %if.end12 ], [ %13, %if.end13 ], [ %add36, %if.then34 ], [ %sub38, %if.else ], [ %sub46, %if.then45 ], [ %sub48, %if.else47 ], [ %sub83, %if.then81 ], [ %sub86, %if.else84 ]
  ret float %retval.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fmuladd.f32(float, float, float) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fabs.f32(float) #1

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare float @expf(float noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nofree nounwind willreturn memory(write, inaccessiblemem: readwrite) uwtable
define dso_local float @ref_erfcf(float noundef %x) local_unnamed_addr #0 {
entry:
  %0 = bitcast float %x to i32
  %1 = tail call float @llvm.fabs.f32(float %x)
  %and = bitcast float %1 to i32
  %cmp = icmp sgt i32 %and, 2139095039
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %2 = lshr i32 %0, 30
  %shl = and i32 %2, 2
  %conv = uitofp i32 %shl to float
  %div = fdiv float 1.000000e+00, %x
  %add = fadd float %div, %conv
  br label %cleanup

if.end:                                           ; preds = %entry
  %cmp1 = icmp slt i32 %and, 1062731776
  br i1 %cmp1, label %if.then3, label %if.end23

if.then3:                                         ; preds = %if.end
  %cmp4 = icmp slt i32 %and, 864026624
  br i1 %cmp4, label %if.then6, label %if.end7

if.then6:                                         ; preds = %if.then3
  %sub = fsub float 1.000000e+00, %x
  br label %cleanup

if.end7:                                          ; preds = %if.then3
  %mul = fmul float %x, %x
  %3 = tail call float @llvm.fmuladd.f32(float %mul, float 0xBF5E845E80000000, float 0xBFD58185A0000000)
  %4 = tail call float @llvm.fmuladd.f32(float %mul, float %3, float 0x3FC06EBA80000000)
  %5 = tail call float @llvm.fmuladd.f32(float %mul, float 0xBF604A6220000000, float 0x3F96202800000000)
  %6 = tail call float @llvm.fmuladd.f32(float %mul, float %5, float 0x3FD3FD1F20000000)
  %7 = tail call float @llvm.fmuladd.f32(float %mul, float %6, float 1.000000e+00)
  %div13 = fdiv float %4, %7
  %cmp14 = icmp slt i32 %0, 1048576000
  br i1 %cmp14, label %if.then16, label %if.else

if.then16:                                        ; preds = %if.end7
  %8 = tail call float @llvm.fmuladd.f32(float %x, float %div13, float %x)
  %sub18 = fsub float 1.000000e+00, %8
  br label %cleanup

if.else:                                          ; preds = %if.end7
  %mul19 = fmul float %div13, %x
  %sub20 = fadd float %x, -5.000000e-01
  %add21 = fadd float %sub20, %mul19
  %sub22 = fsub float 5.000000e-01, %add21
  br label %cleanup

if.end23:                                         ; preds = %if.end
  %cmp24 = icmp ult i32 %and, 1067450368
  br i1 %cmp24, label %if.then26, label %if.end43

if.then26:                                        ; preds = %if.end23
  %sub27 = fadd float %1, -1.000000e+00
  %9 = tail call float @llvm.fmuladd.f32(float %sub27, float 0x3FB6366960000000, float 0xBFCACD7AA0000000)
  %10 = tail call float @llvm.fmuladd.f32(float %sub27, float %9, float 0x3FDA9126A0000000)
  %11 = tail call float @llvm.fmuladd.f32(float %sub27, float %10, float 0x3ECE9F3260000000)
  %12 = tail call float @llvm.fmuladd.f32(float %sub27, float 0x3FA4184CE0000000, float 0x3FD7C28920000000)
  %13 = tail call float @llvm.fmuladd.f32(float %sub27, float %12, float 0x3FDFB74560000000)
  %14 = tail call float @llvm.fmuladd.f32(float %sub27, float %13, float 1.000000e+00)
  %cmp34 = icmp sgt i32 %0, -1
  %div37 = fdiv float %11, %14
  br i1 %cmp34, label %if.then36, label %if.else39

if.then36:                                        ; preds = %if.then26
  %sub38 = fsub float 0x3FC4228000000000, %div37
  br label %cleanup

if.else39:                                        ; preds = %if.then26
  %add41 = fadd float %div37, 0x3FEAF76000000000
  %add42 = fadd float %add41, 1.000000e+00
  br label %cleanup

if.end43:                                         ; preds = %if.end23
  %cmp44 = icmp ult i32 %and, 1093664768
  br i1 %cmp44, label %if.then46, label %if.else90

if.then46:                                        ; preds = %if.end43
  %mul47 = fmul float %x, %x
  %div48 = fdiv float 1.000000e+00, %mul47
  %cmp49 = icmp ult i32 %and, 1077336972
  br i1 %cmp49, label %if.then51, label %if.else58

if.then51:                                        ; preds = %if.then46
  %15 = tail call float @llvm.fmuladd.f32(float %div48, float 0xBFE6213640000000, float 0xBFFAB149A0000000)
  %16 = insertelement <2 x float> poison, float %div48, i64 0
  %17 = shufflevector <2 x float> %16, <2 x float> poison, <2 x i32> zeroinitializer
  %18 = insertelement <2 x float> <float 0x3FE1B92E80000000, float poison>, float %15, i64 1
  %19 = tail call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %17, <2 x float> %18, <2 x float> <float 0x40106E9560000000, float 0xBFE165A640000000>)
  %20 = tail call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %17, <2 x float> %19, <2 x float> <float 0x4011F179A0000000, float 0xBF843CC980000000>)
  br label %do.body72

if.else58:                                        ; preds = %if.then46
  %cmp59 = icmp slt i32 %0, 0
  %cmp61 = icmp ugt i32 %and, 1084227583
  %or.cond = and i1 %cmp59, %cmp61
  br i1 %or.cond, label %if.then63, label %if.end65

if.then63:                                        ; preds = %if.else58
  %21 = load volatile float, ptr @tiny, align 4, !tbaa !5
  %sub64 = fsub float 2.000000e+00, %21
  br label %cleanup

if.end65:                                         ; preds = %if.else58
  %22 = insertelement <2 x float> poison, float %div48, i64 0
  %23 = shufflevector <2 x float> %22, <2 x float> poison, <2 x i32> zeroinitializer
  %24 = tail call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %23, <2 x float> <float 0xBFE8617D80000000, float 0xBFFD7560E0000000>, <2 x float> <float 0x4008660BC0000000, float 0xBFE1899FC0000000>)
  %25 = tail call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %23, <2 x float> %24, <2 x float> <float 0x40137C3D40000000, float 0xBF843415C0000000>)
  br label %do.body72

do.body72:                                        ; preds = %if.then51, %if.end65
  %26 = phi <2 x float> [ %20, %if.then51 ], [ %25, %if.end65 ]
  %27 = extractelement <2 x float> %26, i64 0
  %28 = tail call float @llvm.fmuladd.f32(float %div48, float %27, float 1.000000e+00)
  %and73 = and i32 %0, -8192
  %29 = bitcast i32 %and73 to float
  %fneg = fneg float %29
  %30 = tail call float @llvm.fmuladd.f32(float %fneg, float %29, float -5.625000e-01)
  %call = tail call float @expf(float noundef %30) #4
  %sub77 = fsub float %29, %1
  %add78 = fadd float %1, %29
  %31 = extractelement <2 x float> %26, i64 1
  %div80 = fdiv float %31, %28
  %32 = tail call float @llvm.fmuladd.f32(float %sub77, float %add78, float %div80)
  %call81 = tail call float @expf(float noundef %32) #4
  %mul82 = fmul float %call, %call81
  %cmp83 = icmp sgt i32 %0, 0
  %div86 = fdiv float %mul82, %1
  br i1 %cmp83, label %cleanup, label %if.else87

if.else87:                                        ; preds = %do.body72
  %sub89 = fsub float 2.000000e+00, %div86
  br label %cleanup

if.else90:                                        ; preds = %if.end43
  %cmp91 = icmp sgt i32 %0, 0
  %33 = load volatile float, ptr @tiny, align 4, !tbaa !5
  br i1 %cmp91, label %if.then93, label %if.else95

if.then93:                                        ; preds = %if.else90
  %34 = load volatile float, ptr @tiny, align 4, !tbaa !5
  %mul94 = fmul float %33, %34
  br label %cleanup

if.else95:                                        ; preds = %if.else90
  %sub96 = fsub float 2.000000e+00, %33
  br label %cleanup

cleanup:                                          ; preds = %do.body72, %if.else95, %if.then93, %if.else87, %if.then63, %if.else39, %if.then36, %if.else, %if.then16, %if.then6, %if.then
  %retval.0 = phi float [ %add, %if.then ], [ %sub, %if.then6 ], [ %sub18, %if.then16 ], [ %sub22, %if.else ], [ %sub38, %if.then36 ], [ %add42, %if.else39 ], [ %sub89, %if.else87 ], [ %sub64, %if.then63 ], [ %mul94, %if.then93 ], [ %sub96, %if.else95 ], [ %div86, %do.body72 ]
  ret float %retval.0
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare <2 x float> @llvm.fmuladd.v2f32(<2 x float>, <2 x float>, <2 x float>) #3

attributes #0 = { mustprogress nofree nounwind willreturn memory(write, inaccessiblemem: readwrite) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { mustprogress nofree nounwind willreturn memory(write) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #4 = { nounwind }

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
