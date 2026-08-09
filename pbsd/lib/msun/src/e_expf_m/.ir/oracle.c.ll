; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/e_expf_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/e_expf_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@huge = internal global float 0x46293E5940000000, align 4
@twom100 = internal global float 0x39B0000000000000, align 4
@ln2HI = internal unnamed_addr constant [2 x float] [float 0x3FE62E4000000000, float 0xBFE62E4000000000], align 4
@ln2LO = internal unnamed_addr constant [2 x float] [float 0x3EB7F7D1C0000000, float 0xBEB7F7D1C0000000], align 4
@halF = internal unnamed_addr constant [2 x float] [float 5.000000e-01, float -5.000000e-01], align 4

; Function Attrs: mustprogress nofree norecurse nounwind willreturn memory(readwrite, argmem: none) uwtable
define dso_local float @ref_expf(float noundef %x) local_unnamed_addr #0 {
entry:
  %0 = bitcast float %x to i32
  %shr = lshr i32 %0, 31
  %1 = tail call float @llvm.fabs.f32(float %x)
  %and1 = bitcast float %1 to i32
  %cmp = icmp ugt i32 %and1, 1118925335
  br i1 %cmp, label %if.then, label %if.end18

if.then:                                          ; preds = %entry
  %cmp2 = icmp ugt i32 %and1, 2139095040
  br i1 %cmp2, label %if.then3, label %if.end

if.then3:                                         ; preds = %if.then
  %add = fadd float %x, %x
  br label %cleanup

if.end:                                           ; preds = %if.then
  %cmp4 = icmp eq i32 %and1, 2139095040
  br i1 %cmp4, label %if.then5, label %if.end8

if.then5:                                         ; preds = %if.end
  %cmp6 = icmp sgt i32 %0, -1
  %conv7 = select i1 %cmp6, float %x, float 0.000000e+00
  br label %cleanup

if.end8:                                          ; preds = %if.end
  %cmp9 = fcmp ogt float %x, 0x40562E3000000000
  br i1 %cmp9, label %if.then11, label %if.end12

if.then11:                                        ; preds = %if.end8
  %2 = load volatile float, ptr @huge, align 4, !tbaa !5
  %3 = load volatile float, ptr @huge, align 4, !tbaa !5
  %mul = fmul float %2, %3
  br label %cleanup

if.end12:                                         ; preds = %if.end8
  %cmp13 = fcmp olt float %x, 0xC059FE36A0000000
  br i1 %cmp13, label %if.then15, label %if.else

if.then15:                                        ; preds = %if.end12
  %4 = load volatile float, ptr @twom100, align 4, !tbaa !5
  %5 = load volatile float, ptr @twom100, align 4, !tbaa !5
  %mul16 = fmul float %4, %5
  br label %cleanup

if.end18:                                         ; preds = %entry
  %cmp19 = icmp ugt i32 %and1, 1051816472
  br i1 %cmp19, label %if.then21, label %if.else38

if.then21:                                        ; preds = %if.end18
  %cmp22 = icmp ult i32 %and1, 1065686418
  br i1 %cmp22, label %if.then24, label %if.else

if.then24:                                        ; preds = %if.then21
  %idxprom = zext nneg i32 %shr to i64
  %arrayidx = getelementptr inbounds [2 x float], ptr @ln2HI, i64 0, i64 %idxprom
  %6 = load float, ptr %arrayidx, align 4, !tbaa !5
  %sub = fsub float %x, %6
  %arrayidx26 = getelementptr inbounds [2 x float], ptr @ln2LO, i64 0, i64 %idxprom
  %7 = load float, ptr %arrayidx26, align 4, !tbaa !5
  %sub27 = xor i32 %shr, 1
  %sub28 = sub nsw i32 %sub27, %shr
  br label %if.end36

if.else:                                          ; preds = %if.end12, %if.then21
  %idxprom30 = zext nneg i32 %shr to i64
  %arrayidx31 = getelementptr inbounds [2 x float], ptr @halF, i64 0, i64 %idxprom30
  %8 = load float, ptr %arrayidx31, align 4, !tbaa !5
  %9 = tail call float @llvm.fmuladd.f32(float %x, float 0x3FF7154760000000, float %8)
  %conv32 = fptosi float %9 to i32
  %conv33 = sitofp i32 %conv32 to float
  %neg = fneg float %conv33
  %10 = tail call float @llvm.fmuladd.f32(float %neg, float 0x3FE62E4000000000, float %x)
  %mul35 = fmul float %conv33, 0x3EB7F7D1C0000000
  br label %if.end36

if.end36:                                         ; preds = %if.else, %if.then24
  %k.0 = phi i32 [ %sub28, %if.then24 ], [ %conv32, %if.else ]
  %lo.0 = phi float [ %7, %if.then24 ], [ %mul35, %if.else ]
  %hi.0 = phi float [ %sub, %if.then24 ], [ %10, %if.else ]
  %sub37 = fsub float %hi.0, %lo.0
  br label %if.end50

if.else38:                                        ; preds = %if.end18
  %cmp39 = icmp ult i32 %and1, 956301312
  br i1 %cmp39, label %if.then41, label %if.end50

if.then41:                                        ; preds = %if.else38
  %11 = load volatile float, ptr @huge, align 4, !tbaa !5
  %add42 = fadd float %11, %x
  %cmp43 = fcmp ogt float %add42, 1.000000e+00
  br i1 %cmp43, label %if.then45, label %if.end50

if.then45:                                        ; preds = %if.then41
  %add46 = fadd float %x, 1.000000e+00
  br label %cleanup

if.end50:                                         ; preds = %if.else38, %if.then41, %if.end36
  %k.1 = phi i32 [ %k.0, %if.end36 ], [ 0, %if.then41 ], [ 0, %if.else38 ]
  %lo.1 = phi float [ %lo.0, %if.end36 ], [ 0.000000e+00, %if.then41 ], [ 0.000000e+00, %if.else38 ]
  %hi.1 = phi float [ %hi.0, %if.end36 ], [ 0.000000e+00, %if.then41 ], [ 0.000000e+00, %if.else38 ]
  %x.addr.0 = phi float [ %sub37, %if.end36 ], [ %x, %if.then41 ], [ %x, %if.else38 ]
  %mul51 = fmul float %x.addr.0, %x.addr.0
  %cmp52 = icmp sgt i32 %k.1, -126
  %add56 = shl i32 %k.1, 23
  %twopk.0.in.v = select i1 %cmp52, i32 1065353216, i32 1904214016
  %twopk.0.in = add i32 %twopk.0.in.v, %add56
  %twopk.0 = bitcast i32 %twopk.0.in to float
  %12 = tail call float @llvm.fmuladd.f32(float %mul51, float 0xBF66AA42A0000000, float 0x3FC55551E0000000)
  %neg70 = fneg float %mul51
  %13 = tail call float @llvm.fmuladd.f32(float %neg70, float %12, float %x.addr.0)
  %cmp71 = icmp eq i32 %k.1, 0
  %mul74 = fmul float %x.addr.0, %13
  br i1 %cmp71, label %if.then73, label %if.else78

if.then73:                                        ; preds = %if.end50
  %sub75 = fadd float %13, -2.000000e+00
  %div = fdiv float %mul74, %sub75
  %14 = fsub float %x.addr.0, %div
  %sub77 = fadd float %14, 1.000000e+00
  br label %cleanup

if.else78:                                        ; preds = %if.end50
  %sub80 = fsub float 2.000000e+00, %13
  %div81 = fdiv float %mul74, %sub80
  %sub82 = fsub float %lo.1, %div81
  %15 = fsub float %hi.1, %sub82
  %sub84 = fadd float %15, 1.000000e+00
  br i1 %cmp52, label %if.then88, label %if.else96

if.then88:                                        ; preds = %if.else78
  %cmp89 = icmp eq i32 %k.1, 128
  br i1 %cmp89, label %if.then91, label %if.end94

if.then91:                                        ; preds = %if.then88
  %mul92 = fmul float %sub84, 2.000000e+00
  %mul93 = fmul float %mul92, 0x47E0000000000000
  br label %cleanup

if.end94:                                         ; preds = %if.then88
  %mul95 = fmul float %sub84, %twopk.0
  br label %cleanup

if.else96:                                        ; preds = %if.else78
  %mul97 = fmul float %sub84, %twopk.0
  %16 = load volatile float, ptr @twom100, align 4, !tbaa !5
  %mul98 = fmul float %mul97, %16
  br label %cleanup

cleanup:                                          ; preds = %if.else96, %if.end94, %if.then91, %if.then73, %if.then45, %if.then15, %if.then11, %if.then5, %if.then3
  %retval.0 = phi float [ %add, %if.then3 ], [ %conv7, %if.then5 ], [ %mul, %if.then11 ], [ %mul16, %if.then15 ], [ %sub77, %if.then73 ], [ %mul93, %if.then91 ], [ %mul95, %if.end94 ], [ %mul98, %if.else96 ], [ %add46, %if.then45 ]
  ret float %retval.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fmuladd.f32(float, float, float) #1

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fabs.f32(float) #2

attributes #0 = { mustprogress nofree norecurse nounwind willreturn memory(readwrite, argmem: none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

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
