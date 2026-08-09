; ModuleID = '/tmp/port-40b247.pcm'
source_filename = "/tmp/port-40b247.pcm"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@_ZN4pbsd3lib4msun3src5e_expL4hugeE = internal global double 1.000000e+300, align 8
@_ZN4pbsd3lib4msun3src5e_expL8twom1000E = internal global double 0x170000000000000, align 8
@_ZN4pbsd3lib4msun3src5e_expL5ln2HIE = internal unnamed_addr constant [2 x double] [double 0x3FE62E42FEE00000, double 0xBFE62E42FEE00000], align 16
@_ZN4pbsd3lib4msun3src5e_expL5ln2LOE = internal unnamed_addr constant [2 x double] [double 0x3DEA39EF35793C76, double 0xBDEA39EF35793C76], align 16
@_ZN4pbsd3lib4msun3src5e_expL4halFE = internal unnamed_addr constant [2 x double] [double 5.000000e-01, double -5.000000e-01], align 16
@llvm.global_ctors = appending global [0 x { i32, ptr, ptr }] zeroinitializer

; Function Attrs: mustprogress nofree norecurse nounwind willreturn memory(readwrite, argmem: none) uwtable
define dso_local noundef double @_ZN4pbsd3lib4msun3src5e_expW4pbsdW3libW4msunW3srcW5e_exp3expEd(double noundef %x) local_unnamed_addr #0 {
entry:
  %0 = bitcast double %x to i64
  %gh_u.sroa.0.4.extract.shift = lshr i64 %0, 32
  %gh_u.sroa.0.4.extract.trunc = trunc i64 %gh_u.sroa.0.4.extract.shift to i32
  %shr = lshr i32 %gh_u.sroa.0.4.extract.trunc, 31
  %and1 = and i32 %gh_u.sroa.0.4.extract.trunc, 2147483647
  %cmp = icmp ugt i32 %and1, 1082535489
  br i1 %cmp, label %if.then, label %if.end18

if.then:                                          ; preds = %entry
  %cmp2 = icmp ugt i32 %and1, 2146435071
  br i1 %cmp2, label %if.then3, label %if.end

if.then3:                                         ; preds = %if.then
  %gl_u.sroa.0.0.extract.trunc = trunc i64 %0 to i32
  %and7 = and i32 %gh_u.sroa.0.4.extract.trunc, 1048575
  %or = or i32 %and7, %gl_u.sroa.0.0.extract.trunc
  %cmp8 = icmp eq i32 %or, 0
  br i1 %cmp8, label %if.then9, label %if.else

if.then9:                                         ; preds = %if.then3
  %add = fadd double %x, %x
  br label %cleanup98

if.else:                                          ; preds = %if.then3
  %cmp10 = icmp sgt i64 %0, -1
  %cond = select i1 %cmp10, double %x, double 0.000000e+00
  br label %cleanup98

if.end:                                           ; preds = %if.then
  %cmp11 = fcmp ogt double %x, 0x40862E42FEFA39EF
  br i1 %cmp11, label %if.then12, label %if.end13

if.then12:                                        ; preds = %if.end
  %1 = load volatile double, ptr @_ZN4pbsd3lib4msun3src5e_expL4hugeE, align 8, !tbaa !5
  %2 = load volatile double, ptr @_ZN4pbsd3lib4msun3src5e_expL4hugeE, align 8, !tbaa !5
  %mul = fmul double %1, %2
  br label %cleanup98

if.end13:                                         ; preds = %if.end
  %cmp14 = fcmp olt double %x, 0xC0874910D52D3051
  br i1 %cmp14, label %if.then15, label %if.else27

if.then15:                                        ; preds = %if.end13
  %3 = load volatile double, ptr @_ZN4pbsd3lib4msun3src5e_expL8twom1000E, align 8, !tbaa !5
  %4 = load volatile double, ptr @_ZN4pbsd3lib4msun3src5e_expL8twom1000E, align 8, !tbaa !5
  %mul16 = fmul double %3, %4
  br label %cleanup98

if.end18:                                         ; preds = %entry
  %cmp19 = icmp ugt i32 %and1, 1071001154
  br i1 %cmp19, label %if.then20, label %if.else36

if.then20:                                        ; preds = %if.end18
  %cmp21 = icmp ult i32 %and1, 1072734898
  br i1 %cmp21, label %if.then22, label %if.else27

if.then22:                                        ; preds = %if.then20
  %idxprom = zext nneg i32 %shr to i64
  %arrayidx = getelementptr inbounds [2 x double], ptr @_ZN4pbsd3lib4msun3src5e_expL5ln2HIE, i64 0, i64 %idxprom
  %5 = load double, ptr %arrayidx, align 8, !tbaa !5
  %sub = fsub double %x, %5
  %arrayidx24 = getelementptr inbounds [2 x double], ptr @_ZN4pbsd3lib4msun3src5e_expL5ln2LOE, i64 0, i64 %idxprom
  %6 = load double, ptr %arrayidx24, align 8, !tbaa !5
  %sub25 = xor i32 %shr, 1
  %sub26 = sub nsw i32 %sub25, %shr
  br label %if.end34

if.else27:                                        ; preds = %if.end13, %if.then20
  %idxprom29 = zext nneg i32 %shr to i64
  %arrayidx30 = getelementptr inbounds [2 x double], ptr @_ZN4pbsd3lib4msun3src5e_expL4halFE, i64 0, i64 %idxprom29
  %7 = load double, ptr %arrayidx30, align 8, !tbaa !5
  %8 = tail call double @llvm.fmuladd.f64(double %x, double 0x3FF71547652B82FE, double %7)
  %conv = fptosi double %8 to i32
  %conv31 = sitofp i32 %conv to double
  %neg = fneg double %conv31
  %9 = tail call double @llvm.fmuladd.f64(double %neg, double 0x3FE62E42FEE00000, double %x)
  %mul33 = fmul double %conv31, 0x3DEA39EF35793C76
  br label %if.end34

if.end34:                                         ; preds = %if.else27, %if.then22
  %k.0 = phi i32 [ %sub26, %if.then22 ], [ %conv, %if.else27 ]
  %lo.0 = phi double [ %6, %if.then22 ], [ %mul33, %if.else27 ]
  %hi.0 = phi double [ %sub, %if.then22 ], [ %9, %if.else27 ]
  %sub35 = fsub double %hi.0, %lo.0
  br label %if.end46

if.else36:                                        ; preds = %if.end18
  %cmp37 = icmp ult i32 %and1, 1043333120
  br i1 %cmp37, label %if.then38, label %if.end46

if.then38:                                        ; preds = %if.else36
  %10 = load volatile double, ptr @_ZN4pbsd3lib4msun3src5e_expL4hugeE, align 8, !tbaa !5
  %add39 = fadd double %10, %x
  %cmp40 = fcmp ogt double %add39, 1.000000e+00
  br i1 %cmp40, label %if.then41, label %if.end46

if.then41:                                        ; preds = %if.then38
  %add42 = fadd double %x, 1.000000e+00
  br label %cleanup98

if.end46:                                         ; preds = %if.else36, %if.then38, %if.end34
  %k.1 = phi i32 [ %k.0, %if.end34 ], [ 0, %if.then38 ], [ 0, %if.else36 ]
  %lo.1 = phi double [ %lo.0, %if.end34 ], [ 0.000000e+00, %if.then38 ], [ 0.000000e+00, %if.else36 ]
  %hi.1 = phi double [ %hi.0, %if.end34 ], [ 0.000000e+00, %if.then38 ], [ 0.000000e+00, %if.else36 ]
  %x.addr.0 = phi double [ %sub35, %if.end34 ], [ %x, %if.then38 ], [ %x, %if.else36 ]
  %mul47 = fmul double %x.addr.0, %x.addr.0
  %cmp48 = icmp sgt i32 %k.1, -1022
  %add51 = shl i32 %k.1, 20
  %twopk.0.in.in.in.v = select i1 %cmp48, i32 1072693248, i32 2121269248
  %twopk.0.in.in.in = add i32 %twopk.0.in.in.in.v, %add51
  %twopk.0.in.in = zext i32 %twopk.0.in.in.in to i64
  %twopk.0.in = shl nuw i64 %twopk.0.in.in, 32
  %twopk.0 = bitcast i64 %twopk.0.in to double
  %11 = tail call double @llvm.fmuladd.f64(double %mul47, double 0x3E66376972BEA4D0, double 0xBEBBBD41C5D26BF1)
  %12 = tail call double @llvm.fmuladd.f64(double %mul47, double %11, double 0x3F11566AAF25DE2C)
  %13 = tail call double @llvm.fmuladd.f64(double %mul47, double %12, double 0xBF66C16C16BEBD93)
  %14 = tail call double @llvm.fmuladd.f64(double %mul47, double %13, double 0x3FC555555555553E)
  %neg72 = fneg double %mul47
  %15 = tail call double @llvm.fmuladd.f64(double %neg72, double %14, double %x.addr.0)
  %cmp73 = icmp eq i32 %k.1, 0
  %mul75 = fmul double %x.addr.0, %15
  br i1 %cmp73, label %if.then74, label %if.else79

if.then74:                                        ; preds = %if.end46
  %sub76 = fadd double %15, -2.000000e+00
  %div = fdiv double %mul75, %sub76
  %16 = fsub double %x.addr.0, %div
  %sub78 = fadd double %16, 1.000000e+00
  br label %cleanup98

if.else79:                                        ; preds = %if.end46
  %sub81 = fsub double 2.000000e+00, %15
  %div82 = fdiv double %mul75, %sub81
  %sub83 = fsub double %lo.1, %div82
  %17 = fsub double %hi.1, %sub83
  %sub85 = fadd double %17, 1.000000e+00
  br i1 %cmp48, label %if.then88, label %if.else95

if.then88:                                        ; preds = %if.else79
  %cmp89 = icmp eq i32 %k.1, 1024
  br i1 %cmp89, label %if.then90, label %if.end93

if.then90:                                        ; preds = %if.then88
  %mul91 = fmul double %sub85, 2.000000e+00
  %mul92 = fmul double %mul91, 0x7FE0000000000000
  br label %cleanup98

if.end93:                                         ; preds = %if.then88
  %mul94 = fmul double %sub85, %twopk.0
  br label %cleanup98

if.else95:                                        ; preds = %if.else79
  %mul96 = fmul double %sub85, %twopk.0
  %18 = load volatile double, ptr @_ZN4pbsd3lib4msun3src5e_expL8twom1000E, align 8, !tbaa !5
  %mul97 = fmul double %mul96, %18
  br label %cleanup98

cleanup98:                                        ; preds = %if.then9, %if.else, %if.else95, %if.end93, %if.then90, %if.then74, %if.then41, %if.then15, %if.then12
  %retval.1 = phi double [ %mul, %if.then12 ], [ %mul16, %if.then15 ], [ %sub78, %if.then74 ], [ %mul92, %if.then90 ], [ %mul94, %if.end93 ], [ %mul97, %if.else95 ], [ %add42, %if.then41 ], [ %add, %if.then9 ], [ %cond, %if.else ]
  ret double %retval.1
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fmuladd.f64(double, double, double) #1

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none)
define void @_ZGIW4pbsdW3libW4msunW3srcW5e_exp() local_unnamed_addr #2 section ".text.startup" {
entry:
  ret void
}

attributes #0 = { mustprogress nofree norecurse nounwind willreturn memory(readwrite, argmem: none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) "min-legal-vector-width"="0" }

!llvm.linker.options = !{}
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
!8 = !{!"Simple C++ TBAA"}
