; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/e_fmodf_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/e_fmodf_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@Zero = internal unnamed_addr constant [2 x float] [float 0.000000e+00, float -0.000000e+00], align 4

; Function Attrs: nofree norecurse nosync nounwind memory(none) uwtable
define dso_local float @ref_fmodf(float noundef %x, float noundef %y) local_unnamed_addr #0 {
entry:
  %0 = bitcast float %x to i32
  %and = and i32 %0, -2147483648
  %1 = tail call float @llvm.fabs.f32(float %x)
  %xor = bitcast float %1 to i32
  %2 = tail call float @llvm.fabs.f32(float %y)
  %and5 = bitcast float %2 to i32
  %cmp = icmp eq i32 %and5, 0
  %cmp6 = icmp sgt i32 %xor, 2139095039
  %or.cond = or i1 %cmp6, %cmp
  %cmp8 = icmp sgt i32 %and5, 2139095040
  %or.cond116 = or i1 %cmp8, %or.cond
  br i1 %or.cond116, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %conv = fpext float %x to x86_fp80
  %add = fadd x86_fp80 %conv, 0xK00000000000000000000
  %add9 = fadd float %y, 0.000000e+00
  %conv10 = fpext float %add9 to x86_fp80
  %mul = fmul x86_fp80 %add, %conv10
  %div = fdiv x86_fp80 %mul, %mul
  %conv16 = fptrunc x86_fp80 %div to float
  br label %cleanup

if.end:                                           ; preds = %entry
  %cmp17 = icmp slt i32 %xor, %and5
  br i1 %cmp17, label %cleanup, label %if.end20

if.end20:                                         ; preds = %if.end
  %cmp21 = icmp eq i32 %xor, %and5
  br i1 %cmp21, label %if.then23, label %if.end24

if.then23:                                        ; preds = %if.end20
  %shr = lshr i32 %0, 31
  %idxprom = zext nneg i32 %shr to i64
  %arrayidx = getelementptr inbounds [2 x float], ptr @Zero, i64 0, i64 %idxprom
  %3 = load float, ptr %arrayidx, align 4, !tbaa !5
  br label %cleanup

if.end24:                                         ; preds = %if.end20
  %cmp25 = icmp slt i32 %xor, 8388608
  br i1 %cmp25, label %if.then27, label %if.else

if.then27:                                        ; preds = %if.end24
  %shl.i = shl nuw nsw i32 %xor, 8
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %if.then27
  %i.06.i = phi i32 [ %shl1.i, %for.body.i ], [ %shl.i, %if.then27 ]
  %j.05.i = phi i32 [ %sub.i, %for.body.i ], [ -126, %if.then27 ]
  %sub.i = add nsw i32 %j.05.i, -1
  %shl1.i = shl nuw i32 %i.06.i, 1
  %cmp.i = icmp ult i32 %i.06.i, 1073741824
  br i1 %cmp.i, label %for.body.i, label %if.end29, !llvm.loop !9

if.else:                                          ; preds = %if.end24
  %shr28 = lshr i32 %xor, 23
  %sub = add nsw i32 %shr28, -127
  br label %if.end29

if.end29:                                         ; preds = %for.body.i, %if.else
  %ix.0 = phi i32 [ %sub, %if.else ], [ %sub.i, %for.body.i ]
  %cmp30 = icmp slt i32 %and5, 8388608
  br i1 %cmp30, label %if.then32, label %if.else34

if.then32:                                        ; preds = %if.end29
  %shl.i177 = shl nuw nsw i32 %and5, 8
  br label %for.body.i178

for.body.i178:                                    ; preds = %for.body.i178, %if.then32
  %i.06.i179 = phi i32 [ %shl1.i182, %for.body.i178 ], [ %shl.i177, %if.then32 ]
  %j.05.i180 = phi i32 [ %sub.i181, %for.body.i178 ], [ -126, %if.then32 ]
  %sub.i181 = add nsw i32 %j.05.i180, -1
  %shl1.i182 = shl nuw i32 %i.06.i179, 1
  %cmp.i183 = icmp ult i32 %i.06.i179, 1073741824
  br i1 %cmp.i183, label %for.body.i178, label %if.end37, !llvm.loop !9

if.else34:                                        ; preds = %if.end29
  %shr35 = lshr i32 %and5, 23
  %sub36 = add nsw i32 %shr35, -127
  br label %if.end37

if.end37:                                         ; preds = %for.body.i178, %if.else34
  %iy.0 = phi i32 [ %sub36, %if.else34 ], [ %sub.i181, %for.body.i178 ]
  %cmp38 = icmp sgt i32 %ix.0, -127
  %and41 = and i32 %xor, 8388607
  %or = or disjoint i32 %and41, 8388608
  %sub43 = sub nuw nsw i32 -126, %ix.0
  %shl = shl i32 %xor, %sub43
  %hx.0 = select i1 %cmp38, i32 %or, i32 %shl
  %cmp45 = icmp sgt i32 %iy.0, -127
  %and48 = and i32 %and5, 8388607
  %or49 = or disjoint i32 %and48, 8388608
  %sub51 = sub nuw nsw i32 -126, %iy.0
  %shl52 = shl i32 %and5, %sub51
  %hy.0 = select i1 %cmp45, i32 %or49, i32 %shl52
  %tobool.not188 = icmp eq i32 %ix.0, %iy.0
  br i1 %tobool.not188, label %while.end, label %while.body.preheader

while.body.preheader:                             ; preds = %if.end37
  %4 = xor i32 %iy.0, -1
  %dec187 = add i32 %ix.0, %4
  br label %while.body

while.body:                                       ; preds = %while.body.preheader, %if.end69
  %dec190 = phi i32 [ %dec, %if.end69 ], [ %dec187, %while.body.preheader ]
  %hx.1189 = phi i32 [ %hx.2, %if.end69 ], [ %hx.0, %while.body.preheader ]
  %sub55 = sub nsw i32 %hx.1189, %hy.0
  %cmp56 = icmp slt i32 %sub55, 0
  br i1 %cmp56, label %if.end69, label %if.else60

if.else60:                                        ; preds = %while.body
  %cmp61 = icmp eq i32 %hx.1189, %hy.0
  br i1 %cmp61, label %if.then63, label %if.end69

if.then63:                                        ; preds = %if.else60
  %shr64 = lshr i32 %0, 31
  %idxprom65 = zext nneg i32 %shr64 to i64
  %arrayidx66 = getelementptr inbounds [2 x float], ptr @Zero, i64 0, i64 %idxprom65
  %5 = load float, ptr %arrayidx66, align 4, !tbaa !5
  br label %cleanup

if.end69:                                         ; preds = %if.else60, %while.body
  %hx.2.in = phi i32 [ %hx.1189, %while.body ], [ %sub55, %if.else60 ]
  %hx.2 = shl nsw i32 %hx.2.in, 1
  %dec = add nsw i32 %dec190, -1
  %tobool.not = icmp eq i32 %dec190, 0
  br i1 %tobool.not, label %while.end, label %while.body, !llvm.loop !11

while.end:                                        ; preds = %if.end69, %if.end37
  %hx.1.lcssa = phi i32 [ %hx.0, %if.end37 ], [ %hx.2, %if.end69 ]
  %sub70 = sub nsw i32 %hx.1.lcssa, %hy.0
  %cmp71185 = icmp slt i32 %sub70, 0
  %spec.select = select i1 %cmp71185, i32 %hx.1.lcssa, i32 %sub70
  %cmp75 = icmp eq i32 %spec.select, 0
  br i1 %cmp75, label %if.then77, label %while.cond82.preheader

while.cond82.preheader:                           ; preds = %while.end
  %cmp83191 = icmp slt i32 %spec.select, 8388608
  br i1 %cmp83191, label %while.body85, label %while.end88

if.then77:                                        ; preds = %while.end
  %shr78 = lshr i32 %0, 31
  %idxprom79 = zext nneg i32 %shr78 to i64
  %arrayidx80 = getelementptr inbounds [2 x float], ptr @Zero, i64 0, i64 %idxprom79
  %6 = load float, ptr %arrayidx80, align 4, !tbaa !5
  br label %cleanup

while.body85:                                     ; preds = %while.cond82.preheader, %while.body85
  %hx.4193 = phi i32 [ %add86, %while.body85 ], [ %spec.select, %while.cond82.preheader ]
  %iy.1192 = phi i32 [ %sub87, %while.body85 ], [ %iy.0, %while.cond82.preheader ]
  %add86 = shl nsw i32 %hx.4193, 1
  %sub87 = add nsw i32 %iy.1192, -1
  %cmp83 = icmp slt i32 %hx.4193, 4194304
  br i1 %cmp83, label %while.body85, label %while.end88, !llvm.loop !12

while.end88:                                      ; preds = %while.body85, %while.cond82.preheader
  %iy.1.lcssa = phi i32 [ %iy.0, %while.cond82.preheader ], [ %sub87, %while.body85 ]
  %hx.4.lcssa = phi i32 [ %spec.select, %while.cond82.preheader ], [ %add86, %while.body85 ]
  %cmp89 = icmp sgt i32 %iy.1.lcssa, -127
  br i1 %cmp89, label %if.then91, label %if.else100

if.then91:                                        ; preds = %while.end88
  %sub92 = add nsw i32 %hx.4.lcssa, -8388608
  %add93 = shl i32 %iy.1.lcssa, 23
  %shl94 = add i32 %add93, 1065353216
  %or95 = or i32 %sub92, %shl94
  br label %if.end109

if.else100:                                       ; preds = %while.end88
  %sub101 = sub nuw nsw i32 -126, %iy.1.lcssa
  %shr102 = lshr i32 %hx.4.lcssa, %sub101
  br label %if.end109

if.end109:                                        ; preds = %if.else100, %if.then91
  %or95.pn = phi i32 [ %or95, %if.then91 ], [ %shr102, %if.else100 ]
  %x.addr.0.in = or i32 %or95.pn, %and
  %x.addr.0 = bitcast i32 %x.addr.0.in to float
  br label %cleanup

cleanup:                                          ; preds = %if.end, %if.end109, %if.then77, %if.then63, %if.then23, %if.then
  %retval.0 = phi float [ %conv16, %if.then ], [ %3, %if.then23 ], [ %5, %if.then63 ], [ %6, %if.then77 ], [ %x.addr.0, %if.end109 ], [ %x, %if.end ]
  ret float %retval.0
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fabs.f32(float) #1

attributes #0 = { nofree norecurse nosync nounwind memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

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
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.mustprogress"}
!11 = distinct !{!11, !10}
!12 = distinct !{!12, !10}
