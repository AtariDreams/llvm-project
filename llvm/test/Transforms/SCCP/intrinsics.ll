; NOTE: Assertions have been autogenerated by utils/update_test_checks.py
; RUN: opt -ipsccp -S %s | FileCheck %s

declare i8 @llvm.abs.i8(i8, i1)
declare <2 x i8> @llvm.abs.v2i8(<2 x i8>, i1)
declare i8 @llvm.umax.i8(i8, i8)

declare void @use(i1)
declare void @use_vec(<2 x i1>)

define void @abs1(i8* %p) {
; CHECK-LABEL: @abs1(
; CHECK-NEXT:    [[X:%.*]] = load i8, i8* [[P:%.*]], align 1, [[RNG0:!range !.*]]
; CHECK-NEXT:    [[ABS:%.*]] = call i8 @llvm.abs.i8(i8 [[X]], i1 false)
; CHECK-NEXT:    call void @use(i1 true)
; CHECK-NEXT:    call void @use(i1 true)
; CHECK-NEXT:    [[CMP3:%.*]] = icmp sge i8 [[ABS]], 1
; CHECK-NEXT:    call void @use(i1 [[CMP3]])
; CHECK-NEXT:    [[CMP4:%.*]] = icmp slt i8 [[ABS]], 9
; CHECK-NEXT:    call void @use(i1 [[CMP4]])
; CHECK-NEXT:    ret void
;
  %x = load i8, i8* %p, !range !{i8 -9, i8 10}
  %abs = call i8 @llvm.abs.i8(i8 %x, i1 false)
  %cmp1 = icmp sge i8 %abs, 0
  call void @use(i1 %cmp1)
  %cmp2 = icmp slt i8 %abs, 10
  call void @use(i1 %cmp2)
  %cmp3 = icmp sge i8 %abs, 1
  call void @use(i1 %cmp3)
  %cmp4 = icmp slt i8 %abs, 9
  call void @use(i1 %cmp4)
  ret void
}

; Even if we don't know anything about the input range of the operand,
; we still know something about the result range of abs().
define void @abs2(i8 %x) {
; CHECK-LABEL: @abs2(
; CHECK-NEXT:    [[ABS:%.*]] = call i8 @llvm.abs.i8(i8 [[X:%.*]], i1 true)
; CHECK-NEXT:    call void @use(i1 true)
; CHECK-NEXT:    ret void
;
  %abs = call i8 @llvm.abs.i8(i8 %x, i1 true)
  %cmp = icmp sge i8 %abs, 0
  call void @use(i1 %cmp)
  ret void
}

define void @abs2_vec(<2 x i8> %x) {
; CHECK-LABEL: @abs2_vec(
; CHECK-NEXT:    [[ABS:%.*]] = call <2 x i8> @llvm.abs.v2i8(<2 x i8> [[X:%.*]], i1 true)
; CHECK-NEXT:    [[CMP:%.*]] = icmp sge <2 x i8> [[ABS]], zeroinitializer
; CHECK-NEXT:    call void @use_vec(<2 x i1> [[CMP]])
; CHECK-NEXT:    ret void
;
  %abs = call <2 x i8> @llvm.abs.v2i8(<2 x i8> %x, i1 true)
  %cmp = icmp sge <2 x i8> %abs, zeroinitializer
  call void @use_vec(<2 x i1> %cmp)
  ret void
}

define void @umax1(i8* %p1, i8* %p2) {
; CHECK-LABEL: @umax1(
; CHECK-NEXT:    [[X1:%.*]] = load i8, i8* [[P1:%.*]], align 1, [[RNG1:!range !.*]]
; CHECK-NEXT:    [[X2:%.*]] = load i8, i8* [[P2:%.*]], align 1, [[RNG2:!range !.*]]
; CHECK-NEXT:    [[M:%.*]] = call i8 @llvm.umax.i8(i8 [[X1]], i8 [[X2]])
; CHECK-NEXT:    call void @use(i1 true)
; CHECK-NEXT:    call void @use(i1 true)
; CHECK-NEXT:    [[CMP3:%.*]] = icmp uge i8 [[M]], 6
; CHECK-NEXT:    call void @use(i1 [[CMP3]])
; CHECK-NEXT:    [[CMP4:%.*]] = icmp ult i8 [[M]], 14
; CHECK-NEXT:    call void @use(i1 [[CMP4]])
; CHECK-NEXT:    ret void
;
  %x1 = load i8, i8* %p1, !range !{i8 0, i8 10}
  %x2 = load i8, i8* %p2, !range !{i8 5, i8 15}
  %m = call i8 @llvm.umax.i8(i8 %x1, i8 %x2)
  %cmp1 = icmp uge i8 %m, 5
  call void @use(i1 %cmp1)
  %cmp2 = icmp ult i8 %m, 15
  call void @use(i1 %cmp2)
  %cmp3 = icmp uge i8 %m, 6
  call void @use(i1 %cmp3)
  %cmp4 = icmp ult i8 %m, 14
  call void @use(i1 %cmp4)
  ret void
}

define void @umax2(i8 %x) {
; CHECK-LABEL: @umax2(
; CHECK-NEXT:    [[M:%.*]] = call i8 @llvm.umax.i8(i8 [[X:%.*]], i8 10)
; CHECK-NEXT:    call void @use(i1 true)
; CHECK-NEXT:    ret void
;
  %m = call i8 @llvm.umax.i8(i8 %x, i8 10)
  %cmp = icmp uge i8 %m, 10
  call void @use(i1 %cmp)
  ret void
}
