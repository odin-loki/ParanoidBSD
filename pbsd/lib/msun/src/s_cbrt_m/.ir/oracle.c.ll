; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/s_cbrt_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/s_cbrt_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local double @ref_cbrt(double noundef %x) local_unnamed_addr #0 {
entry:
  %0 = bitcast double %x to i64
  %ew_u.sroa.0.4.extract.shift = lshr i64 %0, 32
  %ew_u.sroa.0.4.extract.trunc = trunc i64 %ew_u.sroa.0.4.extract.shift to i32
  %ew_u.sroa.0.0.extract.trunc = trunc i64 %0 to i32
  %and = and i32 %ew_u.sroa.0.4.extract.trunc, -2147483648
  %xor = and i32 %ew_u.sroa.0.4.extract.trunc, 2147483647
  %cmp = icmp ugt i32 %xor, 2146435071
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %add = fadd double %x, %x
  br label %cleanup

if.end:                                           ; preds = %entry
  %cmp1 = icmp ult i32 %xor, 1048576
  br i1 %cmp1, label %if.then2, label %if.end31

if.then2:                                         ; preds = %if.end
  %or = or i32 %xor, %ew_u.sroa.0.0.extract.trunc
  %cmp3 = icmp eq i32 %or, 0
  br i1 %cmp3, label %cleanup, label %do.body6

do.body6:                                         ; preds = %if.then2
  %mul = fmul double %x, 0x4350000000000000
  %1 = bitcast double %mul to i64
  %gh_u.sroa.0.4.extract.shift = lshr i64 %1, 32
  %gh_u.sroa.0.4.extract.trunc = trunc i64 %gh_u.sroa.0.4.extract.shift to i32
  %and15 = and i32 %gh_u.sroa.0.4.extract.trunc, 2147483647
  br label %if.end31

if.end31:                                         ; preds = %if.end, %do.body6
  %xor.sink = phi i32 [ %and15, %do.body6 ], [ %xor, %if.end ]
  %.sink = phi i32 [ 696219795, %do.body6 ], [ 715094163, %if.end ]
  %div24 = udiv i32 %xor.sink, 3
  %add25 = add nuw nsw i32 %div24, %.sink
  %t.0.in.in.in = or disjoint i32 %add25, %and
  %t.0.in.in = zext i32 %t.0.in.in.in to i64
  %t.0.in = shl nuw i64 %t.0.in.in, 32
  %t.0 = bitcast i64 %t.0.in to double
  %mul32 = fmul double %t.0, %t.0
  %div33 = fdiv double %t.0, %x
  %mul34 = fmul double %mul32, %div33
  %2 = tail call double @llvm.fmuladd.f64(double %mul34, double 0x3FF9F1604A49D6C2, double 0xBFFE28E092F02420)
  %3 = tail call double @llvm.fmuladd.f64(double %mul34, double %2, double 0x3FFE03E60F61E692)
  %mul37 = fmul double %mul34, %mul34
  %mul38 = fmul double %mul34, %mul37
  %4 = tail call double @llvm.fmuladd.f64(double %mul34, double 0x3FC2B000D4E4EDD7, double 0xBFE844CBBEE751D9)
  %5 = tail call double @llvm.fmuladd.f64(double %mul38, double %4, double %3)
  %mul41 = fmul double %5, %t.0
  %6 = bitcast double %mul41 to i64
  %7 = and i64 %6, -1073741824
  %and43 = add i64 %7, 2147483648
  %8 = bitcast i64 %and43 to double
  %mul44 = fmul double %8, %8
  %div45 = fdiv double %x, %mul44
  %add46 = fadd double %8, %8
  %sub = fsub double %div45, %8
  %add47 = fadd double %add46, %div45
  %div48 = fdiv double %sub, %add47
  %9 = tail call double @llvm.fmuladd.f64(double %8, double %div48, double %8)
  br label %cleanup

cleanup:                                          ; preds = %if.then2, %if.end31, %if.then
  %retval.0 = phi double [ %add, %if.then ], [ %9, %if.end31 ], [ %x, %if.then2 ]
  ret double %retval.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fmuladd.f64(double, double, double) #1

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
