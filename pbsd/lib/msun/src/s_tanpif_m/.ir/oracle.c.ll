; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/src/s_tanpif_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/src/s_tanpif_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@vzero = internal constant float 0.000000e+00, align 4

; Function Attrs: mustprogress nofree norecurse nounwind willreturn memory(inaccessiblemem: readwrite) uwtable
define dso_local float @ref_tanpif(float noundef %x) local_unnamed_addr #0 {
entry:
  %0 = bitcast float %x to i32
  %1 = tail call float @llvm.fabs.f32(float %x)
  %and = bitcast float %1 to i32
  %cmp = icmp ult i32 %and, 1065353216
  br i1 %cmp, label %if.then, label %if.end29

if.then:                                          ; preds = %entry
  %cmp4 = icmp ult i32 %and, 1056964608
  br i1 %cmp4, label %if.then5, label %if.else

if.then5:                                         ; preds = %if.then
  %cmp6 = icmp ult i32 %and, 947912704
  br i1 %cmp6, label %if.then7, label %if.end20

if.then7:                                         ; preds = %if.then5
  %cmp8 = icmp eq i32 %and, 0
  br i1 %cmp8, label %cleanup, label %do.body10

do.body10:                                        ; preds = %if.then7
  %and12 = and i32 %0, -65536
  %2 = bitcast i32 %and12 to float
  %mul = fmul float %2, 0x4160000000000000
  %neg = fneg float %mul
  %3 = tail call float @llvm.fmuladd.f32(float %x, float 0x4160000000000000, float %neg)
  %mul17 = fmul float %mul, 0xBEE2AEEF40000000
  %4 = tail call float @llvm.fmuladd.f32(float %3, float 0x400921FB60000000, float %mul17)
  %5 = tail call float @llvm.fmuladd.f32(float %mul, float 0x4009220000000000, float %4)
  %mul19 = fmul float %5, 0x3E80000000000000
  br label %cleanup

if.end20:                                         ; preds = %if.then5
  %cmp.i = fcmp olt float %1, 2.500000e-01
  br i1 %cmp.i, label %if.then.i, label %if.else.i

if.then.i:                                        ; preds = %if.end20
  %conv.i = fpext float %1 to double
  %mul.i = fmul double %conv.i, 0x400921FB54442D18
  %mul.i.i = fmul double %mul.i, %mul.i
  %6 = tail call double @llvm.fmuladd.f64(double %mul.i.i, double 0x3F8362B9BF971BCD, double 0x3F685DADFCECF44E)
  %7 = tail call double @llvm.fmuladd.f64(double %mul.i.i, double 0x3F991DF3908C33CE, double 0x3FAB54C91D865AFE)
  %mul3.i.i = fmul double %mul.i.i, %mul.i.i
  %mul4.i.i = fmul double %mul.i, %mul.i.i
  %8 = tail call double @llvm.fmuladd.f64(double %mul.i.i, double 0x3FC112FD38999F72, double 0x3FD5554D3418C99F)
  %9 = tail call double @llvm.fmuladd.f64(double %mul4.i.i, double %8, double %mul.i)
  %mul7.i.i = fmul double %mul4.i.i, %mul3.i.i
  %10 = tail call double @llvm.fmuladd.f64(double %mul3.i.i, double %6, double %7)
  %11 = tail call double @llvm.fmuladd.f64(double %mul7.i.i, double %10, double %9)
  %retval.0.i.i = fptrunc double %11 to float
  br label %if.end26

if.else.i:                                        ; preds = %if.end20
  %cmp1.i = fcmp ogt float %1, 2.500000e-01
  br i1 %cmp1.i, label %if.then3.i, label %if.end26

if.then3.i:                                       ; preds = %if.else.i
  %conv4.i = fpext float %1 to double
  %sub.i = fsub double 5.000000e-01, %conv4.i
  %mul5.i = fmul double %sub.i, 0x400921FB54442D18
  %mul.i12.i = fmul double %mul5.i, %mul5.i
  %12 = tail call double @llvm.fmuladd.f64(double %mul.i12.i, double 0x3F8362B9BF971BCD, double 0x3F685DADFCECF44E)
  %13 = tail call double @llvm.fmuladd.f64(double %mul.i12.i, double 0x3F991DF3908C33CE, double 0x3FAB54C91D865AFE)
  %mul3.i13.i = fmul double %mul.i12.i, %mul.i12.i
  %mul4.i14.i = fmul double %mul5.i, %mul.i12.i
  %14 = tail call double @llvm.fmuladd.f64(double %mul.i12.i, double 0x3FC112FD38999F72, double 0x3FD5554D3418C99F)
  %15 = tail call double @llvm.fmuladd.f64(double %mul4.i14.i, double %14, double %mul5.i)
  %mul7.i15.i = fmul double %mul4.i14.i, %mul3.i13.i
  %16 = tail call double @llvm.fmuladd.f64(double %mul3.i13.i, double %12, double %13)
  %17 = tail call double @llvm.fmuladd.f64(double %mul7.i15.i, double %16, double %15)
  %div.i16.i = fdiv double -1.000000e+00, %17
  %retval.0.i17.i = fptrunc double %div.i16.i to float
  %fneg.i = fneg float %retval.0.i17.i
  br label %if.end26

if.else:                                          ; preds = %if.then
  %cmp21 = icmp eq i32 %and, 1056964608
  br i1 %cmp21, label %if.then22, label %if.else23

if.then22:                                        ; preds = %if.else
  %18 = load volatile float, ptr @vzero, align 4, !tbaa !5
  %div = fdiv float 1.000000e+00, %18
  br label %if.end26

if.else23:                                        ; preds = %if.else
  %sub = fsub float 1.000000e+00, %1
  %cmp.i151 = fcmp olt float %sub, 2.500000e-01
  br i1 %cmp.i151, label %if.then.i166, label %if.else.i152

if.then.i166:                                     ; preds = %if.else23
  %conv.i167 = fpext float %sub to double
  %mul.i168 = fmul double %conv.i167, 0x400921FB54442D18
  %mul.i.i169 = fmul double %mul.i168, %mul.i168
  %19 = tail call double @llvm.fmuladd.f64(double %mul.i.i169, double 0x3F8362B9BF971BCD, double 0x3F685DADFCECF44E)
  %20 = tail call double @llvm.fmuladd.f64(double %mul.i.i169, double 0x3F991DF3908C33CE, double 0x3FAB54C91D865AFE)
  %mul3.i.i170 = fmul double %mul.i.i169, %mul.i.i169
  %mul4.i.i171 = fmul double %mul.i168, %mul.i.i169
  %21 = tail call double @llvm.fmuladd.f64(double %mul.i.i169, double 0x3FC112FD38999F72, double 0x3FD5554D3418C99F)
  %22 = tail call double @llvm.fmuladd.f64(double %mul4.i.i171, double %21, double %mul.i168)
  %mul7.i.i172 = fmul double %mul4.i.i171, %mul3.i.i170
  %23 = tail call double @llvm.fmuladd.f64(double %mul3.i.i170, double %19, double %20)
  %24 = tail call double @llvm.fmuladd.f64(double %mul7.i.i172, double %23, double %22)
  %retval.0.i.i173 = fptrunc double %24 to float
  br label %ref___kernel_tanpif.exit174

if.else.i152:                                     ; preds = %if.else23
  %cmp1.i153 = fcmp ogt float %sub, 2.500000e-01
  br i1 %cmp1.i153, label %if.then3.i155, label %ref___kernel_tanpif.exit174

if.then3.i155:                                    ; preds = %if.else.i152
  %conv4.i156 = fpext float %sub to double
  %sub.i157 = fsub double 5.000000e-01, %conv4.i156
  %mul5.i158 = fmul double %sub.i157, 0x400921FB54442D18
  %mul.i12.i159 = fmul double %mul5.i158, %mul5.i158
  %25 = tail call double @llvm.fmuladd.f64(double %mul.i12.i159, double 0x3F8362B9BF971BCD, double 0x3F685DADFCECF44E)
  %26 = tail call double @llvm.fmuladd.f64(double %mul.i12.i159, double 0x3F991DF3908C33CE, double 0x3FAB54C91D865AFE)
  %mul3.i13.i160 = fmul double %mul.i12.i159, %mul.i12.i159
  %mul4.i14.i161 = fmul double %mul5.i158, %mul.i12.i159
  %27 = tail call double @llvm.fmuladd.f64(double %mul.i12.i159, double 0x3FC112FD38999F72, double 0x3FD5554D3418C99F)
  %28 = tail call double @llvm.fmuladd.f64(double %mul4.i14.i161, double %27, double %mul5.i158)
  %mul7.i15.i162 = fmul double %mul4.i14.i161, %mul3.i13.i160
  %29 = tail call double @llvm.fmuladd.f64(double %mul3.i13.i160, double %25, double %26)
  %30 = tail call double @llvm.fmuladd.f64(double %mul7.i15.i162, double %29, double %28)
  %div.i16.i163 = fdiv double -1.000000e+00, %30
  %retval.0.i17.i164 = fptrunc double %div.i16.i163 to float
  %fneg.i165 = fneg float %retval.0.i17.i164
  br label %ref___kernel_tanpif.exit174

ref___kernel_tanpif.exit174:                      ; preds = %if.then.i166, %if.else.i152, %if.then3.i155
  %t.0.i154 = phi float [ %retval.0.i.i173, %if.then.i166 ], [ %fneg.i165, %if.then3.i155 ], [ 1.000000e+00, %if.else.i152 ]
  %fneg = fneg float %t.0.i154
  br label %if.end26

if.end26:                                         ; preds = %if.then3.i, %if.else.i, %if.then.i, %if.then22, %ref___kernel_tanpif.exit174
  %t.0 = phi float [ %div, %if.then22 ], [ %fneg, %ref___kernel_tanpif.exit174 ], [ %retval.0.i.i, %if.then.i ], [ %fneg.i, %if.then3.i ], [ 1.000000e+00, %if.else.i ]
  %fneg28 = fneg float %t.0
  %tobool.not150 = icmp slt i32 %0, 0
  %cond = select i1 %tobool.not150, float %fneg28, float %t.0
  br label %cleanup

if.end29:                                         ; preds = %entry
  %cmp30 = icmp ult i32 %and, 1258291200
  br i1 %cmp30, label %do.body32, label %if.end80

do.body32:                                        ; preds = %if.end29
  %shr = lshr i32 %and, 23
  %sub34 = add nsw i32 %shr, -127
  %not = ashr i32 -8388608, %sub34
  %and36 = and i32 %not, %and
  %31 = bitcast i32 %and36 to float
  %conv = fptoui float %31 to i32
  %and43 = and i32 %conv, 1
  %tobool44.not = icmp eq i32 %and43, 0
  %cond45 = select i1 %tobool44.not, i32 1, i32 -1
  %conv46 = sitofp i32 %cond45 to float
  %sub47 = fsub float %1, %31
  %32 = bitcast float %sub47 to i32
  %cmp52 = icmp ult i32 %32, 1056964608
  br i1 %cmp52, label %if.then54, label %if.else62

if.then54:                                        ; preds = %do.body32
  %cmp55 = icmp eq i32 %32, 0
  br i1 %cmp55, label %cond.true57, label %cond.false58

cond.true57:                                      ; preds = %if.then54
  %33 = tail call float @llvm.copysign.f32(float 0.000000e+00, float %conv46)
  br label %if.end72

cond.false58:                                     ; preds = %if.then54
  %cmp.i175 = fcmp olt float %sub47, 2.500000e-01
  br i1 %cmp.i175, label %if.then.i190, label %if.else.i176

if.then.i190:                                     ; preds = %cond.false58
  %conv.i191 = fpext float %sub47 to double
  %mul.i192 = fmul double %conv.i191, 0x400921FB54442D18
  %mul.i.i193 = fmul double %mul.i192, %mul.i192
  %34 = tail call double @llvm.fmuladd.f64(double %mul.i.i193, double 0x3F8362B9BF971BCD, double 0x3F685DADFCECF44E)
  %35 = tail call double @llvm.fmuladd.f64(double %mul.i.i193, double 0x3F991DF3908C33CE, double 0x3FAB54C91D865AFE)
  %mul3.i.i194 = fmul double %mul.i.i193, %mul.i.i193
  %mul4.i.i195 = fmul double %mul.i192, %mul.i.i193
  %36 = tail call double @llvm.fmuladd.f64(double %mul.i.i193, double 0x3FC112FD38999F72, double 0x3FD5554D3418C99F)
  %37 = tail call double @llvm.fmuladd.f64(double %mul4.i.i195, double %36, double %mul.i192)
  %mul7.i.i196 = fmul double %mul4.i.i195, %mul3.i.i194
  %38 = tail call double @llvm.fmuladd.f64(double %mul3.i.i194, double %34, double %35)
  %39 = tail call double @llvm.fmuladd.f64(double %mul7.i.i196, double %38, double %37)
  %retval.0.i.i197 = fptrunc double %39 to float
  br label %if.end72

if.else.i176:                                     ; preds = %cond.false58
  %cmp1.i177 = fcmp ogt float %sub47, 2.500000e-01
  br i1 %cmp1.i177, label %if.then3.i179, label %if.end72

if.then3.i179:                                    ; preds = %if.else.i176
  %conv4.i180 = fpext float %sub47 to double
  %sub.i181 = fsub double 5.000000e-01, %conv4.i180
  %mul5.i182 = fmul double %sub.i181, 0x400921FB54442D18
  %mul.i12.i183 = fmul double %mul5.i182, %mul5.i182
  %40 = tail call double @llvm.fmuladd.f64(double %mul.i12.i183, double 0x3F8362B9BF971BCD, double 0x3F685DADFCECF44E)
  %41 = tail call double @llvm.fmuladd.f64(double %mul.i12.i183, double 0x3F991DF3908C33CE, double 0x3FAB54C91D865AFE)
  %mul3.i13.i184 = fmul double %mul.i12.i183, %mul.i12.i183
  %mul4.i14.i185 = fmul double %mul5.i182, %mul.i12.i183
  %42 = tail call double @llvm.fmuladd.f64(double %mul.i12.i183, double 0x3FC112FD38999F72, double 0x3FD5554D3418C99F)
  %43 = tail call double @llvm.fmuladd.f64(double %mul4.i14.i185, double %42, double %mul5.i182)
  %mul7.i15.i186 = fmul double %mul4.i14.i185, %mul3.i13.i184
  %44 = tail call double @llvm.fmuladd.f64(double %mul3.i13.i184, double %40, double %41)
  %45 = tail call double @llvm.fmuladd.f64(double %mul7.i15.i186, double %44, double %43)
  %div.i16.i187 = fdiv double -1.000000e+00, %45
  %retval.0.i17.i188 = fptrunc double %div.i16.i187 to float
  %fneg.i189 = fneg float %retval.0.i17.i188
  br label %if.end72

if.else62:                                        ; preds = %do.body32
  %cmp63 = icmp eq i32 %32, 1056964608
  br i1 %cmp63, label %if.then65, label %if.else67

if.then65:                                        ; preds = %if.else62
  %46 = load volatile float, ptr @vzero, align 4, !tbaa !5
  %div66 = fdiv float %conv46, %46
  br label %if.end72

if.else67:                                        ; preds = %if.else62
  %sub68 = fsub float 1.000000e+00, %sub47
  %cmp.i199 = fcmp olt float %sub68, 2.500000e-01
  br i1 %cmp.i199, label %if.then.i214, label %if.else.i200

if.then.i214:                                     ; preds = %if.else67
  %conv.i215 = fpext float %sub68 to double
  %mul.i216 = fmul double %conv.i215, 0x400921FB54442D18
  %mul.i.i217 = fmul double %mul.i216, %mul.i216
  %47 = tail call double @llvm.fmuladd.f64(double %mul.i.i217, double 0x3F8362B9BF971BCD, double 0x3F685DADFCECF44E)
  %48 = tail call double @llvm.fmuladd.f64(double %mul.i.i217, double 0x3F991DF3908C33CE, double 0x3FAB54C91D865AFE)
  %mul3.i.i218 = fmul double %mul.i.i217, %mul.i.i217
  %mul4.i.i219 = fmul double %mul.i216, %mul.i.i217
  %49 = tail call double @llvm.fmuladd.f64(double %mul.i.i217, double 0x3FC112FD38999F72, double 0x3FD5554D3418C99F)
  %50 = tail call double @llvm.fmuladd.f64(double %mul4.i.i219, double %49, double %mul.i216)
  %mul7.i.i220 = fmul double %mul4.i.i219, %mul3.i.i218
  %51 = tail call double @llvm.fmuladd.f64(double %mul3.i.i218, double %47, double %48)
  %52 = tail call double @llvm.fmuladd.f64(double %mul7.i.i220, double %51, double %50)
  %retval.0.i.i221 = fptrunc double %52 to float
  br label %ref___kernel_tanpif.exit222

if.else.i200:                                     ; preds = %if.else67
  %cmp1.i201 = fcmp ogt float %sub68, 2.500000e-01
  br i1 %cmp1.i201, label %if.then3.i203, label %ref___kernel_tanpif.exit222

if.then3.i203:                                    ; preds = %if.else.i200
  %conv4.i204 = fpext float %sub68 to double
  %sub.i205 = fsub double 5.000000e-01, %conv4.i204
  %mul5.i206 = fmul double %sub.i205, 0x400921FB54442D18
  %mul.i12.i207 = fmul double %mul5.i206, %mul5.i206
  %53 = tail call double @llvm.fmuladd.f64(double %mul.i12.i207, double 0x3F8362B9BF971BCD, double 0x3F685DADFCECF44E)
  %54 = tail call double @llvm.fmuladd.f64(double %mul.i12.i207, double 0x3F991DF3908C33CE, double 0x3FAB54C91D865AFE)
  %mul3.i13.i208 = fmul double %mul.i12.i207, %mul.i12.i207
  %mul4.i14.i209 = fmul double %mul5.i206, %mul.i12.i207
  %55 = tail call double @llvm.fmuladd.f64(double %mul.i12.i207, double 0x3FC112FD38999F72, double 0x3FD5554D3418C99F)
  %56 = tail call double @llvm.fmuladd.f64(double %mul4.i14.i209, double %55, double %mul5.i206)
  %mul7.i15.i210 = fmul double %mul4.i14.i209, %mul3.i13.i208
  %57 = tail call double @llvm.fmuladd.f64(double %mul3.i13.i208, double %53, double %54)
  %58 = tail call double @llvm.fmuladd.f64(double %mul7.i15.i210, double %57, double %56)
  %div.i16.i211 = fdiv double -1.000000e+00, %58
  %retval.0.i17.i212 = fptrunc double %div.i16.i211 to float
  %fneg.i213 = fneg float %retval.0.i17.i212
  br label %ref___kernel_tanpif.exit222

ref___kernel_tanpif.exit222:                      ; preds = %if.then.i214, %if.else.i200, %if.then3.i203
  %t.0.i202 = phi float [ %retval.0.i.i221, %if.then.i214 ], [ %fneg.i213, %if.then3.i203 ], [ 1.000000e+00, %if.else.i200 ]
  %fneg70 = fneg float %t.0.i202
  br label %if.end72

if.end72:                                         ; preds = %if.then3.i179, %if.else.i176, %if.then.i190, %cond.true57, %if.then65, %ref___kernel_tanpif.exit222
  %t.1 = phi float [ %div66, %if.then65 ], [ %fneg70, %ref___kernel_tanpif.exit222 ], [ %33, %cond.true57 ], [ %retval.0.i.i197, %if.then.i190 ], [ %fneg.i189, %if.then3.i179 ], [ 1.000000e+00, %if.else.i176 ]
  %fneg76 = fneg float %t.1
  %tobool74.not149 = icmp slt i32 %0, 0
  %cond79 = select i1 %tobool74.not149, float %fneg76, float %t.1
  br label %cleanup

if.end80:                                         ; preds = %if.end29
  %cmp81 = icmp ugt i32 %and, 2139095039
  br i1 %cmp81, label %if.then83, label %if.end85

if.then83:                                        ; preds = %if.end80
  %59 = load volatile float, ptr @vzero, align 4, !tbaa !5
  %60 = load volatile float, ptr @vzero, align 4, !tbaa !5
  %div84 = fdiv float %59, %60
  br label %cleanup

if.end85:                                         ; preds = %if.end80
  %cmp86 = icmp ugt i32 %and, 1266679807
  br i1 %cmp86, label %cond.end94, label %cond.false89

cond.false89:                                     ; preds = %if.end85
  %and90 = and i32 %and, 1
  %tobool91.not = icmp eq i32 %and90, 0
  %cond92 = select i1 %tobool91.not, i32 1, i32 -1
  %conv93 = sitofp i32 %cond92 to float
  %61 = tail call float @llvm.copysign.f32(float 0.000000e+00, float %conv93)
  br label %cond.end94

cond.end94:                                       ; preds = %if.end85, %cond.false89
  %cond95 = phi float [ %61, %cond.false89 ], [ 0.000000e+00, %if.end85 ]
  %fneg99 = fneg float %cond95
  %tobool97.not148 = icmp slt i32 %0, 0
  %cond102 = select i1 %tobool97.not148, float %fneg99, float %cond95
  br label %cleanup

cleanup:                                          ; preds = %if.then7, %cond.end94, %if.then83, %if.end72, %if.end26, %do.body10
  %retval.0 = phi float [ %mul19, %do.body10 ], [ %cond, %if.end26 ], [ %cond79, %if.end72 ], [ %div84, %if.then83 ], [ %cond102, %cond.end94 ], [ %x, %if.then7 ]
  ret float %retval.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fmuladd.f32(float, float, float) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.copysign.f32(float, float) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fmuladd.f64(double, double, double) #1

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fabs.f32(float) #2

attributes #0 = { mustprogress nofree norecurse nounwind willreturn memory(inaccessiblemem: readwrite) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

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
