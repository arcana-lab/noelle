; Function Attrs: noinline norecurse uwtable
define i32 @main(i32, i8**) #4 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i8**, align 8
  %6 = alloca %"class.MARC::ThreadPool", align 8
  %7 = alloca %"class.std::function", align 8
  %8 = alloca i8*
  %9 = alloca i32
  %10 = alloca %"class.MARC::ThreadSafeQueue", align 8
  %11 = alloca %"class.MARC::ThreadSafeQueue"*, align 8
  %12 = alloca i32, align 4
  %13 = alloca %"class.MARC::TaskFuture", align 8
  %14 = alloca i32, align 4
  %15 = alloca %"class.MARC::TaskFuture", align 8
  store i32 0, i32* %3, align 4
  store i32 %0, i32* %4, align 4
  store i8** %1, i8*** %5, align 8
  call void @_ZNSt8functionIFvvEEC2EDn(%"class.std::function"* %7, i8* null) #3
  invoke void @_ZN4MARC10ThreadPoolC2EjSt8functionIFvvEE(%"class.MARC::ThreadPool"* %6, i32 2, %"class.std::function"* %7)
          to label %16 unwind label %37

; <label>:16:                                     ; preds = %2
  call void @_ZNSt8functionIFvvEED2Ev(%"class.std::function"* %7) #3
  invoke void @_ZN4MARC15ThreadSafeQueueIiEC2Ev(%"class.MARC::ThreadSafeQueue"* %10)
          to label %17 unwind label %41

; <label>:17:                                     ; preds = %16
  store %"class.MARC::ThreadSafeQueue"* %10, %"class.MARC::ThreadSafeQueue"** %11, align 8
  invoke void @_ZN4MARC10ThreadPool6submitIRFiPNS_15ThreadSafeQueueIiEEEJRS4_EEEDaOT_DpOT0_(%"class.MARC::TaskFuture"* sret %13, %"class.MARC::ThreadPool"* %6, i32 (%"class.MARC::ThreadSafeQueue"*)* @_Z6stage0PN4MARC15ThreadSafeQueueIiEE, %"class.MARC::ThreadSafeQueue"** dereferenceable(8) %11)
          to label %18 unwind label %45

; <label>:18:                                     ; preds = %17
  %19 = invoke i32 @_ZN4MARC10TaskFutureIiE3getEv(%"class.MARC::TaskFuture"* %13)
          to label %20 unwind label %49

; <label>:20:                                     ; preds = %18
  call void @_ZN4MARC10TaskFutureIiED2Ev(%"class.MARC::TaskFuture"* %13) #3
  store i32 %19, i32* %12, align 4
  invoke void @_ZN4MARC10ThreadPool6submitIRFiPNS_15ThreadSafeQueueIiEEEJRS4_EEEDaOT_DpOT0_(%"class.MARC::TaskFuture"* sret %15, %"class.MARC::ThreadPool"* %6, i32 (%"class.MARC::ThreadSafeQueue"*)* @_Z6stage1PN4MARC15ThreadSafeQueueIiEE, %"class.MARC::ThreadSafeQueue"** dereferenceable(8) %11)
          to label %21 unwind label %45

; <label>:21:                                     ; preds = %20
  %22 = invoke i32 @_ZN4MARC10TaskFutureIiE3getEv(%"class.MARC::TaskFuture"* %15)
          to label %23 unwind label %53

; <label>:23:                                     ; preds = %21
  call void @_ZN4MARC10TaskFutureIiED2Ev(%"class.MARC::TaskFuture"* %15) #3
  store i32 %22, i32* %14, align 4
  %24 = invoke dereferenceable(272) %"class.std::basic_ostream"* @_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(%"class.std::basic_ostream"* dereferenceable(272) @_ZSt4cout, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0))
          to label %25 unwind label %45

; <label>:25:                                     ; preds = %23
  %26 = load i32, i32* %12, align 4
  %27 = invoke dereferenceable(272) %"class.std::basic_ostream"* @_ZNSolsEi(%"class.std::basic_ostream"* %24, i32 %26)
          to label %28 unwind label %45

; <label>:28:                                     ; preds = %25
  %29 = invoke dereferenceable(272) %"class.std::basic_ostream"* @_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(%"class.std::basic_ostream"* dereferenceable(272) %27, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str.1, i32 0, i32 0))
          to label %30 unwind label %45

; <label>:30:                                     ; preds = %28
  %31 = load i32, i32* %14, align 4
  %32 = invoke dereferenceable(272) %"class.std::basic_ostream"* @_ZNSolsEi(%"class.std::basic_ostream"* %29, i32 %31)
          to label %33 unwind label %45

; <label>:33:                                     ; preds = %30
  %34 = invoke dereferenceable(272) %"class.std::basic_ostream"* @_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(%"class.std::basic_ostream"* dereferenceable(272) %32, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str.2, i32 0, i32 0))
          to label %35 unwind label %45

; <label>:35:                                     ; preds = %33
  store i32 0, i32* %3, align 4
  call void @_ZN4MARC15ThreadSafeQueueIiED2Ev(%"class.MARC::ThreadSafeQueue"* %10) #3
  call void @_ZN4MARC10ThreadPoolD2Ev(%"class.MARC::ThreadPool"* %6) #3
  %36 = load i32, i32* %3, align 4
  ret i32 %36

; <label>:37:                                     ; preds = %2
  %38 = landingpad { i8*, i32 }
          cleanup
  %39 = extractvalue { i8*, i32 } %38, 0
  store i8* %39, i8** %8, align 8
  %40 = extractvalue { i8*, i32 } %38, 1
  store i32 %40, i32* %9, align 4
  call void @_ZNSt8functionIFvvEED2Ev(%"class.std::function"* %7) #3
  br label %59

; <label>:41:                                     ; preds = %16
  %42 = landingpad { i8*, i32 }
          cleanup
  %43 = extractvalue { i8*, i32 } %42, 0
  store i8* %43, i8** %8, align 8
  %44 = extractvalue { i8*, i32 } %42, 1
  store i32 %44, i32* %9, align 4
  br label %58

; <label>:45:                                     ; preds = %33, %30, %28, %25, %23, %20, %17
  %46 = landingpad { i8*, i32 }
          cleanup
  %47 = extractvalue { i8*, i32 } %46, 0
  store i8* %47, i8** %8, align 8
  %48 = extractvalue { i8*, i32 } %46, 1
  store i32 %48, i32* %9, align 4
  br label %57

; <label>:49:                                     ; preds = %18
  %50 = landingpad { i8*, i32 }
          cleanup
  %51 = extractvalue { i8*, i32 } %50, 0
  store i8* %51, i8** %8, align 8
  %52 = extractvalue { i8*, i32 } %50, 1
  store i32 %52, i32* %9, align 4
  call void @_ZN4MARC10TaskFutureIiED2Ev(%"class.MARC::TaskFuture"* %13) #3
  br label %57

; <label>:53:                                     ; preds = %21
  %54 = landingpad { i8*, i32 }
          cleanup
  %55 = extractvalue { i8*, i32 } %54, 0
  store i8* %55, i8** %8, align 8
  %56 = extractvalue { i8*, i32 } %54, 1
  store i32 %56, i32* %9, align 4
  call void @_ZN4MARC10TaskFutureIiED2Ev(%"class.MARC::TaskFuture"* %15) #3
  br label %57

; <label>:57:                                     ; preds = %53, %49, %45
  call void @_ZN4MARC15ThreadSafeQueueIiED2Ev(%"class.MARC::ThreadSafeQueue"* %10) #3
  br label %58

; <label>:58:                                     ; preds = %57, %41
  call void @_ZN4MARC10ThreadPoolD2Ev(%"class.MARC::ThreadPool"* %6) #3
  br label %59

; <label>:59:                                     ; preds = %58, %37
  %60 = load i8*, i8** %8, align 8
  %61 = load i32, i32* %9, align 4
  %62 = insertvalue { i8*, i32 } undef, i8* %60, 0
  %63 = insertvalue { i8*, i32 } %62, i32 %61, 1
  resume { i8*, i32 } %63
}

