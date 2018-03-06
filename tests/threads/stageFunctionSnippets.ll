define i32 @_Z6stage0PN4MARC15ThreadSafeQueueIiEE(%"class.MARC::ThreadSafeQueue"*) #0 {
  %2 = alloca %"class.MARC::ThreadSafeQueue"*, align 8
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store %"class.MARC::ThreadSafeQueue"* %0, %"class.MARC::ThreadSafeQueue"** %2, align 8
  store i32 13, i32* %3, align 4
  store i32 0, i32* %4, align 4
  br label %5

; <label>:5:                                      ; preds = %13, %1
  %6 = load i32, i32* %4, align 4
  %7 = icmp slt i32 %6, 5
  br i1 %7, label %8, label %16

; <label>:8:                                      ; preds = %5
  %9 = load i32, i32* %3, align 4
  %10 = add nsw i32 %9, 4
  store i32 %10, i32* %3, align 4
  %11 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %2, align 8
  %12 = load i32, i32* %3, align 4
  call void @_ZN4MARC15ThreadSafeQueueIiE4pushEi(%"class.MARC::ThreadSafeQueue"* %11, i32 %12)
  br label %13

; <label>:13:                                     ; preds = %8
  %14 = load i32, i32* %4, align 4
  %15 = add nsw i32 %14, 1
  store i32 %15, i32* %4, align 4
  br label %5

; <label>:16:                                     ; preds = %5
  %17 = load i32, i32* %3, align 4
  ret i32 %17
}

; Function Attrs: noinline uwtable
define i32 @_Z6stage1PN4MARC15ThreadSafeQueueIiEE(%"class.MARC::ThreadSafeQueue"*) #0 {
  %2 = alloca %"class.MARC::ThreadSafeQueue"*, align 8
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  store %"class.MARC::ThreadSafeQueue"* %0, %"class.MARC::ThreadSafeQueue"** %2, align 8
  store i32 17, i32* %3, align 4
  store i32 0, i32* %5, align 4
  br label %6

; <label>:6:                                      ; preds = %15, %1
  %7 = load i32, i32* %5, align 4
  %8 = icmp slt i32 %7, 5
  br i1 %8, label %9, label %18

; <label>:9:                                      ; preds = %6
  %10 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %2, align 8
  %11 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIiE7waitPopERi(%"class.MARC::ThreadSafeQueue"* %10, i32* dereferenceable(4) %4)
  %12 = load i32, i32* %4, align 4
  %13 = load i32, i32* %3, align 4
  %14 = add nsw i32 %13, %12
  store i32 %14, i32* %3, align 4
  br label %15

; <label>:15:                                     ; preds = %9
  %16 = load i32, i32* %5, align 4
  %17 = add nsw i32 %16, 1
  store i32 %17, i32* %5, align 4
  br label %6

; <label>:18:                                     ; preds = %6
  %19 = load i32, i32* %3, align 4
  ret i32 %19
}


