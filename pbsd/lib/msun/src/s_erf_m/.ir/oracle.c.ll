; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/s_erf_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/s_erf_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@tiny = internal constant double 1.000000e-300, align 8

; Function Attrs: mustprogress nofree nounwind willreturn memory(write, inaccessiblemem: readwrite) uwtable
define dso_local double @ref_erf(double noundef %x) local_unnamed_addr #0 {
entry:
  %0 = bitcast double %x to i64
  %gh_u.sroa.0.4.extract.shift = lshr i64 %0, 32
  %gh_u.sroa.0.4.extract.trunc = trunc i64 %gh_u.sroa.0.4.extract.shift to i32
  %and = and i32 %gh_u.sroa.0.4.extract.trunc, 2147483647
  %cmp = icmp ugt i32 %and, 2146435071
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %1 = lshr i32 %gh_u.sroa.0.4.extract.trunc, 30
  %shl = and i32 %1, 2
  %sub = sub nsw i32 1, %shl
  %conv = sitofp i32 %sub to double
  %div = fdiv double 1.000000e+00, %x
  %add = fadd double %div, %conv
  br label %cleanup

if.end:                                           ; preds = %entry
  %cmp1 = icmp ult i32 %and, 1072365568
  br i1 %cmp1, label %if.then3, label %if.end25

if.then3:                                         ; preds = %if.end
  %cmp4 = icmp ult i32 %and, 1043333120
  br i1 %cmp4, label %if.then6, label %if.end13

if.then6:                                         ; preds = %if.then3
  %cmp7 = icmp ult i32 %and, 8388608
  br i1 %cmp7, label %if.then9, label %if.end12

if.then9:                                         ; preds = %if.then6
  %mul10 = fmul double %x, 0x3FF06EBA8214DB69
  %2 = tail call double @llvm.fmuladd.f64(double %x, double 8.000000e+00, double %mul10)
  %div11 = fmul double %2, 1.250000e-01
  br label %cleanup

if.end12:                                         ; preds = %if.then6
  %3 = tail call double @llvm.fmuladd.f64(double %x, double 0x3FC06EBA8214DB69, double %x)
  br label %cleanup

if.end13:                                         ; preds = %if.then3
  %mul = fmul double %x, %x
  %4 = tail call double @llvm.fmuladd.f64(double %mul, double 0xBED09C4342A26120, double 0x3F215DC9221C1A10)
  %5 = insertelement <2 x double> poison, double %mul, i64 0
  %6 = shufflevector <2 x double> %5, <2 x double> poison, <2 x i32> zeroinitializer
  %7 = insertelement <2 x double> <double 0xBEF8EAD6120016AC, double poison>, double %4, i64 1
  %8 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %6, <2 x double> %7, <2 x double> <double 0xBF77A291236668E4, double 0x3F74D022C4D36B0F>)
  %9 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %6, <2 x double> %8, <2 x double> <double 0xBF9D2A51DBD7194F, double 0x3FB0A54C5536CEBA>)
  %10 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %6, <2 x double> %9, <2 x double> <double 0xBFD4CD7D691CB913, double 0x3FD97779CDDADC09>)
  %11 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %6, <2 x double> %10, <2 x double> <double 0x3FC06EBA8214DB68, double 1.000000e+00>)
  %12 = extractelement <2 x double> %11, i64 0
  %13 = extractelement <2 x double> %11, i64 1
  %div23 = fdiv double %12, %13
  %14 = tail call double @llvm.fmuladd.f64(double %x, double %div23, double %x)
  br label %cleanup

if.end25:                                         ; preds = %if.end
  %cmp26 = icmp ult i32 %and, 1072955392
  br i1 %cmp26, label %if.then28, label %if.end49

if.then28:                                        ; preds = %if.end25
  %15 = tail call double @llvm.fabs.f64(double %x)
  %sub29 = fadd double %15, -1.000000e+00
  %16 = tail call double @llvm.fmuladd.f64(double %sub29, double 0xBF61BF380A96073F, double 0x3FA22A36599795EB)
  %17 = tail call double @llvm.fmuladd.f64(double %sub29, double %16, double 0xBFBC63983D3E28EC)
  %18 = tail call double @llvm.fmuladd.f64(double %sub29, double %17, double 0x3FD45FCA805120E4)
  %19 = tail call double @llvm.fmuladd.f64(double %sub29, double %18, double 0xBFD7D240FBB8C3F1)
  %20 = tail call double @llvm.fmuladd.f64(double %sub29, double %19, double 0x3FDA8D00AD92B34D)
  %21 = tail call double @llvm.fmuladd.f64(double %sub29, double %20, double 0xBF6359B8BEF77538)
  %22 = tail call double @llvm.fmuladd.f64(double %sub29, double 0x3F888B545735151D, double 0x3F8BEDC26B51DD1C)
  %23 = tail call double @llvm.fmuladd.f64(double %sub29, double %22, double 0x3FC02660E763351F)
  %24 = tail call double @llvm.fmuladd.f64(double %sub29, double %23, double 0x3FB2635CD99FE9A7)
  %25 = tail call double @llvm.fmuladd.f64(double %sub29, double %24, double 0x3FE14AF092EB6F33)
  %26 = tail call double @llvm.fmuladd.f64(double %sub29, double %25, double 0x3FBB3E6618EEE323)
  %27 = tail call double @llvm.fmuladd.f64(double %sub29, double %26, double 1.000000e+00)
  %cmp42 = icmp sgt i64 %0, -1
  %div45 = fdiv double %21, %27
  br i1 %cmp42, label %if.then44, label %if.else

if.then44:                                        ; preds = %if.then28
  %add46 = fadd double %div45, 0x3FEB0AC160000000
  br label %cleanup

if.else:                                          ; preds = %if.then28
  %sub48 = fsub double 0xBFEB0AC160000000, %div45
  br label %cleanup

if.end49:                                         ; preds = %if.end25
  %cmp50 = icmp ugt i32 %and, 1075314687
  br i1 %cmp50, label %if.then52, label %if.end59

if.then52:                                        ; preds = %if.end49
  %cmp53 = icmp sgt i64 %0, -1
  %28 = load volatile double, ptr @tiny, align 8, !tbaa !5
  br i1 %cmp53, label %if.then55, label %if.else57

if.then55:                                        ; preds = %if.then52
  %sub56 = fsub double 1.000000e+00, %28
  br label %cleanup

if.else57:                                        ; preds = %if.then52
  %sub58 = fadd double %28, -1.000000e+00
  br label %cleanup

if.end59:                                         ; preds = %if.end49
  %29 = tail call double @llvm.fabs.f64(double %x)
  %mul60 = fmul double %x, %x
  %div61 = fdiv double 1.000000e+00, %mul60
  %cmp62 = icmp ult i32 %and, 1074191214
  br i1 %cmp62, label %if.then64, label %if.else80

if.then64:                                        ; preds = %if.end59
  %30 = insertelement <2 x double> poison, double %div61, i64 0
  %31 = shufflevector <2 x double> %30, <2 x double> poison, <2 x i32> zeroinitializer
  %32 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %31, <2 x double> <double 0xBFAEEFF2EE749A62, double 0xC023A0EFC69AC25C>, <2 x double> <double 0x401A47EF8E484A93, double 0xC054526557E4D2F2>)
  %33 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %31, <2 x double> %32, <2 x double> <double 0x405B28A3EE48AE2C, double 0xC067135CEBCCABB2>)
  %34 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %31, <2 x double> %33, <2 x double> <double 0x407AD02157700314, double 0xC0644CB184282266>)
  %35 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %31, <2 x double> %34, <2 x double> <double 0x40842B1921EC2868, double 0xC04F300AE4CBA38D>)
  %36 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %31, <2 x double> %35, <2 x double> <double 0x407B290DD58A1A71, double 0xC0251E0441B0E726>)
  %37 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %31, <2 x double> %36, <2 x double> <double 0x4061350C526AE721, double 0xBFE63416E4BA7360>)
  %38 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %31, <2 x double> %37, <2 x double> <double 0x4033A6B9BD707687, double 0xBF843412600D6435>)
  br label %if.end94

if.else80:                                        ; preds = %if.end59
  %39 = insertelement <2 x double> poison, double %div61, i64 0
  %40 = shufflevector <2 x double> %39, <2 x double> poison, <2 x i32> zeroinitializer
  %41 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %40, <2 x double> <double 0xC03670E242712D62, double 0xC07E384E9BDC383F>, <2 x double> <double 0x407DA874E79FE763, double 0xC09004616A2E5992>)
  %42 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %40, <2 x double> %41, <2 x double> <double 0x40A3F219CEDF3BE6, double 0xC083EC881375F228>)
  %43 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %40, <2 x double> %42, <2 x double> <double 0x40A8FFB7688C246A, double 0xC064145D43C5ED98>)
  %44 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %40, <2 x double> %43, <2 x double> <double 0x409802EB189D5118, double 0xC031C209555F995A>)
  %45 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %40, <2 x double> %44, <2 x double> <double 0x40745CAE221B9F0A, double 0xBFE993BA70C285DE>)
  %46 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %40, <2 x double> %45, <2 x double> <double 0x403E568B261D5190, double 0xBF84341239E86F4A>)
  br label %if.end94

if.end94:                                         ; preds = %if.else80, %if.then64
  %47 = phi <2 x double> [ %46, %if.else80 ], [ %38, %if.then64 ]
  %48 = extractelement <2 x double> %47, i64 0
  %49 = tail call double @llvm.fmuladd.f64(double %div61, double %48, double 1.000000e+00)
  %50 = bitcast double %29 to i64
  %sl_u.sroa.0.0.insert.mask = and i64 %50, -4294967296
  %51 = bitcast i64 %sl_u.sroa.0.0.insert.mask to double
  %fneg = fneg double %51
  %52 = tail call double @llvm.fmuladd.f64(double %fneg, double %51, double -5.625000e-01)
  %call = tail call double @exp(double noundef %52) #4
  %sub99 = fsub double %51, %29
  %add100 = fadd double %29, %51
  %53 = extractelement <2 x double> %47, i64 1
  %div102 = fdiv double %53, %49
  %54 = tail call double @llvm.fmuladd.f64(double %sub99, double %add100, double %div102)
  %call103 = tail call double @exp(double noundef %54) #4
  %mul104 = fmul double %call, %call103
  %cmp105 = icmp sgt i64 %0, -1
  %div108 = fdiv double %mul104, %29
  br i1 %cmp105, label %if.then107, label %if.else110

if.then107:                                       ; preds = %if.end94
  %sub109 = fsub double 1.000000e+00, %div108
  br label %cleanup

if.else110:                                       ; preds = %if.end94
  %sub112 = fadd double %div108, -1.000000e+00
  br label %cleanup

cleanup:                                          ; preds = %if.else110, %if.then107, %if.else57, %if.then55, %if.else, %if.then44, %if.end13, %if.end12, %if.then9, %if.then
  %retval.0 = phi double [ %add, %if.then ], [ %div11, %if.then9 ], [ %3, %if.end12 ], [ %14, %if.end13 ], [ %add46, %if.then44 ], [ %sub48, %if.else ], [ %sub56, %if.then55 ], [ %sub58, %if.else57 ], [ %sub109, %if.then107 ], [ %sub112, %if.else110 ]
  ret double %retval.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fmuladd.f64(double, double, double) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fabs.f64(double) #1

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare double @exp(double noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nofree nounwind willreturn memory(write, inaccessiblemem: readwrite) uwtable
define dso_local double @ref_erfc(double noundef %x) local_unnamed_addr #0 {
entry:
  %0 = bitcast double %x to i64
  %gh_u.sroa.0.4.extract.shift = lshr i64 %0, 32
  %gh_u.sroa.0.4.extract.trunc = trunc i64 %gh_u.sroa.0.4.extract.shift to i32
  %and = and i32 %gh_u.sroa.0.4.extract.trunc, 2147483647
  %cmp = icmp ugt i32 %and, 2146435071
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %1 = lshr i32 %gh_u.sroa.0.4.extract.trunc, 30
  %shl = and i32 %1, 2
  %conv = uitofp i32 %shl to double
  %div = fdiv double 1.000000e+00, %x
  %add = fadd double %div, %conv
  br label %cleanup

if.end:                                           ; preds = %entry
  %cmp1 = icmp ult i32 %and, 1072365568
  br i1 %cmp1, label %if.then3, label %if.end27

if.then3:                                         ; preds = %if.end
  %cmp4 = icmp ult i32 %and, 1013972992
  br i1 %cmp4, label %if.then6, label %if.end7

if.then6:                                         ; preds = %if.then3
  %sub = fsub double 1.000000e+00, %x
  br label %cleanup

if.end7:                                          ; preds = %if.then3
  %mul = fmul double %x, %x
  %2 = tail call double @llvm.fmuladd.f64(double %mul, double 0xBEF8EAD6120016AC, double 0xBF77A291236668E4)
  %3 = tail call double @llvm.fmuladd.f64(double %mul, double %2, double 0xBF9D2A51DBD7194F)
  %4 = tail call double @llvm.fmuladd.f64(double %mul, double %3, double 0xBFD4CD7D691CB913)
  %5 = tail call double @llvm.fmuladd.f64(double %mul, double %4, double 0x3FC06EBA8214DB68)
  %6 = tail call double @llvm.fmuladd.f64(double %mul, double 0xBED09C4342A26120, double 0x3F215DC9221C1A10)
  %7 = tail call double @llvm.fmuladd.f64(double %mul, double %6, double 0x3F74D022C4D36B0F)
  %8 = tail call double @llvm.fmuladd.f64(double %mul, double %7, double 0x3FB0A54C5536CEBA)
  %9 = tail call double @llvm.fmuladd.f64(double %mul, double %8, double 0x3FD97779CDDADC09)
  %10 = tail call double @llvm.fmuladd.f64(double %mul, double %9, double 1.000000e+00)
  %div17 = fdiv double %5, %10
  %cmp18 = icmp slt i32 %gh_u.sroa.0.4.extract.trunc, 1070596096
  br i1 %cmp18, label %if.then20, label %if.else

if.then20:                                        ; preds = %if.end7
  %11 = tail call double @llvm.fmuladd.f64(double %x, double %div17, double %x)
  %sub22 = fsub double 1.000000e+00, %11
  br label %cleanup

if.else:                                          ; preds = %if.end7
  %mul23 = fmul double %div17, %x
  %sub24 = fadd double %x, -5.000000e-01
  %add25 = fadd double %sub24, %mul23
  %sub26 = fsub double 5.000000e-01, %add25
  br label %cleanup

if.end27:                                         ; preds = %if.end
  %cmp28 = icmp ult i32 %and, 1072955392
  br i1 %cmp28, label %if.then30, label %if.end53

if.then30:                                        ; preds = %if.end27
  %12 = tail call double @llvm.fabs.f64(double %x)
  %sub31 = fadd double %12, -1.000000e+00
  %13 = tail call double @llvm.fmuladd.f64(double %sub31, double 0xBF61BF380A96073F, double 0x3FA22A36599795EB)
  %14 = tail call double @llvm.fmuladd.f64(double %sub31, double %13, double 0xBFBC63983D3E28EC)
  %15 = tail call double @llvm.fmuladd.f64(double %sub31, double %14, double 0x3FD45FCA805120E4)
  %16 = tail call double @llvm.fmuladd.f64(double %sub31, double %15, double 0xBFD7D240FBB8C3F1)
  %17 = tail call double @llvm.fmuladd.f64(double %sub31, double %16, double 0x3FDA8D00AD92B34D)
  %18 = tail call double @llvm.fmuladd.f64(double %sub31, double %17, double 0xBF6359B8BEF77538)
  %19 = tail call double @llvm.fmuladd.f64(double %sub31, double 0x3F888B545735151D, double 0x3F8BEDC26B51DD1C)
  %20 = tail call double @llvm.fmuladd.f64(double %sub31, double %19, double 0x3FC02660E763351F)
  %21 = tail call double @llvm.fmuladd.f64(double %sub31, double %20, double 0x3FB2635CD99FE9A7)
  %22 = tail call double @llvm.fmuladd.f64(double %sub31, double %21, double 0x3FE14AF092EB6F33)
  %23 = tail call double @llvm.fmuladd.f64(double %sub31, double %22, double 0x3FBB3E6618EEE323)
  %24 = tail call double @llvm.fmuladd.f64(double %sub31, double %23, double 1.000000e+00)
  %cmp44 = icmp sgt i64 %0, -1
  %div47 = fdiv double %18, %24
  br i1 %cmp44, label %if.then46, label %if.else49

if.then46:                                        ; preds = %if.then30
  %sub48 = fsub double 0x3FC3D4FA80000000, %div47
  br label %cleanup

if.else49:                                        ; preds = %if.then30
  %add51 = fadd double %div47, 0x3FEB0AC160000000
  %add52 = fadd double %add51, 1.000000e+00
  br label %cleanup

if.end53:                                         ; preds = %if.end27
  %cmp54 = icmp ult i32 %and, 1077673984
  br i1 %cmp54, label %if.then56, label %if.else116

if.then56:                                        ; preds = %if.end53
  %25 = tail call double @llvm.fabs.f64(double %x)
  %mul57 = fmul double %x, %x
  %div58 = fdiv double 1.000000e+00, %mul57
  %cmp59 = icmp ult i32 %and, 1074191213
  br i1 %cmp59, label %if.then61, label %if.else77

if.then61:                                        ; preds = %if.then56
  %26 = insertelement <2 x double> poison, double %div58, i64 0
  %27 = shufflevector <2 x double> %26, <2 x double> poison, <2 x i32> zeroinitializer
  %28 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %27, <2 x double> <double 0xBFAEEFF2EE749A62, double 0xC023A0EFC69AC25C>, <2 x double> <double 0x401A47EF8E484A93, double 0xC054526557E4D2F2>)
  %29 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %27, <2 x double> %28, <2 x double> <double 0x405B28A3EE48AE2C, double 0xC067135CEBCCABB2>)
  %30 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %27, <2 x double> %29, <2 x double> <double 0x407AD02157700314, double 0xC0644CB184282266>)
  %31 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %27, <2 x double> %30, <2 x double> <double 0x40842B1921EC2868, double 0xC04F300AE4CBA38D>)
  %32 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %27, <2 x double> %31, <2 x double> <double 0x407B290DD58A1A71, double 0xC0251E0441B0E726>)
  %33 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %27, <2 x double> %32, <2 x double> <double 0x4061350C526AE721, double 0xBFE63416E4BA7360>)
  %34 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %27, <2 x double> %33, <2 x double> <double 0x4033A6B9BD707687, double 0xBF843412600D6435>)
  br label %if.end98

if.else77:                                        ; preds = %if.then56
  %cmp78 = icmp slt i64 %0, 0
  %cmp80 = icmp ugt i32 %and, 1075314687
  %or.cond = and i1 %cmp78, %cmp80
  br i1 %or.cond, label %if.then82, label %if.end84

if.then82:                                        ; preds = %if.else77
  %35 = load volatile double, ptr @tiny, align 8, !tbaa !5
  %sub83 = fsub double 2.000000e+00, %35
  br label %cleanup

if.end84:                                         ; preds = %if.else77
  %36 = insertelement <2 x double> poison, double %div58, i64 0
  %37 = shufflevector <2 x double> %36, <2 x double> poison, <2 x i32> zeroinitializer
  %38 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %37, <2 x double> <double 0xC03670E242712D62, double 0xC07E384E9BDC383F>, <2 x double> <double 0x407DA874E79FE763, double 0xC09004616A2E5992>)
  %39 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %37, <2 x double> %38, <2 x double> <double 0x40A3F219CEDF3BE6, double 0xC083EC881375F228>)
  %40 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %37, <2 x double> %39, <2 x double> <double 0x40A8FFB7688C246A, double 0xC064145D43C5ED98>)
  %41 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %37, <2 x double> %40, <2 x double> <double 0x409802EB189D5118, double 0xC031C209555F995A>)
  %42 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %37, <2 x double> %41, <2 x double> <double 0x40745CAE221B9F0A, double 0xBFE993BA70C285DE>)
  %43 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %37, <2 x double> %42, <2 x double> <double 0x403E568B261D5190, double 0xBF84341239E86F4A>)
  br label %if.end98

if.end98:                                         ; preds = %if.end84, %if.then61
  %44 = phi <2 x double> [ %43, %if.end84 ], [ %34, %if.then61 ]
  %45 = extractelement <2 x double> %44, i64 0
  %46 = tail call double @llvm.fmuladd.f64(double %div58, double %45, double 1.000000e+00)
  %47 = bitcast double %25 to i64
  %sl_u.sroa.0.0.insert.mask = and i64 %47, -4294967296
  %48 = bitcast i64 %sl_u.sroa.0.0.insert.mask to double
  %fneg = fneg double %48
  %49 = tail call double @llvm.fmuladd.f64(double %fneg, double %48, double -5.625000e-01)
  %call = tail call double @exp(double noundef %49) #4
  %sub103 = fsub double %48, %25
  %add104 = fadd double %25, %48
  %50 = extractelement <2 x double> %44, i64 1
  %div106 = fdiv double %50, %46
  %51 = tail call double @llvm.fmuladd.f64(double %sub103, double %add104, double %div106)
  %call107 = tail call double @exp(double noundef %51) #4
  %mul108 = fmul double %call, %call107
  %cmp109 = icmp sgt i32 %gh_u.sroa.0.4.extract.trunc, 0
  %div112 = fdiv double %mul108, %25
  br i1 %cmp109, label %cleanup, label %if.else113

if.else113:                                       ; preds = %if.end98
  %sub115 = fsub double 2.000000e+00, %div112
  br label %cleanup

if.else116:                                       ; preds = %if.end53
  %cmp117 = icmp sgt i32 %gh_u.sroa.0.4.extract.trunc, 0
  %52 = load volatile double, ptr @tiny, align 8, !tbaa !5
  br i1 %cmp117, label %if.then119, label %if.else121

if.then119:                                       ; preds = %if.else116
  %53 = load volatile double, ptr @tiny, align 8, !tbaa !5
  %mul120 = fmul double %52, %53
  br label %cleanup

if.else121:                                       ; preds = %if.else116
  %sub122 = fsub double 2.000000e+00, %52
  br label %cleanup

cleanup:                                          ; preds = %if.end98, %if.else121, %if.then119, %if.else113, %if.then82, %if.else49, %if.then46, %if.else, %if.then20, %if.then6, %if.then
  %retval.0 = phi double [ %add, %if.then ], [ %sub, %if.then6 ], [ %sub22, %if.then20 ], [ %sub26, %if.else ], [ %sub48, %if.then46 ], [ %add52, %if.else49 ], [ %sub115, %if.else113 ], [ %sub83, %if.then82 ], [ %mul120, %if.then119 ], [ %sub122, %if.else121 ], [ %div112, %if.end98 ]
  ret double %retval.0
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare <2 x double> @llvm.fmuladd.v2f64(<2 x double>, <2 x double>, <2 x double>) #3

attributes #0 = { mustprogress nofree nounwind willreturn memory(write, inaccessiblemem: readwrite) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
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
!5 = !{!6, !6, i64 0}
!6 = !{!"double", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
