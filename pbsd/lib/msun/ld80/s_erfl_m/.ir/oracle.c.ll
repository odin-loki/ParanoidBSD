; ModuleID = '/home/odin/pbsd/pbsd/lib/msun/ld80/s_erfl_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/lib/msun/ld80/s_erfl_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@tiny = internal constant x86_fp80 0xK18EF8000000000000000, align 16

; Function Attrs: mustprogress nofree nounwind willreturn memory(write, inaccessiblemem: readwrite) uwtable
define dso_local x86_fp80 @ref_erfl(x86_fp80 noundef %x) local_unnamed_addr #0 {
entry:
  %0 = bitcast x86_fp80 %x to i80
  %1 = lshr i80 %0, 64
  %conv = trunc i80 %1 to i32
  %and = and i32 %conv, 32767
  %cmp = icmp eq i32 %and, 32767
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %2 = lshr i32 %conv, 14
  %shl = and i32 %2, 2
  %sub = sub nsw i32 1, %shl
  %conv7 = sitofp i32 %sub to x86_fp80
  %div = fdiv x86_fp80 0xK3FFF8000000000000000, %x
  %add = fadd x86_fp80 %div, %conv7
  br label %cleanup

if.end:                                           ; preds = %entry
  %3 = tail call x86_fp80 @llvm.fabs.f80(x86_fp80 %x)
  %cmp8 = fcmp olt x86_fp80 %3, 0xK3FFED800000000000000
  br i1 %cmp8, label %if.then10, label %if.end34

if.then10:                                        ; preds = %if.end
  %cmp11 = fcmp olt x86_fp80 %3, 0xK3FDD8000000000000000
  br i1 %cmp11, label %if.then13, label %if.end20

if.then13:                                        ; preds = %if.then10
  %cmp14 = fcmp olt x86_fp80 %3, 0xK000A8000000000000000
  br i1 %cmp14, label %if.then16, label %if.end19

if.then16:                                        ; preds = %if.then13
  %mul17 = fmul x86_fp80 %x, 0xK3FFF8375D410A6DB446C
  %4 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %x, x86_fp80 0xK40028000000000000000, x86_fp80 %mul17)
  %div18 = fmul x86_fp80 %4, 0xK3FFC8000000000000000
  br label %cleanup

if.end19:                                         ; preds = %if.then13
  %5 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %x, x86_fp80 0xK3FFC8375D410A6DB446C, x86_fp80 %x)
  br label %cleanup

if.end20:                                         ; preds = %if.then10
  %mul = fmul x86_fp80 %x, %x
  %6 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %mul, x86_fp80 0xKBFEF9EC4AD6193470693, x86_fp80 0xKBFF39F42BCBC3D5A601D)
  %7 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %mul, x86_fp80 %6, x86_fp80 0xKBFF8804AC72C9A0B97DD)
  %8 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %mul, x86_fp80 %7, x86_fp80 0xKBFFA9B31E66325576F86)
  %9 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %mul, x86_fp80 %8, x86_fp80 0xKBFFDA46C7D09EC3D0CEC)
  %10 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %mul, x86_fp80 %9, x86_fp80 0xK3FFC8375D410A6DB446C)
  %11 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %mul, x86_fp80 0xK3FE7F5BF98C2F996BF63, x86_fp80 0xK3FEF83466CB6BF9DCA00)
  %12 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %mul, x86_fp80 %11, x86_fp80 0xK3FF487332F82CFF4FF96)
  %13 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %mul, x86_fp80 %12, x86_fp80 0xK3FF88B85D6BD6A90B51C)
  %14 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %mul, x86_fp80 %13, x86_fp80 0xK3FFBA5750835B2459BD1)
  %15 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %mul, x86_fp80 %14, x86_fp80 0xK3FFDDB4B8EB713188D6B)
  %16 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %mul, x86_fp80 %15, x86_fp80 0xK3FFF8000000000000000)
  %div32 = fdiv x86_fp80 %10, %16
  %17 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %x, x86_fp80 %div32, x86_fp80 %x)
  br label %cleanup

if.end34:                                         ; preds = %if.end
  %cmp35 = fcmp olt x86_fp80 %3, 0xK3FFFA000000000000000
  br i1 %cmp35, label %if.then37, label %if.end60

if.then37:                                        ; preds = %if.end34
  %sub38 = fadd x86_fp80 %3, 0xKBFFF8000000000000000
  %18 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub38, x86_fp80 0xK3FF2DDBFBC23677B35CF, x86_fp80 0xK3FF785C8D58FE3993A47)
  %19 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub38, x86_fp80 %18, x86_fp80 0xKBFF7A5B6C4854D2F5452)
  %20 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub38, x86_fp80 %19, x86_fp80 0xK3FFA985D5D5FAFB0551F)
  %21 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub38, x86_fp80 %20, x86_fp80 0xK3FFBC8D31E020727C006)
  %22 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub38, x86_fp80 %21, x86_fp80 0xKBFFBECE74F8C63FA3942)
  %23 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub38, x86_fp80 %22, x86_fp80 0xK3FFDD488F89F36988618)
  %24 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub38, x86_fp80 %23, x86_fp80 0xK3FE4E8211158DA02C692)
  %25 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub38, x86_fp80 0xK3FF68FA866DC20717A91, x86_fp80 0xK3FF8A6C34BA438BDC900)
  %26 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub38, x86_fp80 %25, x86_fp80 0xK3FFABAB144F07DEA45BF)
  %27 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub38, x86_fp80 %26, x86_fp80 0xK3FFC881A4293F6D6C92D)
  %28 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub38, x86_fp80 %27, x86_fp80 0xK3FFD9D0B618EAC67BA07)
  %29 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub38, x86_fp80 %28, x86_fp80 0xK3FFE9FCD662C3D4EAC86)
  %30 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub38, x86_fp80 %29, x86_fp80 0xK3FFEB8A977896F5EFF3F)
  %31 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub38, x86_fp80 %30, x86_fp80 0xK3FFF8000000000000000)
  %cmp53 = fcmp ult x86_fp80 %x, 0xK00000000000000000000
  %div58 = fdiv x86_fp80 %24, %31
  br i1 %cmp53, label %if.else, label %if.then55

if.then55:                                        ; preds = %if.then37
  %add57 = fadd x86_fp80 %div58, 0xK3FFED7BB3D0000000000
  br label %cleanup

if.else:                                          ; preds = %if.then37
  %sub59 = fsub x86_fp80 0xKBFFED7BB3D0000000000, %div58
  br label %cleanup

if.end60:                                         ; preds = %if.end34
  %cmp61 = fcmp ult x86_fp80 %3, 0xK4001E000000000000000
  br i1 %cmp61, label %if.end70, label %if.then63

if.then63:                                        ; preds = %if.end60
  %cmp64 = fcmp ult x86_fp80 %x, 0xK00000000000000000000
  %32 = load volatile x86_fp80, ptr @tiny, align 16, !tbaa !5
  br i1 %cmp64, label %if.else68, label %if.then66

if.then66:                                        ; preds = %if.then63
  %sub67 = fsub x86_fp80 0xK3FFF8000000000000000, %32
  br label %cleanup

if.else68:                                        ; preds = %if.then63
  %sub69 = fadd x86_fp80 %32, 0xKBFFF8000000000000000
  br label %cleanup

if.end70:                                         ; preds = %if.end60
  %mul71 = fmul x86_fp80 %x, %x
  %div72 = fdiv x86_fp80 0xK3FFF8000000000000000, %mul71
  %cmp73 = fcmp olt x86_fp80 %3, 0xK4000B6DB8BAC710CB000
  br i1 %cmp73, label %if.then75, label %if.else94

if.then75:                                        ; preds = %if.end70
  %33 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div72, x86_fp80 0xKC001934C1A247807BB9C, x86_fp80 0xKC005D5AD1FAE77C3D9A3)
  %34 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div72, x86_fp80 %33, x86_fp80 0xKC00892A794E763A6D4DB)
  %35 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div72, x86_fp80 %34, x86_fp80 0xKC009983573E64D5015A9)
  %36 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div72, x86_fp80 %35, x86_fp80 0xKC0098FFE5383C08D4943)
  %37 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div72, x86_fp80 %36, x86_fp80 0xKC0088737C8B7B4062C2F)
  %38 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div72, x86_fp80 %37, x86_fp80 0xKC006813CC205395ADC7D)
  %39 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div72, x86_fp80 %38, x86_fp80 0xKC002F2CEC3EE7DA636C5)
  %40 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div72, x86_fp80 %39, x86_fp80 0xKBFFEC2B0D045AE37DF6B)
  %41 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div72, x86_fp80 %40, x86_fp80 0xKBFF8A1A091E0FB4F335A)
  %42 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div72, x86_fp80 0xK400092F030AEFADF28AD, x86_fp80 0xK4005C82B83855B88E07E)
  %43 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div72, x86_fp80 %42, x86_fp80 0xK4008E02AEE5F83773D1C)
  %44 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div72, x86_fp80 %43, x86_fp80 0xK400AB6EF97F9C753157B)
  %45 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div72, x86_fp80 %44, x86_fp80 0xK400B8796AFF2F3C47968)
  %46 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div72, x86_fp80 %45, x86_fp80 0xK400ACAA83F403713E33E)
  %47 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div72, x86_fp80 %46, x86_fp80 0xK40099F8CBA6D1AE1B24B)
  %48 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div72, x86_fp80 %47, x86_fp80 0xK4007839BE13D9D5DA883)
  %49 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div72, x86_fp80 %48, x86_fp80 0xK4003D342F90012BB1189)
  br label %if.end109

if.else94:                                        ; preds = %if.end70
  %50 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div72, x86_fp80 0xKC00795D21E3E75503C21, x86_fp80 0xKC009BAF655A76E0AB3B5)
  %51 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div72, x86_fp80 %50, x86_fp80 0xKC009E796E1D38C8C70A9)
  %52 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div72, x86_fp80 %51, x86_fp80 0xKC008DDE08465310B472B)
  %53 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div72, x86_fp80 %52, x86_fp80 0xKC006BFF0AE9FC0751DE6)
  %54 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div72, x86_fp80 %53, x86_fp80 0xKC0039A4DD1383E5DAF5B)
  %55 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div72, x86_fp80 %54, x86_fp80 0xKBFFED19D2DF1CBB8DA0A)
  %56 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div72, x86_fp80 %55, x86_fp80 0xKBFF8A1A091CF43ABCD26)
  %57 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div72, x86_fp80 0xK4006D675BBE542C159FA, x86_fp80 0xK400A9D09A35988934631)
  %58 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div72, x86_fp80 %57, x86_fp80 0xK400BBCFA72DA9B820874)
  %59 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div72, x86_fp80 %58, x86_fp80 0xK400BA55284359F3395A8)
  %60 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div72, x86_fp80 %59, x86_fp80 0xK400A809C4ADE46B927C7)
  %61 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div72, x86_fp80 %60, x86_fp80 0xK4007BE4BFBB1301304BE)
  %62 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div72, x86_fp80 %61, x86_fp80 0xK4004814487ED823C8CBD)
  br label %if.end109

if.end109:                                        ; preds = %if.else94, %if.then75
  %.sink = phi x86_fp80 [ %62, %if.else94 ], [ %49, %if.then75 ]
  %R.0 = phi x86_fp80 [ %56, %if.else94 ], [ %41, %if.then75 ]
  %63 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div72, x86_fp80 %.sink, x86_fp80 0xK3FFF8000000000000000)
  %conv110 = fptrunc x86_fp80 %3 to float
  %conv111 = fpext float %conv110 to x86_fp80
  %fneg112 = fneg x86_fp80 %conv111
  %64 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %fneg112, x86_fp80 %conv111, x86_fp80 0xKBFFE9000000000000000)
  %call = tail call x86_fp80 @expl(x86_fp80 noundef %64) #3
  %sub114 = fsub x86_fp80 %conv111, %3
  %add115 = fadd x86_fp80 %3, %conv111
  %div117 = fdiv x86_fp80 %R.0, %63
  %65 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub114, x86_fp80 %add115, x86_fp80 %div117)
  %call118 = tail call x86_fp80 @expl(x86_fp80 noundef %65) #3
  %mul119 = fmul x86_fp80 %call, %call118
  %cmp120 = fcmp ult x86_fp80 %x, 0xK00000000000000000000
  %div126 = fdiv x86_fp80 %mul119, %3
  br i1 %cmp120, label %if.else125, label %if.then122

if.then122:                                       ; preds = %if.end109
  %sub124 = fsub x86_fp80 0xK3FFF8000000000000000, %div126
  br label %cleanup

if.else125:                                       ; preds = %if.end109
  %sub127 = fadd x86_fp80 %div126, 0xKBFFF8000000000000000
  br label %cleanup

cleanup:                                          ; preds = %if.else125, %if.then122, %if.else68, %if.then66, %if.else, %if.then55, %if.end20, %if.end19, %if.then16, %if.then
  %retval.0 = phi x86_fp80 [ %add, %if.then ], [ %div18, %if.then16 ], [ %5, %if.end19 ], [ %17, %if.end20 ], [ %add57, %if.then55 ], [ %sub59, %if.else ], [ %sub67, %if.then66 ], [ %sub69, %if.else68 ], [ %sub124, %if.then122 ], [ %sub127, %if.else125 ]
  ret x86_fp80 %retval.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare x86_fp80 @llvm.fabs.f80(x86_fp80) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare x86_fp80 @llvm.fmuladd.f80(x86_fp80, x86_fp80, x86_fp80) #1

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare x86_fp80 @expl(x86_fp80 noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nofree nounwind willreturn memory(write, inaccessiblemem: readwrite) uwtable
define dso_local x86_fp80 @ref_erfcl(x86_fp80 noundef %x) local_unnamed_addr #0 {
entry:
  %0 = bitcast x86_fp80 %x to i80
  %1 = lshr i80 %0, 64
  %conv = trunc i80 %1 to i32
  %and = and i32 %conv, 32767
  %cmp = icmp eq i32 %and, 32767
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %2 = lshr i32 %conv, 14
  %shl = and i32 %2, 2
  %conv7 = sitofp i32 %shl to x86_fp80
  %div = fdiv x86_fp80 0xK3FFF8000000000000000, %x
  %add = fadd x86_fp80 %div, %conv7
  br label %cleanup

if.end:                                           ; preds = %entry
  %3 = tail call x86_fp80 @llvm.fabs.f80(x86_fp80 %x)
  %cmp8 = fcmp olt x86_fp80 %3, 0xK3FFED800000000000000
  br i1 %cmp8, label %if.then10, label %if.end36

if.then10:                                        ; preds = %if.end
  %cmp11 = fcmp olt x86_fp80 %3, 0xK3FDD8000000000000000
  br i1 %cmp11, label %if.then13, label %if.end14

if.then13:                                        ; preds = %if.then10
  %sub = fsub x86_fp80 0xK3FFF8000000000000000, %x
  br label %cleanup

if.end14:                                         ; preds = %if.then10
  %mul = fmul x86_fp80 %x, %x
  %4 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %mul, x86_fp80 0xKBFEF9EC4AD6193470693, x86_fp80 0xKBFF39F42BCBC3D5A601D)
  %5 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %mul, x86_fp80 %4, x86_fp80 0xKBFF8804AC72C9A0B97DD)
  %6 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %mul, x86_fp80 %5, x86_fp80 0xKBFFA9B31E66325576F86)
  %7 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %mul, x86_fp80 %6, x86_fp80 0xKBFFDA46C7D09EC3D0CEC)
  %8 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %mul, x86_fp80 %7, x86_fp80 0xK3FFC8375D410A6DB446C)
  %9 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %mul, x86_fp80 0xK3FE7F5BF98C2F996BF63, x86_fp80 0xK3FEF83466CB6BF9DCA00)
  %10 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %mul, x86_fp80 %9, x86_fp80 0xK3FF487332F82CFF4FF96)
  %11 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %mul, x86_fp80 %10, x86_fp80 0xK3FF88B85D6BD6A90B51C)
  %12 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %mul, x86_fp80 %11, x86_fp80 0xK3FFBA5750835B2459BD1)
  %13 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %mul, x86_fp80 %12, x86_fp80 0xK3FFDDB4B8EB713188D6B)
  %14 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %mul, x86_fp80 %13, x86_fp80 0xK3FFF8000000000000000)
  %div26 = fdiv x86_fp80 %8, %14
  %cmp27 = fcmp olt x86_fp80 %3, 0xK3FFD8000000000000000
  br i1 %cmp27, label %if.then29, label %if.else

if.then29:                                        ; preds = %if.end14
  %15 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %x, x86_fp80 %div26, x86_fp80 %x)
  %sub31 = fsub x86_fp80 0xK3FFF8000000000000000, %15
  br label %cleanup

if.else:                                          ; preds = %if.end14
  %mul32 = fmul x86_fp80 %div26, %x
  %sub33 = fadd x86_fp80 %x, 0xKBFFE8000000000000000
  %add34 = fadd x86_fp80 %sub33, %mul32
  %sub35 = fsub x86_fp80 0xK3FFE8000000000000000, %add34
  br label %cleanup

if.end36:                                         ; preds = %if.end
  %cmp37 = fcmp olt x86_fp80 %3, 0xK3FFFA000000000000000
  br i1 %cmp37, label %if.then39, label %if.end65

if.then39:                                        ; preds = %if.end36
  %sub40 = fadd x86_fp80 %3, 0xKBFFF8000000000000000
  %16 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub40, x86_fp80 0xK3FF2DDBFBC23677B35CF, x86_fp80 0xK3FF785C8D58FE3993A47)
  %17 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub40, x86_fp80 %16, x86_fp80 0xKBFF7A5B6C4854D2F5452)
  %18 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub40, x86_fp80 %17, x86_fp80 0xK3FFA985D5D5FAFB0551F)
  %19 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub40, x86_fp80 %18, x86_fp80 0xK3FFBC8D31E020727C006)
  %20 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub40, x86_fp80 %19, x86_fp80 0xKBFFBECE74F8C63FA3942)
  %21 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub40, x86_fp80 %20, x86_fp80 0xK3FFDD488F89F36988618)
  %22 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub40, x86_fp80 %21, x86_fp80 0xK3FE4E8211158DA02C692)
  %23 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub40, x86_fp80 0xK3FF68FA866DC20717A91, x86_fp80 0xK3FF8A6C34BA438BDC900)
  %24 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub40, x86_fp80 %23, x86_fp80 0xK3FFABAB144F07DEA45BF)
  %25 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub40, x86_fp80 %24, x86_fp80 0xK3FFC881A4293F6D6C92D)
  %26 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub40, x86_fp80 %25, x86_fp80 0xK3FFD9D0B618EAC67BA07)
  %27 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub40, x86_fp80 %26, x86_fp80 0xK3FFE9FCD662C3D4EAC86)
  %28 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub40, x86_fp80 %27, x86_fp80 0xK3FFEB8A977896F5EFF3F)
  %29 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub40, x86_fp80 %28, x86_fp80 0xK3FFF8000000000000000)
  %cmp55 = fcmp ult x86_fp80 %x, 0xK00000000000000000000
  %div62 = fdiv x86_fp80 %22, %29
  br i1 %cmp55, label %if.else61, label %if.then57

if.then57:                                        ; preds = %if.then39
  %sub60 = fsub x86_fp80 0xK3FFCA1130C0000000000, %div62
  br label %cleanup

if.else61:                                        ; preds = %if.then39
  %add63 = fadd x86_fp80 %div62, 0xK3FFED7BB3D0000000000
  %add64 = fadd x86_fp80 %add63, 0xK3FFF8000000000000000
  br label %cleanup

if.end65:                                         ; preds = %if.end36
  %cmp66 = fcmp olt x86_fp80 %3, 0xK4005D800000000000000
  br i1 %cmp66, label %if.then68, label %if.else144

if.then68:                                        ; preds = %if.end65
  %mul69 = fmul x86_fp80 %x, %x
  %div70 = fdiv x86_fp80 0xK3FFF8000000000000000, %mul69
  %cmp71 = fcmp olt x86_fp80 %3, 0xK4000B6DB8BAC710CB000
  br i1 %cmp71, label %if.then73, label %if.else92

if.then73:                                        ; preds = %if.then68
  %30 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 0xKC001934C1A247807BB9C, x86_fp80 0xKC005D5AD1FAE77C3D9A3)
  %31 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %30, x86_fp80 0xKC00892A794E763A6D4DB)
  %32 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %31, x86_fp80 0xKC009983573E64D5015A9)
  %33 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %32, x86_fp80 0xKC0098FFE5383C08D4943)
  %34 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %33, x86_fp80 0xKC0088737C8B7B4062C2F)
  %35 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %34, x86_fp80 0xKC006813CC205395ADC7D)
  %36 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %35, x86_fp80 0xKC002F2CEC3EE7DA636C5)
  %37 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %36, x86_fp80 0xKBFFEC2B0D045AE37DF6B)
  %38 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %37, x86_fp80 0xKBFF8A1A091E0FB4F335A)
  %39 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 0xK400092F030AEFADF28AD, x86_fp80 0xK4005C82B83855B88E07E)
  %40 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %39, x86_fp80 0xK4008E02AEE5F83773D1C)
  %41 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %40, x86_fp80 0xK400AB6EF97F9C753157B)
  %42 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %41, x86_fp80 0xK400B8796AFF2F3C47968)
  %43 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %42, x86_fp80 0xK400ACAA83F403713E33E)
  %44 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %43, x86_fp80 0xK40099F8CBA6D1AE1B24B)
  %45 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %44, x86_fp80 0xK4007839BE13D9D5DA883)
  %46 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %45, x86_fp80 0xK4003D342F90012BB1189)
  br label %if.end127

if.else92:                                        ; preds = %if.then68
  %cmp93 = fcmp olt x86_fp80 %3, 0xK4001E000000000000000
  br i1 %cmp93, label %if.then95, label %if.else110

if.then95:                                        ; preds = %if.else92
  %47 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 0xKC00795D21E3E75503C21, x86_fp80 0xKC009BAF655A76E0AB3B5)
  %48 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %47, x86_fp80 0xKC009E796E1D38C8C70A9)
  %49 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %48, x86_fp80 0xKC008DDE08465310B472B)
  %50 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %49, x86_fp80 0xKC006BFF0AE9FC0751DE6)
  %51 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %50, x86_fp80 0xKC0039A4DD1383E5DAF5B)
  %52 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %51, x86_fp80 0xKBFFED19D2DF1CBB8DA0A)
  %53 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %52, x86_fp80 0xKBFF8A1A091CF43ABCD26)
  %54 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 0xK4006D675BBE542C159FA, x86_fp80 0xK400A9D09A35988934631)
  %55 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %54, x86_fp80 0xK400BBCFA72DA9B820874)
  %56 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %55, x86_fp80 0xK400BA55284359F3395A8)
  %57 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %56, x86_fp80 0xK400A809C4ADE46B927C7)
  %58 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %57, x86_fp80 0xK4007BE4BFBB1301304BE)
  %59 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %58, x86_fp80 0xK4004814487ED823C8CBD)
  br label %if.end127

if.else110:                                       ; preds = %if.else92
  %cmp111 = fcmp olt x86_fp80 %x, 0xKC001E000000000000000
  br i1 %cmp111, label %if.then113, label %if.end115

if.then113:                                       ; preds = %if.else110
  %60 = load volatile x86_fp80, ptr @tiny, align 16, !tbaa !5
  %sub114 = fsub x86_fp80 0xK40008000000000000000, %60
  br label %cleanup

if.end115:                                        ; preds = %if.else110
  %61 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 0xKC005FE528153C45EC97C, x86_fp80 0xKC006D79676D970D0A21A)
  %62 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %61, x86_fp80 0xKC005B5F69A38F5747AC8)
  %63 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %62, x86_fp80 0xKC002DB26F9BBE31A2794)
  %64 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %63, x86_fp80 0xKBFFEBE79C5A978122B00)
  %65 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %64, x86_fp80 0xKBFF8A1A091CF437A17AD)
  %66 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 0xK4005F80DFCBF37FFC5EA, x86_fp80 0xK400899BE1B89FAA0596A)
  %67 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %66, x86_fp80 0xK4008964E3C7B34DB9170)
  %68 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %67, x86_fp80 0xK4006C5F0F5A5484520EB)
  %69 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %68, x86_fp80 0xK4003C5E8CD46D5604A96)
  br label %if.end127

if.end127:                                        ; preds = %if.then95, %if.end115, %if.then73
  %.sink = phi x86_fp80 [ %59, %if.then95 ], [ %69, %if.end115 ], [ %46, %if.then73 ]
  %R.0 = phi x86_fp80 [ %53, %if.then95 ], [ %65, %if.end115 ], [ %38, %if.then73 ]
  %70 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %div70, x86_fp80 %.sink, x86_fp80 0xK3FFF8000000000000000)
  %conv128 = fptrunc x86_fp80 %3 to float
  %conv129 = fpext float %conv128 to x86_fp80
  %fneg = fneg x86_fp80 %conv129
  %71 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %fneg, x86_fp80 %conv129, x86_fp80 0xKBFFE9000000000000000)
  %call = tail call x86_fp80 @expl(x86_fp80 noundef %71) #3
  %sub131 = fsub x86_fp80 %conv129, %3
  %add132 = fadd x86_fp80 %3, %conv129
  %div134 = fdiv x86_fp80 %R.0, %70
  %72 = tail call x86_fp80 @llvm.fmuladd.f80(x86_fp80 %sub131, x86_fp80 %add132, x86_fp80 %div134)
  %call135 = tail call x86_fp80 @expl(x86_fp80 noundef %72) #3
  %mul136 = fmul x86_fp80 %call, %call135
  %cmp137 = fcmp ogt x86_fp80 %x, 0xK00000000000000000000
  %div140 = fdiv x86_fp80 %mul136, %3
  br i1 %cmp137, label %cleanup, label %if.else141

if.else141:                                       ; preds = %if.end127
  %sub143 = fsub x86_fp80 0xK40008000000000000000, %div140
  br label %cleanup

if.else144:                                       ; preds = %if.end65
  %cmp145 = fcmp ogt x86_fp80 %x, 0xK00000000000000000000
  %73 = load volatile x86_fp80, ptr @tiny, align 16, !tbaa !5
  br i1 %cmp145, label %if.then147, label %if.else149

if.then147:                                       ; preds = %if.else144
  %74 = load volatile x86_fp80, ptr @tiny, align 16, !tbaa !5
  %mul148 = fmul x86_fp80 %73, %74
  br label %cleanup

if.else149:                                       ; preds = %if.else144
  %sub150 = fsub x86_fp80 0xK40008000000000000000, %73
  br label %cleanup

cleanup:                                          ; preds = %if.end127, %if.else149, %if.then147, %if.else141, %if.then113, %if.else61, %if.then57, %if.else, %if.then29, %if.then13, %if.then
  %retval.0 = phi x86_fp80 [ %add, %if.then ], [ %sub, %if.then13 ], [ %sub31, %if.then29 ], [ %sub35, %if.else ], [ %sub60, %if.then57 ], [ %add64, %if.else61 ], [ %sub143, %if.else141 ], [ %sub114, %if.then113 ], [ %mul148, %if.then147 ], [ %sub150, %if.else149 ], [ %div140, %if.end127 ]
  ret x86_fp80 %retval.0
}

attributes #0 = { mustprogress nofree nounwind willreturn memory(write, inaccessiblemem: readwrite) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { mustprogress nofree nounwind willreturn memory(write) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!5 = !{!6, !6, i64 0}
!6 = !{!"long double", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
