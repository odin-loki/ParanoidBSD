; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/e_rem_pio2_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/e_rem_pio2_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local i32 @ref___ieee754_rem_pio2(double noundef %x, ptr nocapture noundef writeonly %y) local_unnamed_addr #0 {
entry:
  %tx = alloca [3 x double], align 16
  %ty = alloca [2 x double], align 16
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %tx) #4
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %ty) #4
  %0 = bitcast double %x to i64
  %gh_u.sroa.0.4.extract.shift = lshr i64 %0, 32
  %gh_u.sroa.0.4.extract.trunc = trunc i64 %gh_u.sroa.0.4.extract.shift to i32
  %and = and i32 %gh_u.sroa.0.4.extract.trunc, 2147483647
  %cmp = icmp ult i32 %and, 1074752123
  br i1 %cmp, label %if.then, label %if.end37

if.then:                                          ; preds = %entry
  %and1 = and i32 %gh_u.sroa.0.4.extract.trunc, 1048575
  %cmp2 = icmp eq i32 %and1, 598523
  br i1 %cmp2, label %medium, label %if.end

if.end:                                           ; preds = %if.then
  %cmp4 = icmp ult i32 %and, 1073928573
  %cmp6 = icmp sgt i32 %gh_u.sroa.0.4.extract.trunc, 0
  %arrayidx12 = getelementptr inbounds double, ptr %y, i64 1
  br i1 %cmp4, label %if.then5, label %if.else19

if.then5:                                         ; preds = %if.end
  br i1 %cmp6, label %if.then7, label %if.else

if.then7:                                         ; preds = %if.then5
  %sub = fadd double %x, 0xBFF921FB54400000
  %sub8 = fadd double %sub, 0xBDD0B4611A626331
  store double %sub8, ptr %y, align 8, !tbaa !5
  %sub10 = fsub double %sub, %sub8
  %sub11 = fadd double %sub10, 0xBDD0B4611A626331
  store double %sub11, ptr %arrayidx12, align 8, !tbaa !5
  br label %cleanup

if.else:                                          ; preds = %if.then5
  %add = fadd double %x, 0x3FF921FB54400000
  %add13 = fadd double %add, 0x3DD0B4611A626331
  store double %add13, ptr %y, align 8, !tbaa !5
  %sub16 = fsub double %add, %add13
  %add17 = fadd double %sub16, 0x3DD0B4611A626331
  store double %add17, ptr %arrayidx12, align 8, !tbaa !5
  br label %cleanup

if.else19:                                        ; preds = %if.end
  br i1 %cmp6, label %if.then21, label %if.else29

if.then21:                                        ; preds = %if.else19
  %sub22 = fadd double %x, 0xC00921FB54400000
  %sub23 = fadd double %sub22, 0xBDE0B4611A626331
  store double %sub23, ptr %y, align 8, !tbaa !5
  %sub26 = fsub double %sub22, %sub23
  %sub27 = fadd double %sub26, 0xBDE0B4611A626331
  store double %sub27, ptr %arrayidx12, align 8, !tbaa !5
  br label %cleanup

if.else29:                                        ; preds = %if.else19
  %add30 = fadd double %x, 0x400921FB54400000
  %add31 = fadd double %add30, 0x3DE0B4611A626331
  store double %add31, ptr %y, align 8, !tbaa !5
  %sub34 = fsub double %add30, %add31
  %add35 = fadd double %sub34, 0x3DE0B4611A626331
  store double %add35, ptr %arrayidx12, align 8, !tbaa !5
  br label %cleanup

if.end37:                                         ; preds = %entry
  %cmp38 = icmp ult i32 %and, 1075594812
  br i1 %cmp38, label %if.then39, label %if.end83

if.then39:                                        ; preds = %if.end37
  %cmp40 = icmp ult i32 %and, 1075183037
  br i1 %cmp40, label %if.then41, label %if.else62

if.then41:                                        ; preds = %if.then39
  %cmp42 = icmp eq i32 %and, 1074977148
  br i1 %cmp42, label %medium, label %if.end44

if.end44:                                         ; preds = %if.then41
  %cmp45 = icmp sgt i32 %gh_u.sroa.0.4.extract.trunc, 0
  %arrayidx53 = getelementptr inbounds double, ptr %y, i64 1
  br i1 %cmp45, label %if.then46, label %if.else54

if.then46:                                        ; preds = %if.end44
  %sub47 = fadd double %x, 0xC012D97C7F300000
  %sub48 = fadd double %sub47, 0xBDE90E91A79394CA
  store double %sub48, ptr %y, align 8, !tbaa !5
  %sub51 = fsub double %sub47, %sub48
  %sub52 = fadd double %sub51, 0xBDE90E91A79394CA
  store double %sub52, ptr %arrayidx53, align 8, !tbaa !5
  br label %cleanup

if.else54:                                        ; preds = %if.end44
  %add55 = fadd double %x, 0x4012D97C7F300000
  %add56 = fadd double %add55, 0x3DE90E91A79394CA
  store double %add56, ptr %y, align 8, !tbaa !5
  %sub59 = fsub double %add55, %add56
  %add60 = fadd double %sub59, 0x3DE90E91A79394CA
  store double %add60, ptr %arrayidx53, align 8, !tbaa !5
  br label %cleanup

if.else62:                                        ; preds = %if.then39
  %cmp63 = icmp eq i32 %and, 1075388923
  br i1 %cmp63, label %medium, label %if.end65

if.end65:                                         ; preds = %if.else62
  %cmp66 = icmp sgt i32 %gh_u.sroa.0.4.extract.trunc, 0
  %arrayidx74 = getelementptr inbounds double, ptr %y, i64 1
  br i1 %cmp66, label %if.then67, label %if.else75

if.then67:                                        ; preds = %if.end65
  %sub68 = fadd double %x, 0xC01921FB54400000
  %sub69 = fadd double %sub68, 0xBDF0B4611A626331
  store double %sub69, ptr %y, align 8, !tbaa !5
  %sub72 = fsub double %sub68, %sub69
  %sub73 = fadd double %sub72, 0xBDF0B4611A626331
  store double %sub73, ptr %arrayidx74, align 8, !tbaa !5
  br label %cleanup

if.else75:                                        ; preds = %if.end65
  %add76 = fadd double %x, 0x401921FB54400000
  %add77 = fadd double %add76, 0x3DF0B4611A626331
  store double %add77, ptr %y, align 8, !tbaa !5
  %sub80 = fsub double %add76, %add77
  %add81 = fadd double %sub80, 0x3DF0B4611A626331
  store double %add81, ptr %arrayidx74, align 8, !tbaa !5
  br label %cleanup

if.end83:                                         ; preds = %if.end37
  %cmp84 = icmp ult i32 %and, 1094263291
  br i1 %cmp84, label %medium, label %if.end136

medium:                                           ; preds = %if.end83, %if.else62, %if.then41, %if.then
  %mul = fmul double %x, 0x3FE45F306DC9C883
  %add.i = fadd double %mul, 0x4338000000000000
  %sub.i = fadd double %add.i, 0xC338000000000000
  %conv = fptosi double %sub.i to i32
  %neg = fneg double %sub.i
  %1 = tail call double @llvm.fmuladd.f64(double %neg, double 0x3FF921FB54400000, double %x)
  %mul87 = fmul double %sub.i, 0x3DD0B4611A626331
  %shr = lshr i32 %and, 20
  %sub88 = fsub double %1, %mul87
  store double %sub88, ptr %y, align 8, !tbaa !5
  %2 = bitcast double %sub88 to i64
  %sum.shift = lshr i64 %2, 52
  %shr96318 = trunc i64 %sum.shift to i32
  %and97 = and i32 %shr96318, 2047
  %sub98 = sub nsw i32 %shr, %and97
  %cmp99 = icmp sgt i32 %sub98, 16
  br i1 %cmp99, label %if.then101, label %if.end131

if.then101:                                       ; preds = %medium
  %mul102 = fmul double %sub.i, 0x3DD0B4611A600000
  %sub103 = fsub double %1, %mul102
  %sub105 = fsub double %1, %sub103
  %sub106 = fsub double %sub105, %mul102
  %neg107 = fneg double %sub106
  %3 = tail call double @llvm.fmuladd.f64(double %sub.i, double 0x3BA3198A2E037073, double %neg107)
  %sub108 = fsub double %sub103, %3
  store double %sub108, ptr %y, align 8, !tbaa !5
  %4 = bitcast double %sub108 to i64
  %sum.shift319 = lshr i64 %4, 52
  %shr116320 = trunc i64 %sum.shift319 to i32
  %and117 = and i32 %shr116320, 2047
  %sub118 = sub nsw i32 %shr, %and117
  %cmp119 = icmp sgt i32 %sub118, 49
  br i1 %cmp119, label %if.then121, label %if.end131

if.then121:                                       ; preds = %if.then101
  %mul122 = fmul double %sub.i, 0x3BA3198A2E000000
  %sub123 = fsub double %sub103, %mul122
  %sub125 = fsub double %sub103, %sub123
  %sub126 = fsub double %sub125, %mul122
  %neg127 = fneg double %sub126
  %5 = tail call double @llvm.fmuladd.f64(double %sub.i, double 0x397B839A252049C1, double %neg127)
  %sub128 = fsub double %sub123, %5
  store double %sub128, ptr %y, align 8, !tbaa !5
  br label %if.end131

if.end131:                                        ; preds = %if.then101, %if.then121, %medium
  %6 = phi double [ %sub128, %if.then121 ], [ %sub108, %if.then101 ], [ %sub88, %medium ]
  %r.0 = phi double [ %sub123, %if.then121 ], [ %sub103, %if.then101 ], [ %1, %medium ]
  %w.0 = phi double [ %5, %if.then121 ], [ %3, %if.then101 ], [ %mul87, %medium ]
  %sub133 = fsub double %r.0, %6
  %sub134 = fsub double %sub133, %w.0
  %arrayidx135 = getelementptr inbounds double, ptr %y, i64 1
  store double %sub134, ptr %arrayidx135, align 8, !tbaa !5
  br label %cleanup

if.end136:                                        ; preds = %if.end83
  %cmp137 = icmp ugt i32 %and, 2146435071
  br i1 %cmp137, label %if.then139, label %do.body144

if.then139:                                       ; preds = %if.end136
  %sub140 = fsub double %x, %x
  %arrayidx141 = getelementptr inbounds double, ptr %y, i64 1
  store double %sub140, ptr %arrayidx141, align 8, !tbaa !5
  store double %sub140, ptr %y, align 8, !tbaa !5
  br label %cleanup

do.body144:                                       ; preds = %if.end136
  %shr147 = lshr i32 %and, 20
  %sub148 = add nsw i32 %shr147, -1046
  %shl = shl nsw i32 %sub148, 20
  %sub150 = sub nsw i32 %and, %shl
  %iw_u.sroa.0.4.insert.ext = zext i32 %sub150 to i64
  %iw_u.sroa.0.4.insert.shift = shl nuw i64 %iw_u.sroa.0.4.insert.ext, 32
  %iw_u.sroa.0.0.insert.ext = and i64 %0, 4294967295
  %iw_u.sroa.0.0.insert.insert = or disjoint i64 %iw_u.sroa.0.4.insert.shift, %iw_u.sroa.0.0.insert.ext
  %7 = bitcast i64 %iw_u.sroa.0.0.insert.insert to double
  %conv157 = fptosi double %7 to i32
  %conv158 = sitofp i32 %conv157 to double
  store double %conv158, ptr %tx, align 16, !tbaa !5
  %sub162 = fsub double %7, %conv158
  %mul163 = fmul double %sub162, 0x4170000000000000
  %conv157.1 = fptosi double %mul163 to i32
  %conv158.1 = sitofp i32 %conv157.1 to double
  %arrayidx159.1 = getelementptr inbounds [3 x double], ptr %tx, i64 0, i64 1
  store double %conv158.1, ptr %arrayidx159.1, align 8, !tbaa !5
  %sub162.1 = fsub double %mul163, %conv158.1
  %mul163.1 = fmul double %sub162.1, 0x4170000000000000
  %arrayidx164 = getelementptr inbounds [3 x double], ptr %tx, i64 0, i64 2
  store double %mul163.1, ptr %arrayidx164, align 16, !tbaa !5
  br label %while.cond

while.cond:                                       ; preds = %while.cond, %do.body144
  %indvars.iv = phi i64 [ %indvars.iv.next, %while.cond ], [ 3, %do.body144 ]
  %indvars.iv.next = add nsw i64 %indvars.iv, -1
  %arrayidx167 = getelementptr inbounds [3 x double], ptr %tx, i64 0, i64 %indvars.iv.next
  %8 = load double, ptr %arrayidx167, align 8, !tbaa !5
  %cmp168 = fcmp oeq double %8, 0.000000e+00
  br i1 %cmp168, label %while.cond, label %while.end, !llvm.loop !9

while.end:                                        ; preds = %while.cond
  %9 = trunc i64 %indvars.iv to i32
  %call171 = call i32 @__kernel_rem_pio2(ptr noundef nonnull %tx, ptr noundef nonnull %ty, i32 noundef %sub148, i32 noundef %9, i32 noundef 1) #4
  %cmp172 = icmp slt i64 %0, 0
  %10 = load double, ptr %ty, align 16, !tbaa !5
  br i1 %cmp172, label %if.then174, label %if.end181

if.then174:                                       ; preds = %while.end
  %arrayidx177 = getelementptr inbounds [2 x double], ptr %ty, i64 0, i64 1
  %11 = load double, ptr %arrayidx177, align 8, !tbaa !5
  %12 = insertelement <2 x double> poison, double %10, i64 0
  %13 = insertelement <2 x double> %12, double %11, i64 1
  %14 = fneg <2 x double> %13
  store <2 x double> %14, ptr %y, align 8, !tbaa !5
  %sub180 = sub nsw i32 0, %call171
  br label %cleanup

if.end181:                                        ; preds = %while.end
  store double %10, ptr %y, align 8, !tbaa !5
  %arrayidx184 = getelementptr inbounds [2 x double], ptr %ty, i64 0, i64 1
  %15 = load double, ptr %arrayidx184, align 8, !tbaa !5
  %arrayidx185 = getelementptr inbounds double, ptr %y, i64 1
  store double %15, ptr %arrayidx185, align 8, !tbaa !5
  br label %cleanup

cleanup:                                          ; preds = %if.end181, %if.then174, %if.then139, %if.end131, %if.else75, %if.then67, %if.else54, %if.then46, %if.else29, %if.then21, %if.else, %if.then7
  %retval.0 = phi i32 [ %conv, %if.end131 ], [ 1, %if.then7 ], [ -1, %if.else ], [ 2, %if.then21 ], [ -2, %if.else29 ], [ 3, %if.then46 ], [ -3, %if.else54 ], [ 4, %if.then67 ], [ -4, %if.else75 ], [ 0, %if.then139 ], [ %sub180, %if.then174 ], [ %call171, %if.end181 ]
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %ty) #4
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %tx) #4
  ret i32 %retval.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fmuladd.f64(double, double, double) #2

declare i32 @__kernel_rem_pio2(ptr noundef, ptr noundef, i32 noundef, i32 noundef, i32 noundef) local_unnamed_addr #3

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #3 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
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
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.mustprogress"}
