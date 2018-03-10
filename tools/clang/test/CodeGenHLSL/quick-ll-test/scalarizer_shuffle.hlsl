; RUN: %opt %s -scalarizer -S | FileCheck %s

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f:64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%"$Globals" = type { i32, i32, i32 }
%dx.types.Handle = type { i8* }
%dx.types.CBufRet.i32 = type { i32, i32, i32, i32 }

@"\01?Loops@@3HB" = constant i32 0, align 4
@"\01?C@@3_NB" = constant i32 0, align 1
@"\01?D@@3_NB" = constant i32 0, align 1
@"$Globals" = external constant %"$Globals"

; Function Attrs: nounwind
define void @main(<3 x float>, <3 x float>* noalias) #0 {
entry:
  %"$Globals_buffer" = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 2, i32 0, i32 0, i1 false)
  %2 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0, i32 undef)
  %3 = insertelement <3 x float> undef, float %2, i64 0
  %4 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 1, i32 undef)
  %5 = insertelement <3 x float> %3, float %4, i64 1
  %6 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 2, i32 undef)
  %7 = insertelement <3 x float> %5, float %6, i64 2
  %8 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %"$Globals_buffer", i32 0)
  %9 = extractvalue %dx.types.CBufRet.i32 %8, 1
  %tobool = icmp ne i32 %9, 0
  br i1 %tobool, label %if.then, label %if.end.17

if.then:                                          ; preds = %entry
  %mul = fmul fast <3 x float> %7, <float 2.000000e+00, float 2.000000e+00, float 2.000000e+00>
  %10 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %"$Globals_buffer", i32 0)
  %11 = extractvalue %dx.types.CBufRet.i32 %10, 0
  %cmp.1 = icmp slt i32 0, %11
  br i1 %cmp.1, label %for.body, label %for.end

for.body:                                         ; preds = %if.then, %for.inc
  %retval.03 = phi <3 x float> [ %retval.1, %for.inc ], [ %mul, %if.then ]
  %i.02 = phi i32 [ %inc, %for.inc ], [ 0, %if.then ]
  %cmp3 = icmp eq i32 %i.02, 0
  %12 = extractelement <3 x float> %retval.03, i32 2
  %cmp5 = fcmp fast olt float %12, 0.000000e+00
  %13 = and i1 %cmp3, %cmp5
  %cmp7 = icmp eq i32 %i.02, 1
  %14 = extractelement <3 x float> %retval.03, i32 2
  %cmp9 = fcmp fast ogt float %14, 1.000000e+00
  %15 = and i1 %cmp7, %cmp9
  %16 = or i1 %13, %15
  br i1 %16, label %if.then.12, label %for.inc

if.then.12:                                       ; preds = %for.body
  %17 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %"$Globals_buffer", i32 0)
  %18 = extractvalue %dx.types.CBufRet.i32 %17, 2
  %tobool13 = icmp ne i32 %18, 0
  %19 = insertelement <3 x float> %retval.03, float 0.000000e+00, i32 2
  %.retval.03 = select i1 %tobool13, <3 x float> %19, <3 x float> %retval.03
  br label %for.inc

for.inc:                                          ; preds = %if.then.12, %for.body
  %retval.1 = phi <3 x float> [ %retval.03, %for.body ], [ %.retval.03, %if.then.12 ]
  %inc = add nsw i32 %i.02, 1
  %20 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %"$Globals_buffer", i32 0)
  %21 = extractvalue %dx.types.CBufRet.i32 %20, 0
  %cmp = icmp slt i32 %inc, %21
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc, %if.then
  %retval.0.lcssa = phi <3 x float> [ %mul, %if.then ], [ %retval.1, %for.inc ]
  %22 = shufflevector <3 x float> %retval.0.lcssa, <3 x float> undef, <2 x i32> <i32 0, i32 1>
  %mul16 = fmul fast <2 x float> %22, <float 2.000000e+00, float 2.000000e+00>
  %23 = extractelement <2 x float> %mul16, i64 0
  %24 = insertelement <3 x float> %retval.0.lcssa, float %23, i32 0
  %25 = extractelement <2 x float> %mul16, i64 1
  %26 = insertelement <3 x float> %24, float %25, i32 1
  br label %if.end.17

if.end.17:                                        ; preds = %for.end, %entry
  %retval.2 = phi <3 x float> [ %26, %for.end ], [ %7, %entry ]
  %27 = extractelement <3 x float> %retval.2, i64 0
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %27)
  %28 = extractelement <3 x float> %retval.2, i64 1
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float %28)
  %29 = extractelement <3 x float> %retval.2, i64 2
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float %29)
  ret void
}

; Function Attrs: nounwind readnone
declare float @dx.op.loadInput.f32(i32, i32, i32, i8, i32) #1

; Function Attrs: nounwind
declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #2

; Function Attrs: nounwind readonly
declare %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32, %dx.types.Handle, i32) #3

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandle(i32, i8, i32, i32, i1) #3

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="0" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind }
attributes #3 = { nounwind readonly }

!pauseresume = !{!0}
!llvm.ident = !{!1}
!dx.version = !{!2}
!dx.valver = !{!3}
!dx.shaderModel = !{!4}
!dx.resources = !{!5}
!dx.typeAnnotations = !{!8, !13}
!dx.entryPoints = !{!22}

!0 = !{!"hlsl-dxilemit", !"hlsl-dxilload"}
!1 = !{!"clang version 3.7 (tags/RELEASE_370/final)"}
!2 = !{i32 1, i32 0}
!3 = !{i32 1, i32 3}
!4 = !{!"vs", i32 6, i32 0}
!5 = !{null, null, !6, null}
!6 = !{!7}
!7 = !{i32 0, %"$Globals"* undef, !"$Globals", i32 0, i32 -1, i32 1, i32 12, null}
!8 = !{i32 0, %"$Globals" undef, !9}
!9 = !{i32 12, !10, !11, !12}
!10 = !{i32 6, !"Loops", i32 3, i32 0, i32 7, i32 4}
!11 = !{i32 6, !"C", i32 3, i32 4, i32 7, i32 1}
!12 = !{i32 6, !"D", i32 3, i32 8, i32 7, i32 1}
!13 = !{i32 1, void (<3 x float>, <3 x float>*)* @main, !14}
!14 = !{!15, !17, !20}
!15 = !{i32 0, !16, !16}
!16 = !{}
!17 = !{i32 0, !18, !19}
!18 = !{i32 4, !"SEM", i32 7, i32 9}
!19 = !{i32 0}
!20 = !{i32 1, !21, !19}
!21 = !{i32 4, !"OUT", i32 7, i32 9}
!22 = !{void (<3 x float>, <3 x float>*)* @main, !"main", !23, !5, null}
!23 = !{!24, !26, null}
!24 = !{!25}
!25 = !{i32 0, !"SEM", i8 9, i8 0, !19, i8 0, i32 1, i8 3, i32 0, i8 0, null}
!26 = !{!27}
!27 = !{i32 0, !"OUT", i8 9, i8 0, !19, i8 2, i32 1, i8 3, i32 0, i8 0, null}
