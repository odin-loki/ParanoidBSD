; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/e_hypotl_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/e_hypotl_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: mustprogress nofree nounwind willreturn memory(write) uwtable
define dso_local x86_fp80 @ref_hypotl(x86_fp80 noundef %x, x86_fp80 noundef %y) local_unnamed_addr #0 {
entry:
  %0 = bitcast x86_fp80 %x to i80
  %1 = lshr i80 %0, 64
  %bf.cast = trunc i80 %1 to i32
  %and = and i32 %bf.cast, 32767
  %2 = bitcast x86_fp80 %y to i80
  %3 = lshr i80 %2, 64
  %bf.cast6 = trunc i80 %3 to i32
  %and9 = and i32 %bf.cast6, 32767
  %cmp = icmp ugt i32 %and9, %and
  %and.and9 = tail call i32 @llvm.umin.i32(i32 %and9, i32 %and)
  %and9.and = tail call i32 @llvm.umax.i32(i32 %and9, i32 %and)
  %x.y = select i1 %cmp, x86_fp80 %x, x86_fp80 %y
  %y.x = select i1 %cmp, x86_fp80 %y, x86_fp80 %x
  %4 = tail call x86_fp80 @llvm.fabs.f80(x86_fp80 %y.x)
  %5 = tail call x86_fp80 @llvm.fabs.f80(x86_fp80 %x.y)
  %sub = sub nsw i32 %and9.and, %and.and9
  %cmp10 = icmp sgt i32 %sub, 71
  br i1 %cmp10, label %if.then11, label %if.end12

if.then11:                                        ; preds = %entry
  %add = fadd x86_fp80 %4, %5
  br label %cleanup185

if.end12:                                         ; preds = %entry
  %cmp13 = icmp ugt i32 %and9.and, 24563
  br i1 %cmp13, label %if.then14, label %if.end71

if.then14:                                        ; preds = %if.end12
  %cmp15 = icmp eq i32 %and9.and, 32767
  br i1 %cmp15, label %if.then16, label %if.end52

if.then16:                                        ; preds = %if.then14
  %add17 = fadd x86_fp80 %x, 0xK00000000000000000000
  %6 = tail call x86_fp80 @llvm.fabs.f80(x86_fp80 %add17)
  %add18 = fadd x86_fp80 %y, 0xK00000000000000000000
  %7 = tail call x86_fp80 @llvm.fabs.f80(x86_fp80 %add18)
  %sub19 = fsub x86_fp80 %6, %7
  %8 = bitcast x86_fp80 %4 to i80
  %9 = and i80 %8, 18446744073709551615
  %or.cond = icmp eq i80 %9, 9223372036854775808
  %spec.select = select i1 %or.cond, x86_fp80 %4, x86_fp80 %sub19
  %10 = bitcast x86_fp80 %5 to i80
  %cmp45 = icmp eq i32 %and.and9, 32767
  %11 = and i80 %10, 18446744073709551615
  %12 = icmp eq i80 %11, 9223372036854775808
  %or.cond197 = and i1 %cmp45, %12
  %w.1 = select i1 %or.cond197, x86_fp80 %5, x86_fp80 %spec.select
  br label %cleanup185

if.end52:                                         ; preds = %if.then14
  %sub53 = add nsw i32 %and9.and, -8280
  %sub54 = add nsw i32 %and.and9, -8280
  %13 = bitcast x86_fp80 %4 to i80
  %bf.value = zext nneg i32 %sub53 to i80
  %bf.shl = shl nuw nsw i80 %bf.value, 64
  %14 = and i80 %13, 18446744073709551615
  %bf.set = or disjoint i80 %14, %bf.shl
  %15 = bitcast i80 %bf.set to x86_fp80
  %16 = bitcast x86_fp80 %5 to i80
  %bf.value64 = zext i32 %sub54 to i80
  %bf.shl65 = shl i80 %bf.value64, 64
  %17 = and i80 %16, 18446744073709551615
  %bf.set67 = or disjoint i80 %17, %bf.shl65
  %18 = bitcast i80 %bf.set67 to x86_fp80
  br label %if.end71

if.end71:                                         ; preds = %if.end52, %if.end12
  %hb.1 = phi i32 [ %sub54, %if.end52 ], [ %and.and9, %if.end12 ]
  %ha.1 = phi i32 [ %sub53, %if.end52 ], [ %and9.and, %if.end12 ]
  %k.0 = phi i32 [ 8280, %if.end52 ], [ 0, %if.end12 ]
  %b.1 = phi x86_fp80 [ %18, %if.end52 ], [ %5, %if.end12 ]
  %a.1 = phi x86_fp80 [ %15, %if.end52 ], [ %4, %if.end12 ]
  %cmp72 = icmp slt i32 %hb.1, 8203
  br i1 %cmp72, label %if.then73, label %if.end127

if.then73:                                        ; preds = %if.end71
  %cmp74 = icmp slt i32 %hb.1, 1
  br i1 %cmp74, label %if.then75, label %if.else102

if.then75:                                        ; preds = %if.then73
  %19 = bitcast x86_fp80 %b.1 to i80
  %20 = lshr i80 %19, 32
  %or318 = or i80 %20, %19
  %or = trunc i80 %or318 to i32
  %cmp89.not = icmp eq i32 %or, 0
  %mul = fmul x86_fp80 %b.1, 0xK7FFD8000000000000000
  %mul99 = fmul x86_fp80 %a.1, 0xK7FFD8000000000000000
  %sub100 = or disjoint i32 %k.0, -16382
  br i1 %cmp89.not, label %cleanup185, label %if.end127

if.else102:                                       ; preds = %if.then73
  %add103 = add nuw nsw i32 %ha.1, 8280
  %add104 = add nuw nsw i32 %hb.1, 8280
  %sub105 = add nsw i32 %k.0, -8280
  %21 = bitcast x86_fp80 %a.1 to i80
  %bf.value109 = zext nneg i32 %add103 to i80
  %bf.shl110 = shl nuw i80 %bf.value109, 64
  %22 = and i80 %21, 18446744073709551615
  %bf.set112 = or disjoint i80 %22, %bf.shl110
  %23 = bitcast i80 %bf.set112 to x86_fp80
  %24 = bitcast x86_fp80 %b.1 to i80
  %bf.value119 = zext nneg i32 %add104 to i80
  %bf.shl120 = shl nuw nsw i80 %bf.value119, 64
  %25 = and i80 %24, 18446744073709551615
  %bf.set122 = or disjoint i80 %25, %bf.shl120
  %26 = bitcast i80 %bf.set122 to x86_fp80
  br label %if.end127

if.end127:                                        ; preds = %if.else102, %if.then75, %if.end71
  %k.2 = phi i32 [ %sub100, %if.then75 ], [ %sub105, %if.else102 ], [ %k.0, %if.end71 ]
  %b.3 = phi x86_fp80 [ %mul, %if.then75 ], [ %26, %if.else102 ], [ %b.1, %if.end71 ]
  %a.3 = phi x86_fp80 [ %mul99, %if.then75 ], [ %23, %if.else102 ], [ %a.1, %if.end71 ]
  %sub128 = fsub x86_fp80 %a.3, %b.3
  %cmp129 = fcmp ogt x86_fp80 %sub128, %b.3
  br i1 %cmp129, label %if.then130, label %if.else141

if.then130:                                       ; preds = %if.end127
  %27 = bitcast x86_fp80 %a.3 to i80
  %28 = and i80 %27, -4294967296
  %29 = bitcast i80 %28 to x86_fp80
  %sub135 = fsub x86_fp80 %a.3, %29
  %fneg = fneg x86_fp80 %b.3
  %add138 = fadd x86_fp80 %a.3, %29
  %30 = fneg x86_fp80 %sub135
  %neg = fmul x86_fp80 %add138, %30
  %31 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %b.3, x86_fp80 %fneg, x86_fp80 %neg)
  %neg140 = fneg x86_fp80 %31
  %32 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %29, x86_fp80 %29, x86_fp80 %neg140)
  br label %if.end160

if.else141:                                       ; preds = %if.end127
  %add142 = fadd x86_fp80 %a.3, %a.3
  %33 = bitcast x86_fp80 %b.3 to i80
  %34 = and i80 %33, -4294967296
  %35 = bitcast i80 %34 to x86_fp80
  %sub147 = fsub x86_fp80 %b.3, %35
  %36 = bitcast x86_fp80 %add142 to i80
  %37 = and i80 %36, -4294967296
  %38 = bitcast i80 %37 to x86_fp80
  %sub151 = fsub x86_fp80 %add142, %38
  %fneg153 = fneg x86_fp80 %sub128
  %mul156 = fmul x86_fp80 %b.3, %sub151
  %39 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %38, x86_fp80 %sub147, x86_fp80 %mul156)
  %neg157 = fneg x86_fp80 %39
  %40 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub128, x86_fp80 %fneg153, x86_fp80 %neg157)
  %neg158 = fneg x86_fp80 %40
  %41 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %38, x86_fp80 %35, x86_fp80 %neg158)
  br label %if.end160

if.end160:                                        ; preds = %if.else141, %if.then130
  %.sink = phi x86_fp80 [ %41, %if.else141 ], [ %32, %if.then130 ]
  %call159 = tail call x86_fp80 @sqrtl(x86_fp80 noundef %.sink) #4
  %cmp161.not = icmp eq i32 %k.2, 0
  br i1 %cmp161.not, label %cleanup185, label %if.then162

if.then162:                                       ; preds = %if.end160
  %add173 = add nsw i32 %k.2, 16383
  %bf.value175 = zext nneg i32 %add173 to i80
  %bf.shl176 = shl nuw nsw i80 %bf.value175, 64
  %bf.set178 = or disjoint i80 %bf.shl176, 9223372036854775808
  %42 = bitcast i80 %bf.set178 to x86_fp80
  %mul182 = fmul x86_fp80 %call159, %42
  br label %cleanup185

cleanup185:                                       ; preds = %if.end160, %if.then75, %if.then162, %if.then16, %if.then11
  %retval.1 = phi x86_fp80 [ %add, %if.then11 ], [ %w.1, %if.then16 ], [ %mul182, %if.then162 ], [ %a.1, %if.then75 ], [ %call159, %if.end160 ]
  ret x86_fp80 %retval.1
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare x86_fp80 @llvm.fabs.f80(x86_fp80) #1

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare x86_fp80 @sqrtl(x86_fp80 noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare x86_fp80 @llvm.fmuladd.f80(x86_fp80, x86_fp80, x86_fp80) #1

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.umin.i32(i32, i32) #3

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.umax.i32(i32, i32) #3

attributes #0 = { mustprogress nofree nounwind willreturn memory(write) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { mustprogress nofree nounwind willreturn memory(write) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
