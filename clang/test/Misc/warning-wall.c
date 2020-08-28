RUN: diagtool tree -Wall > %t 2>&1
RUN: FileCheck --input-file=%t %s

     CHECK:-Wall
CHECK-NEXT:  -Wmost
CHECK-NEXT:    -Wchar-subscripts
CHECK-NEXT:    -Wcomment
CHECK-NEXT:    -Wdelete-non-virtual-dtor
CHECK-NEXT:      -Wdelete-non-abstract-non-virtual-dtor
CHECK-NEXT:      -Wdelete-abstract-non-virtual-dtor
CHECK-NEXT:    -Wformat
CHECK-NEXT:      -Wformat-extra-args
CHECK-NEXT:      -Wformat-zero-length
CHECK-NEXT:      -Wnonnull
CHECK-NEXT:      -Wformat-security
CHECK-NEXT:      -Wformat-y2k
CHECK-NEXT:      -Wformat-invalid-specifier
CHECK-NEXT:    -Wfor-loop-analysis
CHECK-NEXT:    -Wframe-address
CHECK-NEXT:    -Wimplicit
CHECK-NEXT:      -Wimplicit-function-declaration
CHECK-NEXT:      -Wimplicit-int
CHECK-NEXT:    -Winfinite-recursion
CHECK-NEXT:    -Wint-in-bool-context
CHECK-NEXT:    -Wmismatched-tags
CHECK-NEXT:    -Wmissing-braces
CHECK-NEXT:    -Wmove
CHECK-NEXT:      -Wpessimizing-move
CHECK-NEXT:      -Wredundant-move
CHECK-NEXT:      -Wreturn-std-move
CHECK-NEXT:      -Wself-move
CHECK-NEXT:    -Wmultichar
CHECK-NEXT:    -Wrange-loop-construct
CHECK-NEXT:    -Wreorder
CHECK-NEXT:      -Wreorder-ctor
CHECK-NEXT:      -Wreorder-init-list
CHECK-NEXT:    -Wreturn-type
CHECK-NEXT:      -Wreturn-type-c-linkage
CHECK-NEXT:    -Wself-assign
CHECK-NEXT:      -Wself-assign-overloaded
CHECK-NEXT:      -Wself-assign-field
CHECK-NEXT:    -Wself-move
CHECK-NEXT:    -Wsizeof-array-argument
CHECK-NEXT:    -Wsizeof-array-decay
CHECK-NEXT:    -Wstring-plus-int
CHECK-NEXT:    -Wtautological-compare
CHECK-NEXT:      -Wtautological-constant-compare
CHECK-NEXT:        -Wtautological-constant-out-of-range-compare
CHECK-NEXT:      -Wtautological-pointer-compare
CHECK-NEXT:      -Wtautological-overlap-compare
CHECK-NEXT:      -Wtautological-bitwise-compare
CHECK-NEXT:      -Wtautological-undefined-compare
CHECK-NEXT:      -Wtautological-objc-bool-compare
CHECK-NEXT:    -Wtrigraphs
CHECK-NEXT:    -Wuninitialized
CHECK-NEXT:      -Wsometimes-uninitialized
CHECK-NEXT:      -Wstatic-self-init
CHECK-NEXT:      -Wuninitialized-const-reference
CHECK-NEXT:    -Wunknown-pragmas
CHECK-NEXT:    -Wunused
CHECK-NEXT:      -Wunused-argument
CHECK-NEXT:      -Wunused-function
CHECK-NEXT:        -Wunneeded-internal-declaration
CHECK-NEXT:      -Wunused-label
CHECK-NEXT:      -Wunused-private-field
CHECK-NEXT:      -Wunused-lambda-capture
CHECK-NEXT:      -Wunused-local-typedef
CHECK-NEXT:      -Wunused-value
CHECK-NEXT:        -Wunused-comparison
CHECK-NEXT:        -Wunused-result
CHECK-NEXT:        -Wunevaluated-expression
CHECK-NEXT:          -Wpotentially-evaluated-expression
CHECK-NEXT:      -Wunused-variable
CHECK-NEXT:        -Wunused-const-variable
CHECK-NEXT:      -Wunused-property-ivar
CHECK-NEXT:    -Wvolatile-register-var
CHECK-NEXT:    -Wobjc-missing-super-calls
CHECK-NEXT:    -Wobjc-designated-initializers
CHECK-NEXT:    -Wobjc-flexible-array
CHECK-NEXT:    -Woverloaded-virtual
CHECK-NEXT:    -Wprivate-extern
CHECK-NEXT:    -Wcast-of-sel-type
CHECK-NEXT:    -Wextern-c-compat
CHECK-NEXT:    -Wuser-defined-warnings
CHECK-NEXT:  -Wparentheses
CHECK-NEXT:    -Wlogical-op-parentheses
CHECK-NEXT:    -Wlogical-not-parentheses
CHECK-NEXT:    -Wbitwise-conditional-parentheses
CHECK-NEXT:    -Wbitwise-op-parentheses
CHECK-NEXT:    -Wshift-op-parentheses
CHECK-NEXT:    -Woverloaded-shift-op-parentheses
CHECK-NEXT:    -Wparentheses-equality
CHECK-NEXT:    -Wdangling-else
CHECK-NEXT:  -Wswitch
CHECK-NEXT:  -Wswitch-bool
CHECK-NEXT:  -Wmisleading-indentation
CHECK-NEXT:  -Wcompound-token-split
CHECK-NEXT:    -Wcompound-token-split-by-macro
CHECK-NEXT:    -Wcompound-token-split-by-space


CHECK-NOT:-W
