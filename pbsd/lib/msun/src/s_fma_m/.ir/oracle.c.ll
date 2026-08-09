; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/s_fma_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/s_fma_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local double @ref_fma(double noundef %x, double noundef %y, double noundef %z) local_unnamed_addr #0 {
entry:
  %__mxcsr.i = alloca i32, align 4
  %__control.i173 = alloca i16, align 2
  %__control.i = alloca i16, align 2
  %ex = alloca i32, align 4
  %ey = alloca i32, align 4
  %ez = alloca i32, align 4
  %vxs = alloca double, align 8
  %vzs = alloca double, align 8
  %vrlo = alloca double, align 8
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %ex) #10
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %ey) #10
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %ez) #10
  %cmp = fcmp oeq double %x, 0.000000e+00
  %cmp1 = fcmp oeq double %y, 0.000000e+00
  %or.cond = or i1 %cmp, %cmp1
  br i1 %or.cond, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %0 = tail call double @llvm.fmuladd.f64(double %x, double %y, double %z)
  br label %cleanup105

if.end:                                           ; preds = %entry
  %cmp2 = fcmp oeq double %z, 0.000000e+00
  br i1 %cmp2, label %if.then3, label %if.end4

if.then3:                                         ; preds = %if.end
  %mul = fmul double %x, %y
  br label %cleanup105

if.end4:                                          ; preds = %if.end
  %call = tail call i32 @__isfinite(double noundef %x) #11
  %tobool.not = icmp eq i32 %call, 0
  br i1 %tobool.not, label %if.then8, label %lor.lhs.false5

lor.lhs.false5:                                   ; preds = %if.end4
  %call6 = tail call i32 @__isfinite(double noundef %y) #11
  %tobool7.not = icmp eq i32 %call6, 0
  br i1 %tobool7.not, label %if.then8, label %if.end10

if.then8:                                         ; preds = %lor.lhs.false5, %if.end4
  %1 = tail call double @llvm.fmuladd.f64(double %x, double %y, double %z)
  br label %cleanup105

if.end10:                                         ; preds = %lor.lhs.false5
  %call11 = tail call i32 @__isfinite(double noundef %z) #11
  %tobool12.not = icmp eq i32 %call11, 0
  br i1 %tobool12.not, label %cleanup105, label %if.end14

if.end14:                                         ; preds = %if.end10
  %call15 = call double @frexp(double noundef %x, ptr noundef nonnull %ex) #10
  %call16 = call double @frexp(double noundef %y, ptr noundef nonnull %ey) #10
  %call17 = call double @frexp(double noundef %z, ptr noundef nonnull %ez) #10
  call void @llvm.lifetime.start.p0(i64 2, ptr nonnull %__control.i) #10
  call void asm sideeffect "fnstcw $0", "=*m,~{dirflag},~{fpsr},~{flags}"(ptr nonnull elementtype(i16) %__control.i) #10, !srcloc !5
  %2 = load i16, ptr %__control.i, align 2, !tbaa !6
  %3 = and i16 %2, 3072
  %and.i = zext nneg i16 %3 to i32
  call void @llvm.lifetime.end.p0(i64 2, ptr nonnull %__control.i) #10
  %4 = load i32, ptr %ex, align 4, !tbaa !10
  %5 = load i32, ptr %ey, align 4, !tbaa !10
  %add = add nsw i32 %5, %4
  %6 = load i32, ptr %ez, align 4, !tbaa !10
  %sub = sub nsw i32 %add, %6
  %cmp19 = icmp slt i32 %sub, -53
  br i1 %cmp19, label %if.then20, label %if.end56

if.then20:                                        ; preds = %if.end14
  %call21 = call i32 @feraiseexcept(i32 noundef 32) #10
  %call22 = call i32 @__isnormal(double noundef %z) #11
  %tobool23.not = icmp eq i32 %call22, 0
  br i1 %tobool23.not, label %if.then24, label %if.end26

if.then24:                                        ; preds = %if.then20
  %call25 = call i32 @feraiseexcept(i32 noundef 16) #10
  br label %if.end26

if.end26:                                         ; preds = %if.then24, %if.then20
  switch i16 %3, label %sw.default [
    i16 0, label %cleanup105
    i16 3072, label %sw.bb27
    i16 1024, label %sw.bb37
  ]

sw.bb27:                                          ; preds = %if.end26
  %cmp28 = fcmp ogt double %x, 0.000000e+00
  %cmp29 = fcmp olt double %y, 0.000000e+00
  %xor170 = xor i1 %cmp28, %cmp29
  %cmp31 = fcmp olt double %z, 0.000000e+00
  %xor33171 = xor i1 %xor170, %cmp31
  br i1 %xor33171, label %cleanup105, label %if.else

if.else:                                          ; preds = %sw.bb27
  %call36 = call double @nextafter(double noundef %z, double noundef 0.000000e+00) #10
  br label %cleanup105

sw.bb37:                                          ; preds = %if.end26
  %cmp38 = fcmp ogt double %x, 0.000000e+00
  %cmp40 = fcmp olt double %y, 0.000000e+00
  %xor42169 = xor i1 %cmp38, %cmp40
  br i1 %xor42169, label %cleanup105, label %if.else45

if.else45:                                        ; preds = %sw.bb37
  %call46 = call double @nextafter(double noundef %z, double noundef 0xFFF0000000000000) #10
  br label %cleanup105

sw.default:                                       ; preds = %if.end26
  %cmp47 = fcmp ogt double %x, 0.000000e+00
  %cmp49 = fcmp olt double %y, 0.000000e+00
  %xor51172 = xor i1 %cmp47, %cmp49
  br i1 %xor51172, label %if.then53, label %cleanup105

if.then53:                                        ; preds = %sw.default
  %call54 = call double @nextafter(double noundef %z, double noundef 0x7FF0000000000000) #10
  br label %cleanup105

if.end56:                                         ; preds = %if.end14
  %cmp57 = icmp slt i32 %sub, 107
  br i1 %cmp57, label %if.then59, label %if.else62

if.then59:                                        ; preds = %if.end56
  %sub60 = sub nsw i32 0, %sub
  %call61 = call double @ldexp(double noundef %call17, i32 noundef %sub60) #10
  br label %if.end63

if.else62:                                        ; preds = %if.end56
  %7 = call double @llvm.copysign.f64(double 0x10000000000000, double %call17)
  br label %if.end63

if.end63:                                         ; preds = %if.else62, %if.then59
  %zs.0 = phi double [ %call61, %if.then59 ], [ %7, %if.else62 ]
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %__mxcsr.i) #10
  call void @llvm.lifetime.start.p0(i64 2, ptr nonnull %__control.i173) #10
  call void asm sideeffect "fnstcw $0", "=*m,~{dirflag},~{fpsr},~{flags}"(ptr nonnull elementtype(i16) %__control.i173) #10, !srcloc !12
  %8 = load i16, ptr %__control.i173, align 2, !tbaa !6
  %9 = and i16 %8, -3073
  store i16 %9, ptr %__control.i173, align 2, !tbaa !6
  call void asm sideeffect "fldcw $0", "*m,~{dirflag},~{fpsr},~{flags}"(ptr nonnull elementtype(i16) %__control.i173) #10, !srcloc !13
  call void asm sideeffect "stmxcsr $0", "=*m,~{dirflag},~{fpsr},~{flags}"(ptr nonnull elementtype(i32) %__mxcsr.i) #10, !srcloc !14
  %10 = load i32, ptr %__mxcsr.i, align 4, !tbaa !10
  %and5.i = and i32 %10, -24577
  store i32 %and5.i, ptr %__mxcsr.i, align 4, !tbaa !10
  call void asm sideeffect "ldmxcsr $0", "*m,~{dirflag},~{fpsr},~{flags}"(ptr nonnull elementtype(i32) %__mxcsr.i) #10, !srcloc !15
  call void @llvm.lifetime.end.p0(i64 2, ptr nonnull %__control.i173) #10
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %__mxcsr.i) #10
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %vxs)
  store volatile double %call15, ptr %vxs, align 8, !tbaa !16
  %vxs.0.vxs.0.vxs.0.vxs.0. = load volatile double, ptr %vxs, align 8, !tbaa !16
  %11 = insertelement <2 x double> poison, double %call16, i64 0
  %12 = insertelement <2 x double> %11, double %vxs.0.vxs.0.vxs.0.vxs.0., i64 1
  %13 = fmul <2 x double> %12, <double 0x41A0000002000000, double 0x41A0000002000000>
  %14 = fsub <2 x double> %12, %13
  %15 = fadd <2 x double> %13, %14
  %16 = extractelement <2 x double> %15, i64 1
  %sub1.i = fsub double %vxs.0.vxs.0.vxs.0.vxs.0., %16
  %17 = extractelement <2 x double> %15, i64 0
  %sub5.i = fsub double %call16, %17
  %mul6.i = fmul double %17, %16
  %mul8.i = fmul double %17, %sub1.i
  %18 = call double @llvm.fmuladd.f64(double %16, double %sub5.i, double %mul8.i)
  %add9.i = fadd double %mul6.i, %18
  %sub11.i = fsub double %mul6.i, %add9.i
  %add12.i = fadd double %18, %sub11.i
  %19 = call double @llvm.fmuladd.f64(double %sub1.i, double %sub5.i, double %add12.i)
  %add.i174 = fadd double %zs.0, %add9.i
  %sub.i175 = fsub double %add.i174, %add9.i
  %sub3.i176 = fsub double %add.i174, %sub.i175
  %sub4.i = fsub double %add9.i, %sub3.i176
  %sub5.i177 = fsub double %zs.0, %sub.i175
  %add6.i = fadd double %sub5.i177, %sub4.i
  %cmp70 = fcmp oeq double %add.i174, 0.000000e+00
  %cmp72 = fcmp oeq double %19, 0.000000e+00
  %or.cond116 = select i1 %cmp70, i1 %cmp72, i1 false
  br i1 %or.cond116, label %if.then74, label %if.end78

if.then74:                                        ; preds = %if.end63
  call fastcc void @__fesetround_int(i32 noundef %and.i)
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %vzs)
  store volatile double %zs.0, ptr %vzs, align 8, !tbaa !16
  %vzs.0.vzs.0.vzs.0.vzs.0. = load volatile double, ptr %vzs, align 8, !tbaa !16
  %add77 = fadd double %add9.i, %vzs.0.vzs.0.vzs.0.vzs.0.
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %vzs)
  br label %cleanup

if.end78:                                         ; preds = %if.end63
  %cmp79.not = icmp eq i16 %3, 0
  br i1 %cmp79.not, label %if.end89, label %if.then81

if.then81:                                        ; preds = %if.end78
  call fastcc void @__fesetround_int(i32 noundef %and.i)
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %vrlo)
  store volatile double %add6.i, ptr %vrlo, align 8, !tbaa !16
  %vrlo.0.vrlo.0.vrlo.0.vrlo.0. = load volatile double, ptr %vrlo, align 8, !tbaa !16
  %add85 = fadd double %19, %vrlo.0.vrlo.0.vrlo.0.vrlo.0.
  %add87 = fadd double %add.i174, %add85
  %call88 = call double @ldexp(double noundef %add87, i32 noundef %add) #10
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %vrlo)
  br label %cleanup

if.end89:                                         ; preds = %if.end78
  %add.i.i = fadd double %19, %add6.i
  %sub.i.i = fsub double %add.i.i, %add6.i
  %sub3.i.i = fsub double %add.i.i, %sub.i.i
  %sub4.i.i = fsub double %add6.i, %sub3.i.i
  %sub5.i.i = fsub double %19, %sub.i.i
  %add6.i.i = fadd double %sub5.i.i, %sub4.i.i
  %cmp.i = fcmp une double %add6.i.i, 0.000000e+00
  br i1 %cmp.i, label %do.body.i, label %ref_add_adjusted.exit

do.body.i:                                        ; preds = %if.end89
  %20 = bitcast double %add.i.i to i64
  %and.i180 = and i64 %20, 1
  %cmp1.i = icmp eq i64 %and.i180, 0
  br i1 %cmp1.i, label %do.body3.i, label %ref_add_adjusted.exit

do.body3.i:                                       ; preds = %do.body.i
  %21 = bitcast double %add6.i.i to i64
  %xor.i = xor i64 %21, %20
  %shr.i = lshr i64 %xor.i, 62
  %reass.sub.i = or disjoint i64 %20, 1
  %add.i181 = sub i64 %reass.sub.i, %shr.i
  %22 = bitcast i64 %add.i181 to double
  br label %ref_add_adjusted.exit

ref_add_adjusted.exit:                            ; preds = %if.end89, %do.body.i, %do.body3.i
  %sum.sroa.0.0.i = phi double [ %22, %do.body3.i ], [ %add.i.i, %do.body.i ], [ %add.i.i, %if.end89 ]
  %call94 = call i32 @ilogb(double noundef %add.i174) #11
  %add95 = add nsw i32 %call94, %add
  %cmp96 = icmp sgt i32 %add95, -1023
  br i1 %cmp96, label %if.then98, label %if.else102

if.then98:                                        ; preds = %ref_add_adjusted.exit
  %add100 = fadd double %add.i174, %sum.sroa.0.0.i
  %call101 = call double @ldexp(double noundef %add100, i32 noundef %add) #10
  br label %cleanup

if.else102:                                       ; preds = %ref_add_adjusted.exit
  %call104 = call fastcc double @ref_add_and_denormalize(double noundef %add.i174, double noundef %sum.sroa.0.0.i, i32 noundef %add)
  br label %cleanup

cleanup:                                          ; preds = %if.else102, %if.then98, %if.then81, %if.then74
  %retval.0 = phi double [ %add77, %if.then74 ], [ %call88, %if.then81 ], [ %call101, %if.then98 ], [ %call104, %if.else102 ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %vxs)
  br label %cleanup105

cleanup105:                                       ; preds = %sw.default, %sw.bb37, %sw.bb27, %if.end26, %if.end10, %cleanup, %if.then53, %if.else45, %if.else, %if.then8, %if.then3, %if.then
  %retval.1 = phi double [ %0, %if.then ], [ %mul, %if.then3 ], [ %call54, %if.then53 ], [ %call46, %if.else45 ], [ %call36, %if.else ], [ %retval.0, %cleanup ], [ %1, %if.then8 ], [ %z, %if.end10 ], [ %z, %if.end26 ], [ %z, %sw.bb27 ], [ %z, %sw.bb37 ], [ %z, %sw.default ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %ez) #10
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %ey) #10
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %ex) #10
  ret double %retval.1
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fmuladd.f64(double, double, double) #2

; Function Attrs: mustprogress nofree nosync nounwind willreturn memory(none)
declare i32 @__isfinite(double noundef) local_unnamed_addr #3

; Function Attrs: mustprogress nofree nounwind willreturn memory(argmem: write)
declare double @frexp(double noundef, ptr nocapture noundef) local_unnamed_addr #4

declare i32 @feraiseexcept(i32 noundef) local_unnamed_addr #5

; Function Attrs: mustprogress nofree nosync nounwind willreturn memory(none)
declare i32 @__isnormal(double noundef) local_unnamed_addr #3

; Function Attrs: nounwind
declare double @nextafter(double noundef, double noundef) local_unnamed_addr #6

; Function Attrs: mustprogress nofree nounwind willreturn
declare double @ldexp(double noundef, i32 noundef) local_unnamed_addr #7

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.copysign.f64(double, double) #2

; Function Attrs: inlinehint nounwind uwtable
define internal fastcc void @__fesetround_int(i32 noundef %__round) unnamed_addr #8 {
entry:
  %__mxcsr = alloca i32, align 4
  %__control = alloca i16, align 2
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %__mxcsr) #10
  call void @llvm.lifetime.start.p0(i64 2, ptr nonnull %__control) #10
  %and = and i32 %__round, -3073
  %tobool.not = icmp eq i32 %and, 0
  br i1 %tobool.not, label %if.end, label %cleanup

if.end:                                           ; preds = %entry
  call void asm sideeffect "fnstcw $0", "=*m,~{dirflag},~{fpsr},~{flags}"(ptr nonnull elementtype(i16) %__control) #10, !srcloc !12
  %0 = load i16, ptr %__control, align 2, !tbaa !6
  %1 = and i16 %0, -3073
  %2 = trunc i32 %__round to i16
  %conv4 = or disjoint i16 %1, %2
  store i16 %conv4, ptr %__control, align 2, !tbaa !6
  call void asm sideeffect "fldcw $0", "*m,~{dirflag},~{fpsr},~{flags}"(ptr nonnull elementtype(i16) %__control) #10, !srcloc !13
  call void asm sideeffect "stmxcsr $0", "=*m,~{dirflag},~{fpsr},~{flags}"(ptr nonnull elementtype(i32) %__mxcsr) #10, !srcloc !14
  %3 = load i32, ptr %__mxcsr, align 4, !tbaa !10
  %and5 = and i32 %3, -24577
  %shl = shl nuw nsw i32 %__round, 3
  %or6 = or disjoint i32 %and5, %shl
  store i32 %or6, ptr %__mxcsr, align 4, !tbaa !10
  call void asm sideeffect "ldmxcsr $0", "*m,~{dirflag},~{fpsr},~{flags}"(ptr nonnull elementtype(i32) %__mxcsr) #10, !srcloc !15
  br label %cleanup

cleanup:                                          ; preds = %entry, %if.end
  call void @llvm.lifetime.end.p0(i64 2, ptr nonnull %__control) #10
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %__mxcsr) #10
  ret void
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nofree nosync nounwind willreturn memory(none)
declare i32 @ilogb(double noundef) local_unnamed_addr #3

; Function Attrs: inlinehint mustprogress nofree nounwind willreturn uwtable
define internal fastcc double @ref_add_and_denormalize(double noundef %a, double noundef %b, i32 noundef %scale) unnamed_addr #9 {
entry:
  %add.i = fadd double %a, %b
  %sub.i = fsub double %add.i, %a
  %sub3.i = fsub double %add.i, %sub.i
  %sub4.i = fsub double %a, %sub3.i
  %sub5.i = fsub double %b, %sub.i
  %add6.i = fadd double %sub5.i, %sub4.i
  %cmp = fcmp une double %add6.i, 0.000000e+00
  br i1 %cmp, label %do.body, label %if.end23

do.body:                                          ; preds = %entry
  %0 = bitcast double %add.i to i64
  %shr = lshr i64 %0, 52
  %conv = trunc i64 %shr to i32
  %and = and i32 %conv, 2047
  %1 = sub i32 0, %scale
  %cmp2 = icmp ne i32 %and, %1
  %2 = and i64 %0, 1
  %3 = icmp eq i64 %2, 0
  %tobool.not = xor i1 %3, %cmp2
  br i1 %tobool.not, label %if.end23, label %do.body7

do.body7:                                         ; preds = %do.body
  %4 = bitcast double %add6.i to i64
  %xor13 = xor i64 %4, %0
  %shr14 = lshr i64 %xor13, 62
  %and15 = and i64 %shr14, 2
  %reass.sub = add i64 %0, 1
  %add17 = sub i64 %reass.sub, %and15
  %5 = bitcast i64 %add17 to double
  br label %if.end23

if.end23:                                         ; preds = %do.body, %do.body7, %entry
  %sum.sroa.0.0 = phi double [ %5, %do.body7 ], [ %add.i, %do.body ], [ %add.i, %entry ]
  %call25 = tail call double @ldexp(double noundef %sum.sroa.0.0, i32 noundef %scale) #10
  ret double %call25
}

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #3 = { mustprogress nofree nosync nounwind willreturn memory(none) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { mustprogress nofree nounwind willreturn memory(argmem: write) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #7 = { mustprogress nofree nounwind willreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #8 = { inlinehint nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #9 = { inlinehint mustprogress nofree nounwind willreturn uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #10 = { nounwind }
attributes #11 = { nounwind willreturn memory(none) }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!5 = !{i64 2147578959}
!6 = !{!7, !7, i64 0}
!7 = !{!"short", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{!11, !11, i64 0}
!11 = !{!"int", !8, i64 0}
!12 = !{i64 2147580130}
!13 = !{i64 2147580288}
!14 = !{i64 2147580355}
!15 = !{i64 2147580516}
!16 = !{!17, !17, i64 0}
!17 = !{!"double", !8, i64 0}
