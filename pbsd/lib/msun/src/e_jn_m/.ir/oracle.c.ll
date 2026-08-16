; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/e_jn_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/e_jn_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@vzero = internal constant double 0.000000e+00, align 8

; Function Attrs: nounwind uwtable
define dso_local double @ref_jn(i32 noundef %n, double noundef %x) local_unnamed_addr #0 {
entry:
  %c = alloca double, align 8
  %s = alloca double, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %c) #6
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %s) #6
  %0 = bitcast double %x to i64
  %ew_u.sroa.0.4.extract.shift = lshr i64 %0, 32
  %ew_u.sroa.0.4.extract.trunc = trunc i64 %ew_u.sroa.0.4.extract.shift to i32
  %ew_u.sroa.0.0.extract.trunc = trunc i64 %0 to i32
  %and = and i32 %ew_u.sroa.0.4.extract.trunc, 2147483647
  %1 = icmp ne i32 %ew_u.sroa.0.0.extract.trunc, 0
  %shr = zext i1 %1 to i32
  %or1 = or i32 %and, %shr
  %cmp = icmp ugt i32 %or1, 2146435072
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %add = fadd double %x, %x
  br label %cleanup

if.end:                                           ; preds = %entry
  %cmp2 = icmp slt i32 %n, 0
  %fneg = fneg double %x
  %x.addr.0 = select i1 %cmp2, double %fneg, double %x
  %n.addr.0 = tail call i32 @llvm.abs.i32(i32 %n, i1 true)
  switch i32 %n.addr.0, label %if.end12 [
    i32 0, label %if.then7
    i32 1, label %if.then10
  ]

if.then7:                                         ; preds = %if.end
  %call = tail call double @j0(double noundef %x.addr.0) #6
  br label %cleanup

if.then10:                                        ; preds = %if.end
  %call11 = tail call double @j1(double noundef %x.addr.0) #6
  br label %cleanup

if.end12:                                         ; preds = %if.end
  %hx.0 = xor i32 %ew_u.sroa.0.4.extract.trunc, %n
  %and13 = and i32 %n.addr.0, 1
  %isneg = icmp sgt i32 %hx.0, -1
  %2 = tail call double @llvm.fabs.f64(double %x)
  %or16 = or i32 %and, %ew_u.sroa.0.0.extract.trunc
  %cmp17 = icmp eq i32 %or16, 0
  %cmp18 = icmp ugt i32 %and, 2146435071
  %or.cond = or i1 %cmp18, %cmp17
  br i1 %or.cond, label %if.end151, label %if.else

if.else:                                          ; preds = %if.end12
  %conv = sitofp i32 %n.addr.0 to double
  %cmp20 = fcmp ult double %2, %conv
  br i1 %cmp20, label %if.else47, label %if.then22

if.then22:                                        ; preds = %if.else
  %cmp23 = icmp ugt i32 %and, 1389363199
  br i1 %cmp23, label %if.then25, label %for.body.preheader

if.then25:                                        ; preds = %if.then22
  call void @sincos(double noundef %2, ptr noundef nonnull %s, ptr noundef nonnull %c) #6
  %and26 = and i32 %n.addr.0, 3
  %3 = load double, ptr %c, align 8, !tbaa !5
  switch i32 %and26, label %default.unreachable320 [
    i32 0, label %sw.bb
    i32 1, label %sw.bb28
    i32 2, label %sw.bb31
    i32 3, label %sw.bb34
  ]

sw.bb:                                            ; preds = %if.then25
  %4 = load double, ptr %s, align 8, !tbaa !5
  %add27 = fadd double %3, %4
  br label %sw.epilog

sw.bb28:                                          ; preds = %if.then25
  %5 = load double, ptr %s, align 8, !tbaa !5
  %add30 = fsub double %5, %3
  br label %sw.epilog

sw.bb31:                                          ; preds = %if.then25
  %fneg32 = fneg double %3
  %6 = load double, ptr %s, align 8, !tbaa !5
  %sub33 = fsub double %fneg32, %6
  br label %sw.epilog

sw.bb34:                                          ; preds = %if.then25
  %7 = load double, ptr %s, align 8, !tbaa !5
  %sub35 = fsub double %3, %7
  br label %sw.epilog

default.unreachable320:                           ; preds = %if.then25
  unreachable

sw.epilog:                                        ; preds = %sw.bb34, %sw.bb31, %sw.bb28, %sw.bb
  %temp.0 = phi double [ %sub35, %sw.bb34 ], [ %sub33, %sw.bb31 ], [ %add30, %sw.bb28 ], [ %add27, %sw.bb ]
  %mul = fmul double %temp.0, 0x3FE20DD750429B6D
  %sqrt = call double @llvm.sqrt.f64(double %2)
  %div = fdiv double %mul, %sqrt
  br label %if.end151

for.body.preheader:                               ; preds = %if.then22
  %call38 = tail call double @j0(double noundef %2) #6
  %call39 = tail call double @j1(double noundef %2) #6
  %8 = add nsw i32 %n.addr.0, -1
  %xtraiter = and i32 %8, 1
  %9 = icmp eq i32 %n.addr.0, 2
  br i1 %9, label %if.end151.loopexit.unr-lcssa, label %for.body.preheader.new

for.body.preheader.new:                           ; preds = %for.body.preheader
  %unroll_iter = and i32 %8, -2
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader.new
  %i.0285 = phi i32 [ 1, %for.body.preheader.new ], [ %inc.1, %for.body ]
  %b.0284 = phi double [ %call39, %for.body.preheader.new ], [ %11, %for.body ]
  %a.0283 = phi double [ %call38, %for.body.preheader.new ], [ %10, %for.body ]
  %niter = phi i32 [ 0, %for.body.preheader.new ], [ %niter.next.1, %for.body ]
  %add42 = shl nuw nsw i32 %i.0285, 1
  %conv43 = sitofp i32 %add42 to double
  %div44 = fdiv double %conv43, %2
  %neg = fneg double %a.0283
  %10 = tail call double @llvm.fmuladd.f64(double %b.0284, double %div44, double %neg)
  %inc = shl nuw i32 %i.0285, 1
  %add42.1 = add i32 %inc, 2
  %conv43.1 = sitofp i32 %add42.1 to double
  %div44.1 = fdiv double %conv43.1, %2
  %neg.1 = fneg double %b.0284
  %11 = tail call double @llvm.fmuladd.f64(double %10, double %div44.1, double %neg.1)
  %inc.1 = add nuw nsw i32 %i.0285, 2
  %niter.next.1 = add i32 %niter, 2
  %niter.ncmp.1 = icmp eq i32 %niter.next.1, %unroll_iter
  br i1 %niter.ncmp.1, label %if.end151.loopexit.unr-lcssa.loopexit, label %for.body, !llvm.loop !9

if.else47:                                        ; preds = %if.else
  %cmp48 = icmp ult i32 %and, 1041235968
  br i1 %cmp48, label %if.then50, label %if.else68

if.then50:                                        ; preds = %if.else47
  %cmp51 = icmp ugt i32 %n.addr.0, 33
  br i1 %cmp51, label %if.end151, label %for.body59.preheader

for.body59.preheader:                             ; preds = %if.then50
  %mul55 = fmul double %2, 5.000000e-01
  %12 = add nsw i32 %n.addr.0, -1
  %13 = add nsw i32 %n.addr.0, -2
  %xtraiter329 = and i32 %12, 3
  %14 = icmp ult i32 %13, 3
  br i1 %14, label %for.end65.unr-lcssa, label %for.body59.preheader.new

for.body59.preheader.new:                         ; preds = %for.body59.preheader
  %unroll_iter333 = and i32 %12, -4
  br label %for.body59

for.body59:                                       ; preds = %for.body59, %for.body59.preheader.new
  %i.1315 = phi i32 [ 2, %for.body59.preheader.new ], [ %inc64.3, %for.body59 ]
  %b.1314 = phi double [ %mul55, %for.body59.preheader.new ], [ %mul62.3, %for.body59 ]
  %a.1313 = phi double [ 1.000000e+00, %for.body59.preheader.new ], [ %mul61.3, %for.body59 ]
  %niter334 = phi i32 [ 0, %for.body59.preheader.new ], [ %niter334.next.3, %for.body59 ]
  %conv60 = sitofp i32 %i.1315 to double
  %mul61 = fmul double %a.1313, %conv60
  %mul62 = fmul double %mul55, %b.1314
  %inc64 = or disjoint i32 %i.1315, 1
  %conv60.1 = sitofp i32 %inc64 to double
  %mul61.1 = fmul double %mul61, %conv60.1
  %mul62.1 = fmul double %mul55, %mul62
  %inc64.1 = add nuw i32 %i.1315, 2
  %conv60.2 = sitofp i32 %inc64.1 to double
  %mul61.2 = fmul double %mul61.1, %conv60.2
  %mul62.2 = fmul double %mul55, %mul62.1
  %inc64.2 = add nuw i32 %i.1315, 3
  %conv60.3 = sitofp i32 %inc64.2 to double
  %mul61.3 = fmul double %mul61.2, %conv60.3
  %mul62.3 = fmul double %mul55, %mul62.2
  %inc64.3 = add nuw i32 %i.1315, 4
  %niter334.next.3 = add i32 %niter334, 4
  %niter334.ncmp.3 = icmp eq i32 %niter334.next.3, %unroll_iter333
  br i1 %niter334.ncmp.3, label %for.end65.unr-lcssa, label %for.body59, !llvm.loop !11

for.end65.unr-lcssa:                              ; preds = %for.body59, %for.body59.preheader
  %mul61.lcssa.ph = phi double [ undef, %for.body59.preheader ], [ %mul61.3, %for.body59 ]
  %mul62.lcssa.ph = phi double [ undef, %for.body59.preheader ], [ %mul62.3, %for.body59 ]
  %i.1315.unr = phi i32 [ 2, %for.body59.preheader ], [ %inc64.3, %for.body59 ]
  %b.1314.unr = phi double [ %mul55, %for.body59.preheader ], [ %mul62.3, %for.body59 ]
  %a.1313.unr = phi double [ 1.000000e+00, %for.body59.preheader ], [ %mul61.3, %for.body59 ]
  %lcmp.mod330.not = icmp eq i32 %xtraiter329, 0
  br i1 %lcmp.mod330.not, label %for.end65, label %for.body59.epil

for.body59.epil:                                  ; preds = %for.end65.unr-lcssa, %for.body59.epil
  %i.1315.epil = phi i32 [ %inc64.epil, %for.body59.epil ], [ %i.1315.unr, %for.end65.unr-lcssa ]
  %b.1314.epil = phi double [ %mul62.epil, %for.body59.epil ], [ %b.1314.unr, %for.end65.unr-lcssa ]
  %a.1313.epil = phi double [ %mul61.epil, %for.body59.epil ], [ %a.1313.unr, %for.end65.unr-lcssa ]
  %epil.iter = phi i32 [ %epil.iter.next, %for.body59.epil ], [ 0, %for.end65.unr-lcssa ]
  %conv60.epil = sitofp i32 %i.1315.epil to double
  %mul61.epil = fmul double %a.1313.epil, %conv60.epil
  %mul62.epil = fmul double %mul55, %b.1314.epil
  %inc64.epil = add nuw i32 %i.1315.epil, 1
  %epil.iter.next = add i32 %epil.iter, 1
  %epil.iter.cmp.not = icmp eq i32 %epil.iter.next, %xtraiter329
  br i1 %epil.iter.cmp.not, label %for.end65, label %for.body59.epil, !llvm.loop !12

for.end65:                                        ; preds = %for.body59.epil, %for.end65.unr-lcssa
  %mul61.lcssa = phi double [ %mul61.lcssa.ph, %for.end65.unr-lcssa ], [ %mul61.epil, %for.body59.epil ]
  %mul62.lcssa = phi double [ %mul62.lcssa.ph, %for.end65.unr-lcssa ], [ %mul62.epil, %for.body59.epil ]
  %div66 = fdiv double %mul62.lcssa, %mul61.lcssa
  br label %if.end151

if.else68:                                        ; preds = %if.else47
  %add69 = shl nuw nsw i32 %n.addr.0, 1
  %conv70 = sitofp i32 %add69 to double
  %15 = insertelement <2 x double> <double 2.000000e+00, double poison>, double %conv70, i64 1
  %16 = insertelement <2 x double> poison, double %2, i64 0
  %17 = shufflevector <2 x double> %16, <2 x double> poison, <2 x i32> zeroinitializer
  %18 = fdiv <2 x double> %15, %17
  %19 = extractelement <2 x double> %18, i64 0
  %20 = extractelement <2 x double> %18, i64 1
  %add73 = fadd double %19, %20
  %21 = tail call double @llvm.fmuladd.f64(double %20, double %add73, double -1.000000e+00)
  %cmp75286 = fcmp olt double %21, 1.000000e+09
  br i1 %cmp75286, label %while.body, label %for.body87.preheader

while.body:                                       ; preds = %if.else68, %while.body
  %k.0290 = phi i32 [ %add77, %while.body ], [ 1, %if.else68 ]
  %q1.0289 = phi double [ %22, %while.body ], [ %21, %if.else68 ]
  %q0.0288 = phi double [ %q1.0289, %while.body ], [ %20, %if.else68 ]
  %z.0287 = phi double [ %add78, %while.body ], [ %add73, %if.else68 ]
  %add77 = add nuw nsw i32 %k.0290, 1
  %add78 = fadd double %19, %z.0287
  %neg80 = fneg double %q0.0288
  %22 = tail call double @llvm.fmuladd.f64(double %add78, double %q1.0289, double %neg80)
  %cmp75 = fcmp olt double %22, 1.000000e+09
  br i1 %cmp75, label %while.body, label %for.body87.preheader, !llvm.loop !14

for.body87.preheader:                             ; preds = %while.body, %if.else68
  %k.0.lcssa = phi i32 [ 1, %if.else68 ], [ %add77, %while.body ]
  %add82 = add nuw nsw i32 %k.0.lcssa, %n.addr.0
  %mul83 = shl nuw nsw i32 %add82, 1
  br label %for.body87

for.body87:                                       ; preds = %for.body87.preheader, %for.body87
  %t.0294 = phi double [ %div91, %for.body87 ], [ 0.000000e+00, %for.body87.preheader ]
  %i.2293 = phi i32 [ %sub93, %for.body87 ], [ %mul83, %for.body87.preheader ]
  %conv88 = sitofp i32 %i.2293 to double
  %div89 = fdiv double %conv88, %2
  %sub90 = fsub double %div89, %t.0294
  %div91 = fdiv double 1.000000e+00, %sub90
  %sub93 = add nsw i32 %i.2293, -2
  %cmp85.not = icmp slt i32 %sub93, %add69
  br i1 %cmp85.not, label %for.end94, label %for.body87, !llvm.loop !15

for.end94:                                        ; preds = %for.body87
  %mul97 = fmul double %19, %conv
  %23 = tail call double @llvm.fabs.f64(double %mul97)
  %call98 = tail call double @log(double noundef %23) #6
  %mul99 = fmul double %call98, %conv
  %cmp100 = fcmp olt double %mul99, 0x40862E42FEFA39EF
  %cmp107305 = icmp ugt i32 %n.addr.0, 1
  br i1 %cmp100, label %if.then102, label %if.else116

if.then102:                                       ; preds = %for.end94
  br i1 %cmp107305, label %for.body109.preheader, label %if.end137

for.body109.preheader:                            ; preds = %if.then102
  %sub103 = add nsw i32 %n.addr.0, -1
  %add104 = shl nuw nsw i32 %sub103, 1
  %conv105 = sitofp i32 %add104 to double
  %smin = tail call i32 @llvm.smin.i32(i32 %sub103, i32 1)
  %24 = sub i32 %n.addr.0, %smin
  %25 = sub i32 %sub103, %smin
  %xtraiter327 = and i32 %24, 3
  %lcmp.mod328.not = icmp eq i32 %xtraiter327, 0
  br i1 %lcmp.mod328.not, label %for.body109.prol.loopexit, label %for.body109.prol

for.body109.prol:                                 ; preds = %for.body109.preheader, %for.body109.prol
  %i.3309.prol = phi i32 [ %dec.prol, %for.body109.prol ], [ %sub103, %for.body109.preheader ]
  %di.0308.prol = phi double [ %sub113.prol, %for.body109.prol ], [ %conv105, %for.body109.preheader ]
  %b.2307.prol = phi double [ %sub112.prol, %for.body109.prol ], [ 1.000000e+00, %for.body109.preheader ]
  %a.2306.prol = phi double [ %b.2307.prol, %for.body109.prol ], [ %div91, %for.body109.preheader ]
  %prol.iter = phi i32 [ %prol.iter.next, %for.body109.prol ], [ 0, %for.body109.preheader ]
  %mul110.prol = fmul double %b.2307.prol, %di.0308.prol
  %div111.prol = fdiv double %mul110.prol, %2
  %sub112.prol = fsub double %div111.prol, %a.2306.prol
  %sub113.prol = fadd double %di.0308.prol, -2.000000e+00
  %dec.prol = add nsw i32 %i.3309.prol, -1
  %prol.iter.next = add i32 %prol.iter, 1
  %prol.iter.cmp.not = icmp eq i32 %prol.iter.next, %xtraiter327
  br i1 %prol.iter.cmp.not, label %for.body109.prol.loopexit, label %for.body109.prol, !llvm.loop !16

for.body109.prol.loopexit:                        ; preds = %for.body109.prol, %for.body109.preheader
  %i.3309.unr = phi i32 [ %sub103, %for.body109.preheader ], [ %dec.prol, %for.body109.prol ]
  %di.0308.unr = phi double [ %conv105, %for.body109.preheader ], [ %sub113.prol, %for.body109.prol ]
  %b.2307.unr = phi double [ 1.000000e+00, %for.body109.preheader ], [ %sub112.prol, %for.body109.prol ]
  %a.2306.unr = phi double [ %div91, %for.body109.preheader ], [ %b.2307.prol, %for.body109.prol ]
  %b.2307.lcssa.unr = phi double [ undef, %for.body109.preheader ], [ %b.2307.prol, %for.body109.prol ]
  %sub112.lcssa.unr = phi double [ undef, %for.body109.preheader ], [ %sub112.prol, %for.body109.prol ]
  %26 = icmp ult i32 %25, 3
  br i1 %26, label %if.end137, label %for.body109

for.body109:                                      ; preds = %for.body109.prol.loopexit, %for.body109
  %i.3309 = phi i32 [ %dec.3, %for.body109 ], [ %i.3309.unr, %for.body109.prol.loopexit ]
  %di.0308 = phi double [ %sub113.3, %for.body109 ], [ %di.0308.unr, %for.body109.prol.loopexit ]
  %b.2307 = phi double [ %sub112.3, %for.body109 ], [ %b.2307.unr, %for.body109.prol.loopexit ]
  %a.2306 = phi double [ %sub112.2, %for.body109 ], [ %a.2306.unr, %for.body109.prol.loopexit ]
  %mul110 = fmul double %b.2307, %di.0308
  %div111 = fdiv double %mul110, %2
  %sub112 = fsub double %div111, %a.2306
  %sub113 = fadd double %di.0308, -2.000000e+00
  %mul110.1 = fmul double %sub112, %sub113
  %div111.1 = fdiv double %mul110.1, %2
  %sub112.1 = fsub double %div111.1, %b.2307
  %sub113.1 = fadd double %sub113, -2.000000e+00
  %mul110.2 = fmul double %sub112.1, %sub113.1
  %div111.2 = fdiv double %mul110.2, %2
  %sub112.2 = fsub double %div111.2, %sub112
  %sub113.2 = fadd double %sub113.1, -2.000000e+00
  %mul110.3 = fmul double %sub112.2, %sub113.2
  %div111.3 = fdiv double %mul110.3, %2
  %sub112.3 = fsub double %div111.3, %sub112.1
  %sub113.3 = fadd double %sub113.2, -2.000000e+00
  %dec.3 = add nsw i32 %i.3309, -4
  %cmp107.3 = icmp sgt i32 %i.3309, 4
  br i1 %cmp107.3, label %for.body109, label %if.end137, !llvm.loop !17

if.else116:                                       ; preds = %for.end94
  br i1 %cmp107305, label %for.body123.preheader, label %if.end137

for.body123.preheader:                            ; preds = %if.else116
  %sub117 = add nsw i32 %n.addr.0, -1
  %add118 = shl nuw nsw i32 %sub117, 1
  %conv119 = sitofp i32 %add118 to double
  %27 = insertelement <2 x double> poison, double %div91, i64 0
  %28 = shufflevector <2 x double> %27, <2 x double> poison, <2 x i32> zeroinitializer
  br label %for.body123

for.body123:                                      ; preds = %for.body123.preheader, %for.body123
  %i.4300 = phi i32 [ %dec135, %for.body123 ], [ %sub117, %for.body123.preheader ]
  %di.1299 = phi double [ %sub127, %for.body123 ], [ %conv119, %for.body123.preheader ]
  %b.3298 = phi double [ %b.4, %for.body123 ], [ 1.000000e+00, %for.body123.preheader ]
  %29 = phi <2 x double> [ %34, %for.body123 ], [ %28, %for.body123.preheader ]
  %mul124 = fmul double %b.3298, %di.1299
  %div125 = fdiv double %mul124, %2
  %30 = extractelement <2 x double> %29, i64 1
  %sub126 = fsub double %div125, %30
  %sub127 = fadd double %di.1299, -2.000000e+00
  %cmp128 = fcmp ogt double %sub126, 1.000000e+100
  %div131 = select i1 %cmp128, double %sub126, double 1.000000e+00
  %b.4 = select i1 %cmp128, double 1.000000e+00, double %sub126
  %31 = insertelement <2 x double> %29, double %b.3298, i64 1
  %32 = insertelement <2 x double> poison, double %div131, i64 0
  %33 = shufflevector <2 x double> %32, <2 x double> poison, <2 x i32> zeroinitializer
  %34 = fdiv <2 x double> %31, %33
  %dec135 = add nsw i32 %i.4300, -1
  %cmp121 = icmp sgt i32 %i.4300, 1
  br i1 %cmp121, label %for.body123, label %if.end137.loopexit323, !llvm.loop !18

if.end137.loopexit323:                            ; preds = %for.body123
  %35 = extractelement <2 x double> %34, i64 1
  %36 = extractelement <2 x double> %34, i64 0
  br label %if.end137

if.end137:                                        ; preds = %for.body109.prol.loopexit, %for.body109, %if.end137.loopexit323, %if.else116, %if.then102
  %a.5 = phi double [ %div91, %if.then102 ], [ %div91, %if.else116 ], [ %35, %if.end137.loopexit323 ], [ %b.2307.lcssa.unr, %for.body109.prol.loopexit ], [ %sub112.2, %for.body109 ]
  %b.5 = phi double [ 1.000000e+00, %if.then102 ], [ 1.000000e+00, %if.else116 ], [ %b.4, %if.end137.loopexit323 ], [ %sub112.lcssa.unr, %for.body109.prol.loopexit ], [ %sub112.3, %for.body109 ]
  %t.3 = phi double [ %div91, %if.then102 ], [ %div91, %if.else116 ], [ %36, %if.end137.loopexit323 ], [ %div91, %for.body109 ], [ %div91, %for.body109.prol.loopexit ]
  %call138 = tail call double @j0(double noundef %2) #6
  %call139 = tail call double @j1(double noundef %2) #6
  %37 = tail call double @llvm.fabs.f64(double %call138)
  %38 = tail call double @llvm.fabs.f64(double %call139)
  %cmp140 = fcmp ult double %37, %38
  %mul143 = fmul double %t.3, %call138
  %div144 = fdiv double %mul143, %b.5
  %mul146 = fmul double %t.3, %call139
  %div147 = fdiv double %mul146, %a.5
  %b.6 = select i1 %cmp140, double %div147, double %div144
  br label %if.end151

if.end151.loopexit.unr-lcssa.loopexit:            ; preds = %for.body
  %39 = shl nuw nsw i32 %inc.1, 1
  %40 = sitofp i32 %39 to double
  br label %if.end151.loopexit.unr-lcssa

if.end151.loopexit.unr-lcssa:                     ; preds = %if.end151.loopexit.unr-lcssa.loopexit, %for.body.preheader
  %.lcssa325.ph = phi double [ undef, %for.body.preheader ], [ %11, %if.end151.loopexit.unr-lcssa.loopexit ]
  %i.0285.unr = phi double [ 2.000000e+00, %for.body.preheader ], [ %40, %if.end151.loopexit.unr-lcssa.loopexit ]
  %b.0284.unr = phi double [ %call39, %for.body.preheader ], [ %11, %if.end151.loopexit.unr-lcssa.loopexit ]
  %a.0283.unr = phi double [ %call38, %for.body.preheader ], [ %10, %if.end151.loopexit.unr-lcssa.loopexit ]
  %lcmp.mod.not = icmp eq i32 %xtraiter, 0
  %div44.epil = fdiv double %i.0285.unr, %2
  %neg.epil = fneg double %a.0283.unr
  %41 = tail call double @llvm.fmuladd.f64(double %b.0284.unr, double %div44.epil, double %neg.epil)
  %.lcssa325 = select i1 %lcmp.mod.not, double %.lcssa325.ph, double %41
  br label %if.end151

if.end151:                                        ; preds = %if.end151.loopexit.unr-lcssa, %if.then50, %if.end12, %sw.epilog, %for.end65, %if.end137
  %b.7 = phi double [ %div, %sw.epilog ], [ %div66, %for.end65 ], [ %b.6, %if.end137 ], [ 0.000000e+00, %if.end12 ], [ 0.000000e+00, %if.then50 ], [ %.lcssa325, %if.end151.loopexit.unr-lcssa ]
  %cmp152.not280 = icmp eq i32 %and13, 0
  %cmp152.not = or i1 %cmp152.not280, %isneg
  br i1 %cmp152.not, label %cleanup, label %if.then154

if.then154:                                       ; preds = %if.end151
  %fneg155 = fneg double %b.7
  br label %cleanup

cleanup:                                          ; preds = %if.end151, %if.then154, %if.then10, %if.then7, %if.then
  %retval.0 = phi double [ %add, %if.then ], [ %call, %if.then7 ], [ %call11, %if.then10 ], [ %fneg155, %if.then154 ], [ %b.7, %if.end151 ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %s) #6
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %c) #6
  ret double %retval.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

declare double @j0(double noundef) local_unnamed_addr #2

declare double @j1(double noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fabs.f64(double) #3

declare void @sincos(double noundef, ptr noundef, ptr noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare double @sqrt(double noundef) local_unnamed_addr #4

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fmuladd.f64(double, double, double) #3

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare double @log(double noundef) local_unnamed_addr #4

; Function Attrs: nounwind uwtable
define dso_local double @ref_yn(i32 noundef %n, double noundef %x) local_unnamed_addr #0 {
entry:
  %c = alloca double, align 8
  %s = alloca double, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %c) #6
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %s) #6
  %0 = bitcast double %x to i64
  %ew_u.sroa.0.4.extract.shift = lshr i64 %0, 32
  %ew_u.sroa.0.4.extract.trunc = trunc i64 %ew_u.sroa.0.4.extract.shift to i32
  %ew_u.sroa.0.0.extract.trunc = trunc i64 %0 to i32
  %and = and i32 %ew_u.sroa.0.4.extract.trunc, 2147483647
  %1 = icmp ne i32 %ew_u.sroa.0.0.extract.trunc, 0
  %shr = zext i1 %1 to i32
  %or1 = or i32 %and, %shr
  %cmp = icmp ugt i32 %or1, 2146435072
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %add = fadd double %x, %x
  br label %cleanup

if.end:                                           ; preds = %entry
  %or2 = or i32 %and, %ew_u.sroa.0.0.extract.trunc
  %cmp3 = icmp eq i32 %or2, 0
  br i1 %cmp3, label %if.then4, label %if.end5

if.then4:                                         ; preds = %if.end
  %2 = load volatile double, ptr @vzero, align 8, !tbaa !5
  %div = fdiv double -1.000000e+00, %2
  br label %cleanup

if.end5:                                          ; preds = %if.end
  %cmp6 = icmp slt i64 %0, 0
  br i1 %cmp6, label %if.then7, label %if.end9

if.then7:                                         ; preds = %if.end5
  %3 = load volatile double, ptr @vzero, align 8, !tbaa !5
  %4 = load volatile double, ptr @vzero, align 8, !tbaa !5
  %div8 = fdiv double %3, %4
  br label %cleanup

if.end9:                                          ; preds = %if.end5
  %cmp10 = icmp slt i32 %n, 0
  %sub12 = sub nsw i32 0, %n
  %and13 = shl nuw i32 %sub12, 1
  %shl = and i32 %and13, 2
  %sub14 = sub nsw i32 1, %shl
  %sign.0 = select i1 %cmp10, i32 %sub14, i32 1
  %n.addr.0 = select i1 %cmp10, i32 %sub12, i32 %n
  switch i32 %n.addr.0, label %if.end22 [
    i32 0, label %if.then17
    i32 1, label %if.then20
  ]

if.then17:                                        ; preds = %if.end9
  %call = tail call double @y0(double noundef %x) #6
  br label %cleanup

if.then20:                                        ; preds = %if.end9
  %conv = sitofp i32 %sign.0 to double
  %call21 = tail call double @y1(double noundef %x) #6
  %mul = fmul double %call21, %conv
  br label %cleanup

if.end22:                                         ; preds = %if.end9
  %cmp23 = icmp eq i32 %and, 2146435072
  br i1 %cmp23, label %cleanup, label %if.end26

if.end26:                                         ; preds = %if.end22
  %cmp27 = icmp ugt i32 %and, 1389363199
  br i1 %cmp27, label %if.then29, label %if.else

if.then29:                                        ; preds = %if.end26
  call void @sincos(double noundef %x, ptr noundef nonnull %s, ptr noundef nonnull %c) #6
  %and30 = and i32 %n.addr.0, 3
  %5 = load double, ptr %s, align 8, !tbaa !5
  switch i32 %and30, label %default.unreachable114 [
    i32 0, label %sw.bb
    i32 1, label %sw.bb32
    i32 2, label %sw.bb34
    i32 3, label %sw.bb37
  ]

sw.bb:                                            ; preds = %if.then29
  %6 = load double, ptr %c, align 8, !tbaa !5
  %sub31 = fsub double %5, %6
  br label %sw.epilog

sw.bb32:                                          ; preds = %if.then29
  %fneg = fneg double %5
  %7 = load double, ptr %c, align 8, !tbaa !5
  %sub33 = fsub double %fneg, %7
  br label %sw.epilog

sw.bb34:                                          ; preds = %if.then29
  %8 = load double, ptr %c, align 8, !tbaa !5
  %add36 = fsub double %8, %5
  br label %sw.epilog

sw.bb37:                                          ; preds = %if.then29
  %9 = load double, ptr %c, align 8, !tbaa !5
  %add38 = fadd double %5, %9
  br label %sw.epilog

default.unreachable114:                           ; preds = %if.then29
  unreachable

sw.epilog:                                        ; preds = %sw.bb37, %sw.bb34, %sw.bb32, %sw.bb
  %temp.0 = phi double [ %add38, %sw.bb37 ], [ %add36, %sw.bb34 ], [ %sub33, %sw.bb32 ], [ %sub31, %sw.bb ]
  %mul39 = fmul double %temp.0, 0x3FE20DD750429B6D
  %call40 = call double @sqrt(double noundef %x) #6
  %div41 = fdiv double %mul39, %call40
  br label %if.end61

if.else:                                          ; preds = %if.end26
  %call42 = tail call double @y0(double noundef %x) #6
  %call43 = tail call double @y1(double noundef %x) #6
  %high.0.in.in107 = bitcast double %call43 to i64
  %cmp48108 = icmp sgt i32 %n.addr.0, 1
  %high.0.in.mask109 = and i64 %high.0.in.in107, -4294967296
  %cmp50110 = icmp ne i64 %high.0.in.mask109, -4503599627370496
  %10 = select i1 %cmp48108, i1 %cmp50110, i1 false
  br i1 %10, label %for.body, label %if.end61

for.body:                                         ; preds = %if.else, %for.body
  %i.0113 = phi i32 [ %inc, %for.body ], [ 1, %if.else ]
  %b.0112 = phi double [ %11, %for.body ], [ %call43, %if.else ]
  %a.0111 = phi double [ %b.0112, %for.body ], [ %call42, %if.else ]
  %add52 = shl nuw nsw i32 %i.0113, 1
  %conv53 = sitofp i32 %add52 to double
  %div54 = fdiv double %conv53, %x
  %neg = fneg double %a.0111
  %11 = tail call double @llvm.fmuladd.f64(double %div54, double %b.0112, double %neg)
  %inc = add nuw nsw i32 %i.0113, 1
  %high.0.in.in = bitcast double %11 to i64
  %cmp48 = icmp slt i32 %inc, %n.addr.0
  %high.0.in.mask = and i64 %high.0.in.in, -4294967296
  %cmp50 = icmp ne i64 %high.0.in.mask, -4503599627370496
  %12 = select i1 %cmp48, i1 %cmp50, i1 false
  br i1 %12, label %for.body, label %if.end61, !llvm.loop !19

if.end61:                                         ; preds = %for.body, %if.else, %sw.epilog
  %b.1 = phi double [ %div41, %sw.epilog ], [ %call43, %if.else ], [ %11, %for.body ]
  %cmp62 = icmp sgt i32 %sign.0, 0
  br i1 %cmp62, label %cleanup, label %if.else65

if.else65:                                        ; preds = %if.end61
  %fneg66 = fneg double %b.1
  br label %cleanup

cleanup:                                          ; preds = %if.end61, %if.end22, %if.else65, %if.then20, %if.then17, %if.then7, %if.then4, %if.then
  %retval.0 = phi double [ %add, %if.then ], [ %div, %if.then4 ], [ %div8, %if.then7 ], [ %call, %if.then17 ], [ %mul, %if.then20 ], [ %fneg66, %if.else65 ], [ 0.000000e+00, %if.end22 ], [ %b.1, %if.end61 ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %s) #6
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %c) #6
  ret double %retval.0
}

declare double @y0(double noundef) local_unnamed_addr #2

declare double @y1(double noundef) local_unnamed_addr #2

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.abs.i32(i32, i1 immarg) #5

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.sqrt.f64(double) #5

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.smin.i32(i32, i32) #5

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #4 = { mustprogress nofree nounwind willreturn memory(write) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #6 = { nounwind }

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
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.mustprogress"}
!11 = distinct !{!11, !10}
!12 = distinct !{!12, !13}
!13 = !{!"llvm.loop.unroll.disable"}
!14 = distinct !{!14, !10}
!15 = distinct !{!15, !10}
!16 = distinct !{!16, !13}
!17 = distinct !{!17, !10}
!18 = distinct !{!18, !10}
!19 = distinct !{!19, !10}
