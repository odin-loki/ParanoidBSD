; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/e_jnf_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/e_jnf_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@vzero = internal constant float 0.000000e+00, align 4

; Function Attrs: nounwind uwtable
define dso_local float @ref_jnf(i32 noundef %n, float noundef %x) local_unnamed_addr #0 {
entry:
  %0 = tail call float @llvm.fabs.f32(float %x)
  %and = bitcast float %0 to i32
  %cmp = icmp sgt i32 %and, 2139095040
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %add = fadd float %x, %x
  br label %cleanup

if.end:                                           ; preds = %entry
  %cmp1 = icmp slt i32 %n, 0
  %fneg = fneg float %x
  %hx.0.in = select i1 %cmp1, float %fneg, float %x
  %n.addr.0 = tail call i32 @llvm.abs.i32(i32 %n, i1 true)
  switch i32 %n.addr.0, label %if.end10 [
    i32 0, label %if.then5
    i32 1, label %if.then8
  ]

if.then5:                                         ; preds = %if.end
  %call = tail call float @j0f(float noundef %hx.0.in) #5
  br label %cleanup

if.then8:                                         ; preds = %if.end
  %call9 = tail call float @j1f(float noundef %hx.0.in) #5
  br label %cleanup

if.end10:                                         ; preds = %if.end
  %hx.0 = bitcast float %hx.0.in to i32
  %and11 = and i32 %n.addr.0, 1
  %isneg = icmp sgt i32 %hx.0, -1
  switch i32 %and, label %if.else [
    i32 2139095040, label %if.end128
    i32 0, label %if.end128
  ]

if.else:                                          ; preds = %if.end10
  %conv = sitofp i32 %n.addr.0 to float
  %cmp16 = fcmp ult float %0, %conv
  br i1 %cmp16, label %if.else25, label %for.body.preheader

for.body.preheader:                               ; preds = %if.else
  %call19 = tail call float @j0f(float noundef %0) #5
  %call20 = tail call float @j1f(float noundef %0) #5
  %1 = add nsw i32 %n.addr.0, -1
  %xtraiter = and i32 %1, 1
  %2 = icmp eq i32 %n.addr.0, 2
  br i1 %2, label %if.end128.loopexit.unr-lcssa, label %for.body.preheader.new

for.body.preheader.new:                           ; preds = %for.body.preheader
  %unroll_iter = and i32 %1, -2
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader.new
  %i.0252 = phi i32 [ 1, %for.body.preheader.new ], [ %inc.1, %for.body ]
  %b.0251 = phi float [ %call20, %for.body.preheader.new ], [ %4, %for.body ]
  %a.0250 = phi float [ %call19, %for.body.preheader.new ], [ %3, %for.body ]
  %niter = phi i32 [ 0, %for.body.preheader.new ], [ %niter.next.1, %for.body ]
  %add23 = shl nuw nsw i32 %i.0252, 1
  %conv24 = sitofp i32 %add23 to float
  %div = fdiv float %conv24, %0
  %neg = fneg float %a.0250
  %3 = tail call float @llvm.fmuladd.f32(float %b.0251, float %div, float %neg)
  %inc = shl nuw i32 %i.0252, 1
  %add23.1 = add i32 %inc, 2
  %conv24.1 = sitofp i32 %add23.1 to float
  %div.1 = fdiv float %conv24.1, %0
  %neg.1 = fneg float %b.0251
  %4 = tail call float @llvm.fmuladd.f32(float %3, float %div.1, float %neg.1)
  %inc.1 = add nuw nsw i32 %i.0252, 2
  %niter.next.1 = add i32 %niter, 2
  %niter.ncmp.1 = icmp eq i32 %niter.next.1, %unroll_iter
  br i1 %niter.ncmp.1, label %if.end128.loopexit.unr-lcssa.loopexit, label %for.body, !llvm.loop !5

if.else25:                                        ; preds = %if.else
  %cmp26 = icmp slt i32 %and, 813694976
  br i1 %cmp26, label %if.then28, label %if.else45

if.then28:                                        ; preds = %if.else25
  %cmp29 = icmp ugt i32 %n.addr.0, 33
  br i1 %cmp29, label %if.end128, label %for.body36.preheader

for.body36.preheader:                             ; preds = %if.then28
  %mul = fmul float %0, 5.000000e-01
  %5 = add nsw i32 %n.addr.0, -1
  %6 = add nsw i32 %n.addr.0, -2
  %xtraiter295 = and i32 %5, 3
  %7 = icmp ult i32 %6, 3
  br i1 %7, label %for.end42.unr-lcssa, label %for.body36.preheader.new

for.body36.preheader.new:                         ; preds = %for.body36.preheader
  %unroll_iter299 = and i32 %5, -4
  br label %for.body36

for.body36:                                       ; preds = %for.body36, %for.body36.preheader.new
  %i.1282 = phi i32 [ 2, %for.body36.preheader.new ], [ %inc41.3, %for.body36 ]
  %b.1281 = phi float [ %mul, %for.body36.preheader.new ], [ %mul39.3, %for.body36 ]
  %a.1280 = phi float [ 1.000000e+00, %for.body36.preheader.new ], [ %mul38.3, %for.body36 ]
  %niter300 = phi i32 [ 0, %for.body36.preheader.new ], [ %niter300.next.3, %for.body36 ]
  %conv37 = sitofp i32 %i.1282 to float
  %mul38 = fmul float %a.1280, %conv37
  %mul39 = fmul float %mul, %b.1281
  %inc41 = or disjoint i32 %i.1282, 1
  %conv37.1 = sitofp i32 %inc41 to float
  %mul38.1 = fmul float %mul38, %conv37.1
  %mul39.1 = fmul float %mul, %mul39
  %inc41.1 = add nuw i32 %i.1282, 2
  %conv37.2 = sitofp i32 %inc41.1 to float
  %mul38.2 = fmul float %mul38.1, %conv37.2
  %mul39.2 = fmul float %mul, %mul39.1
  %inc41.2 = add nuw i32 %i.1282, 3
  %conv37.3 = sitofp i32 %inc41.2 to float
  %mul38.3 = fmul float %mul38.2, %conv37.3
  %mul39.3 = fmul float %mul, %mul39.2
  %inc41.3 = add nuw i32 %i.1282, 4
  %niter300.next.3 = add i32 %niter300, 4
  %niter300.ncmp.3 = icmp eq i32 %niter300.next.3, %unroll_iter299
  br i1 %niter300.ncmp.3, label %for.end42.unr-lcssa, label %for.body36, !llvm.loop !7

for.end42.unr-lcssa:                              ; preds = %for.body36, %for.body36.preheader
  %mul38.lcssa.ph = phi float [ undef, %for.body36.preheader ], [ %mul38.3, %for.body36 ]
  %mul39.lcssa.ph = phi float [ undef, %for.body36.preheader ], [ %mul39.3, %for.body36 ]
  %i.1282.unr = phi i32 [ 2, %for.body36.preheader ], [ %inc41.3, %for.body36 ]
  %b.1281.unr = phi float [ %mul, %for.body36.preheader ], [ %mul39.3, %for.body36 ]
  %a.1280.unr = phi float [ 1.000000e+00, %for.body36.preheader ], [ %mul38.3, %for.body36 ]
  %lcmp.mod296.not = icmp eq i32 %xtraiter295, 0
  br i1 %lcmp.mod296.not, label %for.end42, label %for.body36.epil

for.body36.epil:                                  ; preds = %for.end42.unr-lcssa, %for.body36.epil
  %i.1282.epil = phi i32 [ %inc41.epil, %for.body36.epil ], [ %i.1282.unr, %for.end42.unr-lcssa ]
  %b.1281.epil = phi float [ %mul39.epil, %for.body36.epil ], [ %b.1281.unr, %for.end42.unr-lcssa ]
  %a.1280.epil = phi float [ %mul38.epil, %for.body36.epil ], [ %a.1280.unr, %for.end42.unr-lcssa ]
  %epil.iter = phi i32 [ %epil.iter.next, %for.body36.epil ], [ 0, %for.end42.unr-lcssa ]
  %conv37.epil = sitofp i32 %i.1282.epil to float
  %mul38.epil = fmul float %a.1280.epil, %conv37.epil
  %mul39.epil = fmul float %mul, %b.1281.epil
  %inc41.epil = add nuw i32 %i.1282.epil, 1
  %epil.iter.next = add i32 %epil.iter, 1
  %epil.iter.cmp.not = icmp eq i32 %epil.iter.next, %xtraiter295
  br i1 %epil.iter.cmp.not, label %for.end42, label %for.body36.epil, !llvm.loop !8

for.end42:                                        ; preds = %for.body36.epil, %for.end42.unr-lcssa
  %mul38.lcssa = phi float [ %mul38.lcssa.ph, %for.end42.unr-lcssa ], [ %mul38.epil, %for.body36.epil ]
  %mul39.lcssa = phi float [ %mul39.lcssa.ph, %for.end42.unr-lcssa ], [ %mul39.epil, %for.body36.epil ]
  %div43 = fdiv float %mul39.lcssa, %mul38.lcssa
  br label %if.end128

if.else45:                                        ; preds = %if.else25
  %add46 = shl nuw nsw i32 %n.addr.0, 1
  %conv47 = sitofp i32 %add46 to float
  %8 = insertelement <2 x float> <float 2.000000e+00, float poison>, float %conv47, i64 1
  %9 = insertelement <2 x float> poison, float %0, i64 0
  %10 = shufflevector <2 x float> %9, <2 x float> poison, <2 x i32> zeroinitializer
  %11 = fdiv <2 x float> %8, %10
  %12 = extractelement <2 x float> %11, i64 0
  %13 = extractelement <2 x float> %11, i64 1
  %add50 = fadd float %12, %13
  %14 = tail call float @llvm.fmuladd.f32(float %13, float %add50, float -1.000000e+00)
  %cmp52253 = fcmp olt float %14, 1.000000e+09
  br i1 %cmp52253, label %while.body, label %for.body64.preheader

while.body:                                       ; preds = %if.else45, %while.body
  %k.0257 = phi i32 [ %add54, %while.body ], [ 1, %if.else45 ]
  %q1.0256 = phi float [ %15, %while.body ], [ %14, %if.else45 ]
  %q0.0255 = phi float [ %q1.0256, %while.body ], [ %13, %if.else45 ]
  %z.0254 = phi float [ %add55, %while.body ], [ %add50, %if.else45 ]
  %add54 = add nuw nsw i32 %k.0257, 1
  %add55 = fadd float %12, %z.0254
  %neg57 = fneg float %q0.0255
  %15 = tail call float @llvm.fmuladd.f32(float %add55, float %q1.0256, float %neg57)
  %cmp52 = fcmp olt float %15, 1.000000e+09
  br i1 %cmp52, label %while.body, label %for.body64.preheader, !llvm.loop !10

for.body64.preheader:                             ; preds = %while.body, %if.else45
  %k.0.lcssa = phi i32 [ 1, %if.else45 ], [ %add54, %while.body ]
  %add59 = add nuw nsw i32 %k.0.lcssa, %n.addr.0
  %mul60 = shl nuw nsw i32 %add59, 1
  br label %for.body64

for.body64:                                       ; preds = %for.body64.preheader, %for.body64
  %t.0261 = phi float [ %div68, %for.body64 ], [ 0.000000e+00, %for.body64.preheader ]
  %i.2260 = phi i32 [ %sub70, %for.body64 ], [ %mul60, %for.body64.preheader ]
  %conv65 = sitofp i32 %i.2260 to float
  %div66 = fdiv float %conv65, %0
  %sub67 = fsub float %div66, %t.0261
  %div68 = fdiv float 1.000000e+00, %sub67
  %sub70 = add nsw i32 %i.2260, -2
  %cmp62.not = icmp slt i32 %sub70, %add46
  br i1 %cmp62.not, label %for.end71, label %for.body64, !llvm.loop !11

for.end71:                                        ; preds = %for.body64
  %mul74 = fmul float %12, %conv
  %16 = tail call float @llvm.fabs.f32(float %mul74)
  %call75 = tail call float @logf(float noundef %16) #5
  %mul76 = fmul float %call75, %conv
  %cmp77 = fcmp olt float %mul76, 0x40562E3000000000
  %cmp84272 = icmp ugt i32 %n.addr.0, 1
  br i1 %cmp77, label %if.then79, label %if.else93

if.then79:                                        ; preds = %for.end71
  br i1 %cmp84272, label %for.body86.preheader, label %if.end114

for.body86.preheader:                             ; preds = %if.then79
  %sub80 = add nsw i32 %n.addr.0, -1
  %add81 = shl nuw nsw i32 %sub80, 1
  %conv82 = sitofp i32 %add81 to float
  %smin = tail call i32 @llvm.smin.i32(i32 %sub80, i32 1)
  %17 = sub i32 %n.addr.0, %smin
  %18 = sub i32 %sub80, %smin
  %xtraiter293 = and i32 %17, 3
  %lcmp.mod294.not = icmp eq i32 %xtraiter293, 0
  br i1 %lcmp.mod294.not, label %for.body86.prol.loopexit, label %for.body86.prol

for.body86.prol:                                  ; preds = %for.body86.preheader, %for.body86.prol
  %i.3276.prol = phi i32 [ %dec.prol, %for.body86.prol ], [ %sub80, %for.body86.preheader ]
  %di.0275.prol = phi float [ %sub90.prol, %for.body86.prol ], [ %conv82, %for.body86.preheader ]
  %b.2274.prol = phi float [ %sub89.prol, %for.body86.prol ], [ 1.000000e+00, %for.body86.preheader ]
  %a.2273.prol = phi float [ %b.2274.prol, %for.body86.prol ], [ %div68, %for.body86.preheader ]
  %prol.iter = phi i32 [ %prol.iter.next, %for.body86.prol ], [ 0, %for.body86.preheader ]
  %mul87.prol = fmul float %b.2274.prol, %di.0275.prol
  %div88.prol = fdiv float %mul87.prol, %0
  %sub89.prol = fsub float %div88.prol, %a.2273.prol
  %sub90.prol = fadd float %di.0275.prol, -2.000000e+00
  %dec.prol = add nsw i32 %i.3276.prol, -1
  %prol.iter.next = add i32 %prol.iter, 1
  %prol.iter.cmp.not = icmp eq i32 %prol.iter.next, %xtraiter293
  br i1 %prol.iter.cmp.not, label %for.body86.prol.loopexit, label %for.body86.prol, !llvm.loop !12

for.body86.prol.loopexit:                         ; preds = %for.body86.prol, %for.body86.preheader
  %i.3276.unr = phi i32 [ %sub80, %for.body86.preheader ], [ %dec.prol, %for.body86.prol ]
  %di.0275.unr = phi float [ %conv82, %for.body86.preheader ], [ %sub90.prol, %for.body86.prol ]
  %b.2274.unr = phi float [ 1.000000e+00, %for.body86.preheader ], [ %sub89.prol, %for.body86.prol ]
  %a.2273.unr = phi float [ %div68, %for.body86.preheader ], [ %b.2274.prol, %for.body86.prol ]
  %b.2274.lcssa.unr = phi float [ undef, %for.body86.preheader ], [ %b.2274.prol, %for.body86.prol ]
  %sub89.lcssa.unr = phi float [ undef, %for.body86.preheader ], [ %sub89.prol, %for.body86.prol ]
  %19 = icmp ult i32 %18, 3
  br i1 %19, label %if.end114, label %for.body86

for.body86:                                       ; preds = %for.body86.prol.loopexit, %for.body86
  %i.3276 = phi i32 [ %dec.3, %for.body86 ], [ %i.3276.unr, %for.body86.prol.loopexit ]
  %di.0275 = phi float [ %sub90.3, %for.body86 ], [ %di.0275.unr, %for.body86.prol.loopexit ]
  %b.2274 = phi float [ %sub89.3, %for.body86 ], [ %b.2274.unr, %for.body86.prol.loopexit ]
  %a.2273 = phi float [ %sub89.2, %for.body86 ], [ %a.2273.unr, %for.body86.prol.loopexit ]
  %mul87 = fmul float %b.2274, %di.0275
  %div88 = fdiv float %mul87, %0
  %sub89 = fsub float %div88, %a.2273
  %sub90 = fadd float %di.0275, -2.000000e+00
  %mul87.1 = fmul float %sub89, %sub90
  %div88.1 = fdiv float %mul87.1, %0
  %sub89.1 = fsub float %div88.1, %b.2274
  %sub90.1 = fadd float %sub90, -2.000000e+00
  %mul87.2 = fmul float %sub89.1, %sub90.1
  %div88.2 = fdiv float %mul87.2, %0
  %sub89.2 = fsub float %div88.2, %sub89
  %sub90.2 = fadd float %sub90.1, -2.000000e+00
  %mul87.3 = fmul float %sub89.2, %sub90.2
  %div88.3 = fdiv float %mul87.3, %0
  %sub89.3 = fsub float %div88.3, %sub89.1
  %sub90.3 = fadd float %sub90.2, -2.000000e+00
  %dec.3 = add nsw i32 %i.3276, -4
  %cmp84.3 = icmp sgt i32 %i.3276, 4
  br i1 %cmp84.3, label %for.body86, label %if.end114, !llvm.loop !13

if.else93:                                        ; preds = %for.end71
  br i1 %cmp84272, label %for.body100.preheader, label %if.end114

for.body100.preheader:                            ; preds = %if.else93
  %sub94 = add nsw i32 %n.addr.0, -1
  %add95 = shl nuw nsw i32 %sub94, 1
  %conv96 = sitofp i32 %add95 to float
  %20 = insertelement <2 x float> poison, float %div68, i64 0
  %21 = shufflevector <2 x float> %20, <2 x float> poison, <2 x i32> zeroinitializer
  br label %for.body100

for.body100:                                      ; preds = %for.body100.preheader, %for.body100
  %i.4267 = phi i32 [ %dec112, %for.body100 ], [ %sub94, %for.body100.preheader ]
  %di.1266 = phi float [ %sub104, %for.body100 ], [ %conv96, %for.body100.preheader ]
  %b.3265 = phi float [ %b.4, %for.body100 ], [ 1.000000e+00, %for.body100.preheader ]
  %22 = phi <2 x float> [ %27, %for.body100 ], [ %21, %for.body100.preheader ]
  %mul101 = fmul float %b.3265, %di.1266
  %div102 = fdiv float %mul101, %0
  %23 = extractelement <2 x float> %22, i64 1
  %sub103 = fsub float %div102, %23
  %sub104 = fadd float %di.1266, -2.000000e+00
  %cmp105 = fcmp ogt float %sub103, 1.000000e+10
  %div108 = select i1 %cmp105, float %sub103, float 1.000000e+00
  %b.4 = select i1 %cmp105, float 1.000000e+00, float %sub103
  %24 = insertelement <2 x float> %22, float %b.3265, i64 1
  %25 = insertelement <2 x float> poison, float %div108, i64 0
  %26 = shufflevector <2 x float> %25, <2 x float> poison, <2 x i32> zeroinitializer
  %27 = fdiv <2 x float> %24, %26
  %dec112 = add nsw i32 %i.4267, -1
  %cmp98 = icmp sgt i32 %i.4267, 1
  br i1 %cmp98, label %for.body100, label %if.end114.loopexit289, !llvm.loop !14

if.end114.loopexit289:                            ; preds = %for.body100
  %28 = extractelement <2 x float> %27, i64 1
  %29 = extractelement <2 x float> %27, i64 0
  br label %if.end114

if.end114:                                        ; preds = %for.body86.prol.loopexit, %for.body86, %if.end114.loopexit289, %if.else93, %if.then79
  %a.5 = phi float [ %div68, %if.then79 ], [ %div68, %if.else93 ], [ %28, %if.end114.loopexit289 ], [ %b.2274.lcssa.unr, %for.body86.prol.loopexit ], [ %sub89.2, %for.body86 ]
  %b.5 = phi float [ 1.000000e+00, %if.then79 ], [ 1.000000e+00, %if.else93 ], [ %b.4, %if.end114.loopexit289 ], [ %sub89.lcssa.unr, %for.body86.prol.loopexit ], [ %sub89.3, %for.body86 ]
  %t.3 = phi float [ %div68, %if.then79 ], [ %div68, %if.else93 ], [ %29, %if.end114.loopexit289 ], [ %div68, %for.body86 ], [ %div68, %for.body86.prol.loopexit ]
  %call115 = tail call float @j0f(float noundef %0) #5
  %call116 = tail call float @j1f(float noundef %0) #5
  %30 = tail call float @llvm.fabs.f32(float %call115)
  %31 = tail call float @llvm.fabs.f32(float %call116)
  %cmp117 = fcmp ult float %30, %31
  %mul120 = fmul float %t.3, %call115
  %div121 = fdiv float %mul120, %b.5
  %mul123 = fmul float %t.3, %call116
  %div124 = fdiv float %mul123, %a.5
  %b.6 = select i1 %cmp117, float %div124, float %div121
  br label %if.end128

if.end128.loopexit.unr-lcssa.loopexit:            ; preds = %for.body
  %32 = shl nuw nsw i32 %inc.1, 1
  %33 = sitofp i32 %32 to float
  br label %if.end128.loopexit.unr-lcssa

if.end128.loopexit.unr-lcssa:                     ; preds = %if.end128.loopexit.unr-lcssa.loopexit, %for.body.preheader
  %.lcssa291.ph = phi float [ undef, %for.body.preheader ], [ %4, %if.end128.loopexit.unr-lcssa.loopexit ]
  %i.0252.unr = phi float [ 2.000000e+00, %for.body.preheader ], [ %33, %if.end128.loopexit.unr-lcssa.loopexit ]
  %b.0251.unr = phi float [ %call20, %for.body.preheader ], [ %4, %if.end128.loopexit.unr-lcssa.loopexit ]
  %a.0250.unr = phi float [ %call19, %for.body.preheader ], [ %3, %if.end128.loopexit.unr-lcssa.loopexit ]
  %lcmp.mod.not = icmp eq i32 %xtraiter, 0
  %div.epil = fdiv float %i.0252.unr, %0
  %neg.epil = fneg float %a.0250.unr
  %34 = tail call float @llvm.fmuladd.f32(float %b.0251.unr, float %div.epil, float %neg.epil)
  %.lcssa291 = select i1 %lcmp.mod.not, float %.lcssa291.ph, float %34
  br label %if.end128

if.end128:                                        ; preds = %if.end128.loopexit.unr-lcssa, %if.then28, %if.end10, %if.end10, %for.end42, %if.end114
  %b.7 = phi float [ %div43, %for.end42 ], [ %b.6, %if.end114 ], [ 0.000000e+00, %if.end10 ], [ 0.000000e+00, %if.end10 ], [ 0.000000e+00, %if.then28 ], [ %.lcssa291, %if.end128.loopexit.unr-lcssa ]
  %cmp129.not247 = icmp eq i32 %and11, 0
  %cmp129.not = or i1 %cmp129.not247, %isneg
  br i1 %cmp129.not, label %cleanup, label %if.then131

if.then131:                                       ; preds = %if.end128
  %fneg132 = fneg float %b.7
  br label %cleanup

cleanup:                                          ; preds = %if.end128, %if.then131, %if.then8, %if.then5, %if.then
  %retval.0 = phi float [ %add, %if.then ], [ %call, %if.then5 ], [ %call9, %if.then8 ], [ %fneg132, %if.then131 ], [ %b.7, %if.end128 ]
  ret float %retval.0
}

declare float @j0f(float noundef) local_unnamed_addr #1

declare float @j1f(float noundef) local_unnamed_addr #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fabs.f32(float) #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fmuladd.f32(float, float, float) #2

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare float @logf(float noundef) local_unnamed_addr #3

; Function Attrs: nounwind uwtable
define dso_local float @ref_ynf(i32 noundef %n, float noundef %x) local_unnamed_addr #0 {
entry:
  %0 = bitcast float %x to i32
  %1 = tail call float @llvm.fabs.f32(float %x)
  %and = bitcast float %1 to i32
  %cmp = icmp sgt i32 %and, 2139095040
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %add = fadd float %x, %x
  br label %cleanup

if.end:                                           ; preds = %entry
  %cmp1 = icmp eq i32 %and, 0
  br i1 %cmp1, label %if.then2, label %if.end3

if.then2:                                         ; preds = %if.end
  %2 = load volatile float, ptr @vzero, align 4, !tbaa !15
  %div = fdiv float -1.000000e+00, %2
  br label %cleanup

if.end3:                                          ; preds = %if.end
  %cmp4 = icmp slt i32 %0, 0
  br i1 %cmp4, label %if.then5, label %if.end7

if.then5:                                         ; preds = %if.end3
  %3 = load volatile float, ptr @vzero, align 4, !tbaa !15
  %4 = load volatile float, ptr @vzero, align 4, !tbaa !15
  %div6 = fdiv float %3, %4
  br label %cleanup

if.end7:                                          ; preds = %if.end3
  %cmp8 = icmp slt i32 %n, 0
  %sub = sub nsw i32 0, %n
  %and10 = shl nuw i32 %sub, 1
  %shl = and i32 %and10, 2
  %sub11 = sub nsw i32 1, %shl
  %sign.0 = select i1 %cmp8, i32 %sub11, i32 1
  %n.addr.0 = select i1 %cmp8, i32 %sub, i32 %n
  switch i32 %n.addr.0, label %if.end19 [
    i32 0, label %if.then14
    i32 1, label %if.then17
  ]

if.then14:                                        ; preds = %if.end7
  %call = tail call float @y0f(float noundef %x) #5
  br label %cleanup

if.then17:                                        ; preds = %if.end7
  %conv = sitofp i32 %sign.0 to float
  %call18 = tail call float @y1f(float noundef %x) #5
  %mul = fmul float %call18, %conv
  br label %cleanup

if.end19:                                         ; preds = %if.end7
  %cmp20 = icmp eq i32 %and, 2139095040
  br i1 %cmp20, label %cleanup, label %if.end23

if.end23:                                         ; preds = %if.end19
  %call24 = tail call float @y0f(float noundef %x) #5
  %call25 = tail call float @y1f(float noundef %x) #5
  %ib.076 = bitcast float %call25 to i32
  %cmp3077 = icmp sgt i32 %n.addr.0, 1
  %cmp3278 = icmp ne i32 %ib.076, -8388608
  %5 = select i1 %cmp3077, i1 %cmp3278, i1 false
  br i1 %5, label %for.body, label %for.end

for.body:                                         ; preds = %if.end23, %for.body
  %i.081 = phi i32 [ %inc, %for.body ], [ 1, %if.end23 ]
  %b.080 = phi float [ %6, %for.body ], [ %call25, %if.end23 ]
  %a.079 = phi float [ %b.080, %for.body ], [ %call24, %if.end23 ]
  %add34 = shl nuw nsw i32 %i.081, 1
  %conv35 = sitofp i32 %add34 to float
  %div36 = fdiv float %conv35, %x
  %neg = fneg float %a.079
  %6 = tail call float @llvm.fmuladd.f32(float %div36, float %b.080, float %neg)
  %inc = add nuw nsw i32 %i.081, 1
  %ib.0 = bitcast float %6 to i32
  %cmp30 = icmp slt i32 %inc, %n.addr.0
  %cmp32 = icmp ne i32 %ib.0, -8388608
  %7 = select i1 %cmp30, i1 %cmp32, i1 false
  br i1 %7, label %for.body, label %for.end, !llvm.loop !19

for.end:                                          ; preds = %for.body, %if.end23
  %b.0.lcssa = phi float [ %call25, %if.end23 ], [ %6, %for.body ]
  %cmp42 = icmp sgt i32 %sign.0, 0
  br i1 %cmp42, label %cleanup, label %if.else

if.else:                                          ; preds = %for.end
  %fneg = fneg float %b.0.lcssa
  br label %cleanup

cleanup:                                          ; preds = %for.end, %if.end19, %if.else, %if.then17, %if.then14, %if.then5, %if.then2, %if.then
  %retval.0 = phi float [ %add, %if.then ], [ %div, %if.then2 ], [ %div6, %if.then5 ], [ %call, %if.then14 ], [ %mul, %if.then17 ], [ %fneg, %if.else ], [ 0.000000e+00, %if.end19 ], [ %b.0.lcssa, %for.end ]
  ret float %retval.0
}

declare float @y0f(float noundef) local_unnamed_addr #1

declare float @y1f(float noundef) local_unnamed_addr #1

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.abs.i32(i32, i1 immarg) #4

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.smin.i32(i32, i32) #4

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #3 = { mustprogress nofree nounwind willreturn memory(write) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #5 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.mustprogress"}
!7 = distinct !{!7, !6}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.unroll.disable"}
!10 = distinct !{!10, !6}
!11 = distinct !{!11, !6}
!12 = distinct !{!12, !9}
!13 = distinct !{!13, !6}
!14 = distinct !{!14, !6}
!15 = !{!16, !16, i64 0}
!16 = !{!"float", !17, i64 0}
!17 = !{!"omnipotent char", !18, i64 0}
!18 = !{!"Simple C/C++ TBAA"}
!19 = distinct !{!19, !6}
