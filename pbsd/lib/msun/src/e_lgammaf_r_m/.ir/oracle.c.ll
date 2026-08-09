; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/e_lgammaf_r_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/e_lgammaf_r_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@vzero = internal constant float 0.000000e+00, align 4

; Function Attrs: nounwind uwtable
define dso_local float @ref_lgammaf_r(float noundef %x, ptr nocapture noundef writeonly %signgamp) local_unnamed_addr #0 {
entry:
  %vz.i = alloca float, align 4
  %0 = bitcast float %x to i32
  store i32 1, ptr %signgamp, align 4, !tbaa !5
  %1 = tail call float @llvm.fabs.f32(float %x)
  %and = bitcast float %1 to i32
  %cmp = icmp sgt i32 %and, 2139095039
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %mul = fmul float %x, %x
  br label %cleanup

if.end:                                           ; preds = %entry
  %2 = lshr i32 %0, 30
  %mul1 = and i32 %2, 2
  %sub = sub nsw i32 1, %mul1
  store i32 %sub, ptr %signgamp, align 4, !tbaa !5
  %cmp2 = icmp slt i32 %and, 838860800
  br i1 %cmp2, label %if.then3, label %if.end7

if.then3:                                         ; preds = %if.end
  %cmp4 = icmp eq i32 %and, 0
  br i1 %cmp4, label %if.then5, label %if.end6

if.then5:                                         ; preds = %if.then3
  %3 = load volatile float, ptr @vzero, align 4, !tbaa !9
  %div = fdiv float 1.000000e+00, %3
  br label %cleanup

if.end6:                                          ; preds = %if.then3
  %call = tail call float @logf(float noundef %1) #5
  %fneg = fneg float %call
  br label %cleanup

if.end7:                                          ; preds = %if.end
  %cmp8 = icmp slt i32 %0, 0
  br i1 %cmp8, label %if.then9, label %if.end26

if.then9:                                         ; preds = %if.end7
  store i32 1, ptr %signgamp, align 4, !tbaa !5
  %cmp10 = icmp ugt i32 %and, 1258291199
  br i1 %cmp10, label %if.then11, label %if.end13

if.then11:                                        ; preds = %if.then9
  %4 = load volatile float, ptr @vzero, align 4, !tbaa !9
  %div12 = fdiv float 1.000000e+00, %4
  br label %cleanup

if.end13:                                         ; preds = %if.then9
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %vz.i)
  %fneg.i = fneg float %x
  %add.i = fsub float 0x4160000000000000, %x
  store volatile float %add.i, ptr %vz.i, align 4, !tbaa !9
  %vz.i.0.vz.i.0.vz.i.0.vz.0.vz.0.vz.0..i = load volatile float, ptr %vz.i, align 4, !tbaa !9
  %sub.i = fadd float %vz.i.0.vz.i.0.vz.i.0.vz.0.vz.0.vz.0..i, 0xC160000000000000
  %cmp.i = fcmp oeq float %sub.i, %fneg.i
  br i1 %cmp.i, label %ref_sin_pif.exit.thread, label %if.end.i

ref_sin_pif.exit.thread:                          ; preds = %if.end13
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %vz.i)
  br label %if.then16

if.end.i:                                         ; preds = %if.end13
  %add1.i = fsub float 0x4140000000000000, %x
  store volatile float %add1.i, ptr %vz.i, align 4, !tbaa !9
  %vz.i.0.vz.i.0.vz.i.0.vz.0.vz.0.vz.0.49.i = load volatile float, ptr %vz.i, align 4, !tbaa !9
  %5 = bitcast float %vz.i.0.vz.i.0.vz.i.0.vz.0.vz.0.vz.0.49.i to i32
  %vz.i.0.vz.i.0.vz.i.0.vz.0.vz.0.vz.0.50.i = load volatile float, ptr %vz.i, align 4, !tbaa !9
  %sub2.i = fadd float %vz.i.0.vz.i.0.vz.i.0.vz.0.vz.0.vz.0.50.i, 0xC140000000000000
  %cmp3.i = fcmp ogt float %sub2.i, %fneg.i
  %sub5.i = fadd float %sub2.i, -2.500000e-01
  %z.0.i = select i1 %cmp3.i, float %sub5.i, float %sub2.i
  %dec.i = sext i1 %cmp3.i to i32
  %n.0.i = add nsw i32 %dec.i, %5
  %and.i = and i32 %n.0.i, 7
  %sub7.i = fsub float %fneg.i, %z.0.i
  %conv.i = sitofp i32 %and.i to float
  %6 = tail call float @llvm.fmuladd.f32(float %conv.i, float 2.500000e-01, float %sub7.i)
  switch i32 %and.i, label %sw.default.i [
    i32 0, label %sw.bb.i
    i32 1, label %sw.bb9.i
    i32 2, label %sw.bb9.i
    i32 3, label %sw.bb14.i
    i32 4, label %sw.bb14.i
    i32 5, label %sw.bb19.i
    i32 6, label %sw.bb19.i
  ]

sw.bb.i:                                          ; preds = %if.end.i
  %mul.i = fmul float %6, 0x400921FB60000000
  %conv8.i = fpext float %mul.i to double
  %call.i = tail call float @__kernel_sindf(double noundef %conv8.i) #5
  br label %ref_sin_pif.exit

sw.bb9.i:                                         ; preds = %if.end.i, %if.end.i
  %sub10.i = fsub float 5.000000e-01, %6
  %mul11.i = fmul float %sub10.i, 0x400921FB60000000
  %conv12.i = fpext float %mul11.i to double
  %call13.i = tail call float @__kernel_cosdf(double noundef %conv12.i) #5
  br label %ref_sin_pif.exit

sw.bb14.i:                                        ; preds = %if.end.i, %if.end.i
  %sub15.i = fsub float 1.000000e+00, %6
  %mul16.i = fmul float %sub15.i, 0x400921FB60000000
  %conv17.i = fpext float %mul16.i to double
  %call18.i = tail call float @__kernel_sindf(double noundef %conv17.i) #5
  br label %ref_sin_pif.exit

sw.bb19.i:                                        ; preds = %if.end.i, %if.end.i
  %sub20.i = fadd float %6, -1.500000e+00
  %mul21.i = fmul float %sub20.i, 0x400921FB60000000
  %conv22.i = fpext float %mul21.i to double
  %call23.i = tail call float @__kernel_cosdf(double noundef %conv22.i) #5
  %fneg24.i = fneg float %call23.i
  br label %ref_sin_pif.exit

sw.default.i:                                     ; preds = %if.end.i
  %sub25.i = fadd float %6, -2.000000e+00
  %mul26.i = fmul float %sub25.i, 0x400921FB60000000
  %conv27.i = fpext float %mul26.i to double
  %call28.i = tail call float @__kernel_sindf(double noundef %conv27.i) #5
  br label %ref_sin_pif.exit

ref_sin_pif.exit:                                 ; preds = %sw.bb.i, %sw.bb9.i, %sw.bb14.i, %sw.bb19.i, %sw.default.i
  %y.0.i = phi float [ %call28.i, %sw.default.i ], [ %fneg24.i, %sw.bb19.i ], [ %call18.i, %sw.bb14.i ], [ %call13.i, %sw.bb9.i ], [ %call.i, %sw.bb.i ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %vz.i)
  %cmp15 = fcmp oeq float %y.0.i, 0.000000e+00
  br i1 %cmp15, label %if.then16, label %if.end18

if.then16:                                        ; preds = %ref_sin_pif.exit.thread, %ref_sin_pif.exit
  %7 = load volatile float, ptr @vzero, align 4, !tbaa !9
  %div17 = fdiv float 1.000000e+00, %7
  br label %cleanup

if.end18:                                         ; preds = %ref_sin_pif.exit
  %fneg29.i = fneg float %y.0.i
  %mul19 = fmul float %fneg29.i, %x
  %8 = tail call float @llvm.fabs.f32(float %mul19)
  %div20 = fdiv float 0x400921FB60000000, %8
  %call21 = tail call float @logf(float noundef %div20) #5
  %cmp22 = fcmp ogt float %y.0.i, 0.000000e+00
  br i1 %cmp22, label %if.then23, label %if.end26

if.then23:                                        ; preds = %if.end18
  store i32 -1, ptr %signgamp, align 4, !tbaa !5
  br label %if.end26

if.end26:                                         ; preds = %if.end18, %if.then23, %if.end7
  %nadj.0 = phi float [ undef, %if.end7 ], [ %call21, %if.then23 ], [ %call21, %if.end18 ]
  %x.addr.0 = phi float [ %x, %if.end7 ], [ %fneg.i, %if.then23 ], [ %fneg.i, %if.end18 ]
  switch i32 %and, label %if.else [
    i32 1073741824, label %if.end142
    i32 1065353216, label %if.end142
  ]

if.else:                                          ; preds = %if.end26
  %cmp30 = icmp ult i32 %and, 1073741824
  br i1 %cmp30, label %if.then31, label %if.else90

if.then31:                                        ; preds = %if.else
  %cmp32 = icmp ult i32 %and, 1063675495
  br i1 %cmp32, label %if.then33, label %if.else46

if.then33:                                        ; preds = %if.then31
  %call34 = tail call float @logf(float noundef %x.addr.0) #5
  %fneg35 = fneg float %call34
  %cmp36 = icmp ugt i32 %and, 1060850207
  br i1 %cmp36, label %sw.bb, label %if.else39

if.else39:                                        ; preds = %if.then33
  %cmp40 = icmp ugt i32 %and, 1047343879
  br i1 %cmp40, label %sw.bb68, label %sw.bb79

if.else46:                                        ; preds = %if.then31
  %cmp47 = icmp ugt i32 %and, 1071490583
  br i1 %cmp47, label %sw.bb, label %if.else50

if.else50:                                        ; preds = %if.else46
  %cmp51 = icmp ugt i32 %and, 1067296287
  br i1 %cmp51, label %sw.bb68, label %if.else54

if.else54:                                        ; preds = %if.else50
  %sub55 = fadd float %x.addr.0, -1.000000e+00
  br label %sw.bb79

sw.bb:                                            ; preds = %if.else46, %if.then33
  %r.0.ph = phi float [ %fneg35, %if.then33 ], [ 0.000000e+00, %if.else46 ]
  %.pn = phi float [ 1.000000e+00, %if.then33 ], [ 2.000000e+00, %if.else46 ]
  %y.0.ph = fsub float %.pn, %x.addr.0
  %mul59 = fmul float %y.0.ph, %y.0.ph
  %9 = tail call float @llvm.fmuladd.f32(float %mul59, float 0x3F7C99F360000000, float 0x3FB13DC000000000)
  %10 = tail call float @llvm.fmuladd.f32(float %mul59, float %9, float 0x3FB3C467E0000000)
  %11 = tail call float @llvm.fmuladd.f32(float %mul59, float 0x3F70DDB480000000, float 0x3F952289E0000000)
  %12 = tail call float @llvm.fmuladd.f32(float %mul59, float %11, float 0x3FD4A34D20000000)
  %mul64 = fmul float %mul59, %12
  %13 = tail call float @llvm.fmuladd.f32(float %y.0.ph, float %10, float %mul64)
  %div66 = fmul float %y.0.ph, 5.000000e-01
  %sub67 = fsub float %13, %div66
  %add = fadd float %r.0.ph, %sub67
  br label %if.end142

sw.bb68:                                          ; preds = %if.else50, %if.else39
  %.sink = phi float [ 0xBFDD8B6180000000, %if.else39 ], [ 0xBFF762D860000000, %if.else50 ]
  %r.0.ph266 = phi float [ %fneg35, %if.else39 ], [ 0.000000e+00, %if.else50 ]
  %sub42 = fadd float %x.addr.0, %.sink
  %14 = tail call float @llvm.fmuladd.f32(float %sub42, float 0xBE59557700000000, float 0xBDC02A96E0000000)
  %mul70 = fmul float %sub42, %sub42
  %15 = tail call float @llvm.fmuladd.f32(float %sub42, float 0xBF84424A20000000, float 0x3F93188F40000000)
  %16 = tail call float @llvm.fmuladd.f32(float %sub42, float %15, float 0xBFA0D11080000000)
  %17 = tail call float @llvm.fmuladd.f32(float %sub42, float %16, float 0x3FB089B620000000)
  %18 = tail call float @llvm.fmuladd.f32(float %sub42, float %17, float 0xBFC2E41AE0000000)
  %19 = tail call float @llvm.fmuladd.f32(float %sub42, float %18, float 0x3FDEF72D00000000)
  %20 = tail call float @llvm.fmuladd.f32(float %mul70, float %19, float %14)
  %add77 = fadd float %20, 0xBFBF19B9C0000000
  %add78 = fadd float %r.0.ph266, %add77
  br label %if.end142

sw.bb79:                                          ; preds = %if.else54, %if.else39
  %r.0 = phi float [ 0.000000e+00, %if.else54 ], [ %fneg35, %if.else39 ]
  %y.0 = phi float [ %sub55, %if.else54 ], [ %x.addr.0, %if.else39 ]
  %21 = tail call float @llvm.fmuladd.f32(float %y.0, float 0x3FDFB8B6C0000000, float 0x3FE793C800000000)
  %22 = tail call float @llvm.fmuladd.f32(float %y.0, float %21, float 0xBFB3C467E0000000)
  %mul82 = fmul float %y.0, %22
  %23 = tail call float @llvm.fmuladd.f32(float %y.0, float 0xBF8517EE20000000, float 0x3FCAF4E100000000)
  %24 = tail call float @llvm.fmuladd.f32(float %y.0, float %23, float 0x3FF1C0DB60000000)
  %25 = tail call float @llvm.fmuladd.f32(float %y.0, float %24, float 1.000000e+00)
  %div86 = fdiv float %mul82, %25
  %div87 = fmul float %y.0, 5.000000e-01
  %sub88 = fsub float %div86, %div87
  %add89 = fadd float %r.0, %sub88
  br label %if.end142

if.else90:                                        ; preds = %if.else
  %cmp91 = icmp ult i32 %and, 1090519040
  br i1 %cmp91, label %if.then92, label %if.else123

if.then92:                                        ; preds = %if.else90
  %conv = fptosi float %x.addr.0 to i32
  %conv93 = sitofp i32 %conv to float
  %sub94 = fsub float %x.addr.0, %conv93
  %26 = tail call float @llvm.fmuladd.f32(float %sub94, float 0x3F88737300000000, float 0x3FC248F120000000)
  %27 = tail call float @llvm.fmuladd.f32(float %sub94, float %26, float 0x3FD1477940000000)
  %28 = tail call float @llvm.fmuladd.f32(float %sub94, float %27, float 0xBFB3C467E0000000)
  %mul98 = fmul float %sub94, %28
  %29 = tail call float @llvm.fmuladd.f32(float %sub94, float 0x3F6EC672C0000000, float 0x3FBDB60660000000)
  %30 = tail call float @llvm.fmuladd.f32(float %sub94, float %29, float 0x3FE5BFB180000000)
  %31 = tail call float @llvm.fmuladd.f32(float %sub94, float %30, float 1.000000e+00)
  %div102 = fmul float %sub94, 5.000000e-01
  %div103 = fdiv float %mul98, %31
  %add104 = fadd float %div102, %div103
  switch i32 %conv, label %if.end142 [
    i32 7, label %sw.bb105
    i32 6, label %sw.bb108
    i32 5, label %sw.bb111
    i32 4, label %sw.bb114
    i32 3, label %sw.bb117
  ]

sw.bb105:                                         ; preds = %if.then92
  %add106 = fadd float %sub94, 6.000000e+00
  br label %sw.bb108

sw.bb108:                                         ; preds = %if.then92, %sw.bb105
  %z.0 = phi float [ 1.000000e+00, %if.then92 ], [ %add106, %sw.bb105 ]
  %add109 = fadd float %sub94, 5.000000e+00
  %mul110 = fmul float %add109, %z.0
  br label %sw.bb111

sw.bb111:                                         ; preds = %if.then92, %sw.bb108
  %z.1 = phi float [ 1.000000e+00, %if.then92 ], [ %mul110, %sw.bb108 ]
  %add112 = fadd float %sub94, 4.000000e+00
  %mul113 = fmul float %add112, %z.1
  br label %sw.bb114

sw.bb114:                                         ; preds = %if.then92, %sw.bb111
  %z.2 = phi float [ 1.000000e+00, %if.then92 ], [ %mul113, %sw.bb111 ]
  %add115 = fadd float %sub94, 3.000000e+00
  %mul116 = fmul float %add115, %z.2
  br label %sw.bb117

sw.bb117:                                         ; preds = %if.then92, %sw.bb114
  %z.3 = phi float [ 1.000000e+00, %if.then92 ], [ %mul116, %sw.bb114 ]
  %add118 = fadd float %sub94, 2.000000e+00
  %mul119 = fmul float %add118, %z.3
  %call120 = tail call float @logf(float noundef %mul119) #5
  %add121 = fadd float %add104, %call120
  br label %if.end142

if.else123:                                       ; preds = %if.else90
  %cmp124 = icmp ult i32 %and, 1291845632
  %call127 = tail call float @logf(float noundef %x.addr.0) #5
  br i1 %cmp124, label %if.then126, label %if.else135

if.then126:                                       ; preds = %if.else123
  %div128 = fdiv float 1.000000e+00, %x.addr.0
  %mul129 = fmul float %div128, %div128
  %32 = tail call float @llvm.fmuladd.f32(float %mul129, float 0xBF669ED8C0000000, float 0x3FB55553E0000000)
  %33 = tail call float @llvm.fmuladd.f32(float %div128, float %32, float 0x3FDACFE3A0000000)
  %sub132 = fadd float %x.addr.0, -5.000000e-01
  %sub133 = fadd float %call127, -1.000000e+00
  %34 = tail call float @llvm.fmuladd.f32(float %sub132, float %sub133, float %33)
  br label %if.end142

if.else135:                                       ; preds = %if.else123
  %sub137 = fadd float %call127, -1.000000e+00
  %mul138 = fmul float %x.addr.0, %sub137
  br label %if.end142

if.end142:                                        ; preds = %if.end26, %if.end26, %sw.bb79, %sw.bb68, %sw.bb, %if.then126, %if.else135, %sw.bb117, %if.then92
  %r.1 = phi float [ %add89, %sw.bb79 ], [ %add78, %sw.bb68 ], [ %add, %sw.bb ], [ %add104, %if.then92 ], [ %add121, %sw.bb117 ], [ %34, %if.then126 ], [ %mul138, %if.else135 ], [ 0.000000e+00, %if.end26 ], [ 0.000000e+00, %if.end26 ]
  %sub146 = fsub float %nadj.0, %r.1
  %r.2 = select i1 %cmp8, float %sub146, float %r.1
  br label %cleanup

cleanup:                                          ; preds = %if.end142, %if.then16, %if.then11, %if.end6, %if.then5, %if.then
  %retval.0 = phi float [ %mul, %if.then ], [ %div, %if.then5 ], [ %fneg, %if.end6 ], [ %div12, %if.then11 ], [ %div17, %if.then16 ], [ %r.2, %if.end142 ]
  ret float %retval.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare float @logf(float noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fabs.f32(float) #3

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fmuladd.f32(float, float, float) #3

declare float @__kernel_sindf(double noundef) local_unnamed_addr #4

declare float @__kernel_cosdf(double noundef) local_unnamed_addr #4

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { mustprogress nofree nounwind willreturn memory(write) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #4 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{!10, !10, i64 0}
!10 = !{!"float", !7, i64 0}
