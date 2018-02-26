; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=i386-unknown -mattr=+sse2 | FileCheck %s

define <2 x i64> @doload64(i16 signext  %x) nounwind  {
; CHECK-LABEL: doload64:
; CHECK:       # %bb.0:
; CHECK-NEXT:    movd {{.*#+}} xmm0 = mem[0],zero,zero,zero
; CHECK-NEXT:    pshuflw {{.*#+}} xmm0 = xmm0[0,0,2,3,4,5,6,7]
; CHECK-NEXT:    pshufd {{.*#+}} xmm0 = xmm0[0,0,0,0]
; CHECK-NEXT:    retl
  %tmp36 = insertelement <8 x i16> undef, i16 %x, i32 0
  %tmp37 = insertelement <8 x i16> %tmp36, i16 %x, i32 1
  %tmp38 = insertelement <8 x i16> %tmp37, i16 %x, i32 2
  %tmp39 = insertelement <8 x i16> %tmp38, i16 %x, i32 3
  %tmp40 = insertelement <8 x i16> %tmp39, i16 %x, i32 4
  %tmp41 = insertelement <8 x i16> %tmp40, i16 %x, i32 5
  %tmp42 = insertelement <8 x i16> %tmp41, i16 %x, i32 6
  %tmp43 = insertelement <8 x i16> %tmp42, i16 %x, i32 7
  %tmp46 = bitcast <8 x i16> %tmp43 to <2 x i64>
  ret <2 x i64> %tmp46
}
