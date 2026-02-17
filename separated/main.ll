; ModuleID = 'main'
source_filename = "main"
target triple = "x86_64-pc-linux-gnu"

define i32 @main(i32 %argc, ptr %argv) {
  ret i32 0
}

declare internal i32 @strLen(ptr)

define internal i32 @main_2(i32 %0, ptr %1) {
  %errCond = icmp ult i32 %argc, 2
  br i1 %errCond, label %argError1, label %getStr

argError1:                                        ; preds = %2
  tail call void @printStr(ptr @Str1)
  br label %mainRet

getStr:                                           ; preds = %2
  %strPP = getelementptr [2 x ptr], ptr %argv, i64 0, i64 1
  %strP = load ptr, ptr %strPP, align 8
  %strLen = tail call i32 @strLen(ptr noundef %strP)
  tail call void @printlnUInt(i32 noundef %strLen)
  br label %mainRet

mainRet:                                          ; preds = %getStr, %argError1
  ret i32 0
}
