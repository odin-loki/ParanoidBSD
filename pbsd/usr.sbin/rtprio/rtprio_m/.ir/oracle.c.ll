; ModuleID = '/home/odin/pbsd/pbsd/usr.sbin/rtprio/rtprio_m/oracle.c'
source_filename = "/home/odin/pbsd/pbsd/usr.sbin/rtprio/rtprio_m/oracle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

module asm ".symver __qsort_r_compat, qsort_r@FBSD_1.0"

%struct.rtprio = type { i16, i16 }
%struct._RuneLocale = type { [8 x i8], [32 x i8], ptr, ptr, i32, [256 x i64], [256 x i32], [256 x i32], %struct._RuneRange, %struct._RuneRange, %struct._RuneRange, ptr, i32 }
%struct._RuneRange = type { i32, ptr }

@.str = private unnamed_addr constant [7 x i8] c"rtprio\00", align 1
@.str.1 = private unnamed_addr constant [7 x i8] c"idprio\00", align 1
@.str.2 = private unnamed_addr constant [17 x i8] c"invalid progname\00", align 1
@.str.3 = private unnamed_addr constant [4 x i8] c"pid\00", align 1
@.str.4 = private unnamed_addr constant [11 x i8] c"RTP_LOOKUP\00", align 1
@.str.5 = private unnamed_addr constant [21 x i8] c"realtime priority %d\00", align 1
@.str.6 = private unnamed_addr constant [16 x i8] c"normal priority\00", align 1
@.str.7 = private unnamed_addr constant [17 x i8] c"idle priority %d\00", align 1
@.str.8 = private unnamed_addr constant [25 x i8] c"invalid priority type %d\00", align 1
@.str.9 = private unnamed_addr constant [3 x i8] c"-t\00", align 1
@.str.10 = private unnamed_addr constant [9 x i8] c"priority\00", align 1
@.str.11 = private unnamed_addr constant [8 x i8] c"RTP_SET\00", align 1
@.str.12 = private unnamed_addr constant [11 x i8] c"execvp: %s\00", align 1
@__mb_sb_limit = external local_unnamed_addr global i32, align 4
@_ThreadRuneLocale = external thread_local global ptr, align 8
@_CurrentRuneLocale = external local_unnamed_addr global ptr, align 8
@.str.13 = private unnamed_addr constant [20 x i8] c"%s must be a number\00", align 1
@.str.14 = private unnamed_addr constant [28 x i8] c"Integer overflow parsing %s\00", align 1
@__stderrp = external local_unnamed_addr global ptr, align 8
@.str.15 = private unnamed_addr constant [19 x i8] c"%s\0A%s\0A%s\0A%s\0A%s\0A%s\0A\00", align 1
@.str.16 = private unnamed_addr constant [19 x i8] c"usage: [id|rt]prio\00", align 1
@.str.17 = private unnamed_addr constant [26 x i8] c"       [id|rt]prio [-]pid\00", align 1
@.str.18 = private unnamed_addr constant [43 x i8] c"       [id|rt]prio priority command [args]\00", align 1
@.str.19 = private unnamed_addr constant [33 x i8] c"       [id|rt]prio priority -pid\00", align 1
@.str.20 = private unnamed_addr constant [37 x i8] c"       [id|rt]prio -t command [args]\00", align 1
@.str.21 = private unnamed_addr constant [27 x i8] c"       [id|rt]prio -t -pid\00", align 1

; Function Attrs: noreturn nounwind uwtable
define dso_local noundef i32 @ref_main(i32 noundef %argc, ptr noundef %argv) local_unnamed_addr #0 {
entry:
  %endp.i109 = alloca ptr, align 8
  %endp.i94 = alloca ptr, align 8
  %endp.i = alloca ptr, align 8
  %rtp = alloca %struct.rtprio, align 2
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %rtp) #9
  %call = tail call ptr @getprogname() #9
  %call1 = tail call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %call, ptr noundef nonnull dereferenceable(7) @.str) #10
  %cmp = icmp eq i32 %call1, 0
  br i1 %cmp, label %if.end7, label %if.else

if.else:                                          ; preds = %entry
  %call2 = tail call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %call, ptr noundef nonnull dereferenceable(7) @.str.1) #10
  %cmp3 = icmp eq i32 %call2, 0
  br i1 %cmp3, label %if.end7, label %if.else6

if.else6:                                         ; preds = %if.else
  tail call void (i32, ptr, ...) @errx(i32 noundef 1, ptr noundef nonnull @.str.2) #11
  unreachable

if.end7:                                          ; preds = %if.else, %entry
  %storemerge = phi i16 [ 2, %entry ], [ 4, %if.else ]
  store i16 %storemerge, ptr %rtp, align 2, !tbaa !5
  switch i32 %argc, label %sw.default23 [
    i32 2, label %sw.bb
    i32 1, label %sw.bb9
  ]

sw.bb:                                            ; preds = %if.end7
  %arrayidx = getelementptr inbounds ptr, ptr %argv, i64 1
  %0 = load ptr, ptr %arrayidx, align 8, !tbaa !10
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %endp.i) #9
  %call.i = tail call ptr @__error() #9
  store i32 0, ptr %call.i, align 4, !tbaa !12
  %call1.i = call i64 @strtol(ptr noundef %0, ptr noundef nonnull %endp.i, i32 noundef 10)
  %call2.i = tail call ptr @__error() #9
  %1 = load i32, ptr %call2.i, align 4, !tbaa !12
  %cmp.not.i = icmp eq i32 %1, 0
  br i1 %cmp.not.i, label %lor.lhs.false.i, label %if.then.i

lor.lhs.false.i:                                  ; preds = %sw.bb
  %2 = load ptr, ptr %endp.i, align 8, !tbaa !10
  %cmp3.i = icmp eq ptr %2, %0
  br i1 %cmp3.i, label %if.then.i, label %lor.lhs.false4.i

lor.lhs.false4.i:                                 ; preds = %lor.lhs.false.i
  %3 = load i8, ptr %2, align 1, !tbaa !14
  %cmp5.not.i = icmp eq i8 %3, 0
  br i1 %cmp5.not.i, label %if.end.i, label %if.then.i

if.then.i:                                        ; preds = %lor.lhs.false4.i, %lor.lhs.false.i, %sw.bb
  tail call void (i32, ptr, ...) @errx(i32 noundef 1, ptr noundef nonnull @.str.13, ptr noundef nonnull @.str.3) #11
  unreachable

if.end.i:                                         ; preds = %lor.lhs.false4.i
  %cmp7.i = icmp sgt i64 %call1.i, 2147483646
  br i1 %cmp7.i, label %if.then9.i, label %ref_parseint.exit

if.then9.i:                                       ; preds = %if.end.i
  tail call void (i32, ptr, ...) @errx(i32 noundef 1, ptr noundef nonnull @.str.14, ptr noundef nonnull @.str.3) #11
  unreachable

ref_parseint.exit:                                ; preds = %if.end.i
  %conv11.i = trunc i64 %call1.i to i32
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %endp.i) #9
  %4 = tail call i32 @llvm.abs.i32(i32 %conv11.i, i1 true)
  br label %sw.bb9

sw.bb9:                                           ; preds = %if.end7, %ref_parseint.exit
  %proc.0 = phi i32 [ 0, %if.end7 ], [ %4, %ref_parseint.exit ]
  %call10 = call i32 @rtprio(i32 noundef 0, i32 noundef %proc.0, ptr noundef nonnull %rtp) #9
  %cmp11.not = icmp eq i32 %call10, 0
  br i1 %cmp11.not, label %if.end13, label %if.then12

if.then12:                                        ; preds = %sw.bb9
  call void (i32, ptr, ...) @err(i32 noundef 1, ptr noundef nonnull @.str.4) #11
  unreachable

if.end13:                                         ; preds = %sw.bb9
  %5 = load i16, ptr %rtp, align 2, !tbaa !5
  switch i16 %5, label %sw.default [
    i16 2, label %sw.bb15
    i16 10, label %sw.bb15
    i16 3, label %sw.bb17
    i16 4, label %sw.bb18
  ]

sw.bb15:                                          ; preds = %if.end13, %if.end13
  %prio = getelementptr inbounds %struct.rtprio, ptr %rtp, i64 0, i32 1
  %6 = load i16, ptr %prio, align 2, !tbaa !15
  %conv16 = zext i16 %6 to i32
  call void (ptr, ...) @warnx(ptr noundef nonnull @.str.5, i32 noundef %conv16) #9
  br label %sw.epilog

sw.bb17:                                          ; preds = %if.end13
  call void (ptr, ...) @warnx(ptr noundef nonnull @.str.6) #9
  br label %sw.epilog

sw.bb18:                                          ; preds = %if.end13
  %prio19 = getelementptr inbounds %struct.rtprio, ptr %rtp, i64 0, i32 1
  %7 = load i16, ptr %prio19, align 2, !tbaa !15
  %conv20 = zext i16 %7 to i32
  call void (ptr, ...) @warnx(ptr noundef nonnull @.str.7, i32 noundef %conv20) #9
  br label %sw.epilog

sw.default:                                       ; preds = %if.end13
  %conv = zext i16 %5 to i32
  call void (i32, ptr, ...) @errx(i32 noundef 1, ptr noundef nonnull @.str.8, i32 noundef %conv) #11
  unreachable

sw.epilog:                                        ; preds = %sw.bb18, %sw.bb17, %sw.bb15
  call void @exit(i32 noundef 0) #11
  unreachable

sw.default23:                                     ; preds = %if.end7
  %arrayidx24 = getelementptr inbounds ptr, ptr %argv, i64 1
  %8 = load ptr, ptr %arrayidx24, align 8, !tbaa !10
  %9 = load i8, ptr %8, align 1, !tbaa !14
  %cmp27 = icmp eq i8 %9, 45
  br i1 %cmp27, label %if.then39, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %sw.default23
  %conv26 = sext i8 %9 to i32
  %cmp.i.i = icmp sgt i8 %9, -1
  %10 = load i32, ptr @__mb_sb_limit, align 4
  %cmp1.not.i.i = icmp sgt i32 %10, %conv26
  %or.cond.i.i = select i1 %cmp.i.i, i1 %cmp1.not.i.i, i1 false
  br i1 %or.cond.i.i, label %__sbistype.exit, label %if.else55

__sbistype.exit:                                  ; preds = %lor.lhs.false
  %11 = tail call align 8 ptr @llvm.threadlocal.address.p0(ptr align 8 @_ThreadRuneLocale)
  %12 = load ptr, ptr %11, align 8, !tbaa !10
  %tobool.not.i.i.i = icmp eq ptr %12, null
  %13 = load ptr, ptr @_CurrentRuneLocale, align 8
  %retval.0.i.i.i = select i1 %tobool.not.i.i.i, ptr %13, ptr %12
  %idxprom.i.i = zext nneg i32 %conv26 to i64
  %arrayidx.i.i = getelementptr inbounds %struct._RuneLocale, ptr %retval.0.i.i.i, i64 0, i32 5, i64 %idxprom.i.i
  %14 = load i64, ptr %arrayidx.i.i, align 8, !tbaa !16
  %15 = and i64 %14, 1024
  %tobool.not = icmp eq i64 %15, 0
  br i1 %tobool.not, label %if.else55, label %if.else49

if.then39:                                        ; preds = %sw.default23
  %call41 = tail call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %8, ptr noundef nonnull dereferenceable(3) @.str.9) #10
  %cmp42 = icmp eq i32 %call41, 0
  br i1 %cmp42, label %if.then44, label %if.else47

if.then44:                                        ; preds = %if.then39
  store i16 3, ptr %rtp, align 2, !tbaa !5
  br label %if.end56

if.else47:                                        ; preds = %if.then39
  tail call fastcc void @ref_usage() #12
  unreachable

if.else49:                                        ; preds = %__sbistype.exit
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %endp.i94) #9
  %call.i95 = tail call ptr @__error() #9
  store i32 0, ptr %call.i95, align 4, !tbaa !12
  %call1.i96 = call i64 @strtol(ptr noundef nonnull %8, ptr noundef nonnull %endp.i94, i32 noundef 10)
  %call2.i97 = tail call ptr @__error() #9
  %16 = load i32, ptr %call2.i97, align 4, !tbaa !12
  %cmp.not.i98 = icmp eq i32 %16, 0
  br i1 %cmp.not.i98, label %lor.lhs.false.i100, label %if.then.i99

lor.lhs.false.i100:                               ; preds = %if.else49
  %17 = load ptr, ptr %endp.i94, align 8, !tbaa !10
  %cmp3.i101 = icmp eq ptr %17, %8
  br i1 %cmp3.i101, label %if.then.i99, label %lor.lhs.false4.i102

lor.lhs.false4.i102:                              ; preds = %lor.lhs.false.i100
  %18 = load i8, ptr %17, align 1, !tbaa !14
  %cmp5.not.i103 = icmp eq i8 %18, 0
  br i1 %cmp5.not.i103, label %if.end.i104, label %if.then.i99

if.then.i99:                                      ; preds = %lor.lhs.false4.i102, %lor.lhs.false.i100, %if.else49
  tail call void (i32, ptr, ...) @errx(i32 noundef 1, ptr noundef nonnull @.str.13, ptr noundef nonnull @.str.10) #11
  unreachable

if.end.i104:                                      ; preds = %lor.lhs.false4.i102
  %cmp7.i105 = icmp sgt i64 %call1.i96, 2147483646
  br i1 %cmp7.i105, label %if.then9.i107, label %ref_parseint.exit108

if.then9.i107:                                    ; preds = %if.end.i104
  tail call void (i32, ptr, ...) @errx(i32 noundef 1, ptr noundef nonnull @.str.14, ptr noundef nonnull @.str.10) #11
  unreachable

ref_parseint.exit108:                             ; preds = %if.end.i104
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %endp.i94) #9
  %conv52 = trunc i64 %call1.i96 to i16
  br label %if.end56

if.else55:                                        ; preds = %lor.lhs.false, %__sbistype.exit
  tail call fastcc void @ref_usage() #12
  unreachable

if.end56:                                         ; preds = %if.then44, %ref_parseint.exit108
  %.sink = phi i16 [ 0, %if.then44 ], [ %conv52, %ref_parseint.exit108 ]
  %prio46 = getelementptr inbounds %struct.rtprio, ptr %rtp, i64 0, i32 1
  store i16 %.sink, ptr %prio46, align 2, !tbaa !15
  %arrayidx57 = getelementptr inbounds ptr, ptr %argv, i64 2
  %19 = load ptr, ptr %arrayidx57, align 8, !tbaa !10
  %20 = load i8, ptr %19, align 1, !tbaa !14
  %cmp60 = icmp eq i8 %20, 45
  br i1 %cmp60, label %if.then62, label %if.end65.thread

if.then62:                                        ; preds = %if.end56
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %endp.i109) #9
  %call.i110 = tail call ptr @__error() #9
  store i32 0, ptr %call.i110, align 4, !tbaa !12
  %call1.i111 = call i64 @strtol(ptr noundef nonnull %19, ptr noundef nonnull %endp.i109, i32 noundef 10)
  %call2.i112 = tail call ptr @__error() #9
  %21 = load i32, ptr %call2.i112, align 4, !tbaa !12
  %cmp.not.i113 = icmp eq i32 %21, 0
  br i1 %cmp.not.i113, label %lor.lhs.false.i115, label %if.then.i114

lor.lhs.false.i115:                               ; preds = %if.then62
  %22 = load ptr, ptr %endp.i109, align 8, !tbaa !10
  %cmp3.i116 = icmp eq ptr %22, %19
  br i1 %cmp3.i116, label %if.then.i114, label %lor.lhs.false4.i117

lor.lhs.false4.i117:                              ; preds = %lor.lhs.false.i115
  %23 = load i8, ptr %22, align 1, !tbaa !14
  %cmp5.not.i118 = icmp eq i8 %23, 0
  br i1 %cmp5.not.i118, label %if.end.i119, label %if.then.i114

if.then.i114:                                     ; preds = %lor.lhs.false4.i117, %lor.lhs.false.i115, %if.then62
  tail call void (i32, ptr, ...) @errx(i32 noundef 1, ptr noundef nonnull @.str.13, ptr noundef nonnull @.str.3) #11
  unreachable

if.end.i119:                                      ; preds = %lor.lhs.false4.i117
  %cmp7.i120 = icmp sgt i64 %call1.i111, 2147483646
  br i1 %cmp7.i120, label %if.then9.i122, label %if.end65

if.then9.i122:                                    ; preds = %if.end.i119
  tail call void (i32, ptr, ...) @errx(i32 noundef 1, ptr noundef nonnull @.str.14, ptr noundef nonnull @.str.3) #11
  unreachable

if.end65:                                         ; preds = %if.end.i119
  %conv11.i121 = trunc i64 %call1.i111 to i32
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %endp.i109) #9
  %24 = tail call i32 @llvm.abs.i32(i32 %conv11.i121, i1 true)
  %call66 = call i32 @rtprio(i32 noundef 1, i32 noundef %24, ptr noundef nonnull %rtp) #9
  %cmp67.not = icmp eq i32 %call66, 0
  br i1 %cmp67.not, label %if.end70, label %if.then69

if.end65.thread:                                  ; preds = %if.end56
  %call66127 = call i32 @rtprio(i32 noundef 1, i32 noundef 0, ptr noundef nonnull %rtp) #9
  %cmp67.not128 = icmp eq i32 %call66127, 0
  br i1 %cmp67.not128, label %if.then73, label %if.then69

if.then69:                                        ; preds = %if.end65.thread, %if.end65
  call void (i32, ptr, ...) @err(i32 noundef 1, ptr noundef nonnull @.str.11) #11
  unreachable

if.end70:                                         ; preds = %if.end65
  %cmp71 = icmp eq i32 %conv11.i121, 0
  br i1 %cmp71, label %if.then73, label %if.end78

if.then73:                                        ; preds = %if.end65.thread, %if.end70
  %25 = load ptr, ptr %arrayidx57, align 8, !tbaa !10
  %call76 = call i32 @execvp(ptr noundef %25, ptr noundef nonnull %arrayidx57) #9
  %26 = load ptr, ptr %arrayidx57, align 8, !tbaa !10
  call void (i32, ptr, ...) @err(i32 noundef 1, ptr noundef nonnull @.str.12, ptr noundef %26) #11
  unreachable

if.end78:                                         ; preds = %if.end70
  call void @exit(i32 noundef 0) #11
  unreachable
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

declare ptr @getprogname() local_unnamed_addr #2

; Function Attrs: mustprogress nofree nounwind willreturn memory(argmem: read)
declare i32 @strcmp(ptr nocapture noundef, ptr nocapture noundef) local_unnamed_addr #3

; Function Attrs: noreturn
declare void @errx(i32 noundef, ptr noundef, ...) local_unnamed_addr #4

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.abs.i32(i32, i1 immarg) #5

declare i32 @rtprio(i32 noundef, i32 noundef, ptr noundef) local_unnamed_addr #2

; Function Attrs: noreturn
declare void @err(i32 noundef, ptr noundef, ...) local_unnamed_addr #4

declare void @warnx(ptr noundef, ...) local_unnamed_addr #2

; Function Attrs: noreturn
declare void @exit(i32 noundef) local_unnamed_addr #4

; Function Attrs: noreturn nounwind uwtable
define internal fastcc void @ref_usage() unnamed_addr #0 {
entry:
  %0 = load ptr, ptr @__stderrp, align 8, !tbaa !10
  %call = tail call i32 (ptr, ptr, ...) @fprintf(ptr noundef %0, ptr noundef nonnull @.str.15, ptr noundef nonnull @.str.16, ptr noundef nonnull @.str.17, ptr noundef nonnull @.str.18, ptr noundef nonnull @.str.19, ptr noundef nonnull @.str.20, ptr noundef nonnull @.str.21)
  tail call void @exit(i32 noundef 1) #11
  unreachable
}

; Function Attrs: nofree
declare i32 @execvp(ptr noundef, ptr noundef) local_unnamed_addr #6

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare nonnull ptr @llvm.threadlocal.address.p0(ptr nonnull) #5

declare ptr @__error() local_unnamed_addr #2

; Function Attrs: mustprogress nofree nounwind willreturn
declare i64 @strtol(ptr noundef readonly, ptr nocapture noundef, i32 noundef) local_unnamed_addr #7

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nofree nounwind
declare noundef i32 @fprintf(ptr nocapture noundef, ptr nocapture noundef readonly, ...) local_unnamed_addr #8

attributes #0 = { noreturn nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { mustprogress nofree nounwind willreturn memory(argmem: read) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { noreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #6 = { nofree "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #7 = { mustprogress nofree nounwind willreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #8 = { nofree nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #9 = { nounwind }
attributes #10 = { nounwind willreturn memory(read) }
attributes #11 = { noreturn nounwind }
attributes #12 = { noreturn }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!5 = !{!6, !7, i64 0}
!6 = !{!"rtprio", !7, i64 0, !7, i64 2}
!7 = !{!"short", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{!11, !11, i64 0}
!11 = !{!"any pointer", !8, i64 0}
!12 = !{!13, !13, i64 0}
!13 = !{!"int", !8, i64 0}
!14 = !{!8, !8, i64 0}
!15 = !{!6, !7, i64 2}
!16 = !{!17, !17, i64 0}
!17 = !{!"long", !8, i64 0}
