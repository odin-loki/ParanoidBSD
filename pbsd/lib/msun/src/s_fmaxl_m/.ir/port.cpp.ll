; ModuleID = '/tmp/port-59bf2c.pcm'
source_filename = "/tmp/port-59bf2c.pcm"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@llvm.global_ctors = appending global [0 x { i32, ptr, ptr }] zeroinitializer

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local noundef x86_fp80 @_ZN4pbsd3lib4msun3src7s_fmaxlW4pbsdW3libW4msunW3srcW7s_fmaxl5fmaxlEee(x86_fp80 noundef %x, x86_fp80 noundef %y) local_unnamed_addr #0 {
entry:
  %0 = bitcast x86_fp80 %x to i80
  %1 = and i80 %0, -9223372036854775809
  %2 = bitcast x86_fp80 %y to i80
  %3 = and i80 %2, -9223372036854775809
  %u.sroa.0.sroa.0.0.insert.ext118 = zext i80 %1 to i128
  %4 = and i128 %u.sroa.0.sroa.0.0.insert.ext118, 604444463063240877801472
  %cmp = icmp eq i128 %4, 604444463063240877801472
  br i1 %cmp, label %land.lhs.true, label %if.end

land.lhs.true:                                    ; preds = %entry
  %bf.lshr24 = lshr i128 %u.sroa.0.sroa.0.0.insert.ext118, 32
  %bf.cast26 = trunc i128 %bf.lshr24 to i32
  %bf.cast30 = trunc i80 %0 to i32
  %or = or i32 %bf.cast26, %bf.cast30
  %cmp31.not = icmp eq i32 %or, 0
  br i1 %cmp31.not, label %if.end, label %cleanup

if.end:                                           ; preds = %land.lhs.true, %entry
  %u.sroa.11.sroa.0.0.insert.ext78 = zext i80 %3 to i128
  %5 = and i128 %u.sroa.11.sroa.0.0.insert.ext78, 604444463063240877801472
  %cmp37 = icmp eq i128 %5, 604444463063240877801472
  br i1 %cmp37, label %land.lhs.true38, label %if.end51

land.lhs.true38:                                  ; preds = %if.end
  %bf.lshr41 = lshr i128 %u.sroa.11.sroa.0.0.insert.ext78, 32
  %bf.cast43 = trunc i128 %bf.lshr41 to i32
  %bf.cast47 = trunc i80 %2 to i32
  %or48 = or i32 %bf.cast43, %bf.cast47
  %cmp49.not = icmp eq i32 %or48, 0
  br i1 %cmp49.not, label %if.end51, label %cleanup

if.end51:                                         ; preds = %land.lhs.true38, %if.end
  %bf.lshr54 = lshr i128 %u.sroa.0.sroa.0.0.insert.ext118, 79
  %bf.cast56 = trunc i128 %bf.lshr54 to i32
  %bf.lshr59 = lshr i128 %u.sroa.11.sroa.0.0.insert.ext78, 79
  %bf.cast61 = trunc i128 %bf.lshr59 to i32
  %cmp62 = icmp eq i32 %bf.cast56, %bf.cast61
  br i1 %cmp62, label %if.then63, label %if.end69

if.then63:                                        ; preds = %if.end51
  %tobool.not = icmp eq i32 %bf.cast56, 0
  %cond = select i1 %tobool.not, x86_fp80 %x, x86_fp80 %y
  br label %cleanup

if.end69:                                         ; preds = %if.end51
  %cmp70 = fcmp ogt x86_fp80 %x, %y
  %cond74 = select i1 %cmp70, x86_fp80 %x, x86_fp80 %y
  br label %cleanup

cleanup:                                          ; preds = %land.lhs.true38, %land.lhs.true, %if.end69, %if.then63
  %retval.0 = phi x86_fp80 [ %cond, %if.then63 ], [ %cond74, %if.end69 ], [ %y, %land.lhs.true ], [ %x, %land.lhs.true38 ]
  ret x86_fp80 %retval.0
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none)
define void @_ZGIW4pbsdW3libW4msunW3srcW7s_fmaxl() local_unnamed_addr #1 section ".text.startup" {
entry:
  ret void
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) "min-legal-vector-width"="0" }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
