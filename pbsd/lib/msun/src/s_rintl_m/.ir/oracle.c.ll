; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/s_rintl_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/s_rintl_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@shift = internal unnamed_addr constant [2 x float] [float 0x43E0000000000000, float 0xC3E0000000000000], align 4
@zero = internal unnamed_addr constant [2 x float] [float 0.000000e+00, float -0.000000e+00], align 4

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local x86_fp80 @ref_rintl(x86_fp80 noundef %x) local_unnamed_addr #0 {
entry:
  %0 = bitcast x86_fp80 %x to i80
  %1 = lshr i80 %0, 64
  %bf.cast = trunc i80 %1 to i32
  %and = and i32 %bf.cast, 32767
  %cmp = icmp ugt i32 %and, 16445
  br i1 %cmp, label %if.then, label %if.end3

if.then:                                          ; preds = %entry
  %cmp1 = icmp eq i32 %and, 32767
  %add = select i1 %cmp1, x86_fp80 %x, x86_fp80 0xK80000000000000000000
  %spec.select = fadd x86_fp80 %add, %x
  br label %cleanup

if.end3:                                          ; preds = %entry
  %shr = lshr i32 %bf.cast, 15
  %idxprom = zext nneg i32 %shr to i64
  %arrayidx = getelementptr inbounds [2 x float], ptr @shift, i64 0, i64 %idxprom
  %2 = load float, ptr %arrayidx, align 4, !tbaa !5
  %conv = fpext float %2 to x86_fp80
  %add4 = fadd x86_fp80 %conv, %x
  %sub = fsub x86_fp80 %add4, %conv
  %cmp8 = icmp ult i32 %and, 16383
  %cmp10 = fcmp oeq x86_fp80 %sub, 0xK00000000000000000000
  %or.cond = select i1 %cmp8, i1 %cmp10, i1 false
  br i1 %or.cond, label %if.then12, label %cleanup

if.then12:                                        ; preds = %if.end3
  %arrayidx14 = getelementptr inbounds [2 x float], ptr @zero, i64 0, i64 %idxprom
  %3 = load float, ptr %arrayidx14, align 4, !tbaa !5
  %conv15 = fpext float %3 to x86_fp80
  br label %cleanup

cleanup:                                          ; preds = %if.then, %if.end3, %if.then12
  %retval.0 = phi x86_fp80 [ %conv15, %if.then12 ], [ %sub, %if.end3 ], [ %spec.select, %if.then ]
  ret x86_fp80 %retval.0
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

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
