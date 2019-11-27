; RUN: opt < %s -disable-output "-passes=print<ddg>" 2>&1 | FileCheck %s


; CHECK-LABEL: 'DDG' for loop 'test1.for.cond1.preheader':
; CHECK: Node Address:[[N1:0x[0-9a-f]*]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %sub = add i64 %n, -1
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N2:0x[0-9a-f]*]]
; CHECK-NEXT:  [def-use] to [[N3:0x[0-9a-f]*]]

; CHECK: Node Address:[[N3]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %cmp21 = icmp ult i64 1, %sub
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N4:0x[0-9a-f]*]]

; CHECK: Node Address:[[N4]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    br i1 %cmp21, label %for.body4.preheader, label %for.inc12
; CHECK-NEXT: Edges:none!

; CHECK: Node Address:[[N5:0x[0-9a-f]*]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %0 = mul nsw i64 %i.04, %n
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N6:0x[0-9a-f]*]]

; CHECK: Node Address:[[N6]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %arrayidx = getelementptr inbounds float, float* %b, i64 %0
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N7:0x[0-9a-f]*]]

; CHECK: Node Address:[[N7]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %arrayidx5 = getelementptr inbounds float, float* %arrayidx, i64 %j.02
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N8:0x[0-9a-f]*]]

; CHECK: Node Address:[[N8]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %1 = load float, float* %arrayidx5, align 4
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N9:0x[0-9a-f]*]]

; CHECK: Node Address:[[N10:0x[0-9a-f]*]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %2 = mul nsw i64 %i.04, %n
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N11:0x[0-9a-f]*]]

; CHECK: Node Address:[[N11]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %arrayidx6 = getelementptr inbounds float, float* %a, i64 %2
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N12:0x[0-9a-f]*]]

; CHECK: Node Address:[[N13:0x[0-9a-f]*]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %sub7 = add i64 %j.02, -1
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N12]]

; CHECK: Node Address:[[N12]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %arrayidx8 = getelementptr inbounds float, float* %arrayidx6, i64 %sub7
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N9]]

; CHECK: Node Address:[[N14:0x[0-9a-f]*]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %4 = mul nsw i64 %i.04, %n
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N15:0x[0-9a-f]*]]

; CHECK: Node Address:[[N15]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %arrayidx10 = getelementptr inbounds float, float* %a, i64 %4
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N16:0x[0-9a-f]*]]

; CHECK: Node Address:[[N16]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %arrayidx11 = getelementptr inbounds float, float* %arrayidx10, i64 %j.02
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N9]]

; CHECK: Node Address:[[N2]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %cmp2 = icmp ult i64 %inc, %sub
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N17:0x[0-9a-f]*]]

; CHECK: Node Address:[[N17]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    br i1 %cmp2, label %for.body4, label %for.inc12.loopexit
; CHECK-NEXT: Edges:none!

; CHECK: Node Address:[[N18:0x[0-9a-f]*]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %exitcond = icmp ne i64 %inc13, %n
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N19:0x[0-9a-f]*]]

; CHECK: Node Address:[[N19]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    br i1 %exitcond, label %test1.for.cond1.preheader, label %for.end14.loopexit
; CHECK-NEXT: Edges:none!

; CHECK: Node Address:[[N20:0x[0-9a-f]*]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    br label %for.body4
; CHECK-NEXT: Edges:none!

; CHECK: Node Address:[[N21:0x[0-9a-f]*]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    br label %for.inc12
; CHECK-NEXT: Edges:none!

; CHECK: Node Address:[[N9]]:pi-block
; CHECK-NEXT:--- start of nodes in pi-block ---
; CHECK: Node Address:[[N22:0x[0-9a-f]*]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %add = fadd float %1, %3
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N23:0x[0-9a-f]*]]

; CHECK: Node Address:[[N24:0x[0-9a-f]*]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %3 = load float, float* %arrayidx8, align 4
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N22]]

; CHECK: Node Address:[[N23]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    store float %add, float* %arrayidx11, align 4
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [memory] to [[N24]]
; CHECK-NEXT:--- end of nodes in pi-block ---
; CHECK-NEXT: Edges:none!

; CHECK: Node Address:[[N25:0x[0-9a-f]*]]:pi-block
; CHECK-NEXT:--- start of nodes in pi-block ---
; CHECK: Node Address:[[N26:0x[0-9a-f]*]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %inc13 = add i64 %i.04, 1
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N27:0x[0-9a-f]*]]

; CHECK: Node Address:[[N27]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %i.04 = phi i64 [ %inc13, %for.inc12 ], [ 0, %test1.for.cond1.preheader.preheader ]
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N26]]
; CHECK-NEXT:--- end of nodes in pi-block ---
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N5]]
; CHECK-NEXT:  [def-use] to [[N10]]
; CHECK-NEXT:  [def-use] to [[N14]]
; CHECK-NEXT:  [def-use] to [[N18]]

; CHECK: Node Address:[[N28:0x[0-9a-f]*]]:pi-block
; CHECK-NEXT:--- start of nodes in pi-block ---
; CHECK: Node Address:[[N29:0x[0-9a-f]*]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %inc = add i64 %j.02, 1
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N30:0x[0-9a-f]*]]

; CHECK: Node Address:[[N30]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %j.02 = phi i64 [ %inc, %for.body4 ], [ 1, %for.body4.preheader ]
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N29]]
; CHECK-NEXT:--- end of nodes in pi-block ---
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N7]]
; CHECK-NEXT:  [def-use] to [[N13]]
; CHECK-NEXT:  [def-use] to [[N16]]
; CHECK-NEXT:  [def-use] to [[N2]]



;; This test has a cycle.
;; void test1(unsigned long n, float a[][n], float b[][n]) {
;;  for (unsigned long i = 0; i < n; i++)
;;    for (unsigned long j = 1; j < n-1; j++)
;;      a[i][j] = b[i][j] + a[i][j-1];
;; }

define void @test1(i64 %n, float* noalias %a, float* noalias %b) {
entry:
  %exitcond3 = icmp ne i64 0, %n
  br i1 %exitcond3, label %test1.for.cond1.preheader, label %for.end14

test1.for.cond1.preheader:                              ; preds = %entry, %for.inc12
  %i.04 = phi i64 [ %inc13, %for.inc12 ], [ 0, %entry ]
  %sub = add i64 %n, -1
  %cmp21 = icmp ult i64 1, %sub
  br i1 %cmp21, label %for.body4, label %for.inc12

for.body4:                                        ; preds = %test1.for.cond1.preheader, %for.body4
  %j.02 = phi i64 [ %inc, %for.body4 ], [ 1, %test1.for.cond1.preheader ]
  %0 = mul nsw i64 %i.04, %n
  %arrayidx = getelementptr inbounds float, float* %b, i64 %0
  %arrayidx5 = getelementptr inbounds float, float* %arrayidx, i64 %j.02
  %1 = load float, float* %arrayidx5, align 4
  %2 = mul nsw i64 %i.04, %n
  %arrayidx6 = getelementptr inbounds float, float* %a, i64 %2
  %sub7 = add i64 %j.02, -1
  %arrayidx8 = getelementptr inbounds float, float* %arrayidx6, i64 %sub7
  %3 = load float, float* %arrayidx8, align 4
  %add = fadd float %1, %3
  %4 = mul nsw i64 %i.04, %n
  %arrayidx10 = getelementptr inbounds float, float* %a, i64 %4
  %arrayidx11 = getelementptr inbounds float, float* %arrayidx10, i64 %j.02
  store float %add, float* %arrayidx11, align 4
  %inc = add i64 %j.02, 1
  %cmp2 = icmp ult i64 %inc, %sub
  br i1 %cmp2, label %for.body4, label %for.inc12

for.inc12:                                        ; preds = %for.body4, %test1.for.cond1.preheader
  %inc13 = add i64 %i.04, 1
  %exitcond = icmp ne i64 %inc13, %n
  br i1 %exitcond, label %test1.for.cond1.preheader, label %for.end14

for.end14:                                        ; preds = %for.inc12, %entry
  ret void
}



; CHECK-LABEL: 'DDG' for loop 'test2.for.cond1.preheader':
; CHECK: Node Address:[[N1:0x[0-9a-f]*]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %sub = add i64 %n, -1
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N2:0x[0-9a-f]*]]
; CHECK-NEXT:  [def-use] to [[N3:0x[0-9a-f]*]]

; CHECK: Node Address:[[N3]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %cmp21 = icmp ult i64 1, %sub
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N4:0x[0-9a-f]*]]

; CHECK: Node Address:[[N4]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    br i1 %cmp21, label %for.body4.preheader, label %for.inc12
; CHECK-NEXT: Edges:none!

; CHECK: Node Address:[[N5:0x[0-9a-f]*]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %0 = mul nsw i64 %i.04, %n
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N6:0x[0-9a-f]*]]

; CHECK: Node Address:[[N6]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %arrayidx = getelementptr inbounds float, float* %b, i64 %0
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N7:0x[0-9a-f]*]]

; CHECK: Node Address:[[N7]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %arrayidx5 = getelementptr inbounds float, float* %arrayidx, i64 %j.02
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N8:0x[0-9a-f]*]]

; CHECK: Node Address:[[N8]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %1 = load float, float* %arrayidx5, align 4
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N9:0x[0-9a-f]*]]

; CHECK: Node Address:[[N10:0x[0-9a-f]*]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %2 = mul nsw i64 %i.04, %n
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N11:0x[0-9a-f]*]]

; CHECK: Node Address:[[N11]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %arrayidx6 = getelementptr inbounds float, float* %a, i64 %2
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N12:0x[0-9a-f]*]]

; CHECK: Node Address:[[N13:0x[0-9a-f]*]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %add7 = add i64 %j.02, 1
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N12]]

; CHECK: Node Address:[[N12]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %arrayidx8 = getelementptr inbounds float, float* %arrayidx6, i64 %add7
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N14:0x[0-9a-f]*]]

; CHECK: Node Address:[[N14]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %3 = load float, float* %arrayidx8, align 4
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N9]]
; CHECK-NEXT:  [memory] to [[N15:0x[0-9a-f]*]]

; CHECK: Node Address:[[N9]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %add = fadd float %1, %3
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N15]]

; CHECK: Node Address:[[N16:0x[0-9a-f]*]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %4 = mul nsw i64 %i.04, %n
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N17:0x[0-9a-f]*]]

; CHECK: Node Address:[[N17]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %arrayidx10 = getelementptr inbounds float, float* %a, i64 %4
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N18:0x[0-9a-f]*]]

; CHECK: Node Address:[[N18]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %arrayidx11 = getelementptr inbounds float, float* %arrayidx10, i64 %j.02
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N15]]

; CHECK: Node Address:[[N15]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    store float %add, float* %arrayidx11, align 4
; CHECK-NEXT: Edges:none!

; CHECK: Node Address:[[N2]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %cmp2 = icmp ult i64 %inc, %sub
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N19:0x[0-9a-f]*]]

; CHECK: Node Address:[[N19]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    br i1 %cmp2, label %for.body4, label %for.inc12.loopexit
; CHECK-NEXT: Edges:none!

; CHECK: Node Address:[[N20:0x[0-9a-f]*]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %exitcond = icmp ne i64 %inc13, %n
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N21:0x[0-9a-f]*]]

; CHECK: Node Address:[[N21]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    br i1 %exitcond, label %test2.for.cond1.preheader, label %for.end14.loopexit
; CHECK-NEXT: Edges:none!

; CHECK: Node Address:[[N22:0x[0-9a-f]*]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    br label %for.body4
; CHECK-NEXT: Edges:none!

; CHECK: Node Address:[[N23:0x[0-9a-f]*]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    br label %for.inc12
; CHECK-NEXT: Edges:none!

; CHECK: Node Address:[[N24:0x[0-9a-f]*]]:pi-block
; CHECK-NEXT:--- start of nodes in pi-block ---
; CHECK: Node Address:[[N25:0x[0-9a-f]*]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %inc13 = add i64 %i.04, 1
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N26:0x[0-9a-f]*]]

; CHECK: Node Address:[[N26]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %i.04 = phi i64 [ %inc13, %for.inc12 ], [ 0, %test2.for.cond1.preheader.preheader ]
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N25]]
; CHECK-NEXT:--- end of nodes in pi-block ---
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N5]]
; CHECK-NEXT:  [def-use] to [[N10]]
; CHECK-NEXT:  [def-use] to [[N16]]
; CHECK-NEXT:  [def-use] to [[N20]]

; CHECK: Node Address:[[N27:0x[0-9a-f]*]]:pi-block
; CHECK-NEXT:--- start of nodes in pi-block ---
; CHECK: Node Address:[[N28:0x[0-9a-f]*]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %inc = add i64 %j.02, 1
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N29:0x[0-9a-f]*]]

; CHECK: Node Address:[[N29]]:single-instruction
; CHECK-NEXT: Instructions:
; CHECK-NEXT:    %j.02 = phi i64 [ %inc, %for.body4 ], [ 1, %for.body4.preheader ]
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N28]]
; CHECK-NEXT:--- end of nodes in pi-block ---
; CHECK-NEXT: Edges:
; CHECK-NEXT:  [def-use] to [[N7]]
; CHECK-NEXT:  [def-use] to [[N13]]
; CHECK-NEXT:  [def-use] to [[N18]]
; CHECK-NEXT:  [def-use] to [[N2]]


;; This test has no cycles.
;; void test2(unsigned long n, float a[][n], float b[][n]) {
;;  for (unsigned long i = 0; i < n; i++)
;;    for (unsigned long j = 1; j < n-1; j++)
;;      a[i][j] = b[i][j] + a[i][j+1];
;; }

define void @test2(i64 %n, float* noalias %a, float* noalias %b) {
entry:
  %exitcond3 = icmp ne i64 0, %n
  br i1 %exitcond3, label %test2.for.cond1.preheader, label %for.end14

test2.for.cond1.preheader:                              ; preds = %entry, %for.inc12
  %i.04 = phi i64 [ %inc13, %for.inc12 ], [ 0, %entry ]
  %sub = add i64 %n, -1
  %cmp21 = icmp ult i64 1, %sub
  br i1 %cmp21, label %for.body4, label %for.inc12

for.body4:                                        ; preds = %test2.for.cond1.preheader, %for.body4
  %j.02 = phi i64 [ %inc, %for.body4 ], [ 1, %test2.for.cond1.preheader ]
  %0 = mul nsw i64 %i.04, %n
  %arrayidx = getelementptr inbounds float, float* %b, i64 %0
  %arrayidx5 = getelementptr inbounds float, float* %arrayidx, i64 %j.02
  %1 = load float, float* %arrayidx5, align 4
  %2 = mul nsw i64 %i.04, %n
  %arrayidx6 = getelementptr inbounds float, float* %a, i64 %2
  %add7 = add i64 %j.02, 1
  %arrayidx8 = getelementptr inbounds float, float* %arrayidx6, i64 %add7
  %3 = load float, float* %arrayidx8, align 4
  %add = fadd float %1, %3
  %4 = mul nsw i64 %i.04, %n
  %arrayidx10 = getelementptr inbounds float, float* %a, i64 %4
  %arrayidx11 = getelementptr inbounds float, float* %arrayidx10, i64 %j.02
  store float %add, float* %arrayidx11, align 4
  %inc = add i64 %j.02, 1
  %cmp2 = icmp ult i64 %inc, %sub
  br i1 %cmp2, label %for.body4, label %for.inc12

for.inc12:                                        ; preds = %for.body4, %test2.for.cond1.preheader
  %inc13 = add i64 %i.04, 1
  %exitcond = icmp ne i64 %inc13, %n
  br i1 %exitcond, label %test2.for.cond1.preheader, label %for.end14

for.end14:                                        ; preds = %for.inc12, %entry
  ret void
}