; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/s_sinpi_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/s_sinpi_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@vzero = internal constant double 0.000000e+00, align 8

; Function Attrs: nounwind uwtable
define dso_local double @ref_sinpi(double noundef %x) local_unnamed_addr #0 {
entry:
  %0 = bitcast double %x to i64
  %ew_u.sroa.0.4.extract.shift = lshr i64 %0, 32
  %ew_u.sroa.0.4.extract.trunc = trunc i64 %ew_u.sroa.0.4.extract.shift to i32
  %ew_u.sroa.0.0.extract.trunc = trunc i64 %0 to i32
  %and = and i32 %ew_u.sroa.0.4.extract.trunc, 2147483647
  %1 = tail call double @llvm.fabs.f64(double %x)
  %cmp = icmp ult i32 %and, 1072693248
  br i1 %cmp, label %if.then, label %if.end45

if.then:                                          ; preds = %entry
  %cmp6 = icmp ult i32 %and, 1070596096
  br i1 %cmp6, label %if.then7, label %if.end25

if.then7:                                         ; preds = %if.then
  %cmp8 = icmp ult i32 %and, 1042284544
  br i1 %cmp8, label %if.then9, label %if.end23

if.then9:                                         ; preds = %if.then7
  %cmp10 = fcmp oeq double %x, 0.000000e+00
  br i1 %cmp10, label %cleanup, label %do.body12

do.body12:                                        ; preds = %if.then9
  %iw_u13.sroa.0.4.insert.shift = and i64 %0, -4294967296
  %2 = bitcast i64 %iw_u13.sroa.0.4.insert.shift to double
  %mul = fmul double %2, 0x4340000000000000
  %neg = fneg double %mul
  %3 = tail call double @llvm.fmuladd.f64(double %x, double 0x4340000000000000, double %neg)
  %mul20 = fmul double %mul, 0xBE5DDE973DCB3B3A
  %4 = tail call double @llvm.fmuladd.f64(double %3, double 0x400921FB54442D18, double %mul20)
  %5 = tail call double @llvm.fmuladd.f64(double %mul, double 0x400921FB58000000, double %4)
  %mul22 = fmul double %5, 0x3CA0000000000000
  br label %cleanup

if.end23:                                         ; preds = %if.then7
  %conv.i = fptrunc double %1 to float
  %conv1.i = fpext float %conv.i to double
  %sub.i = fsub double %1, %conv1.i
  %mul2.i = fmul double %conv1.i, 0xBE5DDE973DCB3B3A
  %6 = tail call double @llvm.fmuladd.f64(double %sub.i, double 0x400921FB54442D18, double %mul2.i)
  %mul.i = fmul double %conv1.i, 0x400921FB58000000
  %add.i = fadd double %mul.i, %6
  %sub3.i = fsub double %mul.i, %add.i
  %add4.i = fadd double %6, %sub3.i
  %call.i = tail call double @__kernel_sin(double noundef %add.i, double noundef %add4.i, i32 noundef 1) #4
  %fneg = fneg double %call.i
  %tobool.not179 = icmp slt i64 %0, 0
  %cond = select i1 %tobool.not179, double %fneg, double %call.i
  br label %cleanup

if.end25:                                         ; preds = %if.then
  %cmp26 = icmp ult i32 %and, 1071644672
  br i1 %cmp26, label %if.then27, label %if.else

if.then27:                                        ; preds = %if.end25
  %sub = fsub double 5.000000e-01, %1
  %conv.i180 = fptrunc double %sub to float
  %conv1.i181 = fpext float %conv.i180 to double
  %sub.i182 = fsub double %sub, %conv1.i181
  %mul2.i183 = fmul double %conv1.i181, 0xBE5DDE973DCB3B3A
  %7 = tail call double @llvm.fmuladd.f64(double %sub.i182, double 0x400921FB54442D18, double %mul2.i183)
  %mul.i184 = fmul double %conv1.i181, 0x400921FB58000000
  %add.i185 = fadd double %mul.i184, %7
  %sub3.i186 = fsub double %mul.i184, %add.i185
  %add4.i187 = fadd double %7, %sub3.i186
  %call.i188 = tail call double @__kernel_cos(double noundef %add.i185, double noundef %add4.i187) #4
  br label %if.end37

if.else:                                          ; preds = %if.end25
  %cmp29 = icmp ult i32 %and, 1072168960
  br i1 %cmp29, label %if.then30, label %if.else33

if.then30:                                        ; preds = %if.else
  %sub31 = fadd double %1, -5.000000e-01
  %conv.i189 = fptrunc double %sub31 to float
  %conv1.i190 = fpext float %conv.i189 to double
  %sub.i191 = fsub double %sub31, %conv1.i190
  %mul2.i192 = fmul double %conv1.i190, 0xBE5DDE973DCB3B3A
  %8 = tail call double @llvm.fmuladd.f64(double %sub.i191, double 0x400921FB54442D18, double %mul2.i192)
  %mul.i193 = fmul double %conv1.i190, 0x400921FB58000000
  %add.i194 = fadd double %mul.i193, %8
  %sub3.i195 = fsub double %mul.i193, %add.i194
  %add4.i196 = fadd double %8, %sub3.i195
  %call.i197 = tail call double @__kernel_cos(double noundef %add.i194, double noundef %add4.i196) #4
  br label %if.end37

if.else33:                                        ; preds = %if.else
  %sub34 = fsub double 1.000000e+00, %1
  %conv.i198 = fptrunc double %sub34 to float
  %conv1.i199 = fpext float %conv.i198 to double
  %sub.i200 = fsub double %sub34, %conv1.i199
  %mul2.i201 = fmul double %conv1.i199, 0xBE5DDE973DCB3B3A
  %9 = tail call double @llvm.fmuladd.f64(double %sub.i200, double 0x400921FB54442D18, double %mul2.i201)
  %mul.i202 = fmul double %conv1.i199, 0x400921FB58000000
  %add.i203 = fadd double %mul.i202, %9
  %sub3.i204 = fsub double %mul.i202, %add.i203
  %add4.i205 = fadd double %9, %sub3.i204
  %call.i206 = tail call double @__kernel_sin(double noundef %add.i203, double noundef %add4.i205, i32 noundef 1) #4
  br label %if.end37

if.end37:                                         ; preds = %if.then30, %if.else33, %if.then27
  %s.0 = phi double [ %call.i188, %if.then27 ], [ %call.i197, %if.then30 ], [ %call.i206, %if.else33 ]
  %fneg41 = fneg double %s.0
  %tobool39.not178 = icmp slt i64 %0, 0
  %cond44 = select i1 %tobool39.not178, double %fneg41, double %s.0
  br label %cleanup

if.end45:                                         ; preds = %entry
  %cmp46 = icmp ult i32 %and, 1127219200
  br i1 %cmp46, label %do.body48, label %if.end115

do.body48:                                        ; preds = %if.end45
  %shr = lshr i32 %ew_u.sroa.0.4.extract.trunc, 20
  %and49 = and i32 %shr, 2047
  %sub50 = add nsw i32 %and49, -1023
  %cmp51 = icmp ult i32 %sub50, 20
  br i1 %cmp51, label %if.then52, label %if.else55

if.then52:                                        ; preds = %do.body48
  %not = ashr i32 -1048576, %sub50
  %and54 = and i32 %not, %and
  br label %do.body61

if.else55:                                        ; preds = %do.body48
  %sub56 = add nsw i32 %and49, -1043
  %shr57 = lshr i32 -1, %sub56
  %not58 = xor i32 %shr57, -1
  %and59 = and i32 %not58, %ew_u.sroa.0.0.extract.trunc
  %10 = zext i32 %and59 to i64
  br label %do.body61

do.body61:                                        ; preds = %if.then52, %if.else55
  %ix.0 = phi i32 [ %and54, %if.then52 ], [ %and, %if.else55 ]
  %lx.0 = phi i64 [ 0, %if.then52 ], [ %10, %if.else55 ]
  %iw_u62.sroa.0.4.insert.ext = zext nneg i32 %ix.0 to i64
  %iw_u62.sroa.0.4.insert.shift = shl nuw nsw i64 %iw_u62.sroa.0.4.insert.ext, 32
  %iw_u62.sroa.0.0.insert.insert = or disjoint i64 %iw_u62.sroa.0.4.insert.shift, %lx.0
  %11 = bitcast i64 %iw_u62.sroa.0.0.insert.insert to double
  %sub69 = fsub double %1, %11
  %12 = bitcast double %sub69 to i64
  %ew_u71.sroa.0.4.extract.shift = lshr i64 %12, 32
  %ew_u71.sroa.0.4.extract.trunc = trunc i64 %ew_u71.sroa.0.4.extract.shift to i32
  %cmp76 = icmp eq i32 %ew_u71.sroa.0.4.extract.trunc, 0
  br i1 %cmp76, label %if.end107, label %if.else78

if.else78:                                        ; preds = %do.body61
  %cmp79 = icmp ult i32 %ew_u71.sroa.0.4.extract.trunc, 1071644672
  br i1 %cmp79, label %if.then80, label %if.else88

if.then80:                                        ; preds = %if.else78
  %cmp81 = icmp ult i32 %ew_u71.sroa.0.4.extract.trunc, 1070596096
  br i1 %cmp81, label %if.then82, label %if.else84

if.then82:                                        ; preds = %if.then80
  %conv.i207 = fptrunc double %sub69 to float
  %conv1.i208 = fpext float %conv.i207 to double
  %sub.i209 = fsub double %sub69, %conv1.i208
  %mul2.i210 = fmul double %conv1.i208, 0xBE5DDE973DCB3B3A
  %13 = tail call double @llvm.fmuladd.f64(double %sub.i209, double 0x400921FB54442D18, double %mul2.i210)
  %mul.i211 = fmul double %conv1.i208, 0x400921FB58000000
  %add.i212 = fadd double %mul.i211, %13
  %sub3.i213 = fsub double %mul.i211, %add.i212
  %add4.i214 = fadd double %13, %sub3.i213
  %call.i215 = tail call double @__kernel_sin(double noundef %add.i212, double noundef %add4.i214, i32 noundef 1) #4
  br label %if.end97

if.else84:                                        ; preds = %if.then80
  %sub85 = fsub double 5.000000e-01, %sub69
  %conv.i216 = fptrunc double %sub85 to float
  %conv1.i217 = fpext float %conv.i216 to double
  %sub.i218 = fsub double %sub85, %conv1.i217
  %mul2.i219 = fmul double %conv1.i217, 0xBE5DDE973DCB3B3A
  %14 = tail call double @llvm.fmuladd.f64(double %sub.i218, double 0x400921FB54442D18, double %mul2.i219)
  %mul.i220 = fmul double %conv1.i217, 0x400921FB58000000
  %add.i221 = fadd double %mul.i220, %14
  %sub3.i222 = fsub double %mul.i220, %add.i221
  %add4.i223 = fadd double %14, %sub3.i222
  %call.i224 = tail call double @__kernel_cos(double noundef %add.i221, double noundef %add4.i223) #4
  br label %if.end97

if.else88:                                        ; preds = %if.else78
  %cmp89 = icmp ult i32 %ew_u71.sroa.0.4.extract.trunc, 1072168960
  br i1 %cmp89, label %if.then90, label %if.else93

if.then90:                                        ; preds = %if.else88
  %sub91 = fadd double %sub69, -5.000000e-01
  %conv.i225 = fptrunc double %sub91 to float
  %conv1.i226 = fpext float %conv.i225 to double
  %sub.i227 = fsub double %sub91, %conv1.i226
  %mul2.i228 = fmul double %conv1.i226, 0xBE5DDE973DCB3B3A
  %15 = tail call double @llvm.fmuladd.f64(double %sub.i227, double 0x400921FB54442D18, double %mul2.i228)
  %mul.i229 = fmul double %conv1.i226, 0x400921FB58000000
  %add.i230 = fadd double %mul.i229, %15
  %sub3.i231 = fsub double %mul.i229, %add.i230
  %add4.i232 = fadd double %15, %sub3.i231
  %call.i233 = tail call double @__kernel_cos(double noundef %add.i230, double noundef %add4.i232) #4
  br label %if.end97

if.else93:                                        ; preds = %if.else88
  %sub94 = fsub double 1.000000e+00, %sub69
  %conv.i234 = fptrunc double %sub94 to float
  %conv1.i235 = fpext float %conv.i234 to double
  %sub.i236 = fsub double %sub94, %conv1.i235
  %mul2.i237 = fmul double %conv1.i235, 0xBE5DDE973DCB3B3A
  %16 = tail call double @llvm.fmuladd.f64(double %sub.i236, double 0x400921FB54442D18, double %mul2.i237)
  %mul.i238 = fmul double %conv1.i235, 0x400921FB58000000
  %add.i239 = fadd double %mul.i238, %16
  %sub3.i240 = fsub double %mul.i238, %add.i239
  %add4.i241 = fadd double %16, %sub3.i240
  %call.i242 = tail call double @__kernel_sin(double noundef %add.i239, double noundef %add4.i241, i32 noundef 1) #4
  br label %if.end97

if.end97:                                         ; preds = %if.then90, %if.else93, %if.then82, %if.else84
  %s.1 = phi double [ %call.i215, %if.then82 ], [ %call.i224, %if.else84 ], [ %call.i233, %if.then90 ], [ %call.i242, %if.else93 ]
  %cmp98 = icmp ugt i32 %sub50, 30
  %sub100 = fadd double %11, 0xC1D0000000000000
  %x.addr.0 = select i1 %cmp98, double %sub100, double %11
  %conv = fptoui double %x.addr.0 to i32
  %and102 = and i32 %conv, 1
  %tobool103.not = icmp eq i32 %and102, 0
  br i1 %tobool103.not, label %if.end107, label %if.then104

if.then104:                                       ; preds = %if.end97
  %fneg105 = fneg double %s.1
  br label %if.end107

if.end107:                                        ; preds = %do.body61, %if.end97, %if.then104
  %s.2 = phi double [ %fneg105, %if.then104 ], [ %s.1, %if.end97 ], [ 0.000000e+00, %do.body61 ]
  %fneg111 = fneg double %s.2
  %tobool109.not177 = icmp slt i64 %0, 0
  %cond114 = select i1 %tobool109.not177, double %fneg111, double %s.2
  br label %cleanup

if.end115:                                        ; preds = %if.end45
  %cmp116 = icmp ugt i32 %and, 2146435071
  br i1 %cmp116, label %if.then118, label %if.end119

if.then118:                                       ; preds = %if.end115
  %17 = load volatile double, ptr @vzero, align 8, !tbaa !5
  %18 = load volatile double, ptr @vzero, align 8, !tbaa !5
  %div = fdiv double %17, %18
  br label %cleanup

if.end119:                                        ; preds = %if.end115
  %19 = tail call double @llvm.copysign.f64(double 0.000000e+00, double %x)
  br label %cleanup

cleanup:                                          ; preds = %if.then9, %if.end119, %if.then118, %if.end107, %if.end37, %if.end23, %do.body12
  %retval.0 = phi double [ %mul22, %do.body12 ], [ %cond, %if.end23 ], [ %cond44, %if.end37 ], [ %cond114, %if.end107 ], [ %div, %if.then118 ], [ %19, %if.end119 ], [ %x, %if.then9 ]
  ret double %retval.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fmuladd.f64(double, double, double) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.copysign.f64(double, double) #1

declare double @__kernel_sin(double noundef, double noundef, i32 noundef) local_unnamed_addr #2

declare double @__kernel_cos(double noundef, double noundef) local_unnamed_addr #2

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fabs.f64(double) #3

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
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
!6 = !{!"double", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
