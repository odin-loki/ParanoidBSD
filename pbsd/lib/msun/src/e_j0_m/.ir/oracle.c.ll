; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/e_j0_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/e_j0_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@vone = internal constant double 1.000000e+00, align 8
@vzero = internal constant double 0.000000e+00, align 8
@pR8 = internal unnamed_addr constant [6 x double] [double 0.000000e+00, double 0xBFB1FFFFFFFFFD32, double 0xC02029D0B44FA779, double 0xC07011027B19E863, double 0xC0A36A6ECD4DCAFC, double 0xC0B4850B36CC643D], align 16
@pS8 = internal unnamed_addr constant [5 x double] [double 0x405D223307A96751, double 0x40ADF37D50596938, double 0x40E3D2BB6EB6B05F, double 0x40FC810F8F9FA9BD, double 0x40E741774F2C49DC], align 16
@pR5 = internal unnamed_addr constant [6 x double] [double 0xBDA918B147E495CC, double 0xBFB1FFFFE69AFBC6, double 0xC010A370F90C6BBF, double 0xC050EB2F5A7D1783, double 0xC074B3B36742CC63, double 0xC075A6EF28A38BD7], align 16
@pS5 = internal unnamed_addr constant [5 x double] [double 0x404E60810C98C5DE, double 0x40906D025C7E2864, double 0x40B75AF88FBE1D60, double 0x40C2CCB8FA76FA38, double 0x40A2CC1DC70BE864], align 16
@pR3 = internal unnamed_addr constant [6 x double] [double 0xBE25E1036FE1AA86, double 0xBFB1FFF6F7C0E24B, double 0xC00345B2AEA48074, double 0xC035F74A4CB94E14, double 0xC04D0A22420A1A45, double 0xC03F72ACA892D80F], align 16
@pS3 = internal unnamed_addr constant [5 x double] [double 0x4041ED9284077DD3, double 0x40769839464A7C0E, double 0x4092A66E6D1061D6, double 0x40919FFCB8C39B7E, double 0x4065B296FC379081], align 16
@pR2 = internal unnamed_addr constant [6 x double] [double 0xBE77D316E927026D, double 0xBFB1FF62495E1E42, double 0xBFF736398A24A843, double 0xC01E8AF3EDAFA7F3, double 0xC02662E6C5246303, double 0xC009DE81AF8FE70F], align 16
@pS2 = internal unnamed_addr constant [5 x double] [double 0x40363865908B5959, double 0x4061069E0EE8878F, double 0x4070E78642EA079B, double 0x40633C033AB6FAFF, double 0x402D50B344391809], align 16
@qR8 = internal unnamed_addr constant [6 x double] [double 0.000000e+00, double 0x3FB2BFFFFFFFFE2C, double 0x402789525BB334D6, double 0x40816D6315301825, double 0x40C14D993E18F46D, double 0x40E212D40E901566], align 16
@qS8 = internal unnamed_addr constant [6 x double] [double 0x406478D5365B39BC, double 0x40BFA2584E6B0563, double 0x4101665254D38C3F, double 0x412883DA83A52B43, double 0x4129A66B28DE0B3D, double 0xC114FD6D2C9530C5], align 16
@qR5 = internal unnamed_addr constant [6 x double] [double 0x3DB43D8F29CC8CD9, double 0x3FB2BFFFD172B04C, double 0x401757B0B9953DD3, double 0x4060E3920A8788E9, double 0x40900CF99DC8C481, double 0x409F17E953C6E3A6], align 16
@qS5 = internal unnamed_addr constant [6 x double] [double 0x4054B1B3FB5E1543, double 0x40A03BA0DA21C0CE, double 0x40D267D27B591E6D, double 0x40EBB5E397E02372, double 0x40E191181F7A54A0, double 0xC0B4EA57BEDBC609], align 16
@qR3 = internal unnamed_addr constant [6 x double] [double 0x3E32CD036ADECB82, double 0x3FB2BFEE0E8D0842, double 0x400AC0FC61149CF5, double 0x40454F98962DAEDD, double 0x406559DBE25EFD1F, double 0x4064D77C81FA21E0], align 16
@qS3 = internal unnamed_addr constant [6 x double] [double 0x40486122BFE343A6, double 0x40862D8386544EB3, double 0x40ACF04BE44DFC63, double 0x40B93C6CD7C76A28, double 0x40A3A8AAD94FB1C0, double 0xC062A7EB201CF40F], align 16
@qR2 = internal unnamed_addr constant [6 x double] [double 0x3E84313B54F76BDB, double 0x3FB2BEC53E883E34, double 0x3FFFF897E727779C, double 0x402CFDBFAAF96FE5, double 0x403FAA8E29FBDC4A, double 0x403040B171814BB4], align 16
@qS2 = internal unnamed_addr constant [6 x double] [double 0x403E5D96F7C07AED, double 0x4070D591E4D14B40, double 0x408A664522B3BF22, double 0x408B977C9C5CC214, double 0x406A95530E001365, double 0xC0153E6AF8B32931], align 16

; Function Attrs: nounwind uwtable
define dso_local double @ref_j0(double noundef %x) local_unnamed_addr #0 {
entry:
  %s = alloca double, align 8
  %c = alloca double, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %s) #6
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %c) #6
  %0 = bitcast double %x to i64
  %gh_u.sroa.0.4.extract.shift = lshr i64 %0, 32
  %gh_u.sroa.0.4.extract.trunc = trunc i64 %gh_u.sroa.0.4.extract.shift to i32
  %and = and i32 %gh_u.sroa.0.4.extract.trunc, 2147483647
  %cmp = icmp ugt i32 %and, 2146435071
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %mul = fmul double %x, %x
  %div = fdiv double 1.000000e+00, %mul
  br label %cleanup

if.end:                                           ; preds = %entry
  %1 = tail call double @llvm.fabs.f64(double %x)
  %cmp1 = icmp ugt i32 %and, 1073741823
  br i1 %cmp1, label %if.then2, label %if.end27

if.then2:                                         ; preds = %if.end
  call void @sincos(double noundef %1, ptr noundef nonnull %s, ptr noundef nonnull %c) #6
  %2 = load double, ptr %s, align 8, !tbaa !5
  %3 = load double, ptr %c, align 8, !tbaa !5
  %add = fadd double %2, %3
  %cmp3 = icmp ult i32 %and, 2145386496
  br i1 %cmp3, label %if.then4, label %if.then14

if.then4:                                         ; preds = %if.then2
  %sub = fsub double %2, %3
  %add5 = fadd double %1, %1
  %call = call double @cos(double noundef %add5) #6
  %fneg = fneg double %call
  %4 = load double, ptr %s, align 8, !tbaa !5
  %5 = load double, ptr %c, align 8, !tbaa !5
  %mul6 = fmul double %4, %5
  %cmp7 = fcmp olt double %mul6, 0.000000e+00
  %div9 = fdiv double %fneg, %sub
  %div10 = fdiv double %fneg, %add
  %cc.0 = select i1 %cmp7, double %div9, double %add
  %ss.0 = select i1 %cmp7, double %sub, double %div10
  %cmp13 = icmp ugt i32 %and, 1207959552
  br i1 %cmp13, label %if.then14, label %if.else18

if.then14:                                        ; preds = %if.then2, %if.then4
  %cc.0135 = phi double [ %cc.0, %if.then4 ], [ %add, %if.then2 ]
  %mul15 = fmul double %cc.0135, 0x3FE20DD750429B6D
  %sqrt = call double @llvm.sqrt.f64(double %1)
  %div17 = fdiv double %mul15, %sqrt
  br label %cleanup

if.else18:                                        ; preds = %if.then4
  %6 = bitcast double %1 to i64
  %gh_u.sroa.0.4.extract.shift.i = lshr i64 %6, 32
  %gh_u.sroa.0.4.extract.trunc.i = trunc i64 %gh_u.sroa.0.4.extract.shift.i to i32
  %and.i = and i32 %gh_u.sroa.0.4.extract.trunc.i, 2147483647
  %cmp.i = icmp ugt i32 %and.i, 1075838975
  br i1 %cmp.i, label %ref_pzero.exit, label %if.else.i

if.else.i:                                        ; preds = %if.else18
  %cmp1.i = icmp ugt i32 %and.i, 1074933386
  br i1 %cmp1.i, label %ref_pzero.exit, label %if.else3.i

if.else3.i:                                       ; preds = %if.else.i
  %cmp4.i = icmp ugt i32 %and.i, 1074191212
  %pS3.pS2.i = select i1 %cmp4.i, ptr @pS3, ptr @pS2
  %pR3.pR2.i = select i1 %cmp4.i, ptr @pR3, ptr @pR2
  br label %ref_pzero.exit

ref_pzero.exit:                                   ; preds = %if.else18, %if.else.i, %if.else3.i
  %q.0.i = phi ptr [ @pS8, %if.else18 ], [ @pS5, %if.else.i ], [ %pS3.pS2.i, %if.else3.i ]
  %p.0.i = phi ptr [ @pR8, %if.else18 ], [ @pR5, %if.else.i ], [ %pR3.pR2.i, %if.else3.i ]
  %mul.i = fmul double %x, %x
  %div.i = fdiv double 1.000000e+00, %mul.i
  %7 = load double, ptr %p.0.i, align 8, !tbaa !5
  %arrayidx9.i = getelementptr inbounds double, ptr %p.0.i, i64 1
  %8 = load double, ptr %arrayidx9.i, align 8, !tbaa !5
  %arrayidx10.i = getelementptr inbounds double, ptr %p.0.i, i64 2
  %9 = load double, ptr %arrayidx10.i, align 8, !tbaa !5
  %arrayidx11.i = getelementptr inbounds double, ptr %p.0.i, i64 3
  %10 = load double, ptr %arrayidx11.i, align 8, !tbaa !5
  %arrayidx12.i = getelementptr inbounds double, ptr %p.0.i, i64 4
  %11 = load double, ptr %arrayidx12.i, align 8, !tbaa !5
  %arrayidx13.i = getelementptr inbounds double, ptr %p.0.i, i64 5
  %12 = load double, ptr %arrayidx13.i, align 8, !tbaa !5
  %13 = call double @llvm.fmuladd.f64(double %div.i, double %12, double %11)
  %14 = call double @llvm.fmuladd.f64(double %div.i, double %13, double %10)
  %15 = call double @llvm.fmuladd.f64(double %div.i, double %14, double %9)
  %16 = call double @llvm.fmuladd.f64(double %div.i, double %15, double %8)
  %17 = call double @llvm.fmuladd.f64(double %div.i, double %16, double %7)
  %18 = load double, ptr %q.0.i, align 8, !tbaa !5
  %arrayidx20.i = getelementptr inbounds double, ptr %q.0.i, i64 1
  %19 = load double, ptr %arrayidx20.i, align 8, !tbaa !5
  %arrayidx21.i = getelementptr inbounds double, ptr %q.0.i, i64 2
  %20 = load double, ptr %arrayidx21.i, align 8, !tbaa !5
  %arrayidx22.i = getelementptr inbounds double, ptr %q.0.i, i64 3
  %21 = load double, ptr %arrayidx22.i, align 8, !tbaa !5
  %arrayidx23.i = getelementptr inbounds double, ptr %q.0.i, i64 4
  %22 = load double, ptr %arrayidx23.i, align 8, !tbaa !5
  %23 = call double @llvm.fmuladd.f64(double %div.i, double %22, double %21)
  %24 = call double @llvm.fmuladd.f64(double %div.i, double %23, double %20)
  %25 = call double @llvm.fmuladd.f64(double %div.i, double %24, double %19)
  %26 = call double @llvm.fmuladd.f64(double %div.i, double %25, double %18)
  %27 = call double @llvm.fmuladd.f64(double %div.i, double %26, double 1.000000e+00)
  %div29.i = fdiv double %17, %27
  %add.i = fadd double %div29.i, 1.000000e+00
  br i1 %cmp.i, label %ref_qzero.exit, label %if.else.i115

if.else.i115:                                     ; preds = %ref_pzero.exit
  %cmp1.i116 = icmp ugt i32 %and.i, 1074933386
  br i1 %cmp1.i116, label %ref_qzero.exit, label %if.else3.i117

if.else3.i117:                                    ; preds = %if.else.i115
  %cmp4.i118 = icmp ugt i32 %and.i, 1074191212
  %qS3.qS2.i = select i1 %cmp4.i118, ptr @qS3, ptr @qS2
  %qR3.qR2.i = select i1 %cmp4.i118, ptr @qR3, ptr @qR2
  br label %ref_qzero.exit

ref_qzero.exit:                                   ; preds = %ref_pzero.exit, %if.else.i115, %if.else3.i117
  %q.0.i119 = phi ptr [ @qS8, %ref_pzero.exit ], [ @qS5, %if.else.i115 ], [ %qS3.qS2.i, %if.else3.i117 ]
  %p.0.i120 = phi ptr [ @qR8, %ref_pzero.exit ], [ @qR5, %if.else.i115 ], [ %qR3.qR2.i, %if.else3.i117 ]
  %28 = load double, ptr %p.0.i120, align 8, !tbaa !5
  %arrayidx9.i123 = getelementptr inbounds double, ptr %p.0.i120, i64 1
  %29 = load double, ptr %arrayidx9.i123, align 8, !tbaa !5
  %arrayidx10.i124 = getelementptr inbounds double, ptr %p.0.i120, i64 2
  %30 = load double, ptr %arrayidx10.i124, align 8, !tbaa !5
  %arrayidx11.i125 = getelementptr inbounds double, ptr %p.0.i120, i64 3
  %31 = load double, ptr %arrayidx11.i125, align 8, !tbaa !5
  %arrayidx12.i126 = getelementptr inbounds double, ptr %p.0.i120, i64 4
  %32 = load double, ptr %arrayidx12.i126, align 8, !tbaa !5
  %arrayidx13.i127 = getelementptr inbounds double, ptr %p.0.i120, i64 5
  %33 = load double, ptr %arrayidx13.i127, align 8, !tbaa !5
  %34 = load double, ptr %q.0.i119, align 8, !tbaa !5
  %arrayidx20.i128 = getelementptr inbounds double, ptr %q.0.i119, i64 1
  %35 = load double, ptr %arrayidx20.i128, align 8, !tbaa !5
  %arrayidx21.i129 = getelementptr inbounds double, ptr %q.0.i119, i64 2
  %36 = load double, ptr %arrayidx21.i129, align 8, !tbaa !5
  %arrayidx22.i130 = getelementptr inbounds double, ptr %q.0.i119, i64 3
  %37 = load double, ptr %arrayidx22.i130, align 8, !tbaa !5
  %arrayidx23.i131 = getelementptr inbounds double, ptr %q.0.i119, i64 4
  %38 = load double, ptr %arrayidx23.i131, align 8, !tbaa !5
  %arrayidx24.i = getelementptr inbounds double, ptr %q.0.i119, i64 5
  %39 = load double, ptr %arrayidx24.i, align 8, !tbaa !5
  %40 = call double @llvm.fmuladd.f64(double %div.i, double %39, double %38)
  %41 = insertelement <2 x double> poison, double %div.i, i64 0
  %42 = shufflevector <2 x double> %41, <2 x double> poison, <2 x i32> zeroinitializer
  %43 = insertelement <2 x double> poison, double %33, i64 0
  %44 = insertelement <2 x double> %43, double %40, i64 1
  %45 = insertelement <2 x double> poison, double %32, i64 0
  %46 = insertelement <2 x double> %45, double %37, i64 1
  %47 = call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %42, <2 x double> %44, <2 x double> %46)
  %48 = insertelement <2 x double> poison, double %31, i64 0
  %49 = insertelement <2 x double> %48, double %36, i64 1
  %50 = call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %42, <2 x double> %47, <2 x double> %49)
  %51 = insertelement <2 x double> poison, double %30, i64 0
  %52 = insertelement <2 x double> %51, double %35, i64 1
  %53 = call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %42, <2 x double> %50, <2 x double> %52)
  %54 = insertelement <2 x double> poison, double %29, i64 0
  %55 = insertelement <2 x double> %54, double %34, i64 1
  %56 = call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %42, <2 x double> %53, <2 x double> %55)
  %57 = insertelement <2 x double> <double poison, double 1.000000e+00>, double %28, i64 0
  %58 = call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %42, <2 x double> %56, <2 x double> %57)
  %59 = extractelement <2 x double> %58, i64 0
  %60 = extractelement <2 x double> %58, i64 1
  %div31.i = fdiv double %59, %60
  %sub.i = fadd double %div31.i, -1.250000e-01
  %61 = fneg double %sub.i
  %62 = fdiv double %61, %1
  %neg = fmul double %ss.0, %62
  %63 = call double @llvm.fmuladd.f64(double %add.i, double %cc.0, double %neg)
  %mul23 = fmul double %63, 0x3FE20DD750429B6D
  %sqrt136 = call double @llvm.sqrt.f64(double %1)
  %div25 = fdiv double %mul23, %sqrt136
  br label %cleanup

if.end27:                                         ; preds = %if.end
  %cmp28 = icmp ult i32 %and, 1059061760
  %add30 = fadd double %1, 1.000000e+300
  %cmp31 = fcmp ogt double %add30, 1.000000e+00
  %or.cond = and i1 %cmp31, %cmp28
  br i1 %or.cond, label %if.then32, label %if.end40

if.then32:                                        ; preds = %if.end27
  %cmp33 = icmp ult i32 %and, 1044381696
  br i1 %cmp33, label %cleanup, label %if.else35

if.else35:                                        ; preds = %if.then32
  %mul36 = fmul double %x, %x
  %div37 = fmul double %mul36, 2.500000e-01
  %sub38 = fsub double 1.000000e+00, %div37
  br label %cleanup

if.end40:                                         ; preds = %if.end27
  %mul41 = fmul double %x, %x
  %64 = tail call double @llvm.fmuladd.f64(double %mul41, double 0xBE33D5E773D63FCE, double 0x3EBEB1D10C503919)
  %65 = tail call double @llvm.fmuladd.f64(double %mul41, double %64, double 0xBF28E6A5B61AC6E9)
  %66 = tail call double @llvm.fmuladd.f64(double %mul41, double %65, double 0x3F8FFFFFFFFFFFFD)
  %mul45 = fmul double %mul41, %66
  %67 = tail call double @llvm.fmuladd.f64(double %mul41, double 0x3E1408BCF4745D8F, double 0x3EA13B54CE84D5A9)
  %68 = tail call double @llvm.fmuladd.f64(double %mul41, double %67, double 0x3F1EA6D2DD57DBF4)
  %69 = tail call double @llvm.fmuladd.f64(double %mul41, double %68, double 0x3F8FFCE882C8C2A4)
  %70 = tail call double @llvm.fmuladd.f64(double %mul41, double %69, double 1.000000e+00)
  %cmp50 = icmp ult i32 %and, 1072693248
  br i1 %cmp50, label %if.then51, label %if.else55

if.then51:                                        ; preds = %if.end40
  %div52 = fdiv double %mul45, %70
  %sub53 = fadd double %div52, -2.500000e-01
  %71 = tail call double @llvm.fmuladd.f64(double %mul41, double %sub53, double 1.000000e+00)
  br label %cleanup

if.else55:                                        ; preds = %if.end40
  %div56 = fmul double %1, 5.000000e-01
  %add57 = fadd double %div56, 1.000000e+00
  %sub58 = fsub double 1.000000e+00, %div56
  %div60 = fdiv double %mul45, %70
  %mul61 = fmul double %mul41, %div60
  %72 = tail call double @llvm.fmuladd.f64(double %add57, double %sub58, double %mul61)
  br label %cleanup

cleanup:                                          ; preds = %if.then32, %if.then14, %ref_qzero.exit, %if.else55, %if.then51, %if.else35, %if.then
  %retval.0 = phi double [ %div, %if.then ], [ %sub38, %if.else35 ], [ %71, %if.then51 ], [ %72, %if.else55 ], [ %div17, %if.then14 ], [ %div25, %ref_qzero.exit ], [ 1.000000e+00, %if.then32 ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %c) #6
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %s) #6
  ret double %retval.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fabs.f64(double) #2

declare void @sincos(double noundef, ptr noundef, ptr noundef) local_unnamed_addr #3

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare double @cos(double noundef) local_unnamed_addr #4

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare double @sqrt(double noundef) local_unnamed_addr #4

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fmuladd.f64(double, double, double) #2

; Function Attrs: nounwind uwtable
define dso_local double @ref_y0(double noundef %x) local_unnamed_addr #0 {
entry:
  %s = alloca double, align 8
  %c = alloca double, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %s) #6
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %c) #6
  %0 = bitcast double %x to i64
  %ew_u.sroa.0.4.extract.shift = lshr i64 %0, 32
  %ew_u.sroa.0.4.extract.trunc = trunc i64 %ew_u.sroa.0.4.extract.shift to i32
  %and = and i32 %ew_u.sroa.0.4.extract.trunc, 2147483647
  %cmp = icmp ugt i32 %and, 2146435071
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %1 = load volatile double, ptr @vone, align 8, !tbaa !5
  %2 = tail call double @llvm.fmuladd.f64(double %x, double %x, double %x)
  %div = fdiv double %1, %2
  br label %cleanup

if.end:                                           ; preds = %entry
  %ew_u.sroa.0.0.extract.trunc = trunc i64 %0 to i32
  %or = or i32 %and, %ew_u.sroa.0.0.extract.trunc
  %cmp1 = icmp eq i32 %or, 0
  br i1 %cmp1, label %if.then2, label %if.end4

if.then2:                                         ; preds = %if.end
  %3 = load volatile double, ptr @vzero, align 8, !tbaa !5
  %div3 = fdiv double -1.000000e+00, %3
  br label %cleanup

if.end4:                                          ; preds = %if.end
  %cmp5 = icmp slt i64 %0, 0
  br i1 %cmp5, label %if.then6, label %if.end8

if.then6:                                         ; preds = %if.end4
  %4 = load volatile double, ptr @vzero, align 8, !tbaa !5
  %5 = load volatile double, ptr @vzero, align 8, !tbaa !5
  %div7 = fdiv double %4, %5
  br label %cleanup

if.end8:                                          ; preds = %if.end4
  %cmp9 = icmp ugt i32 %and, 1073741823
  br i1 %cmp9, label %if.then10, label %if.end34

if.then10:                                        ; preds = %if.end8
  call void @sincos(double noundef %x, ptr noundef nonnull %s, ptr noundef nonnull %c) #6
  %6 = load double, ptr %s, align 8, !tbaa !5
  %7 = load double, ptr %c, align 8, !tbaa !5
  %sub = fsub double %6, %7
  %cmp11 = icmp ult i32 %and, 2145386496
  br i1 %cmp11, label %if.then12, label %if.then21

if.then12:                                        ; preds = %if.then10
  %add = fadd double %6, %7
  %add13 = fadd double %x, %x
  %call = call double @cos(double noundef %add13) #6
  %fneg = fneg double %call
  %8 = load double, ptr %s, align 8, !tbaa !5
  %9 = load double, ptr %c, align 8, !tbaa !5
  %mul = fmul double %8, %9
  %cmp14 = fcmp olt double %mul, 0.000000e+00
  %div16 = fdiv double %fneg, %sub
  %div17 = fdiv double %fneg, %add
  %cc.0 = select i1 %cmp14, double %div16, double %add
  %ss.0 = select i1 %cmp14, double %sub, double %div17
  %cmp20 = icmp ugt i32 %and, 1207959552
  br i1 %cmp20, label %if.then21, label %if.else25

if.then21:                                        ; preds = %if.then10, %if.then12
  %ss.0127 = phi double [ %ss.0, %if.then12 ], [ %sub, %if.then10 ]
  %mul22 = fmul double %ss.0127, 0x3FE20DD750429B6D
  %call23 = call double @sqrt(double noundef %x) #6
  %div24 = fdiv double %mul22, %call23
  br label %cleanup

if.else25:                                        ; preds = %if.then12
  %cmp.i = icmp ugt i32 %and, 1075838975
  br i1 %cmp.i, label %ref_pzero.exit, label %if.else.i

if.else.i:                                        ; preds = %if.else25
  %cmp1.i = icmp ugt i32 %and, 1074933386
  br i1 %cmp1.i, label %ref_pzero.exit, label %if.else3.i

if.else3.i:                                       ; preds = %if.else.i
  %cmp4.i = icmp ugt i32 %and, 1074191212
  %pS3.pS2.i = select i1 %cmp4.i, ptr @pS3, ptr @pS2
  %pR3.pR2.i = select i1 %cmp4.i, ptr @pR3, ptr @pR2
  br label %ref_pzero.exit

ref_pzero.exit:                                   ; preds = %if.else25, %if.else.i, %if.else3.i
  %q.0.i = phi ptr [ @pS8, %if.else25 ], [ @pS5, %if.else.i ], [ %pS3.pS2.i, %if.else3.i ]
  %p.0.i = phi ptr [ @pR8, %if.else25 ], [ @pR5, %if.else.i ], [ %pR3.pR2.i, %if.else3.i ]
  %mul.i = fmul double %x, %x
  %div.i = fdiv double 1.000000e+00, %mul.i
  %10 = load double, ptr %p.0.i, align 8, !tbaa !5
  %arrayidx9.i = getelementptr inbounds double, ptr %p.0.i, i64 1
  %11 = load double, ptr %arrayidx9.i, align 8, !tbaa !5
  %arrayidx10.i = getelementptr inbounds double, ptr %p.0.i, i64 2
  %12 = load double, ptr %arrayidx10.i, align 8, !tbaa !5
  %arrayidx11.i = getelementptr inbounds double, ptr %p.0.i, i64 3
  %13 = load double, ptr %arrayidx11.i, align 8, !tbaa !5
  %arrayidx12.i = getelementptr inbounds double, ptr %p.0.i, i64 4
  %14 = load double, ptr %arrayidx12.i, align 8, !tbaa !5
  %arrayidx13.i = getelementptr inbounds double, ptr %p.0.i, i64 5
  %15 = load double, ptr %arrayidx13.i, align 8, !tbaa !5
  %16 = call double @llvm.fmuladd.f64(double %div.i, double %15, double %14)
  %17 = call double @llvm.fmuladd.f64(double %div.i, double %16, double %13)
  %18 = call double @llvm.fmuladd.f64(double %div.i, double %17, double %12)
  %19 = call double @llvm.fmuladd.f64(double %div.i, double %18, double %11)
  %20 = call double @llvm.fmuladd.f64(double %div.i, double %19, double %10)
  %21 = load double, ptr %q.0.i, align 8, !tbaa !5
  %arrayidx20.i = getelementptr inbounds double, ptr %q.0.i, i64 1
  %22 = load double, ptr %arrayidx20.i, align 8, !tbaa !5
  %arrayidx21.i = getelementptr inbounds double, ptr %q.0.i, i64 2
  %23 = load double, ptr %arrayidx21.i, align 8, !tbaa !5
  %arrayidx22.i = getelementptr inbounds double, ptr %q.0.i, i64 3
  %24 = load double, ptr %arrayidx22.i, align 8, !tbaa !5
  %arrayidx23.i = getelementptr inbounds double, ptr %q.0.i, i64 4
  %25 = load double, ptr %arrayidx23.i, align 8, !tbaa !5
  %26 = call double @llvm.fmuladd.f64(double %div.i, double %25, double %24)
  %27 = call double @llvm.fmuladd.f64(double %div.i, double %26, double %23)
  %28 = call double @llvm.fmuladd.f64(double %div.i, double %27, double %22)
  %29 = call double @llvm.fmuladd.f64(double %div.i, double %28, double %21)
  %30 = call double @llvm.fmuladd.f64(double %div.i, double %29, double 1.000000e+00)
  %div29.i = fdiv double %20, %30
  %add.i = fadd double %div29.i, 1.000000e+00
  br i1 %cmp.i, label %ref_qzero.exit, label %if.else.i107

if.else.i107:                                     ; preds = %ref_pzero.exit
  %cmp1.i108 = icmp ugt i32 %and, 1074933386
  br i1 %cmp1.i108, label %ref_qzero.exit, label %if.else3.i109

if.else3.i109:                                    ; preds = %if.else.i107
  %cmp4.i110 = icmp ugt i32 %and, 1074191212
  %qS3.qS2.i = select i1 %cmp4.i110, ptr @qS3, ptr @qS2
  %qR3.qR2.i = select i1 %cmp4.i110, ptr @qR3, ptr @qR2
  br label %ref_qzero.exit

ref_qzero.exit:                                   ; preds = %ref_pzero.exit, %if.else.i107, %if.else3.i109
  %q.0.i111 = phi ptr [ @qS8, %ref_pzero.exit ], [ @qS5, %if.else.i107 ], [ %qS3.qS2.i, %if.else3.i109 ]
  %p.0.i112 = phi ptr [ @qR8, %ref_pzero.exit ], [ @qR5, %if.else.i107 ], [ %qR3.qR2.i, %if.else3.i109 ]
  %31 = load double, ptr %p.0.i112, align 8, !tbaa !5
  %arrayidx9.i115 = getelementptr inbounds double, ptr %p.0.i112, i64 1
  %32 = load double, ptr %arrayidx9.i115, align 8, !tbaa !5
  %arrayidx10.i116 = getelementptr inbounds double, ptr %p.0.i112, i64 2
  %33 = load double, ptr %arrayidx10.i116, align 8, !tbaa !5
  %arrayidx11.i117 = getelementptr inbounds double, ptr %p.0.i112, i64 3
  %34 = load double, ptr %arrayidx11.i117, align 8, !tbaa !5
  %arrayidx12.i118 = getelementptr inbounds double, ptr %p.0.i112, i64 4
  %35 = load double, ptr %arrayidx12.i118, align 8, !tbaa !5
  %arrayidx13.i119 = getelementptr inbounds double, ptr %p.0.i112, i64 5
  %36 = load double, ptr %arrayidx13.i119, align 8, !tbaa !5
  %37 = load double, ptr %q.0.i111, align 8, !tbaa !5
  %arrayidx20.i120 = getelementptr inbounds double, ptr %q.0.i111, i64 1
  %38 = load double, ptr %arrayidx20.i120, align 8, !tbaa !5
  %arrayidx21.i121 = getelementptr inbounds double, ptr %q.0.i111, i64 2
  %39 = load double, ptr %arrayidx21.i121, align 8, !tbaa !5
  %arrayidx22.i122 = getelementptr inbounds double, ptr %q.0.i111, i64 3
  %40 = load double, ptr %arrayidx22.i122, align 8, !tbaa !5
  %arrayidx23.i123 = getelementptr inbounds double, ptr %q.0.i111, i64 4
  %41 = load double, ptr %arrayidx23.i123, align 8, !tbaa !5
  %arrayidx24.i = getelementptr inbounds double, ptr %q.0.i111, i64 5
  %42 = load double, ptr %arrayidx24.i, align 8, !tbaa !5
  %43 = call double @llvm.fmuladd.f64(double %div.i, double %42, double %41)
  %44 = insertelement <2 x double> poison, double %div.i, i64 0
  %45 = shufflevector <2 x double> %44, <2 x double> poison, <2 x i32> zeroinitializer
  %46 = insertelement <2 x double> poison, double %36, i64 0
  %47 = insertelement <2 x double> %46, double %43, i64 1
  %48 = insertelement <2 x double> poison, double %35, i64 0
  %49 = insertelement <2 x double> %48, double %40, i64 1
  %50 = call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %45, <2 x double> %47, <2 x double> %49)
  %51 = insertelement <2 x double> poison, double %34, i64 0
  %52 = insertelement <2 x double> %51, double %39, i64 1
  %53 = call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %45, <2 x double> %50, <2 x double> %52)
  %54 = insertelement <2 x double> poison, double %33, i64 0
  %55 = insertelement <2 x double> %54, double %38, i64 1
  %56 = call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %45, <2 x double> %53, <2 x double> %55)
  %57 = insertelement <2 x double> poison, double %32, i64 0
  %58 = insertelement <2 x double> %57, double %37, i64 1
  %59 = call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %45, <2 x double> %56, <2 x double> %58)
  %60 = insertelement <2 x double> <double poison, double 1.000000e+00>, double %31, i64 0
  %61 = call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %45, <2 x double> %59, <2 x double> %60)
  %62 = extractelement <2 x double> %61, i64 0
  %63 = extractelement <2 x double> %61, i64 1
  %div31.i = fdiv double %62, %63
  %sub.i = fadd double %div31.i, -1.250000e-01
  %div32.i = fdiv double %sub.i, %x
  %mul29 = fmul double %cc.0, %div32.i
  %64 = call double @llvm.fmuladd.f64(double %add.i, double %ss.0, double %mul29)
  %mul30 = fmul double %64, 0x3FE20DD750429B6D
  %call31 = call double @sqrt(double noundef %x) #6
  %div32 = fdiv double %mul30, %call31
  br label %cleanup

if.end34:                                         ; preds = %if.end8
  %cmp35 = icmp ult i32 %and, 1044381697
  br i1 %cmp35, label %if.then36, label %if.end39

if.then36:                                        ; preds = %if.end34
  %call37 = tail call double @log(double noundef %x) #6
  %65 = tail call double @llvm.fmuladd.f64(double %call37, double 0x3FE45F306DC9C883, double 0xBFB2E4D699CBD01F)
  br label %cleanup

if.end39:                                         ; preds = %if.end34
  %mul40 = fmul double %x, %x
  %66 = tail call double @llvm.fmuladd.f64(double %mul40, double 0xBDC5E43D693FB3C8, double 0x3E5500573B4EABD4)
  %67 = tail call double @llvm.fmuladd.f64(double %mul40, double %66, double 0xBECFFEA773D25CAD)
  %68 = insertelement <2 x double> poison, double %mul40, i64 0
  %69 = shufflevector <2 x double> %68, <2 x double> poison, <2 x i32> zeroinitializer
  %70 = insertelement <2 x double> <double poison, double 0x3DFE50183BD6D9EF>, double %67, i64 0
  %71 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %69, <2 x double> %70, <2 x double> <double 0x3F36C54D20B29B6B, double 0x3E91642D7FF202FD>)
  %72 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %69, <2 x double> %71, <2 x double> <double 0xBF8C4CE8B16CFA97, double 0x3F13ECBBF578C6C1>)
  %73 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %69, <2 x double> %72, <2 x double> <double 0x3FC69D019DE9E3FC, double 0x3F8A127091C9C71A>)
  %74 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %69, <2 x double> %73, <2 x double> <double 0xBFB2E4D699CBD01F, double 1.000000e+00>)
  %75 = extractelement <2 x double> %74, i64 0
  %76 = extractelement <2 x double> %74, i64 1
  %div51 = fdiv double %75, %76
  %call52 = tail call double @ref_j0(double noundef %x)
  %call53 = tail call double @log(double noundef %x) #6
  %mul54 = fmul double %call52, %call53
  %77 = tail call double @llvm.fmuladd.f64(double %mul54, double 0x3FE45F306DC9C883, double %div51)
  br label %cleanup

cleanup:                                          ; preds = %if.then21, %ref_qzero.exit, %if.end39, %if.then36, %if.then6, %if.then2, %if.then
  %retval.0 = phi double [ %div, %if.then ], [ %div3, %if.then2 ], [ %div7, %if.then6 ], [ %65, %if.then36 ], [ %77, %if.end39 ], [ %div24, %if.then21 ], [ %div32, %ref_qzero.exit ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %c) #6
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %s) #6
  ret double %retval.0
}

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare double @log(double noundef) local_unnamed_addr #4

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.sqrt.f64(double) #5

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare <2 x double> @llvm.fmuladd.v2f64(<2 x double>, <2 x double>, <2 x double>) #5

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
!6 = !{!"double", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
