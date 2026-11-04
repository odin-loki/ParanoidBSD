; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/s_csqrtl_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/s_csqrtl_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local { x86_fp80, x86_fp80 } @ref_csqrtl(ptr nocapture noundef readonly byval({ x86_fp80, x86_fp80 }) align 16 %z) local_unnamed_addr #0 {
entry:
  %z.real = load x86_fp80, ptr %z, align 16
  %z.imagp = getelementptr inbounds { x86_fp80, x86_fp80 }, ptr %z, i64 0, i32 1
  %z.imag4 = load x86_fp80, ptr %z.imagp, align 16
  %cmp.r = fcmp oeq x86_fp80 %z.real, 0xK00000000000000000000
  %cmp.i = fcmp oeq x86_fp80 %z.imag4, 0xK00000000000000000000
  %and.ri = and i1 %cmp.r, %cmp.i
  br i1 %and.ri, label %cleanup, label %if.end

if.end:                                           ; preds = %entry
  %call = tail call i32 @__isinfl(x86_fp80 noundef %z.imag4) #5
  %tobool.not = icmp eq i32 %call, 0
  br i1 %tobool.not, label %if.end17, label %cleanup

if.end17:                                         ; preds = %if.end
  %cmp.i170 = fcmp ord x86_fp80 %z.real, 0xK00000000000000000000
  br i1 %cmp.i170, label %if.end30, label %if.then20

if.then20:                                        ; preds = %if.end17
  %sub = fsub x86_fp80 %z.imag4, %z.imag4
  %div = fdiv x86_fp80 %sub, %sub
  %add = fadd x86_fp80 %z.real, 0xK00000000000000000000
  %add23 = fadd x86_fp80 %add, %div
  br label %cleanup

if.end30:                                         ; preds = %if.end17
  %call31 = tail call i32 @__isinfl(x86_fp80 noundef %z.real) #5
  %tobool32.not = icmp eq i32 %call31, 0
  br i1 %tobool32.not, label %if.end49, label %if.then33

if.then33:                                        ; preds = %if.end30
  %call34 = tail call i32 @__signbitl(x86_fp80 noundef %z.real) #5
  %tobool35.not = icmp eq i32 %call34, 0
  %sub44 = fsub x86_fp80 %z.imag4, %z.imag4
  br i1 %tobool35.not, label %if.else, label %if.then36

if.then36:                                        ; preds = %if.then33
  %0 = tail call x86_fp80 @llvm.fabs.f80(x86_fp80 %sub44)
  %1 = tail call x86_fp80 @llvm.copysign.f80(x86_fp80 %z.real, x86_fp80 %z.imag4)
  br label %cleanup

if.else:                                          ; preds = %if.then33
  %2 = tail call x86_fp80 @llvm.copysign.f80(x86_fp80 %sub44, x86_fp80 %z.imag4)
  br label %cleanup

if.end49:                                         ; preds = %if.end30
  %cmp.i171 = fcmp ord x86_fp80 %z.imag4, 0xK00000000000000000000
  br i1 %cmp.i171, label %if.end65, label %if.then52

if.then52:                                        ; preds = %if.end49
  %sub53 = fsub x86_fp80 %z.real, %z.real
  %div55 = fdiv x86_fp80 %sub53, %sub53
  %add57 = fadd x86_fp80 %z.imag4, 0xK00000000000000000000
  %add58 = fadd x86_fp80 %add57, %div55
  br label %cleanup

if.end65:                                         ; preds = %if.end49
  %3 = tail call x86_fp80 @llvm.fabs.f80(x86_fp80 %z.real)
  %cmp = fcmp ult x86_fp80 %3, 0xK7FFD8000000000000000
  %4 = tail call x86_fp80 @llvm.fabs.f80(x86_fp80 %z.imag4)
  %cmp66 = fcmp ult x86_fp80 %4, 0xK7FFD8000000000000000
  %or.cond = and i1 %cmp, %cmp66
  br i1 %or.cond, label %if.end76, label %if.then67

if.then67:                                        ; preds = %if.end65
  %cmp68 = fcmp ult x86_fp80 %3, 0xK00038000000000000000
  %mul = fmul x86_fp80 %z.real, 0xK3FFD8000000000000000
  %a.0 = select i1 %cmp68, x86_fp80 %z.real, x86_fp80 %mul
  %cmp71 = fcmp ult x86_fp80 %4, 0xK00038000000000000000
  %mul73 = fmul x86_fp80 %z.imag4, 0xK3FFD8000000000000000
  %b.0 = select i1 %cmp71, x86_fp80 %z.imag4, x86_fp80 %mul73
  %.pre = tail call x86_fp80 @llvm.fabs.f80(x86_fp80 %a.0)
  %.pre173 = tail call x86_fp80 @llvm.fabs.f80(x86_fp80 %b.0)
  br label %if.end76

if.end76:                                         ; preds = %if.end65, %if.then67
  %.pre-phi174 = phi x86_fp80 [ %4, %if.end65 ], [ %.pre173, %if.then67 ]
  %.pre-phi = phi x86_fp80 [ %3, %if.end65 ], [ %.pre, %if.then67 ]
  %scale.0 = phi x86_fp80 [ 0xK3FFF8000000000000000, %if.end65 ], [ 0xK40008000000000000000, %if.then67 ]
  %b.1 = phi x86_fp80 [ %z.imag4, %if.end65 ], [ %b.0, %if.then67 ]
  %a.1 = phi x86_fp80 [ %z.real, %if.end65 ], [ %a.0, %if.then67 ]
  %cmp77 = fcmp olt x86_fp80 %.pre-phi, 0xK00018000000000000000
  %cmp78 = fcmp olt x86_fp80 %.pre-phi174, 0xK00018000000000000000
  %or.cond169 = and i1 %cmp78, %cmp77
  %mul80 = fmul x86_fp80 %a.1, 0xK403F8000000000000000
  %mul81 = fmul x86_fp80 %b.1, 0xK403F8000000000000000
  %scale.1 = select i1 %or.cond169, x86_fp80 0xK3FDF8000000000000000, x86_fp80 %scale.0
  %b.2 = select i1 %or.cond169, x86_fp80 %mul81, x86_fp80 %b.1
  %a.2 = select i1 %or.cond169, x86_fp80 %mul80, x86_fp80 %a.1
  %cmp83 = fcmp ult x86_fp80 %a.2, 0xK00000000000000000000
  %call94 = tail call x86_fp80 @hypotl(x86_fp80 noundef %a.2, x86_fp80 noundef %b.2) #6
  br i1 %cmp83, label %if.else93, label %if.then84

if.then84:                                        ; preds = %if.end76
  %add86 = fadd x86_fp80 %a.2, %call94
  %mul87 = fmul x86_fp80 %add86, 0xK3FFE8000000000000000
  %call88 = tail call x86_fp80 @sqrtl(x86_fp80 noundef %mul87) #6
  %mul89 = fmul x86_fp80 %scale.1, %call88
  %mul90 = fmul x86_fp80 %scale.1, %b.2
  %mul91 = fmul x86_fp80 %call88, 0xK40008000000000000000
  %div92 = fdiv x86_fp80 %mul90, %mul91
  br label %cleanup

if.else93:                                        ; preds = %if.end76
  %add95 = fsub x86_fp80 %call94, %a.2
  %mul96 = fmul x86_fp80 %add95, 0xK3FFE8000000000000000
  %call97 = tail call x86_fp80 @sqrtl(x86_fp80 noundef %mul96) #6
  %5 = tail call x86_fp80 @llvm.fabs.f80(x86_fp80 %b.2)
  %mul98 = fmul x86_fp80 %scale.1, %5
  %mul99 = fmul x86_fp80 %call97, 0xK40008000000000000000
  %div100 = fdiv x86_fp80 %mul98, %mul99
  %mul101 = fmul x86_fp80 %scale.1, %call97
  %6 = tail call x86_fp80 @llvm.copysign.f80(x86_fp80 %mul101, x86_fp80 %b.2)
  br label %cleanup

cleanup:                                          ; preds = %if.then84, %if.else93, %if.end, %entry, %if.then52, %if.else, %if.then36, %if.then20
  %retval.sroa.0.0 = phi x86_fp80 [ %add23, %if.then20 ], [ %0, %if.then36 ], [ %z.real, %if.else ], [ %add58, %if.then52 ], [ 0xK00000000000000000000, %entry ], [ 0xK7FFF8000000000000000, %if.end ], [ %mul89, %if.then84 ], [ %div100, %if.else93 ]
  %retval.sroa.8.0 = phi x86_fp80 [ %add23, %if.then20 ], [ %1, %if.then36 ], [ %2, %if.else ], [ %add58, %if.then52 ], [ %z.imag4, %entry ], [ %z.imag4, %if.end ], [ %div92, %if.then84 ], [ %6, %if.else93 ]
  %.fca.0.insert = insertvalue { x86_fp80, x86_fp80 } poison, x86_fp80 %retval.sroa.0.0, 0
  %.fca.1.insert = insertvalue { x86_fp80, x86_fp80 } %.fca.0.insert, x86_fp80 %retval.sroa.8.0, 1
  ret { x86_fp80, x86_fp80 } %.fca.1.insert
}

; Function Attrs: mustprogress nofree nosync nounwind willreturn memory(none)
declare i32 @__isinfl(x86_fp80 noundef) local_unnamed_addr #1

; Function Attrs: mustprogress nofree nosync nounwind willreturn memory(none)
declare i32 @__signbitl(x86_fp80 noundef) local_unnamed_addr #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare x86_fp80 @llvm.fabs.f80(x86_fp80) #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare x86_fp80 @llvm.copysign.f80(x86_fp80, x86_fp80) #2

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare x86_fp80 @sqrtl(x86_fp80 noundef) local_unnamed_addr #3

; Function Attrs: nounwind
declare x86_fp80 @hypotl(x86_fp80 noundef, x86_fp80 noundef) local_unnamed_addr #4

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nofree nosync nounwind willreturn memory(none) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #3 = { mustprogress nofree nounwind willreturn memory(write) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { nounwind willreturn memory(none) }
attributes #6 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
