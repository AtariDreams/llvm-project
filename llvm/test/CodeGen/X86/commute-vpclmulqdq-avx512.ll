; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=x86_64-unknown -mattr=+vpclmulqdq,+avx512vl | FileCheck %s
; FIXME: actual vpclmulqdq operation should be eliminated

declare <2 x i64> @llvm.x86.pclmulqdq(<2 x i64>, <2 x i64>, i8) nounwind readnone
declare <4 x i64> @llvm.x86.pclmulqdq.256(<4 x i64>, <4 x i64>, i8) nounwind readnone
declare <8 x i64> @llvm.x86.pclmulqdq.512(<8 x i64>, <8 x i64>, i8) nounwind readnone

define <2 x i64> @commute_xmm_v1(<2 x i64> %a0, <2 x i64> %a1) {
; CHECK-LABEL: commute_xmm_v1:
; CHECK:       # %bb.0:
; CHECK-NEXT:    vpclmulqdq $0, %xmm1, %xmm0, %xmm0
; CHECK-NEXT:    vpxor %xmm0, %xmm0, %xmm0
; CHECK-NEXT:    retq
  %1 = call <2 x i64> @llvm.x86.pclmulqdq(<2 x i64> %a0, <2 x i64> %a1, i8 0)
  %2 = call <2 x i64> @llvm.x86.pclmulqdq(<2 x i64> %a1, <2 x i64> %a0, i8 0)
  %3 = xor <2 x i64> %1, %2
  ret <2 x i64> %3
}

define <2 x i64> @commute_xmm_v2(<2 x i64> %a0, <2 x i64> %a1) {
; CHECK-LABEL: commute_xmm_v2:
; CHECK:       # %bb.0:
; CHECK-NEXT:    vpclmulqdq $16, %xmm1, %xmm0, %xmm0
; CHECK-NEXT:    vpxor %xmm0, %xmm0, %xmm0
; CHECK-NEXT:    retq
  %1 = call <2 x i64> @llvm.x86.pclmulqdq(<2 x i64> %a0, <2 x i64> %a1, i8 16)
  %2 = call <2 x i64> @llvm.x86.pclmulqdq(<2 x i64> %a1, <2 x i64> %a0, i8 1)
  %3 = xor <2 x i64> %2, %1
  ret <2 x i64> %3
}

define <2 x i64> @commute_xmm_v3(<2 x i64> %a0, <2 x i64> %a1) {
; CHECK-LABEL: commute_xmm_v3:
; CHECK:       # %bb.0:
; CHECK-NEXT:    vpclmulqdq $17, %xmm1, %xmm0, %xmm0
; CHECK-NEXT:    vpxor %xmm0, %xmm0, %xmm0
; CHECK-NEXT:    retq
  %1 = call <2 x i64> @llvm.x86.pclmulqdq(<2 x i64> %a0, <2 x i64> %a1, i8 17)
  %2 = call <2 x i64> @llvm.x86.pclmulqdq(<2 x i64> %a1, <2 x i64> %a0, i8 17)
  %3 = xor <2 x i64> %2, %1
  ret <2 x i64> %3
}

define <4 x i64> @commute_ymm_v1(<4 x i64> %a0, <4 x i64> %a1) {
; CHECK-LABEL: commute_ymm_v1:
; CHECK:       # %bb.0:
; CHECK-NEXT:    vpclmulqdq $0, %ymm1, %ymm0, %ymm0
; CHECK-NEXT:    vpxor %ymm0, %ymm0, %ymm0
; CHECK-NEXT:    retq
  %1 = call <4 x i64> @llvm.x86.pclmulqdq.256(<4 x i64> %a0, <4 x i64> %a1, i8 0)
  %2 = call <4 x i64> @llvm.x86.pclmulqdq.256(<4 x i64> %a1, <4 x i64> %a0, i8 0)
  %3 = xor <4 x i64> %1, %2
  ret <4 x i64> %3
}

define <4 x i64> @commute_ymm_v2(<4 x i64> %a0, <4 x i64> %a1) {
; CHECK-LABEL: commute_ymm_v2:
; CHECK:       # %bb.0:
; CHECK-NEXT:    vpclmulqdq $16, %ymm1, %ymm0, %ymm0
; CHECK-NEXT:    vpxor %ymm0, %ymm0, %ymm0
; CHECK-NEXT:    retq
  %1 = call <4 x i64> @llvm.x86.pclmulqdq.256(<4 x i64> %a0, <4 x i64> %a1, i8 16)
  %2 = call <4 x i64> @llvm.x86.pclmulqdq.256(<4 x i64> %a1, <4 x i64> %a0, i8 1)
  %3 = xor <4 x i64> %2, %1
  ret <4 x i64> %3
}

define <4 x i64> @commute_ymm_v3(<4 x i64> %a0, <4 x i64> %a1) {
; CHECK-LABEL: commute_ymm_v3:
; CHECK:       # %bb.0:
; CHECK-NEXT:    vpclmulqdq $17, %ymm1, %ymm0, %ymm0
; CHECK-NEXT:    vpxor %ymm0, %ymm0, %ymm0
; CHECK-NEXT:    retq
  %1 = call <4 x i64> @llvm.x86.pclmulqdq.256(<4 x i64> %a0, <4 x i64> %a1, i8 17)
  %2 = call <4 x i64> @llvm.x86.pclmulqdq.256(<4 x i64> %a1, <4 x i64> %a0, i8 17)
  %3 = xor <4 x i64> %2, %1
  ret <4 x i64> %3
}

define <8 x i64> @commute_zmm_v1(<8 x i64> %a0, <8 x i64> %a1) {
; CHECK-LABEL: commute_zmm_v1:
; CHECK:       # %bb.0:
; CHECK-NEXT:    vpclmulqdq $0, %zmm1, %zmm0, %zmm0
; CHECK-NEXT:    vpxorq %zmm0, %zmm0, %zmm0
; CHECK-NEXT:    retq
  %1 = call <8 x i64> @llvm.x86.pclmulqdq.512(<8 x i64> %a0, <8 x i64> %a1, i8 0)
  %2 = call <8 x i64> @llvm.x86.pclmulqdq.512(<8 x i64> %a1, <8 x i64> %a0, i8 0)
  %3 = xor <8 x i64> %1, %2
  ret <8 x i64> %3
}

define <8 x i64> @commute_zmm_v2(<8 x i64> %a0, <8 x i64> %a1) {
; CHECK-LABEL: commute_zmm_v2:
; CHECK:       # %bb.0:
; CHECK-NEXT:    vpclmulqdq $16, %zmm1, %zmm0, %zmm0
; CHECK-NEXT:    vpxorq %zmm0, %zmm0, %zmm0
; CHECK-NEXT:    retq
  %1 = call <8 x i64> @llvm.x86.pclmulqdq.512(<8 x i64> %a0, <8 x i64> %a1, i8 16)
  %2 = call <8 x i64> @llvm.x86.pclmulqdq.512(<8 x i64> %a1, <8 x i64> %a0, i8 1)
  %3 = xor <8 x i64> %2, %1
  ret <8 x i64> %3
}

define <8 x i64> @commute_zmm_v3(<8 x i64> %a0, <8 x i64> %a1) {
; CHECK-LABEL: commute_zmm_v3:
; CHECK:       # %bb.0:
; CHECK-NEXT:    vpclmulqdq $17, %zmm1, %zmm0, %zmm0
; CHECK-NEXT:    vpxorq %zmm0, %zmm0, %zmm0
; CHECK-NEXT:    retq
  %1 = call <8 x i64> @llvm.x86.pclmulqdq.512(<8 x i64> %a0, <8 x i64> %a1, i8 17)
  %2 = call <8 x i64> @llvm.x86.pclmulqdq.512(<8 x i64> %a1, <8 x i64> %a0, i8 17)
  %3 = xor <8 x i64> %2, %1
  ret <8 x i64> %3
}

