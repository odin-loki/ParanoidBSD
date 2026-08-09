; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/e_powf_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/e_powf_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@bp = internal unnamed_addr constant [2 x float] [float 1.000000e+00, float 1.500000e+00], align 4
@dp_l = internal unnamed_addr constant [2 x float] [float 0.000000e+00, float 0x3EBA39FB80000000], align 4
@dp_h = internal unnamed_addr constant [2 x float] [float 0.000000e+00, float 0x3FE2B80000000000], align 4

; Function Attrs: nounwind uwtable
define dso_local float @ref_powf(float noundef %x, float noundef %y) local_unnamed_addr #0 {
entry:
  %0 = bitcast float %x to i32
  %1 = bitcast float %y to i32
  %2 = tail call float @llvm.fabs.f32(float %x)
  %and = bitcast float %2 to i32
  %3 = tail call float @llvm.fabs.f32(float %y)
  %and5 = bitcast float %3 to i32
  %cmp = icmp eq i32 %and5, 0
  %cmp6 = icmp eq i32 %0, 1065353216
  %or.cond698 = or i1 %cmp6, %cmp
  br i1 %or.cond698, label %cleanup, label %if.end8

if.end8:                                          ; preds = %entry
  %cmp9 = icmp sgt i32 %and, 2139095040
  %cmp10 = icmp sgt i32 %and5, 2139095040
  %or.cond = or i1 %cmp9, %cmp10
  br i1 %or.cond, label %if.then11, label %if.end16

if.then11:                                        ; preds = %if.end8
  %conv = fpext float %x to x86_fp80
  %add = fadd x86_fp80 %conv, 0xK00000000000000000000
  %add12 = fadd float %y, 0.000000e+00
  %conv13 = fpext float %add12 to x86_fp80
  %add14 = fadd x86_fp80 %add, %conv13
  %conv15 = fptrunc x86_fp80 %add14 to float
  br label %cleanup

if.end16:                                         ; preds = %if.end8
  %cmp17 = icmp slt i32 %0, 0
  br i1 %cmp17, label %if.then19, label %if.end37

if.then19:                                        ; preds = %if.end16
  %cmp20 = icmp sgt i32 %and5, 1266679807
  br i1 %cmp20, label %if.end37, label %if.else

if.else:                                          ; preds = %if.then19
  %cmp23 = icmp sgt i32 %and5, 1065353215
  br i1 %cmp23, label %if.then25, label %if.end37

if.then25:                                        ; preds = %if.else
  %shr = lshr i32 %and5, 23
  %sub26 = sub nuw nsw i32 150, %shr
  %shr27 = lshr i32 %and5, %sub26
  %shl = shl i32 %shr27, %sub26
  %cmp29 = icmp eq i32 %shl, %and5
  br i1 %cmp29, label %if.then31, label %if.end37

if.then31:                                        ; preds = %if.then25
  %and32 = and i32 %shr27, 1
  %sub33 = sub nuw nsw i32 2, %and32
  br label %if.end37

if.end37:                                         ; preds = %if.then19, %if.then25, %if.then31, %if.else, %if.end16
  %yisint.0 = phi i32 [ %sub33, %if.then31 ], [ 0, %if.then25 ], [ 0, %if.else ], [ 0, %if.end16 ], [ 2, %if.then19 ]
  switch i32 %and5, label %if.end65 [
    i32 2139095040, label %if.then40
    i32 1065353216, label %if.then60
  ]

if.then40:                                        ; preds = %if.end37
  %cmp41 = icmp eq i32 %and, 1065353216
  br i1 %cmp41, label %cleanup, label %if.else44

if.else44:                                        ; preds = %if.then40
  %cmp45 = icmp sgt i32 %and, 1065353216
  br i1 %cmp45, label %if.then47, label %if.else50

if.then47:                                        ; preds = %if.else44
  %cmp48 = icmp sgt i32 %1, -1
  %cond = select i1 %cmp48, float %y, float 0.000000e+00
  br label %cleanup

if.else50:                                        ; preds = %if.else44
  %cmp51 = icmp slt i32 %1, 0
  %fneg = fneg float %y
  %cond56 = select i1 %cmp51, float %fneg, float 0.000000e+00
  br label %cleanup

if.then60:                                        ; preds = %if.end37
  %cmp61 = icmp slt i32 %1, 0
  br i1 %cmp61, label %if.then63, label %cleanup

if.then63:                                        ; preds = %if.then60
  %div = fdiv float 1.000000e+00, %x
  br label %cleanup

if.end65:                                         ; preds = %if.end37
  %cmp66 = icmp eq i32 %1, 1073741824
  br i1 %cmp66, label %if.then68, label %if.end69

if.then68:                                        ; preds = %if.end65
  %mul = fmul float %x, %x
  br label %cleanup

if.end69:                                         ; preds = %if.end65
  %cmp70 = icmp eq i32 %1, 1056964608
  %cmp73 = icmp sgt i32 %0, -1
  %or.cond476 = and i1 %cmp73, %cmp70
  br i1 %or.cond476, label %if.then75, label %if.end77

if.then75:                                        ; preds = %if.end69
  %call = tail call float @sqrtf(float noundef %x) #4
  br label %cleanup

if.end77:                                         ; preds = %if.end69
  switch i32 %and, label %if.end110 [
    i32 2139095040, label %if.then86
    i32 1065353216, label %if.then86
    i32 0, label %if.then86
  ]

if.then86:                                        ; preds = %if.end77, %if.end77, %if.end77
  %cmp87 = icmp slt i32 %1, 0
  %div90 = fdiv float 1.000000e+00, %2
  %z.0 = select i1 %cmp87, float %div90, float %2
  br i1 %cmp17, label %if.then94, label %cleanup

if.then94:                                        ; preds = %if.then86
  %sub95 = add nsw i32 %and, -1065353216
  %or = or i32 %yisint.0, %sub95
  %cmp96 = icmp eq i32 %or, 0
  br i1 %cmp96, label %if.then98, label %if.else102

if.then98:                                        ; preds = %if.then94
  %sub99 = fsub float %z.0, %z.0
  %div101 = fdiv float %sub99, %sub99
  br label %cleanup

if.else102:                                       ; preds = %if.then94
  %cmp103 = icmp eq i32 %yisint.0, 1
  br i1 %cmp103, label %if.then105, label %cleanup

if.then105:                                       ; preds = %if.else102
  %fneg106 = fneg float %z.0
  br label %cleanup

if.end110:                                        ; preds = %if.end77
  %shr111 = lshr i32 %0, 31
  %sub112 = add nsw i32 %shr111, -1
  %or113 = or i32 %yisint.0, %sub112
  %cmp114 = icmp eq i32 %or113, 0
  br i1 %cmp114, label %if.then116, label %if.end120

if.then116:                                       ; preds = %if.end110
  %sub117 = fsub float %x, %x
  %div119 = fdiv float %sub117, %sub117
  br label %cleanup

if.end120:                                        ; preds = %if.end110
  %sub121 = add nsw i32 %yisint.0, -1
  %or122 = or i32 %sub121, %sub112
  %cmp123 = icmp eq i32 %or122, 0
  %spec.store.select = select i1 %cmp123, float -1.000000e+00, float 1.000000e+00
  %cmp127 = icmp sgt i32 %and5, 1291845632
  br i1 %cmp127, label %if.then129, label %if.else179

if.then129:                                       ; preds = %if.end120
  %cmp130 = icmp slt i32 %and, 1065353206
  br i1 %cmp130, label %if.then132, label %if.end143

if.then132:                                       ; preds = %if.then129
  %cmp133 = icmp slt i32 %1, 0
  %mul136 = fmul float %spec.store.select, 0x46293E5940000000
  %mul137 = fmul float %mul136, 0x46293E5940000000
  %mul139 = fmul float %spec.store.select, 0x39B4484C00000000
  %mul140 = fmul float %mul139, 0x39B4484C00000000
  %cond142 = select i1 %cmp133, float %mul137, float %mul140
  br label %cleanup

if.end143:                                        ; preds = %if.then129
  %cmp144 = icmp ugt i32 %and, 1065353223
  br i1 %cmp144, label %if.then146, label %if.end157

if.then146:                                       ; preds = %if.end143
  %cmp147 = icmp sgt i32 %1, 0
  %mul150 = fmul float %spec.store.select, 0x46293E5940000000
  %mul151 = fmul float %mul150, 0x46293E5940000000
  %mul153 = fmul float %spec.store.select, 0x39B4484C00000000
  %mul154 = fmul float %mul153, 0x39B4484C00000000
  %cond156 = select i1 %cmp147, float %mul151, float %mul154
  br label %cleanup

if.end157:                                        ; preds = %if.end143
  %sub158 = fadd float %2, -1.000000e+00
  %mul159 = fmul float %sub158, %sub158
  %neg = fneg float %sub158
  %4 = tail call float @llvm.fmuladd.f32(float %neg, float 2.500000e-01, float 0x3FD5555560000000)
  %5 = tail call float @llvm.fmuladd.f32(float %neg, float %4, float 5.000000e-01)
  %mul163 = fmul float %mul159, %5
  %mul164 = fmul float %sub158, 0x3FF7154000000000
  %neg167 = fmul float %mul163, 0xBFF7154760000000
  %6 = tail call float @llvm.fmuladd.f32(float %sub158, float 0x3EDD94AE00000000, float %neg167)
  %add168 = fadd float %mul164, %6
  %7 = bitcast float %add168 to i32
  %and174 = and i32 %7, -4096
  %8 = bitcast i32 %and174 to float
  %sub177 = fsub float %8, %mul164
  %sub178 = fsub float %6, %sub177
  br label %do.body313

if.else179:                                       ; preds = %if.end120
  %cmp180 = icmp slt i32 %and, 8388608
  %mul183 = fmul float %2, 0x4170000000000000
  %9 = bitcast float %mul183 to i32
  %ix.0 = select i1 %cmp180, i32 %9, i32 %and
  %shr190 = ashr i32 %ix.0, 23
  %sub191 = select i1 %cmp180, i32 -151, i32 -127
  %add192 = add nsw i32 %sub191, %shr190
  %and193 = and i32 %ix.0, 8388607
  %or194 = or disjoint i32 %and193, 1065353216
  %cmp195 = icmp ult i32 %and193, 1885298
  br i1 %cmp195, label %do.body207, label %if.else198

if.else198:                                       ; preds = %if.else179
  %cmp199 = icmp ult i32 %and193, 6140887
  br i1 %cmp199, label %do.body207, label %if.else202

if.else202:                                       ; preds = %if.else198
  %add203 = add nsw i32 %add192, 1
  %sub204 = or disjoint i32 %and193, 1056964608
  br label %do.body207

do.body207:                                       ; preds = %if.else198, %if.else179, %if.else202
  %ix.1 = phi i32 [ %sub204, %if.else202 ], [ %or194, %if.else179 ], [ %or194, %if.else198 ]
  %n.1 = phi i32 [ %add203, %if.else202 ], [ %add192, %if.else179 ], [ %add192, %if.else198 ]
  %k.0 = phi i32 [ 0, %if.else202 ], [ 0, %if.else179 ], [ 1, %if.else198 ]
  %10 = bitcast i32 %ix.1 to float
  %idxprom = zext nneg i32 %k.0 to i64
  %arrayidx = getelementptr inbounds [2 x float], ptr @bp, i64 0, i64 %idxprom
  %11 = load float, ptr %arrayidx, align 4, !tbaa !5
  %sub211 = fsub float %10, %11
  %add214 = fadd float %11, %10
  %div215 = fdiv float 1.000000e+00, %add214
  %mul216 = fmul float %sub211, %div215
  %12 = bitcast float %mul216 to i32
  %and223 = and i32 %12, -4096
  %13 = bitcast i32 %and223 to float
  %shr226 = lshr i32 %ix.1, 1
  %and227 = and i32 %shr226, 1610608640
  %add231 = add nuw nsw i32 %and227, 541065216
  %shl232 = shl nuw nsw i32 %k.0, 21
  %add233 = add nuw nsw i32 %add231, %shl232
  %14 = bitcast i32 %add233 to float
  %sub238 = fsub float %14, %11
  %sub239 = fsub float %10, %sub238
  %neg241 = fneg float %13
  %15 = tail call float @llvm.fmuladd.f32(float %neg241, float %14, float %sub211)
  %16 = tail call float @llvm.fmuladd.f32(float %neg241, float %sub239, float %15)
  %mul244 = fmul float %div215, %16
  %mul245 = fmul float %mul216, %mul216
  %mul246 = fmul float %mul245, %mul245
  %17 = tail call float @llvm.fmuladd.f32(float %mul245, float 0x3FCA7E2840000000, float 0x3FCD864AA0000000)
  %18 = tail call float @llvm.fmuladd.f32(float %mul245, float %17, float 0x3FD17460A0000000)
  %19 = tail call float @llvm.fmuladd.f32(float %mul245, float %18, float 0x3FD5555560000000)
  %20 = tail call float @llvm.fmuladd.f32(float %mul245, float %19, float 0x3FDB6DB6E0000000)
  %21 = tail call float @llvm.fmuladd.f32(float %mul245, float %20, float 0x3FE3333340000000)
  %mul252 = fmul float %mul246, %21
  %add253 = fadd float %mul216, %13
  %22 = tail call float @llvm.fmuladd.f32(float %mul244, float %add253, float %mul252)
  %mul255 = fmul float %13, %13
  %add256 = fadd float %mul255, 3.000000e+00
  %add257 = fadd float %add256, %22
  %23 = bitcast float %add257 to i32
  %and264 = and i32 %23, -4096
  %24 = bitcast i32 %and264 to float
  %sub267 = fadd float %24, -3.000000e+00
  %sub268 = fsub float %sub267, %mul255
  %sub269 = fsub float %22, %sub268
  %mul270 = fmul float %13, %24
  %mul272 = fmul float %mul216, %sub269
  %25 = tail call float @llvm.fmuladd.f32(float %mul244, float %24, float %mul272)
  %add273 = fadd float %mul270, %25
  %26 = bitcast float %add273 to i32
  %and280 = and i32 %26, -4096
  %27 = bitcast i32 %and280 to float
  %sub283 = fsub float %27, %mul270
  %sub284 = fsub float %25, %sub283
  %mul285 = fmul float %27, 0x3FEEC80000000000
  %mul287 = fmul float %sub284, 0x3FEEC709E0000000
  %28 = tail call float @llvm.fmuladd.f32(float %27, float 0xBF1EC478C0000000, float %mul287)
  %arrayidx289 = getelementptr inbounds [2 x float], ptr @dp_l, i64 0, i64 %idxprom
  %29 = load float, ptr %arrayidx289, align 4, !tbaa !5
  %add290 = fadd float %29, %28
  %conv291 = sitofp i32 %n.1 to float
  %add292 = fadd float %mul285, %add290
  %arrayidx294 = getelementptr inbounds [2 x float], ptr @dp_h, i64 0, i64 %idxprom
  %30 = load float, ptr %arrayidx294, align 4, !tbaa !5
  %add295 = fadd float %30, %add292
  %add296 = fadd float %add295, %conv291
  %31 = bitcast float %add296 to i32
  %and303 = and i32 %31, -4096
  %32 = bitcast i32 %and303 to float
  %sub306 = fsub float %32, %conv291
  %sub309 = fsub float %sub306, %30
  %sub310 = fsub float %sub309, %mul285
  %sub311 = fsub float %add290, %sub310
  br label %do.body313

do.body313:                                       ; preds = %if.end157, %do.body207
  %t2.0 = phi float [ %sub178, %if.end157 ], [ %sub311, %do.body207 ]
  %t1.0 = phi float [ %8, %if.end157 ], [ %32, %do.body207 ]
  %and319 = and i32 %1, -4096
  %33 = bitcast i32 %and319 to float
  %sub322 = fsub float %y, %33
  %mul324 = fmul float %t2.0, %y
  %34 = tail call float @llvm.fmuladd.f32(float %sub322, float %t1.0, float %mul324)
  %mul325 = fmul float %t1.0, %33
  %add326 = fadd float %34, %mul325
  %35 = bitcast float %add326 to i32
  %cmp331 = icmp sgt i32 %35, 1124073472
  br i1 %cmp331, label %if.then333, label %if.else336

if.then333:                                       ; preds = %do.body313
  %mul334 = fmul float %spec.store.select, 0x46293E5940000000
  %mul335 = fmul float %mul334, 0x46293E5940000000
  br label %cleanup

if.else336:                                       ; preds = %do.body313
  %cmp337 = icmp eq i32 %35, 1124073472
  br i1 %cmp337, label %if.then339, label %if.else348

if.then339:                                       ; preds = %if.else336
  %add340 = fadd float %34, 0x3E67154780000000
  %sub341 = fsub float %add326, %mul325
  %cmp342 = fcmp ogt float %add340, %sub341
  br i1 %cmp342, label %if.then344, label %if.then339.if.end369_crit_edge

if.then339.if.end369_crit_edge:                   ; preds = %if.then339
  %.pre700 = tail call float @llvm.fabs.f32(float %add326)
  %.pre702 = bitcast float %.pre700 to i32
  br label %if.end369

if.then344:                                       ; preds = %if.then339
  %mul345 = fmul float %spec.store.select, 0x46293E5940000000
  %mul346 = fmul float %mul345, 0x46293E5940000000
  br label %cleanup

if.else348:                                       ; preds = %if.else336
  %36 = tail call float @llvm.fabs.f32(float %add326)
  %and349 = bitcast float %36 to i32
  %cmp350 = icmp sgt i32 %and349, 1125515264
  br i1 %cmp350, label %if.then352, label %if.else355

if.then352:                                       ; preds = %if.else348
  %mul353 = fmul float %spec.store.select, 0x39B4484C00000000
  %mul354 = fmul float %mul353, 0x39B4484C00000000
  br label %cleanup

if.else355:                                       ; preds = %if.else348
  %cmp356 = icmp ne i32 %35, -1021968384
  %sub359 = fsub float %add326, %mul325
  %cmp360 = fcmp ugt float %34, %sub359
  %or.cond697 = or i1 %cmp356, %cmp360
  br i1 %or.cond697, label %if.end369, label %if.then362

if.then362:                                       ; preds = %if.else355
  %mul363 = fmul float %spec.store.select, 0x39B4484C00000000
  %mul364 = fmul float %mul363, 0x39B4484C00000000
  br label %cleanup

if.end369:                                        ; preds = %if.then339.if.end369_crit_edge, %if.else355
  %and370.pre-phi = phi i32 [ %.pre702, %if.then339.if.end369_crit_edge ], [ %and349, %if.else355 ]
  %cmp373 = icmp sgt i32 %and370.pre-phi, 1056964608
  br i1 %cmp373, label %if.then375, label %if.end398

if.then375:                                       ; preds = %if.end369
  %shr371 = lshr i32 %and370.pre-phi, 23
  %add376 = add nsw i32 %shr371, -126
  %shr377 = lshr i32 8388608, %add376
  %add378 = add nsw i32 %shr377, %35
  %and379 = lshr i32 %add378, 23
  %shr380 = and i32 %and379, 255
  %sub381 = add nsw i32 %shr380, -127
  %not = ashr i32 -8388608, %sub381
  %and385 = and i32 %not, %add378
  %37 = bitcast i32 %and385 to float
  %and388 = and i32 %add378, 8388607
  %or389 = or disjoint i32 %and388, 8388608
  %sub390 = sub nsw i32 150, %shr380
  %shr391 = lshr i32 %or389, %sub390
  %cmp392 = icmp slt i32 %35, 0
  %sub395 = sub nsw i32 0, %shr391
  %spec.select = select i1 %cmp392, i32 %sub395, i32 %shr391
  %sub397 = fsub float %mul325, %37
  %.pre = fadd float %34, %sub397
  %.pre699 = bitcast float %.pre to i32
  br label %if.end398

if.end398:                                        ; preds = %if.then375, %if.end369
  %.pre-phi = phi i32 [ %.pre699, %if.then375 ], [ %35, %if.end369 ]
  %n.3 = phi i32 [ %spec.select, %if.then375 ], [ 0, %if.end369 ]
  %p_h.0 = phi float [ %sub397, %if.then375 ], [ %mul325, %if.end369 ]
  %and406 = and i32 %.pre-phi, -32768
  %38 = bitcast i32 %and406 to float
  %mul409 = fmul float %38, 0x3FE62E4000000000
  %sub410 = fsub float %38, %p_h.0
  %sub411 = fsub float %34, %sub410
  %mul413 = fmul float %38, 0x3EB7F7D180000000
  %39 = tail call float @llvm.fmuladd.f32(float %sub411, float 0x3FE62E4300000000, float %mul413)
  %add414 = fadd float %mul409, %39
  %sub415 = fsub float %add414, %mul409
  %sub416 = fsub float %39, %sub415
  %mul417 = fmul float %add414, %add414
  %40 = tail call float @llvm.fmuladd.f32(float %mul417, float 0x3E66376980000000, float 0xBEBBBD41C0000000)
  %41 = tail call float @llvm.fmuladd.f32(float %mul417, float %40, float 0x3F11566AA0000000)
  %42 = tail call float @llvm.fmuladd.f32(float %mul417, float %41, float 0xBF66C16C20000000)
  %43 = tail call float @llvm.fmuladd.f32(float %mul417, float %42, float 0x3FC5555560000000)
  %neg423 = fneg float %mul417
  %44 = tail call float @llvm.fmuladd.f32(float %neg423, float %43, float %add414)
  %mul424 = fmul float %add414, %44
  %sub425 = fadd float %44, -2.000000e+00
  %div426 = fdiv float %mul424, %sub425
  %45 = tail call float @llvm.fmuladd.f32(float %add414, float %sub416, float %sub416)
  %sub428 = fsub float %div426, %45
  %46 = fsub float %add414, %sub428
  %sub430 = fadd float %46, 1.000000e+00
  %47 = bitcast float %sub430 to i32
  %shl435 = shl i32 %n.3, 23
  %add436 = add nsw i32 %shl435, %47
  %cmp438 = icmp slt i32 %add436, 8388608
  br i1 %cmp438, label %if.then440, label %do.body443

if.then440:                                       ; preds = %if.end398
  %call441 = tail call float @scalbnf(float noundef %sub430, i32 noundef %n.3) #4
  br label %if.end447

do.body443:                                       ; preds = %if.end398
  %48 = bitcast i32 %add436 to float
  br label %if.end447

if.end447:                                        ; preds = %do.body443, %if.then440
  %z.2 = phi float [ %call441, %if.then440 ], [ %48, %do.body443 ]
  %mul448 = fmul float %spec.store.select, %z.2
  br label %cleanup

cleanup:                                          ; preds = %if.then86, %if.else102, %if.then105, %if.then98, %if.then60, %if.then40, %entry, %if.end447, %if.then362, %if.then352, %if.then344, %if.then333, %if.then146, %if.then132, %if.then116, %if.then75, %if.then68, %if.then63, %if.else50, %if.then47, %if.then11
  %retval.0 = phi float [ %conv15, %if.then11 ], [ %cond, %if.then47 ], [ %cond56, %if.else50 ], [ %div, %if.then63 ], [ %mul, %if.then68 ], [ %call, %if.then75 ], [ %div119, %if.then116 ], [ %cond142, %if.then132 ], [ %cond156, %if.then146 ], [ %mul335, %if.then333 ], [ %mul346, %if.then344 ], [ %mul448, %if.end447 ], [ %mul354, %if.then352 ], [ %mul364, %if.then362 ], [ 1.000000e+00, %entry ], [ 1.000000e+00, %if.then40 ], [ %x, %if.then60 ], [ %div101, %if.then98 ], [ %fneg106, %if.then105 ], [ %z.0, %if.else102 ], [ %z.0, %if.then86 ]
  ret float %retval.0
}

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare float @sqrtf(float noundef) local_unnamed_addr #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fabs.f32(float) #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fmuladd.f32(float, float, float) #2

; Function Attrs: nounwind
declare float @scalbnf(float noundef, i32 noundef) local_unnamed_addr #3

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nofree nounwind willreturn memory(write) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #3 = { nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
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
