; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=i686-unknown-unknown | FileCheck %s
; RUN: llc < %s -mtriple=x86_64-unknown-unknown | FileCheck %s --check-prefix=CHECK64

; Check reconstructing bswap from shifted masks and tree of ORs

; Match a 32-bit packed halfword bswap. That is
; ((x & 0x000000ff) << 8) |
; ((x & 0x0000ff00) >> 8) |
; ((x & 0x00ff0000) << 8) |
; ((x & 0xff000000) >> 8)
; => (rotl (bswap x), 16)
define i32 @test1(i32 %x) nounwind {
; CHECK-LABEL: test1:
; CHECK:       # %bb.0:
; CHECK-NEXT:    movl {{[0-9]+}}(%esp), %eax
; CHECK-NEXT:    bswapl %eax
; CHECK-NEXT:    roll $16, %eax
; CHECK-NEXT:    retl
;
; CHECK64-LABEL: test1:
; CHECK64:       # %bb.0:
; CHECK64-NEXT:    bswapl %edi
; CHECK64-NEXT:    roll $16, %edi
; CHECK64-NEXT:    movl %edi, %eax
; CHECK64-NEXT:    retq
  %byte0 = and i32 %x, 255        ; 0x000000ff
  %byte1 = and i32 %x, 65280      ; 0x0000ff00
  %byte2 = and i32 %x, 16711680   ; 0x00ff0000
  %byte3 = and i32 %x, 4278190080 ; 0xff000000
  %tmp0 = shl  i32 %byte0, 8
  %tmp1 = lshr i32 %byte1, 8
  %tmp2 = shl  i32 %byte2, 8
  %tmp3 = lshr i32 %byte3, 8
  %or0 = or i32 %tmp0, %tmp1
  %or1 = or i32 %tmp2, %tmp3
  %result = or i32 %or0, %or1
  ret i32 %result
}

; the same as test1, just shifts before the "and"
; ((x << 8) & 0x0000ff00) |
; ((x >> 8) & 0x000000ff) |
; ((x << 8) & 0xff000000) |
; ((x >> 8) & 0x00ff0000)
define i32 @test2(i32 %x) nounwind {
; CHECK-LABEL: test2:
; CHECK:       # %bb.0:
; CHECK-NEXT:    movl {{[0-9]+}}(%esp), %eax
; CHECK-NEXT:    bswapl %eax
; CHECK-NEXT:    roll $16, %eax
; CHECK-NEXT:    retl
;
; CHECK64-LABEL: test2:
; CHECK64:       # %bb.0:
; CHECK64-NEXT:    bswapl %edi
; CHECK64-NEXT:    roll $16, %edi
; CHECK64-NEXT:    movl %edi, %eax
; CHECK64-NEXT:    retq
  %byte1 = shl  i32 %x, 8
  %byte0 = lshr i32 %x, 8
  %byte3 = shl  i32 %x, 8
  %byte2 = lshr i32 %x, 8
  %tmp1 = and i32 %byte1, 65280      ; 0x0000ff00
  %tmp0 = and i32 %byte0, 255        ; 0x000000ff
  %tmp3 = and i32 %byte3, 4278190080 ; 0xff000000
  %tmp2 = and i32 %byte2, 16711680   ; 0x00ff0000
  %or0 = or i32 %tmp0, %tmp1
  %or1 = or i32 %tmp2, %tmp3
  %result = or i32 %or0, %or1
  ret i32 %result
}
