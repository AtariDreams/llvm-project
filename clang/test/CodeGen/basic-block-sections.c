// REQUIRES: x86-registered-target

// RUN: %clang_cc1 -triple x86_64 -S -o - < %s | FileCheck %s --check-prefix=PLAIN
// RUN: %clang_cc1 -triple x86_64 -S -fbasic-block-sections=all -fbasic-block-sections=none -o - < %s | FileCheck %s --check-prefix=PLAIN

// RUN: %clang_cc1 -triple x86_64 -S -fbasic-block-sections=all -o - < %s | FileCheck %s --check-prefix=BB_WORLD --check-prefix=BB_ALL
// RUN: %clang_cc1 -triple x86_64 -S -fbasic-block-sections=list=%S/Inputs/basic-block-sections.funcnames -o - < %s | FileCheck %s --check-prefix=BB_WORLD --check-prefix=BB_LIST
// RUN: %clang_cc1 -triple x86_64 -S -fbasic-block-sections=all -funique-basic-block-section-names -o - < %s | FileCheck %s --check-prefix=UNIQUE
// RUN: not %clang_cc1 -fbasic-block-sections=list= -emit-obj %s 2>&1 | FileCheck %s --check-prefix=ERROR

int world(int a) {
  if (a > 10)
    return 10;
  else if (a > 5)
    return 5;
  else
    return 0;
}

int another(int a) {
  if (a > 10)
    return 20;
  return 0;
}

// PLAIN-NOT: section
// PLAIN: world:
//
// BB_WORLD: .section .text.world,"ax",@progbits{{$}}
// BB_WORLD: world:
// BB_WORLD: .section .text.world,"ax",@progbits,unique
// BB_WORLD: world.1:
// BB_WORLD: .section .text.another,"ax",@progbits
// BB_ALL: .section .text.another,"ax",@progbits,unique
// BB_ALL: another.1:
// BB_LIST-NOT: .section .text.another,"ax",@progbits,unique
// BB_LIST: another:
// BB_LIST-NOT: another.1:
//
// UNIQUE: .section .text.world.world.1,
// UNIQUE: .section .text.another.another.1,
// ERROR: error:  unable to load basic block sections function list: 'No such file or directory'
