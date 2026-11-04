; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/e_fmod_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/e_fmod_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@Zero = internal unnamed_addr constant [2 x double] [double 0.000000e+00, double -0.000000e+00], align 16

; Function Attrs: nofree norecurse nosync nounwind memory(none) uwtable
define dso_local double @ref_fmod(double noundef %x, double noundef %y) local_unnamed_addr #0 {
entry:
  %0 = bitcast double %x to i64
  %ew_u.sroa.0.4.extract.shift = lshr i64 %0, 32
  %ew_u.sroa.0.4.extract.trunc = trunc i64 %ew_u.sroa.0.4.extract.shift to i32
  %ew_u.sroa.0.0.extract.trunc = trunc i64 %0 to i32
  %1 = bitcast double %y to i64
  %ew_u2.sroa.0.4.extract.shift = lshr i64 %1, 32
  %ew_u2.sroa.0.4.extract.trunc = trunc i64 %ew_u2.sroa.0.4.extract.shift to i32
  %ew_u2.sroa.0.0.extract.trunc = trunc i64 %1 to i32
  %and = and i32 %ew_u.sroa.0.4.extract.trunc, -2147483648
  %xor = and i32 %ew_u.sroa.0.4.extract.trunc, 2147483647
  %and7 = and i32 %ew_u2.sroa.0.4.extract.trunc, 2147483647
  %or = or i32 %and7, %ew_u2.sroa.0.0.extract.trunc
  %cmp = icmp eq i32 %or, 0
  %cmp8 = icmp ugt i32 %xor, 2146435071
  %or.cond = or i1 %cmp8, %cmp
  br i1 %or.cond, label %if.then, label %lor.lhs.false9

lor.lhs.false9:                                   ; preds = %entry
  %2 = icmp ne i32 %ew_u2.sroa.0.0.extract.trunc, 0
  %shr = zext i1 %2 to i32
  %or11 = or i32 %and7, %shr
  %cmp12 = icmp ugt i32 %or11, 2146435072
  br i1 %cmp12, label %if.then, label %if.end

if.then:                                          ; preds = %lor.lhs.false9, %entry
  %conv = fpext double %x to x86_fp80
  %add = fadd x86_fp80 %conv, 0xK00000000000000000000
  %add13 = fadd double %y, 0.000000e+00
  %conv14 = fpext double %add13 to x86_fp80
  %mul = fmul x86_fp80 %add, %conv14
  %div = fdiv x86_fp80 %mul, %mul
  %conv20 = fptrunc x86_fp80 %div to double
  br label %cleanup

if.end:                                           ; preds = %lor.lhs.false9
  %cmp21.not = icmp ugt i32 %xor, %and7
  br i1 %cmp21.not, label %if.end36, label %if.then23

if.then23:                                        ; preds = %if.end
  %cmp24 = icmp ult i32 %xor, %and7
  %cmp27 = icmp ult i32 %ew_u.sroa.0.0.extract.trunc, %ew_u2.sroa.0.0.extract.trunc
  %or.cond320 = or i1 %cmp27, %cmp24
  br i1 %or.cond320, label %cleanup, label %if.end30

if.end30:                                         ; preds = %if.then23
  %cmp31 = icmp eq i32 %ew_u.sroa.0.0.extract.trunc, %ew_u2.sroa.0.0.extract.trunc
  br i1 %cmp31, label %if.then33, label %if.end36

if.then33:                                        ; preds = %if.end30
  %shr34 = lshr i64 %0, 63
  %arrayidx = getelementptr inbounds [2 x double], ptr @Zero, i64 0, i64 %shr34
  %3 = load double, ptr %arrayidx, align 8, !tbaa !5
  br label %cleanup

if.end36:                                         ; preds = %if.end30, %if.end
  %cmp37 = icmp ult i32 %xor, 1048576
  br i1 %cmp37, label %if.then39, label %if.else

if.then39:                                        ; preds = %if.end36
  %cmp.i = icmp eq i32 %xor, 0
  %shl.i = shl nuw nsw i32 %xor, 11
  %j.0.i = select i1 %cmp.i, i32 -1043, i32 -1022
  %i.0.i = select i1 %cmp.i, i32 %ew_u.sroa.0.0.extract.trunc, i32 %shl.i
  %cmp18.i = icmp ult i32 %i.0.i, 2147483647
  br i1 %cmp18.i, label %for.body.i, label %if.end42

for.body.i:                                       ; preds = %if.then39, %for.body.i
  %i.110.i = phi i32 [ %shl3.i, %for.body.i ], [ %i.0.i, %if.then39 ]
  %j.19.i = phi i32 [ %sub2.i, %for.body.i ], [ %j.0.i, %if.then39 ]
  %sub2.i = add nsw i32 %j.19.i, -1
  %shl3.i = shl nuw i32 %i.110.i, 1
  %cmp1.i = icmp ult i32 %i.110.i, 1073741824
  br i1 %cmp1.i, label %for.body.i, label %if.end42, !llvm.loop !9

if.else:                                          ; preds = %if.end36
  %shr40 = lshr i32 %xor, 20
  %sub41 = add nsw i32 %shr40, -1023
  br label %if.end42

if.end42:                                         ; preds = %for.body.i, %if.then39, %if.else
  %ix.0 = phi i32 [ %sub41, %if.else ], [ %j.0.i, %if.then39 ], [ %sub2.i, %for.body.i ]
  %cmp43 = icmp ult i32 %and7, 1048576
  br i1 %cmp43, label %if.then45, label %if.else47

if.then45:                                        ; preds = %if.end42
  %cmp.i322 = icmp eq i32 %and7, 0
  %shl.i323 = shl nuw nsw i32 %and7, 11
  %j.0.i324 = select i1 %cmp.i322, i32 -1043, i32 -1022
  %i.0.i325 = select i1 %cmp.i322, i32 %ew_u2.sroa.0.0.extract.trunc, i32 %shl.i323
  %cmp18.i326 = icmp ult i32 %i.0.i325, 2147483647
  br i1 %cmp18.i326, label %for.body.i328, label %if.end50

for.body.i328:                                    ; preds = %if.then45, %for.body.i328
  %i.110.i329 = phi i32 [ %shl3.i332, %for.body.i328 ], [ %i.0.i325, %if.then45 ]
  %j.19.i330 = phi i32 [ %sub2.i331, %for.body.i328 ], [ %j.0.i324, %if.then45 ]
  %sub2.i331 = add nsw i32 %j.19.i330, -1
  %shl3.i332 = shl nuw i32 %i.110.i329, 1
  %cmp1.i333 = icmp ult i32 %i.110.i329, 1073741824
  br i1 %cmp1.i333, label %for.body.i328, label %if.end50, !llvm.loop !9

if.else47:                                        ; preds = %if.end42
  %shr48 = lshr i32 %and7, 20
  %sub49 = add nsw i32 %shr48, -1023
  br label %if.end50

if.end50:                                         ; preds = %for.body.i328, %if.then45, %if.else47
  %iy.0 = phi i32 [ %sub49, %if.else47 ], [ %j.0.i324, %if.then45 ], [ %sub2.i331, %for.body.i328 ]
  %cmp51 = icmp sgt i32 %ix.0, -1023
  br i1 %cmp51, label %if.then53, label %if.else56

if.then53:                                        ; preds = %if.end50
  %and54 = and i32 %ew_u.sroa.0.4.extract.trunc, 1048575
  %or55 = or disjoint i32 %and54, 1048576
  br label %if.end69

if.else56:                                        ; preds = %if.end50
  %cmp58 = icmp ugt i32 %ix.0, -1054
  br i1 %cmp58, label %if.then60, label %if.else65

if.then60:                                        ; preds = %if.else56
  %sub57 = sub nuw nsw i32 -1022, %ix.0
  %shl = shl i32 %xor, %sub57
  %sub61 = add nsw i32 %ix.0, 1054
  %shr62 = lshr i32 %ew_u.sroa.0.0.extract.trunc, %sub61
  %or63 = or i32 %shl, %shr62
  %shl64 = shl i32 %ew_u.sroa.0.0.extract.trunc, %sub57
  br label %if.end69

if.else65:                                        ; preds = %if.else56
  %sub66 = sub nuw nsw i32 -1054, %ix.0
  %shl67 = shl i32 %ew_u.sroa.0.0.extract.trunc, %sub66
  br label %if.end69

if.end69:                                         ; preds = %if.then60, %if.else65, %if.then53
  %hx.0 = phi i32 [ %or55, %if.then53 ], [ %or63, %if.then60 ], [ %shl67, %if.else65 ]
  %lx.0 = phi i32 [ %ew_u.sroa.0.0.extract.trunc, %if.then53 ], [ %shl64, %if.then60 ], [ 0, %if.else65 ]
  %cmp70 = icmp sgt i32 %iy.0, -1023
  br i1 %cmp70, label %if.then72, label %if.else75

if.then72:                                        ; preds = %if.end69
  %and73 = and i32 %ew_u2.sroa.0.4.extract.trunc, 1048575
  %or74 = or disjoint i32 %and73, 1048576
  br label %if.end89

if.else75:                                        ; preds = %if.end69
  %cmp77 = icmp ugt i32 %iy.0, -1054
  br i1 %cmp77, label %if.then79, label %if.else85

if.then79:                                        ; preds = %if.else75
  %sub76 = sub nuw nsw i32 -1022, %iy.0
  %shl80 = shl i32 %and7, %sub76
  %sub81 = add nsw i32 %iy.0, 1054
  %shr82 = lshr i32 %ew_u2.sroa.0.0.extract.trunc, %sub81
  %or83 = or i32 %shl80, %shr82
  %shl84 = shl i32 %ew_u2.sroa.0.0.extract.trunc, %sub76
  br label %if.end89

if.else85:                                        ; preds = %if.else75
  %sub86 = sub nuw nsw i32 -1054, %iy.0
  %shl87 = shl i32 %ew_u2.sroa.0.0.extract.trunc, %sub86
  br label %if.end89

if.end89:                                         ; preds = %if.then79, %if.else85, %if.then72
  %hy.0 = phi i32 [ %or74, %if.then72 ], [ %or83, %if.then79 ], [ %shl87, %if.else85 ]
  %ly.0 = phi i32 [ %ew_u2.sroa.0.0.extract.trunc, %if.then72 ], [ %shl84, %if.then79 ], [ 0, %if.else85 ]
  %tobool.not338 = icmp eq i32 %ix.0, %iy.0
  br i1 %tobool.not338, label %while.end, label %while.body.preheader

while.body.preheader:                             ; preds = %if.end89
  %4 = xor i32 %iy.0, -1
  %dec337 = add i32 %ix.0, %4
  br label %while.body

while.body:                                       ; preds = %while.body.preheader, %if.end118
  %dec341 = phi i32 [ %dec, %if.end118 ], [ %dec337, %while.body.preheader ]
  %lx.1340 = phi i32 [ %lx.2, %if.end118 ], [ %lx.0, %while.body.preheader ]
  %hx.1339 = phi i32 [ %hx.2, %if.end118 ], [ %hx.0, %while.body.preheader ]
  %sub91 = sub nsw i32 %hx.1339, %hy.0
  %sub92 = sub i32 %lx.1340, %ly.0
  %cmp93 = icmp ult i32 %lx.1340, %ly.0
  %sub96 = sext i1 %cmp93 to i32
  %spec.select = add nsw i32 %sub91, %sub96
  %cmp98 = icmp slt i32 %spec.select, 0
  br i1 %cmp98, label %if.then100, label %if.else105

if.then100:                                       ; preds = %while.body
  %add103 = tail call i32 @llvm.fshl.i32(i32 %hx.1339, i32 %lx.1340, i32 1)
  br label %if.end118

if.else105:                                       ; preds = %while.body
  %or106 = or i32 %spec.select, %sub92
  %cmp107 = icmp eq i32 %or106, 0
  br i1 %cmp107, label %if.then109, label %if.end113

if.then109:                                       ; preds = %if.else105
  %shr110 = lshr i64 %0, 63
  %arrayidx112 = getelementptr inbounds [2 x double], ptr @Zero, i64 0, i64 %shr110
  %5 = load double, ptr %arrayidx112, align 8, !tbaa !5
  br label %cleanup

if.end113:                                        ; preds = %if.else105
  %add116 = tail call i32 @llvm.fshl.i32(i32 %spec.select, i32 %sub92, i32 1)
  br label %if.end118

if.end118:                                        ; preds = %if.end113, %if.then100
  %hx.2 = phi i32 [ %add103, %if.then100 ], [ %add116, %if.end113 ]
  %lx.2.in = phi i32 [ %lx.1340, %if.then100 ], [ %sub92, %if.end113 ]
  %lx.2 = shl i32 %lx.2.in, 1
  %dec = add nsw i32 %dec341, -1
  %tobool.not = icmp eq i32 %dec341, 0
  br i1 %tobool.not, label %while.end, label %while.body, !llvm.loop !11

while.end:                                        ; preds = %if.end118, %if.end89
  %hx.1.lcssa = phi i32 [ %hx.0, %if.end89 ], [ %hx.2, %if.end118 ]
  %lx.1.lcssa = phi i32 [ %lx.0, %if.end89 ], [ %lx.2, %if.end118 ]
  %sub119 = sub nsw i32 %hx.1.lcssa, %hy.0
  %cmp121 = icmp ult i32 %lx.1.lcssa, %ly.0
  %sub124 = sext i1 %cmp121 to i32
  %spec.select321 = add nsw i32 %sub119, %sub124
  %cmp126 = icmp sgt i32 %spec.select321, -1
  %hx.3 = select i1 %cmp126, i32 %spec.select321, i32 %hx.1.lcssa
  %sub120 = select i1 %cmp126, i32 %ly.0, i32 0
  %lx.3 = sub i32 %lx.1.lcssa, %sub120
  %or130 = or i32 %lx.3, %hx.3
  %cmp131 = icmp eq i32 %or130, 0
  br i1 %cmp131, label %if.then133, label %while.cond138.preheader

while.cond138.preheader:                          ; preds = %while.end
  %cmp139343 = icmp slt i32 %hx.3, 1048576
  br i1 %cmp139343, label %while.body141, label %while.end147

if.then133:                                       ; preds = %while.end
  %shr134 = lshr i64 %0, 63
  %arrayidx136 = getelementptr inbounds [2 x double], ptr @Zero, i64 0, i64 %shr134
  %6 = load double, ptr %arrayidx136, align 8, !tbaa !5
  br label %cleanup

while.body141:                                    ; preds = %while.cond138.preheader, %while.body141
  %lx.4346 = phi i32 [ %add145, %while.body141 ], [ %lx.3, %while.cond138.preheader ]
  %hx.4345 = phi i32 [ %add144, %while.body141 ], [ %hx.3, %while.cond138.preheader ]
  %iy.1344 = phi i32 [ %sub146, %while.body141 ], [ %iy.0, %while.cond138.preheader ]
  %add144 = tail call i32 @llvm.fshl.i32(i32 %hx.4345, i32 %lx.4346, i32 1)
  %add145 = shl i32 %lx.4346, 1
  %sub146 = add nsw i32 %iy.1344, -1
  %cmp139 = icmp slt i32 %add144, 1048576
  br i1 %cmp139, label %while.body141, label %while.end147, !llvm.loop !12

while.end147:                                     ; preds = %while.body141, %while.cond138.preheader
  %iy.1.lcssa = phi i32 [ %iy.0, %while.cond138.preheader ], [ %sub146, %while.body141 ]
  %hx.4.lcssa = phi i32 [ %hx.3, %while.cond138.preheader ], [ %add144, %while.body141 ]
  %lx.4.lcssa = phi i32 [ %lx.3, %while.cond138.preheader ], [ %add145, %while.body141 ]
  %cmp148 = icmp sgt i32 %iy.1.lcssa, -1023
  br i1 %cmp148, label %if.then150, label %if.else161

if.then150:                                       ; preds = %while.end147
  %sub151 = add nsw i32 %hx.4.lcssa, -1048576
  %add152 = shl i32 %iy.1.lcssa, 20
  %shl153 = add i32 %add152, 1072693248
  %7 = or i32 %shl153, %sub151
  br label %if.end192

if.else161:                                       ; preds = %while.end147
  %sub162 = sub nuw nsw i32 -1022, %iy.1.lcssa
  %cmp163 = icmp ugt i32 %iy.1.lcssa, -1043
  br i1 %cmp163, label %if.then165, label %if.else171

if.then165:                                       ; preds = %if.else161
  %shr166 = lshr i32 %lx.4.lcssa, %sub162
  %sub167 = add nsw i32 %iy.1.lcssa, 1054
  %shl168 = shl i32 %hx.4.lcssa, %sub167
  %or169 = or i32 %shr166, %shl168
  %shr170 = lshr i32 %hx.4.lcssa, %sub162
  br label %if.end192

if.else171:                                       ; preds = %if.else161
  %cmp172 = icmp ugt i32 %iy.1.lcssa, -1054
  br i1 %cmp172, label %if.then174, label %if.else179

if.then174:                                       ; preds = %if.else171
  %sub175 = add nsw i32 %iy.1.lcssa, 1054
  %shl176 = shl i32 %hx.4.lcssa, %sub175
  %shr177 = lshr i32 %lx.4.lcssa, %sub162
  %or178 = or i32 %shr177, %shl176
  br label %if.end192

if.else179:                                       ; preds = %if.else171
  %sub180 = sub nuw nsw i32 -1054, %iy.1.lcssa
  %shr181 = lshr i32 %hx.4.lcssa, %sub180
  br label %if.end192

if.end192:                                        ; preds = %if.then174, %if.else179, %if.then165, %if.then150
  %hx.5.sink = phi i32 [ %7, %if.then150 ], [ %shr170, %if.then165 ], [ %and, %if.then174 ], [ %and, %if.else179 ]
  %lx.5.sink = phi i32 [ %lx.4.lcssa, %if.then150 ], [ %or169, %if.then165 ], [ %or178, %if.then174 ], [ %shr181, %if.else179 ]
  %or186 = or i32 %hx.5.sink, %and
  %iw_u185.sroa.0.4.insert.ext = zext i32 %or186 to i64
  %iw_u185.sroa.0.4.insert.shift = shl nuw i64 %iw_u185.sroa.0.4.insert.ext, 32
  %iw_u185.sroa.0.0.insert.ext = zext i32 %lx.5.sink to i64
  %iw_u185.sroa.0.0.insert.insert = or disjoint i64 %iw_u185.sroa.0.4.insert.shift, %iw_u185.sroa.0.0.insert.ext
  %x.addr.0 = bitcast i64 %iw_u185.sroa.0.0.insert.insert to double
  br label %cleanup

cleanup:                                          ; preds = %if.then23, %if.end192, %if.then133, %if.then109, %if.then33, %if.then
  %retval.0 = phi double [ %conv20, %if.then ], [ %3, %if.then33 ], [ %5, %if.then109 ], [ %6, %if.then133 ], [ %x.addr.0, %if.end192 ], [ %x, %if.then23 ]
  ret double %retval.0
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.fshl.i32(i32, i32, i32) #1

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
!6 = !{!"double", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.mustprogress"}
!11 = distinct !{!11, !10}
!12 = distinct !{!12, !10}
