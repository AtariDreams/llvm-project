; RUN: not --crash llc < %s -mtriple powerpc64-ibm-aix-xcoff -mcpu=pwr8 2>&1 | FileCheck %s --check-prefix=DFLTERROR
; RUN: not --crash llc < %s -mtriple powerpc-ibm-aix-xcoff -mcpu=pwr8 2>&1 | FileCheck %s --check-prefix=DFLTERROR

; RUN: not --crash llc < %s -mtriple powerpc64-ibm-aix-xcoff -mcpu=pwr8 -vec-extabi 2>&1 | FileCheck %s --check-prefix=VEXTERROR
; RUN: not --crash llc < %s -mtriple powerpc-ibm-aix-xcoff -mcpu=pwr8 -vec-extabi 2>&1 | FileCheck %s --check-prefix=VEXTERROR

define void @vec_callee(<4 x i32> %vec1) {
    ret void 
}

; DFLTERROR:  LLVM ERROR: the default Altivec AIX ABI is not yet supported
; VEXTERROR:  LLVM ERROR: the extended Altivec AIX ABI is not yet supported
