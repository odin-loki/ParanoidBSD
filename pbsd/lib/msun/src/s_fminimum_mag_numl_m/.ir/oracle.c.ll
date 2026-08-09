; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/s_fminimum_mag_numl_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/s_fminimum_mag_numl_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: nofree norecurse nounwind memory(inaccessiblemem: readwrite) uwtable
define dso_local noundef x86_fp80 @ref_fminimum_mag_numl(x86_fp80 noundef %x, x86_fp80 noundef %y) local_unnamed_addr #0 {
entry:
  %force_except = alloca x86_fp80, align 16
  %0 = bitcast x86_fp80 %x to i80
  %1 = bitcast x86_fp80 %y to i80
  %cmp.i = fcmp ord x86_fp80 %x, 0xK00000000000000000000
  %brmerge.demorgan = fcmp uno x86_fp80 %x, %y
  br i1 %brmerge.demorgan, label %if.then, label %if.end28

if.then:                                          ; preds = %entry
  %cmp.i121 = fcmp ord x86_fp80 %y, 0xK00000000000000000000
  %brmerge120 = or i1 %cmp.i, %cmp.i121
  br i1 %brmerge120, label %if.end, label %if.then24

if.then24:                                        ; preds = %if.then
  %add = fadd x86_fp80 %x, %y
  br label %cleanup55

if.end:                                           ; preds = %if.then
  call void @llvm.lifetime.start.p0(i64 10, ptr nonnull %force_except)
  %add25 = fadd x86_fp80 %x, %y
  store volatile x86_fp80 %add25, ptr %force_except, align 16, !tbaa !5
  %force_except.0.force_except.0.force_except.0.force_except.0. = load volatile x86_fp80, ptr %force_except, align 16, !tbaa !5
  %x.y = select i1 %cmp.i, x86_fp80 %x, x86_fp80 %y
  call void @llvm.lifetime.end.p0(i64 10, ptr nonnull %force_except)
  br label %cleanup55

if.end28:                                         ; preds = %entry
  %2 = tail call x86_fp80 @llvm.fabs.f80(x86_fp80 %x)
  %3 = tail call x86_fp80 @llvm.fabs.f80(x86_fp80 %y)
  %cmp = fcmp olt x86_fp80 %3, %2
  br i1 %cmp, label %cleanup55, label %if.end30

if.end30:                                         ; preds = %if.end28
  %cmp31 = fcmp olt x86_fp80 %2, %3
  br i1 %cmp31, label %cleanup55, label %if.end33

if.end33:                                         ; preds = %if.end30
  %4 = lshr i80 %0, 79
  %bf.cast38 = trunc i80 %4 to i32
  %5 = lshr i80 %1, 79
  %bf.cast43 = trunc i80 %5 to i32
  %cmp44.not = icmp eq i32 %bf.cast38, %bf.cast43
  br i1 %cmp44.not, label %cleanup55, label %if.then45

if.then45:                                        ; preds = %if.end33
  %tobool51.not119 = icmp slt i80 %1, 0
  %cond = select i1 %tobool51.not119, x86_fp80 %y, x86_fp80 %x
  br label %cleanup55

cleanup55:                                        ; preds = %if.then45, %if.end28, %if.end30, %if.end33, %if.end, %if.then24
  %retval.2 = phi x86_fp80 [ %add, %if.then24 ], [ %x.y, %if.end ], [ %cond, %if.then45 ], [ %y, %if.end28 ], [ %x, %if.end30 ], [ %x, %if.end33 ]
  ret x86_fp80 %retval.2
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare x86_fp80 @llvm.fabs.f80(x86_fp80) #2

attributes #0 = { nofree norecurse nounwind memory(inaccessiblemem: readwrite) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!5 = !{!6, !6, i64 0}
!6 = !{!"long double", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
