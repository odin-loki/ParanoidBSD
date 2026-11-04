; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/e_hypot_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/e_hypot_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: mustprogress nofree nounwind willreturn memory(write) uwtable
define dso_local double @ref_hypot(double noundef %x, double noundef %y) local_unnamed_addr #0 {
entry:
  %0 = bitcast double %x to i64
  %gh_u.sroa.0.4.extract.shift = lshr i64 %0, 32
  %gh_u.sroa.0.4.extract.trunc = trunc i64 %gh_u.sroa.0.4.extract.shift to i32
  %and = and i32 %gh_u.sroa.0.4.extract.trunc, 2147483647
  %1 = bitcast double %y to i64
  %gh_u2.sroa.0.4.extract.shift = lshr i64 %1, 32
  %gh_u2.sroa.0.4.extract.trunc = trunc i64 %gh_u2.sroa.0.4.extract.shift to i32
  %and6 = and i32 %gh_u2.sroa.0.4.extract.trunc, 2147483647
  %cmp = icmp ugt i32 %and6, %and
  %and.and6 = tail call i32 @llvm.umin.i32(i32 %and6, i32 %and)
  %and6.and = tail call i32 @llvm.umax.i32(i32 %and6, i32 %and)
  %2 = insertelement <2 x i1> poison, i1 %cmp, i64 0
  %3 = shufflevector <2 x i1> %2, <2 x i1> poison, <2 x i32> zeroinitializer
  %4 = insertelement <2 x double> poison, double %x, i64 0
  %5 = insertelement <2 x double> %4, double %y, i64 1
  %6 = shufflevector <2 x double> %5, <2 x double> poison, <2 x i32> <i32 1, i32 0>
  %7 = select <2 x i1> %3, <2 x double> %5, <2 x double> %6
  %8 = tail call <2 x double> @llvm.fabs.v2f64(<2 x double> %7)
  %sub = sub nsw i32 %and6.and, %and.and6
  %cmp7 = icmp sgt i32 %sub, 62914560
  br i1 %cmp7, label %if.then8, label %if.end9

if.then8:                                         ; preds = %entry
  %shift = shufflevector <2 x double> %8, <2 x double> poison, <2 x i32> <i32 1, i32 poison>
  %9 = fadd <2 x double> %shift, %8
  %add = extractelement <2 x double> %9, i64 0
  br label %cleanup141

if.end9:                                          ; preds = %entry
  %cmp10 = icmp ugt i32 %and6.and, 1596981248
  %10 = insertelement <2 x i32> poison, i32 %and.and6, i64 0
  %11 = insertelement <2 x i32> %10, i32 %and6.and, i64 1
  br i1 %cmp10, label %if.then11, label %if.end50

if.then11:                                        ; preds = %if.end9
  %cmp12 = icmp ugt i32 %and6.and, 2146435071
  br i1 %cmp12, label %if.then13, label %if.end37

if.then13:                                        ; preds = %if.then11
  %conv = fpext double %x to x86_fp80
  %add14 = fadd x86_fp80 %conv, 0xK00000000000000000000
  %12 = tail call x86_fp80 @llvm.fabs.f80(x86_fp80 %add14)
  %add15 = fadd double %y, 0.000000e+00
  %13 = tail call double @llvm.fabs.f64(double %add15)
  %conv16 = fpext double %13 to x86_fp80
  %sub17 = fsub x86_fp80 %12, %conv16
  %conv18 = fptrunc x86_fp80 %sub17 to double
  %14 = extractelement <2 x double> %8, i64 1
  %15 = bitcast double %14 to i64
  %gl_u.sroa.0.0.extract.trunc = trunc i64 %15 to i32
  %and22 = and i32 %and6.and, 1048575
  %or = or i32 %and22, %gl_u.sroa.0.0.extract.trunc
  %cmp23 = icmp eq i32 %or, 0
  %w.0 = select i1 %cmp23, double %14, double %conv18
  %16 = extractelement <2 x double> %8, i64 0
  %17 = bitcast double %16 to i64
  %gl_u28.sroa.0.0.extract.trunc = trunc i64 %17 to i32
  %xor = xor i32 %and.and6, 2146435072
  %or32 = or i32 %xor, %gl_u28.sroa.0.0.extract.trunc
  %cmp33 = icmp eq i32 %or32, 0
  %w.1 = select i1 %cmp33, double %16, double %w.0
  br label %cleanup141

if.end37:                                         ; preds = %if.then11
  %18 = add nsw <2 x i32> %11, <i32 -629145600, i32 -629145600>
  %19 = bitcast <2 x double> %8 to <2 x i64>
  %20 = zext <2 x i32> %18 to <2 x i64>
  %21 = shl nuw <2 x i64> %20, <i64 32, i64 32>
  %22 = and <2 x i64> %19, <i64 4294967295, i64 4294967295>
  %23 = or disjoint <2 x i64> %22, %21
  %24 = bitcast <2 x i64> %23 to <2 x double>
  br label %if.end50

if.end50:                                         ; preds = %if.end37, %if.end9
  %k.0 = phi i32 [ 600, %if.end37 ], [ 0, %if.end9 ]
  %25 = phi <2 x double> [ %24, %if.end37 ], [ %8, %if.end9 ]
  %26 = phi <2 x i32> [ %18, %if.end37 ], [ %11, %if.end9 ]
  %27 = extractelement <2 x i32> %26, i64 0
  %cmp51 = icmp slt i32 %27, 548405248
  %28 = extractelement <2 x i32> %26, i64 1
  br i1 %cmp51, label %if.then53, label %if.end90

if.then53:                                        ; preds = %if.end50
  %cmp54 = icmp slt i32 %27, 1048576
  br i1 %cmp54, label %if.then56, label %if.else75

if.then56:                                        ; preds = %if.then53
  %29 = bitcast <2 x double> %25 to <4 x i32>
  %gl_u59.sroa.0.0.extract.trunc = extractelement <4 x i32> %29, i64 0
  %or63 = or i32 %27, %gl_u59.sroa.0.0.extract.trunc
  %cmp64.not = icmp eq i32 %or63, 0
  %30 = fmul <2 x double> %25, <double 0x7FD0000000000000, double 0x7FD0000000000000>
  %sub74 = or disjoint i32 %k.0, -1022
  %31 = extractelement <2 x double> %25, i64 1
  br i1 %cmp64.not, label %cleanup141, label %if.end90

if.else75:                                        ; preds = %if.then53
  %32 = add nuw nsw <2 x i32> %26, <i32 629145600, i32 629145600>
  %sub78 = add nsw i32 %k.0, -600
  %33 = bitcast <2 x double> %25 to <2 x i64>
  %34 = zext <2 x i32> %32 to <2 x i64>
  %35 = shl nuw nsw <2 x i64> %34, <i64 32, i64 32>
  %36 = and <2 x i64> %33, <i64 4294967295, i64 4294967295>
  %37 = or disjoint <2 x i64> %36, %35
  %38 = bitcast <2 x i64> %37 to <2 x double>
  %39 = extractelement <2 x i32> %32, i64 0
  %40 = extractelement <2 x i32> %32, i64 1
  br label %if.end90

if.end90:                                         ; preds = %if.else75, %if.then56, %if.end50
  %hb.2 = phi i32 [ %27, %if.then56 ], [ %39, %if.else75 ], [ %27, %if.end50 ]
  %ha.2 = phi i32 [ %28, %if.then56 ], [ %40, %if.else75 ], [ %28, %if.end50 ]
  %k.2 = phi i32 [ %sub74, %if.then56 ], [ %sub78, %if.else75 ], [ %k.0, %if.end50 ]
  %41 = phi <2 x double> [ %30, %if.then56 ], [ %38, %if.else75 ], [ %25, %if.end50 ]
  %42 = extractelement <2 x double> %41, i64 0
  %43 = extractelement <2 x double> %41, i64 1
  %sub91 = fsub double %43, %42
  %cmp92 = fcmp ogt double %sub91, %42
  br i1 %cmp92, label %if.then94, label %if.else106

if.then94:                                        ; preds = %if.end90
  %sh_u96.sroa.0.4.insert.ext = zext nneg i32 %ha.2 to i64
  %sh_u96.sroa.0.4.insert.shift = shl nuw nsw i64 %sh_u96.sroa.0.4.insert.ext, 32
  %44 = bitcast i64 %sh_u96.sroa.0.4.insert.shift to double
  %sub100 = fsub double %43, %44
  %fneg = fneg double %42
  %add103 = fadd double %43, %44
  %45 = fneg double %sub100
  %neg = fmul double %add103, %45
  %46 = tail call double @llvm.fmuladd.f64(double %42, double %fneg, double %neg)
  %neg105 = fneg double %46
  %47 = tail call double @llvm.fmuladd.f64(double %44, double %44, double %neg105)
  br label %if.end129

if.else106:                                       ; preds = %if.end90
  %add107 = fadd double %43, %43
  %sh_u109.sroa.0.4.insert.ext = zext i32 %hb.2 to i64
  %sh_u109.sroa.0.4.insert.shift = shl nuw i64 %sh_u109.sroa.0.4.insert.ext, 32
  %48 = bitcast i64 %sh_u109.sroa.0.4.insert.shift to double
  %sub113 = fsub double %42, %48
  %add116 = add nuw nsw i32 %ha.2, 1048576
  %sh_u115.sroa.0.4.insert.ext = zext nneg i32 %add116 to i64
  %sh_u115.sroa.0.4.insert.shift = shl nuw nsw i64 %sh_u115.sroa.0.4.insert.ext, 32
  %49 = bitcast i64 %sh_u115.sroa.0.4.insert.shift to double
  %sub120 = fsub double %add107, %49
  %fneg122 = fneg double %sub91
  %mul125 = fmul double %42, %sub120
  %50 = tail call double @llvm.fmuladd.f64(double %49, double %sub113, double %mul125)
  %neg126 = fneg double %50
  %51 = tail call double @llvm.fmuladd.f64(double %sub91, double %fneg122, double %neg126)
  %neg127 = fneg double %51
  %52 = tail call double @llvm.fmuladd.f64(double %49, double %48, double %neg127)
  br label %if.end129

if.end129:                                        ; preds = %if.else106, %if.then94
  %.sink = phi double [ %52, %if.else106 ], [ %47, %if.then94 ]
  %call128 = tail call double @sqrt(double noundef %.sink) #4
  %cmp130.not = icmp eq i32 %k.2, 0
  br i1 %cmp130.not, label %cleanup141, label %if.then132

if.then132:                                       ; preds = %if.end129
  %add135 = shl nsw i32 %k.2, 20
  %shl = add i32 %add135, 1072693248
  %sh_u134.sroa.0.4.insert.ext = zext nneg i32 %shl to i64
  %sh_u134.sroa.0.4.insert.shift = shl nuw nsw i64 %sh_u134.sroa.0.4.insert.ext, 32
  %53 = bitcast i64 %sh_u134.sroa.0.4.insert.shift to double
  %mul139 = fmul double %call128, %53
  br label %cleanup141

cleanup141:                                       ; preds = %if.end129, %if.then56, %if.then132, %if.then13, %if.then8
  %retval.1 = phi double [ %add, %if.then8 ], [ %w.1, %if.then13 ], [ %mul139, %if.then132 ], [ %31, %if.then56 ], [ %call128, %if.end129 ]
  ret double %retval.1
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fabs.f64(double) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare x86_fp80 @llvm.fabs.f80(x86_fp80) #1

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare double @sqrt(double noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fmuladd.f64(double, double, double) #1

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.umin.i32(i32, i32) #3

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.umax.i32(i32, i32) #3

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare <2 x double> @llvm.fabs.v2f64(<2 x double>) #3

attributes #0 = { mustprogress nofree nounwind willreturn memory(write) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
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
