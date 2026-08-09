; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/s_fminimum_numl_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/s_fminimum_numl_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: nofree norecurse nounwind memory(inaccessiblemem: readwrite) uwtable
define dso_local x86_fp80 @ref_fminimum_numl(x86_fp80 noundef %x, x86_fp80 noundef %y) local_unnamed_addr #0 {
entry:
  %force_except = alloca x86_fp80, align 16
  %0 = bitcast x86_fp80 %x to i80
  %1 = and i80 %0, -9223372036854775809
  %2 = bitcast x86_fp80 %y to i80
  %3 = and i80 %2, -9223372036854775809
  %u.sroa.0.sroa.0.0.insert.ext141 = zext i80 %1 to i128
  %4 = and i128 %u.sroa.0.sroa.0.0.insert.ext141, 604444463063240877801472
  %cmp = icmp eq i128 %4, 604444463063240877801472
  %bf.lshr24 = lshr i128 %u.sroa.0.sroa.0.0.insert.ext141, 32
  %bf.cast26 = trunc i128 %bf.lshr24 to i32
  %bf.cast30 = trunc i80 %0 to i32
  %or = or i32 %bf.cast26, %bf.cast30
  %cmp31 = icmp ne i32 %or, 0
  %5 = and i1 %cmp, %cmp31
  %u.sroa.10.sroa.0.0.insert.ext93 = zext i80 %3 to i128
  %6 = and i128 %u.sroa.10.sroa.0.0.insert.ext93, 604444463063240877801472
  %cmp37 = icmp eq i128 %6, 604444463063240877801472
  %bf.lshr41 = lshr i128 %u.sroa.10.sroa.0.0.insert.ext93, 32
  %bf.cast43 = trunc i128 %bf.lshr41 to i32
  %bf.cast47 = trunc i80 %2 to i32
  %or48 = or i32 %bf.cast43, %bf.cast47
  %cmp49 = icmp ne i32 %or48, 0
  %7 = and i1 %cmp37, %cmp49
  %brmerge = select i1 %5, i1 true, i1 %7
  br i1 %brmerge, label %if.then, label %if.end59

if.then:                                          ; preds = %entry
  %8 = select i1 %5, i1 %7, i1 false
  br i1 %8, label %if.then55, label %if.end

if.then55:                                        ; preds = %if.then
  %add = fadd x86_fp80 %x, %y
  br label %cleanup84

if.end:                                           ; preds = %if.then
  call void @llvm.lifetime.start.p0(i64 10, ptr nonnull %force_except)
  %add56 = fadd x86_fp80 %x, %y
  store volatile x86_fp80 %add56, ptr %force_except, align 16, !tbaa !5
  %force_except.0.force_except.0.force_except.0.force_except.0. = load volatile x86_fp80, ptr %force_except, align 16, !tbaa !5
  %y.x = select i1 %5, x86_fp80 %y, x86_fp80 %x
  call void @llvm.lifetime.end.p0(i64 10, ptr nonnull %force_except)
  br label %cleanup84

if.end59:                                         ; preds = %entry
  %bf.lshr62 = lshr i128 %u.sroa.0.sroa.0.0.insert.ext141, 79
  %bf.cast64 = trunc i128 %bf.lshr62 to i32
  %bf.lshr67 = lshr i128 %u.sroa.10.sroa.0.0.insert.ext93, 79
  %bf.cast69 = trunc i128 %bf.lshr67 to i32
  %cmp70.not = icmp eq i32 %bf.cast64, %bf.cast69
  br i1 %cmp70.not, label %if.end78, label %if.then71

if.then71:                                        ; preds = %if.end59
  %tobool77.not = icmp eq i32 %bf.cast69, 0
  %cond = select i1 %tobool77.not, x86_fp80 %x, x86_fp80 %y
  br label %cleanup84

if.end78:                                         ; preds = %if.end59
  %cmp79 = fcmp olt x86_fp80 %x, %y
  %cond83 = select i1 %cmp79, x86_fp80 %x, x86_fp80 %y
  br label %cleanup84

cleanup84:                                        ; preds = %if.end78, %if.then71, %if.end, %if.then55
  %retval.1 = phi x86_fp80 [ %add, %if.then55 ], [ %y.x, %if.end ], [ %cond, %if.then71 ], [ %cond83, %if.end78 ]
  ret x86_fp80 %retval.1
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { nofree norecurse nounwind memory(inaccessiblemem: readwrite) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }

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
