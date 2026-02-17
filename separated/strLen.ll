; ModuleID = 'strLen'
source_filename = "strLen"
target triple = "x86_64-pc-linux-gnu"

define i32 @main(i32 %argc, ptr %argv) {
  ret i32 0
}

define internal i32 @strLen(ptr %0) {
  %lenP = alloca i32, align 4
  store i32 0, ptr %lenP, align 4
  br label %loop

loop:                                             ; preds = %loopInner, %1
  %len.0 = load i32, ptr %lenP, align 4
  %chrP = getelementptr [0 x i8], ptr %strP, i64 0, i32 %len.0
  %chr = load i8, ptr %chrP, align 1
  %loopCond = icmp eq i8 %chr, 0
  br i1 %loopCond, label %ret, label %loopInner

loopInner:                                        ; preds = %loop
  %len.1 = add i32 %len.0, 1
  store i32 %len.1, ptr %lenP, align 4
  br label %loop

ret:                                              ; preds = %loop
  ret i32 %len.0
}
