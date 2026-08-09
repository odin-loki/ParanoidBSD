; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/s_clogl_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/s_clogl_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local { x86_fp80, x86_fp80 } @ref_clogl(ptr nocapture noundef readonly byval({ x86_fp80, x86_fp80 }) align 16 %z) local_unnamed_addr #0 {
entry:
  %z.real = load x86_fp80, ptr %z, align 16
  %z.imagp = getelementptr inbounds { x86_fp80, x86_fp80 }, ptr %z, i64 0, i32 1
  %z.imag4 = load x86_fp80, ptr %z.imagp, align 16
  %call = tail call x86_fp80 @atan2l(x86_fp80 noundef %z.imag4, x86_fp80 noundef %z.real) #4
  %0 = tail call x86_fp80 @llvm.fabs.f80(x86_fp80 %z.real)
  %1 = tail call x86_fp80 @llvm.fabs.f80(x86_fp80 %z.imag4)
  %cmp = fcmp olt x86_fp80 %0, %1
  %ay.0 = select i1 %cmp, x86_fp80 %0, x86_fp80 %1
  %ax.0 = select i1 %cmp, x86_fp80 %1, x86_fp80 %0
  %2 = bitcast x86_fp80 %ax.0 to i80
  %3 = lshr i80 %2, 64
  %conv = trunc i80 %3 to i32
  %4 = bitcast x86_fp80 %ay.0 to i80
  %5 = lshr i80 %4, 64
  %conv12 = trunc i80 %5 to i32
  %cmp17 = icmp eq i32 %conv, 32767
  %cmp19 = icmp eq i32 %conv12, 32767
  %or.cond = or i1 %cmp17, %cmp19
  br i1 %or.cond, label %if.then21, label %if.end26

if.then21:                                        ; preds = %entry
  %call22 = tail call x86_fp80 @hypotl(x86_fp80 noundef %z.real, x86_fp80 noundef %z.imag4) #4
  %call23 = tail call x86_fp80 @logl(x86_fp80 noundef %call22) #4
  br label %cleanup

if.end26:                                         ; preds = %entry
  %cmp27 = fcmp oeq x86_fp80 %ax.0, 0xK3FFF8000000000000000
  br i1 %cmp27, label %if.then29, label %if.end47

if.then29:                                        ; preds = %if.end26
  %cmp30 = icmp ult i32 %conv12, 8192
  br i1 %cmp30, label %if.then32, label %if.end38

if.then32:                                        ; preds = %if.then29
  %div = fmul x86_fp80 %ay.0, 0xK3FFE8000000000000000
  %mul = fmul x86_fp80 %ay.0, %div
  br label %cleanup

if.end38:                                         ; preds = %if.then29
  %mul40 = fmul x86_fp80 %ay.0, %ay.0
  %call41 = tail call x86_fp80 @log1pl(x86_fp80 noundef %mul40) #4
  %div42 = fmul x86_fp80 %call41, 0xK3FFE8000000000000000
  br label %cleanup

if.end47:                                         ; preds = %if.end26
  %sub48 = sub nsw i32 %conv, %conv12
  %cmp49 = icmp sgt i32 %sub48, 64
  %cmp52 = fcmp oeq x86_fp80 %ay.0, 0xK00000000000000000000
  %or.cond235 = or i1 %cmp52, %cmp49
  br i1 %or.cond235, label %if.then54, label %if.end61

if.then54:                                        ; preds = %if.end47
  %call56 = tail call x86_fp80 @logl(x86_fp80 noundef %ax.0) #4
  br label %cleanup

if.end61:                                         ; preds = %if.end47
  %cmp62 = icmp ugt i32 %conv, 32765
  br i1 %cmp62, label %if.then64, label %if.end75

if.then64:                                        ; preds = %if.end61
  %mul66 = fmul x86_fp80 %z.real, 0xK00018000000000000000
  %mul67 = fmul x86_fp80 %z.imag4, 0xK00018000000000000000
  %call68 = tail call x86_fp80 @hypotl(x86_fp80 noundef %mul66, x86_fp80 noundef %mul67) #4
  %call69 = tail call x86_fp80 @logl(x86_fp80 noundef %call68) #4
  %add = fadd x86_fp80 %call69, 0xK3FE5E7B597FE42B24000
  %add70 = fadd x86_fp80 %add, 0xK400CB16C8C6710418000
  br label %cleanup

if.end75:                                         ; preds = %if.end61
  %cmp76 = icmp ugt i32 %conv, 24573
  br i1 %cmp76, label %if.then78, label %if.end86

if.then78:                                        ; preds = %if.end75
  %call80 = tail call x86_fp80 @hypotl(x86_fp80 noundef %z.real, x86_fp80 noundef %z.imag4) #4
  %call81 = tail call x86_fp80 @logl(x86_fp80 noundef %call80) #4
  br label %cleanup

if.end86:                                         ; preds = %if.end75
  %cmp87 = icmp eq i32 %conv, 0
  br i1 %cmp87, label %if.then89, label %if.end101

if.then89:                                        ; preds = %if.end86
  %mul91 = fmul x86_fp80 %z.real, 0xK7FFE8000000000000000
  %mul92 = fmul x86_fp80 %z.imag4, 0xK7FFE8000000000000000
  %call93 = tail call x86_fp80 @hypotl(x86_fp80 noundef %mul91, x86_fp80 noundef %mul92) #4
  %call94 = tail call x86_fp80 @logl(x86_fp80 noundef %call93) #4
  %add95 = fadd x86_fp80 %call94, 0xKBFE5E7B936F19A460800
  %add96 = fadd x86_fp80 %add95, 0xKC00CB16F522F7020C000
  br label %cleanup

if.end101:                                        ; preds = %if.end86
  %cmp102 = icmp ult i32 %conv12, 8256
  br i1 %cmp102, label %if.then104, label %if.end112

if.then104:                                       ; preds = %if.end101
  %call106 = tail call x86_fp80 @hypotl(x86_fp80 noundef %z.real, x86_fp80 noundef %z.imag4) #4
  %call107 = tail call x86_fp80 @logl(x86_fp80 noundef %call106) #4
  br label %cleanup

if.end112:                                        ; preds = %if.end101
  %mul113 = fmul x86_fp80 %ax.0, 0xK401F8000000080000000
  %sub114 = fsub x86_fp80 %ax.0, %mul113
  %add115 = fadd x86_fp80 %mul113, %sub114
  %sub116 = fsub x86_fp80 %ax.0, %add115
  %mul117 = fmul x86_fp80 %ax.0, %ax.0
  %neg = fneg x86_fp80 %mul117
  %6 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %add115, x86_fp80 %add115, x86_fp80 %neg)
  %mul119 = fmul x86_fp80 %add115, 0xK40008000000000000000
  %7 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %mul119, x86_fp80 %sub116, x86_fp80 %6)
  %8 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub116, x86_fp80 %sub116, x86_fp80 %7)
  %mul122 = fmul x86_fp80 %ay.0, 0xK401F8000000080000000
  %sub123 = fsub x86_fp80 %ay.0, %mul122
  %add124 = fadd x86_fp80 %mul122, %sub123
  %sub125 = fsub x86_fp80 %ay.0, %add124
  %mul126 = fmul x86_fp80 %ay.0, %ay.0
  %neg128 = fneg x86_fp80 %mul126
  %9 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %add124, x86_fp80 %add124, x86_fp80 %neg128)
  %mul129 = fmul x86_fp80 %add124, 0xK40008000000000000000
  %10 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %mul129, x86_fp80 %sub125, x86_fp80 %9)
  %11 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub125, x86_fp80 %sub125, x86_fp80 %10)
  %add133 = fadd x86_fp80 %mul117, %mul126
  %sub134 = fsub x86_fp80 %mul117, %add133
  %add135 = fadd x86_fp80 %mul126, %sub134
  %cmp138 = fcmp olt x86_fp80 %add133, 0xK3FFE8000000000000000
  %cmp141 = fcmp oge x86_fp80 %add133, 0xK4000C000000000000000
  %or.cond236 = or i1 %cmp138, %cmp141
  br i1 %or.cond236, label %if.then143, label %if.end154

if.then143:                                       ; preds = %if.end112
  %add145 = fadd x86_fp80 %11, %8
  %add146 = fadd x86_fp80 %add135, %add145
  %add147 = fadd x86_fp80 %add133, %add146
  %call148 = tail call x86_fp80 @logl(x86_fp80 noundef %add147) #4
  %div149 = fmul x86_fp80 %call148, 0xK3FFE8000000000000000
  br label %cleanup

if.end154:                                        ; preds = %if.end112
  %sub155 = fadd x86_fp80 %add133, 0xKBFFF8000000000000000
  %add158 = fadd x86_fp80 %sub155, %add135
  %sub159 = fsub x86_fp80 %add158, %sub155
  %12 = fsub x86_fp80 %sub159, %add158
  %sub161 = fadd x86_fp80 %sub155, %12
  %sub162 = fsub x86_fp80 %add135, %sub159
  %add163 = fadd x86_fp80 %sub162, %sub161
  %add169 = fadd x86_fp80 %8, %11
  %sub170 = fsub x86_fp80 %add169, %8
  %13 = fsub x86_fp80 %sub170, %add169
  %sub172 = fadd x86_fp80 %8, %13
  %sub173 = fsub x86_fp80 %11, %sub170
  %add174 = fadd x86_fp80 %sub173, %sub172
  %add180 = fadd x86_fp80 %add158, %add169
  %sub181 = fsub x86_fp80 %add180, %add158
  %14 = fsub x86_fp80 %sub181, %add180
  %sub183 = fadd x86_fp80 %add158, %14
  %sub184 = fsub x86_fp80 %add169, %sub181
  %add185 = fadd x86_fp80 %sub184, %sub183
  %add191 = fadd x86_fp80 %add163, %add174
  %sub192 = fsub x86_fp80 %add191, %add163
  %15 = fsub x86_fp80 %sub192, %add191
  %sub194 = fadd x86_fp80 %add163, %15
  %sub195 = fsub x86_fp80 %add174, %sub192
  %add196 = fadd x86_fp80 %sub195, %sub194
  %add199 = fadd x86_fp80 %add185, %add191
  %add202 = fadd x86_fp80 %add180, %add199
  %sub203 = fsub x86_fp80 %add180, %add202
  %add204 = fadd x86_fp80 %add199, %sub203
  %add208 = fadd x86_fp80 %add196, %add204
  %add209 = fadd x86_fp80 %add202, %add208
  %call210 = tail call x86_fp80 @log1pl(x86_fp80 noundef %add209) #4
  %div211 = fmul x86_fp80 %call210, 0xK3FFE8000000000000000
  br label %cleanup

cleanup:                                          ; preds = %if.end154, %if.then143, %if.then104, %if.then89, %if.then78, %if.then64, %if.then54, %if.end38, %if.then32, %if.then21
  %retval.sroa.0.0 = phi x86_fp80 [ %call23, %if.then21 ], [ %mul, %if.then32 ], [ %div42, %if.end38 ], [ %call56, %if.then54 ], [ %add70, %if.then64 ], [ %call81, %if.then78 ], [ %add96, %if.then89 ], [ %call107, %if.then104 ], [ %div149, %if.then143 ], [ %div211, %if.end154 ]
  %.fca.0.insert = insertvalue { x86_fp80, x86_fp80 } poison, x86_fp80 %retval.sroa.0.0, 0
  %.fca.1.insert = insertvalue { x86_fp80, x86_fp80 } %.fca.0.insert, x86_fp80 %call, 1
  ret { x86_fp80, x86_fp80 } %.fca.1.insert
}

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare x86_fp80 @atan2l(x86_fp80 noundef, x86_fp80 noundef) local_unnamed_addr #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare x86_fp80 @llvm.fabs.f80(x86_fp80) #2

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare x86_fp80 @logl(x86_fp80 noundef) local_unnamed_addr #1

; Function Attrs: nounwind
declare x86_fp80 @hypotl(x86_fp80 noundef, x86_fp80 noundef) local_unnamed_addr #3

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare x86_fp80 @log1pl(x86_fp80 noundef) local_unnamed_addr #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare x86_fp80 @llvm.fmuladd.f80(x86_fp80, x86_fp80, x86_fp80) #2

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nofree nounwind willreturn memory(write) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #3 = { nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
