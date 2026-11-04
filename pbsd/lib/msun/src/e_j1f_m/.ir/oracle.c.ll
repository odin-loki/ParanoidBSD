; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/e_j1f_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/e_j1f_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@vone = internal constant float 1.000000e+00, align 4
@vzero = internal constant float 0.000000e+00, align 4
@pr8 = internal unnamed_addr constant [6 x float] [float 0.000000e+00, float 0x3FBE000000000000, float 0x402A7A9D40000000, float 0x4079C0D460000000, float 0x40AE457DA0000000, float 0x40BEEA7AC0000000], align 16
@ps8 = internal unnamed_addr constant [5 x float] [float 0x405C8D4580000000, float 0x40AC85DCA0000000, float 0x40E20B86A0000000, float 0x40F7D42CC0000000, float 0x40DE151160000000], align 16
@pr5 = internal unnamed_addr constant [6 x float] [float 0x3DAD0667E0000000, float 0x3FBDFFFFE0000000, float 0x401B360460000000, float 0x405B13B940000000, float 0x40802D16E0000000, float 0x408085B8C0000000], align 16
@ps5 = internal unnamed_addr constant [5 x float] [float 0x404DA3EAA0000000, float 0x408EFB3620000000, float 0x40B4E94460000000, float 0x40BEA4B0C0000000, float 0x4097803000000000], align 16
@pr3 = internal unnamed_addr constant [6 x float] [float 0x3E29FC21A0000000, float 0x3FBDFFF560000000, float 0x400F76BCE0000000, float 0x40418F48A0000000, float 0x4056C38540000000, float 0x4048478F80000000], align 16
@ps3 = internal unnamed_addr constant [5 x float] [float 0x40416549A0000000, float 0x40750C3300000000, float 0x40905B7C60000000, float 0x408BD67DA0000000, float 0x4059F26D80000000], align 16
@pr2 = internal unnamed_addr constant [6 x float] [float 0x3E7CE9D500000000, float 0x3FBDFF42C0000000, float 0x4002F2B800000000, float 0x40287C3780000000, float 0x4031B1A820000000, float 0x40144B49A0000000], align 16
@ps2 = internal unnamed_addr constant [5 x float] [float 0x40356FBD80000000, float 0x405F529320000000, float 0x406D08D8E0000000, float 0x405D6B7AE0000000, float 0x4020BAB200000000], align 16
@qr8 = internal unnamed_addr constant [6 x float] [float 0.000000e+00, float 0xBFBA400000000000, float 0xC0304591A0000000, float 0xC087BCD060000000, float 0xC0C724E740000000, float 0xC0E7A6D060000000], align 16
@qs8 = internal unnamed_addr constant [6 x float] [float 0x40642CA6E0000000, float 0x40BE9162E0000000, float 0x4100579AC0000000, float 0x4125F65380000000, float 0x412457D280000000, float 0xC111F96900000000], align 16
@qr5 = internal unnamed_addr constant [6 x float] [float 0xBDB6FA4320000000, float 0xBFBA3FFFC0000000, float 0xC0201CE6C0000000, float 0xC066F56D60000000, float 0xC09574C660000000, float 0xC0A468E380000000], align 16
@qs5 = internal unnamed_addr constant [6 x float] [float 0x405451B300000000, float 0x409F1F31E0000000, float 0x40D10F1F00000000, float 0x40E8576DA0000000, float 0x40DB4B04C0000000, float 0xC0B26F2F00000000], align 16
@qr3 = internal unnamed_addr constant [6 x float] [float 0xBE35CFA9E0000000, float 0xBFBA3FEB60000000, float 0xC01270C240000000, float 0xC04CEC71C0000000, float 0xC06C87D340000000, float 0xC06B66B960000000], align 16
@qs3 = internal unnamed_addr constant [6 x float] [float 0x4047D523C0000000, float 0x40850EEBC0000000, float 0x40AA684E40000000, float 0x40B5ABBAA0000000, float 0x409DBC7A00000000, float 0xC060E67020000000], align 16
@qr2 = internal unnamed_addr constant [6 x float] [float 0xBE87F12640000000, float 0xBFBA3E8EA0000000, float 0xC006048460000000, float 0xC033A9E2C0000000, float 0xC04529A3E0000000, float 0xC0355F3640000000], align 16
@qs2 = internal unnamed_addr constant [6 x float] [float 0x403D888A80000000, float 0x406F9F68E0000000, float 0x4087AC05C0000000, float 0x40871B2540000000, float 0x40637E5E40000000, float 0xC013D686E0000000], align 16

; Function Attrs: nounwind uwtable
define dso_local float @ref_j1f(float noundef %x) local_unnamed_addr #0 {
entry:
  %s = alloca float, align 4
  %c = alloca float, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %s) #6
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %c) #6
  %0 = bitcast float %x to i32
  %1 = tail call float @llvm.fabs.f32(float %x)
  %and = bitcast float %1 to i32
  %cmp = icmp sgt i32 %and, 2139095039
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %div = fdiv float 1.000000e+00, %x
  br label %cleanup

if.end:                                           ; preds = %entry
  %cmp1 = icmp sgt i32 %and, 1073741823
  br i1 %cmp1, label %if.then2, label %if.end30

if.then2:                                         ; preds = %if.end
  call void @sincosf(float noundef %1, ptr noundef nonnull %s, ptr noundef nonnull %c) #6
  %2 = load float, ptr %s, align 4, !tbaa !5
  %3 = load float, ptr %c, align 4, !tbaa !5
  %sub3 = fsub float %2, %3
  %cmp4 = icmp ult i32 %and, 2130706432
  br i1 %cmp4, label %if.then5, label %if.end25

if.then5:                                         ; preds = %if.then2
  %fneg = fneg float %2
  %sub = fsub float %fneg, %3
  %add = fadd float %1, %1
  %call = call float @cosf(float noundef %add) #6
  %4 = load float, ptr %s, align 4, !tbaa !5
  %5 = load float, ptr %c, align 4, !tbaa !5
  %mul = fmul float %4, %5
  %cmp6 = fcmp ogt float %mul, 0.000000e+00
  %div8 = fdiv float %call, %sub
  %div9 = fdiv float %call, %sub3
  %cc.0 = select i1 %cmp6, float %div8, float %sub3
  %ss.0 = select i1 %cmp6, float %sub, float %div9
  %cmp12 = icmp ugt i32 %and, 1476395008
  br i1 %cmp12, label %if.end25, label %if.else17

if.else17:                                        ; preds = %if.then5
  %cmp.i = icmp ugt i32 %and, 1090519039
  br i1 %cmp.i, label %ref_ponef.exit, label %if.else.i

if.else.i:                                        ; preds = %if.else17
  %cmp1.i = icmp ugt i32 %and, 1083274218
  br i1 %cmp1.i, label %ref_ponef.exit, label %if.else3.i

if.else3.i:                                       ; preds = %if.else.i
  %cmp4.i = icmp ugt i32 %and, 1077336342
  %ps3.ps2.i = select i1 %cmp4.i, ptr @ps3, ptr @ps2
  %pr3.pr2.i = select i1 %cmp4.i, ptr @pr3, ptr @pr2
  br label %ref_ponef.exit

ref_ponef.exit:                                   ; preds = %if.else17, %if.else.i, %if.else3.i
  %q.0.i = phi ptr [ @ps8, %if.else17 ], [ @ps5, %if.else.i ], [ %ps3.ps2.i, %if.else3.i ]
  %p.0.i = phi ptr [ @pr8, %if.else17 ], [ @pr5, %if.else.i ], [ %pr3.pr2.i, %if.else3.i ]
  %mul.i = fmul float %x, %x
  %div.i = fdiv float 1.000000e+00, %mul.i
  %6 = load float, ptr %p.0.i, align 4, !tbaa !5
  %arrayidx9.i = getelementptr inbounds float, ptr %p.0.i, i64 1
  %7 = load float, ptr %arrayidx9.i, align 4, !tbaa !5
  %arrayidx10.i = getelementptr inbounds float, ptr %p.0.i, i64 2
  %8 = load float, ptr %arrayidx10.i, align 4, !tbaa !5
  %arrayidx11.i = getelementptr inbounds float, ptr %p.0.i, i64 3
  %9 = load float, ptr %arrayidx11.i, align 4, !tbaa !5
  %arrayidx12.i = getelementptr inbounds float, ptr %p.0.i, i64 4
  %10 = load float, ptr %arrayidx12.i, align 4, !tbaa !5
  %arrayidx13.i = getelementptr inbounds float, ptr %p.0.i, i64 5
  %11 = load float, ptr %arrayidx13.i, align 4, !tbaa !5
  %12 = call float @llvm.fmuladd.f32(float %div.i, float %11, float %10)
  %13 = call float @llvm.fmuladd.f32(float %div.i, float %12, float %9)
  %14 = call float @llvm.fmuladd.f32(float %div.i, float %13, float %8)
  %15 = call float @llvm.fmuladd.f32(float %div.i, float %14, float %7)
  %16 = call float @llvm.fmuladd.f32(float %div.i, float %15, float %6)
  %17 = load float, ptr %q.0.i, align 4, !tbaa !5
  %arrayidx20.i = getelementptr inbounds float, ptr %q.0.i, i64 1
  %18 = load float, ptr %arrayidx20.i, align 4, !tbaa !5
  %arrayidx21.i = getelementptr inbounds float, ptr %q.0.i, i64 2
  %19 = load float, ptr %arrayidx21.i, align 4, !tbaa !5
  %arrayidx22.i = getelementptr inbounds float, ptr %q.0.i, i64 3
  %20 = load float, ptr %arrayidx22.i, align 4, !tbaa !5
  %arrayidx23.i = getelementptr inbounds float, ptr %q.0.i, i64 4
  %21 = load float, ptr %arrayidx23.i, align 4, !tbaa !5
  %22 = call float @llvm.fmuladd.f32(float %div.i, float %21, float %20)
  %23 = call float @llvm.fmuladd.f32(float %div.i, float %22, float %19)
  %24 = call float @llvm.fmuladd.f32(float %div.i, float %23, float %18)
  %25 = call float @llvm.fmuladd.f32(float %div.i, float %24, float %17)
  %26 = call float @llvm.fmuladd.f32(float %div.i, float %25, float 1.000000e+00)
  %div29.i = fdiv float %16, %26
  %add.i = fadd float %div29.i, 1.000000e+00
  br i1 %cmp.i, label %ref_qonef.exit, label %if.else.i99

if.else.i99:                                      ; preds = %ref_ponef.exit
  %cmp1.i100 = icmp ugt i32 %and, 1083274218
  br i1 %cmp1.i100, label %ref_qonef.exit, label %if.else3.i101

if.else3.i101:                                    ; preds = %if.else.i99
  %cmp4.i102 = icmp ugt i32 %and, 1077336342
  %qs3.qs2.i = select i1 %cmp4.i102, ptr @qs3, ptr @qs2
  %qr3.qr2.i = select i1 %cmp4.i102, ptr @qr3, ptr @qr2
  br label %ref_qonef.exit

ref_qonef.exit:                                   ; preds = %ref_ponef.exit, %if.else.i99, %if.else3.i101
  %q.0.i103 = phi ptr [ @qs8, %ref_ponef.exit ], [ @qs5, %if.else.i99 ], [ %qs3.qs2.i, %if.else3.i101 ]
  %p.0.i104 = phi ptr [ @qr8, %ref_ponef.exit ], [ @qr5, %if.else.i99 ], [ %qr3.qr2.i, %if.else3.i101 ]
  %27 = load float, ptr %p.0.i104, align 4, !tbaa !5
  %arrayidx9.i107 = getelementptr inbounds float, ptr %p.0.i104, i64 1
  %28 = load float, ptr %arrayidx9.i107, align 4, !tbaa !5
  %arrayidx10.i108 = getelementptr inbounds float, ptr %p.0.i104, i64 2
  %29 = load float, ptr %arrayidx10.i108, align 4, !tbaa !5
  %arrayidx11.i109 = getelementptr inbounds float, ptr %p.0.i104, i64 3
  %30 = load float, ptr %arrayidx11.i109, align 4, !tbaa !5
  %arrayidx12.i110 = getelementptr inbounds float, ptr %p.0.i104, i64 4
  %31 = load float, ptr %arrayidx12.i110, align 4, !tbaa !5
  %arrayidx13.i111 = getelementptr inbounds float, ptr %p.0.i104, i64 5
  %32 = load float, ptr %arrayidx13.i111, align 4, !tbaa !5
  %33 = load float, ptr %q.0.i103, align 4, !tbaa !5
  %arrayidx20.i112 = getelementptr inbounds float, ptr %q.0.i103, i64 1
  %34 = load float, ptr %arrayidx20.i112, align 4, !tbaa !5
  %arrayidx21.i113 = getelementptr inbounds float, ptr %q.0.i103, i64 2
  %35 = load float, ptr %arrayidx21.i113, align 4, !tbaa !5
  %arrayidx22.i114 = getelementptr inbounds float, ptr %q.0.i103, i64 3
  %36 = load float, ptr %arrayidx22.i114, align 4, !tbaa !5
  %arrayidx23.i115 = getelementptr inbounds float, ptr %q.0.i103, i64 4
  %37 = load float, ptr %arrayidx23.i115, align 4, !tbaa !5
  %arrayidx24.i = getelementptr inbounds float, ptr %q.0.i103, i64 5
  %38 = load float, ptr %arrayidx24.i, align 4, !tbaa !5
  %39 = call float @llvm.fmuladd.f32(float %div.i, float %38, float %37)
  %40 = insertelement <2 x float> poison, float %div.i, i64 0
  %41 = shufflevector <2 x float> %40, <2 x float> poison, <2 x i32> zeroinitializer
  %42 = insertelement <2 x float> poison, float %32, i64 0
  %43 = insertelement <2 x float> %42, float %39, i64 1
  %44 = insertelement <2 x float> poison, float %31, i64 0
  %45 = insertelement <2 x float> %44, float %36, i64 1
  %46 = call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %41, <2 x float> %43, <2 x float> %45)
  %47 = insertelement <2 x float> poison, float %30, i64 0
  %48 = insertelement <2 x float> %47, float %35, i64 1
  %49 = call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %41, <2 x float> %46, <2 x float> %48)
  %50 = insertelement <2 x float> poison, float %29, i64 0
  %51 = insertelement <2 x float> %50, float %34, i64 1
  %52 = call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %41, <2 x float> %49, <2 x float> %51)
  %53 = insertelement <2 x float> poison, float %28, i64 0
  %54 = insertelement <2 x float> %53, float %33, i64 1
  %55 = call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %41, <2 x float> %52, <2 x float> %54)
  %56 = insertelement <2 x float> <float poison, float 1.000000e+00>, float %27, i64 0
  %57 = call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %41, <2 x float> %55, <2 x float> %56)
  %58 = extractelement <2 x float> %57, i64 0
  %59 = extractelement <2 x float> %57, i64 1
  %div31.i = fdiv float %58, %59
  %add.i116 = fadd float %div31.i, 3.750000e-01
  %60 = fneg float %add.i116
  %61 = fdiv float %60, %1
  %neg = fmul float %ss.0, %61
  %62 = call float @llvm.fmuladd.f32(float %add.i, float %cc.0, float %neg)
  br label %if.end25

if.end25:                                         ; preds = %if.then5, %if.then2, %ref_qonef.exit
  %.sink = phi float [ %62, %ref_qonef.exit ], [ %cc.0, %if.then5 ], [ %sub3, %if.then2 ]
  %mul22 = fmul float %.sink, 0x3FE20DD760000000
  %sqrt121 = call float @llvm.sqrt.f32(float %1)
  %div24 = fdiv float %mul22, %sqrt121
  %cmp26 = icmp slt i32 %0, 0
  br i1 %cmp26, label %if.then27, label %cleanup

if.then27:                                        ; preds = %if.end25
  %fneg28 = fneg float %div24
  br label %cleanup

if.end30:                                         ; preds = %if.end
  %cmp31 = icmp slt i32 %and, 956301312
  %add33 = fadd float %x, 0x46293E5940000000
  %cmp34 = fcmp ogt float %add33, 1.000000e+00
  %or.cond = and i1 %cmp34, %cmp31
  br i1 %or.cond, label %if.then35, label %if.end38

if.then35:                                        ; preds = %if.end30
  %mul36 = fmul float %x, 5.000000e-01
  br label %cleanup

if.end38:                                         ; preds = %if.end30
  %mul39 = fmul float %x, %x
  %63 = tail call float @llvm.fmuladd.f32(float %mul39, float 0x3E6AAAFA40000000, float 0xBEF0C5C6C0000000)
  %64 = tail call float @llvm.fmuladd.f32(float %mul39, float %63, float 0x3F570D9FA0000000)
  %65 = tail call float @llvm.fmuladd.f32(float %mul39, float %64, float -6.250000e-02)
  %mul43 = fmul float %mul39, %65
  %66 = tail call float @llvm.fmuladd.f32(float %mul39, float 0x3DAB2ACFC0000000, float 0x3E35AC88C0000000)
  %67 = tail call float @llvm.fmuladd.f32(float %mul39, float %66, float 0x3EB3BFF840000000)
  %68 = tail call float @llvm.fmuladd.f32(float %mul39, float %67, float 0x3F285F56C0000000)
  %69 = tail call float @llvm.fmuladd.f32(float %mul39, float %68, float 0x3F939D0B20000000)
  %70 = tail call float @llvm.fmuladd.f32(float %mul39, float %69, float 1.000000e+00)
  %mul49 = fmul float %mul43, %x
  %div51 = fdiv float %mul49, %70
  %71 = tail call float @llvm.fmuladd.f32(float %x, float 5.000000e-01, float %div51)
  br label %cleanup

cleanup:                                          ; preds = %if.end25, %if.end38, %if.then35, %if.then27, %if.then
  %retval.0 = phi float [ %div, %if.then ], [ %fneg28, %if.then27 ], [ %mul36, %if.then35 ], [ %71, %if.end38 ], [ %div24, %if.end25 ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %c) #6
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %s) #6
  ret float %retval.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fabs.f32(float) #2

declare void @sincosf(float noundef, ptr noundef, ptr noundef) local_unnamed_addr #3

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare float @cosf(float noundef) local_unnamed_addr #4

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare float @sqrtf(float noundef) local_unnamed_addr #4

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fmuladd.f32(float, float, float) #2

; Function Attrs: nounwind uwtable
define dso_local float @ref_y1f(float noundef %x) local_unnamed_addr #0 {
entry:
  %s = alloca float, align 4
  %c = alloca float, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %s) #6
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %c) #6
  %0 = bitcast float %x to i32
  %1 = tail call float @llvm.fabs.f32(float %x)
  %and = bitcast float %1 to i32
  %cmp = icmp sgt i32 %and, 2139095039
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %2 = load volatile float, ptr @vone, align 4, !tbaa !5
  %3 = tail call float @llvm.fmuladd.f32(float %x, float %x, float %x)
  %div = fdiv float %2, %3
  br label %cleanup

if.end:                                           ; preds = %entry
  %cmp1 = icmp eq i32 %and, 0
  br i1 %cmp1, label %if.then2, label %if.end4

if.then2:                                         ; preds = %if.end
  %4 = load volatile float, ptr @vzero, align 4, !tbaa !5
  %div3 = fdiv float -1.000000e+00, %4
  br label %cleanup

if.end4:                                          ; preds = %if.end
  %cmp5 = icmp slt i32 %0, 0
  br i1 %cmp5, label %if.then6, label %if.end8

if.then6:                                         ; preds = %if.end4
  %5 = load volatile float, ptr @vzero, align 4, !tbaa !5
  %6 = load volatile float, ptr @vzero, align 4, !tbaa !5
  %div7 = fdiv float %5, %6
  br label %cleanup

if.end8:                                          ; preds = %if.end4
  %cmp9 = icmp sgt i32 %and, 1073741823
  br i1 %cmp9, label %if.then10, label %if.end34

if.then10:                                        ; preds = %if.end8
  call void @sincosf(float noundef %x, ptr noundef nonnull %s, ptr noundef nonnull %c) #6
  %7 = load float, ptr %s, align 4, !tbaa !5
  %fneg = fneg float %7
  %8 = load float, ptr %c, align 4, !tbaa !5
  %sub = fsub float %fneg, %8
  %cmp12 = icmp ult i32 %and, 2130706432
  br i1 %cmp12, label %if.then13, label %if.then21

if.then13:                                        ; preds = %if.then10
  %sub11 = fsub float %7, %8
  %add = fadd float %x, %x
  %call = call float @cosf(float noundef %add) #6
  %9 = load float, ptr %s, align 4, !tbaa !5
  %10 = load float, ptr %c, align 4, !tbaa !5
  %mul = fmul float %9, %10
  %cmp14 = fcmp ogt float %mul, 0.000000e+00
  %div16 = fdiv float %call, %sub
  %div17 = fdiv float %call, %sub11
  %cc.0 = select i1 %cmp14, float %div16, float %sub11
  %ss.0 = select i1 %cmp14, float %sub, float %div17
  %cmp20 = icmp ugt i32 %and, 1476395008
  br i1 %cmp20, label %if.then21, label %if.else25

if.then21:                                        ; preds = %if.then10, %if.then13
  %ss.0126 = phi float [ %ss.0, %if.then13 ], [ %sub, %if.then10 ]
  %mul22 = fmul float %ss.0126, 0x3FE20DD760000000
  %call23 = call float @sqrtf(float noundef %x) #6
  %div24 = fdiv float %mul22, %call23
  br label %cleanup

if.else25:                                        ; preds = %if.then13
  %cmp.i = icmp ugt i32 %and, 1090519039
  br i1 %cmp.i, label %ref_ponef.exit, label %if.else.i

if.else.i:                                        ; preds = %if.else25
  %cmp1.i = icmp ugt i32 %and, 1083274218
  br i1 %cmp1.i, label %ref_ponef.exit, label %if.else3.i

if.else3.i:                                       ; preds = %if.else.i
  %cmp4.i = icmp ugt i32 %and, 1077336342
  %ps3.ps2.i = select i1 %cmp4.i, ptr @ps3, ptr @ps2
  %pr3.pr2.i = select i1 %cmp4.i, ptr @pr3, ptr @pr2
  br label %ref_ponef.exit

ref_ponef.exit:                                   ; preds = %if.else25, %if.else.i, %if.else3.i
  %q.0.i = phi ptr [ @ps8, %if.else25 ], [ @ps5, %if.else.i ], [ %ps3.ps2.i, %if.else3.i ]
  %p.0.i = phi ptr [ @pr8, %if.else25 ], [ @pr5, %if.else.i ], [ %pr3.pr2.i, %if.else3.i ]
  %mul.i = fmul float %x, %x
  %div.i = fdiv float 1.000000e+00, %mul.i
  %11 = load float, ptr %p.0.i, align 4, !tbaa !5
  %arrayidx9.i = getelementptr inbounds float, ptr %p.0.i, i64 1
  %12 = load float, ptr %arrayidx9.i, align 4, !tbaa !5
  %arrayidx10.i = getelementptr inbounds float, ptr %p.0.i, i64 2
  %13 = load float, ptr %arrayidx10.i, align 4, !tbaa !5
  %arrayidx11.i = getelementptr inbounds float, ptr %p.0.i, i64 3
  %14 = load float, ptr %arrayidx11.i, align 4, !tbaa !5
  %arrayidx12.i = getelementptr inbounds float, ptr %p.0.i, i64 4
  %15 = load float, ptr %arrayidx12.i, align 4, !tbaa !5
  %arrayidx13.i = getelementptr inbounds float, ptr %p.0.i, i64 5
  %16 = load float, ptr %arrayidx13.i, align 4, !tbaa !5
  %17 = call float @llvm.fmuladd.f32(float %div.i, float %16, float %15)
  %18 = call float @llvm.fmuladd.f32(float %div.i, float %17, float %14)
  %19 = call float @llvm.fmuladd.f32(float %div.i, float %18, float %13)
  %20 = call float @llvm.fmuladd.f32(float %div.i, float %19, float %12)
  %21 = call float @llvm.fmuladd.f32(float %div.i, float %20, float %11)
  %22 = load float, ptr %q.0.i, align 4, !tbaa !5
  %arrayidx20.i = getelementptr inbounds float, ptr %q.0.i, i64 1
  %23 = load float, ptr %arrayidx20.i, align 4, !tbaa !5
  %arrayidx21.i = getelementptr inbounds float, ptr %q.0.i, i64 2
  %24 = load float, ptr %arrayidx21.i, align 4, !tbaa !5
  %arrayidx22.i = getelementptr inbounds float, ptr %q.0.i, i64 3
  %25 = load float, ptr %arrayidx22.i, align 4, !tbaa !5
  %arrayidx23.i = getelementptr inbounds float, ptr %q.0.i, i64 4
  %26 = load float, ptr %arrayidx23.i, align 4, !tbaa !5
  %27 = call float @llvm.fmuladd.f32(float %div.i, float %26, float %25)
  %28 = call float @llvm.fmuladd.f32(float %div.i, float %27, float %24)
  %29 = call float @llvm.fmuladd.f32(float %div.i, float %28, float %23)
  %30 = call float @llvm.fmuladd.f32(float %div.i, float %29, float %22)
  %31 = call float @llvm.fmuladd.f32(float %div.i, float %30, float 1.000000e+00)
  %div29.i = fdiv float %21, %31
  %add.i = fadd float %div29.i, 1.000000e+00
  br i1 %cmp.i, label %ref_qonef.exit, label %if.else.i105

if.else.i105:                                     ; preds = %ref_ponef.exit
  %cmp1.i106 = icmp ugt i32 %and, 1083274218
  br i1 %cmp1.i106, label %ref_qonef.exit, label %if.else3.i107

if.else3.i107:                                    ; preds = %if.else.i105
  %cmp4.i108 = icmp ugt i32 %and, 1077336342
  %qs3.qs2.i = select i1 %cmp4.i108, ptr @qs3, ptr @qs2
  %qr3.qr2.i = select i1 %cmp4.i108, ptr @qr3, ptr @qr2
  br label %ref_qonef.exit

ref_qonef.exit:                                   ; preds = %ref_ponef.exit, %if.else.i105, %if.else3.i107
  %q.0.i109 = phi ptr [ @qs8, %ref_ponef.exit ], [ @qs5, %if.else.i105 ], [ %qs3.qs2.i, %if.else3.i107 ]
  %p.0.i110 = phi ptr [ @qr8, %ref_ponef.exit ], [ @qr5, %if.else.i105 ], [ %qr3.qr2.i, %if.else3.i107 ]
  %32 = load float, ptr %p.0.i110, align 4, !tbaa !5
  %arrayidx9.i113 = getelementptr inbounds float, ptr %p.0.i110, i64 1
  %33 = load float, ptr %arrayidx9.i113, align 4, !tbaa !5
  %arrayidx10.i114 = getelementptr inbounds float, ptr %p.0.i110, i64 2
  %34 = load float, ptr %arrayidx10.i114, align 4, !tbaa !5
  %arrayidx11.i115 = getelementptr inbounds float, ptr %p.0.i110, i64 3
  %35 = load float, ptr %arrayidx11.i115, align 4, !tbaa !5
  %arrayidx12.i116 = getelementptr inbounds float, ptr %p.0.i110, i64 4
  %36 = load float, ptr %arrayidx12.i116, align 4, !tbaa !5
  %arrayidx13.i117 = getelementptr inbounds float, ptr %p.0.i110, i64 5
  %37 = load float, ptr %arrayidx13.i117, align 4, !tbaa !5
  %38 = load float, ptr %q.0.i109, align 4, !tbaa !5
  %arrayidx20.i118 = getelementptr inbounds float, ptr %q.0.i109, i64 1
  %39 = load float, ptr %arrayidx20.i118, align 4, !tbaa !5
  %arrayidx21.i119 = getelementptr inbounds float, ptr %q.0.i109, i64 2
  %40 = load float, ptr %arrayidx21.i119, align 4, !tbaa !5
  %arrayidx22.i120 = getelementptr inbounds float, ptr %q.0.i109, i64 3
  %41 = load float, ptr %arrayidx22.i120, align 4, !tbaa !5
  %arrayidx23.i121 = getelementptr inbounds float, ptr %q.0.i109, i64 4
  %42 = load float, ptr %arrayidx23.i121, align 4, !tbaa !5
  %arrayidx24.i = getelementptr inbounds float, ptr %q.0.i109, i64 5
  %43 = load float, ptr %arrayidx24.i, align 4, !tbaa !5
  %44 = call float @llvm.fmuladd.f32(float %div.i, float %43, float %42)
  %45 = insertelement <2 x float> poison, float %div.i, i64 0
  %46 = shufflevector <2 x float> %45, <2 x float> poison, <2 x i32> zeroinitializer
  %47 = insertelement <2 x float> poison, float %37, i64 0
  %48 = insertelement <2 x float> %47, float %44, i64 1
  %49 = insertelement <2 x float> poison, float %36, i64 0
  %50 = insertelement <2 x float> %49, float %41, i64 1
  %51 = call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %46, <2 x float> %48, <2 x float> %50)
  %52 = insertelement <2 x float> poison, float %35, i64 0
  %53 = insertelement <2 x float> %52, float %40, i64 1
  %54 = call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %46, <2 x float> %51, <2 x float> %53)
  %55 = insertelement <2 x float> poison, float %34, i64 0
  %56 = insertelement <2 x float> %55, float %39, i64 1
  %57 = call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %46, <2 x float> %54, <2 x float> %56)
  %58 = insertelement <2 x float> poison, float %33, i64 0
  %59 = insertelement <2 x float> %58, float %38, i64 1
  %60 = call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %46, <2 x float> %57, <2 x float> %59)
  %61 = insertelement <2 x float> <float poison, float 1.000000e+00>, float %32, i64 0
  %62 = call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %46, <2 x float> %60, <2 x float> %61)
  %63 = extractelement <2 x float> %62, i64 0
  %64 = extractelement <2 x float> %62, i64 1
  %div31.i = fdiv float %63, %64
  %add.i122 = fadd float %div31.i, 3.750000e-01
  %div32.i = fdiv float %add.i122, %x
  %mul29 = fmul float %cc.0, %div32.i
  %65 = call float @llvm.fmuladd.f32(float %add.i, float %ss.0, float %mul29)
  %mul30 = fmul float %65, 0x3FE20DD760000000
  %call31 = call float @sqrtf(float noundef %x) #6
  %div32 = fdiv float %mul30, %call31
  br label %cleanup

if.end34:                                         ; preds = %if.end8
  %cmp35 = icmp slt i32 %and, 855638017
  br i1 %cmp35, label %if.then36, label %if.end38

if.then36:                                        ; preds = %if.end34
  %div37 = fdiv float 0xBFE45F3060000000, %x
  br label %cleanup

if.end38:                                         ; preds = %if.end34
  %mul39 = fmul float %x, %x
  %66 = tail call float @llvm.fmuladd.f32(float %mul39, float 0x3DB25039E0000000, float 0x3E3ABF1D60000000)
  %67 = insertelement <2 x float> poison, float %mul39, i64 0
  %68 = shufflevector <2 x float> %67, <2 x float> poison, <2 x i32> zeroinitializer
  %69 = insertelement <2 x float> <float 0xBE78AC0060000000, float poison>, float %66, i64 1
  %70 = tail call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %68, <2 x float> %69, <2 x float> <float 0x3EF8AB0380000000, float 0x3EB6C05A80000000>)
  %71 = tail call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %68, <2 x float> %70, <2 x float> <float 0xBF5F55E540000000, float 0x3F2A8C8960000000>)
  %72 = tail call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %68, <2 x float> %71, <2 x float> <float 0x3FA9D3C780000000, float 0x3F94650D40000000>)
  %73 = tail call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %68, <2 x float> %72, <2 x float> <float 0xBFC9186620000000, float 1.000000e+00>)
  %74 = extractelement <2 x float> %73, i64 0
  %75 = extractelement <2 x float> %73, i64 1
  %div49 = fdiv float %74, %75
  %call51 = tail call float @ref_j1f(float noundef %x)
  %call52 = tail call float @logf(float noundef %x) #6
  %neg = fdiv float -1.000000e+00, %x
  %76 = tail call float @llvm.fmuladd.f32(float %call51, float %call52, float %neg)
  %mul55 = fmul float %76, 0x3FE45F3060000000
  %77 = tail call float @llvm.fmuladd.f32(float %x, float %div49, float %mul55)
  br label %cleanup

cleanup:                                          ; preds = %if.then21, %ref_qonef.exit, %if.end38, %if.then36, %if.then6, %if.then2, %if.then
  %retval.0 = phi float [ %div, %if.then ], [ %div3, %if.then2 ], [ %div7, %if.then6 ], [ %div37, %if.then36 ], [ %77, %if.end38 ], [ %div24, %if.then21 ], [ %div32, %ref_qonef.exit ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %c) #6
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %s) #6
  ret float %retval.0
}

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare float @logf(float noundef) local_unnamed_addr #4

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.sqrt.f32(float) #5

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare <2 x float> @llvm.fmuladd.v2f32(<2 x float>, <2 x float>, <2 x float>) #5

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #3 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { mustprogress nofree nounwind willreturn memory(write) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #6 = { nounwind }

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
