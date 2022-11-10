; ModuleID = '/home/ychen/LLVM/llvm/tools/llvm-mctoll/test/mtest/hello-riscv'
source_filename = "/home/ychen/LLVM/llvm/tools/llvm-mctoll/test/mtest/hello-riscv"

@f = common dso_local global i32 0, align 32
@i = dso_local global i32 1, align 32
@RO-String = private constant [12 x i8] c"hello world\00"

define i32 @main() {
EntryBlock:
  %stack.3 = alloca i32, align 4
  %stack.4 = alloca i32, align 4
  %0 = alloca i1, align 32
  store i1 false, i1* %0, align 1
  %1 = alloca i1, align 32
  store i1 false, i1* %1, align 1
  %2 = alloca i1, align 32
  store i1 false, i1* %2, align 1
  %3 = alloca i1, align 32
  store i1 false, i1* %3, align 1
  %4 = add i32 1, 0
  store i32 %4, i32* %stack.3, align 2
  %5 = ptrtoint i32* @f to i32
  %6 = ptrtoint i32* @i to i32
  store i32 %6, i32* %stack.4, align 2
  %7 = ptrtoint [12 x i8]* @RO-String to i32
  store i32 %6, i32* %stack.3, align 2
  %8 = inttoptr i32 %7 to i8*
  %9 = call i32 @puts(i8* %8)
  %10 = add i32 0, 0
  %11 = add i32 %10, 0
  br label %12

12:                                               ; preds = %EntryBlock
  %13 = phi i32 [ %11, %EntryBlock ]
  ret i32 %13
}

declare dso_local i32 @puts(i8*)
