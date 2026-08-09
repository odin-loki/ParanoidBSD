; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/s_tanf_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/s_tanf_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local float @ref_tanf(float noundef %x) local_unnamed_addr #0 {
entry:
  %tx.i = alloca [1 x double], align 8
  %ty.i = alloca [1 x double], align 8
  %0 = bitcast float %x to i32
  %1 = tail call float @llvm.fabs.f32(float %x)
  %and = bitcast float %1 to i32
  %cmp = icmp slt i32 %and, 1061752795
  br i1 %cmp, label %if.then, label %if.end8

if.then:                                          ; preds = %entry
  %cmp1 = icmp slt i32 %and, 964689920
  %conv = fptosi float %x to i32
  %cmp3 = icmp eq i32 %conv, 0
  %or.cond = select i1 %cmp1, i1 %cmp3, i1 false
  br i1 %or.cond, label %cleanup, label %if.end6

if.end6:                                          ; preds = %if.then
  %conv7 = fpext float %x to double
  %mul.i77 = fmul double %conv7, %conv7
  %2 = tail call double @llvm.fmuladd.f64(double %mul.i77, double 0x3F8362B9BF971BCD, double 0x3F685DADFCECF44E)
  %3 = tail call double @llvm.fmuladd.f64(double %mul.i77, double 0x3F991DF3908C33CE, double 0x3FAB54C91D865AFE)
  %mul3.i = fmul double %mul.i77, %mul.i77
  %mul4.i78 = fmul double %mul.i77, %conv7
  %4 = tail call double @llvm.fmuladd.f64(double %mul.i77, double 0x3FC112FD38999F72, double 0x3FD5554D3418C99F)
  %5 = tail call double @llvm.fmuladd.f64(double %mul4.i78, double %4, double %conv7)
  %mul7.i = fmul double %mul4.i78, %mul3.i
  %6 = tail call double @llvm.fmuladd.f64(double %mul3.i, double %2, double %3)
  %7 = tail call double @llvm.fmuladd.f64(double %mul7.i, double %6, double %5)
  %retval.0.i79 = fptrunc double %7 to float
  br label %cleanup

if.end8:                                          ; preds = %entry
  %cmp9 = icmp ult i32 %and, 1081824210
  br i1 %cmp9, label %if.then11, label %if.end25

if.then11:                                        ; preds = %if.end8
  %cmp12 = icmp ult i32 %and, 1075235812
  %conv15 = fpext float %x to double
  %cmp16 = icmp sgt i32 %0, 0
  br i1 %cmp12, label %if.then14, label %if.else

if.then14:                                        ; preds = %if.then11
  %cond = select i1 %cmp16, double 0xBFF921FB54442D18, double 0x3FF921FB54442D18
  %add = fadd double %cond, %conv15
  %mul.i80 = fmul double %add, %add
  %8 = tail call double @llvm.fmuladd.f64(double %mul.i80, double 0x3F8362B9BF971BCD, double 0x3F685DADFCECF44E)
  %9 = tail call double @llvm.fmuladd.f64(double %mul.i80, double 0x3F991DF3908C33CE, double 0x3FAB54C91D865AFE)
  %mul3.i81 = fmul double %mul.i80, %mul.i80
  %mul4.i82 = fmul double %add, %mul.i80
  %10 = tail call double @llvm.fmuladd.f64(double %mul.i80, double 0x3FC112FD38999F72, double 0x3FD5554D3418C99F)
  %11 = tail call double @llvm.fmuladd.f64(double %mul4.i82, double %10, double %add)
  %mul7.i83 = fmul double %mul4.i82, %mul3.i81
  %12 = tail call double @llvm.fmuladd.f64(double %mul3.i81, double %8, double %9)
  %13 = tail call double @llvm.fmuladd.f64(double %mul7.i83, double %12, double %11)
  %div.i84 = fdiv double -1.000000e+00, %13
  %retval.0.i85 = fptrunc double %div.i84 to float
  br label %cleanup

if.else:                                          ; preds = %if.then11
  %cond22 = select i1 %cmp16, double 0xC00921FB54442D18, double 0x400921FB54442D18
  %add23 = fadd double %cond22, %conv15
  %mul.i86 = fmul double %add23, %add23
  %14 = tail call double @llvm.fmuladd.f64(double %mul.i86, double 0x3F8362B9BF971BCD, double 0x3F685DADFCECF44E)
  %15 = tail call double @llvm.fmuladd.f64(double %mul.i86, double 0x3F991DF3908C33CE, double 0x3FAB54C91D865AFE)
  %mul3.i87 = fmul double %mul.i86, %mul.i86
  %mul4.i88 = fmul double %add23, %mul.i86
  %16 = tail call double @llvm.fmuladd.f64(double %mul.i86, double 0x3FC112FD38999F72, double 0x3FD5554D3418C99F)
  %17 = tail call double @llvm.fmuladd.f64(double %mul4.i88, double %16, double %add23)
  %mul7.i89 = fmul double %mul4.i88, %mul3.i87
  %18 = tail call double @llvm.fmuladd.f64(double %mul3.i87, double %14, double %15)
  %19 = tail call double @llvm.fmuladd.f64(double %mul7.i89, double %18, double %17)
  %retval.0.i91 = fptrunc double %19 to float
  br label %cleanup

if.end25:                                         ; preds = %if.end8
  %cmp26 = icmp ult i32 %and, 1088565718
  br i1 %cmp26, label %if.then28, label %if.else45

if.then28:                                        ; preds = %if.end25
  %cmp29 = icmp ult i32 %and, 1085271520
  %conv32 = fpext float %x to double
  %cmp33 = icmp sgt i32 %0, 0
  br i1 %cmp29, label %if.then31, label %if.else38

if.then31:                                        ; preds = %if.then28
  %cond35 = select i1 %cmp33, double 0xC012D97C7F3321D2, double 0x4012D97C7F3321D2
  %add36 = fadd double %cond35, %conv32
  %mul.i92 = fmul double %add36, %add36
  %20 = tail call double @llvm.fmuladd.f64(double %mul.i92, double 0x3F8362B9BF971BCD, double 0x3F685DADFCECF44E)
  %21 = tail call double @llvm.fmuladd.f64(double %mul.i92, double 0x3F991DF3908C33CE, double 0x3FAB54C91D865AFE)
  %mul3.i93 = fmul double %mul.i92, %mul.i92
  %mul4.i94 = fmul double %add36, %mul.i92
  %22 = tail call double @llvm.fmuladd.f64(double %mul.i92, double 0x3FC112FD38999F72, double 0x3FD5554D3418C99F)
  %23 = tail call double @llvm.fmuladd.f64(double %mul4.i94, double %22, double %add36)
  %mul7.i95 = fmul double %mul4.i94, %mul3.i93
  %24 = tail call double @llvm.fmuladd.f64(double %mul3.i93, double %20, double %21)
  %25 = tail call double @llvm.fmuladd.f64(double %mul7.i95, double %24, double %23)
  %div.i96 = fdiv double -1.000000e+00, %25
  %retval.0.i97 = fptrunc double %div.i96 to float
  br label %cleanup

if.else38:                                        ; preds = %if.then28
  %cond42 = select i1 %cmp33, double 0xC01921FB54442D18, double 0x401921FB54442D18
  %add43 = fadd double %cond42, %conv32
  %mul.i98 = fmul double %add43, %add43
  %26 = tail call double @llvm.fmuladd.f64(double %mul.i98, double 0x3F8362B9BF971BCD, double 0x3F685DADFCECF44E)
  %27 = tail call double @llvm.fmuladd.f64(double %mul.i98, double 0x3F991DF3908C33CE, double 0x3FAB54C91D865AFE)
  %mul3.i99 = fmul double %mul.i98, %mul.i98
  %mul4.i100 = fmul double %add43, %mul.i98
  %28 = tail call double @llvm.fmuladd.f64(double %mul.i98, double 0x3FC112FD38999F72, double 0x3FD5554D3418C99F)
  %29 = tail call double @llvm.fmuladd.f64(double %mul4.i100, double %28, double %add43)
  %mul7.i101 = fmul double %mul4.i100, %mul3.i99
  %30 = tail call double @llvm.fmuladd.f64(double %mul3.i99, double %26, double %27)
  %31 = tail call double @llvm.fmuladd.f64(double %mul7.i101, double %30, double %29)
  %retval.0.i103 = fptrunc double %31 to float
  br label %cleanup

if.else45:                                        ; preds = %if.end25
  %cmp46 = icmp ugt i32 %and, 2139095039
  br i1 %cmp46, label %if.then48, label %if.else49

if.then48:                                        ; preds = %if.else45
  %sub = fsub float %x, %x
  br label %cleanup

if.else49:                                        ; preds = %if.else45
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %tx.i) #5
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %ty.i) #5
  %cmp.i = icmp ult i32 %and, 1305022427
  br i1 %cmp.i, label %if.then.i, label %if.end10.i

if.then.i:                                        ; preds = %if.else49
  %conv.i = fpext float %x to double
  %mul.i = fmul double %conv.i, 0x3FE45F306DC9C883
  %add.i = fadd double %mul.i, 0x4338000000000000
  %sub.i104 = fadd double %add.i, 0xC338000000000000
  %conv1.i = fptosi double %sub.i104 to i32
  %neg.i = fneg double %sub.i104
  %32 = tail call double @llvm.fmuladd.f64(double %neg.i, double 0x3FF921FB50000000, double %conv.i)
  %mul4.i = fmul double %sub.i104, 0x3E5110B4611A6263
  %sub.i = fsub double %32, %mul4.i
  br label %__ieee754_rem_pio2f.exit

if.end10.i:                                       ; preds = %if.else49
  %shr.i = lshr i32 %and, 23
  %sub11.i = add nsw i32 %shr.i, -150
  %shl.i = shl nuw nsw i32 %sub11.i, 23
  %sub13.i = sub nuw nsw i32 %and, %shl.i
  %33 = bitcast i32 %sub13.i to float
  %conv16.i = fpext float %33 to double
  store double %conv16.i, ptr %tx.i, align 8, !tbaa !5
  %call18.i = call i32 @__kernel_rem_pio2(ptr noundef nonnull %tx.i, ptr noundef nonnull %ty.i, i32 noundef %sub11.i, i32 noundef 1, i32 noundef 0) #5
  %cmp19.i = icmp slt i32 %0, 0
  %34 = load double, ptr %ty.i, align 8, !tbaa !5
  br i1 %cmp19.i, label %if.then21.i, label %__ieee754_rem_pio2f.exit

if.then21.i:                                      ; preds = %if.end10.i
  %fneg.i = fneg double %34
  %sub23.i = sub nsw i32 0, %call18.i
  br label %__ieee754_rem_pio2f.exit

__ieee754_rem_pio2f.exit:                         ; preds = %if.end10.i, %if.then.i, %if.then21.i
  %y.0 = phi double [ %sub.i, %if.then.i ], [ %fneg.i, %if.then21.i ], [ %34, %if.end10.i ]
  %retval.0.i = phi i32 [ %conv1.i, %if.then.i ], [ %sub23.i, %if.then21.i ], [ %call18.i, %if.end10.i ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %ty.i) #5
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %tx.i) #5
  %mul.i105 = fmul double %y.0, %y.0
  %35 = call double @llvm.fmuladd.f64(double %mul.i105, double 0x3F8362B9BF971BCD, double 0x3F685DADFCECF44E)
  %36 = call double @llvm.fmuladd.f64(double %mul.i105, double 0x3F991DF3908C33CE, double 0x3FAB54C91D865AFE)
  %mul3.i106 = fmul double %mul.i105, %mul.i105
  %mul4.i107 = fmul double %y.0, %mul.i105
  %37 = call double @llvm.fmuladd.f64(double %mul.i105, double 0x3FC112FD38999F72, double 0x3FD5554D3418C99F)
  %38 = call double @llvm.fmuladd.f64(double %mul4.i107, double %37, double %y.0)
  %mul7.i108 = fmul double %mul4.i107, %mul3.i106
  %39 = call double @llvm.fmuladd.f64(double %mul3.i106, double %35, double %36)
  %40 = call double @llvm.fmuladd.f64(double %mul7.i108, double %39, double %38)
  %41 = and i32 %retval.0.i, 1
  %cmp.i109 = icmp eq i32 %41, 0
  %div.i110 = fdiv double -1.000000e+00, %40
  %retval.0.in.i = select i1 %cmp.i109, double %40, double %div.i110
  %retval.0.i111 = fptrunc double %retval.0.in.i to float
  br label %cleanup

cleanup:                                          ; preds = %if.then, %__ieee754_rem_pio2f.exit, %if.then48, %if.else38, %if.then31, %if.else, %if.then14, %if.end6
  %retval.0 = phi float [ %retval.0.i79, %if.end6 ], [ %retval.0.i85, %if.then14 ], [ %retval.0.i91, %if.else ], [ %retval.0.i97, %if.then31 ], [ %retval.0.i103, %if.else38 ], [ %sub, %if.then48 ], [ %retval.0.i111, %__ieee754_rem_pio2f.exit ], [ %x, %if.then ]
  ret float %retval.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fmuladd.f64(double, double, double) #2

declare i32 @__kernel_rem_pio2(ptr noundef, ptr noundef, i32 noundef, i32 noundef, i32 noundef) local_unnamed_addr #3

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fabs.f32(float) #4

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #3 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #5 = { nounwind }

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
