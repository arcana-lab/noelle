; ModuleID = 'test_seq.bc'
source_filename = "llvm-link"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.std::ios_base::Init" = type { i8 }
%"class.std::basic_ostream" = type { i32 (...)**, %"class.std::basic_ios" }
%"class.std::basic_ios" = type { %"class.std::ios_base", %"class.std::basic_ostream"*, i8, i8, %"class.std::basic_streambuf"*, %"class.std::ctype"*, %"class.std::num_put"*, %"class.std::num_put"* }
%"class.std::ios_base" = type { i32 (...)**, i64, i64, i32, i32, i32, %"struct.std::ios_base::_Callback_list"*, %"struct.std::ios_base::_Words", [8 x %"struct.std::ios_base::_Words"], i32, %"struct.std::ios_base::_Words"*, %"class.std::locale" }
%"struct.std::ios_base::_Callback_list" = type { %"struct.std::ios_base::_Callback_list"*, void (i32, %"class.std::ios_base"*, i32)*, i32, i32 }
%"struct.std::ios_base::_Words" = type { i8*, i64 }
%"class.std::locale" = type { %"class.std::locale::_Impl"* }
%"class.std::locale::_Impl" = type { i32, %"class.std::locale::facet"**, i64, %"class.std::locale::facet"**, i8** }
%"class.std::locale::facet" = type <{ i32 (...)**, i32, [4 x i8] }>
%"class.std::basic_streambuf" = type { i32 (...)**, i8*, i8*, i8*, i8*, i8*, i8*, %"class.std::locale" }
%"class.std::ctype" = type <{ %"class.std::locale::facet.base", [4 x i8], %struct.__locale_struct*, i8, [7 x i8], i32*, i32*, i16*, i8, [256 x i8], [256 x i8], i8, [6 x i8] }>
%"class.std::locale::facet.base" = type <{ i32 (...)**, i32 }>
%struct.__locale_struct = type { [13 x %struct.__locale_data*], i16*, i32*, i32*, [13 x i8*] }
%struct.__locale_data = type opaque
%"class.std::num_put" = type { %"class.std::locale::facet.base", [4 x i8] }
%"class.std::basic_string" = type { %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider" }
%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider" = type { i8* }
%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Rep" = type { %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Rep_base" }
%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Rep_base" = type { i64, i64, i32 }
%"class.MARC::ThreadSafeQueue" = type { %"struct.std::atomic", %"class.std::mutex", %"class.std::queue", %"class.std::condition_variable", %"class.std::condition_variable" }
%"struct.std::atomic" = type { %"class.std::ios_base::Init" }
%"class.std::mutex" = type { %"class.std::__mutex_base" }
%"class.std::__mutex_base" = type { %union.pthread_mutex_t }
%union.pthread_mutex_t = type { %"struct.(anonymous union)::__pthread_mutex_s" }
%"struct.(anonymous union)::__pthread_mutex_s" = type { i32, i32, i32, i32, i32, i16, i16, %struct.__pthread_internal_list }
%struct.__pthread_internal_list = type { %struct.__pthread_internal_list*, %struct.__pthread_internal_list* }
%"class.std::queue" = type { %"class.std::deque" }
%"class.std::deque" = type { %"class.std::_Deque_base" }
%"class.std::_Deque_base" = type { %"struct.std::_Deque_base<signed char, std::allocator<signed char> >::_Deque_impl" }
%"struct.std::_Deque_base<signed char, std::allocator<signed char> >::_Deque_impl" = type { i8**, i64, %"struct.std::_Deque_iterator", %"struct.std::_Deque_iterator" }
%"struct.std::_Deque_iterator" = type { i8*, i8*, i8*, i8** }
%"class.std::condition_variable" = type { %union.pthread_cond_t }
%union.pthread_cond_t = type { %struct.anon }
%struct.anon = type { i32, i32, i64, i64, i64, i8*, i32, i32 }
%"class.std::unique_lock" = type <{ %"class.std::mutex"*, i8, [7 x i8] }>
%"class.MARC::ThreadSafeQueue.3" = type { %"struct.std::atomic", %"class.std::mutex", %"class.std::queue.4", %"class.std::condition_variable", %"class.std::condition_variable" }
%"class.std::queue.4" = type { %"class.std::deque.5" }
%"class.std::deque.5" = type { %"class.std::_Deque_base.6" }
%"class.std::_Deque_base.6" = type { %"struct.std::_Deque_base<short, std::allocator<short> >::_Deque_impl" }
%"struct.std::_Deque_base<short, std::allocator<short> >::_Deque_impl" = type { i16**, i64, %"struct.std::_Deque_iterator.10", %"struct.std::_Deque_iterator.10" }
%"struct.std::_Deque_iterator.10" = type { i16*, i16*, i16*, i16** }
%"class.MARC::ThreadSafeQueue.11" = type { %"struct.std::atomic", %"class.std::mutex", %"class.std::queue.12", %"class.std::condition_variable", %"class.std::condition_variable" }
%"class.std::queue.12" = type { %"class.std::deque.13" }
%"class.std::deque.13" = type { %"class.std::_Deque_base.14" }
%"class.std::_Deque_base.14" = type { %"struct.std::_Deque_base<int, std::allocator<int> >::_Deque_impl" }
%"struct.std::_Deque_base<int, std::allocator<int> >::_Deque_impl" = type { i32**, i64, %"struct.std::_Deque_iterator.18", %"struct.std::_Deque_iterator.18" }
%"struct.std::_Deque_iterator.18" = type { i32*, i32*, i32*, i32** }
%"class.MARC::ThreadSafeQueue.19" = type { %"struct.std::atomic", %"class.std::mutex", %"class.std::queue.20", %"class.std::condition_variable", %"class.std::condition_variable" }
%"class.std::queue.20" = type { %"class.std::deque.21" }
%"class.std::deque.21" = type { %"class.std::_Deque_base.22" }
%"class.std::_Deque_base.22" = type { %"struct.std::_Deque_base<long, std::allocator<long> >::_Deque_impl" }
%"struct.std::_Deque_base<long, std::allocator<long> >::_Deque_impl" = type { i64**, i64, %"struct.std::_Deque_iterator.26", %"struct.std::_Deque_iterator.26" }
%"struct.std::_Deque_iterator.26" = type { i64*, i64*, i64*, i64** }
%"class.MARC::ThreadPool" = type { %"struct.std::atomic", %"class.MARC::ThreadSafeQueue.32", %"class.std::vector", %"struct.std::atomic"*, %"class.MARC::ThreadSafeQueue.45" }
%"class.MARC::ThreadSafeQueue.32" = type { %"struct.std::atomic", %"class.std::mutex", %"class.std::queue.33", %"class.std::condition_variable", %"class.std::condition_variable" }
%"class.std::queue.33" = type { %"class.std::deque.34" }
%"class.std::deque.34" = type { %"class.std::_Deque_base.35" }
%"class.std::_Deque_base.35" = type { %"struct.std::_Deque_base<std::unique_ptr<MARC::IThreadTask, std::default_delete<MARC::IThreadTask> >, std::allocator<std::unique_ptr<MARC::IThreadTask, std::default_delete<MARC::IThreadTask> > > >::_Deque_impl" }
%"struct.std::_Deque_base<std::unique_ptr<MARC::IThreadTask, std::default_delete<MARC::IThreadTask> >, std::allocator<std::unique_ptr<MARC::IThreadTask, std::default_delete<MARC::IThreadTask> > > >::_Deque_impl" = type { %"class.std::unique_ptr"**, i64, %"struct.std::_Deque_iterator.41", %"struct.std::_Deque_iterator.41" }
%"class.std::unique_ptr" = type { %"class.std::tuple" }
%"class.std::tuple" = type { %"struct.std::_Tuple_impl" }
%"struct.std::_Tuple_impl" = type { %"struct.std::_Head_base.40" }
%"struct.std::_Head_base.40" = type { %"class.MARC::IThreadTask"* }
%"class.MARC::IThreadTask" = type { i32 (...)** }
%"struct.std::_Deque_iterator.41" = type { %"class.std::unique_ptr"*, %"class.std::unique_ptr"*, %"class.std::unique_ptr"*, %"class.std::unique_ptr"** }
%"class.std::vector" = type { %"struct.std::_Vector_base" }
%"struct.std::_Vector_base" = type { %"struct.std::_Vector_base<std::thread, std::allocator<std::thread> >::_Vector_impl" }
%"struct.std::_Vector_base<std::thread, std::allocator<std::thread> >::_Vector_impl" = type { %"class.std::thread"*, %"class.std::thread"*, %"class.std::thread"* }
%"class.std::thread" = type { %"class.std::thread::id" }
%"class.std::thread::id" = type { i64 }
%"class.MARC::ThreadSafeQueue.45" = type { %"struct.std::atomic", %"class.std::mutex", %"class.std::queue.46", %"class.std::condition_variable", %"class.std::condition_variable" }
%"class.std::queue.46" = type { %"class.std::deque.47" }
%"class.std::deque.47" = type { %"class.std::_Deque_base.48" }
%"class.std::_Deque_base.48" = type { %"struct.std::_Deque_base<std::function<void ()>, std::allocator<std::function<void ()> > >::_Deque_impl" }
%"struct.std::_Deque_base<std::function<void ()>, std::allocator<std::function<void ()> > >::_Deque_impl" = type { %"class.std::function"**, i64, %"struct.std::_Deque_iterator.52", %"struct.std::_Deque_iterator.52" }
%"class.std::function" = type { %"class.std::_Function_base", void (%"union.std::_Any_data"*)* }
%"class.std::_Function_base" = type { %"union.std::_Any_data", i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* }
%"union.std::_Any_data" = type { %"union.std::_Nocopy_types" }
%"union.std::_Nocopy_types" = type { { i64, i64 } }
%"struct.std::_Deque_iterator.52" = type { %"class.std::function"*, %"class.std::function"*, %"class.std::function"*, %"class.std::function"** }
%"class.std::vector.53" = type { %"struct.std::_Vector_base.54" }
%"struct.std::_Vector_base.54" = type { %"struct.std::_Vector_base<MARC::TaskFuture<void>, std::allocator<MARC::TaskFuture<void> > >::_Vector_impl" }
%"struct.std::_Vector_base<MARC::TaskFuture<void>, std::allocator<MARC::TaskFuture<void> > >::_Vector_impl" = type { %"class.MARC::TaskFuture"*, %"class.MARC::TaskFuture"*, %"class.MARC::TaskFuture"* }
%"class.MARC::TaskFuture" = type { %"class.std::future" }
%"class.std::future" = type { %"class.std::__basic_future" }
%"class.std::__basic_future" = type { %"class.std::shared_ptr" }
%"class.std::shared_ptr" = type { %"class.std::__shared_ptr" }
%"class.std::__shared_ptr" = type { %"class.std::__future_base::_State_baseV2"*, %"class.std::__shared_count" }
%"class.std::__future_base::_State_baseV2" = type <{ i32 (...)**, %"class.std::unique_ptr.58", %"class.std::__atomic_futex_unsigned", %"struct.std::atomic", [3 x i8], %"struct.std::__atomic_base.65", [4 x i8] }>
%"class.std::unique_ptr.58" = type { %"class.std::tuple.59" }
%"class.std::tuple.59" = type { %"struct.std::_Tuple_impl.60" }
%"struct.std::_Tuple_impl.60" = type { %"struct.std::_Head_base.63" }
%"struct.std::_Head_base.63" = type { %"struct.std::__future_base::_Result_base"* }
%"struct.std::__future_base::_Result_base" = type { i32 (...)**, %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider" }
%"class.std::__atomic_futex_unsigned" = type { %"struct.std::atomic.64" }
%"struct.std::atomic.64" = type { %"struct.std::__atomic_base.65" }
%"struct.std::__atomic_base.65" = type { i32 }
%"class.std::__shared_count" = type { %"class.std::_Sp_counted_base"* }
%"class.std::_Sp_counted_base" = type { i32 (...)**, i32, i32 }
%"class.std::shared_ptr.121" = type { %"class.std::__shared_ptr.122" }
%"class.std::__shared_ptr.122" = type { %"struct.std::__future_base::_Task_state"*, %"class.std::__shared_count" }
%"struct.std::__future_base::_Task_state" = type { %"struct.std::__future_base::_Task_state_base", %"struct.std::__future_base::_Task_state<std::_Bind<void (*(void *, void *))(void *, void *)>, std::allocator<int>, void ()>::_Impl" }
%"struct.std::__future_base::_Task_state_base" = type { %"class.std::__future_base::_State_baseV2.base", %"class.std::unique_ptr.109" }
%"class.std::__future_base::_State_baseV2.base" = type <{ i32 (...)**, %"class.std::unique_ptr.58", %"class.std::__atomic_futex_unsigned", %"struct.std::atomic", [3 x i8], %"struct.std::__atomic_base.65" }>
%"class.std::unique_ptr.109" = type { %"class.std::tuple.110" }
%"class.std::tuple.110" = type { %"struct.std::_Tuple_impl.111" }
%"struct.std::_Tuple_impl.111" = type { %"struct.std::_Head_base.112" }
%"struct.std::_Head_base.112" = type { %"struct.std::__future_base::_Result"* }
%"struct.std::__future_base::_Result" = type { %"struct.std::__future_base::_Result_base" }
%"struct.std::__future_base::_Task_state<std::_Bind<void (*(void *, void *))(void *, void *)>, std::allocator<int>, void ()>::_Impl" = type { %"class.std::_Bind" }
%"class.std::_Bind" = type { void (i8*, i8*)*, %"class.std::tuple.102" }
%"class.std::tuple.102" = type { %"struct.std::_Tuple_impl.103" }
%"struct.std::_Tuple_impl.103" = type { %"class.std::basic_string", %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider" }
%"class.std::packaged_task" = type { %"class.std::shared_ptr.107" }
%"class.std::shared_ptr.107" = type { %"class.std::__shared_ptr.108" }
%"class.std::__shared_ptr.108" = type { %"struct.std::__future_base::_Task_state_base"*, %"class.std::__shared_count" }
%"class.std::type_info" = type { i32 (...)**, i8* }
%"class.std::future_error" = type { %"class.std::logic_error", %"struct.std::error_code" }
%"class.std::logic_error" = type { %"class.MARC::IThreadTask", %"class.std::basic_string" }
%"struct.std::error_code" = type { i32, %"class.MARC::IThreadTask"* }
%"class.MARC::ThreadTask" = type { %"class.MARC::IThreadTask", %"class.std::packaged_task" }
%class.anon.135 = type { { i64, i64 }*, %"class.std::__future_base::_State_baseV2"**, %"class.std::function.133"**, i8** }
%"class.std::function.133" = type { %"class.std::_Function_base", void (%"class.std::unique_ptr.58"*, %"union.std::_Any_data"*)* }
%"struct.std::_Bind_simple.127" = type { %"class.std::tuple.128" }
%"class.std::tuple.128" = type { %"struct.std::_Tuple_impl.129" }
%"struct.std::_Tuple_impl.129" = type { %"struct.std::_Head_base.130" }
%"struct.std::_Head_base.130" = type { %"class.std::reference_wrapper" }
%"class.std::reference_wrapper" = type { %"class.std::_Bind"* }
%"class.std::_Sp_counted_ptr_inplace" = type { %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_ptr_inplace<std::__future_base::_Task_state<std::_Bind<void (*(void *, void *))(void *, void *)>, std::allocator<int>, void ()>, std::allocator<int>, __gnu_cxx::_Lock_policy::_S_atomic>::_Impl" }
%"class.std::_Sp_counted_ptr_inplace<std::__future_base::_Task_state<std::_Bind<void (*(void *, void *))(void *, void *)>, std::allocator<int>, void ()>, std::allocator<int>, __gnu_cxx::_Lock_policy::_S_atomic>::_Impl" = type { %"struct.__gnu_cxx::__aligned_buffer" }
%"struct.__gnu_cxx::__aligned_buffer" = type { %"union.std::aligned_storage<64, 8>::type" }
%"union.std::aligned_storage<64, 8>::type" = type { [64 x i8] }
%"struct.std::__future_base::_Task_setter" = type { %"class.std::unique_ptr.109"*, %"struct.std::_Bind_simple.127"* }
%"struct.std::__future_base::_State_baseV2::_Make_ready" = type { %"struct.std::__at_thread_exit_elt", %"class.std::shared_ptr" }
%"struct.std::__at_thread_exit_elt" = type { %"struct.std::__at_thread_exit_elt"*, void (i8*)* }
%union.pthread_attr_t = type { i64, [48 x i8] }
%"struct.std::thread::_State_impl" = type { %"class.MARC::IThreadTask", %"struct.std::_Bind_simple" }
%"struct.std::_Bind_simple" = type { %"class.std::tuple.92" }
%"class.std::tuple.92" = type { %"struct.std::_Tuple_impl.93" }
%"struct.std::_Tuple_impl.93" = type { %"struct.std::_Tuple_impl.94", %"struct.std::_Head_base.98" }
%"struct.std::_Tuple_impl.94" = type { %"struct.std::_Tuple_impl.95", %"struct.std::_Head_base.97" }
%"struct.std::_Tuple_impl.95" = type { %"struct.std::_Head_base.96" }
%"struct.std::_Head_base.96" = type { %"struct.std::atomic"* }
%"struct.std::_Head_base.97" = type { %"class.MARC::ThreadPool"* }
%"struct.std::_Head_base.98" = type { %"union.std::_Any_data" }

$_ZSt4sqrtf = comdat any

$_ZN4MARC15ThreadSafeQueueIaE4pushEa = comdat any

$_ZNSt5dequeIaSaIaEE17_M_reallocate_mapEmb = comdat any

$_ZN4MARC15ThreadSafeQueueIaE7waitPopERa = comdat any

$_ZN4MARC15ThreadSafeQueueIsE4pushEs = comdat any

$_ZNSt5dequeIsSaIsEE17_M_reallocate_mapEmb = comdat any

$_ZN4MARC15ThreadSafeQueueIsE7waitPopERs = comdat any

$_ZN4MARC15ThreadSafeQueueIiE4pushEi = comdat any

$_ZNSt5dequeIiSaIiEE17_M_reallocate_mapEmb = comdat any

$_ZN4MARC15ThreadSafeQueueIiE7waitPopERi = comdat any

$_ZN4MARC15ThreadSafeQueueIlE4pushEl = comdat any

$_ZNSt5dequeIlSaIlEE17_M_reallocate_mapEmb = comdat any

$_ZN4MARC15ThreadSafeQueueIlE7waitPopERl = comdat any

$_ZN4MARC15ThreadSafeQueueIcE4pushEc = comdat any

$_ZNSt5dequeIcSaIcEE17_M_reallocate_mapEmb = comdat any

$_ZN4MARC15ThreadSafeQueueIcE7waitPopERc = comdat any

$_ZN4MARC10ThreadPoolC2EjSt8functionIFvvEE = comdat any

$_ZN4MARC15ThreadSafeQueueIaEC2Ev = comdat any

$_ZN4MARC15ThreadSafeQueueIsEC2Ev = comdat any

$_ZN4MARC15ThreadSafeQueueIiEC2Ev = comdat any

$_ZN4MARC15ThreadSafeQueueIlEC2Ev = comdat any

$__clang_call_terminate = comdat any

$_ZN4MARC10ThreadPool6submitIRPFvPvS2_EJRS2_S2_EEEDaOT_DpOT0_ = comdat any

$_ZNSt6vectorIN4MARC10TaskFutureIvEESaIS2_EE19_M_emplace_back_auxIJS2_EEEvDpOT_ = comdat any

$_ZNSt6futureIvE3getEv = comdat any

$_ZNSt14__basic_futureIvED2Ev = comdat any

$_ZNSt6vectorIN4MARC10TaskFutureIvEESaIS2_EED2Ev = comdat any

$_ZN4MARC10ThreadPoolD2Ev = comdat any

$_ZN4MARC15ThreadSafeQueueIaED2Ev = comdat any

$_ZN4MARC15ThreadSafeQueueIsED2Ev = comdat any

$_ZN4MARC15ThreadSafeQueueIiED2Ev = comdat any

$_ZN4MARC15ThreadSafeQueueIlED2Ev = comdat any

$_ZNSt5queueIlSt5dequeIlSaIlEEED2Ev = comdat any

$_ZNSt5queueIiSt5dequeIiSaIiEEED2Ev = comdat any

$_ZNSt5queueIsSt5dequeIsSaIsEEED2Ev = comdat any

$_ZNSt5queueIaSt5dequeIaSaIaEEED2Ev = comdat any

$_ZN4MARC10ThreadPool7destroyEv = comdat any

$_ZN4MARC15ThreadSafeQueueISt8functionIFvvEEED2Ev = comdat any

$_ZN4MARC15ThreadSafeQueueISt10unique_ptrINS_11IThreadTaskESt14default_deleteIS2_EEED2Ev = comdat any

$_ZNSt6vectorISt6threadSaIS0_EED2Ev = comdat any

$_ZNSt5dequeISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EESaIS5_EED2Ev = comdat any

$_ZNSt5dequeISt8functionIFvvEESaIS2_EED2Ev = comdat any

$_ZNSt5dequeISt8functionIFvvEESaIS2_EE19_M_destroy_data_auxESt15_Deque_iteratorIS2_RS2_PS2_ES8_ = comdat any

$_ZNSt11_Deque_baseISt8functionIFvvEESaIS2_EED2Ev = comdat any

$_ZN4MARC15ThreadSafeQueueISt8functionIFvvEEE7waitPopERS3_ = comdat any

$_ZN4MARC15ThreadSafeQueueISt8functionIFvvEEE12internal_popERS3_ = comdat any

$_ZNKSt14__basic_futureIvE13_M_get_resultEv = comdat any

$_ZSt32__enable_shared_from_this_helperILN9__gnu_cxx12_Lock_policyE2EEvRKSt14__shared_countIXT_EEz = comdat any

$_ZNSt13packaged_taskIFvvEE10get_futureEv = comdat any

$_ZNSt5dequeISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EESaIS5_EE17_M_reallocate_mapEmb = comdat any

$_ZNSt13packaged_taskIFvvEED2Ev = comdat any

$_ZNSt13__future_base13_State_baseV216_M_break_promiseESt10unique_ptrINS_12_Result_baseENS2_8_DeleterEE = comdat any

$_ZNSt10unique_ptrINSt13__future_base12_Result_baseENS1_8_DeleterEED2Ev = comdat any

$_ZNSt12__shared_ptrINSt13__future_base16_Task_state_baseIFvvEEELN9__gnu_cxx12_Lock_policyE2EED2Ev = comdat any

$_ZNSt12future_errorC2ESt10error_code = comdat any

$_ZSt18make_exception_ptrISt12future_errorENSt15__exception_ptr13exception_ptrET_ = comdat any

$_ZN4MARC10ThreadTaskISt13packaged_taskIFvvEEE7executeEv = comdat any

$_ZN4MARC10ThreadTaskISt13packaged_taskIFvvEEED2Ev = comdat any

$_ZN4MARC10ThreadTaskISt13packaged_taskIFvvEEED0Ev = comdat any

$_ZNSt13__future_base11_Task_stateISt5_BindIFPFvPvS2_ES2_S2_EESaIiEFvvEED2Ev = comdat any

$_ZNSt13__future_base11_Task_stateISt5_BindIFPFvPvS2_ES2_S2_EESaIiEFvvEED0Ev = comdat any

$_ZNSt13__future_base13_State_baseV217_M_complete_asyncEv = comdat any

$_ZNKSt13__future_base13_State_baseV221_M_is_deferred_futureEv = comdat any

$_ZNSt13__future_base11_Task_stateISt5_BindIFPFvPvS2_ES2_S2_EESaIiEFvvEE6_M_runEv = comdat any

$_ZNSt13__future_base11_Task_stateISt5_BindIFPFvPvS2_ES2_S2_EESaIiEFvvEE14_M_run_delayedESt8weak_ptrINS_13_State_baseV2EE = comdat any

$_ZNSt13__future_base11_Task_stateISt5_BindIFPFvPvS2_ES2_S2_EESaIiEFvvEE8_M_resetEv = comdat any

$_ZNSt23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EEC2IJS7_RKS8_EEES8_DpOT_ = comdat any

$_ZNSt17_Function_handlerIFSt10unique_ptrINSt13__future_base12_Result_baseENS2_8_DeleterEEvENS1_12_Task_setterIS0_INS1_7_ResultIvEES3_ESt12_Bind_simpleIFSt17reference_wrapperISt5_BindIFPFvPvSD_ESD_SD_EEEvEEvEEE9_M_invokeERKSt9_Any_data = comdat any

$_ZNSt14_Function_base13_Base_managerINSt13__future_base12_Task_setterISt10unique_ptrINS1_7_ResultIvEENS1_12_Result_base8_DeleterEESt12_Bind_simpleIFSt17reference_wrapperISt5_BindIFPFvPvSC_ESC_SC_EEEvEEvEEE10_M_managerERSt9_Any_dataRKSM_St18_Manager_operation = comdat any

$_ZNSt13__future_base13_State_baseV221_M_set_delayed_resultESt8functionIFSt10unique_ptrINS_12_Result_baseENS3_8_DeleterEEvEESt8weak_ptrIS0_E = comdat any

$_ZNSt13__future_base13_State_baseV29_M_do_setEPSt8functionIFSt10unique_ptrINS_12_Result_baseENS3_8_DeleterEEvEEPb = comdat any

$_ZZSt9call_onceIMNSt13__future_base13_State_baseV2EFvPSt8functionIFSt10unique_ptrINS0_12_Result_baseENS4_8_DeleterEEvEEPbEJPS1_S9_SA_EEvRSt9once_flagOT_DpOT0_ENUlvE0_8__invokeEv = comdat any

$_ZNKSt13__future_base12_Task_setterISt10unique_ptrINS_7_ResultIvEENS_12_Result_base8_DeleterEESt12_Bind_simpleIFSt17reference_wrapperISt5_BindIFPFvPvSA_ESA_SA_EEEvEEvEclEv = comdat any

$_ZNSt13__future_base7_ResultIvE10_M_destroyEv = comdat any

$_ZNSt13__future_base7_ResultIvED0Ev = comdat any

$_ZNSt13__future_base13_State_baseV2D2Ev = comdat any

$_ZNSt13__future_base13_State_baseV2D0Ev = comdat any

$_ZNSt13__future_base16_Task_state_baseIFvvEED2Ev = comdat any

$_ZNSt13__future_base16_Task_state_baseIFvvEED0Ev = comdat any

$_ZNSt23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EED2Ev = comdat any

$_ZNSt23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EED0Ev = comdat any

$_ZNSt23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EE10_M_disposeEv = comdat any

$_ZNSt23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EE10_M_destroyEv = comdat any

$_ZNSt23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EE14_M_get_deleterERKSt9type_info = comdat any

$_ZNSt11_Deque_baseIlSaIlEE17_M_initialize_mapEm = comdat any

$_ZNSt11_Deque_baseIiSaIiEE17_M_initialize_mapEm = comdat any

$_ZNSt11_Deque_baseIsSaIsEE17_M_initialize_mapEm = comdat any

$_ZNSt11_Deque_baseIaSaIaEE17_M_initialize_mapEm = comdat any

$_ZN4MARC15ThreadSafeQueueISt10unique_ptrINS_11IThreadTaskESt14default_deleteIS2_EEEC2Ev = comdat any

$_ZN4MARC15ThreadSafeQueueISt8functionIFvvEEEC2Ev = comdat any

$_ZN4MARC10ThreadPool6workerEPSt6atomicIbE = comdat any

$_ZNSt6vectorISt6threadSaIS0_EE12emplace_backIJMN4MARC10ThreadPoolEFvPSt6atomicIbEEPS5_S8_EEEvDpOT_ = comdat any

$_ZN4MARC15ThreadSafeQueueISt8functionIFvvEEE4pushES3_ = comdat any

$_ZNSt5dequeISt8functionIFvvEESaIS2_EE17_M_reallocate_mapEmb = comdat any

$_ZNSt6vectorISt6threadSaIS0_EE19_M_emplace_back_auxIJMN4MARC10ThreadPoolEFvPSt6atomicIbEEPS5_S8_EEEvDpOT_ = comdat any

$_ZNSt6thread11_State_implISt12_Bind_simpleIFSt7_Mem_fnIMN4MARC10ThreadPoolEFvPSt6atomicIbEEEPS4_S7_EEED0Ev = comdat any

$_ZNSt6thread11_State_implISt12_Bind_simpleIFSt7_Mem_fnIMN4MARC10ThreadPoolEFvPSt6atomicIbEEEPS4_S7_EEE6_M_runEv = comdat any

$_ZN4MARC15ThreadSafeQueueISt10unique_ptrINS_11IThreadTaskESt14default_deleteIS2_EEE7waitPopERS5_ = comdat any

$_ZNSt11_Deque_baseISt8functionIFvvEESaIS2_EE17_M_initialize_mapEm = comdat any

$_ZNSt11_Deque_baseISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EESaIS5_EE17_M_initialize_mapEm = comdat any

$_ZTVSt23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EE = comdat any

$_ZTVNSt13__future_base16_Task_state_baseIFvvEEE = comdat any

$_ZTVNSt13__future_base13_State_baseV2E = comdat any

$_ZTVNSt13__future_base7_ResultIvEE = comdat any

$_ZTVNSt13__future_base11_Task_stateISt5_BindIFPFvPvS2_ES2_S2_EESaIiEFvvEEE = comdat any

$_ZTISt19_Sp_make_shared_tag = comdat any

$_ZTVN4MARC10ThreadTaskISt13packaged_taskIFvvEEEE = comdat any

$_ZTIN4MARC10ThreadTaskISt13packaged_taskIFvvEEEE = comdat any

$_ZTSN4MARC10ThreadTaskISt13packaged_taskIFvvEEEE = comdat any

$_ZTIN4MARC11IThreadTaskE = comdat any

$_ZTSN4MARC11IThreadTaskE = comdat any

$_ZTSSt19_Sp_make_shared_tag = comdat any

$_ZTINSt13__future_base11_Task_stateISt5_BindIFPFvPvS2_ES2_S2_EESaIiEFvvEEE = comdat any

$_ZTINSt13__future_base12_Task_setterISt10unique_ptrINS_7_ResultIvEENS_12_Result_base8_DeleterEESt12_Bind_simpleIFSt17reference_wrapperISt5_BindIFPFvPvSA_ESA_SA_EEEvEEvEE = comdat any

$_ZTSNSt13__future_base12_Task_setterISt10unique_ptrINS_7_ResultIvEENS_12_Result_base8_DeleterEESt12_Bind_simpleIFSt17reference_wrapperISt5_BindIFPFvPvSA_ESA_SA_EEEvEEvEE = comdat any

$_ZTSNSt13__future_base11_Task_stateISt5_BindIFPFvPvS2_ES2_S2_EESaIiEFvvEEE = comdat any

$_ZTINSt13__future_base16_Task_state_baseIFvvEEE = comdat any

$_ZTSNSt13__future_base16_Task_state_baseIFvvEEE = comdat any

$_ZTINSt13__future_base13_State_baseV2E = comdat any

$_ZTSNSt13__future_base13_State_baseV2E = comdat any

$_ZTINSt13__future_base7_ResultIvEE = comdat any

$_ZTSNSt13__future_base7_ResultIvEE = comdat any

$_ZTISt23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EE = comdat any

$_ZTSSt23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EE = comdat any

$_ZTISt16_Sp_counted_baseILN9__gnu_cxx12_Lock_policyE2EE = comdat any

$_ZTSSt16_Sp_counted_baseILN9__gnu_cxx12_Lock_policyE2EE = comdat any

$_ZTISt11_Mutex_baseILN9__gnu_cxx12_Lock_policyE2EE = comdat any

$_ZTSSt11_Mutex_baseILN9__gnu_cxx12_Lock_policyE2EE = comdat any

$_ZTVNSt6thread11_State_implISt12_Bind_simpleIFSt7_Mem_fnIMN4MARC10ThreadPoolEFvPSt6atomicIbEEEPS4_S7_EEEE = comdat any

$_ZTINSt6thread11_State_implISt12_Bind_simpleIFSt7_Mem_fnIMN4MARC10ThreadPoolEFvPSt6atomicIbEEEPS4_S7_EEEE = comdat any

$_ZTSNSt6thread11_State_implISt12_Bind_simpleIFSt7_Mem_fnIMN4MARC10ThreadPoolEFvPSt6atomicIbEEEPS4_S7_EEEE = comdat any

@.str = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @_GLOBAL__sub_I_DSWP_utils.cpp, i8* null }]
@_ZStL8__ioinit = internal global %"class.std::ios_base::Init" zeroinitializer, align 1
@__dso_handle = external hidden global i8
@_ZNSs4_Rep20_S_empty_rep_storageE = external global [0 x i64], align 8
@.str.1 = private unnamed_addr constant [10 x i8] c"Reached: \00", align 1
@.str.2 = private unnamed_addr constant [13 x i8] c"Reached: %d\0A\00", align 1
@str = private unnamed_addr constant [13 x i8] c"Spurious pop\00"
@_ZSt4cerr = external global %"class.std::basic_ostream", align 8
@.str.4 = private unnamed_addr constant [23 x i8] c"QUEUE SIZE INCORRECT!\0A\00", align 1
@_ZTVSt23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EE = linkonce_odr unnamed_addr constant { [7 x i8*] } { [7 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTISt23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EE to i8*), i8* bitcast (void (%"class.std::_Sp_counted_ptr_inplace"*)* @_ZNSt23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EED2Ev to i8*), i8* bitcast (void (%"class.std::_Sp_counted_ptr_inplace"*)* @_ZNSt23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EED0Ev to i8*), i8* bitcast (void (%"class.std::_Sp_counted_ptr_inplace"*)* @_ZNSt23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EE10_M_disposeEv to i8*), i8* bitcast (void (%"class.std::_Sp_counted_ptr_inplace"*)* @_ZNSt23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EE10_M_destroyEv to i8*), i8* bitcast (i8* (%"class.std::_Sp_counted_ptr_inplace"*, %"class.std::type_info"*)* @_ZNSt23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EE14_M_get_deleterERKSt9type_info to i8*)] }, comdat, align 8
@_ZTVNSt13__future_base16_Task_state_baseIFvvEEE = linkonce_odr unnamed_addr constant { [9 x i8*] } { [9 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTINSt13__future_base16_Task_state_baseIFvvEEE to i8*), i8* bitcast (void (%"struct.std::__future_base::_Task_state_base"*)* @_ZNSt13__future_base16_Task_state_baseIFvvEED2Ev to i8*), i8* bitcast (void (%"struct.std::__future_base::_Task_state_base"*)* @_ZNSt13__future_base16_Task_state_baseIFvvEED0Ev to i8*), i8* bitcast (void (%"class.std::__future_base::_State_baseV2"*)* @_ZNSt13__future_base13_State_baseV217_M_complete_asyncEv to i8*), i8* bitcast (i1 (%"class.std::__future_base::_State_baseV2"*)* @_ZNKSt13__future_base13_State_baseV221_M_is_deferred_futureEv to i8*), i8* bitcast (void ()* @__cxa_pure_virtual to i8*), i8* bitcast (void ()* @__cxa_pure_virtual to i8*), i8* bitcast (void ()* @__cxa_pure_virtual to i8*)] }, comdat, align 8
@_ZTVNSt13__future_base13_State_baseV2E = linkonce_odr unnamed_addr constant { [6 x i8*] } { [6 x i8*] [i8* null, i8* bitcast ({ i8*, i8* }* @_ZTINSt13__future_base13_State_baseV2E to i8*), i8* bitcast (void (%"class.std::__future_base::_State_baseV2"*)* @_ZNSt13__future_base13_State_baseV2D2Ev to i8*), i8* bitcast (void (%"class.std::__future_base::_State_baseV2"*)* @_ZNSt13__future_base13_State_baseV2D0Ev to i8*), i8* bitcast (void (%"class.std::__future_base::_State_baseV2"*)* @_ZNSt13__future_base13_State_baseV217_M_complete_asyncEv to i8*), i8* bitcast (i1 (%"class.std::__future_base::_State_baseV2"*)* @_ZNKSt13__future_base13_State_baseV221_M_is_deferred_futureEv to i8*)] }, comdat, align 8
@_ZTVNSt13__future_base7_ResultIvEE = linkonce_odr unnamed_addr constant { [5 x i8*] } { [5 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTINSt13__future_base7_ResultIvEE to i8*), i8* bitcast (void (%"struct.std::__future_base::_Result"*)* @_ZNSt13__future_base7_ResultIvE10_M_destroyEv to i8*), i8* bitcast (void (%"struct.std::__future_base::_Result_base"*)* @_ZNSt13__future_base12_Result_baseD2Ev to i8*), i8* bitcast (void (%"struct.std::__future_base::_Result"*)* @_ZNSt13__future_base7_ResultIvED0Ev to i8*)] }, comdat, align 8
@_ZTVNSt13__future_base11_Task_stateISt5_BindIFPFvPvS2_ES2_S2_EESaIiEFvvEEE = linkonce_odr unnamed_addr constant { [9 x i8*] } { [9 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTINSt13__future_base11_Task_stateISt5_BindIFPFvPvS2_ES2_S2_EESaIiEFvvEEE to i8*), i8* bitcast (void (%"struct.std::__future_base::_Task_state"*)* @_ZNSt13__future_base11_Task_stateISt5_BindIFPFvPvS2_ES2_S2_EESaIiEFvvEED2Ev to i8*), i8* bitcast (void (%"struct.std::__future_base::_Task_state"*)* @_ZNSt13__future_base11_Task_stateISt5_BindIFPFvPvS2_ES2_S2_EESaIiEFvvEED0Ev to i8*), i8* bitcast (void (%"class.std::__future_base::_State_baseV2"*)* @_ZNSt13__future_base13_State_baseV217_M_complete_asyncEv to i8*), i8* bitcast (i1 (%"class.std::__future_base::_State_baseV2"*)* @_ZNKSt13__future_base13_State_baseV221_M_is_deferred_futureEv to i8*), i8* bitcast (void (%"struct.std::__future_base::_Task_state"*)* @_ZNSt13__future_base11_Task_stateISt5_BindIFPFvPvS2_ES2_S2_EESaIiEFvvEE6_M_runEv to i8*), i8* bitcast (void (%"struct.std::__future_base::_Task_state"*, %"class.std::shared_ptr"*)* @_ZNSt13__future_base11_Task_stateISt5_BindIFPFvPvS2_ES2_S2_EESaIiEFvvEE14_M_run_delayedESt8weak_ptrINS_13_State_baseV2EE to i8*), i8* bitcast (void (%"class.std::shared_ptr.107"*, %"struct.std::__future_base::_Task_state"*)* @_ZNSt13__future_base11_Task_stateISt5_BindIFPFvPvS2_ES2_S2_EESaIiEFvvEE8_M_resetEv to i8*)] }, comdat, align 8
@_ZTISt19_Sp_make_shared_tag = linkonce_odr constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([24 x i8], [24 x i8]* @_ZTSSt19_Sp_make_shared_tag, i32 0, i32 0) }, comdat
@_ZTVN4MARC10ThreadTaskISt13packaged_taskIFvvEEEE = linkonce_odr unnamed_addr constant { [5 x i8*] } { [5 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTIN4MARC10ThreadTaskISt13packaged_taskIFvvEEEE to i8*), i8* bitcast (void (%"class.MARC::ThreadTask"*)* @_ZN4MARC10ThreadTaskISt13packaged_taskIFvvEEE7executeEv to i8*), i8* bitcast (void (%"class.MARC::ThreadTask"*)* @_ZN4MARC10ThreadTaskISt13packaged_taskIFvvEEED2Ev to i8*), i8* bitcast (void (%"class.MARC::ThreadTask"*)* @_ZN4MARC10ThreadTaskISt13packaged_taskIFvvEEED0Ev to i8*)] }, comdat, align 8
@_ZTVSt11logic_error = external unnamed_addr constant { [5 x i8*] }, align 8
@_ZTVSt12future_error = external unnamed_addr constant { [5 x i8*] }, align 8
@_ZTISt12future_error = external constant i8*
@.str.6 = private unnamed_addr constant [20 x i8] c"std::future_error: \00", align 1
@_ZTIN4MARC10ThreadTaskISt13packaged_taskIFvvEEEE = linkonce_odr constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([45 x i8], [45 x i8]* @_ZTSN4MARC10ThreadTaskISt13packaged_taskIFvvEEEE, i32 0, i32 0), i8* bitcast ({ i8*, i8* }* @_ZTIN4MARC11IThreadTaskE to i8*) }, comdat
@_ZTVN10__cxxabiv120__si_class_type_infoE = external global i8*
@_ZTSN4MARC10ThreadTaskISt13packaged_taskIFvvEEEE = linkonce_odr constant [45 x i8] c"N4MARC10ThreadTaskISt13packaged_taskIFvvEEEE\00", comdat
@_ZTIN4MARC11IThreadTaskE = linkonce_odr constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([21 x i8], [21 x i8]* @_ZTSN4MARC11IThreadTaskE, i32 0, i32 0) }, comdat
@_ZTVN10__cxxabiv117__class_type_infoE = external global i8*
@_ZTSN4MARC11IThreadTaskE = linkonce_odr constant [21 x i8] c"N4MARC11IThreadTaskE\00", comdat
@_ZTSSt19_Sp_make_shared_tag = linkonce_odr constant [24 x i8] c"St19_Sp_make_shared_tag\00", comdat
@_ZTINSt13__future_base11_Task_stateISt5_BindIFPFvPvS2_ES2_S2_EESaIiEFvvEEE = linkonce_odr constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([71 x i8], [71 x i8]* @_ZTSNSt13__future_base11_Task_stateISt5_BindIFPFvPvS2_ES2_S2_EESaIiEFvvEEE, i32 0, i32 0), i8* bitcast ({ i8*, i8*, i8* }* @_ZTINSt13__future_base16_Task_state_baseIFvvEEE to i8*) }, comdat
@_ZSt15__once_callable = external thread_local local_unnamed_addr global i8*, align 8
@_ZSt11__once_call = external thread_local local_unnamed_addr global void ()*, align 8
@_ZTINSt13__future_base12_Task_setterISt10unique_ptrINS_7_ResultIvEENS_12_Result_base8_DeleterEESt12_Bind_simpleIFSt17reference_wrapperISt5_BindIFPFvPvSA_ESA_SA_EEEvEEvEE = linkonce_odr constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([166 x i8], [166 x i8]* @_ZTSNSt13__future_base12_Task_setterISt10unique_ptrINS_7_ResultIvEENS_12_Result_base8_DeleterEESt12_Bind_simpleIFSt17reference_wrapperISt5_BindIFPFvPvSA_ESA_SA_EEEvEEvEE, i32 0, i32 0) }, comdat
@_ZTSNSt13__future_base12_Task_setterISt10unique_ptrINS_7_ResultIvEENS_12_Result_base8_DeleterEESt12_Bind_simpleIFSt17reference_wrapperISt5_BindIFPFvPvSA_ESA_SA_EEEvEEvEE = linkonce_odr constant [166 x i8] c"NSt13__future_base12_Task_setterISt10unique_ptrINS_7_ResultIvEENS_12_Result_base8_DeleterEESt12_Bind_simpleIFSt17reference_wrapperISt5_BindIFPFvPvSA_ESA_SA_EEEvEEvEE\00", comdat
@_ZTIN10__cxxabiv115__forced_unwindE = external constant i8*
@_ZTSNSt13__future_base11_Task_stateISt5_BindIFPFvPvS2_ES2_S2_EESaIiEFvvEEE = linkonce_odr constant [71 x i8] c"NSt13__future_base11_Task_stateISt5_BindIFPFvPvS2_ES2_S2_EESaIiEFvvEEE\00", comdat
@_ZTINSt13__future_base16_Task_state_baseIFvvEEE = linkonce_odr constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([44 x i8], [44 x i8]* @_ZTSNSt13__future_base16_Task_state_baseIFvvEEE, i32 0, i32 0), i8* bitcast ({ i8*, i8* }* @_ZTINSt13__future_base13_State_baseV2E to i8*) }, comdat
@_ZTSNSt13__future_base16_Task_state_baseIFvvEEE = linkonce_odr constant [44 x i8] c"NSt13__future_base16_Task_state_baseIFvvEEE\00", comdat
@_ZTINSt13__future_base13_State_baseV2E = linkonce_odr constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([35 x i8], [35 x i8]* @_ZTSNSt13__future_base13_State_baseV2E, i32 0, i32 0) }, comdat
@_ZTSNSt13__future_base13_State_baseV2E = linkonce_odr constant [35 x i8] c"NSt13__future_base13_State_baseV2E\00", comdat
@_ZTINSt13__future_base7_ResultIvEE = linkonce_odr constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([31 x i8], [31 x i8]* @_ZTSNSt13__future_base7_ResultIvEE, i32 0, i32 0), i8* bitcast (i8** @_ZTINSt13__future_base12_Result_baseE to i8*) }, comdat
@_ZTSNSt13__future_base7_ResultIvEE = linkonce_odr constant [31 x i8] c"NSt13__future_base7_ResultIvEE\00", comdat
@_ZTINSt13__future_base12_Result_baseE = external constant i8*
@_ZTISt23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EE = linkonce_odr constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([132 x i8], [132 x i8]* @_ZTSSt23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EE, i32 0, i32 0), i8* bitcast ({ i8*, i8*, i8* }* @_ZTISt16_Sp_counted_baseILN9__gnu_cxx12_Lock_policyE2EE to i8*) }, comdat
@_ZTSSt23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EE = linkonce_odr constant [132 x i8] c"St23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EE\00", comdat
@_ZTISt16_Sp_counted_baseILN9__gnu_cxx12_Lock_policyE2EE = linkonce_odr constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([52 x i8], [52 x i8]* @_ZTSSt16_Sp_counted_baseILN9__gnu_cxx12_Lock_policyE2EE, i32 0, i32 0), i8* bitcast ({ i8*, i8* }* @_ZTISt11_Mutex_baseILN9__gnu_cxx12_Lock_policyE2EE to i8*) }, comdat
@_ZTSSt16_Sp_counted_baseILN9__gnu_cxx12_Lock_policyE2EE = linkonce_odr constant [52 x i8] c"St16_Sp_counted_baseILN9__gnu_cxx12_Lock_policyE2EE\00", comdat
@_ZTISt11_Mutex_baseILN9__gnu_cxx12_Lock_policyE2EE = linkonce_odr constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([47 x i8], [47 x i8]* @_ZTSSt11_Mutex_baseILN9__gnu_cxx12_Lock_policyE2EE, i32 0, i32 0) }, comdat
@_ZTSSt11_Mutex_baseILN9__gnu_cxx12_Lock_policyE2EE = linkonce_odr constant [47 x i8] c"St11_Mutex_baseILN9__gnu_cxx12_Lock_policyE2EE\00", comdat
@_ZTVNSt6thread11_State_implISt12_Bind_simpleIFSt7_Mem_fnIMN4MARC10ThreadPoolEFvPSt6atomicIbEEEPS4_S7_EEEE = linkonce_odr unnamed_addr constant { [5 x i8*] } { [5 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTINSt6thread11_State_implISt12_Bind_simpleIFSt7_Mem_fnIMN4MARC10ThreadPoolEFvPSt6atomicIbEEEPS4_S7_EEEE to i8*), i8* bitcast (void (%"class.MARC::IThreadTask"*)* @_ZNSt6thread6_StateD2Ev to i8*), i8* bitcast (void (%"struct.std::thread::_State_impl"*)* @_ZNSt6thread11_State_implISt12_Bind_simpleIFSt7_Mem_fnIMN4MARC10ThreadPoolEFvPSt6atomicIbEEEPS4_S7_EEED0Ev to i8*), i8* bitcast (void (%"struct.std::thread::_State_impl"*)* @_ZNSt6thread11_State_implISt12_Bind_simpleIFSt7_Mem_fnIMN4MARC10ThreadPoolEFvPSt6atomicIbEEEPS4_S7_EEE6_M_runEv to i8*)] }, comdat, align 8
@_ZTINSt6thread11_State_implISt12_Bind_simpleIFSt7_Mem_fnIMN4MARC10ThreadPoolEFvPSt6atomicIbEEEPS4_S7_EEEE = linkonce_odr constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([102 x i8], [102 x i8]* @_ZTSNSt6thread11_State_implISt12_Bind_simpleIFSt7_Mem_fnIMN4MARC10ThreadPoolEFvPSt6atomicIbEEEPS4_S7_EEEE, i32 0, i32 0), i8* bitcast (i8** @_ZTINSt6thread6_StateE to i8*) }, comdat
@_ZTSNSt6thread11_State_implISt12_Bind_simpleIFSt7_Mem_fnIMN4MARC10ThreadPoolEFvPSt6atomicIbEEEPS4_S7_EEEE = linkonce_odr constant [102 x i8] c"NSt6thread11_State_implISt12_Bind_simpleIFSt7_Mem_fnIMN4MARC10ThreadPoolEFvPSt6atomicIbEEEPS4_S7_EEEE\00", comdat
@_ZTINSt6thread6_StateE = external constant i8*

; Function Attrs: noinline uwtable
define float @_Z16heavyComputationff(float, float) #0 {
  br label %3

; <label>:3:                                      ; preds = %30, %2
  %.04 = phi i32 [ 0, %2 ], [ %31, %30 ]
  %.01 = phi float [ %1, %2 ], [ %.3, %30 ]
  %.0 = phi float [ %0, %2 ], [ %.2, %30 ]
  %exitcond12 = icmp eq i32 %.04, 10
  br i1 %exitcond12, label %32, label %4

; <label>:4:                                      ; preds = %3
  %5 = icmp ugt i32 %.04, 3
  br i1 %5, label %6, label %18

; <label>:6:                                      ; preds = %4
  br label %7

; <label>:7:                                      ; preds = %16, %6
  %.05 = phi i32 [ 0, %6 ], [ %17, %16 ]
  %.1 = phi float [ %.0, %6 ], [ %.06.lcssa, %16 ]
  %exitcond10 = icmp eq i32 %.05, 10
  br i1 %exitcond10, label %.loopexit9, label %8

; <label>:8:                                      ; preds = %7
  %9 = fpext float %.1 to double
  %10 = fadd double %9, 1.430000e-01
  %11 = fptrunc double %10 to float
  br label %12

; <label>:12:                                     ; preds = %13, %8
  %.07 = phi i32 [ 0, %8 ], [ %15, %13 ]
  %.06 = phi float [ %11, %8 ], [ %14, %13 ]
  %exitcond = icmp eq i32 %.07, 3
  br i1 %exitcond, label %16, label %13

; <label>:13:                                     ; preds = %12
  %14 = tail call float @_ZSt4sqrtf(float %.06)
  %15 = add nuw nsw i32 %.07, 1
  br label %12

; <label>:16:                                     ; preds = %12
  %.06.lcssa = phi float [ %.06, %12 ]
  %17 = add nuw nsw i32 %.05, 1
  br label %7

.loopexit9:                                       ; preds = %7
  %.1.lcssa = phi float [ %.1, %7 ]
  br label %18

; <label>:18:                                     ; preds = %.loopexit9, %4
  %.2 = phi float [ %.0, %4 ], [ %.1.lcssa, %.loopexit9 ]
  %19 = icmp eq i32 %.04, 9
  %20 = fadd float %.01, %.2
  %..01 = select i1 %19, float %20, float %.01
  %21 = icmp ult i32 %.04, 7
  br i1 %21, label %22, label %30

; <label>:22:                                     ; preds = %18
  br label %23

; <label>:23:                                     ; preds = %24, %22
  %.08 = phi i32 [ 0, %22 ], [ %29, %24 ]
  %.23 = phi float [ %..01, %22 ], [ %28, %24 ]
  %exitcond11 = icmp eq i32 %.08, 5
  br i1 %exitcond11, label %.loopexit, label %24

; <label>:24:                                     ; preds = %23
  %25 = fpext float %.23 to double
  %26 = fadd double %25, -1.980000e-01
  %27 = fptrunc double %26 to float
  %28 = tail call float @_ZSt4sqrtf(float %27)
  %29 = add nuw nsw i32 %.08, 1
  br label %23

.loopexit:                                        ; preds = %23
  %.23.lcssa = phi float [ %.23, %23 ]
  br label %30

; <label>:30:                                     ; preds = %.loopexit, %18
  %.3 = phi float [ %..01, %18 ], [ %.23.lcssa, %.loopexit ]
  %31 = add nuw nsw i32 %.04, 1
  br label %3

; <label>:32:                                     ; preds = %3
  %.0.lcssa = phi float [ %.0, %3 ]
  ret float %.0.lcssa
}

; Function Attrs: noinline nounwind uwtable
define linkonce_odr float @_ZSt4sqrtf(float) #1 comdat {
  %2 = tail call float @sqrtf(float %0) #21
  ret float %2
}

; Function Attrs: nounwind readnone
declare float @sqrtf(float) #2

; Function Attrs: noinline norecurse uwtable
define i32 @main() #3 {
  %1 = tail call float @_Z16heavyComputationff(float 0x4041A66660000000, float 0x4038B33340000000)
  %2 = fpext float %1 to double
  %3 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), double %2)
  ret i32 0
}

declare i32 @printf(i8*, ...) #4

; Function Attrs: uwtable
define internal void @_GLOBAL__sub_I_DSWP_utils.cpp() #5 section ".text.startup" {
  tail call void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"* nonnull @_ZStL8__ioinit)
  %1 = tail call i32 @__cxa_atexit(void (i8*)* bitcast (void (%"class.std::ios_base::Init"*)* @_ZNSt8ios_base4InitD1Ev to void (i8*)*), i8* getelementptr inbounds (%"class.std::ios_base::Init", %"class.std::ios_base::Init"* @_ZStL8__ioinit, i64 0, i32 0), i8* nonnull @__dso_handle) #8
  ret void
}

declare void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"*) unnamed_addr #6

; Function Attrs: nounwind
declare void @_ZNSt8ios_base4InitD1Ev(%"class.std::ios_base::Init"*) unnamed_addr #7

; Function Attrs: nounwind
declare i32 @__cxa_atexit(void (i8*)*, i8*, i8*) local_unnamed_addr #8

; Function Attrs: uwtable
define void @printReachedS(%"class.std::basic_string"*) local_unnamed_addr #5 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %2 = alloca %"class.std::ios_base::Init", align 1
  %3 = alloca %"class.std::ios_base::Init", align 1
  %4 = alloca %"class.std::basic_string", align 8
  %5 = bitcast %"class.std::basic_string"* %4 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %5) #8
  %6 = getelementptr inbounds %"class.std::basic_string", %"class.std::basic_string"* %4, i64 0, i32 0, i32 0
  store i8* bitcast (i64* getelementptr inbounds ([0 x i64], [0 x i64]* @_ZNSs4_Rep20_S_empty_rep_storageE, i64 0, i64 3) to i8*), i8** %6, align 8, !tbaa !2, !alias.scope !7
  %7 = getelementptr inbounds %"class.std::basic_string", %"class.std::basic_string"* %0, i64 0, i32 0, i32 0
  %8 = load i8*, i8** %7, align 8, !tbaa !10, !noalias !7
  %9 = getelementptr inbounds i8, i8* %8, i64 -24
  %10 = bitcast i8* %9 to i64*
  %11 = load i64, i64* %10, align 8, !tbaa !12, !noalias !7
  %12 = add i64 %11, 9
  invoke void @_ZNSs7reserveEm(%"class.std::basic_string"* nonnull %4, i64 %12)
          to label %13 unwind label %17

; <label>:13:                                     ; preds = %1
  %14 = invoke dereferenceable(8) %"class.std::basic_string"* @_ZNSs6appendEPKcm(%"class.std::basic_string"* nonnull %4, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str.1, i64 0, i64 0), i64 9)
          to label %15 unwind label %17

; <label>:15:                                     ; preds = %13
  %16 = invoke dereferenceable(8) %"class.std::basic_string"* @_ZNSs6appendERKSs(%"class.std::basic_string"* nonnull %4, %"class.std::basic_string"* nonnull dereferenceable(8) %0)
          to label %37 unwind label %17

; <label>:17:                                     ; preds = %15, %13, %1
  %18 = landingpad { i8*, i32 }
          cleanup
  %19 = load i8*, i8** %6, align 8, !tbaa !10, !alias.scope !7
  %20 = getelementptr inbounds i8, i8* %19, i64 -24
  %21 = bitcast i8* %20 to %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Rep"*
  %22 = getelementptr inbounds %"class.std::ios_base::Init", %"class.std::ios_base::Init"* %3, i64 0, i32 0
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %22) #8, !noalias !7
  %23 = icmp eq i8* %20, bitcast ([0 x i64]* @_ZNSs4_Rep20_S_empty_rep_storageE to i8*)
  br i1 %23, label %36, label %24, !prof !16

; <label>:24:                                     ; preds = %17
  %25 = getelementptr inbounds i8, i8* %19, i64 -8
  %26 = bitcast i8* %25 to i32*
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %27, label %29

; <label>:27:                                     ; preds = %24
  %28 = atomicrmw volatile add i32* %26, i32 -1 acq_rel
  br label %32

; <label>:29:                                     ; preds = %24
  %30 = load i32, i32* %26, align 4, !tbaa !17
  %31 = add nsw i32 %30, -1
  store i32 %31, i32* %26, align 4, !tbaa !17
  br label %32

; <label>:32:                                     ; preds = %29, %27
  %33 = phi i32 [ %28, %27 ], [ %30, %29 ]
  %34 = icmp slt i32 %33, 1
  br i1 %34, label %35, label %36

; <label>:35:                                     ; preds = %32
  call void @_ZNSs4_Rep10_M_destroyERKSaIcE(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Rep"* %21, %"class.std::ios_base::Init"* nonnull dereferenceable(1) %3) #8
  br label %36

; <label>:36:                                     ; preds = %35, %32, %17
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %22) #8, !noalias !7
  resume { i8*, i32 } %18

; <label>:37:                                     ; preds = %15
  %38 = load i8*, i8** %6, align 8, !tbaa !10
  %39 = call i32 @puts(i8* %38)
  %40 = load i8*, i8** %6, align 8, !tbaa !10
  %41 = getelementptr inbounds i8, i8* %40, i64 -24
  %42 = bitcast i8* %41 to %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Rep"*
  %43 = getelementptr inbounds %"class.std::ios_base::Init", %"class.std::ios_base::Init"* %2, i64 0, i32 0
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %43) #8
  %44 = icmp eq i8* %41, bitcast ([0 x i64]* @_ZNSs4_Rep20_S_empty_rep_storageE to i8*)
  br i1 %44, label %57, label %45, !prof !16

; <label>:45:                                     ; preds = %37
  %46 = getelementptr inbounds i8, i8* %40, i64 -8
  %47 = bitcast i8* %46 to i32*
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %48, label %50

; <label>:48:                                     ; preds = %45
  %49 = atomicrmw volatile add i32* %47, i32 -1 acq_rel
  br label %53

; <label>:50:                                     ; preds = %45
  %51 = load i32, i32* %47, align 4, !tbaa !17
  %52 = add nsw i32 %51, -1
  store i32 %52, i32* %47, align 4, !tbaa !17
  br label %53

; <label>:53:                                     ; preds = %50, %48
  %54 = phi i32 [ %49, %48 ], [ %51, %50 ]
  %55 = icmp slt i32 %54, 1
  br i1 %55, label %56, label %57

; <label>:56:                                     ; preds = %53
  call void @_ZNSs4_Rep10_M_destroyERKSaIcE(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Rep"* %42, %"class.std::ios_base::Init"* nonnull dereferenceable(1) %2) #8
  br label %57

; <label>:57:                                     ; preds = %37, %53, %56
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %43) #8
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %5) #8
  ret void
}

declare i32 @__gxx_personality_v0(...)

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #9

declare void @_ZNSs7reserveEm(%"class.std::basic_string"*, i64) local_unnamed_addr #6

declare dereferenceable(8) %"class.std::basic_string"* @_ZNSs6appendEPKcm(%"class.std::basic_string"*, i8*, i64) local_unnamed_addr #6

declare dereferenceable(8) %"class.std::basic_string"* @_ZNSs6appendERKSs(%"class.std::basic_string"*, %"class.std::basic_string"* dereferenceable(8)) local_unnamed_addr #6

; Function Attrs: nounwind
declare extern_weak i32 @__pthread_key_create(i32*, void (i8*)*) #7

; Function Attrs: nounwind
declare void @_ZNSs4_Rep10_M_destroyERKSaIcE(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Rep"*, %"class.std::ios_base::Init"* dereferenceable(1)) local_unnamed_addr #7

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #9

; Function Attrs: nounwind
declare i32 @puts(i8* nocapture readonly) #8

; Function Attrs: nounwind uwtable
define void @printReachedI(i32) local_unnamed_addr #10 {
  %2 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str.2, i64 0, i64 0), i32 %0)
  ret void
}

; Function Attrs: uwtable
define void @queuePush8(%"class.MARC::ThreadSafeQueue"*, i8* nocapture readonly) local_unnamed_addr #5 {
  %3 = load i8, i8* %1, align 1, !tbaa !18
  tail call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %0, i8 signext %3)
  ret void
}

; Function Attrs: uwtable
define linkonce_odr void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"*, i8 signext) local_unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %3 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %4, label %9

; <label>:4:                                      ; preds = %2
  %5 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %3, i64 0, i32 0, i32 0
  %6 = tail call i32 @pthread_mutex_lock(%union.pthread_mutex_t* nonnull %5) #8
  %7 = icmp eq i32 %6, 0
  br i1 %7, label %9, label %8

; <label>:8:                                      ; preds = %4
  tail call void @_ZSt20__throw_system_errori(i32 %6) #23
  unreachable

; <label>:9:                                      ; preds = %2, %4
  %10 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2
  %11 = getelementptr inbounds %"class.std::queue", %"class.std::queue"* %10, i64 0, i32 0
  %12 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 0
  %13 = load i8*, i8** %12, align 8, !tbaa !19
  %14 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 2
  %15 = load i8*, i8** %14, align 8, !tbaa !23
  %16 = getelementptr inbounds i8, i8* %15, i64 -1
  %17 = icmp eq i8* %13, %16
  br i1 %17, label %21, label %18

; <label>:18:                                     ; preds = %9
  store i8 %1, i8* %13, align 1, !tbaa !18
  %19 = load i8*, i8** %12, align 8, !tbaa !19
  %20 = getelementptr inbounds i8, i8* %19, i64 1
  store i8* %20, i8** %12, align 8, !tbaa !19
  br label %49

; <label>:21:                                     ; preds = %9
  %22 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 1
  %23 = load i64, i64* %22, align 8, !tbaa !24
  %24 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 3
  %25 = bitcast i8*** %24 to i64*
  %26 = load i64, i64* %25, align 8, !tbaa !25
  %27 = bitcast %"class.std::queue"* %10 to i64*
  %28 = load i64, i64* %27, align 8, !tbaa !26
  %29 = sub i64 %26, %28
  %30 = ashr exact i64 %29, 3
  %31 = sub i64 %23, %30
  %32 = icmp ult i64 %31, 2
  br i1 %32, label %33, label %34

; <label>:33:                                     ; preds = %21
  invoke void @_ZNSt5dequeIaSaIaEE17_M_reallocate_mapEmb(%"class.std::deque"* nonnull %11, i64 1, i1 zeroext false)
          to label %34 unwind label %55

; <label>:34:                                     ; preds = %33, %21
  %35 = invoke i8* @_Znwm(i64 512)
          to label %36 unwind label %55

; <label>:36:                                     ; preds = %34
  %37 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3
  %38 = load i8**, i8*** %24, align 8, !tbaa !25
  %39 = getelementptr inbounds i8*, i8** %38, i64 1
  store i8* %35, i8** %39, align 8, !tbaa !27
  %40 = getelementptr inbounds %"struct.std::_Deque_iterator", %"struct.std::_Deque_iterator"* %37, i64 0, i32 0
  %41 = load i8*, i8** %40, align 8, !tbaa !19
  store i8 %1, i8* %41, align 1, !tbaa !18
  %42 = load i8**, i8*** %24, align 8, !tbaa !25
  %43 = getelementptr inbounds i8*, i8** %42, i64 1
  store i8** %43, i8*** %24, align 8, !tbaa !28
  %44 = load i8*, i8** %43, align 8, !tbaa !27
  %45 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 1
  store i8* %44, i8** %45, align 8, !tbaa !29
  %46 = getelementptr inbounds i8, i8* %44, i64 512
  store i8* %46, i8** %14, align 8, !tbaa !30
  %47 = ptrtoint i8* %44 to i64
  %48 = bitcast %"struct.std::_Deque_iterator"* %37 to i64*
  store i64 %47, i64* %48, align 8, !tbaa !19
  br label %49

; <label>:49:                                     ; preds = %36, %18
  %50 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 3
  tail call void @_ZNSt18condition_variable10notify_oneEv(%"class.std::condition_variable"* %50) #8
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %51, label %54

; <label>:51:                                     ; preds = %49
  %52 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %3, i64 0, i32 0, i32 0
  %53 = tail call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %52) #8
  br label %54

; <label>:54:                                     ; preds = %49, %51
  ret void

; <label>:55:                                     ; preds = %34, %33
  %56 = landingpad { i8*, i32 }
          cleanup
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %57, label %60

; <label>:57:                                     ; preds = %55
  %58 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %3, i64 0, i32 0, i32 0
  %59 = tail call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %58) #8
  br label %60

; <label>:60:                                     ; preds = %55, %57
  resume { i8*, i32 } %56
}

; Function Attrs: nounwind
declare extern_weak i32 @pthread_mutex_lock(%union.pthread_mutex_t*) local_unnamed_addr #7

; Function Attrs: noreturn
declare void @_ZSt20__throw_system_errori(i32) local_unnamed_addr #11

; Function Attrs: uwtable
define linkonce_odr void @_ZNSt5dequeIaSaIaEE17_M_reallocate_mapEmb(%"class.std::deque"*, i64, i1 zeroext) local_unnamed_addr #5 comdat align 2 personality i32 (...)* @__gxx_personality_v0 {
  %4 = getelementptr inbounds %"class.std::deque", %"class.std::deque"* %0, i64 0, i32 0, i32 0, i32 3, i32 3
  %5 = load i8**, i8*** %4, align 8, !tbaa !25
  %6 = getelementptr inbounds %"class.std::deque", %"class.std::deque"* %0, i64 0, i32 0, i32 0, i32 2, i32 3
  %7 = load i8**, i8*** %6, align 8, !tbaa !31
  %8 = ptrtoint i8** %5 to i64
  %9 = ptrtoint i8** %7 to i64
  %10 = sub i64 %8, %9
  %11 = ashr exact i64 %10, 3
  %12 = add nsw i64 %11, 1
  %13 = add i64 %12, %1
  %14 = getelementptr inbounds %"class.std::deque", %"class.std::deque"* %0, i64 0, i32 0, i32 0, i32 1
  %15 = load i64, i64* %14, align 8, !tbaa !24
  %16 = shl i64 %13, 1
  %17 = icmp ugt i64 %15, %16
  br i1 %17, label %18, label %44

; <label>:18:                                     ; preds = %3
  %19 = getelementptr inbounds %"class.std::deque", %"class.std::deque"* %0, i64 0, i32 0, i32 0, i32 0
  %20 = load i8**, i8*** %19, align 8, !tbaa !26
  %21 = sub i64 %15, %13
  %22 = lshr i64 %21, 1
  %23 = getelementptr inbounds i8*, i8** %20, i64 %22
  %24 = select i1 %2, i64 %1, i64 0
  %25 = getelementptr inbounds i8*, i8** %23, i64 %24
  %26 = icmp ult i8** %25, %7
  %27 = getelementptr inbounds i8*, i8** %5, i64 1
  %28 = ptrtoint i8** %27 to i64
  %29 = sub i64 %28, %9
  br i1 %26, label %30, label %35

; <label>:30:                                     ; preds = %18
  %31 = icmp eq i64 %29, 0
  br i1 %31, label %73, label %32

; <label>:32:                                     ; preds = %30
  %33 = bitcast i8** %25 to i8*
  %34 = bitcast i8** %7 to i8*
  tail call void @llvm.memmove.p0i8.p0i8.i64(i8* %33, i8* %34, i64 %29, i32 8, i1 false) #8
  br label %73

; <label>:35:                                     ; preds = %18
  %36 = ashr exact i64 %29, 3
  %37 = icmp eq i64 %36, 0
  br i1 %37, label %73, label %38

; <label>:38:                                     ; preds = %35
  %39 = getelementptr inbounds i8*, i8** %25, i64 %12
  %40 = sub nsw i64 0, %36
  %41 = getelementptr inbounds i8*, i8** %39, i64 %40
  %42 = bitcast i8** %41 to i8*
  %43 = bitcast i8** %7 to i8*
  tail call void @llvm.memmove.p0i8.p0i8.i64(i8* %42, i8* %43, i64 %29, i32 8, i1 false) #8
  br label %73

; <label>:44:                                     ; preds = %3
  %45 = icmp ult i64 %15, %1
  %46 = select i1 %45, i64 %1, i64 %15
  %47 = add i64 %15, 2
  %48 = add i64 %47, %46
  %49 = icmp ugt i64 %48, 2305843009213693951
  br i1 %49, label %50, label %51

; <label>:50:                                     ; preds = %44
  tail call void @_ZSt17__throw_bad_allocv() #23
  unreachable

; <label>:51:                                     ; preds = %44
  %52 = shl i64 %48, 3
  %53 = tail call i8* @_Znwm(i64 %52)
  %54 = bitcast i8* %53 to i8**
  %55 = sub i64 %48, %13
  %56 = lshr i64 %55, 1
  %57 = getelementptr inbounds i8*, i8** %54, i64 %56
  %58 = select i1 %2, i64 %1, i64 0
  %59 = getelementptr inbounds i8*, i8** %57, i64 %58
  %60 = load i8**, i8*** %6, align 8, !tbaa !31
  %61 = load i8**, i8*** %4, align 8, !tbaa !25
  %62 = getelementptr inbounds i8*, i8** %61, i64 1
  %63 = ptrtoint i8** %62 to i64
  %64 = ptrtoint i8** %60 to i64
  %65 = sub i64 %63, %64
  %66 = icmp eq i64 %65, 0
  br i1 %66, label %70, label %67

; <label>:67:                                     ; preds = %51
  %68 = bitcast i8** %59 to i8*
  %69 = bitcast i8** %60 to i8*
  tail call void @llvm.memmove.p0i8.p0i8.i64(i8* %68, i8* %69, i64 %65, i32 8, i1 false) #8
  br label %70

; <label>:70:                                     ; preds = %51, %67
  %71 = bitcast %"class.std::deque"* %0 to i8**
  %72 = load i8*, i8** %71, align 8, !tbaa !26
  tail call void @_ZdlPv(i8* %72) #8
  store i8* %53, i8** %71, align 8, !tbaa !26
  store i64 %48, i64* %14, align 8, !tbaa !24
  br label %73

; <label>:73:                                     ; preds = %38, %35, %32, %30, %70
  %74 = phi i8** [ %59, %70 ], [ %25, %30 ], [ %25, %32 ], [ %25, %35 ], [ %25, %38 ]
  store i8** %74, i8*** %6, align 8, !tbaa !28
  %75 = load i8*, i8** %74, align 8, !tbaa !27
  %76 = getelementptr inbounds %"class.std::deque", %"class.std::deque"* %0, i64 0, i32 0, i32 0, i32 2, i32 1
  store i8* %75, i8** %76, align 8, !tbaa !29
  %77 = getelementptr inbounds i8, i8* %75, i64 512
  %78 = getelementptr inbounds %"class.std::deque", %"class.std::deque"* %0, i64 0, i32 0, i32 0, i32 2, i32 2
  store i8* %77, i8** %78, align 8, !tbaa !30
  %79 = getelementptr inbounds i8*, i8** %74, i64 %11
  store i8** %79, i8*** %4, align 8, !tbaa !28
  %80 = load i8*, i8** %79, align 8, !tbaa !27
  %81 = getelementptr inbounds %"class.std::deque", %"class.std::deque"* %0, i64 0, i32 0, i32 0, i32 3, i32 1
  store i8* %80, i8** %81, align 8, !tbaa !29
  %82 = getelementptr inbounds i8, i8* %80, i64 512
  %83 = getelementptr inbounds %"class.std::deque", %"class.std::deque"* %0, i64 0, i32 0, i32 0, i32 3, i32 2
  store i8* %82, i8** %83, align 8, !tbaa !30
  ret void
}

; Function Attrs: nobuiltin
declare noalias nonnull i8* @_Znwm(i64) local_unnamed_addr #12

; Function Attrs: nounwind
declare void @_ZNSt18condition_variable10notify_oneEv(%"class.std::condition_variable"*) local_unnamed_addr #7

; Function Attrs: nounwind
declare extern_weak i32 @pthread_mutex_unlock(%union.pthread_mutex_t*) local_unnamed_addr #7

; Function Attrs: argmemonly nounwind
declare void @llvm.memmove.p0i8.p0i8.i64(i8* nocapture, i8* nocapture readonly, i64, i32, i1) #9

; Function Attrs: noreturn
declare void @_ZSt17__throw_bad_allocv() local_unnamed_addr #11

; Function Attrs: nobuiltin nounwind
declare void @_ZdlPv(i8*) local_unnamed_addr #13

; Function Attrs: uwtable
define void @queuePop8(%"class.MARC::ThreadSafeQueue"*, i8*) local_unnamed_addr #5 {
  %3 = tail call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %0, i8* dereferenceable(1) %1)
  ret void
}

; Function Attrs: uwtable
define linkonce_odr zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"*, i8* dereferenceable(1)) local_unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %3 = alloca %"class.std::unique_lock", align 8
  %4 = bitcast %"class.std::unique_lock"* %3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %4) #8
  %5 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 1
  %6 = getelementptr inbounds %"class.std::unique_lock", %"class.std::unique_lock"* %3, i64 0, i32 0
  store %"class.std::mutex"* %5, %"class.std::mutex"** %6, align 8, !tbaa !32
  %7 = getelementptr inbounds %"class.std::unique_lock", %"class.std::unique_lock"* %3, i64 0, i32 1
  store i8 0, i8* %7, align 8, !tbaa !35
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %8, label %13

; <label>:8:                                      ; preds = %2
  %9 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %5, i64 0, i32 0, i32 0
  %10 = tail call i32 @pthread_mutex_lock(%union.pthread_mutex_t* nonnull %9) #8
  %11 = icmp eq i32 %10, 0
  br i1 %11, label %13, label %12

; <label>:12:                                     ; preds = %8
  tail call void @_ZSt20__throw_system_errori(i32 %10) #23
  unreachable

; <label>:13:                                     ; preds = %2, %8
  store i8 1, i8* %7, align 8, !tbaa !35
  %14 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 0, i32 0, i32 0
  %15 = load atomic i8, i8* %14 seq_cst, align 1
  %16 = and i8 %15, 1
  %17 = icmp eq i8 %16, 0
  br i1 %17, label %61, label %18

; <label>:18:                                     ; preds = %13
  %19 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 0
  %20 = load i8*, i8** %19, align 8, !tbaa !36
  %21 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 0
  %22 = load i8*, i8** %21, align 8, !tbaa !36
  %23 = icmp eq i8* %20, %22
  br i1 %23, label %24, label %34

; <label>:24:                                     ; preds = %18
  %25 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 3
  br label %26

; <label>:26:                                     ; preds = %30, %24
  %27 = load atomic i8, i8* %14 seq_cst, align 1
  %28 = and i8 %27, 1
  %29 = icmp eq i8 %28, 0
  br i1 %29, label %34, label %30

; <label>:30:                                     ; preds = %26
  call void @_ZNSt18condition_variable4waitERSt11unique_lockISt5mutexE(%"class.std::condition_variable"* nonnull %25, %"class.std::unique_lock"* nonnull dereferenceable(16) %3) #8
  %31 = load i8*, i8** %19, align 8, !tbaa !36
  %32 = load i8*, i8** %21, align 8, !tbaa !36
  %33 = icmp eq i8* %31, %32
  br i1 %33, label %26, label %34

; <label>:34:                                     ; preds = %30, %26, %18
  %35 = load atomic i8, i8* %14 seq_cst, align 1
  %36 = and i8 %35, 1
  %37 = icmp eq i8 %36, 0
  br i1 %37, label %61, label %38

; <label>:38:                                     ; preds = %34
  %39 = load i8*, i8** %21, align 8, !tbaa !36, !noalias !37
  %40 = load i8, i8* %39, align 1, !tbaa !18
  store i8 %40, i8* %1, align 1, !tbaa !18
  %41 = load i8*, i8** %21, align 8, !tbaa !40
  %42 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 2
  %43 = load i8*, i8** %42, align 8, !tbaa !41
  %44 = getelementptr inbounds i8, i8* %43, i64 -1
  %45 = icmp eq i8* %41, %44
  br i1 %45, label %48, label %46

; <label>:46:                                     ; preds = %38
  %47 = getelementptr inbounds i8, i8* %41, i64 1
  store i8* %47, i8** %21, align 8, !tbaa !40
  br label %59

; <label>:48:                                     ; preds = %38
  %49 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2
  %50 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 1
  %51 = load i8*, i8** %50, align 8, !tbaa !42
  call void @_ZdlPv(i8* %51) #8
  %52 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 3
  %53 = load i8**, i8*** %52, align 8, !tbaa !31
  %54 = getelementptr inbounds i8*, i8** %53, i64 1
  store i8** %54, i8*** %52, align 8, !tbaa !28
  %55 = load i8*, i8** %54, align 8, !tbaa !27
  store i8* %55, i8** %50, align 8, !tbaa !29
  %56 = getelementptr inbounds i8, i8* %55, i64 512
  store i8* %56, i8** %42, align 8, !tbaa !30
  %57 = ptrtoint i8* %55 to i64
  %58 = bitcast %"struct.std::_Deque_iterator"* %49 to i64*
  store i64 %57, i64* %58, align 8, !tbaa !40
  br label %59

; <label>:59:                                     ; preds = %46, %48
  %60 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 4
  call void @_ZNSt18condition_variable10notify_oneEv(%"class.std::condition_variable"* %60) #8
  br label %61

; <label>:61:                                     ; preds = %34, %13, %59
  %62 = phi i1 [ false, %13 ], [ false, %34 ], [ true, %59 ]
  %63 = load i8, i8* %7, align 8, !tbaa !35, !range !43
  %64 = icmp eq i8 %63, 0
  br i1 %64, label %73, label %65

; <label>:65:                                     ; preds = %61
  %66 = load %"class.std::mutex"*, %"class.std::mutex"** %6, align 8, !tbaa !32
  %67 = icmp eq %"class.std::mutex"* %66, null
  br i1 %67, label %73, label %68

; <label>:68:                                     ; preds = %65
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %69, label %72

; <label>:69:                                     ; preds = %68
  %70 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %66, i64 0, i32 0, i32 0
  %71 = call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %70) #8
  br label %72

; <label>:72:                                     ; preds = %69, %68
  store i8 0, i8* %7, align 8, !tbaa !35
  br label %73

; <label>:73:                                     ; preds = %61, %65, %72
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %4) #8
  ret i1 %62
}

; Function Attrs: nounwind
declare void @_ZNSt18condition_variable4waitERSt11unique_lockISt5mutexE(%"class.std::condition_variable"*, %"class.std::unique_lock"* dereferenceable(16)) local_unnamed_addr #7

; Function Attrs: uwtable
define void @queuePush16(%"class.MARC::ThreadSafeQueue.3"*, i16* nocapture readonly) local_unnamed_addr #5 {
  %3 = load i16, i16* %1, align 2, !tbaa !44
  tail call void @_ZN4MARC15ThreadSafeQueueIsE4pushEs(%"class.MARC::ThreadSafeQueue.3"* %0, i16 signext %3)
  ret void
}

; Function Attrs: uwtable
define linkonce_odr void @_ZN4MARC15ThreadSafeQueueIsE4pushEs(%"class.MARC::ThreadSafeQueue.3"*, i16 signext) local_unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %3 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %4, label %9

; <label>:4:                                      ; preds = %2
  %5 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %3, i64 0, i32 0, i32 0
  %6 = tail call i32 @pthread_mutex_lock(%union.pthread_mutex_t* nonnull %5) #8
  %7 = icmp eq i32 %6, 0
  br i1 %7, label %9, label %8

; <label>:8:                                      ; preds = %4
  tail call void @_ZSt20__throw_system_errori(i32 %6) #23
  unreachable

; <label>:9:                                      ; preds = %2, %4
  %10 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 2
  %11 = getelementptr inbounds %"class.std::queue.4", %"class.std::queue.4"* %10, i64 0, i32 0
  %12 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 0
  %13 = load i16*, i16** %12, align 8, !tbaa !46
  %14 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 2
  %15 = load i16*, i16** %14, align 8, !tbaa !50
  %16 = getelementptr inbounds i16, i16* %15, i64 -1
  %17 = icmp eq i16* %13, %16
  br i1 %17, label %20, label %18

; <label>:18:                                     ; preds = %9
  store i16 %1, i16* %13, align 2, !tbaa !44
  %19 = getelementptr inbounds i16, i16* %13, i64 1
  store i16* %19, i16** %12, align 8, !tbaa !46
  br label %49

; <label>:20:                                     ; preds = %9
  %21 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 1
  %22 = load i64, i64* %21, align 8, !tbaa !51
  %23 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 3
  %24 = bitcast i16*** %23 to i64*
  %25 = load i64, i64* %24, align 8, !tbaa !52
  %26 = bitcast %"class.std::queue.4"* %10 to i64*
  %27 = load i64, i64* %26, align 8, !tbaa !53
  %28 = sub i64 %25, %27
  %29 = ashr exact i64 %28, 3
  %30 = sub i64 %22, %29
  %31 = icmp ult i64 %30, 2
  br i1 %31, label %32, label %33

; <label>:32:                                     ; preds = %20
  invoke void @_ZNSt5dequeIsSaIsEE17_M_reallocate_mapEmb(%"class.std::deque.5"* nonnull %11, i64 1, i1 zeroext false)
          to label %33 unwind label %55

; <label>:33:                                     ; preds = %32, %20
  %34 = invoke i8* @_Znwm(i64 512)
          to label %35 unwind label %55

; <label>:35:                                     ; preds = %33
  %36 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3
  %37 = load i16**, i16*** %23, align 8, !tbaa !52
  %38 = getelementptr inbounds i16*, i16** %37, i64 1
  %39 = bitcast i16** %38 to i8**
  store i8* %34, i8** %39, align 8, !tbaa !27
  %40 = getelementptr inbounds %"struct.std::_Deque_iterator.10", %"struct.std::_Deque_iterator.10"* %36, i64 0, i32 0
  %41 = load i16*, i16** %40, align 8, !tbaa !46
  store i16 %1, i16* %41, align 2, !tbaa !44
  %42 = load i16**, i16*** %23, align 8, !tbaa !52
  %43 = getelementptr inbounds i16*, i16** %42, i64 1
  store i16** %43, i16*** %23, align 8, !tbaa !54
  %44 = load i16*, i16** %43, align 8, !tbaa !27
  %45 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 1
  store i16* %44, i16** %45, align 8, !tbaa !55
  %46 = getelementptr inbounds i16, i16* %44, i64 256
  store i16* %46, i16** %14, align 8, !tbaa !56
  %47 = ptrtoint i16* %44 to i64
  %48 = bitcast %"struct.std::_Deque_iterator.10"* %36 to i64*
  store i64 %47, i64* %48, align 8, !tbaa !46
  br label %49

; <label>:49:                                     ; preds = %35, %18
  %50 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 3
  tail call void @_ZNSt18condition_variable10notify_oneEv(%"class.std::condition_variable"* %50) #8
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %51, label %54

; <label>:51:                                     ; preds = %49
  %52 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %3, i64 0, i32 0, i32 0
  %53 = tail call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %52) #8
  br label %54

; <label>:54:                                     ; preds = %49, %51
  ret void

; <label>:55:                                     ; preds = %33, %32
  %56 = landingpad { i8*, i32 }
          cleanup
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %57, label %60

; <label>:57:                                     ; preds = %55
  %58 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %3, i64 0, i32 0, i32 0
  %59 = tail call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %58) #8
  br label %60

; <label>:60:                                     ; preds = %55, %57
  resume { i8*, i32 } %56
}

; Function Attrs: uwtable
define linkonce_odr void @_ZNSt5dequeIsSaIsEE17_M_reallocate_mapEmb(%"class.std::deque.5"*, i64, i1 zeroext) local_unnamed_addr #5 comdat align 2 personality i32 (...)* @__gxx_personality_v0 {
  %4 = getelementptr inbounds %"class.std::deque.5", %"class.std::deque.5"* %0, i64 0, i32 0, i32 0, i32 3, i32 3
  %5 = load i16**, i16*** %4, align 8, !tbaa !52
  %6 = getelementptr inbounds %"class.std::deque.5", %"class.std::deque.5"* %0, i64 0, i32 0, i32 0, i32 2, i32 3
  %7 = load i16**, i16*** %6, align 8, !tbaa !57
  %8 = ptrtoint i16** %5 to i64
  %9 = ptrtoint i16** %7 to i64
  %10 = sub i64 %8, %9
  %11 = ashr exact i64 %10, 3
  %12 = add nsw i64 %11, 1
  %13 = add i64 %12, %1
  %14 = getelementptr inbounds %"class.std::deque.5", %"class.std::deque.5"* %0, i64 0, i32 0, i32 0, i32 1
  %15 = load i64, i64* %14, align 8, !tbaa !51
  %16 = shl i64 %13, 1
  %17 = icmp ugt i64 %15, %16
  br i1 %17, label %18, label %44

; <label>:18:                                     ; preds = %3
  %19 = getelementptr inbounds %"class.std::deque.5", %"class.std::deque.5"* %0, i64 0, i32 0, i32 0, i32 0
  %20 = load i16**, i16*** %19, align 8, !tbaa !53
  %21 = sub i64 %15, %13
  %22 = lshr i64 %21, 1
  %23 = getelementptr inbounds i16*, i16** %20, i64 %22
  %24 = select i1 %2, i64 %1, i64 0
  %25 = getelementptr inbounds i16*, i16** %23, i64 %24
  %26 = icmp ult i16** %25, %7
  %27 = getelementptr inbounds i16*, i16** %5, i64 1
  %28 = ptrtoint i16** %27 to i64
  %29 = sub i64 %28, %9
  br i1 %26, label %30, label %35

; <label>:30:                                     ; preds = %18
  %31 = icmp eq i64 %29, 0
  br i1 %31, label %73, label %32

; <label>:32:                                     ; preds = %30
  %33 = bitcast i16** %25 to i8*
  %34 = bitcast i16** %7 to i8*
  tail call void @llvm.memmove.p0i8.p0i8.i64(i8* %33, i8* %34, i64 %29, i32 8, i1 false) #8
  br label %73

; <label>:35:                                     ; preds = %18
  %36 = ashr exact i64 %29, 3
  %37 = icmp eq i64 %36, 0
  br i1 %37, label %73, label %38

; <label>:38:                                     ; preds = %35
  %39 = getelementptr inbounds i16*, i16** %25, i64 %12
  %40 = sub nsw i64 0, %36
  %41 = getelementptr inbounds i16*, i16** %39, i64 %40
  %42 = bitcast i16** %41 to i8*
  %43 = bitcast i16** %7 to i8*
  tail call void @llvm.memmove.p0i8.p0i8.i64(i8* %42, i8* %43, i64 %29, i32 8, i1 false) #8
  br label %73

; <label>:44:                                     ; preds = %3
  %45 = icmp ult i64 %15, %1
  %46 = select i1 %45, i64 %1, i64 %15
  %47 = add i64 %15, 2
  %48 = add i64 %47, %46
  %49 = icmp ugt i64 %48, 2305843009213693951
  br i1 %49, label %50, label %51

; <label>:50:                                     ; preds = %44
  tail call void @_ZSt17__throw_bad_allocv() #23
  unreachable

; <label>:51:                                     ; preds = %44
  %52 = shl i64 %48, 3
  %53 = tail call i8* @_Znwm(i64 %52)
  %54 = bitcast i8* %53 to i16**
  %55 = sub i64 %48, %13
  %56 = lshr i64 %55, 1
  %57 = getelementptr inbounds i16*, i16** %54, i64 %56
  %58 = select i1 %2, i64 %1, i64 0
  %59 = getelementptr inbounds i16*, i16** %57, i64 %58
  %60 = load i16**, i16*** %6, align 8, !tbaa !57
  %61 = load i16**, i16*** %4, align 8, !tbaa !52
  %62 = getelementptr inbounds i16*, i16** %61, i64 1
  %63 = ptrtoint i16** %62 to i64
  %64 = ptrtoint i16** %60 to i64
  %65 = sub i64 %63, %64
  %66 = icmp eq i64 %65, 0
  br i1 %66, label %70, label %67

; <label>:67:                                     ; preds = %51
  %68 = bitcast i16** %59 to i8*
  %69 = bitcast i16** %60 to i8*
  tail call void @llvm.memmove.p0i8.p0i8.i64(i8* %68, i8* %69, i64 %65, i32 8, i1 false) #8
  br label %70

; <label>:70:                                     ; preds = %51, %67
  %71 = bitcast %"class.std::deque.5"* %0 to i8**
  %72 = load i8*, i8** %71, align 8, !tbaa !53
  tail call void @_ZdlPv(i8* %72) #8
  store i8* %53, i8** %71, align 8, !tbaa !53
  store i64 %48, i64* %14, align 8, !tbaa !51
  br label %73

; <label>:73:                                     ; preds = %38, %35, %32, %30, %70
  %74 = phi i16** [ %59, %70 ], [ %25, %30 ], [ %25, %32 ], [ %25, %35 ], [ %25, %38 ]
  store i16** %74, i16*** %6, align 8, !tbaa !54
  %75 = load i16*, i16** %74, align 8, !tbaa !27
  %76 = getelementptr inbounds %"class.std::deque.5", %"class.std::deque.5"* %0, i64 0, i32 0, i32 0, i32 2, i32 1
  store i16* %75, i16** %76, align 8, !tbaa !55
  %77 = getelementptr inbounds i16, i16* %75, i64 256
  %78 = getelementptr inbounds %"class.std::deque.5", %"class.std::deque.5"* %0, i64 0, i32 0, i32 0, i32 2, i32 2
  store i16* %77, i16** %78, align 8, !tbaa !56
  %79 = getelementptr inbounds i16*, i16** %74, i64 %11
  store i16** %79, i16*** %4, align 8, !tbaa !54
  %80 = load i16*, i16** %79, align 8, !tbaa !27
  %81 = getelementptr inbounds %"class.std::deque.5", %"class.std::deque.5"* %0, i64 0, i32 0, i32 0, i32 3, i32 1
  store i16* %80, i16** %81, align 8, !tbaa !55
  %82 = getelementptr inbounds i16, i16* %80, i64 256
  %83 = getelementptr inbounds %"class.std::deque.5", %"class.std::deque.5"* %0, i64 0, i32 0, i32 0, i32 3, i32 2
  store i16* %82, i16** %83, align 8, !tbaa !56
  ret void
}

; Function Attrs: uwtable
define void @queuePop16(%"class.MARC::ThreadSafeQueue.3"*, i16*) local_unnamed_addr #5 {
  %3 = tail call zeroext i1 @_ZN4MARC15ThreadSafeQueueIsE7waitPopERs(%"class.MARC::ThreadSafeQueue.3"* %0, i16* dereferenceable(2) %1)
  ret void
}

; Function Attrs: uwtable
define linkonce_odr zeroext i1 @_ZN4MARC15ThreadSafeQueueIsE7waitPopERs(%"class.MARC::ThreadSafeQueue.3"*, i16* dereferenceable(2)) local_unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %3 = alloca %"class.std::unique_lock", align 8
  %4 = bitcast %"class.std::unique_lock"* %3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %4) #8
  %5 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 1
  %6 = getelementptr inbounds %"class.std::unique_lock", %"class.std::unique_lock"* %3, i64 0, i32 0
  store %"class.std::mutex"* %5, %"class.std::mutex"** %6, align 8, !tbaa !32
  %7 = getelementptr inbounds %"class.std::unique_lock", %"class.std::unique_lock"* %3, i64 0, i32 1
  store i8 0, i8* %7, align 8, !tbaa !35
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %8, label %13

; <label>:8:                                      ; preds = %2
  %9 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %5, i64 0, i32 0, i32 0
  %10 = tail call i32 @pthread_mutex_lock(%union.pthread_mutex_t* nonnull %9) #8
  %11 = icmp eq i32 %10, 0
  br i1 %11, label %13, label %12

; <label>:12:                                     ; preds = %8
  tail call void @_ZSt20__throw_system_errori(i32 %10) #23
  unreachable

; <label>:13:                                     ; preds = %2, %8
  store i8 1, i8* %7, align 8, !tbaa !35
  %14 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 0, i32 0, i32 0
  %15 = load atomic i8, i8* %14 seq_cst, align 1
  %16 = and i8 %15, 1
  %17 = icmp eq i8 %16, 0
  br i1 %17, label %61, label %18

; <label>:18:                                     ; preds = %13
  %19 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 0
  %20 = load i16*, i16** %19, align 8, !tbaa !58
  %21 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 0
  %22 = load i16*, i16** %21, align 8, !tbaa !58
  %23 = icmp eq i16* %20, %22
  br i1 %23, label %24, label %34

; <label>:24:                                     ; preds = %18
  %25 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 3
  br label %26

; <label>:26:                                     ; preds = %30, %24
  %27 = load atomic i8, i8* %14 seq_cst, align 1
  %28 = and i8 %27, 1
  %29 = icmp eq i8 %28, 0
  br i1 %29, label %34, label %30

; <label>:30:                                     ; preds = %26
  call void @_ZNSt18condition_variable4waitERSt11unique_lockISt5mutexE(%"class.std::condition_variable"* nonnull %25, %"class.std::unique_lock"* nonnull dereferenceable(16) %3) #8
  %31 = load i16*, i16** %19, align 8, !tbaa !58
  %32 = load i16*, i16** %21, align 8, !tbaa !58
  %33 = icmp eq i16* %31, %32
  br i1 %33, label %26, label %34

; <label>:34:                                     ; preds = %30, %26, %18
  %35 = load atomic i8, i8* %14 seq_cst, align 1
  %36 = and i8 %35, 1
  %37 = icmp eq i8 %36, 0
  br i1 %37, label %61, label %38

; <label>:38:                                     ; preds = %34
  %39 = load i16*, i16** %21, align 8, !tbaa !58, !noalias !59
  %40 = load i16, i16* %39, align 2, !tbaa !44
  store i16 %40, i16* %1, align 2, !tbaa !44
  %41 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 2
  %42 = load i16*, i16** %41, align 8, !tbaa !62
  %43 = getelementptr inbounds i16, i16* %42, i64 -1
  %44 = icmp eq i16* %39, %43
  br i1 %44, label %47, label %45

; <label>:45:                                     ; preds = %38
  %46 = getelementptr inbounds i16, i16* %39, i64 1
  store i16* %46, i16** %21, align 8, !tbaa !63
  br label %59

; <label>:47:                                     ; preds = %38
  %48 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2
  %49 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 1
  %50 = bitcast i16** %49 to i8**
  %51 = load i8*, i8** %50, align 8, !tbaa !64
  call void @_ZdlPv(i8* %51) #8
  %52 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 3
  %53 = load i16**, i16*** %52, align 8, !tbaa !57
  %54 = getelementptr inbounds i16*, i16** %53, i64 1
  store i16** %54, i16*** %52, align 8, !tbaa !54
  %55 = load i16*, i16** %54, align 8, !tbaa !27
  store i16* %55, i16** %49, align 8, !tbaa !55
  %56 = getelementptr inbounds i16, i16* %55, i64 256
  store i16* %56, i16** %41, align 8, !tbaa !56
  %57 = ptrtoint i16* %55 to i64
  %58 = bitcast %"struct.std::_Deque_iterator.10"* %48 to i64*
  store i64 %57, i64* %58, align 8, !tbaa !63
  br label %59

; <label>:59:                                     ; preds = %45, %47
  %60 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 4
  call void @_ZNSt18condition_variable10notify_oneEv(%"class.std::condition_variable"* %60) #8
  br label %61

; <label>:61:                                     ; preds = %34, %13, %59
  %62 = phi i1 [ false, %13 ], [ false, %34 ], [ true, %59 ]
  %63 = load i8, i8* %7, align 8, !tbaa !35, !range !43
  %64 = icmp eq i8 %63, 0
  br i1 %64, label %73, label %65

; <label>:65:                                     ; preds = %61
  %66 = load %"class.std::mutex"*, %"class.std::mutex"** %6, align 8, !tbaa !32
  %67 = icmp eq %"class.std::mutex"* %66, null
  br i1 %67, label %73, label %68

; <label>:68:                                     ; preds = %65
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %69, label %72

; <label>:69:                                     ; preds = %68
  %70 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %66, i64 0, i32 0, i32 0
  %71 = call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %70) #8
  br label %72

; <label>:72:                                     ; preds = %69, %68
  store i8 0, i8* %7, align 8, !tbaa !35
  br label %73

; <label>:73:                                     ; preds = %61, %65, %72
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %4) #8
  ret i1 %62
}

; Function Attrs: uwtable
define void @queuePush32(%"class.MARC::ThreadSafeQueue.11"*, i32* nocapture readonly) local_unnamed_addr #5 {
  %3 = load i32, i32* %1, align 4, !tbaa !17
  tail call void @_ZN4MARC15ThreadSafeQueueIiE4pushEi(%"class.MARC::ThreadSafeQueue.11"* %0, i32 %3)
  ret void
}

; Function Attrs: uwtable
define linkonce_odr void @_ZN4MARC15ThreadSafeQueueIiE4pushEi(%"class.MARC::ThreadSafeQueue.11"*, i32) local_unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %3 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %4, label %9

; <label>:4:                                      ; preds = %2
  %5 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %3, i64 0, i32 0, i32 0
  %6 = tail call i32 @pthread_mutex_lock(%union.pthread_mutex_t* nonnull %5) #8
  %7 = icmp eq i32 %6, 0
  br i1 %7, label %9, label %8

; <label>:8:                                      ; preds = %4
  tail call void @_ZSt20__throw_system_errori(i32 %6) #23
  unreachable

; <label>:9:                                      ; preds = %2, %4
  %10 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 2
  %11 = getelementptr inbounds %"class.std::queue.12", %"class.std::queue.12"* %10, i64 0, i32 0
  %12 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 0
  %13 = load i32*, i32** %12, align 8, !tbaa !65
  %14 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 2
  %15 = load i32*, i32** %14, align 8, !tbaa !69
  %16 = getelementptr inbounds i32, i32* %15, i64 -1
  %17 = icmp eq i32* %13, %16
  br i1 %17, label %20, label %18

; <label>:18:                                     ; preds = %9
  store i32 %1, i32* %13, align 4, !tbaa !17
  %19 = getelementptr inbounds i32, i32* %13, i64 1
  store i32* %19, i32** %12, align 8, !tbaa !65
  br label %49

; <label>:20:                                     ; preds = %9
  %21 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 1
  %22 = load i64, i64* %21, align 8, !tbaa !70
  %23 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 3
  %24 = bitcast i32*** %23 to i64*
  %25 = load i64, i64* %24, align 8, !tbaa !71
  %26 = bitcast %"class.std::queue.12"* %10 to i64*
  %27 = load i64, i64* %26, align 8, !tbaa !72
  %28 = sub i64 %25, %27
  %29 = ashr exact i64 %28, 3
  %30 = sub i64 %22, %29
  %31 = icmp ult i64 %30, 2
  br i1 %31, label %32, label %33

; <label>:32:                                     ; preds = %20
  invoke void @_ZNSt5dequeIiSaIiEE17_M_reallocate_mapEmb(%"class.std::deque.13"* nonnull %11, i64 1, i1 zeroext false)
          to label %33 unwind label %55

; <label>:33:                                     ; preds = %32, %20
  %34 = invoke i8* @_Znwm(i64 512)
          to label %35 unwind label %55

; <label>:35:                                     ; preds = %33
  %36 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3
  %37 = load i32**, i32*** %23, align 8, !tbaa !71
  %38 = getelementptr inbounds i32*, i32** %37, i64 1
  %39 = bitcast i32** %38 to i8**
  store i8* %34, i8** %39, align 8, !tbaa !27
  %40 = getelementptr inbounds %"struct.std::_Deque_iterator.18", %"struct.std::_Deque_iterator.18"* %36, i64 0, i32 0
  %41 = load i32*, i32** %40, align 8, !tbaa !65
  store i32 %1, i32* %41, align 4, !tbaa !17
  %42 = load i32**, i32*** %23, align 8, !tbaa !71
  %43 = getelementptr inbounds i32*, i32** %42, i64 1
  store i32** %43, i32*** %23, align 8, !tbaa !73
  %44 = load i32*, i32** %43, align 8, !tbaa !27
  %45 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 1
  store i32* %44, i32** %45, align 8, !tbaa !74
  %46 = getelementptr inbounds i32, i32* %44, i64 128
  store i32* %46, i32** %14, align 8, !tbaa !75
  %47 = ptrtoint i32* %44 to i64
  %48 = bitcast %"struct.std::_Deque_iterator.18"* %36 to i64*
  store i64 %47, i64* %48, align 8, !tbaa !65
  br label %49

; <label>:49:                                     ; preds = %35, %18
  %50 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 3
  tail call void @_ZNSt18condition_variable10notify_oneEv(%"class.std::condition_variable"* %50) #8
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %51, label %54

; <label>:51:                                     ; preds = %49
  %52 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %3, i64 0, i32 0, i32 0
  %53 = tail call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %52) #8
  br label %54

; <label>:54:                                     ; preds = %49, %51
  ret void

; <label>:55:                                     ; preds = %33, %32
  %56 = landingpad { i8*, i32 }
          cleanup
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %57, label %60

; <label>:57:                                     ; preds = %55
  %58 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %3, i64 0, i32 0, i32 0
  %59 = tail call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %58) #8
  br label %60

; <label>:60:                                     ; preds = %55, %57
  resume { i8*, i32 } %56
}

; Function Attrs: uwtable
define linkonce_odr void @_ZNSt5dequeIiSaIiEE17_M_reallocate_mapEmb(%"class.std::deque.13"*, i64, i1 zeroext) local_unnamed_addr #5 comdat align 2 personality i32 (...)* @__gxx_personality_v0 {
  %4 = getelementptr inbounds %"class.std::deque.13", %"class.std::deque.13"* %0, i64 0, i32 0, i32 0, i32 3, i32 3
  %5 = load i32**, i32*** %4, align 8, !tbaa !71
  %6 = getelementptr inbounds %"class.std::deque.13", %"class.std::deque.13"* %0, i64 0, i32 0, i32 0, i32 2, i32 3
  %7 = load i32**, i32*** %6, align 8, !tbaa !76
  %8 = ptrtoint i32** %5 to i64
  %9 = ptrtoint i32** %7 to i64
  %10 = sub i64 %8, %9
  %11 = ashr exact i64 %10, 3
  %12 = add nsw i64 %11, 1
  %13 = add i64 %12, %1
  %14 = getelementptr inbounds %"class.std::deque.13", %"class.std::deque.13"* %0, i64 0, i32 0, i32 0, i32 1
  %15 = load i64, i64* %14, align 8, !tbaa !70
  %16 = shl i64 %13, 1
  %17 = icmp ugt i64 %15, %16
  br i1 %17, label %18, label %44

; <label>:18:                                     ; preds = %3
  %19 = getelementptr inbounds %"class.std::deque.13", %"class.std::deque.13"* %0, i64 0, i32 0, i32 0, i32 0
  %20 = load i32**, i32*** %19, align 8, !tbaa !72
  %21 = sub i64 %15, %13
  %22 = lshr i64 %21, 1
  %23 = getelementptr inbounds i32*, i32** %20, i64 %22
  %24 = select i1 %2, i64 %1, i64 0
  %25 = getelementptr inbounds i32*, i32** %23, i64 %24
  %26 = icmp ult i32** %25, %7
  %27 = getelementptr inbounds i32*, i32** %5, i64 1
  %28 = ptrtoint i32** %27 to i64
  %29 = sub i64 %28, %9
  br i1 %26, label %30, label %35

; <label>:30:                                     ; preds = %18
  %31 = icmp eq i64 %29, 0
  br i1 %31, label %73, label %32

; <label>:32:                                     ; preds = %30
  %33 = bitcast i32** %25 to i8*
  %34 = bitcast i32** %7 to i8*
  tail call void @llvm.memmove.p0i8.p0i8.i64(i8* %33, i8* %34, i64 %29, i32 8, i1 false) #8
  br label %73

; <label>:35:                                     ; preds = %18
  %36 = ashr exact i64 %29, 3
  %37 = icmp eq i64 %36, 0
  br i1 %37, label %73, label %38

; <label>:38:                                     ; preds = %35
  %39 = getelementptr inbounds i32*, i32** %25, i64 %12
  %40 = sub nsw i64 0, %36
  %41 = getelementptr inbounds i32*, i32** %39, i64 %40
  %42 = bitcast i32** %41 to i8*
  %43 = bitcast i32** %7 to i8*
  tail call void @llvm.memmove.p0i8.p0i8.i64(i8* %42, i8* %43, i64 %29, i32 8, i1 false) #8
  br label %73

; <label>:44:                                     ; preds = %3
  %45 = icmp ult i64 %15, %1
  %46 = select i1 %45, i64 %1, i64 %15
  %47 = add i64 %15, 2
  %48 = add i64 %47, %46
  %49 = icmp ugt i64 %48, 2305843009213693951
  br i1 %49, label %50, label %51

; <label>:50:                                     ; preds = %44
  tail call void @_ZSt17__throw_bad_allocv() #23
  unreachable

; <label>:51:                                     ; preds = %44
  %52 = shl i64 %48, 3
  %53 = tail call i8* @_Znwm(i64 %52)
  %54 = bitcast i8* %53 to i32**
  %55 = sub i64 %48, %13
  %56 = lshr i64 %55, 1
  %57 = getelementptr inbounds i32*, i32** %54, i64 %56
  %58 = select i1 %2, i64 %1, i64 0
  %59 = getelementptr inbounds i32*, i32** %57, i64 %58
  %60 = load i32**, i32*** %6, align 8, !tbaa !76
  %61 = load i32**, i32*** %4, align 8, !tbaa !71
  %62 = getelementptr inbounds i32*, i32** %61, i64 1
  %63 = ptrtoint i32** %62 to i64
  %64 = ptrtoint i32** %60 to i64
  %65 = sub i64 %63, %64
  %66 = icmp eq i64 %65, 0
  br i1 %66, label %70, label %67

; <label>:67:                                     ; preds = %51
  %68 = bitcast i32** %59 to i8*
  %69 = bitcast i32** %60 to i8*
  tail call void @llvm.memmove.p0i8.p0i8.i64(i8* %68, i8* %69, i64 %65, i32 8, i1 false) #8
  br label %70

; <label>:70:                                     ; preds = %51, %67
  %71 = bitcast %"class.std::deque.13"* %0 to i8**
  %72 = load i8*, i8** %71, align 8, !tbaa !72
  tail call void @_ZdlPv(i8* %72) #8
  store i8* %53, i8** %71, align 8, !tbaa !72
  store i64 %48, i64* %14, align 8, !tbaa !70
  br label %73

; <label>:73:                                     ; preds = %38, %35, %32, %30, %70
  %74 = phi i32** [ %59, %70 ], [ %25, %30 ], [ %25, %32 ], [ %25, %35 ], [ %25, %38 ]
  store i32** %74, i32*** %6, align 8, !tbaa !73
  %75 = load i32*, i32** %74, align 8, !tbaa !27
  %76 = getelementptr inbounds %"class.std::deque.13", %"class.std::deque.13"* %0, i64 0, i32 0, i32 0, i32 2, i32 1
  store i32* %75, i32** %76, align 8, !tbaa !74
  %77 = getelementptr inbounds i32, i32* %75, i64 128
  %78 = getelementptr inbounds %"class.std::deque.13", %"class.std::deque.13"* %0, i64 0, i32 0, i32 0, i32 2, i32 2
  store i32* %77, i32** %78, align 8, !tbaa !75
  %79 = getelementptr inbounds i32*, i32** %74, i64 %11
  store i32** %79, i32*** %4, align 8, !tbaa !73
  %80 = load i32*, i32** %79, align 8, !tbaa !27
  %81 = getelementptr inbounds %"class.std::deque.13", %"class.std::deque.13"* %0, i64 0, i32 0, i32 0, i32 3, i32 1
  store i32* %80, i32** %81, align 8, !tbaa !74
  %82 = getelementptr inbounds i32, i32* %80, i64 128
  %83 = getelementptr inbounds %"class.std::deque.13", %"class.std::deque.13"* %0, i64 0, i32 0, i32 0, i32 3, i32 2
  store i32* %82, i32** %83, align 8, !tbaa !75
  ret void
}

; Function Attrs: uwtable
define void @queuePop32(%"class.MARC::ThreadSafeQueue.11"*, i32*) local_unnamed_addr #5 {
  %3 = tail call zeroext i1 @_ZN4MARC15ThreadSafeQueueIiE7waitPopERi(%"class.MARC::ThreadSafeQueue.11"* %0, i32* dereferenceable(4) %1)
  ret void
}

; Function Attrs: uwtable
define linkonce_odr zeroext i1 @_ZN4MARC15ThreadSafeQueueIiE7waitPopERi(%"class.MARC::ThreadSafeQueue.11"*, i32* dereferenceable(4)) local_unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %3 = alloca %"class.std::unique_lock", align 8
  %4 = bitcast %"class.std::unique_lock"* %3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %4) #8
  %5 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 1
  %6 = getelementptr inbounds %"class.std::unique_lock", %"class.std::unique_lock"* %3, i64 0, i32 0
  store %"class.std::mutex"* %5, %"class.std::mutex"** %6, align 8, !tbaa !32
  %7 = getelementptr inbounds %"class.std::unique_lock", %"class.std::unique_lock"* %3, i64 0, i32 1
  store i8 0, i8* %7, align 8, !tbaa !35
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %8, label %13

; <label>:8:                                      ; preds = %2
  %9 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %5, i64 0, i32 0, i32 0
  %10 = tail call i32 @pthread_mutex_lock(%union.pthread_mutex_t* nonnull %9) #8
  %11 = icmp eq i32 %10, 0
  br i1 %11, label %13, label %12

; <label>:12:                                     ; preds = %8
  tail call void @_ZSt20__throw_system_errori(i32 %10) #23
  unreachable

; <label>:13:                                     ; preds = %2, %8
  store i8 1, i8* %7, align 8, !tbaa !35
  %14 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 0, i32 0, i32 0
  %15 = load atomic i8, i8* %14 seq_cst, align 1
  %16 = and i8 %15, 1
  %17 = icmp eq i8 %16, 0
  br i1 %17, label %61, label %18

; <label>:18:                                     ; preds = %13
  %19 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 0
  %20 = load i32*, i32** %19, align 8, !tbaa !77
  %21 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 0
  %22 = load i32*, i32** %21, align 8, !tbaa !77
  %23 = icmp eq i32* %20, %22
  br i1 %23, label %24, label %34

; <label>:24:                                     ; preds = %18
  %25 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 3
  br label %26

; <label>:26:                                     ; preds = %30, %24
  %27 = load atomic i8, i8* %14 seq_cst, align 1
  %28 = and i8 %27, 1
  %29 = icmp eq i8 %28, 0
  br i1 %29, label %34, label %30

; <label>:30:                                     ; preds = %26
  call void @_ZNSt18condition_variable4waitERSt11unique_lockISt5mutexE(%"class.std::condition_variable"* nonnull %25, %"class.std::unique_lock"* nonnull dereferenceable(16) %3) #8
  %31 = load i32*, i32** %19, align 8, !tbaa !77
  %32 = load i32*, i32** %21, align 8, !tbaa !77
  %33 = icmp eq i32* %31, %32
  br i1 %33, label %26, label %34

; <label>:34:                                     ; preds = %30, %26, %18
  %35 = load atomic i8, i8* %14 seq_cst, align 1
  %36 = and i8 %35, 1
  %37 = icmp eq i8 %36, 0
  br i1 %37, label %61, label %38

; <label>:38:                                     ; preds = %34
  %39 = load i32*, i32** %21, align 8, !tbaa !77, !noalias !78
  %40 = load i32, i32* %39, align 4, !tbaa !17
  store i32 %40, i32* %1, align 4, !tbaa !17
  %41 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 2
  %42 = load i32*, i32** %41, align 8, !tbaa !81
  %43 = getelementptr inbounds i32, i32* %42, i64 -1
  %44 = icmp eq i32* %39, %43
  br i1 %44, label %47, label %45

; <label>:45:                                     ; preds = %38
  %46 = getelementptr inbounds i32, i32* %39, i64 1
  store i32* %46, i32** %21, align 8, !tbaa !82
  br label %59

; <label>:47:                                     ; preds = %38
  %48 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2
  %49 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 1
  %50 = bitcast i32** %49 to i8**
  %51 = load i8*, i8** %50, align 8, !tbaa !83
  call void @_ZdlPv(i8* %51) #8
  %52 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 3
  %53 = load i32**, i32*** %52, align 8, !tbaa !76
  %54 = getelementptr inbounds i32*, i32** %53, i64 1
  store i32** %54, i32*** %52, align 8, !tbaa !73
  %55 = load i32*, i32** %54, align 8, !tbaa !27
  store i32* %55, i32** %49, align 8, !tbaa !74
  %56 = getelementptr inbounds i32, i32* %55, i64 128
  store i32* %56, i32** %41, align 8, !tbaa !75
  %57 = ptrtoint i32* %55 to i64
  %58 = bitcast %"struct.std::_Deque_iterator.18"* %48 to i64*
  store i64 %57, i64* %58, align 8, !tbaa !82
  br label %59

; <label>:59:                                     ; preds = %45, %47
  %60 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 4
  call void @_ZNSt18condition_variable10notify_oneEv(%"class.std::condition_variable"* %60) #8
  br label %61

; <label>:61:                                     ; preds = %34, %13, %59
  %62 = phi i1 [ false, %13 ], [ false, %34 ], [ true, %59 ]
  %63 = load i8, i8* %7, align 8, !tbaa !35, !range !43
  %64 = icmp eq i8 %63, 0
  br i1 %64, label %73, label %65

; <label>:65:                                     ; preds = %61
  %66 = load %"class.std::mutex"*, %"class.std::mutex"** %6, align 8, !tbaa !32
  %67 = icmp eq %"class.std::mutex"* %66, null
  br i1 %67, label %73, label %68

; <label>:68:                                     ; preds = %65
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %69, label %72

; <label>:69:                                     ; preds = %68
  %70 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %66, i64 0, i32 0, i32 0
  %71 = call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %70) #8
  br label %72

; <label>:72:                                     ; preds = %69, %68
  store i8 0, i8* %7, align 8, !tbaa !35
  br label %73

; <label>:73:                                     ; preds = %61, %65, %72
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %4) #8
  ret i1 %62
}

; Function Attrs: uwtable
define void @queuePush64(%"class.MARC::ThreadSafeQueue.19"*, i64* nocapture readonly) local_unnamed_addr #5 {
  %3 = load i64, i64* %1, align 8, !tbaa !84
  tail call void @_ZN4MARC15ThreadSafeQueueIlE4pushEl(%"class.MARC::ThreadSafeQueue.19"* %0, i64 %3)
  ret void
}

; Function Attrs: uwtable
define linkonce_odr void @_ZN4MARC15ThreadSafeQueueIlE4pushEl(%"class.MARC::ThreadSafeQueue.19"*, i64) local_unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %3 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %4, label %9

; <label>:4:                                      ; preds = %2
  %5 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %3, i64 0, i32 0, i32 0
  %6 = tail call i32 @pthread_mutex_lock(%union.pthread_mutex_t* nonnull %5) #8
  %7 = icmp eq i32 %6, 0
  br i1 %7, label %9, label %8

; <label>:8:                                      ; preds = %4
  tail call void @_ZSt20__throw_system_errori(i32 %6) #23
  unreachable

; <label>:9:                                      ; preds = %2, %4
  %10 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 2
  %11 = getelementptr inbounds %"class.std::queue.20", %"class.std::queue.20"* %10, i64 0, i32 0
  %12 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 0
  %13 = load i64*, i64** %12, align 8, !tbaa !85
  %14 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 2
  %15 = load i64*, i64** %14, align 8, !tbaa !89
  %16 = getelementptr inbounds i64, i64* %15, i64 -1
  %17 = icmp eq i64* %13, %16
  br i1 %17, label %20, label %18

; <label>:18:                                     ; preds = %9
  store i64 %1, i64* %13, align 8, !tbaa !84
  %19 = getelementptr inbounds i64, i64* %13, i64 1
  store i64* %19, i64** %12, align 8, !tbaa !85
  br label %49

; <label>:20:                                     ; preds = %9
  %21 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 1
  %22 = load i64, i64* %21, align 8, !tbaa !90
  %23 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 3
  %24 = bitcast i64*** %23 to i64*
  %25 = load i64, i64* %24, align 8, !tbaa !91
  %26 = bitcast %"class.std::queue.20"* %10 to i64*
  %27 = load i64, i64* %26, align 8, !tbaa !92
  %28 = sub i64 %25, %27
  %29 = ashr exact i64 %28, 3
  %30 = sub i64 %22, %29
  %31 = icmp ult i64 %30, 2
  br i1 %31, label %32, label %33

; <label>:32:                                     ; preds = %20
  invoke void @_ZNSt5dequeIlSaIlEE17_M_reallocate_mapEmb(%"class.std::deque.21"* nonnull %11, i64 1, i1 zeroext false)
          to label %33 unwind label %55

; <label>:33:                                     ; preds = %32, %20
  %34 = invoke i8* @_Znwm(i64 512)
          to label %35 unwind label %55

; <label>:35:                                     ; preds = %33
  %36 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3
  %37 = load i64**, i64*** %23, align 8, !tbaa !91
  %38 = getelementptr inbounds i64*, i64** %37, i64 1
  %39 = bitcast i64** %38 to i8**
  store i8* %34, i8** %39, align 8, !tbaa !27
  %40 = getelementptr inbounds %"struct.std::_Deque_iterator.26", %"struct.std::_Deque_iterator.26"* %36, i64 0, i32 0
  %41 = load i64*, i64** %40, align 8, !tbaa !85
  store i64 %1, i64* %41, align 8, !tbaa !84
  %42 = load i64**, i64*** %23, align 8, !tbaa !91
  %43 = getelementptr inbounds i64*, i64** %42, i64 1
  store i64** %43, i64*** %23, align 8, !tbaa !93
  %44 = load i64*, i64** %43, align 8, !tbaa !27
  %45 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 1
  store i64* %44, i64** %45, align 8, !tbaa !94
  %46 = getelementptr inbounds i64, i64* %44, i64 64
  store i64* %46, i64** %14, align 8, !tbaa !95
  %47 = ptrtoint i64* %44 to i64
  %48 = bitcast %"struct.std::_Deque_iterator.26"* %36 to i64*
  store i64 %47, i64* %48, align 8, !tbaa !85
  br label %49

; <label>:49:                                     ; preds = %35, %18
  %50 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 3
  tail call void @_ZNSt18condition_variable10notify_oneEv(%"class.std::condition_variable"* %50) #8
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %51, label %54

; <label>:51:                                     ; preds = %49
  %52 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %3, i64 0, i32 0, i32 0
  %53 = tail call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %52) #8
  br label %54

; <label>:54:                                     ; preds = %49, %51
  ret void

; <label>:55:                                     ; preds = %33, %32
  %56 = landingpad { i8*, i32 }
          cleanup
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %57, label %60

; <label>:57:                                     ; preds = %55
  %58 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %3, i64 0, i32 0, i32 0
  %59 = tail call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %58) #8
  br label %60

; <label>:60:                                     ; preds = %55, %57
  resume { i8*, i32 } %56
}

; Function Attrs: uwtable
define linkonce_odr void @_ZNSt5dequeIlSaIlEE17_M_reallocate_mapEmb(%"class.std::deque.21"*, i64, i1 zeroext) local_unnamed_addr #5 comdat align 2 personality i32 (...)* @__gxx_personality_v0 {
  %4 = getelementptr inbounds %"class.std::deque.21", %"class.std::deque.21"* %0, i64 0, i32 0, i32 0, i32 3, i32 3
  %5 = load i64**, i64*** %4, align 8, !tbaa !91
  %6 = getelementptr inbounds %"class.std::deque.21", %"class.std::deque.21"* %0, i64 0, i32 0, i32 0, i32 2, i32 3
  %7 = load i64**, i64*** %6, align 8, !tbaa !96
  %8 = ptrtoint i64** %5 to i64
  %9 = ptrtoint i64** %7 to i64
  %10 = sub i64 %8, %9
  %11 = ashr exact i64 %10, 3
  %12 = add nsw i64 %11, 1
  %13 = add i64 %12, %1
  %14 = getelementptr inbounds %"class.std::deque.21", %"class.std::deque.21"* %0, i64 0, i32 0, i32 0, i32 1
  %15 = load i64, i64* %14, align 8, !tbaa !90
  %16 = shl i64 %13, 1
  %17 = icmp ugt i64 %15, %16
  br i1 %17, label %18, label %44

; <label>:18:                                     ; preds = %3
  %19 = getelementptr inbounds %"class.std::deque.21", %"class.std::deque.21"* %0, i64 0, i32 0, i32 0, i32 0
  %20 = load i64**, i64*** %19, align 8, !tbaa !92
  %21 = sub i64 %15, %13
  %22 = lshr i64 %21, 1
  %23 = getelementptr inbounds i64*, i64** %20, i64 %22
  %24 = select i1 %2, i64 %1, i64 0
  %25 = getelementptr inbounds i64*, i64** %23, i64 %24
  %26 = icmp ult i64** %25, %7
  %27 = getelementptr inbounds i64*, i64** %5, i64 1
  %28 = ptrtoint i64** %27 to i64
  %29 = sub i64 %28, %9
  br i1 %26, label %30, label %35

; <label>:30:                                     ; preds = %18
  %31 = icmp eq i64 %29, 0
  br i1 %31, label %73, label %32

; <label>:32:                                     ; preds = %30
  %33 = bitcast i64** %25 to i8*
  %34 = bitcast i64** %7 to i8*
  tail call void @llvm.memmove.p0i8.p0i8.i64(i8* %33, i8* %34, i64 %29, i32 8, i1 false) #8
  br label %73

; <label>:35:                                     ; preds = %18
  %36 = ashr exact i64 %29, 3
  %37 = icmp eq i64 %36, 0
  br i1 %37, label %73, label %38

; <label>:38:                                     ; preds = %35
  %39 = getelementptr inbounds i64*, i64** %25, i64 %12
  %40 = sub nsw i64 0, %36
  %41 = getelementptr inbounds i64*, i64** %39, i64 %40
  %42 = bitcast i64** %41 to i8*
  %43 = bitcast i64** %7 to i8*
  tail call void @llvm.memmove.p0i8.p0i8.i64(i8* %42, i8* %43, i64 %29, i32 8, i1 false) #8
  br label %73

; <label>:44:                                     ; preds = %3
  %45 = icmp ult i64 %15, %1
  %46 = select i1 %45, i64 %1, i64 %15
  %47 = add i64 %15, 2
  %48 = add i64 %47, %46
  %49 = icmp ugt i64 %48, 2305843009213693951
  br i1 %49, label %50, label %51

; <label>:50:                                     ; preds = %44
  tail call void @_ZSt17__throw_bad_allocv() #23
  unreachable

; <label>:51:                                     ; preds = %44
  %52 = shl i64 %48, 3
  %53 = tail call i8* @_Znwm(i64 %52)
  %54 = bitcast i8* %53 to i64**
  %55 = sub i64 %48, %13
  %56 = lshr i64 %55, 1
  %57 = getelementptr inbounds i64*, i64** %54, i64 %56
  %58 = select i1 %2, i64 %1, i64 0
  %59 = getelementptr inbounds i64*, i64** %57, i64 %58
  %60 = load i64**, i64*** %6, align 8, !tbaa !96
  %61 = load i64**, i64*** %4, align 8, !tbaa !91
  %62 = getelementptr inbounds i64*, i64** %61, i64 1
  %63 = ptrtoint i64** %62 to i64
  %64 = ptrtoint i64** %60 to i64
  %65 = sub i64 %63, %64
  %66 = icmp eq i64 %65, 0
  br i1 %66, label %70, label %67

; <label>:67:                                     ; preds = %51
  %68 = bitcast i64** %59 to i8*
  %69 = bitcast i64** %60 to i8*
  tail call void @llvm.memmove.p0i8.p0i8.i64(i8* %68, i8* %69, i64 %65, i32 8, i1 false) #8
  br label %70

; <label>:70:                                     ; preds = %51, %67
  %71 = bitcast %"class.std::deque.21"* %0 to i8**
  %72 = load i8*, i8** %71, align 8, !tbaa !92
  tail call void @_ZdlPv(i8* %72) #8
  store i8* %53, i8** %71, align 8, !tbaa !92
  store i64 %48, i64* %14, align 8, !tbaa !90
  br label %73

; <label>:73:                                     ; preds = %38, %35, %32, %30, %70
  %74 = phi i64** [ %59, %70 ], [ %25, %30 ], [ %25, %32 ], [ %25, %35 ], [ %25, %38 ]
  store i64** %74, i64*** %6, align 8, !tbaa !93
  %75 = load i64*, i64** %74, align 8, !tbaa !27
  %76 = getelementptr inbounds %"class.std::deque.21", %"class.std::deque.21"* %0, i64 0, i32 0, i32 0, i32 2, i32 1
  store i64* %75, i64** %76, align 8, !tbaa !94
  %77 = getelementptr inbounds i64, i64* %75, i64 64
  %78 = getelementptr inbounds %"class.std::deque.21", %"class.std::deque.21"* %0, i64 0, i32 0, i32 0, i32 2, i32 2
  store i64* %77, i64** %78, align 8, !tbaa !95
  %79 = getelementptr inbounds i64*, i64** %74, i64 %11
  store i64** %79, i64*** %4, align 8, !tbaa !93
  %80 = load i64*, i64** %79, align 8, !tbaa !27
  %81 = getelementptr inbounds %"class.std::deque.21", %"class.std::deque.21"* %0, i64 0, i32 0, i32 0, i32 3, i32 1
  store i64* %80, i64** %81, align 8, !tbaa !94
  %82 = getelementptr inbounds i64, i64* %80, i64 64
  %83 = getelementptr inbounds %"class.std::deque.21", %"class.std::deque.21"* %0, i64 0, i32 0, i32 0, i32 3, i32 2
  store i64* %82, i64** %83, align 8, !tbaa !95
  ret void
}

; Function Attrs: uwtable
define void @queuePop64(%"class.MARC::ThreadSafeQueue.19"*, i64*) local_unnamed_addr #5 {
  %3 = tail call zeroext i1 @_ZN4MARC15ThreadSafeQueueIlE7waitPopERl(%"class.MARC::ThreadSafeQueue.19"* %0, i64* dereferenceable(8) %1)
  ret void
}

; Function Attrs: uwtable
define linkonce_odr zeroext i1 @_ZN4MARC15ThreadSafeQueueIlE7waitPopERl(%"class.MARC::ThreadSafeQueue.19"*, i64* dereferenceable(8)) local_unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %3 = alloca %"class.std::unique_lock", align 8
  %4 = bitcast %"class.std::unique_lock"* %3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %4) #8
  %5 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 1
  %6 = getelementptr inbounds %"class.std::unique_lock", %"class.std::unique_lock"* %3, i64 0, i32 0
  store %"class.std::mutex"* %5, %"class.std::mutex"** %6, align 8, !tbaa !32
  %7 = getelementptr inbounds %"class.std::unique_lock", %"class.std::unique_lock"* %3, i64 0, i32 1
  store i8 0, i8* %7, align 8, !tbaa !35
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %8, label %13

; <label>:8:                                      ; preds = %2
  %9 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %5, i64 0, i32 0, i32 0
  %10 = tail call i32 @pthread_mutex_lock(%union.pthread_mutex_t* nonnull %9) #8
  %11 = icmp eq i32 %10, 0
  br i1 %11, label %13, label %12

; <label>:12:                                     ; preds = %8
  tail call void @_ZSt20__throw_system_errori(i32 %10) #23
  unreachable

; <label>:13:                                     ; preds = %2, %8
  store i8 1, i8* %7, align 8, !tbaa !35
  %14 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 0, i32 0, i32 0
  %15 = load atomic i8, i8* %14 seq_cst, align 1
  %16 = and i8 %15, 1
  %17 = icmp eq i8 %16, 0
  br i1 %17, label %61, label %18

; <label>:18:                                     ; preds = %13
  %19 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 0
  %20 = load i64*, i64** %19, align 8, !tbaa !97
  %21 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 0
  %22 = load i64*, i64** %21, align 8, !tbaa !97
  %23 = icmp eq i64* %20, %22
  br i1 %23, label %24, label %34

; <label>:24:                                     ; preds = %18
  %25 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 3
  br label %26

; <label>:26:                                     ; preds = %30, %24
  %27 = load atomic i8, i8* %14 seq_cst, align 1
  %28 = and i8 %27, 1
  %29 = icmp eq i8 %28, 0
  br i1 %29, label %34, label %30

; <label>:30:                                     ; preds = %26
  call void @_ZNSt18condition_variable4waitERSt11unique_lockISt5mutexE(%"class.std::condition_variable"* nonnull %25, %"class.std::unique_lock"* nonnull dereferenceable(16) %3) #8
  %31 = load i64*, i64** %19, align 8, !tbaa !97
  %32 = load i64*, i64** %21, align 8, !tbaa !97
  %33 = icmp eq i64* %31, %32
  br i1 %33, label %26, label %34

; <label>:34:                                     ; preds = %30, %26, %18
  %35 = load atomic i8, i8* %14 seq_cst, align 1
  %36 = and i8 %35, 1
  %37 = icmp eq i8 %36, 0
  br i1 %37, label %61, label %38

; <label>:38:                                     ; preds = %34
  %39 = load i64*, i64** %21, align 8, !tbaa !97, !noalias !98
  %40 = load i64, i64* %39, align 8, !tbaa !84
  store i64 %40, i64* %1, align 8, !tbaa !84
  %41 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 2
  %42 = load i64*, i64** %41, align 8, !tbaa !101
  %43 = getelementptr inbounds i64, i64* %42, i64 -1
  %44 = icmp eq i64* %39, %43
  br i1 %44, label %47, label %45

; <label>:45:                                     ; preds = %38
  %46 = getelementptr inbounds i64, i64* %39, i64 1
  store i64* %46, i64** %21, align 8, !tbaa !102
  br label %59

; <label>:47:                                     ; preds = %38
  %48 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2
  %49 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 1
  %50 = bitcast i64** %49 to i8**
  %51 = load i8*, i8** %50, align 8, !tbaa !103
  call void @_ZdlPv(i8* %51) #8
  %52 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 3
  %53 = load i64**, i64*** %52, align 8, !tbaa !96
  %54 = getelementptr inbounds i64*, i64** %53, i64 1
  store i64** %54, i64*** %52, align 8, !tbaa !93
  %55 = load i64*, i64** %54, align 8, !tbaa !27
  store i64* %55, i64** %49, align 8, !tbaa !94
  %56 = getelementptr inbounds i64, i64* %55, i64 64
  store i64* %56, i64** %41, align 8, !tbaa !95
  %57 = ptrtoint i64* %55 to i64
  %58 = bitcast %"struct.std::_Deque_iterator.26"* %48 to i64*
  store i64 %57, i64* %58, align 8, !tbaa !102
  br label %59

; <label>:59:                                     ; preds = %45, %47
  %60 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 4
  call void @_ZNSt18condition_variable10notify_oneEv(%"class.std::condition_variable"* %60) #8
  br label %61

; <label>:61:                                     ; preds = %34, %13, %59
  %62 = phi i1 [ false, %13 ], [ false, %34 ], [ true, %59 ]
  %63 = load i8, i8* %7, align 8, !tbaa !35, !range !43
  %64 = icmp eq i8 %63, 0
  br i1 %64, label %73, label %65

; <label>:65:                                     ; preds = %61
  %66 = load %"class.std::mutex"*, %"class.std::mutex"** %6, align 8, !tbaa !32
  %67 = icmp eq %"class.std::mutex"* %66, null
  br i1 %67, label %73, label %68

; <label>:68:                                     ; preds = %65
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %69, label %72

; <label>:69:                                     ; preds = %68
  %70 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %66, i64 0, i32 0, i32 0
  %71 = call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %70) #8
  br label %72

; <label>:72:                                     ; preds = %69, %68
  store i8 0, i8* %7, align 8, !tbaa !35
  br label %73

; <label>:73:                                     ; preds = %61, %65, %72
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %4) #8
  ret i1 %62
}

; Function Attrs: uwtable
define void @queuePush(%"class.MARC::ThreadSafeQueue"*, i8* nocapture readonly, i64) local_unnamed_addr #5 {
  %4 = icmp sgt i64 %2, 0
  br i1 %4, label %5, label %6

; <label>:5:                                      ; preds = %3
  br label %7

; <label>:6:                                      ; preds = %7, %3
  ret void

; <label>:7:                                      ; preds = %5, %7
  %8 = phi i64 [ %11, %7 ], [ 0, %5 ]
  %9 = phi i8* [ %12, %7 ], [ %1, %5 ]
  %10 = load i8, i8* %9, align 1, !tbaa !18
  tail call void @_ZN4MARC15ThreadSafeQueueIcE4pushEc(%"class.MARC::ThreadSafeQueue"* %0, i8 signext %10)
  %11 = add nuw i64 %8, 1
  %12 = getelementptr inbounds i8, i8* %9, i64 1
  %13 = icmp slt i64 %11, %2
  br i1 %13, label %7, label %6
}

; Function Attrs: uwtable
define linkonce_odr void @_ZN4MARC15ThreadSafeQueueIcE4pushEc(%"class.MARC::ThreadSafeQueue"*, i8 signext) local_unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %3 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %4, label %9

; <label>:4:                                      ; preds = %2
  %5 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %3, i64 0, i32 0, i32 0
  %6 = tail call i32 @pthread_mutex_lock(%union.pthread_mutex_t* nonnull %5) #8
  %7 = icmp eq i32 %6, 0
  br i1 %7, label %9, label %8

; <label>:8:                                      ; preds = %4
  tail call void @_ZSt20__throw_system_errori(i32 %6) #23
  unreachable

; <label>:9:                                      ; preds = %2, %4
  %10 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2
  %11 = getelementptr inbounds %"class.std::queue", %"class.std::queue"* %10, i64 0, i32 0
  %12 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 0
  %13 = load i8*, i8** %12, align 8, !tbaa !104
  %14 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 2
  %15 = load i8*, i8** %14, align 8, !tbaa !108
  %16 = getelementptr inbounds i8, i8* %15, i64 -1
  %17 = icmp eq i8* %13, %16
  br i1 %17, label %21, label %18

; <label>:18:                                     ; preds = %9
  store i8 %1, i8* %13, align 1, !tbaa !18
  %19 = load i8*, i8** %12, align 8, !tbaa !104
  %20 = getelementptr inbounds i8, i8* %19, i64 1
  store i8* %20, i8** %12, align 8, !tbaa !104
  br label %49

; <label>:21:                                     ; preds = %9
  %22 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 1
  %23 = load i64, i64* %22, align 8, !tbaa !109
  %24 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 3
  %25 = bitcast i8*** %24 to i64*
  %26 = load i64, i64* %25, align 8, !tbaa !110
  %27 = bitcast %"class.std::queue"* %10 to i64*
  %28 = load i64, i64* %27, align 8, !tbaa !111
  %29 = sub i64 %26, %28
  %30 = ashr exact i64 %29, 3
  %31 = sub i64 %23, %30
  %32 = icmp ult i64 %31, 2
  br i1 %32, label %33, label %34

; <label>:33:                                     ; preds = %21
  invoke void @_ZNSt5dequeIcSaIcEE17_M_reallocate_mapEmb(%"class.std::deque"* nonnull %11, i64 1, i1 zeroext false)
          to label %34 unwind label %55

; <label>:34:                                     ; preds = %33, %21
  %35 = invoke i8* @_Znwm(i64 512)
          to label %36 unwind label %55

; <label>:36:                                     ; preds = %34
  %37 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3
  %38 = load i8**, i8*** %24, align 8, !tbaa !110
  %39 = getelementptr inbounds i8*, i8** %38, i64 1
  store i8* %35, i8** %39, align 8, !tbaa !27
  %40 = getelementptr inbounds %"struct.std::_Deque_iterator", %"struct.std::_Deque_iterator"* %37, i64 0, i32 0
  %41 = load i8*, i8** %40, align 8, !tbaa !104
  store i8 %1, i8* %41, align 1, !tbaa !18
  %42 = load i8**, i8*** %24, align 8, !tbaa !110
  %43 = getelementptr inbounds i8*, i8** %42, i64 1
  store i8** %43, i8*** %24, align 8, !tbaa !112
  %44 = load i8*, i8** %43, align 8, !tbaa !27
  %45 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 1
  store i8* %44, i8** %45, align 8, !tbaa !113
  %46 = getelementptr inbounds i8, i8* %44, i64 512
  store i8* %46, i8** %14, align 8, !tbaa !114
  %47 = ptrtoint i8* %44 to i64
  %48 = bitcast %"struct.std::_Deque_iterator"* %37 to i64*
  store i64 %47, i64* %48, align 8, !tbaa !104
  br label %49

; <label>:49:                                     ; preds = %36, %18
  %50 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 3
  tail call void @_ZNSt18condition_variable10notify_oneEv(%"class.std::condition_variable"* %50) #8
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %51, label %54

; <label>:51:                                     ; preds = %49
  %52 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %3, i64 0, i32 0, i32 0
  %53 = tail call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %52) #8
  br label %54

; <label>:54:                                     ; preds = %49, %51
  ret void

; <label>:55:                                     ; preds = %34, %33
  %56 = landingpad { i8*, i32 }
          cleanup
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %57, label %60

; <label>:57:                                     ; preds = %55
  %58 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %3, i64 0, i32 0, i32 0
  %59 = tail call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %58) #8
  br label %60

; <label>:60:                                     ; preds = %55, %57
  resume { i8*, i32 } %56
}

; Function Attrs: uwtable
define linkonce_odr void @_ZNSt5dequeIcSaIcEE17_M_reallocate_mapEmb(%"class.std::deque"*, i64, i1 zeroext) local_unnamed_addr #5 comdat align 2 personality i32 (...)* @__gxx_personality_v0 {
  %4 = getelementptr inbounds %"class.std::deque", %"class.std::deque"* %0, i64 0, i32 0, i32 0, i32 3, i32 3
  %5 = load i8**, i8*** %4, align 8, !tbaa !110
  %6 = getelementptr inbounds %"class.std::deque", %"class.std::deque"* %0, i64 0, i32 0, i32 0, i32 2, i32 3
  %7 = load i8**, i8*** %6, align 8, !tbaa !115
  %8 = ptrtoint i8** %5 to i64
  %9 = ptrtoint i8** %7 to i64
  %10 = sub i64 %8, %9
  %11 = ashr exact i64 %10, 3
  %12 = add nsw i64 %11, 1
  %13 = add i64 %12, %1
  %14 = getelementptr inbounds %"class.std::deque", %"class.std::deque"* %0, i64 0, i32 0, i32 0, i32 1
  %15 = load i64, i64* %14, align 8, !tbaa !109
  %16 = shl i64 %13, 1
  %17 = icmp ugt i64 %15, %16
  br i1 %17, label %18, label %44

; <label>:18:                                     ; preds = %3
  %19 = getelementptr inbounds %"class.std::deque", %"class.std::deque"* %0, i64 0, i32 0, i32 0, i32 0
  %20 = load i8**, i8*** %19, align 8, !tbaa !111
  %21 = sub i64 %15, %13
  %22 = lshr i64 %21, 1
  %23 = getelementptr inbounds i8*, i8** %20, i64 %22
  %24 = select i1 %2, i64 %1, i64 0
  %25 = getelementptr inbounds i8*, i8** %23, i64 %24
  %26 = icmp ult i8** %25, %7
  %27 = getelementptr inbounds i8*, i8** %5, i64 1
  %28 = ptrtoint i8** %27 to i64
  %29 = sub i64 %28, %9
  br i1 %26, label %30, label %35

; <label>:30:                                     ; preds = %18
  %31 = icmp eq i64 %29, 0
  br i1 %31, label %73, label %32

; <label>:32:                                     ; preds = %30
  %33 = bitcast i8** %25 to i8*
  %34 = bitcast i8** %7 to i8*
  tail call void @llvm.memmove.p0i8.p0i8.i64(i8* %33, i8* %34, i64 %29, i32 8, i1 false) #8
  br label %73

; <label>:35:                                     ; preds = %18
  %36 = ashr exact i64 %29, 3
  %37 = icmp eq i64 %36, 0
  br i1 %37, label %73, label %38

; <label>:38:                                     ; preds = %35
  %39 = getelementptr inbounds i8*, i8** %25, i64 %12
  %40 = sub nsw i64 0, %36
  %41 = getelementptr inbounds i8*, i8** %39, i64 %40
  %42 = bitcast i8** %41 to i8*
  %43 = bitcast i8** %7 to i8*
  tail call void @llvm.memmove.p0i8.p0i8.i64(i8* %42, i8* %43, i64 %29, i32 8, i1 false) #8
  br label %73

; <label>:44:                                     ; preds = %3
  %45 = icmp ult i64 %15, %1
  %46 = select i1 %45, i64 %1, i64 %15
  %47 = add i64 %15, 2
  %48 = add i64 %47, %46
  %49 = icmp ugt i64 %48, 2305843009213693951
  br i1 %49, label %50, label %51

; <label>:50:                                     ; preds = %44
  tail call void @_ZSt17__throw_bad_allocv() #23
  unreachable

; <label>:51:                                     ; preds = %44
  %52 = shl i64 %48, 3
  %53 = tail call i8* @_Znwm(i64 %52)
  %54 = bitcast i8* %53 to i8**
  %55 = sub i64 %48, %13
  %56 = lshr i64 %55, 1
  %57 = getelementptr inbounds i8*, i8** %54, i64 %56
  %58 = select i1 %2, i64 %1, i64 0
  %59 = getelementptr inbounds i8*, i8** %57, i64 %58
  %60 = load i8**, i8*** %6, align 8, !tbaa !115
  %61 = load i8**, i8*** %4, align 8, !tbaa !110
  %62 = getelementptr inbounds i8*, i8** %61, i64 1
  %63 = ptrtoint i8** %62 to i64
  %64 = ptrtoint i8** %60 to i64
  %65 = sub i64 %63, %64
  %66 = icmp eq i64 %65, 0
  br i1 %66, label %70, label %67

; <label>:67:                                     ; preds = %51
  %68 = bitcast i8** %59 to i8*
  %69 = bitcast i8** %60 to i8*
  tail call void @llvm.memmove.p0i8.p0i8.i64(i8* %68, i8* %69, i64 %65, i32 8, i1 false) #8
  br label %70

; <label>:70:                                     ; preds = %51, %67
  %71 = bitcast %"class.std::deque"* %0 to i8**
  %72 = load i8*, i8** %71, align 8, !tbaa !111
  tail call void @_ZdlPv(i8* %72) #8
  store i8* %53, i8** %71, align 8, !tbaa !111
  store i64 %48, i64* %14, align 8, !tbaa !109
  br label %73

; <label>:73:                                     ; preds = %38, %35, %32, %30, %70
  %74 = phi i8** [ %59, %70 ], [ %25, %30 ], [ %25, %32 ], [ %25, %35 ], [ %25, %38 ]
  store i8** %74, i8*** %6, align 8, !tbaa !112
  %75 = load i8*, i8** %74, align 8, !tbaa !27
  %76 = getelementptr inbounds %"class.std::deque", %"class.std::deque"* %0, i64 0, i32 0, i32 0, i32 2, i32 1
  store i8* %75, i8** %76, align 8, !tbaa !113
  %77 = getelementptr inbounds i8, i8* %75, i64 512
  %78 = getelementptr inbounds %"class.std::deque", %"class.std::deque"* %0, i64 0, i32 0, i32 0, i32 2, i32 2
  store i8* %77, i8** %78, align 8, !tbaa !114
  %79 = getelementptr inbounds i8*, i8** %74, i64 %11
  store i8** %79, i8*** %4, align 8, !tbaa !112
  %80 = load i8*, i8** %79, align 8, !tbaa !27
  %81 = getelementptr inbounds %"class.std::deque", %"class.std::deque"* %0, i64 0, i32 0, i32 0, i32 3, i32 1
  store i8* %80, i8** %81, align 8, !tbaa !113
  %82 = getelementptr inbounds i8, i8* %80, i64 512
  %83 = getelementptr inbounds %"class.std::deque", %"class.std::deque"* %0, i64 0, i32 0, i32 0, i32 3, i32 2
  store i8* %82, i8** %83, align 8, !tbaa !114
  ret void
}

; Function Attrs: uwtable
define void @queuePop(%"class.MARC::ThreadSafeQueue"*, i8*, i64) local_unnamed_addr #5 {
  %4 = icmp sgt i64 %2, 0
  br i1 %4, label %5, label %6

; <label>:5:                                      ; preds = %3
  br label %7

; <label>:6:                                      ; preds = %15, %3
  ret void

; <label>:7:                                      ; preds = %5, %15
  %8 = phi i64 [ %16, %15 ], [ 0, %5 ]
  %9 = phi i8* [ %17, %15 ], [ %1, %5 ]
  %10 = tail call zeroext i1 @_ZN4MARC15ThreadSafeQueueIcE7waitPopERc(%"class.MARC::ThreadSafeQueue"* %0, i8* dereferenceable(1) %9)
  br i1 %10, label %15, label %11

; <label>:11:                                     ; preds = %7
  br label %12

; <label>:12:                                     ; preds = %11, %12
  %13 = tail call i32 @puts(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @str, i64 0, i64 0))
  %14 = tail call zeroext i1 @_ZN4MARC15ThreadSafeQueueIcE7waitPopERc(%"class.MARC::ThreadSafeQueue"* %0, i8* nonnull dereferenceable(1) %9)
  br i1 %14, label %15, label %12

; <label>:15:                                     ; preds = %12, %7
  %16 = add nuw i64 %8, 1
  %17 = getelementptr inbounds i8, i8* %9, i64 1
  %18 = icmp slt i64 %16, %2
  br i1 %18, label %7, label %6
}

; Function Attrs: uwtable
define linkonce_odr zeroext i1 @_ZN4MARC15ThreadSafeQueueIcE7waitPopERc(%"class.MARC::ThreadSafeQueue"*, i8* dereferenceable(1)) local_unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %3 = alloca %"class.std::unique_lock", align 8
  %4 = bitcast %"class.std::unique_lock"* %3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %4) #8
  %5 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 1
  %6 = getelementptr inbounds %"class.std::unique_lock", %"class.std::unique_lock"* %3, i64 0, i32 0
  store %"class.std::mutex"* %5, %"class.std::mutex"** %6, align 8, !tbaa !32
  %7 = getelementptr inbounds %"class.std::unique_lock", %"class.std::unique_lock"* %3, i64 0, i32 1
  store i8 0, i8* %7, align 8, !tbaa !35
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %8, label %13

; <label>:8:                                      ; preds = %2
  %9 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %5, i64 0, i32 0, i32 0
  %10 = tail call i32 @pthread_mutex_lock(%union.pthread_mutex_t* nonnull %9) #8
  %11 = icmp eq i32 %10, 0
  br i1 %11, label %13, label %12

; <label>:12:                                     ; preds = %8
  tail call void @_ZSt20__throw_system_errori(i32 %10) #23
  unreachable

; <label>:13:                                     ; preds = %2, %8
  store i8 1, i8* %7, align 8, !tbaa !35
  %14 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 0, i32 0, i32 0
  %15 = load atomic i8, i8* %14 seq_cst, align 1
  %16 = and i8 %15, 1
  %17 = icmp eq i8 %16, 0
  br i1 %17, label %61, label %18

; <label>:18:                                     ; preds = %13
  %19 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 0
  %20 = load i8*, i8** %19, align 8, !tbaa !116
  %21 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 0
  %22 = load i8*, i8** %21, align 8, !tbaa !116
  %23 = icmp eq i8* %20, %22
  br i1 %23, label %24, label %34

; <label>:24:                                     ; preds = %18
  %25 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 3
  br label %26

; <label>:26:                                     ; preds = %30, %24
  %27 = load atomic i8, i8* %14 seq_cst, align 1
  %28 = and i8 %27, 1
  %29 = icmp eq i8 %28, 0
  br i1 %29, label %34, label %30

; <label>:30:                                     ; preds = %26
  call void @_ZNSt18condition_variable4waitERSt11unique_lockISt5mutexE(%"class.std::condition_variable"* nonnull %25, %"class.std::unique_lock"* nonnull dereferenceable(16) %3) #8
  %31 = load i8*, i8** %19, align 8, !tbaa !116
  %32 = load i8*, i8** %21, align 8, !tbaa !116
  %33 = icmp eq i8* %31, %32
  br i1 %33, label %26, label %34

; <label>:34:                                     ; preds = %30, %26, %18
  %35 = load atomic i8, i8* %14 seq_cst, align 1
  %36 = and i8 %35, 1
  %37 = icmp eq i8 %36, 0
  br i1 %37, label %61, label %38

; <label>:38:                                     ; preds = %34
  %39 = load i8*, i8** %21, align 8, !tbaa !116, !noalias !117
  %40 = load i8, i8* %39, align 1, !tbaa !18
  store i8 %40, i8* %1, align 1, !tbaa !18
  %41 = load i8*, i8** %21, align 8, !tbaa !120
  %42 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 2
  %43 = load i8*, i8** %42, align 8, !tbaa !121
  %44 = getelementptr inbounds i8, i8* %43, i64 -1
  %45 = icmp eq i8* %41, %44
  br i1 %45, label %48, label %46

; <label>:46:                                     ; preds = %38
  %47 = getelementptr inbounds i8, i8* %41, i64 1
  store i8* %47, i8** %21, align 8, !tbaa !120
  br label %59

; <label>:48:                                     ; preds = %38
  %49 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2
  %50 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 1
  %51 = load i8*, i8** %50, align 8, !tbaa !122
  call void @_ZdlPv(i8* %51) #8
  %52 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 3
  %53 = load i8**, i8*** %52, align 8, !tbaa !115
  %54 = getelementptr inbounds i8*, i8** %53, i64 1
  store i8** %54, i8*** %52, align 8, !tbaa !112
  %55 = load i8*, i8** %54, align 8, !tbaa !27
  store i8* %55, i8** %50, align 8, !tbaa !113
  %56 = getelementptr inbounds i8, i8* %55, i64 512
  store i8* %56, i8** %42, align 8, !tbaa !114
  %57 = ptrtoint i8* %55 to i64
  %58 = bitcast %"struct.std::_Deque_iterator"* %49 to i64*
  store i64 %57, i64* %58, align 8, !tbaa !120
  br label %59

; <label>:59:                                     ; preds = %46, %48
  %60 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 4
  call void @_ZNSt18condition_variable10notify_oneEv(%"class.std::condition_variable"* %60) #8
  br label %61

; <label>:61:                                     ; preds = %34, %13, %59
  %62 = phi i1 [ false, %13 ], [ false, %34 ], [ true, %59 ]
  %63 = load i8, i8* %7, align 8, !tbaa !35, !range !43
  %64 = icmp eq i8 %63, 0
  br i1 %64, label %73, label %65

; <label>:65:                                     ; preds = %61
  %66 = load %"class.std::mutex"*, %"class.std::mutex"** %6, align 8, !tbaa !32
  %67 = icmp eq %"class.std::mutex"* %66, null
  br i1 %67, label %73, label %68

; <label>:68:                                     ; preds = %65
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %69, label %72

; <label>:69:                                     ; preds = %68
  %70 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %66, i64 0, i32 0, i32 0
  %71 = call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %70) #8
  br label %72

; <label>:72:                                     ; preds = %69, %68
  store i8 0, i8* %7, align 8, !tbaa !35
  br label %73

; <label>:73:                                     ; preds = %61, %65, %72
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %4) #8
  ret i1 %62
}

; Function Attrs: uwtable
define void @stageExecuter(void (i8*, i8*)* nocapture, i8*, i8*) local_unnamed_addr #5 {
  tail call void %0(i8* %1, i8* %2)
  ret void
}

; Function Attrs: uwtable
define void @stageDispatcher(i8*, i8* nocapture readnone, i64* nocapture readonly, i8* nocapture readonly, i64, i64) local_unnamed_addr #5 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %7 = alloca i8*, align 8
  %8 = alloca %"class.MARC::ThreadPool", align 8
  %9 = alloca %"class.std::function", align 8
  %10 = alloca %"class.std::vector.53", align 8
  %11 = alloca void (i8*, i8*)*, align 8
  %12 = alloca <2 x i64>, align 16
  %13 = bitcast <2 x i64>* %12 to %"class.MARC::TaskFuture"*
  %14 = alloca i8*, align 8
  store i8* %0, i8** %7, align 8, !tbaa !27
  %15 = alloca i8*, i64 %5, align 16
  %16 = icmp sgt i64 %5, 0
  br i1 %16, label %17, label %18

; <label>:17:                                     ; preds = %6
  br label %22

; <label>:18:                                     ; preds = %63, %6
  %19 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %8, i64 0, i32 0, i32 0, i32 0
  call void @llvm.lifetime.start.p0i8(i64 488, i8* nonnull %19) #8
  %20 = trunc i64 %4 to i32
  %21 = getelementptr inbounds %"class.std::function", %"class.std::function"* %9, i64 0, i32 0, i32 1
  store i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* null, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %21, align 8, !tbaa !123
  invoke void @_ZN4MARC10ThreadPoolC2EjSt8functionIFvvEE(%"class.MARC::ThreadPool"* nonnull %8, i32 %20, %"class.std::function"* nonnull %9)
          to label %68 unwind label %100

; <label>:22:                                     ; preds = %17, %63
  %23 = phi i64 [ %66, %63 ], [ 0, %17 ]
  %24 = getelementptr inbounds i64, i64* %2, i64 %23
  %25 = load i64, i64* %24, align 8, !tbaa !84
  switch i64 %25, label %61 [
    i64 1, label %26
    i64 8, label %33
    i64 16, label %40
    i64 32, label %47
    i64 64, label %54
  ]

; <label>:26:                                     ; preds = %22
  %27 = tail call i8* @_Znwm(i64 224) #24
  %28 = bitcast i8* %27 to %"class.MARC::ThreadSafeQueue"*
  tail call void @llvm.memset.p0i8.i64(i8* nonnull %27, i8 0, i64 224, i32 16, i1 false)
  invoke void @_ZN4MARC15ThreadSafeQueueIaEC2Ev(%"class.MARC::ThreadSafeQueue"* nonnull %28)
          to label %63 unwind label %29

; <label>:29:                                     ; preds = %26
  %30 = landingpad { i8*, i32 }
          cleanup
  %31 = extractvalue { i8*, i32 } %30, 0
  %32 = extractvalue { i8*, i32 } %30, 1
  tail call void @_ZdlPv(i8* nonnull %27) #25
  br label %274

; <label>:33:                                     ; preds = %22
  %34 = tail call i8* @_Znwm(i64 224) #24
  %35 = bitcast i8* %34 to %"class.MARC::ThreadSafeQueue"*
  tail call void @llvm.memset.p0i8.i64(i8* nonnull %34, i8 0, i64 224, i32 16, i1 false)
  invoke void @_ZN4MARC15ThreadSafeQueueIaEC2Ev(%"class.MARC::ThreadSafeQueue"* nonnull %35)
          to label %63 unwind label %36

; <label>:36:                                     ; preds = %33
  %37 = landingpad { i8*, i32 }
          cleanup
  %38 = extractvalue { i8*, i32 } %37, 0
  %39 = extractvalue { i8*, i32 } %37, 1
  tail call void @_ZdlPv(i8* nonnull %34) #25
  br label %274

; <label>:40:                                     ; preds = %22
  %41 = tail call i8* @_Znwm(i64 224) #24
  %42 = bitcast i8* %41 to %"class.MARC::ThreadSafeQueue.3"*
  tail call void @llvm.memset.p0i8.i64(i8* nonnull %41, i8 0, i64 224, i32 16, i1 false)
  invoke void @_ZN4MARC15ThreadSafeQueueIsEC2Ev(%"class.MARC::ThreadSafeQueue.3"* nonnull %42)
          to label %63 unwind label %43

; <label>:43:                                     ; preds = %40
  %44 = landingpad { i8*, i32 }
          cleanup
  %45 = extractvalue { i8*, i32 } %44, 0
  %46 = extractvalue { i8*, i32 } %44, 1
  tail call void @_ZdlPv(i8* nonnull %41) #25
  br label %274

; <label>:47:                                     ; preds = %22
  %48 = tail call i8* @_Znwm(i64 224) #24
  %49 = bitcast i8* %48 to %"class.MARC::ThreadSafeQueue.11"*
  tail call void @llvm.memset.p0i8.i64(i8* nonnull %48, i8 0, i64 224, i32 16, i1 false)
  invoke void @_ZN4MARC15ThreadSafeQueueIiEC2Ev(%"class.MARC::ThreadSafeQueue.11"* nonnull %49)
          to label %63 unwind label %50

; <label>:50:                                     ; preds = %47
  %51 = landingpad { i8*, i32 }
          cleanup
  %52 = extractvalue { i8*, i32 } %51, 0
  %53 = extractvalue { i8*, i32 } %51, 1
  tail call void @_ZdlPv(i8* nonnull %48) #25
  br label %274

; <label>:54:                                     ; preds = %22
  %55 = tail call i8* @_Znwm(i64 224) #24
  %56 = bitcast i8* %55 to %"class.MARC::ThreadSafeQueue.19"*
  tail call void @llvm.memset.p0i8.i64(i8* nonnull %55, i8 0, i64 224, i32 16, i1 false)
  invoke void @_ZN4MARC15ThreadSafeQueueIlEC2Ev(%"class.MARC::ThreadSafeQueue.19"* nonnull %56)
          to label %63 unwind label %57

; <label>:57:                                     ; preds = %54
  %58 = landingpad { i8*, i32 }
          cleanup
  %59 = extractvalue { i8*, i32 } %58, 0
  %60 = extractvalue { i8*, i32 } %58, 1
  tail call void @_ZdlPv(i8* nonnull %55) #25
  br label %274

; <label>:61:                                     ; preds = %22
  %62 = tail call dereferenceable(272) %"class.std::basic_ostream"* @_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(%"class.std::basic_ostream"* nonnull dereferenceable(272) @_ZSt4cerr, i8* getelementptr inbounds ([23 x i8], [23 x i8]* @.str.4, i64 0, i64 0))
  tail call void @abort() #22
  unreachable

; <label>:63:                                     ; preds = %54, %47, %40, %33, %26
  %64 = phi i8* [ %27, %26 ], [ %34, %33 ], [ %41, %40 ], [ %48, %47 ], [ %55, %54 ]
  %65 = getelementptr inbounds i8*, i8** %15, i64 %23
  store i8* %64, i8** %65, align 8, !tbaa !27
  %66 = add nuw i64 %23, 1
  %67 = icmp slt i64 %66, %5
  br i1 %67, label %22, label %18

; <label>:68:                                     ; preds = %18
  %69 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %21, align 8, !tbaa !123
  %70 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %69, null
  br i1 %70, label %77, label %71

; <label>:71:                                     ; preds = %68
  %72 = getelementptr inbounds %"class.std::function", %"class.std::function"* %9, i64 0, i32 0, i32 0
  %73 = invoke zeroext i1 %69(%"union.std::_Any_data"* nonnull dereferenceable(16) %72, %"union.std::_Any_data"* nonnull dereferenceable(16) %72, i32 3)
          to label %77 unwind label %74

; <label>:74:                                     ; preds = %71
  %75 = landingpad { i8*, i32 }
          catch i8* null
  %76 = extractvalue { i8*, i32 } %75, 0
  call void @__clang_call_terminate(i8* %76) #22
  unreachable

; <label>:77:                                     ; preds = %68, %71
  %78 = bitcast %"class.std::vector.53"* %10 to i8*
  call void @llvm.lifetime.start.p0i8(i64 24, i8* nonnull %78) #8
  call void @llvm.memset.p0i8.i64(i8* nonnull %78, i8 0, i64 24, i32 8, i1 false) #8
  %79 = icmp sgt i64 %4, 0
  br i1 %79, label %80, label %218

; <label>:80:                                     ; preds = %77
  %81 = bitcast void (i8*, i8*)** %11 to i8*
  %82 = bitcast i8* %3 to void (i8*, i8*)**
  %83 = bitcast void (i8*, i8*)** %11 to i64*
  %84 = bitcast <2 x i64>* %12 to i8*
  %85 = bitcast i8** %14 to i8*
  %86 = bitcast i8** %14 to i8***
  %87 = getelementptr inbounds %"class.std::vector.53", %"class.std::vector.53"* %10, i64 0, i32 0, i32 0, i32 1
  %88 = getelementptr inbounds %"class.std::vector.53", %"class.std::vector.53"* %10, i64 0, i32 0, i32 0, i32 2
  %89 = bitcast <2 x i64>* %12 to %"class.std::__future_base::_State_baseV2"**
  %90 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %13, i64 0, i32 0, i32 0, i32 0, i32 0, i32 1, i32 0
  %91 = bitcast <2 x i64>* %12 to %"class.std::future"*
  %92 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %13, i64 0, i32 0, i32 0, i32 0, i32 0, i32 1
  %93 = bitcast %"class.std::__shared_count"* %92 to i64*
  br label %112

; <label>:94:                                     ; preds = %166
  %95 = getelementptr inbounds %"class.std::vector.53", %"class.std::vector.53"* %10, i64 0, i32 0, i32 0, i32 0
  %96 = load %"class.MARC::TaskFuture"*, %"class.MARC::TaskFuture"** %95, align 8, !tbaa !27
  %97 = load %"class.MARC::TaskFuture"*, %"class.MARC::TaskFuture"** %87, align 8, !tbaa !27
  %98 = icmp eq %"class.MARC::TaskFuture"* %96, %97
  br i1 %98, label %218, label %99

; <label>:99:                                     ; preds = %94
  br label %220

; <label>:100:                                    ; preds = %18
  %101 = landingpad { i8*, i32 }
          cleanup
  %102 = extractvalue { i8*, i32 } %101, 0
  %103 = extractvalue { i8*, i32 } %101, 1
  %104 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %21, align 8, !tbaa !123
  %105 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %104, null
  br i1 %105, label %271, label %106

; <label>:106:                                    ; preds = %100
  %107 = getelementptr inbounds %"class.std::function", %"class.std::function"* %9, i64 0, i32 0, i32 0
  %108 = invoke zeroext i1 %104(%"union.std::_Any_data"* nonnull dereferenceable(16) %107, %"union.std::_Any_data"* nonnull dereferenceable(16) %107, i32 3)
          to label %271 unwind label %109

; <label>:109:                                    ; preds = %106
  %110 = landingpad { i8*, i32 }
          catch i8* null
  %111 = extractvalue { i8*, i32 } %110, 0
  call void @__clang_call_terminate(i8* %111) #22
  unreachable

; <label>:112:                                    ; preds = %80, %166
  %113 = phi i64 [ 0, %80 ], [ %167, %166 ]
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %81) #8
  %114 = getelementptr inbounds void (i8*, i8*)*, void (i8*, i8*)** %82, i64 %113
  %115 = bitcast void (i8*, i8*)** %114 to i64*
  %116 = load i64, i64* %115, align 8, !tbaa !27
  store i64 %116, i64* %83, align 8, !tbaa !27
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %84) #8
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %85) #8
  store i8** %15, i8*** %86, align 8, !tbaa !27
  invoke void @_ZN4MARC10ThreadPool6submitIRPFvPvS2_EJRS2_S2_EEEDaOT_DpOT0_(%"class.MARC::TaskFuture"* nonnull sret %13, %"class.MARC::ThreadPool"* nonnull %8, void (i8*, i8*)** nonnull dereferenceable(8) %11, i8** nonnull dereferenceable(8) %7, i8** nonnull dereferenceable(8) %14)
          to label %117 unwind label %169

; <label>:117:                                    ; preds = %112
  %118 = load %"class.MARC::TaskFuture"*, %"class.MARC::TaskFuture"** %87, align 8, !tbaa !125
  %119 = load %"class.MARC::TaskFuture"*, %"class.MARC::TaskFuture"** %88, align 8, !tbaa !128
  %120 = icmp eq %"class.MARC::TaskFuture"* %118, %119
  br i1 %120, label %126, label %121

; <label>:121:                                    ; preds = %117
  %122 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %118, i64 0, i32 0, i32 0, i32 0, i32 0, i32 1, i32 0
  store %"class.std::_Sp_counted_base"* null, %"class.std::_Sp_counted_base"** %122, align 8, !tbaa !129
  %123 = load <2 x i64>, <2 x i64>* %12, align 16, !tbaa !27
  store i64 0, i64* %93, align 8, !tbaa !129
  %124 = bitcast %"class.MARC::TaskFuture"* %118 to <2 x i64>*
  store <2 x i64> %123, <2 x i64>* %124, align 8, !tbaa !27
  store %"class.std::__future_base::_State_baseV2"* null, %"class.std::__future_base::_State_baseV2"** %89, align 16, !tbaa !131
  %125 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %118, i64 1
  store %"class.MARC::TaskFuture"* %125, %"class.MARC::TaskFuture"** %87, align 8, !tbaa !125
  br label %135

; <label>:126:                                    ; preds = %117
  invoke void @_ZNSt6vectorIN4MARC10TaskFutureIvEESaIS2_EE19_M_emplace_back_auxIJS2_EEEvDpOT_(%"class.std::vector.53"* nonnull %10, %"class.MARC::TaskFuture"* nonnull dereferenceable(16) %13)
          to label %127 unwind label %173

; <label>:127:                                    ; preds = %126
  %128 = load %"class.std::__future_base::_State_baseV2"*, %"class.std::__future_base::_State_baseV2"** %89, align 16, !tbaa !131
  %129 = icmp eq %"class.std::__future_base::_State_baseV2"* %128, null
  br i1 %129, label %135, label %130

; <label>:130:                                    ; preds = %127
  invoke void @_ZNSt6futureIvE3getEv(%"class.std::future"* nonnull %91)
          to label %135 unwind label %131

; <label>:131:                                    ; preds = %130
  %132 = landingpad { i8*, i32 }
          catch i8* null
  %133 = bitcast <2 x i64>* %12 to %"class.std::__basic_future"*
  %134 = extractvalue { i8*, i32 } %132, 0
  call void @_ZNSt14__basic_futureIvED2Ev(%"class.std::__basic_future"* nonnull %133) #8
  call void @__clang_call_terminate(i8* %134) #22
  unreachable

; <label>:135:                                    ; preds = %121, %130, %127
  %136 = load %"class.std::_Sp_counted_base"*, %"class.std::_Sp_counted_base"** %90, align 8, !tbaa !129
  %137 = icmp eq %"class.std::_Sp_counted_base"* %136, null
  br i1 %137, label %166, label %138

; <label>:138:                                    ; preds = %135
  %139 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %136, i64 0, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %140, label %142

; <label>:140:                                    ; preds = %138
  %141 = atomicrmw volatile add i32* %139, i32 -1 acq_rel
  br label %145

; <label>:142:                                    ; preds = %138
  %143 = load i32, i32* %139, align 4, !tbaa !17
  %144 = add nsw i32 %143, -1
  store i32 %144, i32* %139, align 4, !tbaa !17
  br label %145

; <label>:145:                                    ; preds = %142, %140
  %146 = phi i32 [ %141, %140 ], [ %143, %142 ]
  %147 = icmp eq i32 %146, 1
  br i1 %147, label %148, label %166

; <label>:148:                                    ; preds = %145
  %149 = bitcast %"class.std::_Sp_counted_base"* %136 to void (%"class.std::_Sp_counted_base"*)***
  %150 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %149, align 8, !tbaa !133
  %151 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %150, i64 2
  %152 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %151, align 8
  call void %152(%"class.std::_Sp_counted_base"* nonnull %136) #8
  %153 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %136, i64 0, i32 2
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %154, label %156

; <label>:154:                                    ; preds = %148
  %155 = atomicrmw volatile add i32* %153, i32 -1 acq_rel
  br label %159

; <label>:156:                                    ; preds = %148
  %157 = load i32, i32* %153, align 4, !tbaa !17
  %158 = add nsw i32 %157, -1
  store i32 %158, i32* %153, align 4, !tbaa !17
  br label %159

; <label>:159:                                    ; preds = %156, %154
  %160 = phi i32 [ %155, %154 ], [ %157, %156 ]
  %161 = icmp eq i32 %160, 1
  br i1 %161, label %162, label %166

; <label>:162:                                    ; preds = %159
  %163 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %149, align 8, !tbaa !133
  %164 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %163, i64 3
  %165 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %164, align 8
  call void %165(%"class.std::_Sp_counted_base"* nonnull %136) #8
  br label %166

; <label>:166:                                    ; preds = %135, %145, %159, %162
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %85) #8
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %84) #8
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %81) #8
  %167 = add nuw i64 %113, 1
  %168 = icmp slt i64 %167, %4
  br i1 %168, label %112, label %94

; <label>:169:                                    ; preds = %112
  %170 = landingpad { i8*, i32 }
          cleanup
  %171 = extractvalue { i8*, i32 } %170, 0
  %172 = extractvalue { i8*, i32 } %170, 1
  br label %215

; <label>:173:                                    ; preds = %126
  %174 = landingpad { i8*, i32 }
          cleanup
  %175 = extractvalue { i8*, i32 } %174, 0
  %176 = extractvalue { i8*, i32 } %174, 1
  %177 = bitcast <2 x i64>* %12 to %"class.std::__basic_future"*
  %178 = load %"class.std::__future_base::_State_baseV2"*, %"class.std::__future_base::_State_baseV2"** %89, align 16, !tbaa !131
  %179 = icmp eq %"class.std::__future_base::_State_baseV2"* %178, null
  br i1 %179, label %184, label %180

; <label>:180:                                    ; preds = %173
  invoke void @_ZNSt6futureIvE3getEv(%"class.std::future"* nonnull %91)
          to label %184 unwind label %181

; <label>:181:                                    ; preds = %180
  %182 = landingpad { i8*, i32 }
          catch i8* null
  %183 = extractvalue { i8*, i32 } %182, 0
  call void @_ZNSt14__basic_futureIvED2Ev(%"class.std::__basic_future"* nonnull %177) #8
  call void @__clang_call_terminate(i8* %183) #22
  unreachable

; <label>:184:                                    ; preds = %180, %173
  %185 = load %"class.std::_Sp_counted_base"*, %"class.std::_Sp_counted_base"** %90, align 8, !tbaa !129
  %186 = icmp eq %"class.std::_Sp_counted_base"* %185, null
  br i1 %186, label %215, label %187

; <label>:187:                                    ; preds = %184
  %188 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %185, i64 0, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %189, label %191

; <label>:189:                                    ; preds = %187
  %190 = atomicrmw volatile add i32* %188, i32 -1 acq_rel
  br label %194

; <label>:191:                                    ; preds = %187
  %192 = load i32, i32* %188, align 4, !tbaa !17
  %193 = add nsw i32 %192, -1
  store i32 %193, i32* %188, align 4, !tbaa !17
  br label %194

; <label>:194:                                    ; preds = %191, %189
  %195 = phi i32 [ %190, %189 ], [ %192, %191 ]
  %196 = icmp eq i32 %195, 1
  br i1 %196, label %197, label %215

; <label>:197:                                    ; preds = %194
  %198 = bitcast %"class.std::_Sp_counted_base"* %185 to void (%"class.std::_Sp_counted_base"*)***
  %199 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %198, align 8, !tbaa !133
  %200 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %199, i64 2
  %201 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %200, align 8
  call void %201(%"class.std::_Sp_counted_base"* nonnull %185) #8
  %202 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %185, i64 0, i32 2
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %203, label %205

; <label>:203:                                    ; preds = %197
  %204 = atomicrmw volatile add i32* %202, i32 -1 acq_rel
  br label %208

; <label>:205:                                    ; preds = %197
  %206 = load i32, i32* %202, align 4, !tbaa !17
  %207 = add nsw i32 %206, -1
  store i32 %207, i32* %202, align 4, !tbaa !17
  br label %208

; <label>:208:                                    ; preds = %205, %203
  %209 = phi i32 [ %204, %203 ], [ %206, %205 ]
  %210 = icmp eq i32 %209, 1
  br i1 %210, label %211, label %215

; <label>:211:                                    ; preds = %208
  %212 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %198, align 8, !tbaa !133
  %213 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %212, i64 3
  %214 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %213, align 8
  call void %214(%"class.std::_Sp_counted_base"* nonnull %185) #8
  br label %215

; <label>:215:                                    ; preds = %211, %208, %194, %184, %169
  %216 = phi i32 [ %172, %169 ], [ %176, %184 ], [ %176, %194 ], [ %176, %208 ], [ %176, %211 ]
  %217 = phi i8* [ %171, %169 ], [ %175, %184 ], [ %175, %194 ], [ %175, %208 ], [ %175, %211 ]
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %85) #8
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %84) #8
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %81) #8
  br label %268

; <label>:218:                                    ; preds = %223, %77, %94
  br i1 %16, label %219, label %230

; <label>:219:                                    ; preds = %218
  br label %231

; <label>:220:                                    ; preds = %99, %223
  %221 = phi %"class.MARC::TaskFuture"* [ %224, %223 ], [ %96, %99 ]
  %222 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %221, i64 0, i32 0
  invoke void @_ZNSt6futureIvE3getEv(%"class.std::future"* %222)
          to label %223 unwind label %226

; <label>:223:                                    ; preds = %220
  %224 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %221, i64 1
  %225 = icmp eq %"class.MARC::TaskFuture"* %224, %97
  br i1 %225, label %218, label %220

; <label>:226:                                    ; preds = %220
  %227 = landingpad { i8*, i32 }
          cleanup
  %228 = extractvalue { i8*, i32 } %227, 0
  %229 = extractvalue { i8*, i32 } %227, 1
  br label %268

; <label>:230:                                    ; preds = %265, %218
  call void @_ZNSt6vectorIN4MARC10TaskFutureIvEESaIS2_EED2Ev(%"class.std::vector.53"* nonnull %10) #8
  call void @llvm.lifetime.end.p0i8(i64 24, i8* nonnull %78) #8
  call void @_ZN4MARC10ThreadPoolD2Ev(%"class.MARC::ThreadPool"* nonnull %8) #8
  call void @llvm.lifetime.end.p0i8(i64 488, i8* nonnull %19) #8
  ret void

; <label>:231:                                    ; preds = %219, %265
  %232 = phi i64 [ %266, %265 ], [ 0, %219 ]
  %233 = getelementptr inbounds i64, i64* %2, i64 %232
  %234 = load i64, i64* %233, align 8, !tbaa !84
  switch i64 %234, label %265 [
    i64 1, label %235
    i64 8, label %241
    i64 16, label %247
    i64 32, label %253
    i64 64, label %259
  ]

; <label>:235:                                    ; preds = %231
  %236 = getelementptr inbounds i8*, i8** %15, i64 %232
  %237 = load i8*, i8** %236, align 8, !tbaa !27
  %238 = icmp eq i8* %237, null
  br i1 %238, label %265, label %239

; <label>:239:                                    ; preds = %235
  %240 = bitcast i8* %237 to %"class.MARC::ThreadSafeQueue"*
  call void @_ZN4MARC15ThreadSafeQueueIaED2Ev(%"class.MARC::ThreadSafeQueue"* %240) #8
  call void @_ZdlPv(i8* nonnull %237) #25
  br label %265

; <label>:241:                                    ; preds = %231
  %242 = getelementptr inbounds i8*, i8** %15, i64 %232
  %243 = load i8*, i8** %242, align 8, !tbaa !27
  %244 = icmp eq i8* %243, null
  br i1 %244, label %265, label %245

; <label>:245:                                    ; preds = %241
  %246 = bitcast i8* %243 to %"class.MARC::ThreadSafeQueue"*
  call void @_ZN4MARC15ThreadSafeQueueIaED2Ev(%"class.MARC::ThreadSafeQueue"* %246) #8
  call void @_ZdlPv(i8* nonnull %243) #25
  br label %265

; <label>:247:                                    ; preds = %231
  %248 = getelementptr inbounds i8*, i8** %15, i64 %232
  %249 = load i8*, i8** %248, align 8, !tbaa !27
  %250 = icmp eq i8* %249, null
  br i1 %250, label %265, label %251

; <label>:251:                                    ; preds = %247
  %252 = bitcast i8* %249 to %"class.MARC::ThreadSafeQueue.3"*
  call void @_ZN4MARC15ThreadSafeQueueIsED2Ev(%"class.MARC::ThreadSafeQueue.3"* %252) #8
  call void @_ZdlPv(i8* nonnull %249) #25
  br label %265

; <label>:253:                                    ; preds = %231
  %254 = getelementptr inbounds i8*, i8** %15, i64 %232
  %255 = load i8*, i8** %254, align 8, !tbaa !27
  %256 = icmp eq i8* %255, null
  br i1 %256, label %265, label %257

; <label>:257:                                    ; preds = %253
  %258 = bitcast i8* %255 to %"class.MARC::ThreadSafeQueue.11"*
  call void @_ZN4MARC15ThreadSafeQueueIiED2Ev(%"class.MARC::ThreadSafeQueue.11"* %258) #8
  call void @_ZdlPv(i8* nonnull %255) #25
  br label %265

; <label>:259:                                    ; preds = %231
  %260 = getelementptr inbounds i8*, i8** %15, i64 %232
  %261 = load i8*, i8** %260, align 8, !tbaa !27
  %262 = icmp eq i8* %261, null
  br i1 %262, label %265, label %263

; <label>:263:                                    ; preds = %259
  %264 = bitcast i8* %261 to %"class.MARC::ThreadSafeQueue.19"*
  call void @_ZN4MARC15ThreadSafeQueueIlED2Ev(%"class.MARC::ThreadSafeQueue.19"* %264) #8
  call void @_ZdlPv(i8* nonnull %261) #25
  br label %265

; <label>:265:                                    ; preds = %231, %239, %235, %245, %241, %251, %247, %257, %253, %263, %259
  %266 = add nuw i64 %232, 1
  %267 = icmp slt i64 %266, %5
  br i1 %267, label %231, label %230

; <label>:268:                                    ; preds = %226, %215
  %269 = phi i32 [ %216, %215 ], [ %229, %226 ]
  %270 = phi i8* [ %217, %215 ], [ %228, %226 ]
  call void @_ZNSt6vectorIN4MARC10TaskFutureIvEESaIS2_EED2Ev(%"class.std::vector.53"* nonnull %10) #8
  call void @llvm.lifetime.end.p0i8(i64 24, i8* nonnull %78) #8
  call void @_ZN4MARC10ThreadPoolD2Ev(%"class.MARC::ThreadPool"* nonnull %8) #8
  br label %271

; <label>:271:                                    ; preds = %106, %100, %268
  %272 = phi i32 [ %269, %268 ], [ %103, %100 ], [ %103, %106 ]
  %273 = phi i8* [ %270, %268 ], [ %102, %100 ], [ %102, %106 ]
  call void @llvm.lifetime.end.p0i8(i64 488, i8* nonnull %19) #8
  br label %274

; <label>:274:                                    ; preds = %29, %36, %43, %50, %57, %271
  %275 = phi i32 [ %272, %271 ], [ %60, %57 ], [ %53, %50 ], [ %46, %43 ], [ %39, %36 ], [ %32, %29 ]
  %276 = phi i8* [ %273, %271 ], [ %59, %57 ], [ %52, %50 ], [ %45, %43 ], [ %38, %36 ], [ %31, %29 ]
  %277 = insertvalue { i8*, i32 } undef, i8* %276, 0
  %278 = insertvalue { i8*, i32 } %277, i32 %275, 1
  resume { i8*, i32 } %278
}

; Function Attrs: uwtable
define linkonce_odr void @_ZN4MARC10ThreadPoolC2EjSt8functionIFvvEE(%"class.MARC::ThreadPool"*, i32, %"class.std::function"*) unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %4 = alloca <2 x i64>, align 16
  %5 = bitcast <2 x i64>* %4 to { i64, i64 }*
  %6 = alloca %"class.MARC::ThreadPool"*, align 8
  %7 = alloca %"struct.std::atomic"*, align 8
  %8 = alloca %"class.std::function", align 8
  %9 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 0, i32 0, i32 0
  store i8 0, i8* %9, align 1, !tbaa !135
  %10 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 1
  %11 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %10, i64 0, i32 0, i32 0, i32 0
  tail call void @llvm.memset.p0i8.i64(i8* %11, i8 0, i64 224, i32 8, i1 false)
  tail call void @_ZN4MARC15ThreadSafeQueueISt10unique_ptrINS_11IThreadTaskESt14default_deleteIS2_EEEC2Ev(%"class.MARC::ThreadSafeQueue.32"* %10)
  %12 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 2
  %13 = bitcast %"class.std::vector"* %12 to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %13, i8 0, i64 24, i32 8, i1 false) #8
  %14 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 4
  invoke void @_ZN4MARC15ThreadSafeQueueISt8functionIFvvEEEC2Ev(%"class.MARC::ThreadSafeQueue.45"* %14)
          to label %15 unwind label %50

; <label>:15:                                     ; preds = %3
  %16 = zext i32 %1 to i64
  %17 = invoke i8* @_Znam(i64 %16) #24
          to label %18 unwind label %54

; <label>:18:                                     ; preds = %15
  %19 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 3
  %20 = bitcast %"struct.std::atomic"** %19 to i8**
  store i8* %17, i8** %20, align 8, !tbaa !137
  %21 = icmp eq i32 %1, 0
  br i1 %21, label %86, label %22

; <label>:22:                                     ; preds = %18
  store atomic i8 1, i8* %17 seq_cst, align 1
  %23 = icmp eq i32 %1, 1
  br i1 %23, label %45, label %24

; <label>:24:                                     ; preds = %22
  %25 = add nsw i64 %16, -2
  %26 = add i32 %1, 7
  %27 = and i32 %26, 7
  %28 = zext i32 %27 to i64
  %29 = icmp ult i64 %25, 7
  br i1 %29, label %33, label %30

; <label>:30:                                     ; preds = %24
  %31 = add nsw i64 %16, -1
  %32 = sub nsw i64 %31, %28
  br label %57

; <label>:33:                                     ; preds = %57, %24
  %34 = phi i64 [ 1, %24 ], [ %83, %57 ]
  %35 = icmp eq i32 %27, 0
  br i1 %35, label %45, label %36

; <label>:36:                                     ; preds = %33
  br label %37

; <label>:37:                                     ; preds = %37, %36
  %38 = phi i64 [ %42, %37 ], [ %34, %36 ]
  %39 = phi i64 [ %43, %37 ], [ %28, %36 ]
  %40 = load %"struct.std::atomic"*, %"struct.std::atomic"** %19, align 8, !tbaa !137
  %41 = getelementptr inbounds %"struct.std::atomic", %"struct.std::atomic"* %40, i64 %38, i32 0, i32 0
  store atomic i8 1, i8* %41 seq_cst, align 1
  %42 = add nuw nsw i64 %38, 1
  %43 = add i64 %39, -1
  %44 = icmp eq i64 %43, 0
  br i1 %44, label %45, label %37, !llvm.loop !149

; <label>:45:                                     ; preds = %33, %37, %22
  br i1 %21, label %86, label %46

; <label>:46:                                     ; preds = %45
  %47 = bitcast <2 x i64>* %4 to i8*
  %48 = bitcast %"class.MARC::ThreadPool"** %6 to i8*
  %49 = bitcast %"struct.std::atomic"** %7 to i8*
  br label %90

; <label>:50:                                     ; preds = %3
  %51 = landingpad { i8*, i32 }
          cleanup
  %52 = extractvalue { i8*, i32 } %51, 0
  %53 = extractvalue { i8*, i32 } %51, 1
  br label %152

; <label>:54:                                     ; preds = %15
  %55 = landingpad { i8*, i32 }
          catch i8* null
  %56 = extractvalue { i8*, i32 } %55, 0
  br label %100

; <label>:57:                                     ; preds = %57, %30
  %58 = phi i64 [ 1, %30 ], [ %83, %57 ]
  %59 = phi i64 [ %32, %30 ], [ %84, %57 ]
  %60 = load %"struct.std::atomic"*, %"struct.std::atomic"** %19, align 8, !tbaa !137
  %61 = getelementptr inbounds %"struct.std::atomic", %"struct.std::atomic"* %60, i64 %58, i32 0, i32 0
  store atomic i8 1, i8* %61 seq_cst, align 1
  %62 = add nuw nsw i64 %58, 1
  %63 = load %"struct.std::atomic"*, %"struct.std::atomic"** %19, align 8, !tbaa !137
  %64 = getelementptr inbounds %"struct.std::atomic", %"struct.std::atomic"* %63, i64 %62, i32 0, i32 0
  store atomic i8 1, i8* %64 seq_cst, align 1
  %65 = add nsw i64 %58, 2
  %66 = load %"struct.std::atomic"*, %"struct.std::atomic"** %19, align 8, !tbaa !137
  %67 = getelementptr inbounds %"struct.std::atomic", %"struct.std::atomic"* %66, i64 %65, i32 0, i32 0
  store atomic i8 1, i8* %67 seq_cst, align 1
  %68 = add nsw i64 %58, 3
  %69 = load %"struct.std::atomic"*, %"struct.std::atomic"** %19, align 8, !tbaa !137
  %70 = getelementptr inbounds %"struct.std::atomic", %"struct.std::atomic"* %69, i64 %68, i32 0, i32 0
  store atomic i8 1, i8* %70 seq_cst, align 1
  %71 = add nsw i64 %58, 4
  %72 = load %"struct.std::atomic"*, %"struct.std::atomic"** %19, align 8, !tbaa !137
  %73 = getelementptr inbounds %"struct.std::atomic", %"struct.std::atomic"* %72, i64 %71, i32 0, i32 0
  store atomic i8 1, i8* %73 seq_cst, align 1
  %74 = add nsw i64 %58, 5
  %75 = load %"struct.std::atomic"*, %"struct.std::atomic"** %19, align 8, !tbaa !137
  %76 = getelementptr inbounds %"struct.std::atomic", %"struct.std::atomic"* %75, i64 %74, i32 0, i32 0
  store atomic i8 1, i8* %76 seq_cst, align 1
  %77 = add nsw i64 %58, 6
  %78 = load %"struct.std::atomic"*, %"struct.std::atomic"** %19, align 8, !tbaa !137
  %79 = getelementptr inbounds %"struct.std::atomic", %"struct.std::atomic"* %78, i64 %77, i32 0, i32 0
  store atomic i8 1, i8* %79 seq_cst, align 1
  %80 = add nsw i64 %58, 7
  %81 = load %"struct.std::atomic"*, %"struct.std::atomic"** %19, align 8, !tbaa !137
  %82 = getelementptr inbounds %"struct.std::atomic", %"struct.std::atomic"* %81, i64 %80, i32 0, i32 0
  store atomic i8 1, i8* %82 seq_cst, align 1
  %83 = add nsw i64 %58, 8
  %84 = add i64 %59, -8
  %85 = icmp eq i64 %84, 0
  br i1 %85, label %33, label %57

; <label>:86:                                     ; preds = %94, %18, %45
  %87 = getelementptr inbounds %"class.std::function", %"class.std::function"* %2, i64 0, i32 0, i32 1
  %88 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %87, align 8, !tbaa !123
  %89 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %88, null
  br i1 %89, label %148, label %108

; <label>:90:                                     ; preds = %46, %94
  %91 = phi i64 [ 0, %46 ], [ %95, %94 ]
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %47) #8
  store <2 x i64> <i64 ptrtoint (void (%"class.MARC::ThreadPool"*, %"struct.std::atomic"*)* @_ZN4MARC10ThreadPool6workerEPSt6atomicIbE to i64), i64 0>, <2 x i64>* %4, align 16
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %48) #8
  store %"class.MARC::ThreadPool"* %0, %"class.MARC::ThreadPool"** %6, align 8, !tbaa !27
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %49) #8
  %92 = load %"struct.std::atomic"*, %"struct.std::atomic"** %19, align 8, !tbaa !137
  %93 = getelementptr inbounds %"struct.std::atomic", %"struct.std::atomic"* %92, i64 %91
  store %"struct.std::atomic"* %93, %"struct.std::atomic"** %7, align 8, !tbaa !27
  invoke void @_ZNSt6vectorISt6threadSaIS0_EE12emplace_backIJMN4MARC10ThreadPoolEFvPSt6atomicIbEEPS5_S8_EEEvDpOT_(%"class.std::vector"* nonnull %12, { i64, i64 }* nonnull dereferenceable(16) %5, %"class.MARC::ThreadPool"** nonnull dereferenceable(8) %6, %"struct.std::atomic"** nonnull dereferenceable(8) %7)
          to label %94 unwind label %97

; <label>:94:                                     ; preds = %90
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %49) #8
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %48) #8
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %47) #8
  %95 = add nuw nsw i64 %91, 1
  %96 = icmp ult i64 %95, %16
  br i1 %96, label %90, label %86

; <label>:97:                                     ; preds = %90
  %98 = landingpad { i8*, i32 }
          catch i8* null
  %99 = extractvalue { i8*, i32 } %98, 0
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %49) #8
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %48) #8
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %47) #8
  br label %100

; <label>:100:                                    ; preds = %97, %54
  %101 = phi i8* [ %99, %97 ], [ %56, %54 ]
  %102 = call i8* @__cxa_begin_catch(i8* %101) #8
  invoke void @_ZN4MARC10ThreadPool7destroyEv(%"class.MARC::ThreadPool"* nonnull %0)
          to label %103 unwind label %104

; <label>:103:                                    ; preds = %100
  invoke void @__cxa_rethrow() #23
          to label %180 unwind label %104

; <label>:104:                                    ; preds = %103, %100
  %105 = landingpad { i8*, i32 }
          cleanup
  %106 = extractvalue { i8*, i32 } %105, 0
  %107 = extractvalue { i8*, i32 } %105, 1
  invoke void @__cxa_end_catch()
          to label %149 unwind label %177

; <label>:108:                                    ; preds = %86
  %109 = getelementptr inbounds %"class.std::function", %"class.std::function"* %8, i64 0, i32 0, i32 1
  store i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* null, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %109, align 8, !tbaa !123
  %110 = getelementptr inbounds %"class.std::function", %"class.std::function"* %8, i64 0, i32 0, i32 0
  %111 = getelementptr inbounds %"class.std::function", %"class.std::function"* %2, i64 0, i32 0, i32 0
  %112 = invoke zeroext i1 %88(%"union.std::_Any_data"* nonnull dereferenceable(16) %110, %"union.std::_Any_data"* nonnull dereferenceable(16) %111, i32 2)
          to label %122 unwind label %113

; <label>:113:                                    ; preds = %108
  %114 = landingpad { i8*, i32 }
          cleanup
  %115 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %109, align 8, !tbaa !123
  %116 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %115, null
  br i1 %116, label %134, label %117

; <label>:117:                                    ; preds = %113
  %118 = invoke zeroext i1 %115(%"union.std::_Any_data"* nonnull dereferenceable(16) %110, %"union.std::_Any_data"* nonnull dereferenceable(16) %110, i32 3)
          to label %134 unwind label %119

; <label>:119:                                    ; preds = %117
  %120 = landingpad { i8*, i32 }
          catch i8* null
  %121 = extractvalue { i8*, i32 } %120, 0
  call void @__clang_call_terminate(i8* %121) #22
  unreachable

; <label>:122:                                    ; preds = %108
  %123 = bitcast i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %87 to <2 x i64>*
  %124 = load <2 x i64>, <2 x i64>* %123, align 8, !tbaa !27
  %125 = bitcast i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %109 to <2 x i64>*
  store <2 x i64> %124, <2 x i64>* %125, align 8, !tbaa !27
  invoke void @_ZN4MARC15ThreadSafeQueueISt8functionIFvvEEE4pushES3_(%"class.MARC::ThreadSafeQueue.45"* nonnull %14, %"class.std::function"* nonnull %8)
          to label %126 unwind label %137

; <label>:126:                                    ; preds = %122
  %127 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %109, align 8, !tbaa !123
  %128 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %127, null
  br i1 %128, label %148, label %129

; <label>:129:                                    ; preds = %126
  %130 = invoke zeroext i1 %127(%"union.std::_Any_data"* nonnull dereferenceable(16) %110, %"union.std::_Any_data"* nonnull dereferenceable(16) %110, i32 3)
          to label %148 unwind label %131

; <label>:131:                                    ; preds = %129
  %132 = landingpad { i8*, i32 }
          catch i8* null
  %133 = extractvalue { i8*, i32 } %132, 0
  call void @__clang_call_terminate(i8* %133) #22
  unreachable

; <label>:134:                                    ; preds = %113, %117
  %135 = extractvalue { i8*, i32 } %114, 0
  %136 = extractvalue { i8*, i32 } %114, 1
  br label %149

; <label>:137:                                    ; preds = %122
  %138 = landingpad { i8*, i32 }
          cleanup
  %139 = extractvalue { i8*, i32 } %138, 0
  %140 = extractvalue { i8*, i32 } %138, 1
  %141 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %109, align 8, !tbaa !123
  %142 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %141, null
  br i1 %142, label %149, label %143

; <label>:143:                                    ; preds = %137
  %144 = invoke zeroext i1 %141(%"union.std::_Any_data"* nonnull dereferenceable(16) %110, %"union.std::_Any_data"* nonnull dereferenceable(16) %110, i32 3)
          to label %149 unwind label %145

; <label>:145:                                    ; preds = %143
  %146 = landingpad { i8*, i32 }
          catch i8* null
  %147 = extractvalue { i8*, i32 } %146, 0
  call void @__clang_call_terminate(i8* %147) #22
  unreachable

; <label>:148:                                    ; preds = %86, %129, %126
  ret void

; <label>:149:                                    ; preds = %143, %137, %104, %134
  %150 = phi i8* [ %106, %104 ], [ %135, %134 ], [ %139, %137 ], [ %139, %143 ]
  %151 = phi i32 [ %107, %104 ], [ %136, %134 ], [ %140, %137 ], [ %140, %143 ]
  call void @_ZN4MARC15ThreadSafeQueueISt8functionIFvvEEED2Ev(%"class.MARC::ThreadSafeQueue.45"* %14) #8
  br label %152

; <label>:152:                                    ; preds = %149, %50
  %153 = phi i8* [ %150, %149 ], [ %52, %50 ]
  %154 = phi i32 [ %151, %149 ], [ %53, %50 ]
  %155 = getelementptr inbounds %"class.std::vector", %"class.std::vector"* %12, i64 0, i32 0, i32 0, i32 0
  %156 = load %"class.std::thread"*, %"class.std::thread"** %155, align 8, !tbaa !151
  %157 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 2, i32 0, i32 0, i32 1
  %158 = load %"class.std::thread"*, %"class.std::thread"** %157, align 8, !tbaa !154
  %159 = icmp eq %"class.std::thread"* %156, %158
  br i1 %159, label %170, label %160

; <label>:160:                                    ; preds = %152
  br label %163

; <label>:161:                                    ; preds = %163
  %162 = icmp eq %"class.std::thread"* %168, %158
  br i1 %162, label %170, label %163

; <label>:163:                                    ; preds = %160, %161
  %164 = phi %"class.std::thread"* [ %168, %161 ], [ %156, %160 ]
  %165 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %164, i64 0, i32 0, i32 0
  %166 = load i64, i64* %165, align 8
  %167 = icmp eq i64 %166, 0
  %168 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %164, i64 1
  br i1 %167, label %161, label %169

; <label>:169:                                    ; preds = %163
  call void @_ZSt9terminatev() #22
  unreachable

; <label>:170:                                    ; preds = %161, %152
  %171 = icmp eq %"class.std::thread"* %156, null
  br i1 %171, label %174, label %172

; <label>:172:                                    ; preds = %170
  %173 = bitcast %"class.std::thread"* %156 to i8*
  call void @_ZdlPv(i8* %173) #8
  br label %174

; <label>:174:                                    ; preds = %170, %172
  call void @_ZN4MARC15ThreadSafeQueueISt10unique_ptrINS_11IThreadTaskESt14default_deleteIS2_EEED2Ev(%"class.MARC::ThreadSafeQueue.32"* %10) #8
  %175 = insertvalue { i8*, i32 } undef, i8* %153, 0
  %176 = insertvalue { i8*, i32 } %175, i32 %154, 1
  resume { i8*, i32 } %176

; <label>:177:                                    ; preds = %104
  %178 = landingpad { i8*, i32 }
          catch i8* null
  %179 = extractvalue { i8*, i32 } %178, 0
  call void @__clang_call_terminate(i8* %179) #22
  unreachable

; <label>:180:                                    ; preds = %103
  unreachable
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i32, i1) #9

; Function Attrs: uwtable
define linkonce_odr void @_ZN4MARC15ThreadSafeQueueIaEC2Ev(%"class.MARC::ThreadSafeQueue"*) unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %2 = alloca %"class.std::deque", align 16
  %3 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 0, i32 0, i32 0
  store i8 1, i8* %3, align 1, !tbaa !135
  %4 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 1
  %5 = bitcast %"class.std::mutex"* %4 to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %5, i8 0, i64 40, i32 8, i1 false) #8
  %6 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2
  %7 = bitcast %"class.std::deque"* %2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 80, i8* nonnull %7) #8
  %8 = getelementptr inbounds %"class.std::deque", %"class.std::deque"* %2, i64 0, i32 0
  call void @llvm.memset.p0i8.i64(i8* nonnull %7, i8 0, i64 80, i32 16, i1 false) #8
  call void @_ZNSt11_Deque_baseIaSaIaEE17_M_initialize_mapEm(%"class.std::_Deque_base"* nonnull %8, i64 0)
  %9 = getelementptr inbounds %"class.std::queue", %"class.std::queue"* %6, i64 0, i32 0, i32 0
  %10 = bitcast %"class.std::queue"* %6 to i8*
  call void @llvm.memset.p0i8.i64(i8* %10, i8 0, i64 80, i32 8, i1 false) #8
  invoke void @_ZNSt11_Deque_baseIaSaIaEE17_M_initialize_mapEm(%"class.std::_Deque_base"* %9, i64 0)
          to label %11 unwind label %72

; <label>:11:                                     ; preds = %1
  %12 = getelementptr inbounds %"class.std::deque", %"class.std::deque"* %2, i64 0, i32 0, i32 0, i32 0
  %13 = load i8**, i8*** %12, align 16, !tbaa !26
  %14 = icmp eq i8** %13, null
  br i1 %14, label %69, label %15

; <label>:15:                                     ; preds = %11
  %16 = ptrtoint i8** %13 to i64
  %17 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2
  %18 = getelementptr inbounds %"class.std::deque", %"class.std::deque"* %2, i64 0, i32 0, i32 0, i32 2
  %19 = bitcast %"struct.std::_Deque_iterator"* %17 to <2 x i64>*
  %20 = load <2 x i64>, <2 x i64>* %19, align 8, !tbaa !27
  %21 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 2
  %22 = bitcast i8** %21 to <2 x i64>*
  %23 = load <2 x i64>, <2 x i64>* %22, align 8, !tbaa !27
  %24 = bitcast %"struct.std::_Deque_iterator"* %17 to i8*
  %25 = bitcast %"struct.std::_Deque_iterator"* %18 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %24, i8* nonnull %25, i64 32, i32 8, i1 false) #8, !tbaa.struct !155
  %26 = bitcast %"struct.std::_Deque_iterator"* %18 to <2 x i64>*
  store <2 x i64> %20, <2 x i64>* %26, align 16
  %27 = getelementptr inbounds %"class.std::deque", %"class.std::deque"* %2, i64 0, i32 0, i32 0, i32 2, i32 2
  %28 = bitcast i8** %27 to <2 x i64>*
  store <2 x i64> %23, <2 x i64>* %28, align 16
  %29 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3
  %30 = getelementptr inbounds %"class.std::deque", %"class.std::deque"* %2, i64 0, i32 0, i32 0, i32 3
  %31 = bitcast %"struct.std::_Deque_iterator"* %29 to <2 x i64>*
  %32 = load <2 x i64>, <2 x i64>* %31, align 8, !tbaa !27
  %33 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 2
  %34 = bitcast i8** %33 to <2 x i64>*
  %35 = load <2 x i64>, <2 x i64>* %34, align 8, !tbaa !27
  %36 = bitcast %"struct.std::_Deque_iterator"* %29 to i8*
  %37 = bitcast %"struct.std::_Deque_iterator"* %30 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %36, i8* nonnull %37, i64 32, i32 8, i1 false) #8, !tbaa.struct !155
  %38 = bitcast %"struct.std::_Deque_iterator"* %30 to <2 x i64>*
  store <2 x i64> %32, <2 x i64>* %38, align 16
  %39 = getelementptr inbounds %"class.std::deque", %"class.std::deque"* %2, i64 0, i32 0, i32 0, i32 3, i32 2
  %40 = bitcast i8** %39 to <2 x i64>*
  store <2 x i64> %35, <2 x i64>* %40, align 16
  %41 = bitcast %"class.std::queue"* %6 to i64*
  %42 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 1
  %43 = bitcast %"class.std::queue"* %6 to <2 x i64>*
  %44 = load <2 x i64>, <2 x i64>* %43, align 8, !tbaa !18
  store i64 %16, i64* %41, align 8, !tbaa !27
  %45 = getelementptr inbounds %"class.std::deque", %"class.std::deque"* %2, i64 0, i32 0, i32 0, i32 1
  %46 = load i64, i64* %45, align 8, !tbaa !84
  store i64 %46, i64* %42, align 8, !tbaa !84
  %47 = bitcast %"class.std::deque"* %2 to <2 x i64>*
  store <2 x i64> %44, <2 x i64>* %47, align 16, !tbaa !18
  %48 = extractelement <2 x i64> %44, i32 0
  %49 = icmp eq i64 %48, 0
  %50 = extractelement <2 x i64> %23, i32 1
  %51 = inttoptr i64 %50 to i8**
  %52 = extractelement <2 x i64> %35, i32 1
  %53 = inttoptr i64 %52 to i8**
  br i1 %49, label %69, label %54

; <label>:54:                                     ; preds = %15
  %55 = inttoptr i64 %48 to i8*
  %56 = getelementptr inbounds i8*, i8** %53, i64 1
  %57 = icmp ugt i8** %56, %51
  br i1 %57, label %58, label %67

; <label>:58:                                     ; preds = %54
  br label %59

; <label>:59:                                     ; preds = %58, %59
  %60 = phi i8** [ %62, %59 ], [ %51, %58 ]
  %61 = load i8*, i8** %60, align 8, !tbaa !27
  call void @_ZdlPv(i8* %61) #8
  %62 = getelementptr inbounds i8*, i8** %60, i64 1
  %63 = icmp ult i8** %60, %53
  br i1 %63, label %59, label %64

; <label>:64:                                     ; preds = %59
  %65 = bitcast %"class.std::deque"* %2 to i8**
  %66 = load i8*, i8** %65, align 16, !tbaa !26
  br label %67

; <label>:67:                                     ; preds = %64, %54
  %68 = phi i8* [ %66, %64 ], [ %55, %54 ]
  call void @_ZdlPv(i8* %68) #8
  br label %69

; <label>:69:                                     ; preds = %11, %15, %67
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %7) #8
  %70 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 3
  call void @_ZNSt18condition_variableC1Ev(%"class.std::condition_variable"* %70) #8
  %71 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 4
  call void @_ZNSt18condition_variableC1Ev(%"class.std::condition_variable"* %71) #8
  ret void

; <label>:72:                                     ; preds = %1
  %73 = landingpad { i8*, i32 }
          cleanup
  %74 = getelementptr inbounds %"class.std::deque", %"class.std::deque"* %2, i64 0, i32 0, i32 0, i32 0
  %75 = load i8**, i8*** %74, align 16, !tbaa !26
  %76 = icmp eq i8** %75, null
  br i1 %76, label %96, label %77

; <label>:77:                                     ; preds = %72
  %78 = bitcast i8** %75 to i8*
  %79 = getelementptr inbounds %"class.std::deque", %"class.std::deque"* %2, i64 0, i32 0, i32 0, i32 2, i32 3
  %80 = load i8**, i8*** %79, align 8, !tbaa !31
  %81 = getelementptr inbounds %"class.std::deque", %"class.std::deque"* %2, i64 0, i32 0, i32 0, i32 3, i32 3
  %82 = load i8**, i8*** %81, align 8, !tbaa !25
  %83 = getelementptr inbounds i8*, i8** %82, i64 1
  %84 = icmp ult i8** %80, %83
  br i1 %84, label %85, label %94

; <label>:85:                                     ; preds = %77
  br label %86

; <label>:86:                                     ; preds = %85, %86
  %87 = phi i8** [ %89, %86 ], [ %80, %85 ]
  %88 = load i8*, i8** %87, align 8, !tbaa !27
  call void @_ZdlPv(i8* %88) #8
  %89 = getelementptr inbounds i8*, i8** %87, i64 1
  %90 = icmp ult i8** %87, %82
  br i1 %90, label %86, label %91

; <label>:91:                                     ; preds = %86
  %92 = bitcast %"class.std::deque"* %2 to i8**
  %93 = load i8*, i8** %92, align 16, !tbaa !26
  br label %94

; <label>:94:                                     ; preds = %91, %77
  %95 = phi i8* [ %93, %91 ], [ %78, %77 ]
  call void @_ZdlPv(i8* %95) #8
  br label %96

; <label>:96:                                     ; preds = %72, %94
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %7) #8
  resume { i8*, i32 } %73
}

; Function Attrs: uwtable
define linkonce_odr void @_ZN4MARC15ThreadSafeQueueIsEC2Ev(%"class.MARC::ThreadSafeQueue.3"*) unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %2 = alloca %"class.std::deque.5", align 16
  %3 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 0, i32 0, i32 0
  store i8 1, i8* %3, align 1, !tbaa !135
  %4 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 1
  %5 = bitcast %"class.std::mutex"* %4 to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %5, i8 0, i64 40, i32 8, i1 false) #8
  %6 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 2
  %7 = bitcast %"class.std::deque.5"* %2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 80, i8* nonnull %7) #8
  %8 = getelementptr inbounds %"class.std::deque.5", %"class.std::deque.5"* %2, i64 0, i32 0
  call void @llvm.memset.p0i8.i64(i8* nonnull %7, i8 0, i64 80, i32 16, i1 false) #8
  call void @_ZNSt11_Deque_baseIsSaIsEE17_M_initialize_mapEm(%"class.std::_Deque_base.6"* nonnull %8, i64 0)
  %9 = getelementptr inbounds %"class.std::queue.4", %"class.std::queue.4"* %6, i64 0, i32 0, i32 0
  %10 = bitcast %"class.std::queue.4"* %6 to i8*
  call void @llvm.memset.p0i8.i64(i8* %10, i8 0, i64 80, i32 8, i1 false) #8
  invoke void @_ZNSt11_Deque_baseIsSaIsEE17_M_initialize_mapEm(%"class.std::_Deque_base.6"* %9, i64 0)
          to label %11 unwind label %73

; <label>:11:                                     ; preds = %1
  %12 = getelementptr inbounds %"class.std::deque.5", %"class.std::deque.5"* %2, i64 0, i32 0, i32 0, i32 0
  %13 = load i16**, i16*** %12, align 16, !tbaa !53
  %14 = icmp eq i16** %13, null
  br i1 %14, label %70, label %15

; <label>:15:                                     ; preds = %11
  %16 = ptrtoint i16** %13 to i64
  %17 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2
  %18 = getelementptr inbounds %"class.std::deque.5", %"class.std::deque.5"* %2, i64 0, i32 0, i32 0, i32 2
  %19 = bitcast %"struct.std::_Deque_iterator.10"* %17 to <2 x i64>*
  %20 = load <2 x i64>, <2 x i64>* %19, align 8, !tbaa !27
  %21 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 2
  %22 = bitcast i16** %21 to <2 x i64>*
  %23 = load <2 x i64>, <2 x i64>* %22, align 8, !tbaa !27
  %24 = bitcast %"struct.std::_Deque_iterator.10"* %17 to i8*
  %25 = bitcast %"struct.std::_Deque_iterator.10"* %18 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %24, i8* nonnull %25, i64 32, i32 8, i1 false) #8, !tbaa.struct !155
  %26 = bitcast %"struct.std::_Deque_iterator.10"* %18 to <2 x i64>*
  store <2 x i64> %20, <2 x i64>* %26, align 16
  %27 = getelementptr inbounds %"class.std::deque.5", %"class.std::deque.5"* %2, i64 0, i32 0, i32 0, i32 2, i32 2
  %28 = bitcast i16** %27 to <2 x i64>*
  store <2 x i64> %23, <2 x i64>* %28, align 16
  %29 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3
  %30 = getelementptr inbounds %"class.std::deque.5", %"class.std::deque.5"* %2, i64 0, i32 0, i32 0, i32 3
  %31 = bitcast %"struct.std::_Deque_iterator.10"* %29 to <2 x i64>*
  %32 = load <2 x i64>, <2 x i64>* %31, align 8, !tbaa !27
  %33 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 2
  %34 = bitcast i16** %33 to <2 x i64>*
  %35 = load <2 x i64>, <2 x i64>* %34, align 8, !tbaa !27
  %36 = bitcast %"struct.std::_Deque_iterator.10"* %29 to i8*
  %37 = bitcast %"struct.std::_Deque_iterator.10"* %30 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %36, i8* nonnull %37, i64 32, i32 8, i1 false) #8, !tbaa.struct !155
  %38 = bitcast %"struct.std::_Deque_iterator.10"* %30 to <2 x i64>*
  store <2 x i64> %32, <2 x i64>* %38, align 16
  %39 = getelementptr inbounds %"class.std::deque.5", %"class.std::deque.5"* %2, i64 0, i32 0, i32 0, i32 3, i32 2
  %40 = bitcast i16** %39 to <2 x i64>*
  store <2 x i64> %35, <2 x i64>* %40, align 16
  %41 = bitcast %"class.std::queue.4"* %6 to i64*
  %42 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 1
  %43 = bitcast %"class.std::queue.4"* %6 to <2 x i64>*
  %44 = load <2 x i64>, <2 x i64>* %43, align 8, !tbaa !18
  store i64 %16, i64* %41, align 8, !tbaa !27
  %45 = getelementptr inbounds %"class.std::deque.5", %"class.std::deque.5"* %2, i64 0, i32 0, i32 0, i32 1
  %46 = load i64, i64* %45, align 8, !tbaa !84
  store i64 %46, i64* %42, align 8, !tbaa !84
  %47 = bitcast %"class.std::deque.5"* %2 to <2 x i64>*
  store <2 x i64> %44, <2 x i64>* %47, align 16, !tbaa !18
  %48 = extractelement <2 x i64> %44, i32 0
  %49 = icmp eq i64 %48, 0
  %50 = extractelement <2 x i64> %23, i32 1
  %51 = inttoptr i64 %50 to i16**
  %52 = extractelement <2 x i64> %35, i32 1
  %53 = inttoptr i64 %52 to i16**
  br i1 %49, label %70, label %54

; <label>:54:                                     ; preds = %15
  %55 = inttoptr i64 %48 to i8*
  %56 = getelementptr inbounds i16*, i16** %53, i64 1
  %57 = icmp ugt i16** %56, %51
  br i1 %57, label %58, label %68

; <label>:58:                                     ; preds = %54
  br label %59

; <label>:59:                                     ; preds = %58, %59
  %60 = phi i16** [ %63, %59 ], [ %51, %58 ]
  %61 = bitcast i16** %60 to i8**
  %62 = load i8*, i8** %61, align 8, !tbaa !27
  call void @_ZdlPv(i8* %62) #8
  %63 = getelementptr inbounds i16*, i16** %60, i64 1
  %64 = icmp ult i16** %60, %53
  br i1 %64, label %59, label %65

; <label>:65:                                     ; preds = %59
  %66 = bitcast %"class.std::deque.5"* %2 to i8**
  %67 = load i8*, i8** %66, align 16, !tbaa !53
  br label %68

; <label>:68:                                     ; preds = %65, %54
  %69 = phi i8* [ %67, %65 ], [ %55, %54 ]
  call void @_ZdlPv(i8* %69) #8
  br label %70

; <label>:70:                                     ; preds = %11, %15, %68
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %7) #8
  %71 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 3
  call void @_ZNSt18condition_variableC1Ev(%"class.std::condition_variable"* %71) #8
  %72 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 4
  call void @_ZNSt18condition_variableC1Ev(%"class.std::condition_variable"* %72) #8
  ret void

; <label>:73:                                     ; preds = %1
  %74 = landingpad { i8*, i32 }
          cleanup
  %75 = getelementptr inbounds %"class.std::deque.5", %"class.std::deque.5"* %2, i64 0, i32 0, i32 0, i32 0
  %76 = load i16**, i16*** %75, align 16, !tbaa !53
  %77 = icmp eq i16** %76, null
  br i1 %77, label %98, label %78

; <label>:78:                                     ; preds = %73
  %79 = bitcast i16** %76 to i8*
  %80 = getelementptr inbounds %"class.std::deque.5", %"class.std::deque.5"* %2, i64 0, i32 0, i32 0, i32 2, i32 3
  %81 = load i16**, i16*** %80, align 8, !tbaa !57
  %82 = getelementptr inbounds %"class.std::deque.5", %"class.std::deque.5"* %2, i64 0, i32 0, i32 0, i32 3, i32 3
  %83 = load i16**, i16*** %82, align 8, !tbaa !52
  %84 = getelementptr inbounds i16*, i16** %83, i64 1
  %85 = icmp ult i16** %81, %84
  br i1 %85, label %86, label %96

; <label>:86:                                     ; preds = %78
  br label %87

; <label>:87:                                     ; preds = %86, %87
  %88 = phi i16** [ %91, %87 ], [ %81, %86 ]
  %89 = bitcast i16** %88 to i8**
  %90 = load i8*, i8** %89, align 8, !tbaa !27
  call void @_ZdlPv(i8* %90) #8
  %91 = getelementptr inbounds i16*, i16** %88, i64 1
  %92 = icmp ult i16** %88, %83
  br i1 %92, label %87, label %93

; <label>:93:                                     ; preds = %87
  %94 = bitcast %"class.std::deque.5"* %2 to i8**
  %95 = load i8*, i8** %94, align 16, !tbaa !53
  br label %96

; <label>:96:                                     ; preds = %93, %78
  %97 = phi i8* [ %95, %93 ], [ %79, %78 ]
  call void @_ZdlPv(i8* %97) #8
  br label %98

; <label>:98:                                     ; preds = %73, %96
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %7) #8
  resume { i8*, i32 } %74
}

; Function Attrs: uwtable
define linkonce_odr void @_ZN4MARC15ThreadSafeQueueIiEC2Ev(%"class.MARC::ThreadSafeQueue.11"*) unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %2 = alloca %"class.std::deque.13", align 16
  %3 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 0, i32 0, i32 0
  store i8 1, i8* %3, align 1, !tbaa !135
  %4 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 1
  %5 = bitcast %"class.std::mutex"* %4 to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %5, i8 0, i64 40, i32 8, i1 false) #8
  %6 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 2
  %7 = bitcast %"class.std::deque.13"* %2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 80, i8* nonnull %7) #8
  %8 = getelementptr inbounds %"class.std::deque.13", %"class.std::deque.13"* %2, i64 0, i32 0
  call void @llvm.memset.p0i8.i64(i8* nonnull %7, i8 0, i64 80, i32 16, i1 false) #8
  call void @_ZNSt11_Deque_baseIiSaIiEE17_M_initialize_mapEm(%"class.std::_Deque_base.14"* nonnull %8, i64 0)
  %9 = getelementptr inbounds %"class.std::queue.12", %"class.std::queue.12"* %6, i64 0, i32 0, i32 0
  %10 = bitcast %"class.std::queue.12"* %6 to i8*
  call void @llvm.memset.p0i8.i64(i8* %10, i8 0, i64 80, i32 8, i1 false) #8
  invoke void @_ZNSt11_Deque_baseIiSaIiEE17_M_initialize_mapEm(%"class.std::_Deque_base.14"* %9, i64 0)
          to label %11 unwind label %73

; <label>:11:                                     ; preds = %1
  %12 = getelementptr inbounds %"class.std::deque.13", %"class.std::deque.13"* %2, i64 0, i32 0, i32 0, i32 0
  %13 = load i32**, i32*** %12, align 16, !tbaa !72
  %14 = icmp eq i32** %13, null
  br i1 %14, label %70, label %15

; <label>:15:                                     ; preds = %11
  %16 = ptrtoint i32** %13 to i64
  %17 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2
  %18 = getelementptr inbounds %"class.std::deque.13", %"class.std::deque.13"* %2, i64 0, i32 0, i32 0, i32 2
  %19 = bitcast %"struct.std::_Deque_iterator.18"* %17 to <2 x i64>*
  %20 = load <2 x i64>, <2 x i64>* %19, align 8, !tbaa !27
  %21 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 2
  %22 = bitcast i32** %21 to <2 x i64>*
  %23 = load <2 x i64>, <2 x i64>* %22, align 8, !tbaa !27
  %24 = bitcast %"struct.std::_Deque_iterator.18"* %17 to i8*
  %25 = bitcast %"struct.std::_Deque_iterator.18"* %18 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %24, i8* nonnull %25, i64 32, i32 8, i1 false) #8, !tbaa.struct !155
  %26 = bitcast %"struct.std::_Deque_iterator.18"* %18 to <2 x i64>*
  store <2 x i64> %20, <2 x i64>* %26, align 16
  %27 = getelementptr inbounds %"class.std::deque.13", %"class.std::deque.13"* %2, i64 0, i32 0, i32 0, i32 2, i32 2
  %28 = bitcast i32** %27 to <2 x i64>*
  store <2 x i64> %23, <2 x i64>* %28, align 16
  %29 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3
  %30 = getelementptr inbounds %"class.std::deque.13", %"class.std::deque.13"* %2, i64 0, i32 0, i32 0, i32 3
  %31 = bitcast %"struct.std::_Deque_iterator.18"* %29 to <2 x i64>*
  %32 = load <2 x i64>, <2 x i64>* %31, align 8, !tbaa !27
  %33 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 2
  %34 = bitcast i32** %33 to <2 x i64>*
  %35 = load <2 x i64>, <2 x i64>* %34, align 8, !tbaa !27
  %36 = bitcast %"struct.std::_Deque_iterator.18"* %29 to i8*
  %37 = bitcast %"struct.std::_Deque_iterator.18"* %30 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %36, i8* nonnull %37, i64 32, i32 8, i1 false) #8, !tbaa.struct !155
  %38 = bitcast %"struct.std::_Deque_iterator.18"* %30 to <2 x i64>*
  store <2 x i64> %32, <2 x i64>* %38, align 16
  %39 = getelementptr inbounds %"class.std::deque.13", %"class.std::deque.13"* %2, i64 0, i32 0, i32 0, i32 3, i32 2
  %40 = bitcast i32** %39 to <2 x i64>*
  store <2 x i64> %35, <2 x i64>* %40, align 16
  %41 = bitcast %"class.std::queue.12"* %6 to i64*
  %42 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 1
  %43 = bitcast %"class.std::queue.12"* %6 to <2 x i64>*
  %44 = load <2 x i64>, <2 x i64>* %43, align 8, !tbaa !18
  store i64 %16, i64* %41, align 8, !tbaa !27
  %45 = getelementptr inbounds %"class.std::deque.13", %"class.std::deque.13"* %2, i64 0, i32 0, i32 0, i32 1
  %46 = load i64, i64* %45, align 8, !tbaa !84
  store i64 %46, i64* %42, align 8, !tbaa !84
  %47 = bitcast %"class.std::deque.13"* %2 to <2 x i64>*
  store <2 x i64> %44, <2 x i64>* %47, align 16, !tbaa !18
  %48 = extractelement <2 x i64> %44, i32 0
  %49 = icmp eq i64 %48, 0
  %50 = extractelement <2 x i64> %23, i32 1
  %51 = inttoptr i64 %50 to i32**
  %52 = extractelement <2 x i64> %35, i32 1
  %53 = inttoptr i64 %52 to i32**
  br i1 %49, label %70, label %54

; <label>:54:                                     ; preds = %15
  %55 = inttoptr i64 %48 to i8*
  %56 = getelementptr inbounds i32*, i32** %53, i64 1
  %57 = icmp ugt i32** %56, %51
  br i1 %57, label %58, label %68

; <label>:58:                                     ; preds = %54
  br label %59

; <label>:59:                                     ; preds = %58, %59
  %60 = phi i32** [ %63, %59 ], [ %51, %58 ]
  %61 = bitcast i32** %60 to i8**
  %62 = load i8*, i8** %61, align 8, !tbaa !27
  call void @_ZdlPv(i8* %62) #8
  %63 = getelementptr inbounds i32*, i32** %60, i64 1
  %64 = icmp ult i32** %60, %53
  br i1 %64, label %59, label %65

; <label>:65:                                     ; preds = %59
  %66 = bitcast %"class.std::deque.13"* %2 to i8**
  %67 = load i8*, i8** %66, align 16, !tbaa !72
  br label %68

; <label>:68:                                     ; preds = %65, %54
  %69 = phi i8* [ %67, %65 ], [ %55, %54 ]
  call void @_ZdlPv(i8* %69) #8
  br label %70

; <label>:70:                                     ; preds = %11, %15, %68
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %7) #8
  %71 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 3
  call void @_ZNSt18condition_variableC1Ev(%"class.std::condition_variable"* %71) #8
  %72 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 4
  call void @_ZNSt18condition_variableC1Ev(%"class.std::condition_variable"* %72) #8
  ret void

; <label>:73:                                     ; preds = %1
  %74 = landingpad { i8*, i32 }
          cleanup
  %75 = getelementptr inbounds %"class.std::deque.13", %"class.std::deque.13"* %2, i64 0, i32 0, i32 0, i32 0
  %76 = load i32**, i32*** %75, align 16, !tbaa !72
  %77 = icmp eq i32** %76, null
  br i1 %77, label %98, label %78

; <label>:78:                                     ; preds = %73
  %79 = bitcast i32** %76 to i8*
  %80 = getelementptr inbounds %"class.std::deque.13", %"class.std::deque.13"* %2, i64 0, i32 0, i32 0, i32 2, i32 3
  %81 = load i32**, i32*** %80, align 8, !tbaa !76
  %82 = getelementptr inbounds %"class.std::deque.13", %"class.std::deque.13"* %2, i64 0, i32 0, i32 0, i32 3, i32 3
  %83 = load i32**, i32*** %82, align 8, !tbaa !71
  %84 = getelementptr inbounds i32*, i32** %83, i64 1
  %85 = icmp ult i32** %81, %84
  br i1 %85, label %86, label %96

; <label>:86:                                     ; preds = %78
  br label %87

; <label>:87:                                     ; preds = %86, %87
  %88 = phi i32** [ %91, %87 ], [ %81, %86 ]
  %89 = bitcast i32** %88 to i8**
  %90 = load i8*, i8** %89, align 8, !tbaa !27
  call void @_ZdlPv(i8* %90) #8
  %91 = getelementptr inbounds i32*, i32** %88, i64 1
  %92 = icmp ult i32** %88, %83
  br i1 %92, label %87, label %93

; <label>:93:                                     ; preds = %87
  %94 = bitcast %"class.std::deque.13"* %2 to i8**
  %95 = load i8*, i8** %94, align 16, !tbaa !72
  br label %96

; <label>:96:                                     ; preds = %93, %78
  %97 = phi i8* [ %95, %93 ], [ %79, %78 ]
  call void @_ZdlPv(i8* %97) #8
  br label %98

; <label>:98:                                     ; preds = %73, %96
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %7) #8
  resume { i8*, i32 } %74
}

; Function Attrs: uwtable
define linkonce_odr void @_ZN4MARC15ThreadSafeQueueIlEC2Ev(%"class.MARC::ThreadSafeQueue.19"*) unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %2 = alloca %"class.std::deque.21", align 16
  %3 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 0, i32 0, i32 0
  store i8 1, i8* %3, align 1, !tbaa !135
  %4 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 1
  %5 = bitcast %"class.std::mutex"* %4 to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %5, i8 0, i64 40, i32 8, i1 false) #8
  %6 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 2
  %7 = bitcast %"class.std::deque.21"* %2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 80, i8* nonnull %7) #8
  %8 = getelementptr inbounds %"class.std::deque.21", %"class.std::deque.21"* %2, i64 0, i32 0
  call void @llvm.memset.p0i8.i64(i8* nonnull %7, i8 0, i64 80, i32 16, i1 false) #8
  call void @_ZNSt11_Deque_baseIlSaIlEE17_M_initialize_mapEm(%"class.std::_Deque_base.22"* nonnull %8, i64 0)
  %9 = getelementptr inbounds %"class.std::queue.20", %"class.std::queue.20"* %6, i64 0, i32 0, i32 0
  %10 = bitcast %"class.std::queue.20"* %6 to i8*
  call void @llvm.memset.p0i8.i64(i8* %10, i8 0, i64 80, i32 8, i1 false) #8
  invoke void @_ZNSt11_Deque_baseIlSaIlEE17_M_initialize_mapEm(%"class.std::_Deque_base.22"* %9, i64 0)
          to label %11 unwind label %73

; <label>:11:                                     ; preds = %1
  %12 = getelementptr inbounds %"class.std::deque.21", %"class.std::deque.21"* %2, i64 0, i32 0, i32 0, i32 0
  %13 = load i64**, i64*** %12, align 16, !tbaa !92
  %14 = icmp eq i64** %13, null
  br i1 %14, label %70, label %15

; <label>:15:                                     ; preds = %11
  %16 = ptrtoint i64** %13 to i64
  %17 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2
  %18 = getelementptr inbounds %"class.std::deque.21", %"class.std::deque.21"* %2, i64 0, i32 0, i32 0, i32 2
  %19 = bitcast %"struct.std::_Deque_iterator.26"* %17 to <2 x i64>*
  %20 = load <2 x i64>, <2 x i64>* %19, align 8, !tbaa !27
  %21 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 2
  %22 = bitcast i64** %21 to <2 x i64>*
  %23 = load <2 x i64>, <2 x i64>* %22, align 8, !tbaa !27
  %24 = bitcast %"struct.std::_Deque_iterator.26"* %17 to i8*
  %25 = bitcast %"struct.std::_Deque_iterator.26"* %18 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %24, i8* nonnull %25, i64 32, i32 8, i1 false) #8, !tbaa.struct !155
  %26 = bitcast %"struct.std::_Deque_iterator.26"* %18 to <2 x i64>*
  store <2 x i64> %20, <2 x i64>* %26, align 16
  %27 = getelementptr inbounds %"class.std::deque.21", %"class.std::deque.21"* %2, i64 0, i32 0, i32 0, i32 2, i32 2
  %28 = bitcast i64** %27 to <2 x i64>*
  store <2 x i64> %23, <2 x i64>* %28, align 16
  %29 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3
  %30 = getelementptr inbounds %"class.std::deque.21", %"class.std::deque.21"* %2, i64 0, i32 0, i32 0, i32 3
  %31 = bitcast %"struct.std::_Deque_iterator.26"* %29 to <2 x i64>*
  %32 = load <2 x i64>, <2 x i64>* %31, align 8, !tbaa !27
  %33 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 2
  %34 = bitcast i64** %33 to <2 x i64>*
  %35 = load <2 x i64>, <2 x i64>* %34, align 8, !tbaa !27
  %36 = bitcast %"struct.std::_Deque_iterator.26"* %29 to i8*
  %37 = bitcast %"struct.std::_Deque_iterator.26"* %30 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %36, i8* nonnull %37, i64 32, i32 8, i1 false) #8, !tbaa.struct !155
  %38 = bitcast %"struct.std::_Deque_iterator.26"* %30 to <2 x i64>*
  store <2 x i64> %32, <2 x i64>* %38, align 16
  %39 = getelementptr inbounds %"class.std::deque.21", %"class.std::deque.21"* %2, i64 0, i32 0, i32 0, i32 3, i32 2
  %40 = bitcast i64** %39 to <2 x i64>*
  store <2 x i64> %35, <2 x i64>* %40, align 16
  %41 = bitcast %"class.std::queue.20"* %6 to i64*
  %42 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 1
  %43 = bitcast %"class.std::queue.20"* %6 to <2 x i64>*
  %44 = load <2 x i64>, <2 x i64>* %43, align 8, !tbaa !18
  store i64 %16, i64* %41, align 8, !tbaa !27
  %45 = getelementptr inbounds %"class.std::deque.21", %"class.std::deque.21"* %2, i64 0, i32 0, i32 0, i32 1
  %46 = load i64, i64* %45, align 8, !tbaa !84
  store i64 %46, i64* %42, align 8, !tbaa !84
  %47 = bitcast %"class.std::deque.21"* %2 to <2 x i64>*
  store <2 x i64> %44, <2 x i64>* %47, align 16, !tbaa !18
  %48 = extractelement <2 x i64> %44, i32 0
  %49 = icmp eq i64 %48, 0
  %50 = extractelement <2 x i64> %23, i32 1
  %51 = inttoptr i64 %50 to i64**
  %52 = extractelement <2 x i64> %35, i32 1
  %53 = inttoptr i64 %52 to i64**
  br i1 %49, label %70, label %54

; <label>:54:                                     ; preds = %15
  %55 = inttoptr i64 %48 to i8*
  %56 = getelementptr inbounds i64*, i64** %53, i64 1
  %57 = icmp ugt i64** %56, %51
  br i1 %57, label %58, label %68

; <label>:58:                                     ; preds = %54
  br label %59

; <label>:59:                                     ; preds = %58, %59
  %60 = phi i64** [ %63, %59 ], [ %51, %58 ]
  %61 = bitcast i64** %60 to i8**
  %62 = load i8*, i8** %61, align 8, !tbaa !27
  call void @_ZdlPv(i8* %62) #8
  %63 = getelementptr inbounds i64*, i64** %60, i64 1
  %64 = icmp ult i64** %60, %53
  br i1 %64, label %59, label %65

; <label>:65:                                     ; preds = %59
  %66 = bitcast %"class.std::deque.21"* %2 to i8**
  %67 = load i8*, i8** %66, align 16, !tbaa !92
  br label %68

; <label>:68:                                     ; preds = %65, %54
  %69 = phi i8* [ %67, %65 ], [ %55, %54 ]
  call void @_ZdlPv(i8* %69) #8
  br label %70

; <label>:70:                                     ; preds = %11, %15, %68
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %7) #8
  %71 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 3
  call void @_ZNSt18condition_variableC1Ev(%"class.std::condition_variable"* %71) #8
  %72 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 4
  call void @_ZNSt18condition_variableC1Ev(%"class.std::condition_variable"* %72) #8
  ret void

; <label>:73:                                     ; preds = %1
  %74 = landingpad { i8*, i32 }
          cleanup
  %75 = getelementptr inbounds %"class.std::deque.21", %"class.std::deque.21"* %2, i64 0, i32 0, i32 0, i32 0
  %76 = load i64**, i64*** %75, align 16, !tbaa !92
  %77 = icmp eq i64** %76, null
  br i1 %77, label %98, label %78

; <label>:78:                                     ; preds = %73
  %79 = bitcast i64** %76 to i8*
  %80 = getelementptr inbounds %"class.std::deque.21", %"class.std::deque.21"* %2, i64 0, i32 0, i32 0, i32 2, i32 3
  %81 = load i64**, i64*** %80, align 8, !tbaa !96
  %82 = getelementptr inbounds %"class.std::deque.21", %"class.std::deque.21"* %2, i64 0, i32 0, i32 0, i32 3, i32 3
  %83 = load i64**, i64*** %82, align 8, !tbaa !91
  %84 = getelementptr inbounds i64*, i64** %83, i64 1
  %85 = icmp ult i64** %81, %84
  br i1 %85, label %86, label %96

; <label>:86:                                     ; preds = %78
  br label %87

; <label>:87:                                     ; preds = %86, %87
  %88 = phi i64** [ %91, %87 ], [ %81, %86 ]
  %89 = bitcast i64** %88 to i8**
  %90 = load i8*, i8** %89, align 8, !tbaa !27
  call void @_ZdlPv(i8* %90) #8
  %91 = getelementptr inbounds i64*, i64** %88, i64 1
  %92 = icmp ult i64** %88, %83
  br i1 %92, label %87, label %93

; <label>:93:                                     ; preds = %87
  %94 = bitcast %"class.std::deque.21"* %2 to i8**
  %95 = load i8*, i8** %94, align 16, !tbaa !92
  br label %96

; <label>:96:                                     ; preds = %93, %78
  %97 = phi i8* [ %95, %93 ], [ %79, %78 ]
  call void @_ZdlPv(i8* %97) #8
  br label %98

; <label>:98:                                     ; preds = %73, %96
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %7) #8
  resume { i8*, i32 } %74
}

; Function Attrs: inlinehint uwtable
declare dereferenceable(272) %"class.std::basic_ostream"* @_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(%"class.std::basic_ostream"* dereferenceable(272), i8*) local_unnamed_addr #14

; Function Attrs: noreturn nounwind
declare void @abort() local_unnamed_addr #15

; Function Attrs: noinline noreturn nounwind
define linkonce_odr hidden void @__clang_call_terminate(i8*) local_unnamed_addr #16 comdat {
  %2 = tail call i8* @__cxa_begin_catch(i8* %0) #8
  tail call void @_ZSt9terminatev() #22
  unreachable
}

; Function Attrs: uwtable
define linkonce_odr void @_ZN4MARC10ThreadPool6submitIRPFvPvS2_EJRS2_S2_EEEDaOT_DpOT0_(%"class.MARC::TaskFuture"* noalias sret, %"class.MARC::ThreadPool"*, void (i8*, i8*)** dereferenceable(8), i8** dereferenceable(8), i8** dereferenceable(8)) local_unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %6 = alloca <2 x i64>, align 16
  %7 = bitcast <2 x i64>* %6 to %"class.std::shared_ptr.121"*
  %8 = alloca <2 x i64>, align 16
  %9 = bitcast <2 x i64>* %8 to %"class.std::packaged_task"*
  %10 = alloca <2 x i64>, align 16
  %11 = bitcast <2 x i64>* %10 to %"class.MARC::TaskFuture"*
  %12 = alloca <2 x i64>, align 16
  %13 = bitcast <2 x i64>* %12 to %"class.std::future"*
  %14 = bitcast void (i8*, i8*)** %2 to i64*
  %15 = load i64, i64* %14, align 8, !tbaa !27, !noalias !156
  %16 = bitcast i8** %4 to i64*
  %17 = load i64, i64* %16, align 8, !tbaa !27, !noalias !156
  %18 = bitcast i8** %3 to i64*
  %19 = load i64, i64* %18, align 8, !tbaa !27, !noalias !156
  %20 = bitcast <2 x i64>* %8 to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %20) #8
  %21 = bitcast <2 x i64>* %6 to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %21) #8, !noalias !159
  %22 = getelementptr inbounds %"class.std::shared_ptr.121", %"class.std::shared_ptr.121"* %7, i64 0, i32 0, i32 1
  %23 = bitcast <2 x i64>* %6 to i8*
  call void @llvm.memset.p0i8.i64(i8* nonnull %23, i8 0, i64 16, i32 16, i1 false)
  %24 = invoke i8* @_Znwm(i64 80)
          to label %25 unwind label %57, !noalias !162

; <label>:25:                                     ; preds = %5
  %26 = bitcast i8* %24 to i32 (...)***
  %27 = getelementptr inbounds i8, i8* %24, i64 8
  %28 = bitcast i8* %27 to i32*
  store i32 1, i32* %28, align 8, !tbaa !165, !noalias !162
  %29 = getelementptr inbounds i8, i8* %24, i64 12
  %30 = bitcast i8* %29 to i32*
  store i32 1, i32* %30, align 4, !tbaa !167, !noalias !162
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [7 x i8*] }, { [7 x i8*] }* @_ZTVSt23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %26, align 8, !tbaa !133, !noalias !162
  %31 = getelementptr inbounds i8, i8* %24, i64 16
  %32 = bitcast i8* %31 to i32 (...)***
  %33 = getelementptr inbounds i8, i8* %24, i64 24
  %34 = bitcast i8* %33 to %"struct.std::__future_base::_Result_base"**
  store %"struct.std::__future_base::_Result_base"* null, %"struct.std::__future_base::_Result_base"** %34, align 8, !tbaa !168, !noalias !162
  %35 = getelementptr inbounds i8, i8* %24, i64 32
  %36 = bitcast i8* %35 to i32*
  store i32 0, i32* %36, align 4, !tbaa !170, !noalias !162
  %37 = getelementptr inbounds i8, i8* %24, i64 36
  store i8 0, i8* %37, align 1, !tbaa !172, !noalias !162
  %38 = getelementptr inbounds i8, i8* %24, i64 40
  %39 = bitcast i8* %38 to i32*
  store i32 0, i32* %39, align 4, !tbaa !174, !noalias !162
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [9 x i8*] }, { [9 x i8*] }* @_ZTVNSt13__future_base16_Task_state_baseIFvvEEE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %32, align 8, !tbaa !133, !noalias !162
  %40 = getelementptr inbounds i8, i8* %24, i64 48
  %41 = invoke i8* @_Znwm(i64 16) #24
          to label %42 unwind label %44, !noalias !162

; <label>:42:                                     ; preds = %25
  %43 = bitcast i8* %41 to %"struct.std::__future_base::_Result_base"*
  invoke void @_ZNSt13__future_base12_Result_baseC2Ev(%"struct.std::__future_base::_Result_base"* nonnull %43)
          to label %70 unwind label %46, !noalias !176

; <label>:44:                                     ; preds = %25
  %45 = landingpad { i8*, i32 }
          cleanup
  br label %61

; <label>:46:                                     ; preds = %42
  %47 = landingpad { i8*, i32 }
          cleanup
  tail call void @_ZdlPv(i8* nonnull %41) #25, !noalias !176
  %48 = load %"struct.std::__future_base::_Result_base"*, %"struct.std::__future_base::_Result_base"** %34, align 8, !tbaa !27, !noalias !162
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [6 x i8*] }, { [6 x i8*] }* @_ZTVNSt13__future_base13_State_baseV2E, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %32, align 8, !tbaa !133, !noalias !162
  %49 = icmp eq %"struct.std::__future_base::_Result_base"* %48, null
  br i1 %49, label %61, label %50

; <label>:50:                                     ; preds = %46
  %51 = bitcast %"struct.std::__future_base::_Result_base"* %48 to void (%"struct.std::__future_base::_Result_base"*)***
  %52 = load void (%"struct.std::__future_base::_Result_base"*)**, void (%"struct.std::__future_base::_Result_base"*)*** %51, align 8, !tbaa !133, !noalias !162
  %53 = load void (%"struct.std::__future_base::_Result_base"*)*, void (%"struct.std::__future_base::_Result_base"*)** %52, align 8, !noalias !162
  invoke void %53(%"struct.std::__future_base::_Result_base"* nonnull %48)
          to label %61 unwind label %54, !noalias !162

; <label>:54:                                     ; preds = %50
  %55 = landingpad { i8*, i32 }
          catch i8* null
  %56 = extractvalue { i8*, i32 } %55, 0
  tail call void @__clang_call_terminate(i8* %56) #22, !noalias !162
  unreachable

; <label>:57:                                     ; preds = %5
  %58 = landingpad { i8*, i32 }
          cleanup
  %59 = extractvalue { i8*, i32 } %58, 0
  %60 = extractvalue { i8*, i32 } %58, 1
  br label %65

; <label>:61:                                     ; preds = %44, %46, %50
  %62 = phi { i8*, i32 } [ %45, %44 ], [ %47, %46 ], [ %47, %50 ]
  %63 = extractvalue { i8*, i32 } %62, 0
  %64 = extractvalue { i8*, i32 } %62, 1
  tail call void @_ZdlPv(i8* nonnull %24) #8
  br label %65

; <label>:65:                                     ; preds = %57, %61
  %66 = phi i32 [ %64, %61 ], [ %60, %57 ]
  %67 = phi i8* [ %63, %61 ], [ %59, %57 ]
  %68 = insertvalue { i8*, i32 } undef, i8* %67, 0
  %69 = insertvalue { i8*, i32 } %68, i32 %66, 1
  resume { i8*, i32 } %69

; <label>:70:                                     ; preds = %42
  %71 = bitcast i8* %41 to i32 (...)***
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [5 x i8*] }, { [5 x i8*] }* @_ZTVNSt13__future_base7_ResultIvEE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %71, align 8, !tbaa !133, !noalias !176
  %72 = bitcast i8* %40 to i8**
  store i8* %41, i8** %72, align 8, !tbaa !27, !alias.scope !179, !noalias !162
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [9 x i8*] }, { [9 x i8*] }* @_ZTVNSt13__future_base11_Task_stateISt5_BindIFPFvPvS2_ES2_S2_EESaIiEFvvEEE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %32, align 8, !tbaa !133, !noalias !162
  %73 = getelementptr inbounds i8, i8* %24, i64 56
  %74 = bitcast i8* %73 to i64*
  store i64 %15, i64* %74, align 8, !tbaa !180, !noalias !162
  %75 = getelementptr inbounds i8, i8* %24, i64 64
  %76 = bitcast i8* %75 to i64*
  store i64 %17, i64* %76, align 8, !tbaa !183, !noalias !162
  %77 = getelementptr inbounds i8, i8* %24, i64 72
  %78 = bitcast i8* %77 to i64*
  store i64 %19, i64* %78, align 8, !tbaa !185, !noalias !162
  %79 = bitcast %"class.std::__shared_count"* %22 to i8**
  store i8* %24, i8** %79, align 8, !tbaa !129, !alias.scope !162
  %80 = bitcast i8* %24 to %"class.std::_Sp_counted_base"*
  %81 = bitcast i8* %24 to i8* (%"class.std::_Sp_counted_base"*, %"class.std::type_info"*)***
  %82 = load i8* (%"class.std::_Sp_counted_base"*, %"class.std::type_info"*)**, i8* (%"class.std::_Sp_counted_base"*, %"class.std::type_info"*)*** %81, align 8, !tbaa !133, !noalias !162
  %83 = getelementptr inbounds i8* (%"class.std::_Sp_counted_base"*, %"class.std::type_info"*)*, i8* (%"class.std::_Sp_counted_base"*, %"class.std::type_info"*)** %82, i64 4
  %84 = load i8* (%"class.std::_Sp_counted_base"*, %"class.std::type_info"*)*, i8* (%"class.std::_Sp_counted_base"*, %"class.std::type_info"*)** %83, align 8, !noalias !162
  %85 = tail call i8* %84(%"class.std::_Sp_counted_base"* nonnull %80, %"class.std::type_info"* nonnull dereferenceable(16) bitcast ({ i8*, i8* }* @_ZTISt19_Sp_make_shared_tag to %"class.std::type_info"*)) #8, !noalias !162
  %86 = bitcast <2 x i64>* %6 to i8**
  store i8* %85, i8** %86, align 16, !tbaa !187, !alias.scope !162
  call void (%"class.std::__shared_count"*, ...) @_ZSt32__enable_shared_from_this_helperILN9__gnu_cxx12_Lock_policyE2EEvRKSt14__shared_countIXT_EEz(%"class.std::__shared_count"* nonnull dereferenceable(8) %22, i8* %85, i8* %85) #8
  %87 = load <2 x i64>, <2 x i64>* %6, align 16, !tbaa !27, !noalias !159
  store <2 x i64> %87, <2 x i64>* %8, align 16, !tbaa !27, !alias.scope !159
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %21) #8, !noalias !159
  %88 = bitcast <2 x i64>* %10 to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %88) #8
  %89 = bitcast <2 x i64>* %12 to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %89) #8
  invoke void @_ZNSt13packaged_taskIFvvEE10get_futureEv(%"class.std::future"* nonnull sret %13, %"class.std::packaged_task"* nonnull %9)
          to label %90 unwind label %160

; <label>:90:                                     ; preds = %70
  %91 = load <2 x i64>, <2 x i64>* %12, align 16, !tbaa !27
  store <2 x i64> %91, <2 x i64>* %10, align 16, !tbaa !27
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %89) #8
  %92 = invoke i8* @_Znwm(i64 24) #24
          to label %93 unwind label %164

; <label>:93:                                     ; preds = %90
  %94 = bitcast i8* %92 to i32 (...)***
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [5 x i8*] }, { [5 x i8*] }* @_ZTVN4MARC10ThreadTaskISt13packaged_taskIFvvEEEE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %94, align 8, !tbaa !133, !noalias !189
  %95 = getelementptr inbounds i8, i8* %92, i64 8
  %96 = load <2 x i64>, <2 x i64>* %8, align 16, !tbaa !27, !noalias !189
  store <2 x i64> zeroinitializer, <2 x i64>* %8, align 16, !tbaa !27, !noalias !189
  %97 = bitcast i8* %95 to <2 x i64>*
  store <2 x i64> %96, <2 x i64>* %97, align 8, !tbaa !27, !noalias !189
  %98 = ptrtoint i8* %92 to i64
  %99 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %1, i64 0, i32 1, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %100, label %106

; <label>:100:                                    ; preds = %93
  %101 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %99, i64 0, i32 0, i32 0
  %102 = call i32 @pthread_mutex_lock(%union.pthread_mutex_t* nonnull %101) #8
  %103 = icmp eq i32 %102, 0
  br i1 %103, label %106, label %104

; <label>:104:                                    ; preds = %100
  invoke void @_ZSt20__throw_system_errori(i32 %102) #23
          to label %105 unwind label %170

; <label>:105:                                    ; preds = %104
  unreachable

; <label>:106:                                    ; preds = %100, %93
  %107 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %1, i64 0, i32 1, i32 2
  %108 = getelementptr inbounds %"class.std::queue.33", %"class.std::queue.33"* %107, i64 0, i32 0
  %109 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %1, i64 0, i32 1, i32 2, i32 0, i32 0, i32 0, i32 3, i32 0
  %110 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %109, align 8, !tbaa !192
  %111 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %1, i64 0, i32 1, i32 2, i32 0, i32 0, i32 0, i32 3, i32 2
  %112 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %111, align 8, !tbaa !196
  %113 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %112, i64 -1
  %114 = icmp eq %"class.std::unique_ptr"* %110, %113
  br i1 %114, label %118, label %115

; <label>:115:                                    ; preds = %106
  %116 = bitcast %"class.std::unique_ptr"* %110 to i64*
  store i64 %98, i64* %116, align 8, !tbaa !197
  %117 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %110, i64 1
  store %"class.std::unique_ptr"* %117, %"class.std::unique_ptr"** %109, align 8, !tbaa !192
  br label %147

; <label>:118:                                    ; preds = %106
  %119 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %1, i64 0, i32 1, i32 2, i32 0, i32 0, i32 0, i32 1
  %120 = load i64, i64* %119, align 8, !tbaa !199
  %121 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %1, i64 0, i32 1, i32 2, i32 0, i32 0, i32 0, i32 3, i32 3
  %122 = bitcast %"class.std::unique_ptr"*** %121 to i64*
  %123 = load i64, i64* %122, align 8, !tbaa !200
  %124 = bitcast %"class.std::queue.33"* %107 to i64*
  %125 = load i64, i64* %124, align 8, !tbaa !201
  %126 = sub i64 %123, %125
  %127 = ashr exact i64 %126, 3
  %128 = sub i64 %120, %127
  %129 = icmp ult i64 %128, 2
  br i1 %129, label %130, label %131

; <label>:130:                                    ; preds = %118
  invoke void @_ZNSt5dequeISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EESaIS5_EE17_M_reallocate_mapEmb(%"class.std::deque.34"* nonnull %108, i64 1, i1 zeroext false)
          to label %131 unwind label %152

; <label>:131:                                    ; preds = %130, %118
  %132 = invoke i8* @_Znwm(i64 512)
          to label %133 unwind label %152

; <label>:133:                                    ; preds = %131
  %134 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %1, i64 0, i32 1, i32 2, i32 0, i32 0, i32 0, i32 3
  %135 = load %"class.std::unique_ptr"**, %"class.std::unique_ptr"*** %121, align 8, !tbaa !200
  %136 = getelementptr inbounds %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %135, i64 1
  %137 = bitcast %"class.std::unique_ptr"** %136 to i8**
  store i8* %132, i8** %137, align 8, !tbaa !27
  %138 = bitcast %"struct.std::_Deque_iterator.41"* %134 to i64**
  %139 = load i64*, i64** %138, align 8, !tbaa !192
  store i64 %98, i64* %139, align 8, !tbaa !197
  %140 = load %"class.std::unique_ptr"**, %"class.std::unique_ptr"*** %121, align 8, !tbaa !200
  %141 = getelementptr inbounds %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %140, i64 1
  store %"class.std::unique_ptr"** %141, %"class.std::unique_ptr"*** %121, align 8, !tbaa !202
  %142 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %141, align 8, !tbaa !27
  %143 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %1, i64 0, i32 1, i32 2, i32 0, i32 0, i32 0, i32 3, i32 1
  store %"class.std::unique_ptr"* %142, %"class.std::unique_ptr"** %143, align 8, !tbaa !203
  %144 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %142, i64 64
  store %"class.std::unique_ptr"* %144, %"class.std::unique_ptr"** %111, align 8, !tbaa !204
  %145 = ptrtoint %"class.std::unique_ptr"* %142 to i64
  %146 = bitcast %"struct.std::_Deque_iterator.41"* %134 to i64*
  store i64 %145, i64* %146, align 8, !tbaa !192
  br label %147

; <label>:147:                                    ; preds = %133, %115
  %148 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %1, i64 0, i32 1, i32 3
  call void @_ZNSt18condition_variable10notify_oneEv(%"class.std::condition_variable"* %148) #8
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %149, label %157

; <label>:149:                                    ; preds = %147
  %150 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %99, i64 0, i32 0, i32 0
  %151 = call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %150) #8
  br label %157

; <label>:152:                                    ; preds = %131, %130
  %153 = landingpad { i8*, i32 }
          cleanup
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %154, label %172

; <label>:154:                                    ; preds = %152
  %155 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %99, i64 0, i32 0, i32 0
  %156 = call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %155) #8
  br label %172

; <label>:157:                                    ; preds = %149, %147
  %158 = load <2 x i64>, <2 x i64>* %10, align 16, !tbaa !27
  %159 = bitcast %"class.MARC::TaskFuture"* %0 to <2 x i64>*
  store <2 x i64> %158, <2 x i64>* %159, align 8, !tbaa !27
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %88) #8
  call void @_ZNSt13packaged_taskIFvvEED2Ev(%"class.std::packaged_task"* nonnull %9) #8
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %20) #8
  ret void

; <label>:160:                                    ; preds = %70
  %161 = landingpad { i8*, i32 }
          cleanup
  %162 = extractvalue { i8*, i32 } %161, 0
  %163 = extractvalue { i8*, i32 } %161, 1
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %89) #8
  br label %226

; <label>:164:                                    ; preds = %90
  %165 = landingpad { i8*, i32 }
          cleanup
  %166 = extractelement <2 x i64> %91, i32 0
  %167 = inttoptr i64 %166 to %"class.std::__future_base::_State_baseV2"*
  %168 = extractvalue { i8*, i32 } %165, 0
  %169 = extractvalue { i8*, i32 } %165, 1
  br label %183

; <label>:170:                                    ; preds = %104
  %171 = landingpad { i8*, i32 }
          cleanup
  br label %172

; <label>:172:                                    ; preds = %152, %154, %170
  %173 = phi { i8*, i32 } [ %171, %170 ], [ %153, %154 ], [ %153, %152 ]
  %174 = extractvalue { i8*, i32 } %173, 0
  %175 = extractvalue { i8*, i32 } %173, 1
  %176 = bitcast i8* %92 to %"class.MARC::IThreadTask"*
  %177 = bitcast i8* %92 to void (%"class.MARC::IThreadTask"*)***
  %178 = load void (%"class.MARC::IThreadTask"*)**, void (%"class.MARC::IThreadTask"*)*** %177, align 8, !tbaa !133
  %179 = getelementptr inbounds void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %178, i64 2
  %180 = load void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %179, align 8
  call void %180(%"class.MARC::IThreadTask"* nonnull %176) #8
  %181 = bitcast <2 x i64>* %10 to %"class.std::__future_base::_State_baseV2"**
  %182 = load %"class.std::__future_base::_State_baseV2"*, %"class.std::__future_base::_State_baseV2"** %181, align 16, !tbaa !131
  br label %183

; <label>:183:                                    ; preds = %172, %164
  %184 = phi %"class.std::__future_base::_State_baseV2"* [ %167, %164 ], [ %182, %172 ]
  %185 = phi i8* [ %168, %164 ], [ %174, %172 ]
  %186 = phi i32 [ %169, %164 ], [ %175, %172 ]
  %187 = bitcast <2 x i64>* %10 to %"class.std::__basic_future"*
  %188 = icmp eq %"class.std::__future_base::_State_baseV2"* %184, null
  br i1 %188, label %194, label %189

; <label>:189:                                    ; preds = %183
  %190 = bitcast <2 x i64>* %10 to %"class.std::future"*
  invoke void @_ZNSt6futureIvE3getEv(%"class.std::future"* nonnull %190)
          to label %194 unwind label %191

; <label>:191:                                    ; preds = %189
  %192 = landingpad { i8*, i32 }
          catch i8* null
  %193 = extractvalue { i8*, i32 } %192, 0
  call void @_ZNSt14__basic_futureIvED2Ev(%"class.std::__basic_future"* nonnull %187) #8
  call void @__clang_call_terminate(i8* %193) #22
  unreachable

; <label>:194:                                    ; preds = %189, %183
  %195 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %11, i64 0, i32 0, i32 0, i32 0, i32 0, i32 1, i32 0
  %196 = load %"class.std::_Sp_counted_base"*, %"class.std::_Sp_counted_base"** %195, align 8, !tbaa !129
  %197 = icmp eq %"class.std::_Sp_counted_base"* %196, null
  br i1 %197, label %226, label %198

; <label>:198:                                    ; preds = %194
  %199 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %196, i64 0, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %200, label %202

; <label>:200:                                    ; preds = %198
  %201 = atomicrmw volatile add i32* %199, i32 -1 acq_rel
  br label %205

; <label>:202:                                    ; preds = %198
  %203 = load i32, i32* %199, align 4, !tbaa !17
  %204 = add nsw i32 %203, -1
  store i32 %204, i32* %199, align 4, !tbaa !17
  br label %205

; <label>:205:                                    ; preds = %202, %200
  %206 = phi i32 [ %201, %200 ], [ %203, %202 ]
  %207 = icmp eq i32 %206, 1
  br i1 %207, label %208, label %226

; <label>:208:                                    ; preds = %205
  %209 = bitcast %"class.std::_Sp_counted_base"* %196 to void (%"class.std::_Sp_counted_base"*)***
  %210 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %209, align 8, !tbaa !133
  %211 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %210, i64 2
  %212 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %211, align 8
  call void %212(%"class.std::_Sp_counted_base"* nonnull %196) #8
  %213 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %196, i64 0, i32 2
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %214, label %216

; <label>:214:                                    ; preds = %208
  %215 = atomicrmw volatile add i32* %213, i32 -1 acq_rel
  br label %219

; <label>:216:                                    ; preds = %208
  %217 = load i32, i32* %213, align 4, !tbaa !17
  %218 = add nsw i32 %217, -1
  store i32 %218, i32* %213, align 4, !tbaa !17
  br label %219

; <label>:219:                                    ; preds = %216, %214
  %220 = phi i32 [ %215, %214 ], [ %217, %216 ]
  %221 = icmp eq i32 %220, 1
  br i1 %221, label %222, label %226

; <label>:222:                                    ; preds = %219
  %223 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %209, align 8, !tbaa !133
  %224 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %223, i64 3
  %225 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %224, align 8
  call void %225(%"class.std::_Sp_counted_base"* nonnull %196) #8
  br label %226

; <label>:226:                                    ; preds = %222, %219, %205, %194, %160
  %227 = phi i8* [ %162, %160 ], [ %185, %194 ], [ %185, %205 ], [ %185, %219 ], [ %185, %222 ]
  %228 = phi i32 [ %163, %160 ], [ %186, %194 ], [ %186, %205 ], [ %186, %219 ], [ %186, %222 ]
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %88) #8
  call void @_ZNSt13packaged_taskIFvvEED2Ev(%"class.std::packaged_task"* nonnull %9) #8
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %20) #8
  %229 = insertvalue { i8*, i32 } undef, i8* %227, 0
  %230 = insertvalue { i8*, i32 } %229, i32 %228, 1
  resume { i8*, i32 } %230
}

; Function Attrs: uwtable
define linkonce_odr void @_ZNSt6vectorIN4MARC10TaskFutureIvEESaIS2_EE19_M_emplace_back_auxIJS2_EEEvDpOT_(%"class.std::vector.53"*, %"class.MARC::TaskFuture"* dereferenceable(16)) local_unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %3 = getelementptr inbounds %"class.std::vector.53", %"class.std::vector.53"* %0, i64 0, i32 0, i32 0, i32 1
  %4 = bitcast %"class.MARC::TaskFuture"** %3 to i64*
  %5 = load i64, i64* %4, align 8, !tbaa !125
  %6 = bitcast %"class.std::vector.53"* %0 to i64*
  %7 = load i64, i64* %6, align 8, !tbaa !205
  %8 = sub i64 %5, %7
  %9 = ashr exact i64 %8, 4
  %10 = icmp eq i64 %9, 0
  %11 = select i1 %10, i64 1, i64 %9
  %12 = add nsw i64 %11, %9
  %13 = icmp ult i64 %12, %9
  %14 = icmp ugt i64 %12, 1152921504606846975
  %15 = or i1 %13, %14
  %16 = select i1 %15, i64 1152921504606846975, i64 %12
  %17 = icmp eq i64 %16, 0
  br i1 %17, label %27, label %18

; <label>:18:                                     ; preds = %2
  %19 = icmp ugt i64 %16, 1152921504606846975
  br i1 %19, label %20, label %21

; <label>:20:                                     ; preds = %18
  tail call void @_ZSt17__throw_bad_allocv() #23
  unreachable

; <label>:21:                                     ; preds = %18
  %22 = shl i64 %16, 4
  %23 = tail call i8* @_Znwm(i64 %22)
  %24 = bitcast i8* %23 to %"class.MARC::TaskFuture"*
  %25 = load i64, i64* %4, align 8, !tbaa !125
  %26 = load i64, i64* %6, align 8, !tbaa !205
  br label %27

; <label>:27:                                     ; preds = %21, %2
  %28 = phi i64 [ %25, %21 ], [ %5, %2 ]
  %29 = phi i64 [ %26, %21 ], [ %7, %2 ]
  %30 = phi %"class.MARC::TaskFuture"* [ %24, %21 ], [ null, %2 ]
  %31 = inttoptr i64 %29 to %"class.MARC::TaskFuture"*
  %32 = inttoptr i64 %28 to %"class.MARC::TaskFuture"*
  %33 = sub i64 %28, %29
  %34 = ashr exact i64 %33, 4
  %35 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %30, i64 %34
  %36 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %1, i64 0, i32 0, i32 0, i32 0, i32 0, i32 0
  %37 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %1, i64 0, i32 0, i32 0, i32 0, i32 0, i32 1
  %38 = bitcast %"class.std::__shared_count"* %37 to i64*
  %39 = bitcast %"class.MARC::TaskFuture"* %1 to <2 x i64>*
  %40 = load <2 x i64>, <2 x i64>* %39, align 8, !tbaa !27
  store i64 0, i64* %38, align 8, !tbaa !129
  %41 = bitcast %"class.MARC::TaskFuture"* %35 to <2 x i64>*
  store <2 x i64> %40, <2 x i64>* %41, align 8, !tbaa !27
  store %"class.std::__future_base::_State_baseV2"* null, %"class.std::__future_base::_State_baseV2"** %36, align 8, !tbaa !131
  %42 = getelementptr inbounds %"class.std::vector.53", %"class.std::vector.53"* %0, i64 0, i32 0, i32 0, i32 0
  %43 = icmp eq %"class.MARC::TaskFuture"* %31, %32
  br i1 %43, label %170, label %44

; <label>:44:                                     ; preds = %27
  %45 = getelementptr %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %32, i64 -1, i32 0, i32 0, i32 0, i32 0, i32 0
  %46 = getelementptr %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %32, i64 -1, i32 0, i32 0, i32 0, i32 0, i32 0
  %47 = bitcast %"class.std::__future_base::_State_baseV2"** %46 to i8*
  %48 = sub i64 0, %29
  %49 = getelementptr i8, i8* %47, i64 %48
  %50 = ptrtoint i8* %49 to i64
  %51 = lshr i64 %50, 4
  %52 = add nuw nsw i64 %51, 1
  %53 = and i64 %52, 3
  %54 = icmp eq i64 %53, 0
  br i1 %54, label %71, label %55

; <label>:55:                                     ; preds = %44
  br label %56

; <label>:56:                                     ; preds = %56, %55
  %57 = phi %"class.MARC::TaskFuture"* [ %30, %55 ], [ %68, %56 ]
  %58 = phi %"class.MARC::TaskFuture"* [ %31, %55 ], [ %67, %56 ]
  %59 = phi i64 [ %53, %55 ], [ %69, %56 ]
  %60 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %58, i64 0, i32 0, i32 0, i32 0, i32 0, i32 0
  %61 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %57, i64 0, i32 0, i32 0, i32 0, i32 0, i32 1, i32 0
  store %"class.std::_Sp_counted_base"* null, %"class.std::_Sp_counted_base"** %61, align 8, !tbaa !129
  %62 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %58, i64 0, i32 0, i32 0, i32 0, i32 0, i32 1
  %63 = bitcast %"class.std::__shared_count"* %62 to i64*
  %64 = bitcast %"class.MARC::TaskFuture"* %58 to <2 x i64>*
  %65 = load <2 x i64>, <2 x i64>* %64, align 8, !tbaa !27
  store i64 0, i64* %63, align 8, !tbaa !129
  %66 = bitcast %"class.MARC::TaskFuture"* %57 to <2 x i64>*
  store <2 x i64> %65, <2 x i64>* %66, align 8, !tbaa !27
  store %"class.std::__future_base::_State_baseV2"* null, %"class.std::__future_base::_State_baseV2"** %60, align 8, !tbaa !131
  %67 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %58, i64 1
  %68 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %57, i64 1
  %69 = add i64 %59, -1
  %70 = icmp eq i64 %69, 0
  br i1 %70, label %71, label %56, !llvm.loop !206

; <label>:71:                                     ; preds = %56, %44
  %72 = phi %"class.MARC::TaskFuture"* [ %30, %44 ], [ %68, %56 ]
  %73 = phi %"class.MARC::TaskFuture"* [ %31, %44 ], [ %67, %56 ]
  %74 = icmp ult i8* %49, inttoptr (i64 48 to i8*)
  br i1 %74, label %116, label %75

; <label>:75:                                     ; preds = %71
  br label %76

; <label>:76:                                     ; preds = %76, %75
  %77 = phi %"class.MARC::TaskFuture"* [ %72, %75 ], [ %114, %76 ]
  %78 = phi %"class.MARC::TaskFuture"* [ %73, %75 ], [ %113, %76 ]
  %79 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %78, i64 0, i32 0, i32 0, i32 0, i32 0, i32 0
  %80 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %77, i64 0, i32 0, i32 0, i32 0, i32 0, i32 1, i32 0
  store %"class.std::_Sp_counted_base"* null, %"class.std::_Sp_counted_base"** %80, align 8, !tbaa !129
  %81 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %78, i64 0, i32 0, i32 0, i32 0, i32 0, i32 1
  %82 = bitcast %"class.std::__shared_count"* %81 to i64*
  %83 = bitcast %"class.MARC::TaskFuture"* %78 to <2 x i64>*
  %84 = load <2 x i64>, <2 x i64>* %83, align 8, !tbaa !27
  store i64 0, i64* %82, align 8, !tbaa !129
  %85 = bitcast %"class.MARC::TaskFuture"* %77 to <2 x i64>*
  store <2 x i64> %84, <2 x i64>* %85, align 8, !tbaa !27
  store %"class.std::__future_base::_State_baseV2"* null, %"class.std::__future_base::_State_baseV2"** %79, align 8, !tbaa !131
  %86 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %78, i64 1
  %87 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %77, i64 1
  %88 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %86, i64 0, i32 0, i32 0, i32 0, i32 0, i32 0
  %89 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %77, i64 1, i32 0, i32 0, i32 0, i32 0, i32 1, i32 0
  store %"class.std::_Sp_counted_base"* null, %"class.std::_Sp_counted_base"** %89, align 8, !tbaa !129
  %90 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %78, i64 1, i32 0, i32 0, i32 0, i32 0, i32 1
  %91 = bitcast %"class.std::__shared_count"* %90 to i64*
  %92 = bitcast %"class.MARC::TaskFuture"* %86 to <2 x i64>*
  %93 = load <2 x i64>, <2 x i64>* %92, align 8, !tbaa !27
  store i64 0, i64* %91, align 8, !tbaa !129
  %94 = bitcast %"class.MARC::TaskFuture"* %87 to <2 x i64>*
  store <2 x i64> %93, <2 x i64>* %94, align 8, !tbaa !27
  store %"class.std::__future_base::_State_baseV2"* null, %"class.std::__future_base::_State_baseV2"** %88, align 8, !tbaa !131
  %95 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %78, i64 2
  %96 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %77, i64 2
  %97 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %95, i64 0, i32 0, i32 0, i32 0, i32 0, i32 0
  %98 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %77, i64 2, i32 0, i32 0, i32 0, i32 0, i32 1, i32 0
  store %"class.std::_Sp_counted_base"* null, %"class.std::_Sp_counted_base"** %98, align 8, !tbaa !129
  %99 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %78, i64 2, i32 0, i32 0, i32 0, i32 0, i32 1
  %100 = bitcast %"class.std::__shared_count"* %99 to i64*
  %101 = bitcast %"class.MARC::TaskFuture"* %95 to <2 x i64>*
  %102 = load <2 x i64>, <2 x i64>* %101, align 8, !tbaa !27
  store i64 0, i64* %100, align 8, !tbaa !129
  %103 = bitcast %"class.MARC::TaskFuture"* %96 to <2 x i64>*
  store <2 x i64> %102, <2 x i64>* %103, align 8, !tbaa !27
  store %"class.std::__future_base::_State_baseV2"* null, %"class.std::__future_base::_State_baseV2"** %97, align 8, !tbaa !131
  %104 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %78, i64 3
  %105 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %77, i64 3
  %106 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %104, i64 0, i32 0, i32 0, i32 0, i32 0, i32 0
  %107 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %77, i64 3, i32 0, i32 0, i32 0, i32 0, i32 1, i32 0
  store %"class.std::_Sp_counted_base"* null, %"class.std::_Sp_counted_base"** %107, align 8, !tbaa !129
  %108 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %78, i64 3, i32 0, i32 0, i32 0, i32 0, i32 1
  %109 = bitcast %"class.std::__shared_count"* %108 to i64*
  %110 = bitcast %"class.MARC::TaskFuture"* %104 to <2 x i64>*
  %111 = load <2 x i64>, <2 x i64>* %110, align 8, !tbaa !27
  store i64 0, i64* %109, align 8, !tbaa !129
  %112 = bitcast %"class.MARC::TaskFuture"* %105 to <2 x i64>*
  store <2 x i64> %111, <2 x i64>* %112, align 8, !tbaa !27
  store %"class.std::__future_base::_State_baseV2"* null, %"class.std::__future_base::_State_baseV2"** %106, align 8, !tbaa !131
  %113 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %78, i64 4
  %114 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %77, i64 4
  %115 = icmp eq %"class.MARC::TaskFuture"* %113, %32
  br i1 %115, label %116, label %76

; <label>:116:                                    ; preds = %76, %71
  %117 = ptrtoint %"class.std::__future_base::_State_baseV2"** %45 to i64
  %118 = sub i64 %117, %29
  %119 = lshr i64 %118, 4
  %120 = add nuw nsw i64 %119, 1
  %121 = getelementptr %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %30, i64 %120
  br label %122

; <label>:122:                                    ; preds = %165, %116
  %123 = phi %"class.MARC::TaskFuture"* [ %31, %116 ], [ %166, %165 ]
  %124 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %123, i64 0, i32 0, i32 0, i32 0, i32 0, i32 0
  %125 = load %"class.std::__future_base::_State_baseV2"*, %"class.std::__future_base::_State_baseV2"** %124, align 8, !tbaa !131
  %126 = icmp eq %"class.std::__future_base::_State_baseV2"* %125, null
  br i1 %126, label %133, label %127

; <label>:127:                                    ; preds = %122
  %128 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %123, i64 0, i32 0
  invoke void @_ZNSt6futureIvE3getEv(%"class.std::future"* %128)
          to label %133 unwind label %129

; <label>:129:                                    ; preds = %127
  %130 = landingpad { i8*, i32 }
          catch i8* null
  %131 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %123, i64 0, i32 0, i32 0
  %132 = extractvalue { i8*, i32 } %130, 0
  tail call void @_ZNSt14__basic_futureIvED2Ev(%"class.std::__basic_future"* %131) #8
  tail call void @__clang_call_terminate(i8* %132) #22
  unreachable

; <label>:133:                                    ; preds = %127, %122
  %134 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %123, i64 0, i32 0, i32 0, i32 0, i32 0, i32 1, i32 0
  %135 = load %"class.std::_Sp_counted_base"*, %"class.std::_Sp_counted_base"** %134, align 8, !tbaa !129
  %136 = icmp eq %"class.std::_Sp_counted_base"* %135, null
  br i1 %136, label %165, label %137

; <label>:137:                                    ; preds = %133
  %138 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %135, i64 0, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %139, label %141

; <label>:139:                                    ; preds = %137
  %140 = atomicrmw volatile add i32* %138, i32 -1 acq_rel
  br label %144

; <label>:141:                                    ; preds = %137
  %142 = load i32, i32* %138, align 4, !tbaa !17
  %143 = add nsw i32 %142, -1
  store i32 %143, i32* %138, align 4, !tbaa !17
  br label %144

; <label>:144:                                    ; preds = %141, %139
  %145 = phi i32 [ %140, %139 ], [ %142, %141 ]
  %146 = icmp eq i32 %145, 1
  br i1 %146, label %147, label %165

; <label>:147:                                    ; preds = %144
  %148 = bitcast %"class.std::_Sp_counted_base"* %135 to void (%"class.std::_Sp_counted_base"*)***
  %149 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %148, align 8, !tbaa !133
  %150 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %149, i64 2
  %151 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %150, align 8
  tail call void %151(%"class.std::_Sp_counted_base"* nonnull %135) #8
  %152 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %135, i64 0, i32 2
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %153, label %155

; <label>:153:                                    ; preds = %147
  %154 = atomicrmw volatile add i32* %152, i32 -1 acq_rel
  br label %158

; <label>:155:                                    ; preds = %147
  %156 = load i32, i32* %152, align 4, !tbaa !17
  %157 = add nsw i32 %156, -1
  store i32 %157, i32* %152, align 4, !tbaa !17
  br label %158

; <label>:158:                                    ; preds = %155, %153
  %159 = phi i32 [ %154, %153 ], [ %156, %155 ]
  %160 = icmp eq i32 %159, 1
  br i1 %160, label %161, label %165

; <label>:161:                                    ; preds = %158
  %162 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %148, align 8, !tbaa !133
  %163 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %162, i64 3
  %164 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %163, align 8
  tail call void %164(%"class.std::_Sp_counted_base"* nonnull %135) #8
  br label %165

; <label>:165:                                    ; preds = %161, %158, %144, %133
  %166 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %123, i64 1
  %167 = icmp eq %"class.MARC::TaskFuture"* %166, %32
  br i1 %167, label %168, label %122

; <label>:168:                                    ; preds = %165
  %169 = load %"class.MARC::TaskFuture"*, %"class.MARC::TaskFuture"** %42, align 8, !tbaa !205
  br label %170

; <label>:170:                                    ; preds = %27, %168
  %171 = phi %"class.MARC::TaskFuture"* [ %169, %168 ], [ %32, %27 ]
  %172 = phi %"class.MARC::TaskFuture"* [ %121, %168 ], [ %30, %27 ]
  %173 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %172, i64 1
  %174 = getelementptr inbounds %"class.std::vector.53", %"class.std::vector.53"* %0, i64 0, i32 0, i32 0, i32 2
  %175 = icmp eq %"class.MARC::TaskFuture"* %171, null
  br i1 %175, label %178, label %176

; <label>:176:                                    ; preds = %170
  %177 = bitcast %"class.MARC::TaskFuture"* %171 to i8*
  tail call void @_ZdlPv(i8* %177) #8
  br label %178

; <label>:178:                                    ; preds = %170, %176
  store %"class.MARC::TaskFuture"* %30, %"class.MARC::TaskFuture"** %42, align 8, !tbaa !205
  store %"class.MARC::TaskFuture"* %173, %"class.MARC::TaskFuture"** %3, align 8, !tbaa !125
  %179 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %30, i64 %16
  store %"class.MARC::TaskFuture"* %179, %"class.MARC::TaskFuture"** %174, align 8, !tbaa !128
  ret void
}

; Function Attrs: uwtable
define linkonce_odr void @_ZNSt6futureIvE3getEv(%"class.std::future"*) local_unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %2 = getelementptr inbounds %"class.std::future", %"class.std::future"* %0, i64 0, i32 0
  %3 = invoke dereferenceable(16) %"struct.std::__future_base::_Result"* @_ZNKSt14__basic_futureIvE13_M_get_resultEv(%"class.std::__basic_future"* nonnull %2)
          to label %4 unwind label %38

; <label>:4:                                      ; preds = %1
  %5 = getelementptr inbounds %"class.std::future", %"class.std::future"* %0, i64 0, i32 0, i32 0, i32 0, i32 1, i32 0
  %6 = load %"class.std::_Sp_counted_base"*, %"class.std::_Sp_counted_base"** %5, align 8, !tbaa !129
  %7 = bitcast %"class.std::future"* %0 to <2 x i64>*
  store <2 x i64> zeroinitializer, <2 x i64>* %7, align 8, !tbaa !27
  %8 = icmp eq %"class.std::_Sp_counted_base"* %6, null
  br i1 %8, label %37, label %9

; <label>:9:                                      ; preds = %4
  %10 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %6, i64 0, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %11, label %13

; <label>:11:                                     ; preds = %9
  %12 = atomicrmw volatile add i32* %10, i32 -1 acq_rel
  br label %16

; <label>:13:                                     ; preds = %9
  %14 = load i32, i32* %10, align 4, !tbaa !17
  %15 = add nsw i32 %14, -1
  store i32 %15, i32* %10, align 4, !tbaa !17
  br label %16

; <label>:16:                                     ; preds = %13, %11
  %17 = phi i32 [ %12, %11 ], [ %14, %13 ]
  %18 = icmp eq i32 %17, 1
  br i1 %18, label %19, label %37

; <label>:19:                                     ; preds = %16
  %20 = bitcast %"class.std::_Sp_counted_base"* %6 to void (%"class.std::_Sp_counted_base"*)***
  %21 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %20, align 8, !tbaa !133
  %22 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %21, i64 2
  %23 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %22, align 8
  tail call void %23(%"class.std::_Sp_counted_base"* nonnull %6) #8
  %24 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %6, i64 0, i32 2
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %25, label %27

; <label>:25:                                     ; preds = %19
  %26 = atomicrmw volatile add i32* %24, i32 -1 acq_rel
  br label %30

; <label>:27:                                     ; preds = %19
  %28 = load i32, i32* %24, align 4, !tbaa !17
  %29 = add nsw i32 %28, -1
  store i32 %29, i32* %24, align 4, !tbaa !17
  br label %30

; <label>:30:                                     ; preds = %27, %25
  %31 = phi i32 [ %26, %25 ], [ %28, %27 ]
  %32 = icmp eq i32 %31, 1
  br i1 %32, label %33, label %37

; <label>:33:                                     ; preds = %30
  %34 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %20, align 8, !tbaa !133
  %35 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %34, i64 3
  %36 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %35, align 8
  tail call void %36(%"class.std::_Sp_counted_base"* nonnull %6) #8
  br label %37

; <label>:37:                                     ; preds = %4, %16, %30, %33
  ret void

; <label>:38:                                     ; preds = %1
  %39 = landingpad { i8*, i32 }
          cleanup
  %40 = getelementptr inbounds %"class.std::future", %"class.std::future"* %0, i64 0, i32 0, i32 0, i32 0, i32 1, i32 0
  %41 = load %"class.std::_Sp_counted_base"*, %"class.std::_Sp_counted_base"** %40, align 8, !tbaa !129
  %42 = bitcast %"class.std::future"* %0 to <2 x i64>*
  store <2 x i64> zeroinitializer, <2 x i64>* %42, align 8, !tbaa !27
  %43 = icmp eq %"class.std::_Sp_counted_base"* %41, null
  br i1 %43, label %72, label %44

; <label>:44:                                     ; preds = %38
  %45 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %41, i64 0, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %46, label %48

; <label>:46:                                     ; preds = %44
  %47 = atomicrmw volatile add i32* %45, i32 -1 acq_rel
  br label %51

; <label>:48:                                     ; preds = %44
  %49 = load i32, i32* %45, align 4, !tbaa !17
  %50 = add nsw i32 %49, -1
  store i32 %50, i32* %45, align 4, !tbaa !17
  br label %51

; <label>:51:                                     ; preds = %48, %46
  %52 = phi i32 [ %47, %46 ], [ %49, %48 ]
  %53 = icmp eq i32 %52, 1
  br i1 %53, label %54, label %72

; <label>:54:                                     ; preds = %51
  %55 = bitcast %"class.std::_Sp_counted_base"* %41 to void (%"class.std::_Sp_counted_base"*)***
  %56 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %55, align 8, !tbaa !133
  %57 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %56, i64 2
  %58 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %57, align 8
  tail call void %58(%"class.std::_Sp_counted_base"* nonnull %41) #8
  %59 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %41, i64 0, i32 2
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %60, label %62

; <label>:60:                                     ; preds = %54
  %61 = atomicrmw volatile add i32* %59, i32 -1 acq_rel
  br label %65

; <label>:62:                                     ; preds = %54
  %63 = load i32, i32* %59, align 4, !tbaa !17
  %64 = add nsw i32 %63, -1
  store i32 %64, i32* %59, align 4, !tbaa !17
  br label %65

; <label>:65:                                     ; preds = %62, %60
  %66 = phi i32 [ %61, %60 ], [ %63, %62 ]
  %67 = icmp eq i32 %66, 1
  br i1 %67, label %68, label %72

; <label>:68:                                     ; preds = %65
  %69 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %55, align 8, !tbaa !133
  %70 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %69, i64 3
  %71 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %70, align 8
  tail call void %71(%"class.std::_Sp_counted_base"* nonnull %41) #8
  br label %72

; <label>:72:                                     ; preds = %38, %51, %65, %68
  resume { i8*, i32 } %39
}

; Function Attrs: inlinehint nounwind uwtable
define linkonce_odr void @_ZNSt14__basic_futureIvED2Ev(%"class.std::__basic_future"*) unnamed_addr #17 comdat align 2 personality i32 (...)* @__gxx_personality_v0 {
  %2 = getelementptr inbounds %"class.std::__basic_future", %"class.std::__basic_future"* %0, i64 0, i32 0, i32 0, i32 1, i32 0
  %3 = load %"class.std::_Sp_counted_base"*, %"class.std::_Sp_counted_base"** %2, align 8, !tbaa !129
  %4 = icmp eq %"class.std::_Sp_counted_base"* %3, null
  br i1 %4, label %33, label %5

; <label>:5:                                      ; preds = %1
  %6 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %3, i64 0, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %7, label %9

; <label>:7:                                      ; preds = %5
  %8 = atomicrmw volatile add i32* %6, i32 -1 acq_rel
  br label %12

; <label>:9:                                      ; preds = %5
  %10 = load i32, i32* %6, align 4, !tbaa !17
  %11 = add nsw i32 %10, -1
  store i32 %11, i32* %6, align 4, !tbaa !17
  br label %12

; <label>:12:                                     ; preds = %9, %7
  %13 = phi i32 [ %8, %7 ], [ %10, %9 ]
  %14 = icmp eq i32 %13, 1
  br i1 %14, label %15, label %33

; <label>:15:                                     ; preds = %12
  %16 = bitcast %"class.std::_Sp_counted_base"* %3 to void (%"class.std::_Sp_counted_base"*)***
  %17 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %16, align 8, !tbaa !133
  %18 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %17, i64 2
  %19 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %18, align 8
  tail call void %19(%"class.std::_Sp_counted_base"* nonnull %3) #8
  %20 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %3, i64 0, i32 2
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %21, label %23

; <label>:21:                                     ; preds = %15
  %22 = atomicrmw volatile add i32* %20, i32 -1 acq_rel
  br label %26

; <label>:23:                                     ; preds = %15
  %24 = load i32, i32* %20, align 4, !tbaa !17
  %25 = add nsw i32 %24, -1
  store i32 %25, i32* %20, align 4, !tbaa !17
  br label %26

; <label>:26:                                     ; preds = %23, %21
  %27 = phi i32 [ %22, %21 ], [ %24, %23 ]
  %28 = icmp eq i32 %27, 1
  br i1 %28, label %29, label %33

; <label>:29:                                     ; preds = %26
  %30 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %16, align 8, !tbaa !133
  %31 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %30, i64 3
  %32 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %31, align 8
  tail call void %32(%"class.std::_Sp_counted_base"* nonnull %3) #8
  br label %33

; <label>:33:                                     ; preds = %1, %12, %26, %29
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZNSt6vectorIN4MARC10TaskFutureIvEESaIS2_EED2Ev(%"class.std::vector.53"*) unnamed_addr #10 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %2 = getelementptr inbounds %"class.std::vector.53", %"class.std::vector.53"* %0, i64 0, i32 0, i32 0, i32 0
  %3 = load %"class.MARC::TaskFuture"*, %"class.MARC::TaskFuture"** %2, align 8, !tbaa !205
  %4 = getelementptr inbounds %"class.std::vector.53", %"class.std::vector.53"* %0, i64 0, i32 0, i32 0, i32 1
  %5 = load %"class.MARC::TaskFuture"*, %"class.MARC::TaskFuture"** %4, align 8, !tbaa !125
  %6 = icmp eq %"class.MARC::TaskFuture"* %3, %5
  br i1 %6, label %56, label %7

; <label>:7:                                      ; preds = %1
  br label %8

; <label>:8:                                      ; preds = %7, %51
  %9 = phi %"class.MARC::TaskFuture"* [ %52, %51 ], [ %3, %7 ]
  %10 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %9, i64 0, i32 0, i32 0, i32 0, i32 0, i32 0
  %11 = load %"class.std::__future_base::_State_baseV2"*, %"class.std::__future_base::_State_baseV2"** %10, align 8, !tbaa !131
  %12 = icmp eq %"class.std::__future_base::_State_baseV2"* %11, null
  br i1 %12, label %19, label %13

; <label>:13:                                     ; preds = %8
  %14 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %9, i64 0, i32 0
  invoke void @_ZNSt6futureIvE3getEv(%"class.std::future"* %14)
          to label %19 unwind label %15

; <label>:15:                                     ; preds = %13
  %16 = landingpad { i8*, i32 }
          catch i8* null
  %17 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %9, i64 0, i32 0, i32 0
  %18 = extractvalue { i8*, i32 } %16, 0
  tail call void @_ZNSt14__basic_futureIvED2Ev(%"class.std::__basic_future"* %17) #8
  tail call void @__clang_call_terminate(i8* %18) #22
  unreachable

; <label>:19:                                     ; preds = %13, %8
  %20 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %9, i64 0, i32 0, i32 0, i32 0, i32 0, i32 1, i32 0
  %21 = load %"class.std::_Sp_counted_base"*, %"class.std::_Sp_counted_base"** %20, align 8, !tbaa !129
  %22 = icmp eq %"class.std::_Sp_counted_base"* %21, null
  br i1 %22, label %51, label %23

; <label>:23:                                     ; preds = %19
  %24 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %21, i64 0, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %25, label %27

; <label>:25:                                     ; preds = %23
  %26 = atomicrmw volatile add i32* %24, i32 -1 acq_rel
  br label %30

; <label>:27:                                     ; preds = %23
  %28 = load i32, i32* %24, align 4, !tbaa !17
  %29 = add nsw i32 %28, -1
  store i32 %29, i32* %24, align 4, !tbaa !17
  br label %30

; <label>:30:                                     ; preds = %27, %25
  %31 = phi i32 [ %26, %25 ], [ %28, %27 ]
  %32 = icmp eq i32 %31, 1
  br i1 %32, label %33, label %51

; <label>:33:                                     ; preds = %30
  %34 = bitcast %"class.std::_Sp_counted_base"* %21 to void (%"class.std::_Sp_counted_base"*)***
  %35 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %34, align 8, !tbaa !133
  %36 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %35, i64 2
  %37 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %36, align 8
  tail call void %37(%"class.std::_Sp_counted_base"* nonnull %21) #8
  %38 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %21, i64 0, i32 2
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %39, label %41

; <label>:39:                                     ; preds = %33
  %40 = atomicrmw volatile add i32* %38, i32 -1 acq_rel
  br label %44

; <label>:41:                                     ; preds = %33
  %42 = load i32, i32* %38, align 4, !tbaa !17
  %43 = add nsw i32 %42, -1
  store i32 %43, i32* %38, align 4, !tbaa !17
  br label %44

; <label>:44:                                     ; preds = %41, %39
  %45 = phi i32 [ %40, %39 ], [ %42, %41 ]
  %46 = icmp eq i32 %45, 1
  br i1 %46, label %47, label %51

; <label>:47:                                     ; preds = %44
  %48 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %34, align 8, !tbaa !133
  %49 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %48, i64 3
  %50 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %49, align 8
  tail call void %50(%"class.std::_Sp_counted_base"* nonnull %21) #8
  br label %51

; <label>:51:                                     ; preds = %47, %44, %30, %19
  %52 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %9, i64 1
  %53 = icmp eq %"class.MARC::TaskFuture"* %52, %5
  br i1 %53, label %54, label %8

; <label>:54:                                     ; preds = %51
  %55 = load %"class.MARC::TaskFuture"*, %"class.MARC::TaskFuture"** %2, align 8, !tbaa !205
  br label %56

; <label>:56:                                     ; preds = %54, %1
  %57 = phi %"class.MARC::TaskFuture"* [ %55, %54 ], [ %3, %1 ]
  %58 = icmp eq %"class.MARC::TaskFuture"* %57, null
  br i1 %58, label %61, label %59

; <label>:59:                                     ; preds = %56
  %60 = bitcast %"class.MARC::TaskFuture"* %57 to i8*
  tail call void @_ZdlPv(i8* %60) #8
  br label %61

; <label>:61:                                     ; preds = %56, %59
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN4MARC10ThreadPoolD2Ev(%"class.MARC::ThreadPool"*) unnamed_addr #10 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  invoke void @_ZN4MARC10ThreadPool7destroyEv(%"class.MARC::ThreadPool"* %0)
          to label %2 unwind label %25

; <label>:2:                                      ; preds = %1
  %3 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 4
  tail call void @_ZN4MARC15ThreadSafeQueueISt8functionIFvvEEED2Ev(%"class.MARC::ThreadSafeQueue.45"* %3) #8
  %4 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 2, i32 0, i32 0, i32 0
  %5 = load %"class.std::thread"*, %"class.std::thread"** %4, align 8, !tbaa !151
  %6 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 2, i32 0, i32 0, i32 1
  %7 = load %"class.std::thread"*, %"class.std::thread"** %6, align 8, !tbaa !154
  %8 = icmp eq %"class.std::thread"* %5, %7
  br i1 %8, label %19, label %9

; <label>:9:                                      ; preds = %2
  br label %12

; <label>:10:                                     ; preds = %12
  %11 = icmp eq %"class.std::thread"* %17, %7
  br i1 %11, label %19, label %12

; <label>:12:                                     ; preds = %9, %10
  %13 = phi %"class.std::thread"* [ %17, %10 ], [ %5, %9 ]
  %14 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %13, i64 0, i32 0, i32 0
  %15 = load i64, i64* %14, align 8
  %16 = icmp eq i64 %15, 0
  %17 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %13, i64 1
  br i1 %16, label %10, label %18

; <label>:18:                                     ; preds = %12
  tail call void @_ZSt9terminatev() #22
  unreachable

; <label>:19:                                     ; preds = %10, %2
  %20 = icmp eq %"class.std::thread"* %5, null
  br i1 %20, label %23, label %21

; <label>:21:                                     ; preds = %19
  %22 = bitcast %"class.std::thread"* %5 to i8*
  tail call void @_ZdlPv(i8* %22) #8
  br label %23

; <label>:23:                                     ; preds = %19, %21
  %24 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 1
  tail call void @_ZN4MARC15ThreadSafeQueueISt10unique_ptrINS_11IThreadTaskESt14default_deleteIS2_EEED2Ev(%"class.MARC::ThreadSafeQueue.32"* %24) #8
  ret void

; <label>:25:                                     ; preds = %1
  %26 = landingpad { i8*, i32 }
          catch i8* null
  %27 = extractvalue { i8*, i32 } %26, 0
  %28 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 4
  tail call void @_ZN4MARC15ThreadSafeQueueISt8functionIFvvEEED2Ev(%"class.MARC::ThreadSafeQueue.45"* %28) #8
  %29 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 2
  tail call void @_ZNSt6vectorISt6threadSaIS0_EED2Ev(%"class.std::vector"* %29) #8
  %30 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 1
  tail call void @_ZN4MARC15ThreadSafeQueueISt10unique_ptrINS_11IThreadTaskESt14default_deleteIS2_EEED2Ev(%"class.MARC::ThreadSafeQueue.32"* %30) #8
  tail call void @__clang_call_terminate(i8* %27) #22
  unreachable
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN4MARC15ThreadSafeQueueIaED2Ev(%"class.MARC::ThreadSafeQueue"*) unnamed_addr #10 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %2 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %3, label %9

; <label>:3:                                      ; preds = %1
  %4 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %2, i64 0, i32 0, i32 0
  %5 = tail call i32 @pthread_mutex_lock(%union.pthread_mutex_t* nonnull %4) #8
  %6 = icmp eq i32 %5, 0
  br i1 %6, label %9, label %7

; <label>:7:                                      ; preds = %3
  invoke void @_ZSt20__throw_system_errori(i32 %5) #23
          to label %8 unwind label %48

; <label>:8:                                      ; preds = %7
  unreachable

; <label>:9:                                      ; preds = %3, %1
  %10 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 0, i32 0, i32 0
  %11 = load atomic i8, i8* %10 seq_cst, align 1
  %12 = and i8 %11, 1
  %13 = icmp eq i8 %12, 0
  br i1 %13, label %17, label %14

; <label>:14:                                     ; preds = %9
  store atomic i8 0, i8* %10 seq_cst, align 1
  %15 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 3
  tail call void @_ZNSt18condition_variable10notify_allEv(%"class.std::condition_variable"* %15) #8
  %16 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 4
  tail call void @_ZNSt18condition_variable10notify_allEv(%"class.std::condition_variable"* %16) #8
  br label %17

; <label>:17:                                     ; preds = %14, %9
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %18, label %21

; <label>:18:                                     ; preds = %17
  %19 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %2, i64 0, i32 0, i32 0
  %20 = tail call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %19) #8
  br label %21

; <label>:21:                                     ; preds = %18, %17
  %22 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 4
  tail call void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"* %22) #8
  %23 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 3
  tail call void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"* %23) #8
  %24 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2
  %25 = getelementptr inbounds %"class.std::queue", %"class.std::queue"* %24, i64 0, i32 0, i32 0, i32 0, i32 0
  %26 = load i8**, i8*** %25, align 8, !tbaa !26
  %27 = icmp eq i8** %26, null
  br i1 %27, label %47, label %28

; <label>:28:                                     ; preds = %21
  %29 = bitcast i8** %26 to i8*
  %30 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 3
  %31 = load i8**, i8*** %30, align 8, !tbaa !31
  %32 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 3
  %33 = load i8**, i8*** %32, align 8, !tbaa !25
  %34 = getelementptr inbounds i8*, i8** %33, i64 1
  %35 = icmp ult i8** %31, %34
  br i1 %35, label %36, label %45

; <label>:36:                                     ; preds = %28
  br label %37

; <label>:37:                                     ; preds = %36, %37
  %38 = phi i8** [ %40, %37 ], [ %31, %36 ]
  %39 = load i8*, i8** %38, align 8, !tbaa !27
  tail call void @_ZdlPv(i8* %39) #8
  %40 = getelementptr inbounds i8*, i8** %38, i64 1
  %41 = icmp ult i8** %38, %33
  br i1 %41, label %37, label %42

; <label>:42:                                     ; preds = %37
  %43 = bitcast %"class.std::queue"* %24 to i8**
  %44 = load i8*, i8** %43, align 8, !tbaa !26
  br label %45

; <label>:45:                                     ; preds = %42, %28
  %46 = phi i8* [ %44, %42 ], [ %29, %28 ]
  tail call void @_ZdlPv(i8* %46) #8
  br label %47

; <label>:47:                                     ; preds = %21, %45
  ret void

; <label>:48:                                     ; preds = %7
  %49 = landingpad { i8*, i32 }
          catch i8* null
  %50 = extractvalue { i8*, i32 } %49, 0
  %51 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 4
  tail call void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"* %51) #8
  %52 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 3
  tail call void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"* %52) #8
  %53 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2
  tail call void @_ZNSt5queueIaSt5dequeIaSaIaEEED2Ev(%"class.std::queue"* %53) #8
  tail call void @__clang_call_terminate(i8* %50) #22
  unreachable
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN4MARC15ThreadSafeQueueIsED2Ev(%"class.MARC::ThreadSafeQueue.3"*) unnamed_addr #10 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %2 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %3, label %9

; <label>:3:                                      ; preds = %1
  %4 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %2, i64 0, i32 0, i32 0
  %5 = tail call i32 @pthread_mutex_lock(%union.pthread_mutex_t* nonnull %4) #8
  %6 = icmp eq i32 %5, 0
  br i1 %6, label %9, label %7

; <label>:7:                                      ; preds = %3
  invoke void @_ZSt20__throw_system_errori(i32 %5) #23
          to label %8 unwind label %49

; <label>:8:                                      ; preds = %7
  unreachable

; <label>:9:                                      ; preds = %3, %1
  %10 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 0, i32 0, i32 0
  %11 = load atomic i8, i8* %10 seq_cst, align 1
  %12 = and i8 %11, 1
  %13 = icmp eq i8 %12, 0
  br i1 %13, label %17, label %14

; <label>:14:                                     ; preds = %9
  store atomic i8 0, i8* %10 seq_cst, align 1
  %15 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 3
  tail call void @_ZNSt18condition_variable10notify_allEv(%"class.std::condition_variable"* %15) #8
  %16 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 4
  tail call void @_ZNSt18condition_variable10notify_allEv(%"class.std::condition_variable"* %16) #8
  br label %17

; <label>:17:                                     ; preds = %14, %9
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %18, label %21

; <label>:18:                                     ; preds = %17
  %19 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %2, i64 0, i32 0, i32 0
  %20 = tail call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %19) #8
  br label %21

; <label>:21:                                     ; preds = %18, %17
  %22 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 4
  tail call void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"* %22) #8
  %23 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 3
  tail call void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"* %23) #8
  %24 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 2
  %25 = getelementptr inbounds %"class.std::queue.4", %"class.std::queue.4"* %24, i64 0, i32 0, i32 0, i32 0, i32 0
  %26 = load i16**, i16*** %25, align 8, !tbaa !53
  %27 = icmp eq i16** %26, null
  br i1 %27, label %48, label %28

; <label>:28:                                     ; preds = %21
  %29 = bitcast i16** %26 to i8*
  %30 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 3
  %31 = load i16**, i16*** %30, align 8, !tbaa !57
  %32 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 3
  %33 = load i16**, i16*** %32, align 8, !tbaa !52
  %34 = getelementptr inbounds i16*, i16** %33, i64 1
  %35 = icmp ult i16** %31, %34
  br i1 %35, label %36, label %46

; <label>:36:                                     ; preds = %28
  br label %37

; <label>:37:                                     ; preds = %36, %37
  %38 = phi i16** [ %41, %37 ], [ %31, %36 ]
  %39 = bitcast i16** %38 to i8**
  %40 = load i8*, i8** %39, align 8, !tbaa !27
  tail call void @_ZdlPv(i8* %40) #8
  %41 = getelementptr inbounds i16*, i16** %38, i64 1
  %42 = icmp ult i16** %38, %33
  br i1 %42, label %37, label %43

; <label>:43:                                     ; preds = %37
  %44 = bitcast %"class.std::queue.4"* %24 to i8**
  %45 = load i8*, i8** %44, align 8, !tbaa !53
  br label %46

; <label>:46:                                     ; preds = %43, %28
  %47 = phi i8* [ %45, %43 ], [ %29, %28 ]
  tail call void @_ZdlPv(i8* %47) #8
  br label %48

; <label>:48:                                     ; preds = %21, %46
  ret void

; <label>:49:                                     ; preds = %7
  %50 = landingpad { i8*, i32 }
          catch i8* null
  %51 = extractvalue { i8*, i32 } %50, 0
  %52 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 4
  tail call void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"* %52) #8
  %53 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 3
  tail call void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"* %53) #8
  %54 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 2
  tail call void @_ZNSt5queueIsSt5dequeIsSaIsEEED2Ev(%"class.std::queue.4"* %54) #8
  tail call void @__clang_call_terminate(i8* %51) #22
  unreachable
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN4MARC15ThreadSafeQueueIiED2Ev(%"class.MARC::ThreadSafeQueue.11"*) unnamed_addr #10 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %2 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %3, label %9

; <label>:3:                                      ; preds = %1
  %4 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %2, i64 0, i32 0, i32 0
  %5 = tail call i32 @pthread_mutex_lock(%union.pthread_mutex_t* nonnull %4) #8
  %6 = icmp eq i32 %5, 0
  br i1 %6, label %9, label %7

; <label>:7:                                      ; preds = %3
  invoke void @_ZSt20__throw_system_errori(i32 %5) #23
          to label %8 unwind label %49

; <label>:8:                                      ; preds = %7
  unreachable

; <label>:9:                                      ; preds = %3, %1
  %10 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 0, i32 0, i32 0
  %11 = load atomic i8, i8* %10 seq_cst, align 1
  %12 = and i8 %11, 1
  %13 = icmp eq i8 %12, 0
  br i1 %13, label %17, label %14

; <label>:14:                                     ; preds = %9
  store atomic i8 0, i8* %10 seq_cst, align 1
  %15 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 3
  tail call void @_ZNSt18condition_variable10notify_allEv(%"class.std::condition_variable"* %15) #8
  %16 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 4
  tail call void @_ZNSt18condition_variable10notify_allEv(%"class.std::condition_variable"* %16) #8
  br label %17

; <label>:17:                                     ; preds = %14, %9
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %18, label %21

; <label>:18:                                     ; preds = %17
  %19 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %2, i64 0, i32 0, i32 0
  %20 = tail call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %19) #8
  br label %21

; <label>:21:                                     ; preds = %18, %17
  %22 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 4
  tail call void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"* %22) #8
  %23 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 3
  tail call void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"* %23) #8
  %24 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 2
  %25 = getelementptr inbounds %"class.std::queue.12", %"class.std::queue.12"* %24, i64 0, i32 0, i32 0, i32 0, i32 0
  %26 = load i32**, i32*** %25, align 8, !tbaa !72
  %27 = icmp eq i32** %26, null
  br i1 %27, label %48, label %28

; <label>:28:                                     ; preds = %21
  %29 = bitcast i32** %26 to i8*
  %30 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 3
  %31 = load i32**, i32*** %30, align 8, !tbaa !76
  %32 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 3
  %33 = load i32**, i32*** %32, align 8, !tbaa !71
  %34 = getelementptr inbounds i32*, i32** %33, i64 1
  %35 = icmp ult i32** %31, %34
  br i1 %35, label %36, label %46

; <label>:36:                                     ; preds = %28
  br label %37

; <label>:37:                                     ; preds = %36, %37
  %38 = phi i32** [ %41, %37 ], [ %31, %36 ]
  %39 = bitcast i32** %38 to i8**
  %40 = load i8*, i8** %39, align 8, !tbaa !27
  tail call void @_ZdlPv(i8* %40) #8
  %41 = getelementptr inbounds i32*, i32** %38, i64 1
  %42 = icmp ult i32** %38, %33
  br i1 %42, label %37, label %43

; <label>:43:                                     ; preds = %37
  %44 = bitcast %"class.std::queue.12"* %24 to i8**
  %45 = load i8*, i8** %44, align 8, !tbaa !72
  br label %46

; <label>:46:                                     ; preds = %43, %28
  %47 = phi i8* [ %45, %43 ], [ %29, %28 ]
  tail call void @_ZdlPv(i8* %47) #8
  br label %48

; <label>:48:                                     ; preds = %21, %46
  ret void

; <label>:49:                                     ; preds = %7
  %50 = landingpad { i8*, i32 }
          catch i8* null
  %51 = extractvalue { i8*, i32 } %50, 0
  %52 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 4
  tail call void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"* %52) #8
  %53 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 3
  tail call void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"* %53) #8
  %54 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 2
  tail call void @_ZNSt5queueIiSt5dequeIiSaIiEEED2Ev(%"class.std::queue.12"* %54) #8
  tail call void @__clang_call_terminate(i8* %51) #22
  unreachable
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN4MARC15ThreadSafeQueueIlED2Ev(%"class.MARC::ThreadSafeQueue.19"*) unnamed_addr #10 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %2 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %3, label %9

; <label>:3:                                      ; preds = %1
  %4 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %2, i64 0, i32 0, i32 0
  %5 = tail call i32 @pthread_mutex_lock(%union.pthread_mutex_t* nonnull %4) #8
  %6 = icmp eq i32 %5, 0
  br i1 %6, label %9, label %7

; <label>:7:                                      ; preds = %3
  invoke void @_ZSt20__throw_system_errori(i32 %5) #23
          to label %8 unwind label %49

; <label>:8:                                      ; preds = %7
  unreachable

; <label>:9:                                      ; preds = %3, %1
  %10 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 0, i32 0, i32 0
  %11 = load atomic i8, i8* %10 seq_cst, align 1
  %12 = and i8 %11, 1
  %13 = icmp eq i8 %12, 0
  br i1 %13, label %17, label %14

; <label>:14:                                     ; preds = %9
  store atomic i8 0, i8* %10 seq_cst, align 1
  %15 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 3
  tail call void @_ZNSt18condition_variable10notify_allEv(%"class.std::condition_variable"* %15) #8
  %16 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 4
  tail call void @_ZNSt18condition_variable10notify_allEv(%"class.std::condition_variable"* %16) #8
  br label %17

; <label>:17:                                     ; preds = %14, %9
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %18, label %21

; <label>:18:                                     ; preds = %17
  %19 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %2, i64 0, i32 0, i32 0
  %20 = tail call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %19) #8
  br label %21

; <label>:21:                                     ; preds = %18, %17
  %22 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 4
  tail call void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"* %22) #8
  %23 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 3
  tail call void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"* %23) #8
  %24 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 2
  %25 = getelementptr inbounds %"class.std::queue.20", %"class.std::queue.20"* %24, i64 0, i32 0, i32 0, i32 0, i32 0
  %26 = load i64**, i64*** %25, align 8, !tbaa !92
  %27 = icmp eq i64** %26, null
  br i1 %27, label %48, label %28

; <label>:28:                                     ; preds = %21
  %29 = bitcast i64** %26 to i8*
  %30 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 3
  %31 = load i64**, i64*** %30, align 8, !tbaa !96
  %32 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 3
  %33 = load i64**, i64*** %32, align 8, !tbaa !91
  %34 = getelementptr inbounds i64*, i64** %33, i64 1
  %35 = icmp ult i64** %31, %34
  br i1 %35, label %36, label %46

; <label>:36:                                     ; preds = %28
  br label %37

; <label>:37:                                     ; preds = %36, %37
  %38 = phi i64** [ %41, %37 ], [ %31, %36 ]
  %39 = bitcast i64** %38 to i8**
  %40 = load i8*, i8** %39, align 8, !tbaa !27
  tail call void @_ZdlPv(i8* %40) #8
  %41 = getelementptr inbounds i64*, i64** %38, i64 1
  %42 = icmp ult i64** %38, %33
  br i1 %42, label %37, label %43

; <label>:43:                                     ; preds = %37
  %44 = bitcast %"class.std::queue.20"* %24 to i8**
  %45 = load i8*, i8** %44, align 8, !tbaa !92
  br label %46

; <label>:46:                                     ; preds = %43, %28
  %47 = phi i8* [ %45, %43 ], [ %29, %28 ]
  tail call void @_ZdlPv(i8* %47) #8
  br label %48

; <label>:48:                                     ; preds = %21, %46
  ret void

; <label>:49:                                     ; preds = %7
  %50 = landingpad { i8*, i32 }
          catch i8* null
  %51 = extractvalue { i8*, i32 } %50, 0
  %52 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 4
  tail call void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"* %52) #8
  %53 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 3
  tail call void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"* %53) #8
  %54 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 2
  tail call void @_ZNSt5queueIlSt5dequeIlSaIlEEED2Ev(%"class.std::queue.20"* %54) #8
  tail call void @__clang_call_terminate(i8* %51) #22
  unreachable
}

; Function Attrs: nounwind
declare void @_ZNSt18condition_variable10notify_allEv(%"class.std::condition_variable"*) local_unnamed_addr #7

; Function Attrs: nounwind
declare void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"*) unnamed_addr #7

; Function Attrs: inlinehint nounwind uwtable
define linkonce_odr void @_ZNSt5queueIlSt5dequeIlSaIlEEED2Ev(%"class.std::queue.20"*) unnamed_addr #17 comdat align 2 personality i32 (...)* @__gxx_personality_v0 {
  %2 = getelementptr inbounds %"class.std::queue.20", %"class.std::queue.20"* %0, i64 0, i32 0, i32 0, i32 0, i32 0
  %3 = load i64**, i64*** %2, align 8, !tbaa !92
  %4 = icmp eq i64** %3, null
  br i1 %4, label %25, label %5

; <label>:5:                                      ; preds = %1
  %6 = bitcast i64** %3 to i8*
  %7 = getelementptr inbounds %"class.std::queue.20", %"class.std::queue.20"* %0, i64 0, i32 0, i32 0, i32 0, i32 2, i32 3
  %8 = load i64**, i64*** %7, align 8, !tbaa !96
  %9 = getelementptr inbounds %"class.std::queue.20", %"class.std::queue.20"* %0, i64 0, i32 0, i32 0, i32 0, i32 3, i32 3
  %10 = load i64**, i64*** %9, align 8, !tbaa !91
  %11 = getelementptr inbounds i64*, i64** %10, i64 1
  %12 = icmp ult i64** %8, %11
  br i1 %12, label %13, label %23

; <label>:13:                                     ; preds = %5
  br label %14

; <label>:14:                                     ; preds = %13, %14
  %15 = phi i64** [ %18, %14 ], [ %8, %13 ]
  %16 = bitcast i64** %15 to i8**
  %17 = load i8*, i8** %16, align 8, !tbaa !27
  tail call void @_ZdlPv(i8* %17) #8
  %18 = getelementptr inbounds i64*, i64** %15, i64 1
  %19 = icmp ult i64** %15, %10
  br i1 %19, label %14, label %20

; <label>:20:                                     ; preds = %14
  %21 = bitcast %"class.std::queue.20"* %0 to i8**
  %22 = load i8*, i8** %21, align 8, !tbaa !92
  br label %23

; <label>:23:                                     ; preds = %20, %5
  %24 = phi i8* [ %22, %20 ], [ %6, %5 ]
  tail call void @_ZdlPv(i8* %24) #8
  br label %25

; <label>:25:                                     ; preds = %1, %23
  ret void
}

; Function Attrs: inlinehint nounwind uwtable
define linkonce_odr void @_ZNSt5queueIiSt5dequeIiSaIiEEED2Ev(%"class.std::queue.12"*) unnamed_addr #17 comdat align 2 personality i32 (...)* @__gxx_personality_v0 {
  %2 = getelementptr inbounds %"class.std::queue.12", %"class.std::queue.12"* %0, i64 0, i32 0, i32 0, i32 0, i32 0
  %3 = load i32**, i32*** %2, align 8, !tbaa !72
  %4 = icmp eq i32** %3, null
  br i1 %4, label %25, label %5

; <label>:5:                                      ; preds = %1
  %6 = bitcast i32** %3 to i8*
  %7 = getelementptr inbounds %"class.std::queue.12", %"class.std::queue.12"* %0, i64 0, i32 0, i32 0, i32 0, i32 2, i32 3
  %8 = load i32**, i32*** %7, align 8, !tbaa !76
  %9 = getelementptr inbounds %"class.std::queue.12", %"class.std::queue.12"* %0, i64 0, i32 0, i32 0, i32 0, i32 3, i32 3
  %10 = load i32**, i32*** %9, align 8, !tbaa !71
  %11 = getelementptr inbounds i32*, i32** %10, i64 1
  %12 = icmp ult i32** %8, %11
  br i1 %12, label %13, label %23

; <label>:13:                                     ; preds = %5
  br label %14

; <label>:14:                                     ; preds = %13, %14
  %15 = phi i32** [ %18, %14 ], [ %8, %13 ]
  %16 = bitcast i32** %15 to i8**
  %17 = load i8*, i8** %16, align 8, !tbaa !27
  tail call void @_ZdlPv(i8* %17) #8
  %18 = getelementptr inbounds i32*, i32** %15, i64 1
  %19 = icmp ult i32** %15, %10
  br i1 %19, label %14, label %20

; <label>:20:                                     ; preds = %14
  %21 = bitcast %"class.std::queue.12"* %0 to i8**
  %22 = load i8*, i8** %21, align 8, !tbaa !72
  br label %23

; <label>:23:                                     ; preds = %20, %5
  %24 = phi i8* [ %22, %20 ], [ %6, %5 ]
  tail call void @_ZdlPv(i8* %24) #8
  br label %25

; <label>:25:                                     ; preds = %1, %23
  ret void
}

; Function Attrs: inlinehint nounwind uwtable
define linkonce_odr void @_ZNSt5queueIsSt5dequeIsSaIsEEED2Ev(%"class.std::queue.4"*) unnamed_addr #17 comdat align 2 personality i32 (...)* @__gxx_personality_v0 {
  %2 = getelementptr inbounds %"class.std::queue.4", %"class.std::queue.4"* %0, i64 0, i32 0, i32 0, i32 0, i32 0
  %3 = load i16**, i16*** %2, align 8, !tbaa !53
  %4 = icmp eq i16** %3, null
  br i1 %4, label %25, label %5

; <label>:5:                                      ; preds = %1
  %6 = bitcast i16** %3 to i8*
  %7 = getelementptr inbounds %"class.std::queue.4", %"class.std::queue.4"* %0, i64 0, i32 0, i32 0, i32 0, i32 2, i32 3
  %8 = load i16**, i16*** %7, align 8, !tbaa !57
  %9 = getelementptr inbounds %"class.std::queue.4", %"class.std::queue.4"* %0, i64 0, i32 0, i32 0, i32 0, i32 3, i32 3
  %10 = load i16**, i16*** %9, align 8, !tbaa !52
  %11 = getelementptr inbounds i16*, i16** %10, i64 1
  %12 = icmp ult i16** %8, %11
  br i1 %12, label %13, label %23

; <label>:13:                                     ; preds = %5
  br label %14

; <label>:14:                                     ; preds = %13, %14
  %15 = phi i16** [ %18, %14 ], [ %8, %13 ]
  %16 = bitcast i16** %15 to i8**
  %17 = load i8*, i8** %16, align 8, !tbaa !27
  tail call void @_ZdlPv(i8* %17) #8
  %18 = getelementptr inbounds i16*, i16** %15, i64 1
  %19 = icmp ult i16** %15, %10
  br i1 %19, label %14, label %20

; <label>:20:                                     ; preds = %14
  %21 = bitcast %"class.std::queue.4"* %0 to i8**
  %22 = load i8*, i8** %21, align 8, !tbaa !53
  br label %23

; <label>:23:                                     ; preds = %20, %5
  %24 = phi i8* [ %22, %20 ], [ %6, %5 ]
  tail call void @_ZdlPv(i8* %24) #8
  br label %25

; <label>:25:                                     ; preds = %1, %23
  ret void
}

; Function Attrs: inlinehint nounwind uwtable
define linkonce_odr void @_ZNSt5queueIaSt5dequeIaSaIaEEED2Ev(%"class.std::queue"*) unnamed_addr #17 comdat align 2 personality i32 (...)* @__gxx_personality_v0 {
  %2 = getelementptr inbounds %"class.std::queue", %"class.std::queue"* %0, i64 0, i32 0, i32 0, i32 0, i32 0
  %3 = load i8**, i8*** %2, align 8, !tbaa !26
  %4 = icmp eq i8** %3, null
  br i1 %4, label %24, label %5

; <label>:5:                                      ; preds = %1
  %6 = bitcast i8** %3 to i8*
  %7 = getelementptr inbounds %"class.std::queue", %"class.std::queue"* %0, i64 0, i32 0, i32 0, i32 0, i32 2, i32 3
  %8 = load i8**, i8*** %7, align 8, !tbaa !31
  %9 = getelementptr inbounds %"class.std::queue", %"class.std::queue"* %0, i64 0, i32 0, i32 0, i32 0, i32 3, i32 3
  %10 = load i8**, i8*** %9, align 8, !tbaa !25
  %11 = getelementptr inbounds i8*, i8** %10, i64 1
  %12 = icmp ult i8** %8, %11
  br i1 %12, label %13, label %22

; <label>:13:                                     ; preds = %5
  br label %14

; <label>:14:                                     ; preds = %13, %14
  %15 = phi i8** [ %17, %14 ], [ %8, %13 ]
  %16 = load i8*, i8** %15, align 8, !tbaa !27
  tail call void @_ZdlPv(i8* %16) #8
  %17 = getelementptr inbounds i8*, i8** %15, i64 1
  %18 = icmp ult i8** %15, %10
  br i1 %18, label %14, label %19

; <label>:19:                                     ; preds = %14
  %20 = bitcast %"class.std::queue"* %0 to i8**
  %21 = load i8*, i8** %20, align 8, !tbaa !26
  br label %22

; <label>:22:                                     ; preds = %19, %5
  %23 = phi i8* [ %21, %19 ], [ %6, %5 ]
  tail call void @_ZdlPv(i8* %23) #8
  br label %24

; <label>:24:                                     ; preds = %1, %22
  ret void
}

; Function Attrs: uwtable
define linkonce_odr void @_ZN4MARC10ThreadPool7destroyEv(%"class.MARC::ThreadPool"*) local_unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %2 = alloca %"class.std::function", align 8
  %3 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 4
  %4 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 4, i32 1, i32 0, i32 0
  %5 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 4, i32 2, i32 0, i32 0, i32 0, i32 3
  %6 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 4, i32 2, i32 0, i32 0, i32 0, i32 2
  %7 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 4, i32 2, i32 0, i32 0, i32 0, i32 3, i32 3
  %8 = bitcast %"class.std::function"*** %7 to i64*
  %9 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 4, i32 2, i32 0, i32 0, i32 0, i32 2, i32 3
  %10 = bitcast %"class.std::function"*** %9 to i64*
  %11 = bitcast %"struct.std::_Deque_iterator.52"* %5 to i64*
  %12 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 4, i32 2, i32 0, i32 0, i32 0, i32 3, i32 1
  %13 = bitcast %"class.std::function"** %12 to i64*
  %14 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 4, i32 2, i32 0, i32 0, i32 0, i32 2, i32 2
  %15 = bitcast %"class.std::function"** %14 to i64*
  %16 = bitcast %"struct.std::_Deque_iterator.52"* %6 to i64*
  %17 = bitcast %"class.std::function"* %2 to i8*
  %18 = getelementptr inbounds %"class.std::function", %"class.std::function"* %2, i64 0, i32 0, i32 1
  %19 = getelementptr inbounds %"class.std::function", %"class.std::function"* %2, i64 0, i32 1
  %20 = getelementptr inbounds %"class.std::function", %"class.std::function"* %2, i64 0, i32 0, i32 0
  br label %21

; <label>:21:                                     ; preds = %63, %1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %22, label %26

; <label>:22:                                     ; preds = %21
  %23 = call i32 @pthread_mutex_lock(%union.pthread_mutex_t* nonnull %4) #8
  %24 = icmp eq i32 %23, 0
  br i1 %24, label %26, label %25

; <label>:25:                                     ; preds = %22
  call void @_ZSt20__throw_system_errori(i32 %23) #23
  unreachable

; <label>:26:                                     ; preds = %22, %21
  %27 = load i64, i64* %8, align 8, !tbaa !207
  %28 = load i64, i64* %10, align 8, !tbaa !207
  %29 = sub i64 %27, %28
  %30 = shl i64 %29, 1
  %31 = add i64 %30, -16
  %32 = load i64, i64* %11, align 8, !tbaa !209
  %33 = load i64, i64* %13, align 8, !tbaa !210
  %34 = sub i64 %32, %33
  %35 = ashr exact i64 %34, 5
  %36 = add nsw i64 %31, %35
  %37 = load i64, i64* %15, align 8, !tbaa !211
  %38 = load i64, i64* %16, align 8, !tbaa !209
  %39 = sub i64 %37, %38
  %40 = ashr exact i64 %39, 5
  %41 = add nsw i64 %36, %40
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %42, label %44

; <label>:42:                                     ; preds = %26
  %43 = call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* nonnull %4) #8
  br label %44

; <label>:44:                                     ; preds = %26, %42
  %45 = icmp sgt i64 %41, 0
  br i1 %45, label %46, label %78

; <label>:46:                                     ; preds = %44
  call void @llvm.lifetime.start.p0i8(i64 32, i8* nonnull %17) #8
  store i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* null, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %18, align 8, !tbaa !123
  %47 = invoke zeroext i1 @_ZN4MARC15ThreadSafeQueueISt8functionIFvvEEE7waitPopERS3_(%"class.MARC::ThreadSafeQueue.45"* nonnull %3, %"class.std::function"* nonnull dereferenceable(32) %2)
          to label %48 unwind label %64

; <label>:48:                                     ; preds = %46
  %49 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %18, align 8, !tbaa !123
  %50 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %49, null
  br i1 %50, label %51, label %53

; <label>:51:                                     ; preds = %48
  invoke void @_ZSt25__throw_bad_function_callv() #23
          to label %52 unwind label %66

; <label>:52:                                     ; preds = %51
  unreachable

; <label>:53:                                     ; preds = %48
  %54 = load void (%"union.std::_Any_data"*)*, void (%"union.std::_Any_data"*)** %19, align 8, !tbaa !212
  invoke void %54(%"union.std::_Any_data"* nonnull dereferenceable(16) %20)
          to label %55 unwind label %64

; <label>:55:                                     ; preds = %53
  %56 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %18, align 8, !tbaa !123
  %57 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %56, null
  br i1 %57, label %63, label %58

; <label>:58:                                     ; preds = %55
  %59 = invoke zeroext i1 %56(%"union.std::_Any_data"* nonnull dereferenceable(16) %20, %"union.std::_Any_data"* nonnull dereferenceable(16) %20, i32 3)
          to label %63 unwind label %60

; <label>:60:                                     ; preds = %58
  %61 = landingpad { i8*, i32 }
          catch i8* null
  %62 = extractvalue { i8*, i32 } %61, 0
  call void @__clang_call_terminate(i8* %62) #22
  unreachable

; <label>:63:                                     ; preds = %55, %58
  call void @llvm.lifetime.end.p0i8(i64 32, i8* nonnull %17) #8
  br label %21

; <label>:64:                                     ; preds = %46, %53
  %65 = landingpad { i8*, i32 }
          cleanup
  br label %68

; <label>:66:                                     ; preds = %51
  %67 = landingpad { i8*, i32 }
          cleanup
  br label %68

; <label>:68:                                     ; preds = %66, %64
  %69 = phi { i8*, i32 } [ %65, %64 ], [ %67, %66 ]
  %70 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %18, align 8, !tbaa !123
  %71 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %70, null
  br i1 %71, label %77, label %72

; <label>:72:                                     ; preds = %68
  %73 = invoke zeroext i1 %70(%"union.std::_Any_data"* nonnull dereferenceable(16) %20, %"union.std::_Any_data"* nonnull dereferenceable(16) %20, i32 3)
          to label %77 unwind label %74

; <label>:74:                                     ; preds = %72
  %75 = landingpad { i8*, i32 }
          catch i8* null
  %76 = extractvalue { i8*, i32 } %75, 0
  call void @__clang_call_terminate(i8* %76) #22
  unreachable

; <label>:77:                                     ; preds = %68, %72
  call void @llvm.lifetime.end.p0i8(i64 32, i8* nonnull %17) #8
  resume { i8*, i32 } %69

; <label>:78:                                     ; preds = %44
  %79 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 0, i32 0, i32 0
  store atomic i8 1, i8* %79 seq_cst, align 1
  %80 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 1, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %81, label %86

; <label>:81:                                     ; preds = %78
  %82 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %80, i64 0, i32 0, i32 0
  %83 = call i32 @pthread_mutex_lock(%union.pthread_mutex_t* nonnull %82) #8
  %84 = icmp eq i32 %83, 0
  br i1 %84, label %86, label %85

; <label>:85:                                     ; preds = %81
  call void @_ZSt20__throw_system_errori(i32 %83) #23
  unreachable

; <label>:86:                                     ; preds = %81, %78
  %87 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 1, i32 0, i32 0, i32 0
  %88 = load atomic i8, i8* %87 seq_cst, align 1
  %89 = and i8 %88, 1
  %90 = icmp eq i8 %89, 0
  br i1 %90, label %94, label %91

; <label>:91:                                     ; preds = %86
  store atomic i8 0, i8* %87 seq_cst, align 1
  %92 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 1, i32 3
  call void @_ZNSt18condition_variable10notify_allEv(%"class.std::condition_variable"* %92) #8
  %93 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 1, i32 4
  call void @_ZNSt18condition_variable10notify_allEv(%"class.std::condition_variable"* %93) #8
  br label %94

; <label>:94:                                     ; preds = %91, %86
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %95, label %98

; <label>:95:                                     ; preds = %94
  %96 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %80, i64 0, i32 0, i32 0
  %97 = call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %96) #8
  br label %98

; <label>:98:                                     ; preds = %94, %95
  %99 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 2, i32 0, i32 0, i32 0
  %100 = load %"class.std::thread"*, %"class.std::thread"** %99, align 8, !tbaa !27
  %101 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 2, i32 0, i32 0, i32 1
  %102 = load %"class.std::thread"*, %"class.std::thread"** %101, align 8, !tbaa !27
  %103 = icmp eq %"class.std::thread"* %100, %102
  br i1 %103, label %105, label %104

; <label>:104:                                    ; preds = %98
  br label %109

; <label>:105:                                    ; preds = %115, %98
  %106 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 3
  %107 = load %"struct.std::atomic"*, %"struct.std::atomic"** %106, align 8, !tbaa !137
  %108 = icmp eq %"struct.std::atomic"* %107, null
  br i1 %108, label %120, label %118

; <label>:109:                                    ; preds = %104, %115
  %110 = phi %"class.std::thread"* [ %116, %115 ], [ %100, %104 ]
  %111 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %110, i64 0, i32 0, i32 0
  %112 = load i64, i64* %111, align 8
  %113 = icmp eq i64 %112, 0
  br i1 %113, label %115, label %114

; <label>:114:                                    ; preds = %109
  call void @_ZNSt6thread4joinEv(%"class.std::thread"* nonnull %110)
  br label %115

; <label>:115:                                    ; preds = %109, %114
  %116 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %110, i64 1
  %117 = icmp eq %"class.std::thread"* %116, %102
  br i1 %117, label %105, label %109

; <label>:118:                                    ; preds = %105
  %119 = getelementptr inbounds %"struct.std::atomic", %"struct.std::atomic"* %107, i64 0, i32 0, i32 0
  call void @_ZdaPv(i8* %119) #25
  br label %120

; <label>:120:                                    ; preds = %118, %105
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN4MARC15ThreadSafeQueueISt8functionIFvvEEED2Ev(%"class.MARC::ThreadSafeQueue.45"*) unnamed_addr #10 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %2 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %3, label %9

; <label>:3:                                      ; preds = %1
  %4 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %2, i64 0, i32 0, i32 0
  %5 = tail call i32 @pthread_mutex_lock(%union.pthread_mutex_t* nonnull %4) #8
  %6 = icmp eq i32 %5, 0
  br i1 %6, label %9, label %7

; <label>:7:                                      ; preds = %3
  invoke void @_ZSt20__throw_system_errori(i32 %5) #23
          to label %8 unwind label %25

; <label>:8:                                      ; preds = %7
  unreachable

; <label>:9:                                      ; preds = %3, %1
  %10 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 0, i32 0, i32 0
  %11 = load atomic i8, i8* %10 seq_cst, align 1
  %12 = and i8 %11, 1
  %13 = icmp eq i8 %12, 0
  br i1 %13, label %17, label %14

; <label>:14:                                     ; preds = %9
  store atomic i8 0, i8* %10 seq_cst, align 1
  %15 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 3
  tail call void @_ZNSt18condition_variable10notify_allEv(%"class.std::condition_variable"* %15) #8
  %16 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 4
  tail call void @_ZNSt18condition_variable10notify_allEv(%"class.std::condition_variable"* %16) #8
  br label %17

; <label>:17:                                     ; preds = %14, %9
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %18, label %21

; <label>:18:                                     ; preds = %17
  %19 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %2, i64 0, i32 0, i32 0
  %20 = tail call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %19) #8
  br label %21

; <label>:21:                                     ; preds = %18, %17
  %22 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 4
  tail call void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"* %22) #8
  %23 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 3
  tail call void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"* %23) #8
  %24 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 2, i32 0
  tail call void @_ZNSt5dequeISt8functionIFvvEESaIS2_EED2Ev(%"class.std::deque.47"* %24) #8
  ret void

; <label>:25:                                     ; preds = %7
  %26 = landingpad { i8*, i32 }
          catch i8* null
  %27 = extractvalue { i8*, i32 } %26, 0
  %28 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 4
  tail call void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"* %28) #8
  %29 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 3
  tail call void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"* %29) #8
  %30 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 2, i32 0
  tail call void @_ZNSt5dequeISt8functionIFvvEESaIS2_EED2Ev(%"class.std::deque.47"* %30) #8
  tail call void @__clang_call_terminate(i8* %27) #22
  unreachable
}

declare void @_ZSt9terminatev() local_unnamed_addr

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN4MARC15ThreadSafeQueueISt10unique_ptrINS_11IThreadTaskESt14default_deleteIS2_EEED2Ev(%"class.MARC::ThreadSafeQueue.32"*) unnamed_addr #10 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %2 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %3, label %9

; <label>:3:                                      ; preds = %1
  %4 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %2, i64 0, i32 0, i32 0
  %5 = tail call i32 @pthread_mutex_lock(%union.pthread_mutex_t* nonnull %4) #8
  %6 = icmp eq i32 %5, 0
  br i1 %6, label %9, label %7

; <label>:7:                                      ; preds = %3
  invoke void @_ZSt20__throw_system_errori(i32 %5) #23
          to label %8 unwind label %25

; <label>:8:                                      ; preds = %7
  unreachable

; <label>:9:                                      ; preds = %3, %1
  %10 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 0, i32 0, i32 0
  %11 = load atomic i8, i8* %10 seq_cst, align 1
  %12 = and i8 %11, 1
  %13 = icmp eq i8 %12, 0
  br i1 %13, label %17, label %14

; <label>:14:                                     ; preds = %9
  store atomic i8 0, i8* %10 seq_cst, align 1
  %15 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 3
  tail call void @_ZNSt18condition_variable10notify_allEv(%"class.std::condition_variable"* %15) #8
  %16 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 4
  tail call void @_ZNSt18condition_variable10notify_allEv(%"class.std::condition_variable"* %16) #8
  br label %17

; <label>:17:                                     ; preds = %14, %9
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %18, label %21

; <label>:18:                                     ; preds = %17
  %19 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %2, i64 0, i32 0, i32 0
  %20 = tail call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %19) #8
  br label %21

; <label>:21:                                     ; preds = %18, %17
  %22 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 4
  tail call void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"* %22) #8
  %23 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 3
  tail call void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"* %23) #8
  %24 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 2, i32 0
  tail call void @_ZNSt5dequeISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EESaIS5_EED2Ev(%"class.std::deque.34"* %24) #8
  ret void

; <label>:25:                                     ; preds = %7
  %26 = landingpad { i8*, i32 }
          catch i8* null
  %27 = extractvalue { i8*, i32 } %26, 0
  %28 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 4
  tail call void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"* %28) #8
  %29 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 3
  tail call void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"* %29) #8
  %30 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 2, i32 0
  tail call void @_ZNSt5dequeISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EESaIS5_EED2Ev(%"class.std::deque.34"* %30) #8
  tail call void @__clang_call_terminate(i8* %27) #22
  unreachable
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZNSt6vectorISt6threadSaIS0_EED2Ev(%"class.std::vector"*) unnamed_addr #10 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %2 = getelementptr inbounds %"class.std::vector", %"class.std::vector"* %0, i64 0, i32 0, i32 0, i32 0
  %3 = load %"class.std::thread"*, %"class.std::thread"** %2, align 8, !tbaa !151
  %4 = getelementptr inbounds %"class.std::vector", %"class.std::vector"* %0, i64 0, i32 0, i32 0, i32 1
  %5 = load %"class.std::thread"*, %"class.std::thread"** %4, align 8, !tbaa !154
  %6 = icmp eq %"class.std::thread"* %3, %5
  br i1 %6, label %17, label %7

; <label>:7:                                      ; preds = %1
  br label %10

; <label>:8:                                      ; preds = %10
  %9 = icmp eq %"class.std::thread"* %15, %5
  br i1 %9, label %17, label %10

; <label>:10:                                     ; preds = %7, %8
  %11 = phi %"class.std::thread"* [ %15, %8 ], [ %3, %7 ]
  %12 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %11, i64 0, i32 0, i32 0
  %13 = load i64, i64* %12, align 8
  %14 = icmp eq i64 %13, 0
  %15 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %11, i64 1
  br i1 %14, label %8, label %16

; <label>:16:                                     ; preds = %10
  tail call void @_ZSt9terminatev() #22
  unreachable

; <label>:17:                                     ; preds = %8, %1
  %18 = icmp eq %"class.std::thread"* %3, null
  br i1 %18, label %21, label %19

; <label>:19:                                     ; preds = %17
  %20 = bitcast %"class.std::thread"* %3 to i8*
  tail call void @_ZdlPv(i8* %20) #8
  br label %21

; <label>:21:                                     ; preds = %17, %19
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZNSt5dequeISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EESaIS5_EED2Ev(%"class.std::deque.34"*) unnamed_addr #10 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %2 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %0, i64 0, i32 0, i32 0, i32 2, i32 0
  %3 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %2, align 8, !tbaa !214, !noalias !215
  %4 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %0, i64 0, i32 0, i32 0, i32 2, i32 2
  %5 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %4, align 8, !tbaa !204, !noalias !215
  %6 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %0, i64 0, i32 0, i32 0, i32 2, i32 3
  %7 = load %"class.std::unique_ptr"**, %"class.std::unique_ptr"*** %6, align 8, !tbaa !202, !noalias !215
  %8 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %0, i64 0, i32 0, i32 0, i32 3
  %9 = bitcast %"struct.std::_Deque_iterator.41"* %8 to i64*
  %10 = load i64, i64* %9, align 8, !tbaa !214, !noalias !218
  %11 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %0, i64 0, i32 0, i32 0, i32 3, i32 1
  %12 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %11, align 8, !tbaa !203, !noalias !218
  %13 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %0, i64 0, i32 0, i32 0, i32 3, i32 3
  %14 = load %"class.std::unique_ptr"**, %"class.std::unique_ptr"*** %13, align 8, !tbaa !202, !noalias !218
  %15 = getelementptr inbounds %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %7, i64 1
  %16 = icmp ult %"class.std::unique_ptr"** %15, %14
  br i1 %16, label %17, label %18

; <label>:17:                                     ; preds = %1
  br label %20

; <label>:18:                                     ; preds = %36, %1
  %19 = icmp eq %"class.std::unique_ptr"** %7, %14
  br i1 %19, label %72, label %39

; <label>:20:                                     ; preds = %17, %36
  %21 = phi %"class.std::unique_ptr"** [ %37, %36 ], [ %15, %17 ]
  %22 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %21, align 8, !tbaa !27
  br label %23

; <label>:23:                                     ; preds = %33, %20
  %24 = phi i64 [ 0, %20 ], [ %34, %33 ]
  %25 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %22, i64 %24, i32 0, i32 0, i32 0, i32 0
  %26 = load %"class.MARC::IThreadTask"*, %"class.MARC::IThreadTask"** %25, align 8, !tbaa !27
  %27 = icmp eq %"class.MARC::IThreadTask"* %26, null
  br i1 %27, label %33, label %28

; <label>:28:                                     ; preds = %23
  %29 = bitcast %"class.MARC::IThreadTask"* %26 to void (%"class.MARC::IThreadTask"*)***
  %30 = load void (%"class.MARC::IThreadTask"*)**, void (%"class.MARC::IThreadTask"*)*** %29, align 8, !tbaa !133
  %31 = getelementptr inbounds void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %30, i64 2
  %32 = load void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %31, align 8
  tail call void %32(%"class.MARC::IThreadTask"* nonnull %26) #8
  br label %33

; <label>:33:                                     ; preds = %28, %23
  store %"class.MARC::IThreadTask"* null, %"class.MARC::IThreadTask"** %25, align 8, !tbaa !27
  %34 = add nuw nsw i64 %24, 1
  %35 = icmp eq i64 %34, 64
  br i1 %35, label %36, label %23

; <label>:36:                                     ; preds = %33
  %37 = getelementptr inbounds %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %21, i64 1
  %38 = icmp ult %"class.std::unique_ptr"** %37, %14
  br i1 %38, label %20, label %18

; <label>:39:                                     ; preds = %18
  %40 = icmp eq %"class.std::unique_ptr"* %3, %5
  br i1 %40, label %55, label %41

; <label>:41:                                     ; preds = %39
  br label %42

; <label>:42:                                     ; preds = %41, %52
  %43 = phi %"class.std::unique_ptr"* [ %53, %52 ], [ %3, %41 ]
  %44 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %43, i64 0, i32 0, i32 0, i32 0, i32 0
  %45 = load %"class.MARC::IThreadTask"*, %"class.MARC::IThreadTask"** %44, align 8, !tbaa !27
  %46 = icmp eq %"class.MARC::IThreadTask"* %45, null
  br i1 %46, label %52, label %47

; <label>:47:                                     ; preds = %42
  %48 = bitcast %"class.MARC::IThreadTask"* %45 to void (%"class.MARC::IThreadTask"*)***
  %49 = load void (%"class.MARC::IThreadTask"*)**, void (%"class.MARC::IThreadTask"*)*** %48, align 8, !tbaa !133
  %50 = getelementptr inbounds void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %49, i64 2
  %51 = load void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %50, align 8
  tail call void %51(%"class.MARC::IThreadTask"* nonnull %45) #8
  br label %52

; <label>:52:                                     ; preds = %47, %42
  store %"class.MARC::IThreadTask"* null, %"class.MARC::IThreadTask"** %44, align 8, !tbaa !27
  %53 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %43, i64 1
  %54 = icmp eq %"class.std::unique_ptr"* %53, %5
  br i1 %54, label %55, label %42

; <label>:55:                                     ; preds = %52, %39
  %56 = inttoptr i64 %10 to %"class.std::unique_ptr"*
  %57 = icmp eq %"class.std::unique_ptr"* %12, %56
  br i1 %57, label %89, label %58

; <label>:58:                                     ; preds = %55
  br label %59

; <label>:59:                                     ; preds = %58, %69
  %60 = phi %"class.std::unique_ptr"* [ %70, %69 ], [ %12, %58 ]
  %61 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %60, i64 0, i32 0, i32 0, i32 0, i32 0
  %62 = load %"class.MARC::IThreadTask"*, %"class.MARC::IThreadTask"** %61, align 8, !tbaa !27
  %63 = icmp eq %"class.MARC::IThreadTask"* %62, null
  br i1 %63, label %69, label %64

; <label>:64:                                     ; preds = %59
  %65 = bitcast %"class.MARC::IThreadTask"* %62 to void (%"class.MARC::IThreadTask"*)***
  %66 = load void (%"class.MARC::IThreadTask"*)**, void (%"class.MARC::IThreadTask"*)*** %65, align 8, !tbaa !133
  %67 = getelementptr inbounds void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %66, i64 2
  %68 = load void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %67, align 8
  tail call void %68(%"class.MARC::IThreadTask"* nonnull %62) #8
  br label %69

; <label>:69:                                     ; preds = %64, %59
  store %"class.MARC::IThreadTask"* null, %"class.MARC::IThreadTask"** %61, align 8, !tbaa !27
  %70 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %60, i64 1
  %71 = icmp eq %"class.std::unique_ptr"* %70, %56
  br i1 %71, label %89, label %59

; <label>:72:                                     ; preds = %18
  %73 = inttoptr i64 %10 to %"class.std::unique_ptr"*
  %74 = icmp eq %"class.std::unique_ptr"* %3, %73
  br i1 %74, label %89, label %75

; <label>:75:                                     ; preds = %72
  br label %76

; <label>:76:                                     ; preds = %75, %86
  %77 = phi %"class.std::unique_ptr"* [ %87, %86 ], [ %3, %75 ]
  %78 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %77, i64 0, i32 0, i32 0, i32 0, i32 0
  %79 = load %"class.MARC::IThreadTask"*, %"class.MARC::IThreadTask"** %78, align 8, !tbaa !27
  %80 = icmp eq %"class.MARC::IThreadTask"* %79, null
  br i1 %80, label %86, label %81

; <label>:81:                                     ; preds = %76
  %82 = bitcast %"class.MARC::IThreadTask"* %79 to void (%"class.MARC::IThreadTask"*)***
  %83 = load void (%"class.MARC::IThreadTask"*)**, void (%"class.MARC::IThreadTask"*)*** %82, align 8, !tbaa !133
  %84 = getelementptr inbounds void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %83, i64 2
  %85 = load void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %84, align 8
  tail call void %85(%"class.MARC::IThreadTask"* nonnull %79) #8
  br label %86

; <label>:86:                                     ; preds = %81, %76
  store %"class.MARC::IThreadTask"* null, %"class.MARC::IThreadTask"** %78, align 8, !tbaa !27
  %87 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %77, i64 1
  %88 = icmp eq %"class.std::unique_ptr"* %87, %73
  br i1 %88, label %89, label %76

; <label>:89:                                     ; preds = %69, %86, %72, %55
  %90 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %0, i64 0, i32 0, i32 0, i32 0
  %91 = load %"class.std::unique_ptr"**, %"class.std::unique_ptr"*** %90, align 8, !tbaa !201
  %92 = icmp eq %"class.std::unique_ptr"** %91, null
  br i1 %92, label %111, label %93

; <label>:93:                                     ; preds = %89
  %94 = bitcast %"class.std::unique_ptr"** %91 to i8*
  %95 = load %"class.std::unique_ptr"**, %"class.std::unique_ptr"*** %6, align 8, !tbaa !221
  %96 = load %"class.std::unique_ptr"**, %"class.std::unique_ptr"*** %13, align 8, !tbaa !200
  %97 = getelementptr inbounds %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %96, i64 1
  %98 = icmp ult %"class.std::unique_ptr"** %95, %97
  br i1 %98, label %99, label %109

; <label>:99:                                     ; preds = %93
  br label %100

; <label>:100:                                    ; preds = %99, %100
  %101 = phi %"class.std::unique_ptr"** [ %104, %100 ], [ %95, %99 ]
  %102 = bitcast %"class.std::unique_ptr"** %101 to i8**
  %103 = load i8*, i8** %102, align 8, !tbaa !27
  tail call void @_ZdlPv(i8* %103) #8
  %104 = getelementptr inbounds %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %101, i64 1
  %105 = icmp ult %"class.std::unique_ptr"** %101, %96
  br i1 %105, label %100, label %106

; <label>:106:                                    ; preds = %100
  %107 = bitcast %"class.std::deque.34"* %0 to i8**
  %108 = load i8*, i8** %107, align 8, !tbaa !201
  br label %109

; <label>:109:                                    ; preds = %106, %93
  %110 = phi i8* [ %108, %106 ], [ %94, %93 ]
  tail call void @_ZdlPv(i8* %110) #8
  br label %111

; <label>:111:                                    ; preds = %89, %109
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZNSt5dequeISt8functionIFvvEESaIS2_EED2Ev(%"class.std::deque.47"*) unnamed_addr #10 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %2 = alloca %"struct.std::_Deque_iterator.52", align 16
  %3 = alloca %"struct.std::_Deque_iterator.52", align 16
  %4 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %0, i64 0, i32 0, i32 0, i32 2
  %5 = bitcast %"struct.std::_Deque_iterator.52"* %4 to <2 x i64>*
  %6 = load <2 x i64>, <2 x i64>* %5, align 8, !tbaa !27, !noalias !222
  %7 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %0, i64 0, i32 0, i32 0, i32 2, i32 2
  %8 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %0, i64 0, i32 0, i32 0, i32 2, i32 3
  %9 = bitcast %"class.std::function"** %7 to <2 x i64>*
  %10 = load <2 x i64>, <2 x i64>* %9, align 8, !tbaa !27, !noalias !222
  %11 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %0, i64 0, i32 0, i32 0, i32 3
  %12 = bitcast %"struct.std::_Deque_iterator.52"* %11 to <2 x i64>*
  %13 = load <2 x i64>, <2 x i64>* %12, align 8, !tbaa !27, !noalias !225
  %14 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %0, i64 0, i32 0, i32 0, i32 3, i32 2
  %15 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %0, i64 0, i32 0, i32 0, i32 3, i32 3
  %16 = bitcast %"class.std::function"** %14 to <2 x i64>*
  %17 = load <2 x i64>, <2 x i64>* %16, align 8, !tbaa !27, !noalias !225
  %18 = bitcast %"struct.std::_Deque_iterator.52"* %2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 32, i8* nonnull %18)
  %19 = bitcast %"struct.std::_Deque_iterator.52"* %3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 32, i8* nonnull %19)
  %20 = bitcast %"struct.std::_Deque_iterator.52"* %2 to <2 x i64>*
  store <2 x i64> %6, <2 x i64>* %20, align 16, !tbaa !27
  %21 = getelementptr inbounds %"struct.std::_Deque_iterator.52", %"struct.std::_Deque_iterator.52"* %2, i64 0, i32 2
  %22 = bitcast %"class.std::function"** %21 to <2 x i64>*
  store <2 x i64> %10, <2 x i64>* %22, align 16, !tbaa !27
  %23 = bitcast %"struct.std::_Deque_iterator.52"* %3 to <2 x i64>*
  store <2 x i64> %13, <2 x i64>* %23, align 16, !tbaa !27
  %24 = getelementptr inbounds %"struct.std::_Deque_iterator.52", %"struct.std::_Deque_iterator.52"* %3, i64 0, i32 2
  %25 = bitcast %"class.std::function"** %24 to <2 x i64>*
  store <2 x i64> %17, <2 x i64>* %25, align 16, !tbaa !27
  invoke void @_ZNSt5dequeISt8functionIFvvEESaIS2_EE19_M_destroy_data_auxESt15_Deque_iteratorIS2_RS2_PS2_ES8_(%"class.std::deque.47"* %0, %"struct.std::_Deque_iterator.52"* nonnull %2, %"struct.std::_Deque_iterator.52"* nonnull %3)
          to label %26 unwind label %49

; <label>:26:                                     ; preds = %1
  call void @llvm.lifetime.end.p0i8(i64 32, i8* nonnull %18)
  call void @llvm.lifetime.end.p0i8(i64 32, i8* nonnull %19)
  %27 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %0, i64 0, i32 0, i32 0, i32 0
  %28 = load %"class.std::function"**, %"class.std::function"*** %27, align 8, !tbaa !228
  %29 = icmp eq %"class.std::function"** %28, null
  br i1 %29, label %48, label %30

; <label>:30:                                     ; preds = %26
  %31 = bitcast %"class.std::function"** %28 to i8*
  %32 = load %"class.std::function"**, %"class.std::function"*** %8, align 8, !tbaa !231
  %33 = load %"class.std::function"**, %"class.std::function"*** %15, align 8, !tbaa !232
  %34 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %33, i64 1
  %35 = icmp ult %"class.std::function"** %32, %34
  br i1 %35, label %36, label %46

; <label>:36:                                     ; preds = %30
  br label %37

; <label>:37:                                     ; preds = %36, %37
  %38 = phi %"class.std::function"** [ %41, %37 ], [ %32, %36 ]
  %39 = bitcast %"class.std::function"** %38 to i8**
  %40 = load i8*, i8** %39, align 8, !tbaa !27
  call void @_ZdlPv(i8* %40) #8
  %41 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %38, i64 1
  %42 = icmp ult %"class.std::function"** %38, %33
  br i1 %42, label %37, label %43

; <label>:43:                                     ; preds = %37
  %44 = bitcast %"class.std::deque.47"* %0 to i8**
  %45 = load i8*, i8** %44, align 8, !tbaa !228
  br label %46

; <label>:46:                                     ; preds = %43, %30
  %47 = phi i8* [ %45, %43 ], [ %31, %30 ]
  call void @_ZdlPv(i8* %47) #8
  br label %48

; <label>:48:                                     ; preds = %26, %46
  ret void

; <label>:49:                                     ; preds = %1
  %50 = landingpad { i8*, i32 }
          catch i8* null
  %51 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %0, i64 0, i32 0
  %52 = extractvalue { i8*, i32 } %50, 0
  call void @_ZNSt11_Deque_baseISt8functionIFvvEESaIS2_EED2Ev(%"class.std::_Deque_base.48"* %51) #8
  call void @__clang_call_terminate(i8* %52) #22
  unreachable
}

; Function Attrs: uwtable
define linkonce_odr void @_ZNSt5dequeISt8functionIFvvEESaIS2_EE19_M_destroy_data_auxESt15_Deque_iteratorIS2_RS2_PS2_ES8_(%"class.std::deque.47"*, %"struct.std::_Deque_iterator.52"*, %"struct.std::_Deque_iterator.52"*) local_unnamed_addr #5 comdat align 2 personality i32 (...)* @__gxx_personality_v0 {
  %4 = getelementptr inbounds %"struct.std::_Deque_iterator.52", %"struct.std::_Deque_iterator.52"* %1, i64 0, i32 3
  %5 = load %"class.std::function"**, %"class.std::function"*** %4, align 8, !tbaa !207
  %6 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %5, i64 1
  %7 = getelementptr inbounds %"struct.std::_Deque_iterator.52", %"struct.std::_Deque_iterator.52"* %2, i64 0, i32 3
  %8 = load %"class.std::function"**, %"class.std::function"*** %7, align 8, !tbaa !207
  %9 = icmp ult %"class.std::function"** %6, %8
  br i1 %9, label %10, label %13

; <label>:10:                                     ; preds = %3
  br label %19

; <label>:11:                                     ; preds = %196
  %12 = load %"class.std::function"**, %"class.std::function"*** %4, align 8, !tbaa !207
  br label %13

; <label>:13:                                     ; preds = %11, %3
  %14 = phi %"class.std::function"** [ %5, %3 ], [ %12, %11 ]
  %15 = phi %"class.std::function"** [ %8, %3 ], [ %198, %11 ]
  %16 = icmp eq %"class.std::function"** %14, %15
  %17 = getelementptr inbounds %"struct.std::_Deque_iterator.52", %"struct.std::_Deque_iterator.52"* %1, i64 0, i32 0
  %18 = load %"class.std::function"*, %"class.std::function"** %17, align 8, !tbaa !209
  br i1 %16, label %75, label %35

; <label>:19:                                     ; preds = %10, %196
  %20 = phi %"class.std::function"** [ %197, %196 ], [ %6, %10 ]
  %21 = load %"class.std::function"*, %"class.std::function"** %20, align 8, !tbaa !27
  %22 = getelementptr inbounds %"class.std::function", %"class.std::function"* %21, i64 0, i32 0, i32 1
  %23 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %22, align 8, !tbaa !123
  %24 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %23, null
  br i1 %24, label %31, label %25

; <label>:25:                                     ; preds = %19
  %26 = getelementptr inbounds %"class.std::function", %"class.std::function"* %21, i64 0, i32 0, i32 0
  %27 = invoke zeroext i1 %23(%"union.std::_Any_data"* dereferenceable(16) %26, %"union.std::_Any_data"* dereferenceable(16) %26, i32 3)
          to label %31 unwind label %28

; <label>:28:                                     ; preds = %193, %186, %179, %172, %165, %158, %151, %144, %137, %130, %123, %116, %109, %102, %95, %25
  %29 = landingpad { i8*, i32 }
          catch i8* null
  %30 = extractvalue { i8*, i32 } %29, 0
  tail call void @__clang_call_terminate(i8* %30) #22
  unreachable

; <label>:31:                                     ; preds = %25, %19
  %32 = getelementptr inbounds %"class.std::function", %"class.std::function"* %21, i64 1, i32 0, i32 1
  %33 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %32, align 8, !tbaa !123
  %34 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %33, null
  br i1 %34, label %98, label %95

; <label>:35:                                     ; preds = %13
  %36 = getelementptr inbounds %"struct.std::_Deque_iterator.52", %"struct.std::_Deque_iterator.52"* %1, i64 0, i32 2
  %37 = load %"class.std::function"*, %"class.std::function"** %36, align 8, !tbaa !211
  %38 = icmp eq %"class.std::function"* %18, %37
  br i1 %38, label %54, label %39

; <label>:39:                                     ; preds = %35
  br label %40

; <label>:40:                                     ; preds = %39, %51
  %41 = phi %"class.std::function"* [ %52, %51 ], [ %18, %39 ]
  %42 = getelementptr inbounds %"class.std::function", %"class.std::function"* %41, i64 0, i32 0, i32 1
  %43 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %42, align 8, !tbaa !123
  %44 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %43, null
  br i1 %44, label %51, label %45

; <label>:45:                                     ; preds = %40
  %46 = getelementptr inbounds %"class.std::function", %"class.std::function"* %41, i64 0, i32 0, i32 0
  %47 = invoke zeroext i1 %43(%"union.std::_Any_data"* dereferenceable(16) %46, %"union.std::_Any_data"* dereferenceable(16) %46, i32 3)
          to label %51 unwind label %48

; <label>:48:                                     ; preds = %45
  %49 = landingpad { i8*, i32 }
          catch i8* null
  %50 = extractvalue { i8*, i32 } %49, 0
  tail call void @__clang_call_terminate(i8* %50) #22
  unreachable

; <label>:51:                                     ; preds = %45, %40
  %52 = getelementptr inbounds %"class.std::function", %"class.std::function"* %41, i64 1
  %53 = icmp eq %"class.std::function"* %52, %37
  br i1 %53, label %54, label %40

; <label>:54:                                     ; preds = %51, %35
  %55 = getelementptr inbounds %"struct.std::_Deque_iterator.52", %"struct.std::_Deque_iterator.52"* %2, i64 0, i32 1
  %56 = load %"class.std::function"*, %"class.std::function"** %55, align 8, !tbaa !210
  %57 = getelementptr inbounds %"struct.std::_Deque_iterator.52", %"struct.std::_Deque_iterator.52"* %2, i64 0, i32 0
  %58 = load %"class.std::function"*, %"class.std::function"** %57, align 8, !tbaa !209
  %59 = icmp eq %"class.std::function"* %56, %58
  br i1 %59, label %94, label %60

; <label>:60:                                     ; preds = %54
  br label %61

; <label>:61:                                     ; preds = %60, %72
  %62 = phi %"class.std::function"* [ %73, %72 ], [ %56, %60 ]
  %63 = getelementptr inbounds %"class.std::function", %"class.std::function"* %62, i64 0, i32 0, i32 1
  %64 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %63, align 8, !tbaa !123
  %65 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %64, null
  br i1 %65, label %72, label %66

; <label>:66:                                     ; preds = %61
  %67 = getelementptr inbounds %"class.std::function", %"class.std::function"* %62, i64 0, i32 0, i32 0
  %68 = invoke zeroext i1 %64(%"union.std::_Any_data"* dereferenceable(16) %67, %"union.std::_Any_data"* dereferenceable(16) %67, i32 3)
          to label %72 unwind label %69

; <label>:69:                                     ; preds = %66
  %70 = landingpad { i8*, i32 }
          catch i8* null
  %71 = extractvalue { i8*, i32 } %70, 0
  tail call void @__clang_call_terminate(i8* %71) #22
  unreachable

; <label>:72:                                     ; preds = %66, %61
  %73 = getelementptr inbounds %"class.std::function", %"class.std::function"* %62, i64 1
  %74 = icmp eq %"class.std::function"* %73, %58
  br i1 %74, label %94, label %61

; <label>:75:                                     ; preds = %13
  %76 = getelementptr inbounds %"struct.std::_Deque_iterator.52", %"struct.std::_Deque_iterator.52"* %2, i64 0, i32 0
  %77 = load %"class.std::function"*, %"class.std::function"** %76, align 8, !tbaa !209
  %78 = icmp eq %"class.std::function"* %18, %77
  br i1 %78, label %94, label %79

; <label>:79:                                     ; preds = %75
  br label %80

; <label>:80:                                     ; preds = %79, %91
  %81 = phi %"class.std::function"* [ %92, %91 ], [ %18, %79 ]
  %82 = getelementptr inbounds %"class.std::function", %"class.std::function"* %81, i64 0, i32 0, i32 1
  %83 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %82, align 8, !tbaa !123
  %84 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %83, null
  br i1 %84, label %91, label %85

; <label>:85:                                     ; preds = %80
  %86 = getelementptr inbounds %"class.std::function", %"class.std::function"* %81, i64 0, i32 0, i32 0
  %87 = invoke zeroext i1 %83(%"union.std::_Any_data"* dereferenceable(16) %86, %"union.std::_Any_data"* dereferenceable(16) %86, i32 3)
          to label %91 unwind label %88

; <label>:88:                                     ; preds = %85
  %89 = landingpad { i8*, i32 }
          catch i8* null
  %90 = extractvalue { i8*, i32 } %89, 0
  tail call void @__clang_call_terminate(i8* %90) #22
  unreachable

; <label>:91:                                     ; preds = %85, %80
  %92 = getelementptr inbounds %"class.std::function", %"class.std::function"* %81, i64 1
  %93 = icmp eq %"class.std::function"* %92, %77
  br i1 %93, label %94, label %80

; <label>:94:                                     ; preds = %72, %91, %75, %54
  ret void

; <label>:95:                                     ; preds = %31
  %96 = getelementptr inbounds %"class.std::function", %"class.std::function"* %21, i64 1, i32 0, i32 0
  %97 = invoke zeroext i1 %33(%"union.std::_Any_data"* dereferenceable(16) %96, %"union.std::_Any_data"* dereferenceable(16) %96, i32 3)
          to label %98 unwind label %28

; <label>:98:                                     ; preds = %95, %31
  %99 = getelementptr inbounds %"class.std::function", %"class.std::function"* %21, i64 2, i32 0, i32 1
  %100 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %99, align 8, !tbaa !123
  %101 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %100, null
  br i1 %101, label %105, label %102

; <label>:102:                                    ; preds = %98
  %103 = getelementptr inbounds %"class.std::function", %"class.std::function"* %21, i64 2, i32 0, i32 0
  %104 = invoke zeroext i1 %100(%"union.std::_Any_data"* dereferenceable(16) %103, %"union.std::_Any_data"* dereferenceable(16) %103, i32 3)
          to label %105 unwind label %28

; <label>:105:                                    ; preds = %102, %98
  %106 = getelementptr inbounds %"class.std::function", %"class.std::function"* %21, i64 3, i32 0, i32 1
  %107 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %106, align 8, !tbaa !123
  %108 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %107, null
  br i1 %108, label %112, label %109

; <label>:109:                                    ; preds = %105
  %110 = getelementptr inbounds %"class.std::function", %"class.std::function"* %21, i64 3, i32 0, i32 0
  %111 = invoke zeroext i1 %107(%"union.std::_Any_data"* dereferenceable(16) %110, %"union.std::_Any_data"* dereferenceable(16) %110, i32 3)
          to label %112 unwind label %28

; <label>:112:                                    ; preds = %109, %105
  %113 = getelementptr inbounds %"class.std::function", %"class.std::function"* %21, i64 4, i32 0, i32 1
  %114 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %113, align 8, !tbaa !123
  %115 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %114, null
  br i1 %115, label %119, label %116

; <label>:116:                                    ; preds = %112
  %117 = getelementptr inbounds %"class.std::function", %"class.std::function"* %21, i64 4, i32 0, i32 0
  %118 = invoke zeroext i1 %114(%"union.std::_Any_data"* dereferenceable(16) %117, %"union.std::_Any_data"* dereferenceable(16) %117, i32 3)
          to label %119 unwind label %28

; <label>:119:                                    ; preds = %116, %112
  %120 = getelementptr inbounds %"class.std::function", %"class.std::function"* %21, i64 5, i32 0, i32 1
  %121 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %120, align 8, !tbaa !123
  %122 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %121, null
  br i1 %122, label %126, label %123

; <label>:123:                                    ; preds = %119
  %124 = getelementptr inbounds %"class.std::function", %"class.std::function"* %21, i64 5, i32 0, i32 0
  %125 = invoke zeroext i1 %121(%"union.std::_Any_data"* dereferenceable(16) %124, %"union.std::_Any_data"* dereferenceable(16) %124, i32 3)
          to label %126 unwind label %28

; <label>:126:                                    ; preds = %123, %119
  %127 = getelementptr inbounds %"class.std::function", %"class.std::function"* %21, i64 6, i32 0, i32 1
  %128 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %127, align 8, !tbaa !123
  %129 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %128, null
  br i1 %129, label %133, label %130

; <label>:130:                                    ; preds = %126
  %131 = getelementptr inbounds %"class.std::function", %"class.std::function"* %21, i64 6, i32 0, i32 0
  %132 = invoke zeroext i1 %128(%"union.std::_Any_data"* dereferenceable(16) %131, %"union.std::_Any_data"* dereferenceable(16) %131, i32 3)
          to label %133 unwind label %28

; <label>:133:                                    ; preds = %130, %126
  %134 = getelementptr inbounds %"class.std::function", %"class.std::function"* %21, i64 7, i32 0, i32 1
  %135 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %134, align 8, !tbaa !123
  %136 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %135, null
  br i1 %136, label %140, label %137

; <label>:137:                                    ; preds = %133
  %138 = getelementptr inbounds %"class.std::function", %"class.std::function"* %21, i64 7, i32 0, i32 0
  %139 = invoke zeroext i1 %135(%"union.std::_Any_data"* dereferenceable(16) %138, %"union.std::_Any_data"* dereferenceable(16) %138, i32 3)
          to label %140 unwind label %28

; <label>:140:                                    ; preds = %137, %133
  %141 = getelementptr inbounds %"class.std::function", %"class.std::function"* %21, i64 8, i32 0, i32 1
  %142 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %141, align 8, !tbaa !123
  %143 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %142, null
  br i1 %143, label %147, label %144

; <label>:144:                                    ; preds = %140
  %145 = getelementptr inbounds %"class.std::function", %"class.std::function"* %21, i64 8, i32 0, i32 0
  %146 = invoke zeroext i1 %142(%"union.std::_Any_data"* dereferenceable(16) %145, %"union.std::_Any_data"* dereferenceable(16) %145, i32 3)
          to label %147 unwind label %28

; <label>:147:                                    ; preds = %144, %140
  %148 = getelementptr inbounds %"class.std::function", %"class.std::function"* %21, i64 9, i32 0, i32 1
  %149 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %148, align 8, !tbaa !123
  %150 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %149, null
  br i1 %150, label %154, label %151

; <label>:151:                                    ; preds = %147
  %152 = getelementptr inbounds %"class.std::function", %"class.std::function"* %21, i64 9, i32 0, i32 0
  %153 = invoke zeroext i1 %149(%"union.std::_Any_data"* dereferenceable(16) %152, %"union.std::_Any_data"* dereferenceable(16) %152, i32 3)
          to label %154 unwind label %28

; <label>:154:                                    ; preds = %151, %147
  %155 = getelementptr inbounds %"class.std::function", %"class.std::function"* %21, i64 10, i32 0, i32 1
  %156 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %155, align 8, !tbaa !123
  %157 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %156, null
  br i1 %157, label %161, label %158

; <label>:158:                                    ; preds = %154
  %159 = getelementptr inbounds %"class.std::function", %"class.std::function"* %21, i64 10, i32 0, i32 0
  %160 = invoke zeroext i1 %156(%"union.std::_Any_data"* dereferenceable(16) %159, %"union.std::_Any_data"* dereferenceable(16) %159, i32 3)
          to label %161 unwind label %28

; <label>:161:                                    ; preds = %158, %154
  %162 = getelementptr inbounds %"class.std::function", %"class.std::function"* %21, i64 11, i32 0, i32 1
  %163 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %162, align 8, !tbaa !123
  %164 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %163, null
  br i1 %164, label %168, label %165

; <label>:165:                                    ; preds = %161
  %166 = getelementptr inbounds %"class.std::function", %"class.std::function"* %21, i64 11, i32 0, i32 0
  %167 = invoke zeroext i1 %163(%"union.std::_Any_data"* dereferenceable(16) %166, %"union.std::_Any_data"* dereferenceable(16) %166, i32 3)
          to label %168 unwind label %28

; <label>:168:                                    ; preds = %165, %161
  %169 = getelementptr inbounds %"class.std::function", %"class.std::function"* %21, i64 12, i32 0, i32 1
  %170 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %169, align 8, !tbaa !123
  %171 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %170, null
  br i1 %171, label %175, label %172

; <label>:172:                                    ; preds = %168
  %173 = getelementptr inbounds %"class.std::function", %"class.std::function"* %21, i64 12, i32 0, i32 0
  %174 = invoke zeroext i1 %170(%"union.std::_Any_data"* dereferenceable(16) %173, %"union.std::_Any_data"* dereferenceable(16) %173, i32 3)
          to label %175 unwind label %28

; <label>:175:                                    ; preds = %172, %168
  %176 = getelementptr inbounds %"class.std::function", %"class.std::function"* %21, i64 13, i32 0, i32 1
  %177 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %176, align 8, !tbaa !123
  %178 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %177, null
  br i1 %178, label %182, label %179

; <label>:179:                                    ; preds = %175
  %180 = getelementptr inbounds %"class.std::function", %"class.std::function"* %21, i64 13, i32 0, i32 0
  %181 = invoke zeroext i1 %177(%"union.std::_Any_data"* dereferenceable(16) %180, %"union.std::_Any_data"* dereferenceable(16) %180, i32 3)
          to label %182 unwind label %28

; <label>:182:                                    ; preds = %179, %175
  %183 = getelementptr inbounds %"class.std::function", %"class.std::function"* %21, i64 14, i32 0, i32 1
  %184 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %183, align 8, !tbaa !123
  %185 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %184, null
  br i1 %185, label %189, label %186

; <label>:186:                                    ; preds = %182
  %187 = getelementptr inbounds %"class.std::function", %"class.std::function"* %21, i64 14, i32 0, i32 0
  %188 = invoke zeroext i1 %184(%"union.std::_Any_data"* dereferenceable(16) %187, %"union.std::_Any_data"* dereferenceable(16) %187, i32 3)
          to label %189 unwind label %28

; <label>:189:                                    ; preds = %186, %182
  %190 = getelementptr inbounds %"class.std::function", %"class.std::function"* %21, i64 15, i32 0, i32 1
  %191 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %190, align 8, !tbaa !123
  %192 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %191, null
  br i1 %192, label %196, label %193

; <label>:193:                                    ; preds = %189
  %194 = getelementptr inbounds %"class.std::function", %"class.std::function"* %21, i64 15, i32 0, i32 0
  %195 = invoke zeroext i1 %191(%"union.std::_Any_data"* dereferenceable(16) %194, %"union.std::_Any_data"* dereferenceable(16) %194, i32 3)
          to label %196 unwind label %28

; <label>:196:                                    ; preds = %193, %189
  %197 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %20, i64 1
  %198 = load %"class.std::function"**, %"class.std::function"*** %7, align 8, !tbaa !207
  %199 = icmp ult %"class.std::function"** %197, %198
  br i1 %199, label %19, label %11
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZNSt11_Deque_baseISt8functionIFvvEESaIS2_EED2Ev(%"class.std::_Deque_base.48"*) unnamed_addr #10 comdat align 2 personality i32 (...)* @__gxx_personality_v0 {
  %2 = getelementptr inbounds %"class.std::_Deque_base.48", %"class.std::_Deque_base.48"* %0, i64 0, i32 0, i32 0
  %3 = load %"class.std::function"**, %"class.std::function"*** %2, align 8, !tbaa !228
  %4 = icmp eq %"class.std::function"** %3, null
  br i1 %4, label %25, label %5

; <label>:5:                                      ; preds = %1
  %6 = bitcast %"class.std::function"** %3 to i8*
  %7 = getelementptr inbounds %"class.std::_Deque_base.48", %"class.std::_Deque_base.48"* %0, i64 0, i32 0, i32 2, i32 3
  %8 = load %"class.std::function"**, %"class.std::function"*** %7, align 8, !tbaa !231
  %9 = getelementptr inbounds %"class.std::_Deque_base.48", %"class.std::_Deque_base.48"* %0, i64 0, i32 0, i32 3, i32 3
  %10 = load %"class.std::function"**, %"class.std::function"*** %9, align 8, !tbaa !232
  %11 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %10, i64 1
  %12 = icmp ult %"class.std::function"** %8, %11
  br i1 %12, label %13, label %23

; <label>:13:                                     ; preds = %5
  br label %14

; <label>:14:                                     ; preds = %13, %14
  %15 = phi %"class.std::function"** [ %18, %14 ], [ %8, %13 ]
  %16 = bitcast %"class.std::function"** %15 to i8**
  %17 = load i8*, i8** %16, align 8, !tbaa !27
  tail call void @_ZdlPv(i8* %17) #8
  %18 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %15, i64 1
  %19 = icmp ult %"class.std::function"** %15, %10
  br i1 %19, label %14, label %20

; <label>:20:                                     ; preds = %14
  %21 = bitcast %"class.std::_Deque_base.48"* %0 to i8**
  %22 = load i8*, i8** %21, align 8, !tbaa !228
  br label %23

; <label>:23:                                     ; preds = %5, %20
  %24 = phi i8* [ %22, %20 ], [ %6, %5 ]
  tail call void @_ZdlPv(i8* %24) #8
  br label %25

; <label>:25:                                     ; preds = %1, %23
  ret void
}

; Function Attrs: uwtable
define linkonce_odr zeroext i1 @_ZN4MARC15ThreadSafeQueueISt8functionIFvvEEE7waitPopERS3_(%"class.MARC::ThreadSafeQueue.45"*, %"class.std::function"* dereferenceable(32)) local_unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %3 = alloca %"class.std::unique_lock", align 8
  %4 = bitcast %"class.std::unique_lock"* %3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %4) #8
  %5 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 1
  %6 = getelementptr inbounds %"class.std::unique_lock", %"class.std::unique_lock"* %3, i64 0, i32 0
  store %"class.std::mutex"* %5, %"class.std::mutex"** %6, align 8, !tbaa !32
  %7 = getelementptr inbounds %"class.std::unique_lock", %"class.std::unique_lock"* %3, i64 0, i32 1
  store i8 0, i8* %7, align 8, !tbaa !35
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %8, label %13

; <label>:8:                                      ; preds = %2
  %9 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %5, i64 0, i32 0, i32 0
  %10 = tail call i32 @pthread_mutex_lock(%union.pthread_mutex_t* nonnull %9) #8
  %11 = icmp eq i32 %10, 0
  br i1 %11, label %13, label %12

; <label>:12:                                     ; preds = %8
  tail call void @_ZSt20__throw_system_errori(i32 %10) #23
  unreachable

; <label>:13:                                     ; preds = %2, %8
  store i8 1, i8* %7, align 8, !tbaa !35
  %14 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 0, i32 0, i32 0
  %15 = load atomic i8, i8* %14 seq_cst, align 1
  %16 = and i8 %15, 1
  %17 = icmp eq i8 %16, 0
  br i1 %17, label %52, label %18

; <label>:18:                                     ; preds = %13
  %19 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 0
  %20 = load %"class.std::function"*, %"class.std::function"** %19, align 8, !tbaa !209
  %21 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 0
  %22 = load %"class.std::function"*, %"class.std::function"** %21, align 8, !tbaa !209
  %23 = icmp eq %"class.std::function"* %20, %22
  br i1 %23, label %24, label %47

; <label>:24:                                     ; preds = %18
  %25 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 3
  br label %26

; <label>:26:                                     ; preds = %30, %24
  %27 = load atomic i8, i8* %14 seq_cst, align 1
  %28 = and i8 %27, 1
  %29 = icmp eq i8 %28, 0
  br i1 %29, label %47, label %30

; <label>:30:                                     ; preds = %26
  call void @_ZNSt18condition_variable4waitERSt11unique_lockISt5mutexE(%"class.std::condition_variable"* nonnull %25, %"class.std::unique_lock"* nonnull dereferenceable(16) %3) #8
  %31 = load %"class.std::function"*, %"class.std::function"** %19, align 8, !tbaa !209
  %32 = load %"class.std::function"*, %"class.std::function"** %21, align 8, !tbaa !209
  %33 = icmp eq %"class.std::function"* %31, %32
  br i1 %33, label %26, label %47

; <label>:34:                                     ; preds = %51
  %35 = landingpad { i8*, i32 }
          cleanup
  %36 = load i8, i8* %7, align 8, !tbaa !35, !range !43
  %37 = icmp eq i8 %36, 0
  br i1 %37, label %46, label %38

; <label>:38:                                     ; preds = %34
  %39 = load %"class.std::mutex"*, %"class.std::mutex"** %6, align 8, !tbaa !32
  %40 = icmp eq %"class.std::mutex"* %39, null
  br i1 %40, label %46, label %41

; <label>:41:                                     ; preds = %38
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %42, label %45

; <label>:42:                                     ; preds = %41
  %43 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %39, i64 0, i32 0, i32 0
  %44 = call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %43) #8
  br label %45

; <label>:45:                                     ; preds = %42, %41
  store i8 0, i8* %7, align 8, !tbaa !35
  br label %46

; <label>:46:                                     ; preds = %34, %38, %45
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %4) #8
  resume { i8*, i32 } %35

; <label>:47:                                     ; preds = %30, %26, %18
  %48 = load atomic i8, i8* %14 seq_cst, align 1
  %49 = and i8 %48, 1
  %50 = icmp eq i8 %49, 0
  br i1 %50, label %52, label %51

; <label>:51:                                     ; preds = %47
  invoke void @_ZN4MARC15ThreadSafeQueueISt8functionIFvvEEE12internal_popERS3_(%"class.MARC::ThreadSafeQueue.45"* nonnull %0, %"class.std::function"* nonnull dereferenceable(32) %1)
          to label %52 unwind label %34

; <label>:52:                                     ; preds = %47, %13, %51
  %53 = phi i1 [ false, %13 ], [ false, %47 ], [ true, %51 ]
  %54 = load i8, i8* %7, align 8, !tbaa !35, !range !43
  %55 = icmp eq i8 %54, 0
  br i1 %55, label %64, label %56

; <label>:56:                                     ; preds = %52
  %57 = load %"class.std::mutex"*, %"class.std::mutex"** %6, align 8, !tbaa !32
  %58 = icmp eq %"class.std::mutex"* %57, null
  br i1 %58, label %64, label %59

; <label>:59:                                     ; preds = %56
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %60, label %63

; <label>:60:                                     ; preds = %59
  %61 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %57, i64 0, i32 0, i32 0
  %62 = call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %61) #8
  br label %63

; <label>:63:                                     ; preds = %60, %59
  store i8 0, i8* %7, align 8, !tbaa !35
  br label %64

; <label>:64:                                     ; preds = %52, %56, %63
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %4) #8
  ret i1 %53
}

; Function Attrs: noreturn
declare void @_ZSt25__throw_bad_function_callv() local_unnamed_addr #11

declare void @_ZNSt6thread4joinEv(%"class.std::thread"*) local_unnamed_addr #6

; Function Attrs: nobuiltin nounwind
declare void @_ZdaPv(i8*) local_unnamed_addr #13

; Function Attrs: uwtable
define linkonce_odr void @_ZN4MARC15ThreadSafeQueueISt8functionIFvvEEE12internal_popERS3_(%"class.MARC::ThreadSafeQueue.45"*, %"class.std::function"* dereferenceable(32)) local_unnamed_addr #5 comdat align 2 personality i32 (...)* @__gxx_personality_v0 {
  %3 = alloca { i64, i64 }, align 8
  %4 = alloca %"class.std::function", align 8
  %5 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 0
  %6 = load %"class.std::function"*, %"class.std::function"** %5, align 8, !tbaa !209, !noalias !233
  %7 = bitcast %"class.std::function"* %4 to i8*
  call void @llvm.lifetime.start.p0i8(i64 32, i8* nonnull %7) #8
  %8 = getelementptr inbounds %"class.std::function", %"class.std::function"* %4, i64 0, i32 0, i32 1
  %9 = bitcast %"class.std::function"* %6 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %7, i8* nonnull %9, i64 16, i32 8, i1 false) #8
  %10 = getelementptr inbounds %"class.std::function", %"class.std::function"* %6, i64 0, i32 0, i32 1
  %11 = bitcast i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %10 to i64*
  %12 = bitcast i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %10 to <2 x i64>*
  %13 = load <2 x i64>, <2 x i64>* %12, align 8, !tbaa !27
  store i64 0, i64* %11, align 8, !tbaa !27
  %14 = bitcast { i64, i64 }* %3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %14)
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %14, i8* nonnull %9, i64 16, i32 8, i1 false)
  %15 = bitcast %"class.std::function"* %1 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %7, i8* nonnull %15, i64 16, i32 8, i1 false) #8, !tbaa.struct !236
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %15, i8* nonnull %14, i64 16, i32 8, i1 false) #8
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %14)
  %16 = getelementptr inbounds %"class.std::function", %"class.std::function"* %1, i64 0, i32 0, i32 1
  %17 = bitcast i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %16 to <2 x i64>*
  %18 = load <2 x i64>, <2 x i64>* %17, align 8, !tbaa !27
  %19 = bitcast i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %8 to <2 x i64>*
  store <2 x i64> %18, <2 x i64>* %19, align 8, !tbaa !27
  %20 = bitcast i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %16 to <2 x i64>*
  store <2 x i64> %13, <2 x i64>* %20, align 8, !tbaa !27
  %21 = extractelement <2 x i64> %18, i32 0
  %22 = icmp eq i64 %21, 0
  br i1 %22, label %30, label %23

; <label>:23:                                     ; preds = %2
  %24 = inttoptr i64 %21 to i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*
  %25 = getelementptr inbounds %"class.std::function", %"class.std::function"* %4, i64 0, i32 0, i32 0
  %26 = invoke zeroext i1 %24(%"union.std::_Any_data"* nonnull dereferenceable(16) %25, %"union.std::_Any_data"* nonnull dereferenceable(16) %25, i32 3)
          to label %30 unwind label %27

; <label>:27:                                     ; preds = %23
  %28 = landingpad { i8*, i32 }
          catch i8* null
  %29 = extractvalue { i8*, i32 } %28, 0
  call void @__clang_call_terminate(i8* %29) #22
  unreachable

; <label>:30:                                     ; preds = %2, %23
  call void @llvm.lifetime.end.p0i8(i64 32, i8* nonnull %7) #8
  %31 = load %"class.std::function"*, %"class.std::function"** %5, align 8, !tbaa !237
  %32 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 2
  %33 = load %"class.std::function"*, %"class.std::function"** %32, align 8, !tbaa !238
  %34 = getelementptr inbounds %"class.std::function", %"class.std::function"* %33, i64 -1
  %35 = icmp eq %"class.std::function"* %31, %34
  br i1 %35, label %51, label %36

; <label>:36:                                     ; preds = %30
  %37 = getelementptr inbounds %"class.std::function", %"class.std::function"* %31, i64 0, i32 0, i32 1
  %38 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %37, align 8, !tbaa !123
  %39 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %38, null
  br i1 %39, label %48, label %40

; <label>:40:                                     ; preds = %36
  %41 = getelementptr inbounds %"class.std::function", %"class.std::function"* %31, i64 0, i32 0, i32 0
  %42 = invoke zeroext i1 %38(%"union.std::_Any_data"* dereferenceable(16) %41, %"union.std::_Any_data"* dereferenceable(16) %41, i32 3)
          to label %43 unwind label %45

; <label>:43:                                     ; preds = %40
  %44 = load %"class.std::function"*, %"class.std::function"** %5, align 8, !tbaa !237
  br label %48

; <label>:45:                                     ; preds = %40
  %46 = landingpad { i8*, i32 }
          catch i8* null
  %47 = extractvalue { i8*, i32 } %46, 0
  call void @__clang_call_terminate(i8* %47) #22
  unreachable

; <label>:48:                                     ; preds = %43, %36
  %49 = phi %"class.std::function"* [ %44, %43 ], [ %31, %36 ]
  %50 = getelementptr inbounds %"class.std::function", %"class.std::function"* %49, i64 1
  store %"class.std::function"* %50, %"class.std::function"** %5, align 8, !tbaa !237
  br label %73

; <label>:51:                                     ; preds = %30
  %52 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2
  %53 = getelementptr inbounds %"class.std::function", %"class.std::function"* %31, i64 0, i32 0, i32 1
  %54 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %53, align 8, !tbaa !123
  %55 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %54, null
  br i1 %55, label %62, label %56

; <label>:56:                                     ; preds = %51
  %57 = getelementptr inbounds %"class.std::function", %"class.std::function"* %31, i64 0, i32 0, i32 0
  %58 = invoke zeroext i1 %54(%"union.std::_Any_data"* dereferenceable(16) %57, %"union.std::_Any_data"* dereferenceable(16) %57, i32 3)
          to label %62 unwind label %59

; <label>:59:                                     ; preds = %56
  %60 = landingpad { i8*, i32 }
          catch i8* null
  %61 = extractvalue { i8*, i32 } %60, 0
  call void @__clang_call_terminate(i8* %61) #22
  unreachable

; <label>:62:                                     ; preds = %56, %51
  %63 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 1
  %64 = bitcast %"class.std::function"** %63 to i8**
  %65 = load i8*, i8** %64, align 8, !tbaa !239
  call void @_ZdlPv(i8* %65) #8
  %66 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 3
  %67 = load %"class.std::function"**, %"class.std::function"*** %66, align 8, !tbaa !231
  %68 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %67, i64 1
  store %"class.std::function"** %68, %"class.std::function"*** %66, align 8, !tbaa !207
  %69 = load %"class.std::function"*, %"class.std::function"** %68, align 8, !tbaa !27
  store %"class.std::function"* %69, %"class.std::function"** %63, align 8, !tbaa !210
  %70 = getelementptr inbounds %"class.std::function", %"class.std::function"* %69, i64 16
  store %"class.std::function"* %70, %"class.std::function"** %32, align 8, !tbaa !211
  %71 = ptrtoint %"class.std::function"* %69 to i64
  %72 = bitcast %"struct.std::_Deque_iterator.52"* %52 to i64*
  store i64 %71, i64* %72, align 8, !tbaa !237
  br label %73

; <label>:73:                                     ; preds = %48, %62
  %74 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 4
  call void @_ZNSt18condition_variable10notify_oneEv(%"class.std::condition_variable"* %74) #8
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i32, i1) #9

; Function Attrs: uwtable
define linkonce_odr dereferenceable(16) %"struct.std::__future_base::_Result"* @_ZNKSt14__basic_futureIvE13_M_get_resultEv(%"class.std::__basic_future"*) local_unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %2 = alloca %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider", align 8
  %3 = alloca %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider", align 8
  %4 = getelementptr inbounds %"class.std::__basic_future", %"class.std::__basic_future"* %0, i64 0, i32 0, i32 0, i32 0
  %5 = load %"class.std::__future_base::_State_baseV2"*, %"class.std::__future_base::_State_baseV2"** %4, align 8, !tbaa !131
  %6 = icmp eq %"class.std::__future_base::_State_baseV2"* %5, null
  br i1 %6, label %7, label %8

; <label>:7:                                      ; preds = %1
  tail call void @_ZSt20__throw_future_errori(i32 3) #23
  unreachable

; <label>:8:                                      ; preds = %1
  %9 = bitcast %"class.std::__future_base::_State_baseV2"* %5 to void (%"class.std::__future_base::_State_baseV2"*)***
  %10 = load void (%"class.std::__future_base::_State_baseV2"*)**, void (%"class.std::__future_base::_State_baseV2"*)*** %9, align 8, !tbaa !133
  %11 = getelementptr inbounds void (%"class.std::__future_base::_State_baseV2"*)*, void (%"class.std::__future_base::_State_baseV2"*)** %10, i64 2
  %12 = load void (%"class.std::__future_base::_State_baseV2"*)*, void (%"class.std::__future_base::_State_baseV2"*)** %11, align 8
  tail call void %12(%"class.std::__future_base::_State_baseV2"* nonnull %5)
  %13 = getelementptr inbounds %"class.std::__future_base::_State_baseV2", %"class.std::__future_base::_State_baseV2"* %5, i64 0, i32 2
  %14 = getelementptr inbounds %"class.std::__atomic_futex_unsigned", %"class.std::__atomic_futex_unsigned"* %13, i64 0, i32 0, i32 0, i32 0
  %15 = load atomic i32, i32* %14 acquire, align 4
  %16 = and i32 %15, 2147483647
  %17 = icmp eq i32 %16, 1
  br i1 %17, label %29, label %18

; <label>:18:                                     ; preds = %8
  %19 = bitcast %"class.std::__atomic_futex_unsigned"* %13 to %"class.std::ios_base::Init"*
  br label %20

; <label>:20:                                     ; preds = %20, %18
  %21 = phi i32 [ %16, %18 ], [ %26, %20 ]
  %22 = atomicrmw or i32* %14, i32 -2147483648 monotonic
  %23 = or i32 %21, -2147483648
  %24 = tail call zeroext i1 @_ZNSt28__atomic_futex_unsigned_base19_M_futex_wait_untilEPjjbNSt6chrono8durationIlSt5ratioILl1ELl1EEEENS2_IlS3_ILl1ELl1000000000EEEE(%"class.std::ios_base::Init"* %19, i32* %14, i32 %23, i1 zeroext false, i64 0, i64 0)
  %25 = load atomic i32, i32* %14 acquire, align 4
  %26 = and i32 %25, 2147483647
  %27 = icmp ne i32 %26, 1
  %28 = and i1 %24, %27
  br i1 %28, label %20, label %29

; <label>:29:                                     ; preds = %20, %8
  %30 = getelementptr inbounds %"class.std::__future_base::_State_baseV2", %"class.std::__future_base::_State_baseV2"* %5, i64 0, i32 1, i32 0, i32 0, i32 0, i32 0
  %31 = load %"struct.std::__future_base::_Result_base"*, %"struct.std::__future_base::_Result_base"** %30, align 8, !tbaa !27
  %32 = getelementptr inbounds %"struct.std::__future_base::_Result_base", %"struct.std::__future_base::_Result_base"* %31, i64 0, i32 1
  %33 = bitcast %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* %2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %33) #8
  %34 = getelementptr inbounds %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider", %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* %2, i64 0, i32 0
  store i8* null, i8** %34, align 8, !tbaa !240
  %35 = call zeroext i1 @_ZNSt15__exception_ptreqERKNS_13exception_ptrES2_(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* dereferenceable(8) %32, %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* nonnull dereferenceable(8) %2) #26
  call void @_ZNSt15__exception_ptr13exception_ptrD1Ev(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* nonnull %2) #8
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %33) #8
  br i1 %35, label %40, label %36

; <label>:36:                                     ; preds = %29
  call void @_ZNSt15__exception_ptr13exception_ptrC1ERKS0_(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* nonnull %3, %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* nonnull dereferenceable(8) %32) #8
  invoke void @_ZSt17rethrow_exceptionNSt15__exception_ptr13exception_ptrE(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* nonnull %3) #23
          to label %37 unwind label %38

; <label>:37:                                     ; preds = %36
  unreachable

; <label>:38:                                     ; preds = %36
  %39 = landingpad { i8*, i32 }
          cleanup
  call void @_ZNSt15__exception_ptr13exception_ptrD1Ev(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* nonnull %3) #8
  resume { i8*, i32 } %39

; <label>:40:                                     ; preds = %29
  %41 = bitcast %"struct.std::__future_base::_Result_base"* %31 to %"struct.std::__future_base::_Result"*
  ret %"struct.std::__future_base::_Result"* %41
}

; Function Attrs: noreturn
declare void @_ZSt20__throw_future_errori(i32) local_unnamed_addr #11

declare zeroext i1 @_ZNSt28__atomic_futex_unsigned_base19_M_futex_wait_untilEPjjbNSt6chrono8durationIlSt5ratioILl1ELl1EEEENS2_IlS3_ILl1ELl1000000000EEEE(%"class.std::ios_base::Init"*, i32*, i32, i1 zeroext, i64, i64) local_unnamed_addr #6

; Function Attrs: nounwind readonly
declare zeroext i1 @_ZNSt15__exception_ptreqERKNS_13exception_ptrES2_(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* dereferenceable(8), %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* dereferenceable(8)) local_unnamed_addr #18

; Function Attrs: nounwind
declare void @_ZNSt15__exception_ptr13exception_ptrD1Ev(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"*) unnamed_addr #7

; Function Attrs: nounwind
declare void @_ZNSt15__exception_ptr13exception_ptrC1ERKS0_(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"*, %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* dereferenceable(8)) unnamed_addr #7

; Function Attrs: noreturn
declare void @_ZSt17rethrow_exceptionNSt15__exception_ptr13exception_ptrE(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"*) local_unnamed_addr #11

declare void @_ZNSt13__future_base12_Result_baseC2Ev(%"struct.std::__future_base::_Result_base"*) unnamed_addr #6

; Function Attrs: inlinehint norecurse nounwind uwtable
define linkonce_odr void @_ZSt32__enable_shared_from_this_helperILN9__gnu_cxx12_Lock_policyE2EEvRKSt14__shared_countIXT_EEz(%"class.std::__shared_count"* dereferenceable(8), ...) local_unnamed_addr #19 comdat {
  ret void
}

; Function Attrs: uwtable
define linkonce_odr void @_ZNSt13packaged_taskIFvvEE10get_futureEv(%"class.std::future"* noalias sret, %"class.std::packaged_task"*) local_unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %3 = bitcast %"class.std::packaged_task"* %1 to i64*
  %4 = load i64, i64* %3, align 8, !tbaa !242
  %5 = getelementptr inbounds %"class.std::packaged_task", %"class.std::packaged_task"* %1, i64 0, i32 0, i32 0, i32 1, i32 0
  %6 = load %"class.std::_Sp_counted_base"*, %"class.std::_Sp_counted_base"** %5, align 8, !tbaa !129
  %7 = icmp eq %"class.std::_Sp_counted_base"* %6, null
  br i1 %7, label %15, label %8

; <label>:8:                                      ; preds = %2
  %9 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %6, i64 0, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %10, label %12

; <label>:10:                                     ; preds = %8
  %11 = atomicrmw volatile add i32* %9, i32 1 acq_rel
  br label %19

; <label>:12:                                     ; preds = %8
  %13 = load i32, i32* %9, align 4, !tbaa !17
  %14 = add nsw i32 %13, 1
  store i32 %14, i32* %9, align 4, !tbaa !17
  br label %19

; <label>:15:                                     ; preds = %2
  %16 = bitcast %"class.std::future"* %0 to i64*
  store i64 %4, i64* %16, align 8, !tbaa !131
  %17 = getelementptr inbounds %"class.std::future", %"class.std::future"* %0, i64 0, i32 0, i32 0, i32 0, i32 1, i32 0
  store %"class.std::_Sp_counted_base"* null, %"class.std::_Sp_counted_base"** %17, align 8, !tbaa !129
  %18 = inttoptr i64 %4 to %"class.std::__future_base::_State_baseV2"*
  br label %30

; <label>:19:                                     ; preds = %10, %12
  %20 = bitcast %"class.std::future"* %0 to i64*
  store i64 %4, i64* %20, align 8, !tbaa !131
  %21 = getelementptr inbounds %"class.std::future", %"class.std::future"* %0, i64 0, i32 0, i32 0, i32 0, i32 1, i32 0
  store %"class.std::_Sp_counted_base"* %6, %"class.std::_Sp_counted_base"** %21, align 8, !tbaa !129
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %22, label %26

; <label>:22:                                     ; preds = %19
  %23 = atomicrmw volatile add i32* %9, i32 1 acq_rel
  %24 = getelementptr inbounds %"class.std::future", %"class.std::future"* %0, i64 0, i32 0, i32 0, i32 0, i32 0
  %25 = load %"class.std::__future_base::_State_baseV2"*, %"class.std::__future_base::_State_baseV2"** %24, align 8, !tbaa !131
  br label %30

; <label>:26:                                     ; preds = %19
  %27 = inttoptr i64 %4 to %"class.std::__future_base::_State_baseV2"*
  %28 = load i32, i32* %9, align 4, !tbaa !17
  %29 = add nsw i32 %28, 1
  store i32 %29, i32* %9, align 4, !tbaa !17
  br label %30

; <label>:30:                                     ; preds = %15, %26, %22
  %31 = phi %"class.std::_Sp_counted_base"** [ %17, %15 ], [ %21, %22 ], [ %21, %26 ]
  %32 = phi %"class.std::__future_base::_State_baseV2"* [ %18, %15 ], [ %25, %22 ], [ %27, %26 ]
  %33 = icmp eq %"class.std::__future_base::_State_baseV2"* %32, null
  br i1 %33, label %34, label %36

; <label>:34:                                     ; preds = %30
  invoke void @_ZSt20__throw_future_errori(i32 3) #23
          to label %35 unwind label %42

; <label>:35:                                     ; preds = %34
  unreachable

; <label>:36:                                     ; preds = %30
  %37 = getelementptr inbounds %"class.std::__future_base::_State_baseV2", %"class.std::__future_base::_State_baseV2"* %32, i64 0, i32 3, i32 0, i32 0
  %38 = atomicrmw xchg i8* %37, i8 1 seq_cst
  %39 = icmp eq i8 %38, 0
  br i1 %39, label %74, label %40

; <label>:40:                                     ; preds = %36
  invoke void @_ZSt20__throw_future_errori(i32 1) #23
          to label %41 unwind label %42

; <label>:41:                                     ; preds = %40
  unreachable

; <label>:42:                                     ; preds = %40, %34
  %43 = landingpad { i8*, i32 }
          cleanup
  %44 = load %"class.std::_Sp_counted_base"*, %"class.std::_Sp_counted_base"** %31, align 8, !tbaa !129
  %45 = icmp eq %"class.std::_Sp_counted_base"* %44, null
  br i1 %45, label %104, label %46

; <label>:46:                                     ; preds = %42
  %47 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %44, i64 0, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %48, label %50

; <label>:48:                                     ; preds = %46
  %49 = atomicrmw volatile add i32* %47, i32 -1 acq_rel
  br label %53

; <label>:50:                                     ; preds = %46
  %51 = load i32, i32* %47, align 4, !tbaa !17
  %52 = add nsw i32 %51, -1
  store i32 %52, i32* %47, align 4, !tbaa !17
  br label %53

; <label>:53:                                     ; preds = %50, %48
  %54 = phi i32 [ %49, %48 ], [ %51, %50 ]
  %55 = icmp eq i32 %54, 1
  br i1 %55, label %56, label %104

; <label>:56:                                     ; preds = %53
  %57 = bitcast %"class.std::_Sp_counted_base"* %44 to void (%"class.std::_Sp_counted_base"*)***
  %58 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %57, align 8, !tbaa !133
  %59 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %58, i64 2
  %60 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %59, align 8
  tail call void %60(%"class.std::_Sp_counted_base"* nonnull %44) #8
  %61 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %44, i64 0, i32 2
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %62, label %64

; <label>:62:                                     ; preds = %56
  %63 = atomicrmw volatile add i32* %61, i32 -1 acq_rel
  br label %67

; <label>:64:                                     ; preds = %56
  %65 = load i32, i32* %61, align 4, !tbaa !17
  %66 = add nsw i32 %65, -1
  store i32 %66, i32* %61, align 4, !tbaa !17
  br label %67

; <label>:67:                                     ; preds = %64, %62
  %68 = phi i32 [ %63, %62 ], [ %65, %64 ]
  %69 = icmp eq i32 %68, 1
  br i1 %69, label %70, label %104

; <label>:70:                                     ; preds = %67
  %71 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %57, align 8, !tbaa !133
  %72 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %71, i64 3
  %73 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %72, align 8
  tail call void %73(%"class.std::_Sp_counted_base"* nonnull %44) #8
  br label %104

; <label>:74:                                     ; preds = %36
  br i1 %7, label %103, label %75

; <label>:75:                                     ; preds = %74
  %76 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %6, i64 0, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %77, label %79

; <label>:77:                                     ; preds = %75
  %78 = atomicrmw volatile add i32* %76, i32 -1 acq_rel
  br label %82

; <label>:79:                                     ; preds = %75
  %80 = load i32, i32* %76, align 4, !tbaa !17
  %81 = add nsw i32 %80, -1
  store i32 %81, i32* %76, align 4, !tbaa !17
  br label %82

; <label>:82:                                     ; preds = %79, %77
  %83 = phi i32 [ %78, %77 ], [ %80, %79 ]
  %84 = icmp eq i32 %83, 1
  br i1 %84, label %85, label %103

; <label>:85:                                     ; preds = %82
  %86 = bitcast %"class.std::_Sp_counted_base"* %6 to void (%"class.std::_Sp_counted_base"*)***
  %87 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %86, align 8, !tbaa !133
  %88 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %87, i64 2
  %89 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %88, align 8
  tail call void %89(%"class.std::_Sp_counted_base"* nonnull %6) #8
  %90 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %6, i64 0, i32 2
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %91, label %93

; <label>:91:                                     ; preds = %85
  %92 = atomicrmw volatile add i32* %90, i32 -1 acq_rel
  br label %96

; <label>:93:                                     ; preds = %85
  %94 = load i32, i32* %90, align 4, !tbaa !17
  %95 = add nsw i32 %94, -1
  store i32 %95, i32* %90, align 4, !tbaa !17
  br label %96

; <label>:96:                                     ; preds = %93, %91
  %97 = phi i32 [ %92, %91 ], [ %94, %93 ]
  %98 = icmp eq i32 %97, 1
  br i1 %98, label %99, label %103

; <label>:99:                                     ; preds = %96
  %100 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %86, align 8, !tbaa !133
  %101 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %100, i64 3
  %102 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %101, align 8
  tail call void %102(%"class.std::_Sp_counted_base"* nonnull %6) #8
  br label %103

; <label>:103:                                    ; preds = %74, %82, %96, %99
  ret void

; <label>:104:                                    ; preds = %42, %53, %67, %70
  br i1 %7, label %133, label %105

; <label>:105:                                    ; preds = %104
  %106 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %6, i64 0, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %107, label %109

; <label>:107:                                    ; preds = %105
  %108 = atomicrmw volatile add i32* %106, i32 -1 acq_rel
  br label %112

; <label>:109:                                    ; preds = %105
  %110 = load i32, i32* %106, align 4, !tbaa !17
  %111 = add nsw i32 %110, -1
  store i32 %111, i32* %106, align 4, !tbaa !17
  br label %112

; <label>:112:                                    ; preds = %109, %107
  %113 = phi i32 [ %108, %107 ], [ %110, %109 ]
  %114 = icmp eq i32 %113, 1
  br i1 %114, label %115, label %133

; <label>:115:                                    ; preds = %112
  %116 = bitcast %"class.std::_Sp_counted_base"* %6 to void (%"class.std::_Sp_counted_base"*)***
  %117 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %116, align 8, !tbaa !133
  %118 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %117, i64 2
  %119 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %118, align 8
  tail call void %119(%"class.std::_Sp_counted_base"* nonnull %6) #8
  %120 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %6, i64 0, i32 2
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %121, label %123

; <label>:121:                                    ; preds = %115
  %122 = atomicrmw volatile add i32* %120, i32 -1 acq_rel
  br label %126

; <label>:123:                                    ; preds = %115
  %124 = load i32, i32* %120, align 4, !tbaa !17
  %125 = add nsw i32 %124, -1
  store i32 %125, i32* %120, align 4, !tbaa !17
  br label %126

; <label>:126:                                    ; preds = %123, %121
  %127 = phi i32 [ %122, %121 ], [ %124, %123 ]
  %128 = icmp eq i32 %127, 1
  br i1 %128, label %129, label %133

; <label>:129:                                    ; preds = %126
  %130 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %116, align 8, !tbaa !133
  %131 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %130, i64 3
  %132 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %131, align 8
  tail call void %132(%"class.std::_Sp_counted_base"* nonnull %6) #8
  br label %133

; <label>:133:                                    ; preds = %104, %112, %126, %129
  resume { i8*, i32 } %43
}

; Function Attrs: uwtable
define linkonce_odr void @_ZNSt5dequeISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EESaIS5_EE17_M_reallocate_mapEmb(%"class.std::deque.34"*, i64, i1 zeroext) local_unnamed_addr #5 comdat align 2 personality i32 (...)* @__gxx_personality_v0 {
  %4 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %0, i64 0, i32 0, i32 0, i32 3, i32 3
  %5 = load %"class.std::unique_ptr"**, %"class.std::unique_ptr"*** %4, align 8, !tbaa !200
  %6 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %0, i64 0, i32 0, i32 0, i32 2, i32 3
  %7 = load %"class.std::unique_ptr"**, %"class.std::unique_ptr"*** %6, align 8, !tbaa !221
  %8 = ptrtoint %"class.std::unique_ptr"** %5 to i64
  %9 = ptrtoint %"class.std::unique_ptr"** %7 to i64
  %10 = sub i64 %8, %9
  %11 = ashr exact i64 %10, 3
  %12 = add nsw i64 %11, 1
  %13 = add i64 %12, %1
  %14 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %0, i64 0, i32 0, i32 0, i32 1
  %15 = load i64, i64* %14, align 8, !tbaa !199
  %16 = shl i64 %13, 1
  %17 = icmp ugt i64 %15, %16
  br i1 %17, label %18, label %44

; <label>:18:                                     ; preds = %3
  %19 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %0, i64 0, i32 0, i32 0, i32 0
  %20 = load %"class.std::unique_ptr"**, %"class.std::unique_ptr"*** %19, align 8, !tbaa !201
  %21 = sub i64 %15, %13
  %22 = lshr i64 %21, 1
  %23 = getelementptr inbounds %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %20, i64 %22
  %24 = select i1 %2, i64 %1, i64 0
  %25 = getelementptr inbounds %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %23, i64 %24
  %26 = icmp ult %"class.std::unique_ptr"** %25, %7
  %27 = getelementptr inbounds %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %5, i64 1
  %28 = ptrtoint %"class.std::unique_ptr"** %27 to i64
  %29 = sub i64 %28, %9
  br i1 %26, label %30, label %35

; <label>:30:                                     ; preds = %18
  %31 = icmp eq i64 %29, 0
  br i1 %31, label %73, label %32

; <label>:32:                                     ; preds = %30
  %33 = bitcast %"class.std::unique_ptr"** %25 to i8*
  %34 = bitcast %"class.std::unique_ptr"** %7 to i8*
  tail call void @llvm.memmove.p0i8.p0i8.i64(i8* %33, i8* %34, i64 %29, i32 8, i1 false) #8
  br label %73

; <label>:35:                                     ; preds = %18
  %36 = ashr exact i64 %29, 3
  %37 = icmp eq i64 %36, 0
  br i1 %37, label %73, label %38

; <label>:38:                                     ; preds = %35
  %39 = getelementptr inbounds %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %25, i64 %12
  %40 = sub nsw i64 0, %36
  %41 = getelementptr inbounds %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %39, i64 %40
  %42 = bitcast %"class.std::unique_ptr"** %41 to i8*
  %43 = bitcast %"class.std::unique_ptr"** %7 to i8*
  tail call void @llvm.memmove.p0i8.p0i8.i64(i8* %42, i8* %43, i64 %29, i32 8, i1 false) #8
  br label %73

; <label>:44:                                     ; preds = %3
  %45 = icmp ult i64 %15, %1
  %46 = select i1 %45, i64 %1, i64 %15
  %47 = add i64 %15, 2
  %48 = add i64 %47, %46
  %49 = icmp ugt i64 %48, 2305843009213693951
  br i1 %49, label %50, label %51

; <label>:50:                                     ; preds = %44
  tail call void @_ZSt17__throw_bad_allocv() #23
  unreachable

; <label>:51:                                     ; preds = %44
  %52 = shl i64 %48, 3
  %53 = tail call i8* @_Znwm(i64 %52)
  %54 = bitcast i8* %53 to %"class.std::unique_ptr"**
  %55 = sub i64 %48, %13
  %56 = lshr i64 %55, 1
  %57 = getelementptr inbounds %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %54, i64 %56
  %58 = select i1 %2, i64 %1, i64 0
  %59 = getelementptr inbounds %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %57, i64 %58
  %60 = load %"class.std::unique_ptr"**, %"class.std::unique_ptr"*** %6, align 8, !tbaa !221
  %61 = load %"class.std::unique_ptr"**, %"class.std::unique_ptr"*** %4, align 8, !tbaa !200
  %62 = getelementptr inbounds %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %61, i64 1
  %63 = ptrtoint %"class.std::unique_ptr"** %62 to i64
  %64 = ptrtoint %"class.std::unique_ptr"** %60 to i64
  %65 = sub i64 %63, %64
  %66 = icmp eq i64 %65, 0
  br i1 %66, label %70, label %67

; <label>:67:                                     ; preds = %51
  %68 = bitcast %"class.std::unique_ptr"** %59 to i8*
  %69 = bitcast %"class.std::unique_ptr"** %60 to i8*
  tail call void @llvm.memmove.p0i8.p0i8.i64(i8* %68, i8* %69, i64 %65, i32 8, i1 false) #8
  br label %70

; <label>:70:                                     ; preds = %51, %67
  %71 = bitcast %"class.std::deque.34"* %0 to i8**
  %72 = load i8*, i8** %71, align 8, !tbaa !201
  tail call void @_ZdlPv(i8* %72) #8
  store i8* %53, i8** %71, align 8, !tbaa !201
  store i64 %48, i64* %14, align 8, !tbaa !199
  br label %73

; <label>:73:                                     ; preds = %38, %35, %32, %30, %70
  %74 = phi %"class.std::unique_ptr"** [ %59, %70 ], [ %25, %30 ], [ %25, %32 ], [ %25, %35 ], [ %25, %38 ]
  store %"class.std::unique_ptr"** %74, %"class.std::unique_ptr"*** %6, align 8, !tbaa !202
  %75 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %74, align 8, !tbaa !27
  %76 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %0, i64 0, i32 0, i32 0, i32 2, i32 1
  store %"class.std::unique_ptr"* %75, %"class.std::unique_ptr"** %76, align 8, !tbaa !203
  %77 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %75, i64 64
  %78 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %0, i64 0, i32 0, i32 0, i32 2, i32 2
  store %"class.std::unique_ptr"* %77, %"class.std::unique_ptr"** %78, align 8, !tbaa !204
  %79 = getelementptr inbounds %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %74, i64 %11
  store %"class.std::unique_ptr"** %79, %"class.std::unique_ptr"*** %4, align 8, !tbaa !202
  %80 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %79, align 8, !tbaa !27
  %81 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %0, i64 0, i32 0, i32 0, i32 3, i32 1
  store %"class.std::unique_ptr"* %80, %"class.std::unique_ptr"** %81, align 8, !tbaa !203
  %82 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %80, i64 64
  %83 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %0, i64 0, i32 0, i32 0, i32 3, i32 2
  store %"class.std::unique_ptr"* %82, %"class.std::unique_ptr"** %83, align 8, !tbaa !204
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZNSt13packaged_taskIFvvEED2Ev(%"class.std::packaged_task"*) unnamed_addr #10 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %2 = alloca %"class.std::unique_ptr.58", align 8
  %3 = getelementptr inbounds %"class.std::packaged_task", %"class.std::packaged_task"* %0, i64 0, i32 0, i32 0
  %4 = getelementptr inbounds %"class.std::packaged_task", %"class.std::packaged_task"* %0, i64 0, i32 0, i32 0, i32 0
  %5 = load %"struct.std::__future_base::_Task_state_base"*, %"struct.std::__future_base::_Task_state_base"** %4, align 8, !tbaa !242
  %6 = icmp eq %"struct.std::__future_base::_Task_state_base"* %5, null
  %7 = getelementptr inbounds %"class.std::packaged_task", %"class.std::packaged_task"* %0, i64 0, i32 0, i32 0, i32 1, i32 0
  br i1 %6, label %37, label %8

; <label>:8:                                      ; preds = %1
  %9 = load %"class.std::_Sp_counted_base"*, %"class.std::_Sp_counted_base"** %7, align 8, !tbaa !129
  %10 = icmp eq %"class.std::_Sp_counted_base"* %9, null
  br i1 %10, label %15, label %11

; <label>:11:                                     ; preds = %8
  %12 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %9, i64 0, i32 1
  %13 = load atomic i32, i32* %12 monotonic, align 4
  %14 = icmp eq i32 %13, 1
  br i1 %14, label %37, label %15

; <label>:15:                                     ; preds = %8, %11
  %16 = bitcast %"struct.std::__future_base::_Task_state_base"* %5 to %"class.std::__future_base::_State_baseV2"*
  %17 = getelementptr inbounds %"struct.std::__future_base::_Task_state_base", %"struct.std::__future_base::_Task_state_base"* %5, i64 0, i32 1
  %18 = getelementptr inbounds %"class.std::unique_ptr.109", %"class.std::unique_ptr.109"* %17, i64 0, i32 0, i32 0, i32 0, i32 0
  %19 = bitcast %"class.std::unique_ptr.109"* %17 to i64*
  %20 = load i64, i64* %19, align 8, !tbaa !27
  store %"struct.std::__future_base::_Result"* null, %"struct.std::__future_base::_Result"** %18, align 8, !tbaa !27
  %21 = bitcast %"class.std::unique_ptr.58"* %2 to i64*
  store i64 %20, i64* %21, align 8, !tbaa !168
  invoke void @_ZNSt13__future_base13_State_baseV216_M_break_promiseESt10unique_ptrINS_12_Result_baseENS2_8_DeleterEE(%"class.std::__future_base::_State_baseV2"* %16, %"class.std::unique_ptr.58"* nonnull %2)
          to label %22 unwind label %34

; <label>:22:                                     ; preds = %15
  %23 = getelementptr inbounds %"class.std::unique_ptr.58", %"class.std::unique_ptr.58"* %2, i64 0, i32 0, i32 0, i32 0, i32 0
  %24 = load %"struct.std::__future_base::_Result_base"*, %"struct.std::__future_base::_Result_base"** %23, align 8, !tbaa !27
  %25 = icmp eq %"struct.std::__future_base::_Result_base"* %24, null
  br i1 %25, label %33, label %26

; <label>:26:                                     ; preds = %22
  %27 = bitcast %"struct.std::__future_base::_Result_base"* %24 to void (%"struct.std::__future_base::_Result_base"*)***
  %28 = load void (%"struct.std::__future_base::_Result_base"*)**, void (%"struct.std::__future_base::_Result_base"*)*** %27, align 8, !tbaa !133
  %29 = load void (%"struct.std::__future_base::_Result_base"*)*, void (%"struct.std::__future_base::_Result_base"*)** %28, align 8
  invoke void %29(%"struct.std::__future_base::_Result_base"* nonnull %24)
          to label %33 unwind label %30

; <label>:30:                                     ; preds = %26
  %31 = landingpad { i8*, i32 }
          catch i8* null
  %32 = extractvalue { i8*, i32 } %31, 0
  call void @__clang_call_terminate(i8* %32) #22
  unreachable

; <label>:33:                                     ; preds = %22, %26
  store %"struct.std::__future_base::_Result_base"* null, %"struct.std::__future_base::_Result_base"** %23, align 8, !tbaa !27
  br label %37

; <label>:34:                                     ; preds = %15
  %35 = landingpad { i8*, i32 }
          catch i8* null
  %36 = extractvalue { i8*, i32 } %35, 0
  call void @_ZNSt10unique_ptrINSt13__future_base12_Result_baseENS1_8_DeleterEED2Ev(%"class.std::unique_ptr.58"* nonnull %2) #8
  call void @_ZNSt12__shared_ptrINSt13__future_base16_Task_state_baseIFvvEEELN9__gnu_cxx12_Lock_policyE2EED2Ev(%"class.std::__shared_ptr.108"* nonnull %3) #8
  call void @__clang_call_terminate(i8* %36) #22
  unreachable

; <label>:37:                                     ; preds = %1, %33, %11
  %38 = load %"class.std::_Sp_counted_base"*, %"class.std::_Sp_counted_base"** %7, align 8, !tbaa !129
  %39 = icmp eq %"class.std::_Sp_counted_base"* %38, null
  br i1 %39, label %68, label %40

; <label>:40:                                     ; preds = %37
  %41 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %38, i64 0, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %42, label %44

; <label>:42:                                     ; preds = %40
  %43 = atomicrmw volatile add i32* %41, i32 -1 acq_rel
  br label %47

; <label>:44:                                     ; preds = %40
  %45 = load i32, i32* %41, align 4, !tbaa !17
  %46 = add nsw i32 %45, -1
  store i32 %46, i32* %41, align 4, !tbaa !17
  br label %47

; <label>:47:                                     ; preds = %44, %42
  %48 = phi i32 [ %43, %42 ], [ %45, %44 ]
  %49 = icmp eq i32 %48, 1
  br i1 %49, label %50, label %68

; <label>:50:                                     ; preds = %47
  %51 = bitcast %"class.std::_Sp_counted_base"* %38 to void (%"class.std::_Sp_counted_base"*)***
  %52 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %51, align 8, !tbaa !133
  %53 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %52, i64 2
  %54 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %53, align 8
  call void %54(%"class.std::_Sp_counted_base"* nonnull %38) #8
  %55 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %38, i64 0, i32 2
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %56, label %58

; <label>:56:                                     ; preds = %50
  %57 = atomicrmw volatile add i32* %55, i32 -1 acq_rel
  br label %61

; <label>:58:                                     ; preds = %50
  %59 = load i32, i32* %55, align 4, !tbaa !17
  %60 = add nsw i32 %59, -1
  store i32 %60, i32* %55, align 4, !tbaa !17
  br label %61

; <label>:61:                                     ; preds = %58, %56
  %62 = phi i32 [ %57, %56 ], [ %59, %58 ]
  %63 = icmp eq i32 %62, 1
  br i1 %63, label %64, label %68

; <label>:64:                                     ; preds = %61
  %65 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %51, align 8, !tbaa !133
  %66 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %65, i64 3
  %67 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %66, align 8
  call void %67(%"class.std::_Sp_counted_base"* nonnull %38) #8
  br label %68

; <label>:68:                                     ; preds = %37, %47, %61, %64
  ret void
}

; Function Attrs: uwtable
define linkonce_odr void @_ZNSt13__future_base13_State_baseV216_M_break_promiseESt10unique_ptrINS_12_Result_baseENS2_8_DeleterEE(%"class.std::__future_base::_State_baseV2"*, %"class.std::unique_ptr.58"*) local_unnamed_addr #5 comdat align 2 {
  %3 = alloca %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider", align 8
  %4 = alloca %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider", align 8
  %5 = alloca %"class.std::future_error", align 8
  %6 = getelementptr inbounds %"class.std::unique_ptr.58", %"class.std::unique_ptr.58"* %1, i64 0, i32 0, i32 0, i32 0, i32 0
  %7 = load %"struct.std::__future_base::_Result_base"*, %"struct.std::__future_base::_Result_base"** %6, align 8, !tbaa !27
  %8 = icmp eq %"struct.std::__future_base::_Result_base"* %7, null
  br i1 %8, label %28, label %9

; <label>:9:                                      ; preds = %2
  %10 = tail call dereferenceable(8) %"class.MARC::IThreadTask"* @_ZSt15future_categoryv() #8
  %11 = bitcast %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* %4 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %11) #8
  call void @_ZNSt12future_errorC2ESt10error_code(%"class.std::future_error"* nonnull %5, i32 4, %"class.MARC::IThreadTask"* nonnull %10)
  call void @_ZSt18make_exception_ptrISt12future_errorENSt15__exception_ptr13exception_ptrET_(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* nonnull sret %4, %"class.std::future_error"* nonnull %5) #8
  %12 = load %"struct.std::__future_base::_Result_base"*, %"struct.std::__future_base::_Result_base"** %6, align 8, !tbaa !27
  %13 = getelementptr inbounds %"struct.std::__future_base::_Result_base", %"struct.std::__future_base::_Result_base"* %12, i64 0, i32 1
  %14 = bitcast %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* %3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %14) #8
  %15 = getelementptr inbounds %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider", %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* %4, i64 0, i32 0
  %16 = bitcast %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* %4 to i64*
  %17 = load i64, i64* %16, align 8, !tbaa !240
  %18 = bitcast %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* %3 to i64*
  store i64 %17, i64* %18, align 8, !tbaa !240
  store i8* null, i8** %15, align 8, !tbaa !240
  call void @_ZNSt15__exception_ptr13exception_ptr4swapERS0_(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* nonnull %3, %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* dereferenceable(8) %13) #8
  call void @_ZNSt15__exception_ptr13exception_ptrD1Ev(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* nonnull %3) #8
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %14) #8
  call void @_ZNSt15__exception_ptr13exception_ptrD1Ev(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* nonnull %4) #8
  call void @_ZNSt12future_errorD1Ev(%"class.std::future_error"* nonnull %5) #8
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %11) #8
  %19 = getelementptr inbounds %"class.std::__future_base::_State_baseV2", %"class.std::__future_base::_State_baseV2"* %0, i64 0, i32 1
  %20 = bitcast %"class.std::unique_ptr.58"* %19 to i64*
  %21 = load i64, i64* %20, align 8, !tbaa !27
  %22 = bitcast %"class.std::unique_ptr.58"* %1 to i64*
  %23 = load i64, i64* %22, align 8, !tbaa !27
  store i64 %23, i64* %20, align 8, !tbaa !27
  store i64 %21, i64* %22, align 8, !tbaa !27
  %24 = getelementptr inbounds %"class.std::__future_base::_State_baseV2", %"class.std::__future_base::_State_baseV2"* %0, i64 0, i32 2, i32 0, i32 0, i32 0
  %25 = atomicrmw xchg i32* %24, i32 1 release
  %26 = icmp slt i32 %25, 0
  br i1 %26, label %27, label %28

; <label>:27:                                     ; preds = %9
  call void @_ZNSt28__atomic_futex_unsigned_base19_M_futex_notify_allEPj(i32* %24)
  br label %28

; <label>:28:                                     ; preds = %2, %27, %9
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZNSt10unique_ptrINSt13__future_base12_Result_baseENS1_8_DeleterEED2Ev(%"class.std::unique_ptr.58"*) unnamed_addr #10 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %2 = getelementptr inbounds %"class.std::unique_ptr.58", %"class.std::unique_ptr.58"* %0, i64 0, i32 0, i32 0, i32 0, i32 0
  %3 = load %"struct.std::__future_base::_Result_base"*, %"struct.std::__future_base::_Result_base"** %2, align 8, !tbaa !27
  %4 = icmp eq %"struct.std::__future_base::_Result_base"* %3, null
  br i1 %4, label %12, label %5

; <label>:5:                                      ; preds = %1
  %6 = bitcast %"struct.std::__future_base::_Result_base"* %3 to void (%"struct.std::__future_base::_Result_base"*)***
  %7 = load void (%"struct.std::__future_base::_Result_base"*)**, void (%"struct.std::__future_base::_Result_base"*)*** %6, align 8, !tbaa !133
  %8 = load void (%"struct.std::__future_base::_Result_base"*)*, void (%"struct.std::__future_base::_Result_base"*)** %7, align 8
  invoke void %8(%"struct.std::__future_base::_Result_base"* nonnull %3)
          to label %12 unwind label %9

; <label>:9:                                      ; preds = %5
  %10 = landingpad { i8*, i32 }
          catch i8* null
  %11 = extractvalue { i8*, i32 } %10, 0
  tail call void @__clang_call_terminate(i8* %11) #22
  unreachable

; <label>:12:                                     ; preds = %5, %1
  store %"struct.std::__future_base::_Result_base"* null, %"struct.std::__future_base::_Result_base"** %2, align 8, !tbaa !27
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZNSt12__shared_ptrINSt13__future_base16_Task_state_baseIFvvEEELN9__gnu_cxx12_Lock_policyE2EED2Ev(%"class.std::__shared_ptr.108"*) unnamed_addr #10 comdat align 2 personality i32 (...)* @__gxx_personality_v0 {
  %2 = getelementptr inbounds %"class.std::__shared_ptr.108", %"class.std::__shared_ptr.108"* %0, i64 0, i32 1, i32 0
  %3 = load %"class.std::_Sp_counted_base"*, %"class.std::_Sp_counted_base"** %2, align 8, !tbaa !129
  %4 = icmp eq %"class.std::_Sp_counted_base"* %3, null
  br i1 %4, label %33, label %5

; <label>:5:                                      ; preds = %1
  %6 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %3, i64 0, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %7, label %9

; <label>:7:                                      ; preds = %5
  %8 = atomicrmw volatile add i32* %6, i32 -1 acq_rel
  br label %12

; <label>:9:                                      ; preds = %5
  %10 = load i32, i32* %6, align 4, !tbaa !17
  %11 = add nsw i32 %10, -1
  store i32 %11, i32* %6, align 4, !tbaa !17
  br label %12

; <label>:12:                                     ; preds = %9, %7
  %13 = phi i32 [ %8, %7 ], [ %10, %9 ]
  %14 = icmp eq i32 %13, 1
  br i1 %14, label %15, label %33

; <label>:15:                                     ; preds = %12
  %16 = bitcast %"class.std::_Sp_counted_base"* %3 to void (%"class.std::_Sp_counted_base"*)***
  %17 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %16, align 8, !tbaa !133
  %18 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %17, i64 2
  %19 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %18, align 8
  tail call void %19(%"class.std::_Sp_counted_base"* nonnull %3) #8
  %20 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %3, i64 0, i32 2
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %21, label %23

; <label>:21:                                     ; preds = %15
  %22 = atomicrmw volatile add i32* %20, i32 -1 acq_rel
  br label %26

; <label>:23:                                     ; preds = %15
  %24 = load i32, i32* %20, align 4, !tbaa !17
  %25 = add nsw i32 %24, -1
  store i32 %25, i32* %20, align 4, !tbaa !17
  br label %26

; <label>:26:                                     ; preds = %23, %21
  %27 = phi i32 [ %22, %21 ], [ %24, %23 ]
  %28 = icmp eq i32 %27, 1
  br i1 %28, label %29, label %33

; <label>:29:                                     ; preds = %26
  %30 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %16, align 8, !tbaa !133
  %31 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %30, i64 3
  %32 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %31, align 8
  tail call void %32(%"class.std::_Sp_counted_base"* nonnull %3) #8
  br label %33

; <label>:33:                                     ; preds = %1, %12, %26, %29
  ret void
}

; Function Attrs: nounwind
declare dereferenceable(8) %"class.MARC::IThreadTask"* @_ZSt15future_categoryv() local_unnamed_addr #7

; Function Attrs: uwtable
define linkonce_odr void @_ZNSt12future_errorC2ESt10error_code(%"class.std::future_error"*, i32, %"class.MARC::IThreadTask"*) unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %4 = alloca %"class.std::ios_base::Init", align 1
  %5 = alloca %"class.std::ios_base::Init", align 1
  %6 = alloca %"class.std::ios_base::Init", align 1
  %7 = alloca %"class.std::ios_base::Init", align 1
  %8 = alloca %"class.std::basic_string", align 8
  %9 = alloca %"class.std::basic_string", align 8
  %10 = bitcast %"class.std::basic_string"* %8 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %10) #8
  %11 = bitcast %"class.std::basic_string"* %9 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %11) #8
  %12 = bitcast %"class.MARC::IThreadTask"* %2 to void (%"class.std::basic_string"*, %"class.MARC::IThreadTask"*, i32)***
  %13 = load void (%"class.std::basic_string"*, %"class.MARC::IThreadTask"*, i32)**, void (%"class.std::basic_string"*, %"class.MARC::IThreadTask"*, i32)*** %12, align 8, !tbaa !133, !noalias !244
  %14 = getelementptr inbounds void (%"class.std::basic_string"*, %"class.MARC::IThreadTask"*, i32)*, void (%"class.std::basic_string"*, %"class.MARC::IThreadTask"*, i32)** %13, i64 3
  %15 = load void (%"class.std::basic_string"*, %"class.MARC::IThreadTask"*, i32)*, void (%"class.std::basic_string"*, %"class.MARC::IThreadTask"*, i32)** %14, align 8, !noalias !244
  call void %15(%"class.std::basic_string"* nonnull sret %9, %"class.MARC::IThreadTask"* nonnull %2, i32 %1)
  %16 = invoke dereferenceable(8) %"class.std::basic_string"* @_ZNSs6insertEmPKcm(%"class.std::basic_string"* nonnull %9, i64 0, i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.6, i64 0, i64 0), i64 19)
          to label %17 unwind label %65

; <label>:17:                                     ; preds = %3
  %18 = bitcast %"class.std::basic_string"* %16 to i64*
  %19 = load i64, i64* %18, align 8, !tbaa !2, !noalias !247
  %20 = bitcast %"class.std::basic_string"* %8 to i64*
  store i64 %19, i64* %20, align 8, !tbaa !2, !alias.scope !247
  %21 = getelementptr inbounds %"class.std::basic_string", %"class.std::basic_string"* %16, i64 0, i32 0, i32 0
  store i8* bitcast (i64* getelementptr inbounds ([0 x i64], [0 x i64]* @_ZNSs4_Rep20_S_empty_rep_storageE, i64 0, i64 3) to i8*), i8** %21, align 8, !tbaa !10, !noalias !247
  %22 = getelementptr inbounds %"class.std::future_error", %"class.std::future_error"* %0, i64 0, i32 0
  invoke void @_ZNSt11logic_errorC2ERKSs(%"class.std::logic_error"* %22, %"class.std::basic_string"* nonnull dereferenceable(8) %8)
          to label %23 unwind label %69

; <label>:23:                                     ; preds = %17
  %24 = getelementptr inbounds %"class.std::basic_string", %"class.std::basic_string"* %8, i64 0, i32 0, i32 0
  %25 = load i8*, i8** %24, align 8, !tbaa !10
  %26 = getelementptr inbounds i8, i8* %25, i64 -24
  %27 = bitcast i8* %26 to %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Rep"*
  %28 = getelementptr inbounds %"class.std::ios_base::Init", %"class.std::ios_base::Init"* %7, i64 0, i32 0
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %28) #8
  %29 = icmp eq i8* %26, bitcast ([0 x i64]* @_ZNSs4_Rep20_S_empty_rep_storageE to i8*)
  br i1 %29, label %42, label %30, !prof !16

; <label>:30:                                     ; preds = %23
  %31 = getelementptr inbounds i8, i8* %25, i64 -8
  %32 = bitcast i8* %31 to i32*
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %33, label %35

; <label>:33:                                     ; preds = %30
  %34 = atomicrmw volatile add i32* %32, i32 -1 acq_rel
  br label %38

; <label>:35:                                     ; preds = %30
  %36 = load i32, i32* %32, align 4, !tbaa !17
  %37 = add nsw i32 %36, -1
  store i32 %37, i32* %32, align 4, !tbaa !17
  br label %38

; <label>:38:                                     ; preds = %35, %33
  %39 = phi i32 [ %34, %33 ], [ %36, %35 ]
  %40 = icmp slt i32 %39, 1
  br i1 %40, label %41, label %42

; <label>:41:                                     ; preds = %38
  call void @_ZNSs4_Rep10_M_destroyERKSaIcE(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Rep"* %27, %"class.std::ios_base::Init"* nonnull dereferenceable(1) %7) #8
  br label %42

; <label>:42:                                     ; preds = %23, %38, %41
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %28) #8
  %43 = getelementptr inbounds %"class.std::basic_string", %"class.std::basic_string"* %9, i64 0, i32 0, i32 0
  %44 = load i8*, i8** %43, align 8, !tbaa !10
  %45 = getelementptr inbounds i8, i8* %44, i64 -24
  %46 = bitcast i8* %45 to %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Rep"*
  %47 = getelementptr inbounds %"class.std::ios_base::Init", %"class.std::ios_base::Init"* %6, i64 0, i32 0
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %47) #8
  %48 = icmp eq i8* %45, bitcast ([0 x i64]* @_ZNSs4_Rep20_S_empty_rep_storageE to i8*)
  br i1 %48, label %61, label %49, !prof !16

; <label>:49:                                     ; preds = %42
  %50 = getelementptr inbounds i8, i8* %44, i64 -8
  %51 = bitcast i8* %50 to i32*
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %52, label %54

; <label>:52:                                     ; preds = %49
  %53 = atomicrmw volatile add i32* %51, i32 -1 acq_rel
  br label %57

; <label>:54:                                     ; preds = %49
  %55 = load i32, i32* %51, align 4, !tbaa !17
  %56 = add nsw i32 %55, -1
  store i32 %56, i32* %51, align 4, !tbaa !17
  br label %57

; <label>:57:                                     ; preds = %54, %52
  %58 = phi i32 [ %53, %52 ], [ %55, %54 ]
  %59 = icmp slt i32 %58, 1
  br i1 %59, label %60, label %61

; <label>:60:                                     ; preds = %57
  call void @_ZNSs4_Rep10_M_destroyERKSaIcE(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Rep"* %46, %"class.std::ios_base::Init"* nonnull dereferenceable(1) %6) #8
  br label %61

; <label>:61:                                     ; preds = %42, %57, %60
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %47) #8
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %11) #8
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %10) #8
  %62 = getelementptr inbounds %"class.std::future_error", %"class.std::future_error"* %0, i64 0, i32 0, i32 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [5 x i8*] }, { [5 x i8*] }* @_ZTVSt12future_error, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %62, align 8, !tbaa !133
  %63 = getelementptr inbounds %"class.std::future_error", %"class.std::future_error"* %0, i64 0, i32 1, i32 0
  store i32 %1, i32* %63, align 8
  %64 = getelementptr inbounds %"class.std::future_error", %"class.std::future_error"* %0, i64 0, i32 1, i32 1
  store %"class.MARC::IThreadTask"* %2, %"class.MARC::IThreadTask"** %64, align 8
  ret void

; <label>:65:                                     ; preds = %3
  %66 = landingpad { i8*, i32 }
          cleanup
  %67 = extractvalue { i8*, i32 } %66, 0
  %68 = extractvalue { i8*, i32 } %66, 1
  br label %92

; <label>:69:                                     ; preds = %17
  %70 = landingpad { i8*, i32 }
          cleanup
  %71 = extractvalue { i8*, i32 } %70, 0
  %72 = extractvalue { i8*, i32 } %70, 1
  %73 = getelementptr inbounds %"class.std::basic_string", %"class.std::basic_string"* %8, i64 0, i32 0, i32 0
  %74 = load i8*, i8** %73, align 8, !tbaa !10
  %75 = getelementptr inbounds i8, i8* %74, i64 -24
  %76 = bitcast i8* %75 to %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Rep"*
  %77 = getelementptr inbounds %"class.std::ios_base::Init", %"class.std::ios_base::Init"* %5, i64 0, i32 0
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %77) #8
  %78 = icmp eq i8* %75, bitcast ([0 x i64]* @_ZNSs4_Rep20_S_empty_rep_storageE to i8*)
  br i1 %78, label %91, label %79, !prof !16

; <label>:79:                                     ; preds = %69
  %80 = getelementptr inbounds i8, i8* %74, i64 -8
  %81 = bitcast i8* %80 to i32*
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %82, label %84

; <label>:82:                                     ; preds = %79
  %83 = atomicrmw volatile add i32* %81, i32 -1 acq_rel
  br label %87

; <label>:84:                                     ; preds = %79
  %85 = load i32, i32* %81, align 4, !tbaa !17
  %86 = add nsw i32 %85, -1
  store i32 %86, i32* %81, align 4, !tbaa !17
  br label %87

; <label>:87:                                     ; preds = %84, %82
  %88 = phi i32 [ %83, %82 ], [ %85, %84 ]
  %89 = icmp slt i32 %88, 1
  br i1 %89, label %90, label %91

; <label>:90:                                     ; preds = %87
  call void @_ZNSs4_Rep10_M_destroyERKSaIcE(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Rep"* %76, %"class.std::ios_base::Init"* nonnull dereferenceable(1) %5) #8
  br label %91

; <label>:91:                                     ; preds = %69, %87, %90
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %77) #8
  br label %92

; <label>:92:                                     ; preds = %91, %65
  %93 = phi i8* [ %71, %91 ], [ %67, %65 ]
  %94 = phi i32 [ %72, %91 ], [ %68, %65 ]
  %95 = getelementptr inbounds %"class.std::basic_string", %"class.std::basic_string"* %9, i64 0, i32 0, i32 0
  %96 = load i8*, i8** %95, align 8, !tbaa !10
  %97 = getelementptr inbounds i8, i8* %96, i64 -24
  %98 = bitcast i8* %97 to %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Rep"*
  %99 = getelementptr inbounds %"class.std::ios_base::Init", %"class.std::ios_base::Init"* %4, i64 0, i32 0
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %99) #8
  %100 = icmp eq i8* %97, bitcast ([0 x i64]* @_ZNSs4_Rep20_S_empty_rep_storageE to i8*)
  br i1 %100, label %113, label %101, !prof !16

; <label>:101:                                    ; preds = %92
  %102 = getelementptr inbounds i8, i8* %96, i64 -8
  %103 = bitcast i8* %102 to i32*
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %104, label %106

; <label>:104:                                    ; preds = %101
  %105 = atomicrmw volatile add i32* %103, i32 -1 acq_rel
  br label %109

; <label>:106:                                    ; preds = %101
  %107 = load i32, i32* %103, align 4, !tbaa !17
  %108 = add nsw i32 %107, -1
  store i32 %108, i32* %103, align 4, !tbaa !17
  br label %109

; <label>:109:                                    ; preds = %106, %104
  %110 = phi i32 [ %105, %104 ], [ %107, %106 ]
  %111 = icmp slt i32 %110, 1
  br i1 %111, label %112, label %113

; <label>:112:                                    ; preds = %109
  call void @_ZNSs4_Rep10_M_destroyERKSaIcE(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Rep"* %98, %"class.std::ios_base::Init"* nonnull dereferenceable(1) %4) #8
  br label %113

; <label>:113:                                    ; preds = %92, %109, %112
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %99) #8
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %11) #8
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %10) #8
  %114 = insertvalue { i8*, i32 } undef, i8* %93, 0
  %115 = insertvalue { i8*, i32 } %114, i32 %94, 1
  resume { i8*, i32 } %115
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZSt18make_exception_ptrISt12future_errorENSt15__exception_ptr13exception_ptrET_(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* noalias sret, %"class.std::future_error"*) local_unnamed_addr #10 comdat personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %3 = tail call i8* @__cxa_allocate_exception(i64 32) #8
  %4 = bitcast i8* %3 to i32 (...)***
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [5 x i8*] }, { [5 x i8*] }* @_ZTVSt11logic_error, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %4, align 8, !tbaa !133
  %5 = getelementptr inbounds i8, i8* %3, i64 8
  %6 = bitcast i8* %5 to %"class.std::basic_string"*
  %7 = getelementptr inbounds %"class.std::future_error", %"class.std::future_error"* %1, i64 0, i32 0, i32 1
  invoke void @_ZNSsC1ERKSs(%"class.std::basic_string"* %6, %"class.std::basic_string"* dereferenceable(8) %7)
          to label %8 unwind label %12

; <label>:8:                                      ; preds = %2
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [5 x i8*] }, { [5 x i8*] }* @_ZTVSt12future_error, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %4, align 8, !tbaa !133
  %9 = getelementptr inbounds i8, i8* %3, i64 16
  %10 = getelementptr inbounds %"class.std::future_error", %"class.std::future_error"* %1, i64 0, i32 1
  %11 = bitcast %"struct.std::error_code"* %10 to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %9, i8* %11, i64 16, i32 8, i1 false), !tbaa.struct !250
  invoke void @__cxa_throw(i8* %3, i8* bitcast (i8** @_ZTISt12future_error to i8*), i8* bitcast (void (%"class.std::future_error"*)* @_ZNSt12future_errorD1Ev to i8*)) #23
          to label %26 unwind label %16

; <label>:12:                                     ; preds = %2
  %13 = landingpad { i8*, i32 }
          catch i8* null
  %14 = bitcast i8* %3 to %"class.MARC::IThreadTask"*
  tail call void @_ZNSt9exceptionD2Ev(%"class.MARC::IThreadTask"* %14) #8
  %15 = extractvalue { i8*, i32 } %13, 0
  tail call void @__cxa_free_exception(i8* %3) #8
  br label %19

; <label>:16:                                     ; preds = %8
  %17 = landingpad { i8*, i32 }
          catch i8* null
  %18 = extractvalue { i8*, i32 } %17, 0
  br label %19

; <label>:19:                                     ; preds = %16, %12
  %20 = phi i8* [ %18, %16 ], [ %15, %12 ]
  %21 = tail call i8* @__cxa_begin_catch(i8* %20) #8
  tail call void @_ZSt17current_exceptionv(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* sret %0) #8
  invoke void @__cxa_end_catch()
          to label %22 unwind label %23

; <label>:22:                                     ; preds = %19
  ret void

; <label>:23:                                     ; preds = %19
  %24 = landingpad { i8*, i32 }
          catch i8* null
  %25 = extractvalue { i8*, i32 } %24, 0
  tail call void @__clang_call_terminate(i8* %25) #22
  unreachable

; <label>:26:                                     ; preds = %8
  unreachable
}

; Function Attrs: nounwind
declare void @_ZNSt15__exception_ptr13exception_ptr4swapERS0_(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"*, %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* dereferenceable(8)) local_unnamed_addr #7

; Function Attrs: nounwind
declare void @_ZNSt12future_errorD1Ev(%"class.std::future_error"*) unnamed_addr #7

declare void @_ZNSt28__atomic_futex_unsigned_base19_M_futex_notify_allEPj(i32*) local_unnamed_addr #6

declare i8* @__cxa_allocate_exception(i64) local_unnamed_addr

declare void @_ZNSsC1ERKSs(%"class.std::basic_string"*, %"class.std::basic_string"* dereferenceable(8)) unnamed_addr #6

declare void @__cxa_throw(i8*, i8*, i8*) local_unnamed_addr

; Function Attrs: nounwind
declare void @_ZNSt9exceptionD2Ev(%"class.MARC::IThreadTask"*) unnamed_addr #7

declare void @__cxa_free_exception(i8*) local_unnamed_addr

declare i8* @__cxa_begin_catch(i8*) local_unnamed_addr

; Function Attrs: nounwind
declare void @_ZSt17current_exceptionv(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* sret) local_unnamed_addr #7

declare void @__cxa_end_catch() local_unnamed_addr

declare dereferenceable(8) %"class.std::basic_string"* @_ZNSs6insertEmPKcm(%"class.std::basic_string"*, i64, i8*, i64) local_unnamed_addr #6

declare void @_ZNSt11logic_errorC2ERKSs(%"class.std::logic_error"*, %"class.std::basic_string"* dereferenceable(8)) unnamed_addr #6

; Function Attrs: uwtable
define linkonce_odr void @_ZN4MARC10ThreadTaskISt13packaged_taskIFvvEEE7executeEv(%"class.MARC::ThreadTask"*) unnamed_addr #5 comdat align 2 {
  %2 = getelementptr inbounds %"class.MARC::ThreadTask", %"class.MARC::ThreadTask"* %0, i64 0, i32 1, i32 0, i32 0, i32 0
  %3 = load %"struct.std::__future_base::_Task_state_base"*, %"struct.std::__future_base::_Task_state_base"** %2, align 8, !tbaa !242
  %4 = icmp eq %"struct.std::__future_base::_Task_state_base"* %3, null
  br i1 %4, label %5, label %6

; <label>:5:                                      ; preds = %1
  tail call void @_ZSt20__throw_future_errori(i32 3) #23
  unreachable

; <label>:6:                                      ; preds = %1
  %7 = bitcast %"struct.std::__future_base::_Task_state_base"* %3 to void (%"struct.std::__future_base::_Task_state_base"*)***
  %8 = load void (%"struct.std::__future_base::_Task_state_base"*)**, void (%"struct.std::__future_base::_Task_state_base"*)*** %7, align 8, !tbaa !133
  %9 = getelementptr inbounds void (%"struct.std::__future_base::_Task_state_base"*)*, void (%"struct.std::__future_base::_Task_state_base"*)** %8, i64 4
  %10 = load void (%"struct.std::__future_base::_Task_state_base"*)*, void (%"struct.std::__future_base::_Task_state_base"*)** %9, align 8
  tail call void %10(%"struct.std::__future_base::_Task_state_base"* nonnull %3)
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN4MARC10ThreadTaskISt13packaged_taskIFvvEEED2Ev(%"class.MARC::ThreadTask"*) unnamed_addr #10 comdat align 2 {
  %2 = getelementptr inbounds %"class.MARC::ThreadTask", %"class.MARC::ThreadTask"* %0, i64 0, i32 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [5 x i8*] }, { [5 x i8*] }* @_ZTVN4MARC10ThreadTaskISt13packaged_taskIFvvEEEE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %2, align 8, !tbaa !133
  %3 = getelementptr inbounds %"class.MARC::ThreadTask", %"class.MARC::ThreadTask"* %0, i64 0, i32 1
  tail call void @_ZNSt13packaged_taskIFvvEED2Ev(%"class.std::packaged_task"* %3) #8
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN4MARC10ThreadTaskISt13packaged_taskIFvvEEED0Ev(%"class.MARC::ThreadTask"*) unnamed_addr #10 comdat align 2 {
  %2 = getelementptr inbounds %"class.MARC::ThreadTask", %"class.MARC::ThreadTask"* %0, i64 0, i32 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [5 x i8*] }, { [5 x i8*] }* @_ZTVN4MARC10ThreadTaskISt13packaged_taskIFvvEEEE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %2, align 8, !tbaa !133
  %3 = getelementptr inbounds %"class.MARC::ThreadTask", %"class.MARC::ThreadTask"* %0, i64 0, i32 1
  tail call void @_ZNSt13packaged_taskIFvvEED2Ev(%"class.std::packaged_task"* %3) #8
  %4 = bitcast %"class.MARC::ThreadTask"* %0 to i8*
  tail call void @_ZdlPv(i8* %4) #25
  ret void
}

; Function Attrs: inlinehint nounwind uwtable
define linkonce_odr void @_ZNSt13__future_base11_Task_stateISt5_BindIFPFvPvS2_ES2_S2_EESaIiEFvvEED2Ev(%"struct.std::__future_base::_Task_state"*) unnamed_addr #17 comdat align 2 personality i32 (...)* @__gxx_personality_v0 {
  %2 = getelementptr inbounds %"struct.std::__future_base::_Task_state", %"struct.std::__future_base::_Task_state"* %0, i64 0, i32 0, i32 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [9 x i8*] }, { [9 x i8*] }* @_ZTVNSt13__future_base16_Task_state_baseIFvvEEE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %2, align 8, !tbaa !133
  %3 = getelementptr inbounds %"struct.std::__future_base::_Task_state", %"struct.std::__future_base::_Task_state"* %0, i64 0, i32 0, i32 1, i32 0, i32 0, i32 0, i32 0
  %4 = load %"struct.std::__future_base::_Result"*, %"struct.std::__future_base::_Result"** %3, align 8, !tbaa !27
  %5 = icmp eq %"struct.std::__future_base::_Result"* %4, null
  br i1 %5, label %14, label %6

; <label>:6:                                      ; preds = %1
  %7 = getelementptr inbounds %"struct.std::__future_base::_Result", %"struct.std::__future_base::_Result"* %4, i64 0, i32 0
  %8 = bitcast %"struct.std::__future_base::_Result"* %4 to void (%"struct.std::__future_base::_Result_base"*)***
  %9 = load void (%"struct.std::__future_base::_Result_base"*)**, void (%"struct.std::__future_base::_Result_base"*)*** %8, align 8, !tbaa !133
  %10 = load void (%"struct.std::__future_base::_Result_base"*)*, void (%"struct.std::__future_base::_Result_base"*)** %9, align 8
  invoke void %10(%"struct.std::__future_base::_Result_base"* %7)
          to label %14 unwind label %11

; <label>:11:                                     ; preds = %6
  %12 = landingpad { i8*, i32 }
          catch i8* null
  %13 = extractvalue { i8*, i32 } %12, 0
  tail call void @__clang_call_terminate(i8* %13) #22
  unreachable

; <label>:14:                                     ; preds = %6, %1
  store %"struct.std::__future_base::_Result"* null, %"struct.std::__future_base::_Result"** %3, align 8, !tbaa !27
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [6 x i8*] }, { [6 x i8*] }* @_ZTVNSt13__future_base13_State_baseV2E, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %2, align 8, !tbaa !133
  %15 = getelementptr inbounds %"struct.std::__future_base::_Task_state", %"struct.std::__future_base::_Task_state"* %0, i64 0, i32 0, i32 0, i32 1, i32 0, i32 0, i32 0, i32 0
  %16 = load %"struct.std::__future_base::_Result_base"*, %"struct.std::__future_base::_Result_base"** %15, align 8, !tbaa !27
  %17 = icmp eq %"struct.std::__future_base::_Result_base"* %16, null
  br i1 %17, label %25, label %18

; <label>:18:                                     ; preds = %14
  %19 = bitcast %"struct.std::__future_base::_Result_base"* %16 to void (%"struct.std::__future_base::_Result_base"*)***
  %20 = load void (%"struct.std::__future_base::_Result_base"*)**, void (%"struct.std::__future_base::_Result_base"*)*** %19, align 8, !tbaa !133
  %21 = load void (%"struct.std::__future_base::_Result_base"*)*, void (%"struct.std::__future_base::_Result_base"*)** %20, align 8
  invoke void %21(%"struct.std::__future_base::_Result_base"* nonnull %16)
          to label %25 unwind label %22

; <label>:22:                                     ; preds = %18
  %23 = landingpad { i8*, i32 }
          catch i8* null
  %24 = extractvalue { i8*, i32 } %23, 0
  tail call void @__clang_call_terminate(i8* %24) #22
  unreachable

; <label>:25:                                     ; preds = %14, %18
  store %"struct.std::__future_base::_Result_base"* null, %"struct.std::__future_base::_Result_base"** %15, align 8, !tbaa !27
  ret void
}

; Function Attrs: inlinehint nounwind uwtable
define linkonce_odr void @_ZNSt13__future_base11_Task_stateISt5_BindIFPFvPvS2_ES2_S2_EESaIiEFvvEED0Ev(%"struct.std::__future_base::_Task_state"*) unnamed_addr #17 comdat align 2 personality i32 (...)* @__gxx_personality_v0 {
  %2 = getelementptr inbounds %"struct.std::__future_base::_Task_state", %"struct.std::__future_base::_Task_state"* %0, i64 0, i32 0, i32 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [9 x i8*] }, { [9 x i8*] }* @_ZTVNSt13__future_base16_Task_state_baseIFvvEEE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %2, align 8, !tbaa !133
  %3 = getelementptr inbounds %"struct.std::__future_base::_Task_state", %"struct.std::__future_base::_Task_state"* %0, i64 0, i32 0, i32 1, i32 0, i32 0, i32 0, i32 0
  %4 = load %"struct.std::__future_base::_Result"*, %"struct.std::__future_base::_Result"** %3, align 8, !tbaa !27
  %5 = icmp eq %"struct.std::__future_base::_Result"* %4, null
  br i1 %5, label %14, label %6

; <label>:6:                                      ; preds = %1
  %7 = getelementptr inbounds %"struct.std::__future_base::_Result", %"struct.std::__future_base::_Result"* %4, i64 0, i32 0
  %8 = bitcast %"struct.std::__future_base::_Result"* %4 to void (%"struct.std::__future_base::_Result_base"*)***
  %9 = load void (%"struct.std::__future_base::_Result_base"*)**, void (%"struct.std::__future_base::_Result_base"*)*** %8, align 8, !tbaa !133
  %10 = load void (%"struct.std::__future_base::_Result_base"*)*, void (%"struct.std::__future_base::_Result_base"*)** %9, align 8
  invoke void %10(%"struct.std::__future_base::_Result_base"* %7)
          to label %14 unwind label %11

; <label>:11:                                     ; preds = %6
  %12 = landingpad { i8*, i32 }
          catch i8* null
  %13 = extractvalue { i8*, i32 } %12, 0
  tail call void @__clang_call_terminate(i8* %13) #22
  unreachable

; <label>:14:                                     ; preds = %6, %1
  store %"struct.std::__future_base::_Result"* null, %"struct.std::__future_base::_Result"** %3, align 8, !tbaa !27
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [6 x i8*] }, { [6 x i8*] }* @_ZTVNSt13__future_base13_State_baseV2E, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %2, align 8, !tbaa !133
  %15 = getelementptr inbounds %"struct.std::__future_base::_Task_state", %"struct.std::__future_base::_Task_state"* %0, i64 0, i32 0, i32 0, i32 1, i32 0, i32 0, i32 0, i32 0
  %16 = load %"struct.std::__future_base::_Result_base"*, %"struct.std::__future_base::_Result_base"** %15, align 8, !tbaa !27
  %17 = icmp eq %"struct.std::__future_base::_Result_base"* %16, null
  br i1 %17, label %25, label %18

; <label>:18:                                     ; preds = %14
  %19 = bitcast %"struct.std::__future_base::_Result_base"* %16 to void (%"struct.std::__future_base::_Result_base"*)***
  %20 = load void (%"struct.std::__future_base::_Result_base"*)**, void (%"struct.std::__future_base::_Result_base"*)*** %19, align 8, !tbaa !133
  %21 = load void (%"struct.std::__future_base::_Result_base"*)*, void (%"struct.std::__future_base::_Result_base"*)** %20, align 8
  invoke void %21(%"struct.std::__future_base::_Result_base"* nonnull %16)
          to label %25 unwind label %22

; <label>:22:                                     ; preds = %18
  %23 = landingpad { i8*, i32 }
          catch i8* null
  %24 = extractvalue { i8*, i32 } %23, 0
  tail call void @__clang_call_terminate(i8* %24) #22
  unreachable

; <label>:25:                                     ; preds = %14, %18
  %26 = bitcast %"struct.std::__future_base::_Task_state"* %0 to i8*
  tail call void @_ZdlPv(i8* %26) #25
  ret void
}

; Function Attrs: norecurse nounwind uwtable
define linkonce_odr void @_ZNSt13__future_base13_State_baseV217_M_complete_asyncEv(%"class.std::__future_base::_State_baseV2"*) unnamed_addr #20 comdat align 2 {
  ret void
}

; Function Attrs: norecurse nounwind uwtable
define linkonce_odr zeroext i1 @_ZNKSt13__future_base13_State_baseV221_M_is_deferred_futureEv(%"class.std::__future_base::_State_baseV2"*) unnamed_addr #20 comdat align 2 {
  ret i1 false
}

; Function Attrs: uwtable
define linkonce_odr void @_ZNSt13__future_base11_Task_stateISt5_BindIFPFvPvS2_ES2_S2_EESaIiEFvvEE6_M_runEv(%"struct.std::__future_base::_Task_state"*) unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %2 = alloca %class.anon.135, align 8
  %3 = alloca i8, align 1
  %4 = alloca <2 x i64>, align 16
  %5 = alloca %"class.std::__future_base::_State_baseV2"*, align 8
  %6 = alloca %"class.std::function.133"*, align 8
  %7 = alloca i8*, align 8
  %8 = alloca %"struct.std::_Bind_simple.127", align 8
  %9 = alloca %"class.std::function.133", align 8
  %10 = bitcast %"struct.std::_Bind_simple.127"* %8 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %10) #8
  %11 = getelementptr inbounds %"struct.std::__future_base::_Task_state", %"struct.std::__future_base::_Task_state"* %0, i64 0, i32 1, i32 0
  %12 = ptrtoint %"class.std::_Bind"* %11 to i64
  %13 = bitcast %"struct.std::_Bind_simple.127"* %8 to i64*
  store i64 %12, i64* %13, align 8, !alias.scope !251
  %14 = getelementptr inbounds %"struct.std::__future_base::_Task_state", %"struct.std::__future_base::_Task_state"* %0, i64 0, i32 0, i32 1
  %15 = getelementptr inbounds %"class.std::function.133", %"class.std::function.133"* %9, i64 0, i32 0, i32 1
  %16 = bitcast %"class.std::function.133"* %9 to %"class.std::unique_ptr.109"**
  store %"class.std::unique_ptr.109"* %14, %"class.std::unique_ptr.109"** %16, align 8
  %17 = getelementptr inbounds %"class.std::function.133", %"class.std::function.133"* %9, i64 0, i32 0, i32 0, i32 0, i32 0, i32 1
  %18 = bitcast i64* %17 to %"struct.std::_Bind_simple.127"**
  store %"struct.std::_Bind_simple.127"* %8, %"struct.std::_Bind_simple.127"** %18, align 8
  %19 = getelementptr inbounds %"class.std::function.133", %"class.std::function.133"* %9, i64 0, i32 1
  store void (%"class.std::unique_ptr.58"*, %"union.std::_Any_data"*)* @_ZNSt17_Function_handlerIFSt10unique_ptrINSt13__future_base12_Result_baseENS2_8_DeleterEEvENS1_12_Task_setterIS0_INS1_7_ResultIvEES3_ESt12_Bind_simpleIFSt17reference_wrapperISt5_BindIFPFvPvSD_ESD_SD_EEEvEEvEEE9_M_invokeERKSt9_Any_data, void (%"class.std::unique_ptr.58"*, %"union.std::_Any_data"*)** %19, align 8, !tbaa !254
  store i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* @_ZNSt14_Function_base13_Base_managerINSt13__future_base12_Task_setterISt10unique_ptrINS1_7_ResultIvEENS1_12_Result_base8_DeleterEESt12_Bind_simpleIFSt17reference_wrapperISt5_BindIFPFvPvSC_ESC_SC_EEEvEEvEEE10_M_managerERSt9_Any_dataRKSM_St18_Manager_operation, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %15, align 8, !tbaa !123
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %3) #8
  store i8 0, i8* %3, align 1, !tbaa !256
  %20 = bitcast <2 x i64>* %4 to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %20) #8
  store <2 x i64> <i64 ptrtoint (void (%"class.std::__future_base::_State_baseV2"*, %"class.std::function.133"*, i8*)* @_ZNSt13__future_base13_State_baseV29_M_do_setEPSt8functionIFSt10unique_ptrINS_12_Result_baseENS3_8_DeleterEEvEEPb to i64), i64 0>, <2 x i64>* %4, align 16
  %21 = bitcast %"class.std::__future_base::_State_baseV2"** %5 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %21) #8
  %22 = bitcast %"class.std::__future_base::_State_baseV2"** %5 to %"struct.std::__future_base::_Task_state"**
  store %"struct.std::__future_base::_Task_state"* %0, %"struct.std::__future_base::_Task_state"** %22, align 8, !tbaa !27
  %23 = bitcast %"class.std::function.133"** %6 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %23) #8
  store %"class.std::function.133"* %9, %"class.std::function.133"** %6, align 8, !tbaa !27
  %24 = bitcast i8** %7 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %24) #8
  store i8* %3, i8** %7, align 8, !tbaa !27
  %25 = bitcast %class.anon.135* %2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 32, i8* nonnull %25) #8
  %26 = bitcast %class.anon.135* %2 to <2 x i64>**
  store <2 x i64>* %4, <2 x i64>** %26, align 8, !tbaa !18
  %27 = getelementptr inbounds %class.anon.135, %class.anon.135* %2, i64 0, i32 1
  store %"class.std::__future_base::_State_baseV2"** %5, %"class.std::__future_base::_State_baseV2"*** %27, align 8, !tbaa !18
  %28 = getelementptr inbounds %class.anon.135, %class.anon.135* %2, i64 0, i32 2
  store %"class.std::function.133"** %6, %"class.std::function.133"*** %28, align 8, !tbaa !18
  %29 = getelementptr inbounds %class.anon.135, %class.anon.135* %2, i64 0, i32 3
  store i8** %7, i8*** %29, align 8, !tbaa !18
  store %class.anon.135* %2, %class.anon.135** bitcast (i8** @_ZSt15__once_callable to %class.anon.135**), align 8, !tbaa !27
  store void ()* @_ZZSt9call_onceIMNSt13__future_base13_State_baseV2EFvPSt8functionIFSt10unique_ptrINS0_12_Result_baseENS4_8_DeleterEEvEEPbEJPS1_S9_SA_EEvRSt9once_flagOT_DpOT0_ENUlvE0_8__invokeEv, void ()** @_ZSt11__once_call, align 8, !tbaa !27
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %30, label %35

; <label>:30:                                     ; preds = %1
  %31 = getelementptr inbounds %"struct.std::__future_base::_Task_state", %"struct.std::__future_base::_Task_state"* %0, i64 0, i32 0, i32 0, i32 5, i32 0
  %32 = invoke i32 @pthread_once(i32* nonnull %31, void ()* nonnull @__once_proxy)
          to label %33 unwind label %58

; <label>:33:                                     ; preds = %30
  %34 = icmp eq i32 %32, 0
  br i1 %34, label %38, label %35

; <label>:35:                                     ; preds = %33, %1
  %36 = phi i32 [ %32, %33 ], [ -1, %1 ]
  invoke void @_ZSt20__throw_system_errori(i32 %36) #23
          to label %37 unwind label %58

; <label>:37:                                     ; preds = %35
  unreachable

; <label>:38:                                     ; preds = %33
  call void @llvm.lifetime.end.p0i8(i64 32, i8* nonnull %25) #8
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %24) #8
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %23) #8
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %21) #8
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %20) #8
  %39 = load i8, i8* %3, align 1, !tbaa !256, !range !43
  %40 = icmp eq i8 %39, 0
  br i1 %40, label %46, label %41

; <label>:41:                                     ; preds = %38
  %42 = getelementptr inbounds %"struct.std::__future_base::_Task_state", %"struct.std::__future_base::_Task_state"* %0, i64 0, i32 0, i32 0, i32 2, i32 0, i32 0, i32 0
  %43 = atomicrmw xchg i32* %42, i32 1 release
  %44 = icmp slt i32 %43, 0
  br i1 %44, label %45, label %48

; <label>:45:                                     ; preds = %41
  invoke void @_ZNSt28__atomic_futex_unsigned_base19_M_futex_notify_allEPj(i32* %42)
          to label %48 unwind label %58

; <label>:46:                                     ; preds = %38
  invoke void @_ZSt20__throw_future_errori(i32 2) #23
          to label %47 unwind label %58

; <label>:47:                                     ; preds = %46
  unreachable

; <label>:48:                                     ; preds = %41, %45
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %3) #8
  %49 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %15, align 8, !tbaa !123
  %50 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %49, null
  br i1 %50, label %57, label %51

; <label>:51:                                     ; preds = %48
  %52 = getelementptr inbounds %"class.std::function.133", %"class.std::function.133"* %9, i64 0, i32 0, i32 0
  %53 = invoke zeroext i1 %49(%"union.std::_Any_data"* nonnull dereferenceable(16) %52, %"union.std::_Any_data"* nonnull dereferenceable(16) %52, i32 3)
          to label %57 unwind label %54

; <label>:54:                                     ; preds = %51
  %55 = landingpad { i8*, i32 }
          catch i8* null
  %56 = extractvalue { i8*, i32 } %55, 0
  call void @__clang_call_terminate(i8* %56) #22
  unreachable

; <label>:57:                                     ; preds = %48, %51
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %10) #8
  ret void

; <label>:58:                                     ; preds = %46, %45, %35, %30
  %59 = landingpad { i8*, i32 }
          cleanup
  %60 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %15, align 8, !tbaa !123
  %61 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %60, null
  br i1 %61, label %68, label %62

; <label>:62:                                     ; preds = %58
  %63 = getelementptr inbounds %"class.std::function.133", %"class.std::function.133"* %9, i64 0, i32 0, i32 0
  %64 = invoke zeroext i1 %60(%"union.std::_Any_data"* nonnull dereferenceable(16) %63, %"union.std::_Any_data"* nonnull dereferenceable(16) %63, i32 3)
          to label %68 unwind label %65

; <label>:65:                                     ; preds = %62
  %66 = landingpad { i8*, i32 }
          catch i8* null
  %67 = extractvalue { i8*, i32 } %66, 0
  call void @__clang_call_terminate(i8* %67) #22
  unreachable

; <label>:68:                                     ; preds = %58, %62
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %10) #8
  resume { i8*, i32 } %59
}

; Function Attrs: uwtable
define linkonce_odr void @_ZNSt13__future_base11_Task_stateISt5_BindIFPFvPvS2_ES2_S2_EESaIiEFvvEE14_M_run_delayedESt8weak_ptrINS_13_State_baseV2EE(%"struct.std::__future_base::_Task_state"*, %"class.std::shared_ptr"*) unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %3 = alloca %"struct.std::_Bind_simple.127", align 8
  %4 = alloca %"class.std::function.133", align 8
  %5 = alloca <2 x i64>, align 16
  %6 = bitcast <2 x i64>* %5 to %"class.std::shared_ptr"*
  %7 = bitcast %"struct.std::_Bind_simple.127"* %3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %7) #8
  %8 = getelementptr inbounds %"struct.std::__future_base::_Task_state", %"struct.std::__future_base::_Task_state"* %0, i64 0, i32 1, i32 0
  %9 = ptrtoint %"class.std::_Bind"* %8 to i64
  %10 = bitcast %"struct.std::_Bind_simple.127"* %3 to i64*
  store i64 %9, i64* %10, align 8, !alias.scope !257
  %11 = bitcast %"struct.std::__future_base::_Task_state"* %0 to %"class.std::__future_base::_State_baseV2"*
  %12 = getelementptr inbounds %"struct.std::__future_base::_Task_state", %"struct.std::__future_base::_Task_state"* %0, i64 0, i32 0, i32 1
  %13 = getelementptr inbounds %"class.std::function.133", %"class.std::function.133"* %4, i64 0, i32 0, i32 1
  %14 = bitcast %"class.std::function.133"* %4 to %"class.std::unique_ptr.109"**
  store %"class.std::unique_ptr.109"* %12, %"class.std::unique_ptr.109"** %14, align 8
  %15 = getelementptr inbounds %"class.std::function.133", %"class.std::function.133"* %4, i64 0, i32 0, i32 0, i32 0, i32 0, i32 1
  %16 = bitcast i64* %15 to %"struct.std::_Bind_simple.127"**
  store %"struct.std::_Bind_simple.127"* %3, %"struct.std::_Bind_simple.127"** %16, align 8
  %17 = getelementptr inbounds %"class.std::function.133", %"class.std::function.133"* %4, i64 0, i32 1
  store void (%"class.std::unique_ptr.58"*, %"union.std::_Any_data"*)* @_ZNSt17_Function_handlerIFSt10unique_ptrINSt13__future_base12_Result_baseENS2_8_DeleterEEvENS1_12_Task_setterIS0_INS1_7_ResultIvEES3_ESt12_Bind_simpleIFSt17reference_wrapperISt5_BindIFPFvPvSD_ESD_SD_EEEvEEvEEE9_M_invokeERKSt9_Any_data, void (%"class.std::unique_ptr.58"*, %"union.std::_Any_data"*)** %17, align 8, !tbaa !254
  store i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* @_ZNSt14_Function_base13_Base_managerINSt13__future_base12_Task_setterISt10unique_ptrINS1_7_ResultIvEENS1_12_Result_base8_DeleterEESt12_Bind_simpleIFSt17reference_wrapperISt5_BindIFPFvPvSC_ESC_SC_EEEvEEvEEE10_M_managerERSt9_Any_dataRKSM_St18_Manager_operation, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %13, align 8, !tbaa !123
  %18 = bitcast %"class.std::shared_ptr"* %1 to <2 x i64>*
  %19 = load <2 x i64>, <2 x i64>* %18, align 8, !tbaa !27
  store <2 x i64> %19, <2 x i64>* %5, align 16, !tbaa !27
  %20 = bitcast %"class.std::shared_ptr"* %1 to i8*
  call void @llvm.memset.p0i8.i64(i8* nonnull %20, i8 0, i64 16, i32 8, i1 false) #8
  invoke void @_ZNSt13__future_base13_State_baseV221_M_set_delayed_resultESt8functionIFSt10unique_ptrINS_12_Result_baseENS3_8_DeleterEEvEESt8weak_ptrIS0_E(%"class.std::__future_base::_State_baseV2"* %11, %"class.std::function.133"* nonnull %4, %"class.std::shared_ptr"* nonnull %6)
          to label %21 unwind label %50

; <label>:21:                                     ; preds = %2
  %22 = getelementptr inbounds %"class.std::shared_ptr", %"class.std::shared_ptr"* %6, i64 0, i32 0, i32 1, i32 0
  %23 = load %"class.std::_Sp_counted_base"*, %"class.std::_Sp_counted_base"** %22, align 8, !tbaa !260
  %24 = icmp eq %"class.std::_Sp_counted_base"* %23, null
  br i1 %24, label %40, label %25

; <label>:25:                                     ; preds = %21
  %26 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %23, i64 0, i32 2
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %27, label %29

; <label>:27:                                     ; preds = %25
  %28 = atomicrmw volatile add i32* %26, i32 -1 acq_rel
  br label %32

; <label>:29:                                     ; preds = %25
  %30 = load i32, i32* %26, align 4, !tbaa !17
  %31 = add nsw i32 %30, -1
  store i32 %31, i32* %26, align 4, !tbaa !17
  br label %32

; <label>:32:                                     ; preds = %29, %27
  %33 = phi i32 [ %28, %27 ], [ %30, %29 ]
  %34 = icmp eq i32 %33, 1
  br i1 %34, label %35, label %40

; <label>:35:                                     ; preds = %32
  %36 = bitcast %"class.std::_Sp_counted_base"* %23 to void (%"class.std::_Sp_counted_base"*)***
  %37 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %36, align 8, !tbaa !133
  %38 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %37, i64 3
  %39 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %38, align 8
  call void %39(%"class.std::_Sp_counted_base"* nonnull %23) #8
  br label %40

; <label>:40:                                     ; preds = %21, %32, %35
  %41 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %13, align 8, !tbaa !123
  %42 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %41, null
  br i1 %42, label %49, label %43

; <label>:43:                                     ; preds = %40
  %44 = getelementptr inbounds %"class.std::function.133", %"class.std::function.133"* %4, i64 0, i32 0, i32 0
  %45 = invoke zeroext i1 %41(%"union.std::_Any_data"* nonnull dereferenceable(16) %44, %"union.std::_Any_data"* nonnull dereferenceable(16) %44, i32 3)
          to label %49 unwind label %46

; <label>:46:                                     ; preds = %43
  %47 = landingpad { i8*, i32 }
          catch i8* null
  %48 = extractvalue { i8*, i32 } %47, 0
  call void @__clang_call_terminate(i8* %48) #22
  unreachable

; <label>:49:                                     ; preds = %40, %43
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %7) #8
  ret void

; <label>:50:                                     ; preds = %2
  %51 = landingpad { i8*, i32 }
          cleanup
  %52 = getelementptr inbounds %"class.std::shared_ptr", %"class.std::shared_ptr"* %6, i64 0, i32 0, i32 1, i32 0
  %53 = load %"class.std::_Sp_counted_base"*, %"class.std::_Sp_counted_base"** %52, align 8, !tbaa !260
  %54 = icmp eq %"class.std::_Sp_counted_base"* %53, null
  br i1 %54, label %70, label %55

; <label>:55:                                     ; preds = %50
  %56 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %53, i64 0, i32 2
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %57, label %59

; <label>:57:                                     ; preds = %55
  %58 = atomicrmw volatile add i32* %56, i32 -1 acq_rel
  br label %62

; <label>:59:                                     ; preds = %55
  %60 = load i32, i32* %56, align 4, !tbaa !17
  %61 = add nsw i32 %60, -1
  store i32 %61, i32* %56, align 4, !tbaa !17
  br label %62

; <label>:62:                                     ; preds = %59, %57
  %63 = phi i32 [ %58, %57 ], [ %60, %59 ]
  %64 = icmp eq i32 %63, 1
  br i1 %64, label %65, label %70

; <label>:65:                                     ; preds = %62
  %66 = bitcast %"class.std::_Sp_counted_base"* %53 to void (%"class.std::_Sp_counted_base"*)***
  %67 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %66, align 8, !tbaa !133
  %68 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %67, i64 3
  %69 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %68, align 8
  call void %69(%"class.std::_Sp_counted_base"* nonnull %53) #8
  br label %70

; <label>:70:                                     ; preds = %50, %62, %65
  %71 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %13, align 8, !tbaa !123
  %72 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %71, null
  br i1 %72, label %79, label %73

; <label>:73:                                     ; preds = %70
  %74 = getelementptr inbounds %"class.std::function.133", %"class.std::function.133"* %4, i64 0, i32 0, i32 0
  %75 = invoke zeroext i1 %71(%"union.std::_Any_data"* nonnull dereferenceable(16) %74, %"union.std::_Any_data"* nonnull dereferenceable(16) %74, i32 3)
          to label %79 unwind label %76

; <label>:76:                                     ; preds = %73
  %77 = landingpad { i8*, i32 }
          catch i8* null
  %78 = extractvalue { i8*, i32 } %77, 0
  call void @__clang_call_terminate(i8* %78) #22
  unreachable

; <label>:79:                                     ; preds = %70, %73
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %7) #8
  resume { i8*, i32 } %51
}

; Function Attrs: uwtable
define linkonce_odr void @_ZNSt13__future_base11_Task_stateISt5_BindIFPFvPvS2_ES2_S2_EESaIiEFvvEE8_M_resetEv(%"class.std::shared_ptr.107"* noalias sret, %"struct.std::__future_base::_Task_state"*) unnamed_addr #5 comdat align 2 personality i32 (...)* @__gxx_personality_v0 {
  %3 = alloca %"class.std::ios_base::Init", align 1
  %4 = alloca <2 x i64>, align 16
  %5 = bitcast <2 x i64>* %4 to %"class.std::shared_ptr.121"*
  %6 = getelementptr inbounds %"struct.std::__future_base::_Task_state", %"struct.std::__future_base::_Task_state"* %1, i64 0, i32 1
  %7 = bitcast <2 x i64>* %4 to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %7) #8, !noalias !262
  %8 = bitcast <2 x i64>* %4 to %"struct.std::__future_base::_Task_state"**
  store %"struct.std::__future_base::_Task_state"* null, %"struct.std::__future_base::_Task_state"** %8, align 16, !tbaa !187, !alias.scope !265
  %9 = getelementptr inbounds %"class.std::shared_ptr.121", %"class.std::shared_ptr.121"* %5, i64 0, i32 0, i32 1
  %10 = getelementptr inbounds %"class.std::ios_base::Init", %"class.std::ios_base::Init"* %3, i64 0, i32 0
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %10), !noalias !265
  %11 = getelementptr inbounds %"class.std::__shared_count", %"class.std::__shared_count"* %9, i64 0, i32 0
  store %"class.std::_Sp_counted_base"* null, %"class.std::_Sp_counted_base"** %11, align 8, !tbaa !129, !alias.scope !265
  %12 = invoke i8* @_Znwm(i64 80)
          to label %13 unwind label %17, !noalias !265

; <label>:13:                                     ; preds = %2
  %14 = bitcast %"struct.std::__future_base::_Task_state<std::_Bind<void (*(void *, void *))(void *, void *)>, std::allocator<int>, void ()>::_Impl"* %6 to %"class.std::ios_base::Init"*
  %15 = getelementptr inbounds %"struct.std::__future_base::_Task_state<std::_Bind<void (*(void *, void *))(void *, void *)>, std::allocator<int>, void ()>::_Impl", %"struct.std::__future_base::_Task_state<std::_Bind<void (*(void *, void *))(void *, void *)>, std::allocator<int>, void ()>::_Impl"* %6, i64 0, i32 0
  %16 = bitcast i8* %12 to %"class.std::_Sp_counted_ptr_inplace"*
  invoke void @_ZNSt23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EEC2IJS7_RKS8_EEES8_DpOT_(%"class.std::_Sp_counted_ptr_inplace"* nonnull %16, %"class.std::ios_base::Init"* nonnull %3, %"class.std::_Bind"* nonnull dereferenceable(24) %15, %"class.std::ios_base::Init"* nonnull dereferenceable(1) %14)
          to label %30 unwind label %21, !noalias !265

; <label>:17:                                     ; preds = %2
  %18 = landingpad { i8*, i32 }
          cleanup
  %19 = extractvalue { i8*, i32 } %18, 0
  %20 = extractvalue { i8*, i32 } %18, 1
  br label %25

; <label>:21:                                     ; preds = %13
  %22 = landingpad { i8*, i32 }
          cleanup
  %23 = extractvalue { i8*, i32 } %22, 0
  %24 = extractvalue { i8*, i32 } %22, 1
  call void @_ZdlPv(i8* nonnull %12) #8
  br label %25

; <label>:25:                                     ; preds = %21, %17
  %26 = phi i32 [ %24, %21 ], [ %20, %17 ]
  %27 = phi i8* [ %23, %21 ], [ %19, %17 ]
  %28 = insertvalue { i8*, i32 } undef, i8* %27, 0
  %29 = insertvalue { i8*, i32 } %28, i32 %26, 1
  resume { i8*, i32 } %29

; <label>:30:                                     ; preds = %13
  %31 = bitcast %"class.std::__shared_count"* %9 to i8**
  store i8* %12, i8** %31, align 8, !tbaa !129, !alias.scope !265
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %10), !noalias !265
  %32 = bitcast i8* %12 to %"class.std::_Sp_counted_base"*
  %33 = bitcast i8* %12 to i8* (%"class.std::_Sp_counted_base"*, %"class.std::type_info"*)***
  %34 = load i8* (%"class.std::_Sp_counted_base"*, %"class.std::type_info"*)**, i8* (%"class.std::_Sp_counted_base"*, %"class.std::type_info"*)*** %33, align 8, !tbaa !133, !noalias !265
  %35 = getelementptr inbounds i8* (%"class.std::_Sp_counted_base"*, %"class.std::type_info"*)*, i8* (%"class.std::_Sp_counted_base"*, %"class.std::type_info"*)** %34, i64 4
  %36 = load i8* (%"class.std::_Sp_counted_base"*, %"class.std::type_info"*)*, i8* (%"class.std::_Sp_counted_base"*, %"class.std::type_info"*)** %35, align 8, !noalias !265
  %37 = call i8* %36(%"class.std::_Sp_counted_base"* nonnull %32, %"class.std::type_info"* nonnull dereferenceable(16) bitcast ({ i8*, i8* }* @_ZTISt19_Sp_make_shared_tag to %"class.std::type_info"*)) #8, !noalias !265
  %38 = bitcast <2 x i64>* %4 to i8**
  store i8* %37, i8** %38, align 16, !tbaa !187, !alias.scope !265
  call void (%"class.std::__shared_count"*, ...) @_ZSt32__enable_shared_from_this_helperILN9__gnu_cxx12_Lock_policyE2EEvRKSt14__shared_countIXT_EEz(%"class.std::__shared_count"* nonnull dereferenceable(8) %9, i8* %37, i8* %37) #8
  %39 = load <2 x i64>, <2 x i64>* %4, align 16, !tbaa !27, !noalias !262
  %40 = bitcast %"class.std::shared_ptr.107"* %0 to <2 x i64>*
  store <2 x i64> %39, <2 x i64>* %40, align 8, !tbaa !27, !alias.scope !262
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %7) #8, !noalias !262
  ret void
}

; Function Attrs: uwtable
define linkonce_odr void @_ZNSt23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EEC2IJS7_RKS8_EEES8_DpOT_(%"class.std::_Sp_counted_ptr_inplace"*, %"class.std::ios_base::Init"*, %"class.std::_Bind"* dereferenceable(24), %"class.std::ios_base::Init"* dereferenceable(1)) unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %5 = getelementptr inbounds %"class.std::_Sp_counted_ptr_inplace", %"class.std::_Sp_counted_ptr_inplace"* %0, i64 0, i32 0, i32 0
  %6 = getelementptr inbounds %"class.std::_Sp_counted_ptr_inplace", %"class.std::_Sp_counted_ptr_inplace"* %0, i64 0, i32 0, i32 1
  store i32 1, i32* %6, align 8, !tbaa !165
  %7 = getelementptr inbounds %"class.std::_Sp_counted_ptr_inplace", %"class.std::_Sp_counted_ptr_inplace"* %0, i64 0, i32 0, i32 2
  store i32 1, i32* %7, align 4, !tbaa !167
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [7 x i8*] }, { [7 x i8*] }* @_ZTVSt23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %5, align 8, !tbaa !133
  %8 = getelementptr inbounds %"class.std::_Sp_counted_ptr_inplace", %"class.std::_Sp_counted_ptr_inplace"* %0, i64 0, i32 1, i32 0
  %9 = bitcast %"struct.__gnu_cxx::__aligned_buffer"* %8 to i32 (...)***
  %10 = getelementptr inbounds %"class.std::_Sp_counted_ptr_inplace", %"class.std::_Sp_counted_ptr_inplace"* %0, i64 0, i32 1, i32 0, i32 0, i32 0, i64 8
  %11 = bitcast i8* %10 to %"struct.std::__future_base::_Result_base"**
  store %"struct.std::__future_base::_Result_base"* null, %"struct.std::__future_base::_Result_base"** %11, align 8, !tbaa !168
  %12 = getelementptr inbounds %"class.std::_Sp_counted_ptr_inplace", %"class.std::_Sp_counted_ptr_inplace"* %0, i64 0, i32 1, i32 0, i32 0, i32 0, i64 16
  %13 = bitcast i8* %12 to i32*
  store i32 0, i32* %13, align 4, !tbaa !170
  %14 = getelementptr inbounds %"class.std::_Sp_counted_ptr_inplace", %"class.std::_Sp_counted_ptr_inplace"* %0, i64 0, i32 1, i32 0, i32 0, i32 0, i64 20
  store i8 0, i8* %14, align 1, !tbaa !172
  %15 = getelementptr inbounds %"class.std::_Sp_counted_ptr_inplace", %"class.std::_Sp_counted_ptr_inplace"* %0, i64 0, i32 1, i32 0, i32 0, i32 0, i64 24
  %16 = bitcast i8* %15 to i32*
  store i32 0, i32* %16, align 4, !tbaa !174
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [9 x i8*] }, { [9 x i8*] }* @_ZTVNSt13__future_base16_Task_state_baseIFvvEEE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %9, align 8, !tbaa !133
  %17 = getelementptr inbounds %"class.std::_Sp_counted_ptr_inplace", %"class.std::_Sp_counted_ptr_inplace"* %0, i64 0, i32 1, i32 0, i32 0, i32 0, i64 32
  %18 = invoke i8* @_Znwm(i64 16) #24
          to label %19 unwind label %21

; <label>:19:                                     ; preds = %4
  %20 = bitcast i8* %18 to %"struct.std::__future_base::_Result_base"*
  invoke void @_ZNSt13__future_base12_Result_baseC2Ev(%"struct.std::__future_base::_Result_base"* nonnull %20)
          to label %34 unwind label %23, !noalias !268

; <label>:21:                                     ; preds = %4
  %22 = landingpad { i8*, i32 }
          cleanup
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [6 x i8*] }, { [6 x i8*] }* @_ZTVNSt13__future_base13_State_baseV2E, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %9, align 8, !tbaa !133
  br label %51

; <label>:23:                                     ; preds = %19
  %24 = landingpad { i8*, i32 }
          cleanup
  tail call void @_ZdlPv(i8* nonnull %18) #25, !noalias !268
  %25 = load %"struct.std::__future_base::_Result_base"*, %"struct.std::__future_base::_Result_base"** %11, align 8, !tbaa !27
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [6 x i8*] }, { [6 x i8*] }* @_ZTVNSt13__future_base13_State_baseV2E, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %9, align 8, !tbaa !133
  %26 = icmp eq %"struct.std::__future_base::_Result_base"* %25, null
  br i1 %26, label %51, label %27

; <label>:27:                                     ; preds = %23
  %28 = bitcast %"struct.std::__future_base::_Result_base"* %25 to void (%"struct.std::__future_base::_Result_base"*)***
  %29 = load void (%"struct.std::__future_base::_Result_base"*)**, void (%"struct.std::__future_base::_Result_base"*)*** %28, align 8, !tbaa !133
  %30 = load void (%"struct.std::__future_base::_Result_base"*)*, void (%"struct.std::__future_base::_Result_base"*)** %29, align 8
  invoke void %30(%"struct.std::__future_base::_Result_base"* nonnull %25)
          to label %51 unwind label %31

; <label>:31:                                     ; preds = %27
  %32 = landingpad { i8*, i32 }
          catch i8* null
  %33 = extractvalue { i8*, i32 } %32, 0
  tail call void @__clang_call_terminate(i8* %33) #22
  unreachable

; <label>:34:                                     ; preds = %19
  %35 = bitcast i8* %18 to i32 (...)***
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [5 x i8*] }, { [5 x i8*] }* @_ZTVNSt13__future_base7_ResultIvEE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %35, align 8, !tbaa !133, !noalias !268
  %36 = bitcast i8* %17 to i8**
  store i8* %18, i8** %36, align 8, !tbaa !27, !alias.scope !268
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [9 x i8*] }, { [9 x i8*] }* @_ZTVNSt13__future_base11_Task_stateISt5_BindIFPFvPvS2_ES2_S2_EESaIiEFvvEEE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %9, align 8, !tbaa !133
  %37 = getelementptr inbounds %"class.std::_Sp_counted_ptr_inplace", %"class.std::_Sp_counted_ptr_inplace"* %0, i64 0, i32 1, i32 0, i32 0, i32 0, i64 40
  %38 = bitcast %"class.std::_Bind"* %2 to i64*
  %39 = load i64, i64* %38, align 8, !tbaa !27
  %40 = bitcast i8* %37 to i64*
  store i64 %39, i64* %40, align 8, !tbaa !180
  %41 = getelementptr inbounds %"class.std::_Sp_counted_ptr_inplace", %"class.std::_Sp_counted_ptr_inplace"* %0, i64 0, i32 1, i32 0, i32 0, i32 0, i64 48
  %42 = getelementptr inbounds %"class.std::_Bind", %"class.std::_Bind"* %2, i64 0, i32 1
  %43 = bitcast %"class.std::tuple.102"* %42 to i64*
  %44 = load i64, i64* %43, align 8, !tbaa !27
  %45 = bitcast i8* %41 to i64*
  store i64 %44, i64* %45, align 8, !tbaa !183
  %46 = getelementptr inbounds %"class.std::_Sp_counted_ptr_inplace", %"class.std::_Sp_counted_ptr_inplace"* %0, i64 0, i32 1, i32 0, i32 0, i32 0, i64 56
  %47 = getelementptr inbounds %"class.std::_Bind", %"class.std::_Bind"* %2, i64 0, i32 1, i32 0, i32 1, i32 0
  %48 = bitcast i8** %47 to i64*
  %49 = load i64, i64* %48, align 8, !tbaa !27
  %50 = bitcast i8* %46 to i64*
  store i64 %49, i64* %50, align 8, !tbaa !185
  ret void

; <label>:51:                                     ; preds = %21, %23, %27
  %52 = phi { i8*, i32 } [ %22, %21 ], [ %24, %23 ], [ %24, %27 ]
  store %"struct.std::__future_base::_Result_base"* null, %"struct.std::__future_base::_Result_base"** %11, align 8, !tbaa !27
  resume { i8*, i32 } %52
}

; Function Attrs: uwtable
define linkonce_odr void @_ZNSt17_Function_handlerIFSt10unique_ptrINSt13__future_base12_Result_baseENS2_8_DeleterEEvENS1_12_Task_setterIS0_INS1_7_ResultIvEES3_ESt12_Bind_simpleIFSt17reference_wrapperISt5_BindIFPFvPvSD_ESD_SD_EEEvEEvEEE9_M_invokeERKSt9_Any_data(%"class.std::unique_ptr.58"* noalias sret, %"union.std::_Any_data"* dereferenceable(16)) #5 comdat align 2 personality i32 (...)* @__gxx_personality_v0 {
  %3 = alloca %"class.std::unique_ptr.109", align 8
  %4 = bitcast %"class.std::unique_ptr.109"* %3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %4) #8
  %5 = bitcast %"union.std::_Any_data"* %1 to %"struct.std::__future_base::_Task_setter"*
  call void @_ZNKSt13__future_base12_Task_setterISt10unique_ptrINS_7_ResultIvEENS_12_Result_base8_DeleterEESt12_Bind_simpleIFSt17reference_wrapperISt5_BindIFPFvPvSA_ESA_SA_EEEvEEvEclEv(%"class.std::unique_ptr.109"* nonnull sret %3, %"struct.std::__future_base::_Task_setter"* nonnull %5)
  %6 = bitcast %"class.std::unique_ptr.109"* %3 to i64*
  %7 = load i64, i64* %6, align 8, !tbaa !27
  %8 = bitcast %"class.std::unique_ptr.58"* %0 to i64*
  store i64 %7, i64* %8, align 8, !tbaa !168
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %4) #8
  ret void
}

; Function Attrs: uwtable
define linkonce_odr zeroext i1 @_ZNSt14_Function_base13_Base_managerINSt13__future_base12_Task_setterISt10unique_ptrINS1_7_ResultIvEENS1_12_Result_base8_DeleterEESt12_Bind_simpleIFSt17reference_wrapperISt5_BindIFPFvPvSC_ESC_SC_EEEvEEvEEE10_M_managerERSt9_Any_dataRKSM_St18_Manager_operation(%"union.std::_Any_data"* dereferenceable(16), %"union.std::_Any_data"* dereferenceable(16), i32) #5 comdat align 2 {
  switch i32 %2, label %11 [
    i32 0, label %4
    i32 1, label %6
    i32 2, label %8
  ]

; <label>:4:                                      ; preds = %3
  %5 = bitcast %"union.std::_Any_data"* %0 to %"class.std::type_info"**
  store %"class.std::type_info"* bitcast ({ i8*, i8* }* @_ZTINSt13__future_base12_Task_setterISt10unique_ptrINS_7_ResultIvEENS_12_Result_base8_DeleterEESt12_Bind_simpleIFSt17reference_wrapperISt5_BindIFPFvPvSA_ESA_SA_EEEvEEvEE to %"class.std::type_info"*), %"class.std::type_info"** %5, align 8, !tbaa !27
  br label %11

; <label>:6:                                      ; preds = %3
  %7 = bitcast %"union.std::_Any_data"* %0 to %"union.std::_Any_data"**
  store %"union.std::_Any_data"* %1, %"union.std::_Any_data"** %7, align 8, !tbaa !27
  br label %11

; <label>:8:                                      ; preds = %3
  %9 = bitcast %"union.std::_Any_data"* %0 to i8*
  %10 = bitcast %"union.std::_Any_data"* %1 to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %9, i8* nonnull %10, i64 16, i32 8, i1 false), !tbaa.struct !271
  br label %11

; <label>:11:                                     ; preds = %3, %8, %6, %4
  ret i1 false
}

; Function Attrs: uwtable
define linkonce_odr void @_ZNSt13__future_base13_State_baseV221_M_set_delayed_resultESt8functionIFSt10unique_ptrINS_12_Result_baseENS3_8_DeleterEEvEESt8weak_ptrIS0_E(%"class.std::__future_base::_State_baseV2"*, %"class.std::function.133"*, %"class.std::shared_ptr"*) local_unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %4 = alloca %class.anon.135, align 8
  %5 = alloca i8, align 1
  %6 = alloca <2 x i64>, align 16
  %7 = alloca %"class.std::__future_base::_State_baseV2"*, align 8
  %8 = alloca %"class.std::function.133"*, align 8
  %9 = alloca i8*, align 8
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %5) #8
  store i8 0, i8* %5, align 1, !tbaa !256
  %10 = tail call i8* @_Znwm(i64 32) #24
  %11 = bitcast i8* %10 to %"struct.std::__future_base::_State_baseV2::_Make_ready"*
  %12 = getelementptr inbounds i8, i8* %10, i64 16
  tail call void @llvm.memset.p0i8.i64(i8* %12, i8 0, i64 16, i32 8, i1 false) #8
  %13 = bitcast <2 x i64>* %6 to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %13) #8
  store <2 x i64> <i64 ptrtoint (void (%"class.std::__future_base::_State_baseV2"*, %"class.std::function.133"*, i8*)* @_ZNSt13__future_base13_State_baseV29_M_do_setEPSt8functionIFSt10unique_ptrINS_12_Result_baseENS3_8_DeleterEEvEEPb to i64), i64 0>, <2 x i64>* %6, align 16
  %14 = bitcast %"class.std::__future_base::_State_baseV2"** %7 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %14) #8
  store %"class.std::__future_base::_State_baseV2"* %0, %"class.std::__future_base::_State_baseV2"** %7, align 8, !tbaa !27
  %15 = bitcast %"class.std::function.133"** %8 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %15) #8
  store %"class.std::function.133"* %1, %"class.std::function.133"** %8, align 8, !tbaa !27
  %16 = bitcast i8** %9 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %16) #8
  store i8* %5, i8** %9, align 8, !tbaa !27
  %17 = bitcast %class.anon.135* %4 to i8*
  call void @llvm.lifetime.start.p0i8(i64 32, i8* nonnull %17) #8
  %18 = bitcast %class.anon.135* %4 to <2 x i64>**
  store <2 x i64>* %6, <2 x i64>** %18, align 8, !tbaa !18
  %19 = getelementptr inbounds %class.anon.135, %class.anon.135* %4, i64 0, i32 1
  store %"class.std::__future_base::_State_baseV2"** %7, %"class.std::__future_base::_State_baseV2"*** %19, align 8, !tbaa !18
  %20 = getelementptr inbounds %class.anon.135, %class.anon.135* %4, i64 0, i32 2
  store %"class.std::function.133"** %8, %"class.std::function.133"*** %20, align 8, !tbaa !18
  %21 = getelementptr inbounds %class.anon.135, %class.anon.135* %4, i64 0, i32 3
  store i8** %9, i8*** %21, align 8, !tbaa !18
  store %class.anon.135* %4, %class.anon.135** bitcast (i8** @_ZSt15__once_callable to %class.anon.135**), align 8, !tbaa !27
  store void ()* @_ZZSt9call_onceIMNSt13__future_base13_State_baseV2EFvPSt8functionIFSt10unique_ptrINS0_12_Result_baseENS4_8_DeleterEEvEEPbEJPS1_S9_SA_EEvRSt9once_flagOT_DpOT0_ENUlvE0_8__invokeEv, void ()** @_ZSt11__once_call, align 8, !tbaa !27
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %22, label %27

; <label>:22:                                     ; preds = %3
  %23 = getelementptr inbounds %"class.std::__future_base::_State_baseV2", %"class.std::__future_base::_State_baseV2"* %0, i64 0, i32 5, i32 0
  %24 = invoke i32 @pthread_once(i32* nonnull %23, void ()* nonnull @__once_proxy)
          to label %25 unwind label %35

; <label>:25:                                     ; preds = %22
  %26 = icmp eq i32 %24, 0
  br i1 %26, label %30, label %27

; <label>:27:                                     ; preds = %25, %3
  %28 = phi i32 [ %24, %25 ], [ -1, %3 ]
  invoke void @_ZSt20__throw_system_errori(i32 %28) #23
          to label %29 unwind label %35

; <label>:29:                                     ; preds = %27
  unreachable

; <label>:30:                                     ; preds = %25
  call void @llvm.lifetime.end.p0i8(i64 32, i8* nonnull %17) #8
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %16) #8
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %15) #8
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %14) #8
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %13) #8
  %31 = load i8, i8* %5, align 1, !tbaa !256, !range !43
  %32 = icmp eq i8 %31, 0
  br i1 %32, label %33, label %43

; <label>:33:                                     ; preds = %30
  invoke void @_ZSt20__throw_future_errori(i32 2) #23
          to label %34 unwind label %39

; <label>:34:                                     ; preds = %33
  unreachable

; <label>:35:                                     ; preds = %27, %22
  %36 = landingpad { i8*, i32 }
          cleanup
  %37 = extractvalue { i8*, i32 } %36, 0
  %38 = extractvalue { i8*, i32 } %36, 1
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %16) #8
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %15) #8
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %14) #8
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %13) #8
  br label %73

; <label>:39:                                     ; preds = %67, %33
  %40 = landingpad { i8*, i32 }
          cleanup
  %41 = extractvalue { i8*, i32 } %40, 0
  %42 = extractvalue { i8*, i32 } %40, 1
  br label %73

; <label>:43:                                     ; preds = %30
  %44 = bitcast %"class.std::shared_ptr"* %2 to i64*
  %45 = load i64, i64* %44, align 8, !tbaa !272
  %46 = bitcast i8* %12 to i64*
  store i64 %45, i64* %46, align 8, !tbaa !272
  %47 = getelementptr inbounds %"class.std::shared_ptr", %"class.std::shared_ptr"* %2, i64 0, i32 0, i32 1
  %48 = getelementptr inbounds i8, i8* %10, i64 24
  %49 = bitcast i8* %48 to %"class.std::_Sp_counted_base"**
  %50 = load %"class.std::_Sp_counted_base"*, %"class.std::_Sp_counted_base"** %49, align 8, !tbaa !260
  %51 = icmp eq %"class.std::_Sp_counted_base"* %50, null
  br i1 %51, label %67, label %52

; <label>:52:                                     ; preds = %43
  %53 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %50, i64 0, i32 2
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %54, label %56

; <label>:54:                                     ; preds = %52
  %55 = atomicrmw volatile add i32* %53, i32 -1 acq_rel
  br label %59

; <label>:56:                                     ; preds = %52
  %57 = load i32, i32* %53, align 4, !tbaa !17
  %58 = add nsw i32 %57, -1
  store i32 %58, i32* %53, align 4, !tbaa !17
  br label %59

; <label>:59:                                     ; preds = %56, %54
  %60 = phi i32 [ %55, %54 ], [ %57, %56 ]
  %61 = icmp eq i32 %60, 1
  br i1 %61, label %62, label %67

; <label>:62:                                     ; preds = %59
  %63 = bitcast %"class.std::_Sp_counted_base"* %50 to void (%"class.std::_Sp_counted_base"*)***
  %64 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %63, align 8, !tbaa !133
  %65 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %64, i64 3
  %66 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %65, align 8
  call void %66(%"class.std::_Sp_counted_base"* nonnull %50) #8
  br label %67

; <label>:67:                                     ; preds = %43, %59, %62
  %68 = bitcast %"class.std::__shared_count"* %47 to i64*
  %69 = load i64, i64* %68, align 8, !tbaa !260
  %70 = bitcast i8* %48 to i64*
  store i64 %69, i64* %70, align 8, !tbaa !260
  %71 = bitcast %"class.std::shared_ptr"* %2 to i8*
  call void @llvm.memset.p0i8.i64(i8* nonnull %71, i8 0, i64 16, i32 8, i1 false) #8
  invoke void @_ZNSt13__future_base13_State_baseV211_Make_ready6_M_setEv(%"struct.std::__future_base::_State_baseV2::_Make_ready"* nonnull %11)
          to label %72 unwind label %39

; <label>:72:                                     ; preds = %67
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %5) #8
  ret void

; <label>:73:                                     ; preds = %35, %39
  %74 = phi i8* [ %41, %39 ], [ %37, %35 ]
  %75 = phi i32 [ %42, %39 ], [ %38, %35 ]
  %76 = getelementptr inbounds i8, i8* %10, i64 24
  %77 = bitcast i8* %76 to %"class.std::_Sp_counted_base"**
  %78 = load %"class.std::_Sp_counted_base"*, %"class.std::_Sp_counted_base"** %77, align 8, !tbaa !260
  %79 = icmp eq %"class.std::_Sp_counted_base"* %78, null
  br i1 %79, label %95, label %80

; <label>:80:                                     ; preds = %73
  %81 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %78, i64 0, i32 2
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %82, label %84

; <label>:82:                                     ; preds = %80
  %83 = atomicrmw volatile add i32* %81, i32 -1 acq_rel
  br label %87

; <label>:84:                                     ; preds = %80
  %85 = load i32, i32* %81, align 4, !tbaa !17
  %86 = add nsw i32 %85, -1
  store i32 %86, i32* %81, align 4, !tbaa !17
  br label %87

; <label>:87:                                     ; preds = %84, %82
  %88 = phi i32 [ %83, %82 ], [ %85, %84 ]
  %89 = icmp eq i32 %88, 1
  br i1 %89, label %90, label %95

; <label>:90:                                     ; preds = %87
  %91 = bitcast %"class.std::_Sp_counted_base"* %78 to void (%"class.std::_Sp_counted_base"*)***
  %92 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %91, align 8, !tbaa !133
  %93 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %92, i64 3
  %94 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %93, align 8
  call void %94(%"class.std::_Sp_counted_base"* nonnull %78) #8
  br label %95

; <label>:95:                                     ; preds = %73, %87, %90
  call void @_ZdlPv(i8* nonnull %10) #25
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %5) #8
  %96 = insertvalue { i8*, i32 } undef, i8* %74, 0
  %97 = insertvalue { i8*, i32 } %96, i32 %75, 1
  resume { i8*, i32 } %97
}

; Function Attrs: uwtable
define linkonce_odr void @_ZNSt13__future_base13_State_baseV29_M_do_setEPSt8functionIFSt10unique_ptrINS_12_Result_baseENS3_8_DeleterEEvEEPb(%"class.std::__future_base::_State_baseV2"*, %"class.std::function.133"*, i8*) #5 comdat align 2 personality i32 (...)* @__gxx_personality_v0 {
  %4 = alloca %"class.std::unique_ptr.58", align 8
  %5 = bitcast %"class.std::unique_ptr.58"* %4 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %5) #8
  %6 = getelementptr inbounds %"class.std::function.133", %"class.std::function.133"* %1, i64 0, i32 0, i32 1
  %7 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %6, align 8, !tbaa !123, !noalias !274
  %8 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %7, null
  br i1 %8, label %9, label %10

; <label>:9:                                      ; preds = %3
  tail call void @_ZSt25__throw_bad_function_callv() #23
  unreachable

; <label>:10:                                     ; preds = %3
  %11 = getelementptr inbounds %"class.std::function.133", %"class.std::function.133"* %1, i64 0, i32 1
  %12 = load void (%"class.std::unique_ptr.58"*, %"union.std::_Any_data"*)*, void (%"class.std::unique_ptr.58"*, %"union.std::_Any_data"*)** %11, align 8, !tbaa !254, !noalias !274
  %13 = getelementptr inbounds %"class.std::function.133", %"class.std::function.133"* %1, i64 0, i32 0, i32 0
  call void %12(%"class.std::unique_ptr.58"* nonnull sret %4, %"union.std::_Any_data"* dereferenceable(16) %13)
  store i8 1, i8* %2, align 1, !tbaa !256
  %14 = getelementptr inbounds %"class.std::__future_base::_State_baseV2", %"class.std::__future_base::_State_baseV2"* %0, i64 0, i32 1
  %15 = bitcast %"class.std::unique_ptr.58"* %14 to i64*
  %16 = load i64, i64* %15, align 8, !tbaa !27
  %17 = bitcast %"class.std::unique_ptr.58"* %4 to i64*
  %18 = load i64, i64* %17, align 8, !tbaa !27
  store i64 %18, i64* %15, align 8, !tbaa !27
  store i64 %16, i64* %17, align 8, !tbaa !27
  %19 = icmp eq i64 %16, 0
  br i1 %19, label %28, label %20

; <label>:20:                                     ; preds = %10
  %21 = inttoptr i64 %16 to %"struct.std::__future_base::_Result_base"*
  %22 = inttoptr i64 %16 to void (%"struct.std::__future_base::_Result_base"*)***
  %23 = load void (%"struct.std::__future_base::_Result_base"*)**, void (%"struct.std::__future_base::_Result_base"*)*** %22, align 8, !tbaa !133
  %24 = load void (%"struct.std::__future_base::_Result_base"*)*, void (%"struct.std::__future_base::_Result_base"*)** %23, align 8
  invoke void %24(%"struct.std::__future_base::_Result_base"* nonnull %21)
          to label %28 unwind label %25

; <label>:25:                                     ; preds = %20
  %26 = landingpad { i8*, i32 }
          catch i8* null
  %27 = extractvalue { i8*, i32 } %26, 0
  call void @__clang_call_terminate(i8* %27) #22
  unreachable

; <label>:28:                                     ; preds = %10, %20
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %5) #8
  ret void
}

; Function Attrs: inlinehint uwtable
define linkonce_odr void @_ZZSt9call_onceIMNSt13__future_base13_State_baseV2EFvPSt8functionIFSt10unique_ptrINS0_12_Result_baseENS4_8_DeleterEEvEEPbEJPS1_S9_SA_EEvRSt9once_flagOT_DpOT0_ENUlvE0_8__invokeEv() #14 comdat align 2 {
  %1 = load %class.anon.135*, %class.anon.135** bitcast (i8** @_ZSt15__once_callable to %class.anon.135**), align 8, !tbaa !27
  %2 = getelementptr inbounds %class.anon.135, %class.anon.135* %1, i64 0, i32 0
  %3 = load { i64, i64 }*, { i64, i64 }** %2, align 8, !tbaa !18
  %4 = getelementptr inbounds %class.anon.135, %class.anon.135* %1, i64 0, i32 1
  %5 = bitcast %"class.std::__future_base::_State_baseV2"*** %4 to i8***
  %6 = load i8**, i8*** %5, align 8, !tbaa !18
  %7 = getelementptr inbounds %class.anon.135, %class.anon.135* %1, i64 0, i32 2
  %8 = load %"class.std::function.133"**, %"class.std::function.133"*** %7, align 8, !tbaa !18
  %9 = getelementptr inbounds %class.anon.135, %class.anon.135* %1, i64 0, i32 3
  %10 = load i8**, i8*** %9, align 8, !tbaa !18
  %11 = load i8*, i8** %6, align 8, !tbaa !27
  %12 = getelementptr inbounds { i64, i64 }, { i64, i64 }* %3, i64 0, i32 0
  %13 = load i64, i64* %12, align 8, !tbaa !18
  %14 = getelementptr inbounds { i64, i64 }, { i64, i64 }* %3, i64 0, i32 1
  %15 = load i64, i64* %14, align 8, !tbaa !18
  %16 = getelementptr inbounds i8, i8* %11, i64 %15
  %17 = bitcast i8* %16 to %"class.std::__future_base::_State_baseV2"*
  %18 = and i64 %13, 1
  %19 = icmp eq i64 %18, 0
  br i1 %19, label %27, label %20

; <label>:20:                                     ; preds = %0
  %21 = bitcast i8* %16 to i8**
  %22 = load i8*, i8** %21, align 8, !tbaa !133
  %23 = add i64 %13, -1
  %24 = getelementptr i8, i8* %22, i64 %23
  %25 = bitcast i8* %24 to void (%"class.std::__future_base::_State_baseV2"*, %"class.std::function.133"*, i8*)**
  %26 = load void (%"class.std::__future_base::_State_baseV2"*, %"class.std::function.133"*, i8*)*, void (%"class.std::__future_base::_State_baseV2"*, %"class.std::function.133"*, i8*)** %25, align 8
  br label %29

; <label>:27:                                     ; preds = %0
  %28 = inttoptr i64 %13 to void (%"class.std::__future_base::_State_baseV2"*, %"class.std::function.133"*, i8*)*
  br label %29

; <label>:29:                                     ; preds = %20, %27
  %30 = phi void (%"class.std::__future_base::_State_baseV2"*, %"class.std::function.133"*, i8*)* [ %26, %20 ], [ %28, %27 ]
  %31 = load %"class.std::function.133"*, %"class.std::function.133"** %8, align 8, !tbaa !27
  %32 = load i8*, i8** %10, align 8, !tbaa !27
  tail call void %30(%"class.std::__future_base::_State_baseV2"* %17, %"class.std::function.133"* %31, i8* %32)
  ret void
}

declare void @__once_proxy() #6

declare extern_weak i32 @pthread_once(i32*, void ()*) local_unnamed_addr #6

declare void @_ZNSt13__future_base13_State_baseV211_Make_ready6_M_setEv(%"struct.std::__future_base::_State_baseV2::_Make_ready"*) local_unnamed_addr #6

; Function Attrs: uwtable
define linkonce_odr void @_ZNKSt13__future_base12_Task_setterISt10unique_ptrINS_7_ResultIvEENS_12_Result_base8_DeleterEESt12_Bind_simpleIFSt17reference_wrapperISt5_BindIFPFvPvSA_ESA_SA_EEEvEEvEclEv(%"class.std::unique_ptr.109"* noalias sret, %"struct.std::__future_base::_Task_setter"*) local_unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %3 = alloca %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider", align 8
  %4 = alloca %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider", align 8
  %5 = getelementptr inbounds %"struct.std::__future_base::_Task_setter", %"struct.std::__future_base::_Task_setter"* %1, i64 0, i32 1
  %6 = load %"struct.std::_Bind_simple.127"*, %"struct.std::_Bind_simple.127"** %5, align 8, !tbaa !277
  %7 = getelementptr inbounds %"struct.std::_Bind_simple.127", %"struct.std::_Bind_simple.127"* %6, i64 0, i32 0, i32 0, i32 0, i32 0, i32 0
  %8 = load %"class.std::_Bind"*, %"class.std::_Bind"** %7, align 8, !tbaa !279
  %9 = getelementptr inbounds %"class.std::_Bind", %"class.std::_Bind"* %8, i64 0, i32 0
  %10 = load void (i8*, i8*)*, void (i8*, i8*)** %9, align 8, !tbaa !180
  %11 = getelementptr inbounds %"class.std::_Bind", %"class.std::_Bind"* %8, i64 0, i32 1, i32 0, i32 1, i32 0
  %12 = load i8*, i8** %11, align 8, !tbaa !27
  %13 = getelementptr inbounds %"class.std::_Bind", %"class.std::_Bind"* %8, i64 0, i32 1, i32 0, i32 0, i32 0, i32 0
  %14 = load i8*, i8** %13, align 8, !tbaa !27
  invoke void %10(i8* %12, i8* %14)
          to label %15 unwind label %17

; <label>:15:                                     ; preds = %2
  %16 = getelementptr inbounds %"struct.std::__future_base::_Task_setter", %"struct.std::__future_base::_Task_setter"* %1, i64 0, i32 0
  br label %40

; <label>:17:                                     ; preds = %2
  %18 = landingpad { i8*, i32 }
          catch i8* bitcast (i8** @_ZTIN10__cxxabiv115__forced_unwindE to i8*)
          catch i8* null
  %19 = extractvalue { i8*, i32 } %18, 0
  %20 = extractvalue { i8*, i32 } %18, 1
  %21 = tail call i32 @llvm.eh.typeid.for(i8* bitcast (i8** @_ZTIN10__cxxabiv115__forced_unwindE to i8*)) #8
  %22 = icmp eq i32 %20, %21
  %23 = tail call i8* @__cxa_begin_catch(i8* %19) #8
  br i1 %22, label %24, label %25

; <label>:24:                                     ; preds = %17
  invoke void @__cxa_rethrow() #23
          to label %50 unwind label %37

; <label>:25:                                     ; preds = %17
  %26 = bitcast %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* %4 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %26) #8
  call void @_ZSt17current_exceptionv(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* nonnull sret %4) #8
  %27 = getelementptr inbounds %"struct.std::__future_base::_Task_setter", %"struct.std::__future_base::_Task_setter"* %1, i64 0, i32 0
  %28 = load %"class.std::unique_ptr.109"*, %"class.std::unique_ptr.109"** %27, align 8, !tbaa !281
  %29 = getelementptr inbounds %"class.std::unique_ptr.109", %"class.std::unique_ptr.109"* %28, i64 0, i32 0, i32 0, i32 0, i32 0
  %30 = load %"struct.std::__future_base::_Result"*, %"struct.std::__future_base::_Result"** %29, align 8, !tbaa !27
  %31 = getelementptr inbounds %"struct.std::__future_base::_Result", %"struct.std::__future_base::_Result"* %30, i64 0, i32 0, i32 1
  %32 = bitcast %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* %3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %32) #8
  %33 = getelementptr inbounds %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider", %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* %4, i64 0, i32 0
  %34 = bitcast %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* %4 to i64*
  %35 = load i64, i64* %34, align 8, !tbaa !240
  %36 = bitcast %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* %3 to i64*
  store i64 %35, i64* %36, align 8, !tbaa !240
  store i8* null, i8** %33, align 8, !tbaa !240
  call void @_ZNSt15__exception_ptr13exception_ptr4swapERS0_(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* nonnull %3, %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* dereferenceable(8) %31) #8
  call void @_ZNSt15__exception_ptr13exception_ptrD1Ev(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* nonnull %3) #8
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %32) #8
  call void @_ZNSt15__exception_ptr13exception_ptrD1Ev(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* nonnull %4) #8
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %26) #8
  call void @__cxa_end_catch()
  br label %40

; <label>:37:                                     ; preds = %24
  %38 = landingpad { i8*, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %39 unwind label %47

; <label>:39:                                     ; preds = %37
  resume { i8*, i32 } %38

; <label>:40:                                     ; preds = %15, %25
  %41 = phi %"class.std::unique_ptr.109"** [ %16, %15 ], [ %27, %25 ]
  %42 = load %"class.std::unique_ptr.109"*, %"class.std::unique_ptr.109"** %41, align 8, !tbaa !281
  %43 = getelementptr inbounds %"class.std::unique_ptr.109", %"class.std::unique_ptr.109"* %42, i64 0, i32 0, i32 0, i32 0, i32 0
  %44 = bitcast %"class.std::unique_ptr.109"* %42 to i64*
  %45 = load i64, i64* %44, align 8, !tbaa !27
  store %"struct.std::__future_base::_Result"* null, %"struct.std::__future_base::_Result"** %43, align 8, !tbaa !27
  %46 = bitcast %"class.std::unique_ptr.109"* %0 to i64*
  store i64 %45, i64* %46, align 8, !tbaa !282
  ret void

; <label>:47:                                     ; preds = %37
  %48 = landingpad { i8*, i32 }
          catch i8* null
  %49 = extractvalue { i8*, i32 } %48, 0
  tail call void @__clang_call_terminate(i8* %49) #22
  unreachable

; <label>:50:                                     ; preds = %24
  unreachable
}

; Function Attrs: nounwind readnone
declare i32 @llvm.eh.typeid.for(i8*) #21

declare void @__cxa_rethrow() local_unnamed_addr

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZNSt13__future_base7_ResultIvE10_M_destroyEv(%"struct.std::__future_base::_Result"*) unnamed_addr #10 comdat align 2 {
  %2 = icmp eq %"struct.std::__future_base::_Result"* %0, null
  br i1 %2, label %8, label %3

; <label>:3:                                      ; preds = %1
  %4 = bitcast %"struct.std::__future_base::_Result"* %0 to void (%"struct.std::__future_base::_Result"*)***
  %5 = load void (%"struct.std::__future_base::_Result"*)**, void (%"struct.std::__future_base::_Result"*)*** %4, align 8, !tbaa !133
  %6 = getelementptr inbounds void (%"struct.std::__future_base::_Result"*)*, void (%"struct.std::__future_base::_Result"*)** %5, i64 2
  %7 = load void (%"struct.std::__future_base::_Result"*)*, void (%"struct.std::__future_base::_Result"*)** %6, align 8
  tail call void %7(%"struct.std::__future_base::_Result"* nonnull %0) #8
  br label %8

; <label>:8:                                      ; preds = %3, %1
  ret void
}

; Function Attrs: nounwind
declare void @_ZNSt13__future_base12_Result_baseD2Ev(%"struct.std::__future_base::_Result_base"*) unnamed_addr #7

; Function Attrs: inlinehint nounwind uwtable
define linkonce_odr void @_ZNSt13__future_base7_ResultIvED0Ev(%"struct.std::__future_base::_Result"*) unnamed_addr #17 comdat align 2 {
  %2 = getelementptr inbounds %"struct.std::__future_base::_Result", %"struct.std::__future_base::_Result"* %0, i64 0, i32 0
  tail call void @_ZNSt13__future_base12_Result_baseD2Ev(%"struct.std::__future_base::_Result_base"* %2) #8
  %3 = bitcast %"struct.std::__future_base::_Result"* %0 to i8*
  tail call void @_ZdlPv(i8* %3) #25
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZNSt13__future_base13_State_baseV2D2Ev(%"class.std::__future_base::_State_baseV2"*) unnamed_addr #10 comdat align 2 personality i32 (...)* @__gxx_personality_v0 {
  %2 = getelementptr inbounds %"class.std::__future_base::_State_baseV2", %"class.std::__future_base::_State_baseV2"* %0, i64 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [6 x i8*] }, { [6 x i8*] }* @_ZTVNSt13__future_base13_State_baseV2E, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %2, align 8, !tbaa !133
  %3 = getelementptr inbounds %"class.std::__future_base::_State_baseV2", %"class.std::__future_base::_State_baseV2"* %0, i64 0, i32 1, i32 0, i32 0, i32 0, i32 0
  %4 = load %"struct.std::__future_base::_Result_base"*, %"struct.std::__future_base::_Result_base"** %3, align 8, !tbaa !27
  %5 = icmp eq %"struct.std::__future_base::_Result_base"* %4, null
  br i1 %5, label %13, label %6

; <label>:6:                                      ; preds = %1
  %7 = bitcast %"struct.std::__future_base::_Result_base"* %4 to void (%"struct.std::__future_base::_Result_base"*)***
  %8 = load void (%"struct.std::__future_base::_Result_base"*)**, void (%"struct.std::__future_base::_Result_base"*)*** %7, align 8, !tbaa !133
  %9 = load void (%"struct.std::__future_base::_Result_base"*)*, void (%"struct.std::__future_base::_Result_base"*)** %8, align 8
  invoke void %9(%"struct.std::__future_base::_Result_base"* nonnull %4)
          to label %13 unwind label %10

; <label>:10:                                     ; preds = %6
  %11 = landingpad { i8*, i32 }
          catch i8* null
  %12 = extractvalue { i8*, i32 } %11, 0
  tail call void @__clang_call_terminate(i8* %12) #22
  unreachable

; <label>:13:                                     ; preds = %1, %6
  store %"struct.std::__future_base::_Result_base"* null, %"struct.std::__future_base::_Result_base"** %3, align 8, !tbaa !27
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZNSt13__future_base13_State_baseV2D0Ev(%"class.std::__future_base::_State_baseV2"*) unnamed_addr #10 comdat align 2 personality i32 (...)* @__gxx_personality_v0 {
  %2 = getelementptr inbounds %"class.std::__future_base::_State_baseV2", %"class.std::__future_base::_State_baseV2"* %0, i64 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [6 x i8*] }, { [6 x i8*] }* @_ZTVNSt13__future_base13_State_baseV2E, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %2, align 8, !tbaa !133
  %3 = getelementptr inbounds %"class.std::__future_base::_State_baseV2", %"class.std::__future_base::_State_baseV2"* %0, i64 0, i32 1, i32 0, i32 0, i32 0, i32 0
  %4 = load %"struct.std::__future_base::_Result_base"*, %"struct.std::__future_base::_Result_base"** %3, align 8, !tbaa !27
  %5 = icmp eq %"struct.std::__future_base::_Result_base"* %4, null
  br i1 %5, label %13, label %6

; <label>:6:                                      ; preds = %1
  %7 = bitcast %"struct.std::__future_base::_Result_base"* %4 to void (%"struct.std::__future_base::_Result_base"*)***
  %8 = load void (%"struct.std::__future_base::_Result_base"*)**, void (%"struct.std::__future_base::_Result_base"*)*** %7, align 8, !tbaa !133
  %9 = load void (%"struct.std::__future_base::_Result_base"*)*, void (%"struct.std::__future_base::_Result_base"*)** %8, align 8
  invoke void %9(%"struct.std::__future_base::_Result_base"* nonnull %4)
          to label %13 unwind label %10

; <label>:10:                                     ; preds = %6
  %11 = landingpad { i8*, i32 }
          catch i8* null
  %12 = extractvalue { i8*, i32 } %11, 0
  tail call void @__clang_call_terminate(i8* %12) #22
  unreachable

; <label>:13:                                     ; preds = %1, %6
  %14 = bitcast %"class.std::__future_base::_State_baseV2"* %0 to i8*
  tail call void @_ZdlPv(i8* %14) #25
  ret void
}

; Function Attrs: inlinehint nounwind uwtable
define linkonce_odr void @_ZNSt13__future_base16_Task_state_baseIFvvEED2Ev(%"struct.std::__future_base::_Task_state_base"*) unnamed_addr #17 comdat align 2 personality i32 (...)* @__gxx_personality_v0 {
  %2 = getelementptr inbounds %"struct.std::__future_base::_Task_state_base", %"struct.std::__future_base::_Task_state_base"* %0, i64 0, i32 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [9 x i8*] }, { [9 x i8*] }* @_ZTVNSt13__future_base16_Task_state_baseIFvvEEE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %2, align 8, !tbaa !133
  %3 = getelementptr inbounds %"struct.std::__future_base::_Task_state_base", %"struct.std::__future_base::_Task_state_base"* %0, i64 0, i32 1, i32 0, i32 0, i32 0, i32 0
  %4 = load %"struct.std::__future_base::_Result"*, %"struct.std::__future_base::_Result"** %3, align 8, !tbaa !27
  %5 = icmp eq %"struct.std::__future_base::_Result"* %4, null
  br i1 %5, label %14, label %6

; <label>:6:                                      ; preds = %1
  %7 = getelementptr inbounds %"struct.std::__future_base::_Result", %"struct.std::__future_base::_Result"* %4, i64 0, i32 0
  %8 = bitcast %"struct.std::__future_base::_Result"* %4 to void (%"struct.std::__future_base::_Result_base"*)***
  %9 = load void (%"struct.std::__future_base::_Result_base"*)**, void (%"struct.std::__future_base::_Result_base"*)*** %8, align 8, !tbaa !133
  %10 = load void (%"struct.std::__future_base::_Result_base"*)*, void (%"struct.std::__future_base::_Result_base"*)** %9, align 8
  invoke void %10(%"struct.std::__future_base::_Result_base"* %7)
          to label %14 unwind label %11

; <label>:11:                                     ; preds = %6
  %12 = landingpad { i8*, i32 }
          catch i8* null
  %13 = extractvalue { i8*, i32 } %12, 0
  tail call void @__clang_call_terminate(i8* %13) #22
  unreachable

; <label>:14:                                     ; preds = %1, %6
  store %"struct.std::__future_base::_Result"* null, %"struct.std::__future_base::_Result"** %3, align 8, !tbaa !27
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [6 x i8*] }, { [6 x i8*] }* @_ZTVNSt13__future_base13_State_baseV2E, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %2, align 8, !tbaa !133
  %15 = getelementptr inbounds %"struct.std::__future_base::_Task_state_base", %"struct.std::__future_base::_Task_state_base"* %0, i64 0, i32 0, i32 1, i32 0, i32 0, i32 0, i32 0
  %16 = load %"struct.std::__future_base::_Result_base"*, %"struct.std::__future_base::_Result_base"** %15, align 8, !tbaa !27
  %17 = icmp eq %"struct.std::__future_base::_Result_base"* %16, null
  br i1 %17, label %25, label %18

; <label>:18:                                     ; preds = %14
  %19 = bitcast %"struct.std::__future_base::_Result_base"* %16 to void (%"struct.std::__future_base::_Result_base"*)***
  %20 = load void (%"struct.std::__future_base::_Result_base"*)**, void (%"struct.std::__future_base::_Result_base"*)*** %19, align 8, !tbaa !133
  %21 = load void (%"struct.std::__future_base::_Result_base"*)*, void (%"struct.std::__future_base::_Result_base"*)** %20, align 8
  invoke void %21(%"struct.std::__future_base::_Result_base"* nonnull %16)
          to label %25 unwind label %22

; <label>:22:                                     ; preds = %18
  %23 = landingpad { i8*, i32 }
          catch i8* null
  %24 = extractvalue { i8*, i32 } %23, 0
  tail call void @__clang_call_terminate(i8* %24) #22
  unreachable

; <label>:25:                                     ; preds = %14, %18
  store %"struct.std::__future_base::_Result_base"* null, %"struct.std::__future_base::_Result_base"** %15, align 8, !tbaa !27
  ret void
}

; Function Attrs: inlinehint nounwind uwtable
define linkonce_odr void @_ZNSt13__future_base16_Task_state_baseIFvvEED0Ev(%"struct.std::__future_base::_Task_state_base"*) unnamed_addr #17 comdat align 2 {
  tail call void @llvm.trap() #22
  unreachable
}

declare void @__cxa_pure_virtual() unnamed_addr

; Function Attrs: noreturn nounwind
declare void @llvm.trap() #22

; Function Attrs: norecurse nounwind uwtable
define linkonce_odr void @_ZNSt23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EED2Ev(%"class.std::_Sp_counted_ptr_inplace"*) unnamed_addr #20 comdat align 2 {
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZNSt23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EED0Ev(%"class.std::_Sp_counted_ptr_inplace"*) unnamed_addr #10 comdat align 2 {
  %2 = bitcast %"class.std::_Sp_counted_ptr_inplace"* %0 to i8*
  tail call void @_ZdlPv(i8* %2) #25
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZNSt23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EE10_M_disposeEv(%"class.std::_Sp_counted_ptr_inplace"*) unnamed_addr #10 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %2 = getelementptr inbounds %"class.std::_Sp_counted_ptr_inplace", %"class.std::_Sp_counted_ptr_inplace"* %0, i64 0, i32 1, i32 0
  %3 = bitcast %"struct.__gnu_cxx::__aligned_buffer"* %2 to i32 (...)***
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [9 x i8*] }, { [9 x i8*] }* @_ZTVNSt13__future_base16_Task_state_baseIFvvEEE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %3, align 8, !tbaa !133
  %4 = getelementptr inbounds %"class.std::_Sp_counted_ptr_inplace", %"class.std::_Sp_counted_ptr_inplace"* %0, i64 0, i32 1, i32 0, i32 0, i32 0, i64 32
  %5 = bitcast i8* %4 to %"struct.std::__future_base::_Result"**
  %6 = load %"struct.std::__future_base::_Result"*, %"struct.std::__future_base::_Result"** %5, align 8, !tbaa !27
  %7 = icmp eq %"struct.std::__future_base::_Result"* %6, null
  br i1 %7, label %16, label %8

; <label>:8:                                      ; preds = %1
  %9 = getelementptr inbounds %"struct.std::__future_base::_Result", %"struct.std::__future_base::_Result"* %6, i64 0, i32 0
  %10 = bitcast %"struct.std::__future_base::_Result"* %6 to void (%"struct.std::__future_base::_Result_base"*)***
  %11 = load void (%"struct.std::__future_base::_Result_base"*)**, void (%"struct.std::__future_base::_Result_base"*)*** %10, align 8, !tbaa !133
  %12 = load void (%"struct.std::__future_base::_Result_base"*)*, void (%"struct.std::__future_base::_Result_base"*)** %11, align 8
  invoke void %12(%"struct.std::__future_base::_Result_base"* %9)
          to label %16 unwind label %13

; <label>:13:                                     ; preds = %8
  %14 = landingpad { i8*, i32 }
          catch i8* null
  %15 = extractvalue { i8*, i32 } %14, 0
  tail call void @__clang_call_terminate(i8* %15) #22
  unreachable

; <label>:16:                                     ; preds = %8, %1
  store %"struct.std::__future_base::_Result"* null, %"struct.std::__future_base::_Result"** %5, align 8, !tbaa !27
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [6 x i8*] }, { [6 x i8*] }* @_ZTVNSt13__future_base13_State_baseV2E, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %3, align 8, !tbaa !133
  %17 = getelementptr inbounds %"class.std::_Sp_counted_ptr_inplace", %"class.std::_Sp_counted_ptr_inplace"* %0, i64 0, i32 1, i32 0, i32 0, i32 0, i64 8
  %18 = bitcast i8* %17 to %"struct.std::__future_base::_Result_base"**
  %19 = load %"struct.std::__future_base::_Result_base"*, %"struct.std::__future_base::_Result_base"** %18, align 8, !tbaa !27
  %20 = icmp eq %"struct.std::__future_base::_Result_base"* %19, null
  br i1 %20, label %28, label %21

; <label>:21:                                     ; preds = %16
  %22 = bitcast %"struct.std::__future_base::_Result_base"* %19 to void (%"struct.std::__future_base::_Result_base"*)***
  %23 = load void (%"struct.std::__future_base::_Result_base"*)**, void (%"struct.std::__future_base::_Result_base"*)*** %22, align 8, !tbaa !133
  %24 = load void (%"struct.std::__future_base::_Result_base"*)*, void (%"struct.std::__future_base::_Result_base"*)** %23, align 8
  invoke void %24(%"struct.std::__future_base::_Result_base"* nonnull %19)
          to label %28 unwind label %25

; <label>:25:                                     ; preds = %21
  %26 = landingpad { i8*, i32 }
          catch i8* null
  %27 = extractvalue { i8*, i32 } %26, 0
  tail call void @__clang_call_terminate(i8* %27) #22
  unreachable

; <label>:28:                                     ; preds = %21, %16
  store %"struct.std::__future_base::_Result_base"* null, %"struct.std::__future_base::_Result_base"** %18, align 8, !tbaa !27
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZNSt23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EE10_M_destroyEv(%"class.std::_Sp_counted_ptr_inplace"*) unnamed_addr #10 comdat align 2 personality i32 (...)* @__gxx_personality_v0 {
  %2 = icmp eq %"class.std::_Sp_counted_ptr_inplace"* %0, null
  br i1 %2, label %5, label %3

; <label>:3:                                      ; preds = %1
  %4 = bitcast %"class.std::_Sp_counted_ptr_inplace"* %0 to i8*
  tail call void @_ZdlPv(i8* %4) #8
  br label %5

; <label>:5:                                      ; preds = %1, %3
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr i8* @_ZNSt23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EE14_M_get_deleterERKSt9type_info(%"class.std::_Sp_counted_ptr_inplace"*, %"class.std::type_info"* dereferenceable(16)) unnamed_addr #10 comdat align 2 {
  %3 = getelementptr inbounds %"class.std::type_info", %"class.std::type_info"* %1, i64 0, i32 1
  %4 = load i8*, i8** %3, align 8, !tbaa !284
  %5 = icmp eq i8* %4, getelementptr inbounds ([24 x i8], [24 x i8]* @_ZTSSt19_Sp_make_shared_tag, i64 0, i64 0)
  br i1 %5, label %12, label %6

; <label>:6:                                      ; preds = %2
  %7 = load i8, i8* %4, align 1, !tbaa !18
  %8 = icmp eq i8 %7, 42
  br i1 %8, label %14, label %9

; <label>:9:                                      ; preds = %6
  %10 = tail call i32 @strcmp(i8* nonnull %4, i8* getelementptr inbounds ([24 x i8], [24 x i8]* @_ZTSSt19_Sp_make_shared_tag, i64 0, i64 0)) #8
  %11 = icmp eq i32 %10, 0
  br i1 %11, label %12, label %14

; <label>:12:                                     ; preds = %2, %9
  %13 = getelementptr inbounds %"class.std::_Sp_counted_ptr_inplace", %"class.std::_Sp_counted_ptr_inplace"* %0, i64 0, i32 1, i32 0, i32 0, i32 0, i64 0
  br label %14

; <label>:14:                                     ; preds = %6, %9, %12
  %15 = phi i8* [ %13, %12 ], [ null, %9 ], [ null, %6 ]
  ret i8* %15
}

; Function Attrs: nounwind readonly
declare i32 @strcmp(i8* nocapture, i8* nocapture) local_unnamed_addr #18

; Function Attrs: uwtable
define linkonce_odr void @_ZNSt11_Deque_baseIlSaIlEE17_M_initialize_mapEm(%"class.std::_Deque_base.22"*, i64) local_unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %3 = lshr i64 %1, 6
  %4 = add nuw nsw i64 %3, 1
  %5 = add nuw nsw i64 %3, 3
  %6 = icmp ugt i64 %5, 8
  %7 = select i1 %6, i64 %5, i64 8
  %8 = getelementptr inbounds %"class.std::_Deque_base.22", %"class.std::_Deque_base.22"* %0, i64 0, i32 0, i32 1
  store i64 %7, i64* %8, align 8, !tbaa !90
  %9 = icmp ugt i64 %7, 2305843009213693951
  br i1 %9, label %10, label %11

; <label>:10:                                     ; preds = %2
  tail call void @_ZSt17__throw_bad_allocv() #23
  unreachable

; <label>:11:                                     ; preds = %2
  %12 = shl nuw nsw i64 %7, 3
  %13 = tail call i8* @_Znwm(i64 %12)
  %14 = bitcast i8* %13 to i64**
  %15 = bitcast %"class.std::_Deque_base.22"* %0 to i8**
  store i8* %13, i8** %15, align 8, !tbaa !92
  %16 = load i64, i64* %8, align 8, !tbaa !90
  %17 = sub i64 %16, %4
  %18 = lshr i64 %17, 1
  %19 = getelementptr inbounds i64*, i64** %14, i64 %18
  %20 = getelementptr inbounds i64*, i64** %19, i64 %4
  br label %21

; <label>:21:                                     ; preds = %24, %11
  %22 = phi i64** [ %19, %11 ], [ %26, %24 ]
  %23 = invoke i8* @_Znwm(i64 512)
          to label %24 unwind label %28

; <label>:24:                                     ; preds = %21
  %25 = bitcast i64** %22 to i8**
  store i8* %23, i8** %25, align 8, !tbaa !27
  %26 = getelementptr inbounds i64*, i64** %22, i64 1
  %27 = icmp ult i64** %26, %20
  br i1 %27, label %21, label %55

; <label>:28:                                     ; preds = %21
  %29 = landingpad { i8*, i32 }
          catch i8* null
  %30 = extractvalue { i8*, i32 } %29, 0
  %31 = tail call i8* @__cxa_begin_catch(i8* %30) #8
  %32 = icmp ugt i64** %22, %19
  br i1 %32, label %33, label %40

; <label>:33:                                     ; preds = %28
  br label %34

; <label>:34:                                     ; preds = %33, %34
  %35 = phi i64** [ %38, %34 ], [ %19, %33 ]
  %36 = bitcast i64** %35 to i8**
  %37 = load i8*, i8** %36, align 8, !tbaa !27
  tail call void @_ZdlPv(i8* %37) #8
  %38 = getelementptr inbounds i64*, i64** %35, i64 1
  %39 = icmp ult i64** %38, %22
  br i1 %39, label %34, label %40

; <label>:40:                                     ; preds = %34, %28
  invoke void @__cxa_rethrow() #23
          to label %46 unwind label %41

; <label>:41:                                     ; preds = %40
  %42 = landingpad { i8*, i32 }
          catch i8* null
  invoke void @__cxa_end_catch()
          to label %47 unwind label %43

; <label>:43:                                     ; preds = %41
  %44 = landingpad { i8*, i32 }
          catch i8* null
  %45 = extractvalue { i8*, i32 } %44, 0
  tail call void @__clang_call_terminate(i8* %45) #22
  unreachable

; <label>:46:                                     ; preds = %40
  unreachable

; <label>:47:                                     ; preds = %41
  %48 = extractvalue { i8*, i32 } %42, 0
  %49 = tail call i8* @__cxa_begin_catch(i8* %48) #8
  %50 = load i8*, i8** %15, align 8, !tbaa !92
  tail call void @_ZdlPv(i8* %50) #8
  %51 = bitcast %"class.std::_Deque_base.22"* %0 to i8*
  call void @llvm.memset.p0i8.i64(i8* %51, i8 0, i64 16, i32 8, i1 false)
  invoke void @__cxa_rethrow() #23
          to label %76 unwind label %52

; <label>:52:                                     ; preds = %47
  %53 = landingpad { i8*, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %54 unwind label %73

; <label>:54:                                     ; preds = %52
  resume { i8*, i32 } %53

; <label>:55:                                     ; preds = %24
  %56 = getelementptr inbounds %"class.std::_Deque_base.22", %"class.std::_Deque_base.22"* %0, i64 0, i32 0, i32 2
  %57 = getelementptr inbounds %"class.std::_Deque_base.22", %"class.std::_Deque_base.22"* %0, i64 0, i32 0, i32 2, i32 3
  store i64** %19, i64*** %57, align 8, !tbaa !93
  %58 = load i64*, i64** %19, align 8, !tbaa !27
  %59 = getelementptr inbounds %"class.std::_Deque_base.22", %"class.std::_Deque_base.22"* %0, i64 0, i32 0, i32 2, i32 1
  store i64* %58, i64** %59, align 8, !tbaa !94
  %60 = getelementptr inbounds i64, i64* %58, i64 64
  %61 = getelementptr inbounds %"class.std::_Deque_base.22", %"class.std::_Deque_base.22"* %0, i64 0, i32 0, i32 2, i32 2
  store i64* %60, i64** %61, align 8, !tbaa !95
  %62 = getelementptr inbounds i64*, i64** %20, i64 -1
  %63 = getelementptr inbounds %"class.std::_Deque_base.22", %"class.std::_Deque_base.22"* %0, i64 0, i32 0, i32 3, i32 3
  store i64** %62, i64*** %63, align 8, !tbaa !93
  %64 = load i64*, i64** %62, align 8, !tbaa !27
  %65 = getelementptr inbounds %"class.std::_Deque_base.22", %"class.std::_Deque_base.22"* %0, i64 0, i32 0, i32 3, i32 1
  store i64* %64, i64** %65, align 8, !tbaa !94
  %66 = getelementptr inbounds i64, i64* %64, i64 64
  %67 = getelementptr inbounds %"class.std::_Deque_base.22", %"class.std::_Deque_base.22"* %0, i64 0, i32 0, i32 3, i32 2
  store i64* %66, i64** %67, align 8, !tbaa !95
  %68 = ptrtoint i64* %58 to i64
  %69 = bitcast %"struct.std::_Deque_iterator.26"* %56 to i64*
  store i64 %68, i64* %69, align 8, !tbaa !102
  %70 = and i64 %1, 63
  %71 = getelementptr inbounds i64, i64* %64, i64 %70
  %72 = getelementptr inbounds %"class.std::_Deque_base.22", %"class.std::_Deque_base.22"* %0, i64 0, i32 0, i32 3, i32 0
  store i64* %71, i64** %72, align 8, !tbaa !85
  ret void

; <label>:73:                                     ; preds = %52
  %74 = landingpad { i8*, i32 }
          catch i8* null
  %75 = extractvalue { i8*, i32 } %74, 0
  tail call void @__clang_call_terminate(i8* %75) #22
  unreachable

; <label>:76:                                     ; preds = %47
  unreachable
}

; Function Attrs: nounwind
declare void @_ZNSt18condition_variableC1Ev(%"class.std::condition_variable"*) unnamed_addr #7

; Function Attrs: uwtable
define linkonce_odr void @_ZNSt11_Deque_baseIiSaIiEE17_M_initialize_mapEm(%"class.std::_Deque_base.14"*, i64) local_unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %3 = lshr i64 %1, 7
  %4 = add nuw nsw i64 %3, 1
  %5 = add nuw nsw i64 %3, 3
  %6 = icmp ugt i64 %5, 8
  %7 = select i1 %6, i64 %5, i64 8
  %8 = getelementptr inbounds %"class.std::_Deque_base.14", %"class.std::_Deque_base.14"* %0, i64 0, i32 0, i32 1
  store i64 %7, i64* %8, align 8, !tbaa !70
  %9 = icmp ugt i64 %7, 2305843009213693951
  br i1 %9, label %10, label %11

; <label>:10:                                     ; preds = %2
  tail call void @_ZSt17__throw_bad_allocv() #23
  unreachable

; <label>:11:                                     ; preds = %2
  %12 = shl nuw nsw i64 %7, 3
  %13 = tail call i8* @_Znwm(i64 %12)
  %14 = bitcast i8* %13 to i32**
  %15 = bitcast %"class.std::_Deque_base.14"* %0 to i8**
  store i8* %13, i8** %15, align 8, !tbaa !72
  %16 = load i64, i64* %8, align 8, !tbaa !70
  %17 = sub i64 %16, %4
  %18 = lshr i64 %17, 1
  %19 = getelementptr inbounds i32*, i32** %14, i64 %18
  %20 = getelementptr inbounds i32*, i32** %19, i64 %4
  br label %21

; <label>:21:                                     ; preds = %24, %11
  %22 = phi i32** [ %19, %11 ], [ %26, %24 ]
  %23 = invoke i8* @_Znwm(i64 512)
          to label %24 unwind label %28

; <label>:24:                                     ; preds = %21
  %25 = bitcast i32** %22 to i8**
  store i8* %23, i8** %25, align 8, !tbaa !27
  %26 = getelementptr inbounds i32*, i32** %22, i64 1
  %27 = icmp ult i32** %26, %20
  br i1 %27, label %21, label %55

; <label>:28:                                     ; preds = %21
  %29 = landingpad { i8*, i32 }
          catch i8* null
  %30 = extractvalue { i8*, i32 } %29, 0
  %31 = tail call i8* @__cxa_begin_catch(i8* %30) #8
  %32 = icmp ugt i32** %22, %19
  br i1 %32, label %33, label %40

; <label>:33:                                     ; preds = %28
  br label %34

; <label>:34:                                     ; preds = %33, %34
  %35 = phi i32** [ %38, %34 ], [ %19, %33 ]
  %36 = bitcast i32** %35 to i8**
  %37 = load i8*, i8** %36, align 8, !tbaa !27
  tail call void @_ZdlPv(i8* %37) #8
  %38 = getelementptr inbounds i32*, i32** %35, i64 1
  %39 = icmp ult i32** %38, %22
  br i1 %39, label %34, label %40

; <label>:40:                                     ; preds = %34, %28
  invoke void @__cxa_rethrow() #23
          to label %46 unwind label %41

; <label>:41:                                     ; preds = %40
  %42 = landingpad { i8*, i32 }
          catch i8* null
  invoke void @__cxa_end_catch()
          to label %47 unwind label %43

; <label>:43:                                     ; preds = %41
  %44 = landingpad { i8*, i32 }
          catch i8* null
  %45 = extractvalue { i8*, i32 } %44, 0
  tail call void @__clang_call_terminate(i8* %45) #22
  unreachable

; <label>:46:                                     ; preds = %40
  unreachable

; <label>:47:                                     ; preds = %41
  %48 = extractvalue { i8*, i32 } %42, 0
  %49 = tail call i8* @__cxa_begin_catch(i8* %48) #8
  %50 = load i8*, i8** %15, align 8, !tbaa !72
  tail call void @_ZdlPv(i8* %50) #8
  %51 = bitcast %"class.std::_Deque_base.14"* %0 to i8*
  call void @llvm.memset.p0i8.i64(i8* %51, i8 0, i64 16, i32 8, i1 false)
  invoke void @__cxa_rethrow() #23
          to label %76 unwind label %52

; <label>:52:                                     ; preds = %47
  %53 = landingpad { i8*, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %54 unwind label %73

; <label>:54:                                     ; preds = %52
  resume { i8*, i32 } %53

; <label>:55:                                     ; preds = %24
  %56 = getelementptr inbounds %"class.std::_Deque_base.14", %"class.std::_Deque_base.14"* %0, i64 0, i32 0, i32 2
  %57 = getelementptr inbounds %"class.std::_Deque_base.14", %"class.std::_Deque_base.14"* %0, i64 0, i32 0, i32 2, i32 3
  store i32** %19, i32*** %57, align 8, !tbaa !73
  %58 = load i32*, i32** %19, align 8, !tbaa !27
  %59 = getelementptr inbounds %"class.std::_Deque_base.14", %"class.std::_Deque_base.14"* %0, i64 0, i32 0, i32 2, i32 1
  store i32* %58, i32** %59, align 8, !tbaa !74
  %60 = getelementptr inbounds i32, i32* %58, i64 128
  %61 = getelementptr inbounds %"class.std::_Deque_base.14", %"class.std::_Deque_base.14"* %0, i64 0, i32 0, i32 2, i32 2
  store i32* %60, i32** %61, align 8, !tbaa !75
  %62 = getelementptr inbounds i32*, i32** %20, i64 -1
  %63 = getelementptr inbounds %"class.std::_Deque_base.14", %"class.std::_Deque_base.14"* %0, i64 0, i32 0, i32 3, i32 3
  store i32** %62, i32*** %63, align 8, !tbaa !73
  %64 = load i32*, i32** %62, align 8, !tbaa !27
  %65 = getelementptr inbounds %"class.std::_Deque_base.14", %"class.std::_Deque_base.14"* %0, i64 0, i32 0, i32 3, i32 1
  store i32* %64, i32** %65, align 8, !tbaa !74
  %66 = getelementptr inbounds i32, i32* %64, i64 128
  %67 = getelementptr inbounds %"class.std::_Deque_base.14", %"class.std::_Deque_base.14"* %0, i64 0, i32 0, i32 3, i32 2
  store i32* %66, i32** %67, align 8, !tbaa !75
  %68 = ptrtoint i32* %58 to i64
  %69 = bitcast %"struct.std::_Deque_iterator.18"* %56 to i64*
  store i64 %68, i64* %69, align 8, !tbaa !82
  %70 = and i64 %1, 127
  %71 = getelementptr inbounds i32, i32* %64, i64 %70
  %72 = getelementptr inbounds %"class.std::_Deque_base.14", %"class.std::_Deque_base.14"* %0, i64 0, i32 0, i32 3, i32 0
  store i32* %71, i32** %72, align 8, !tbaa !65
  ret void

; <label>:73:                                     ; preds = %52
  %74 = landingpad { i8*, i32 }
          catch i8* null
  %75 = extractvalue { i8*, i32 } %74, 0
  tail call void @__clang_call_terminate(i8* %75) #22
  unreachable

; <label>:76:                                     ; preds = %47
  unreachable
}

; Function Attrs: uwtable
define linkonce_odr void @_ZNSt11_Deque_baseIsSaIsEE17_M_initialize_mapEm(%"class.std::_Deque_base.6"*, i64) local_unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %3 = lshr i64 %1, 8
  %4 = add nuw nsw i64 %3, 1
  %5 = add nuw nsw i64 %3, 3
  %6 = icmp ugt i64 %5, 8
  %7 = select i1 %6, i64 %5, i64 8
  %8 = getelementptr inbounds %"class.std::_Deque_base.6", %"class.std::_Deque_base.6"* %0, i64 0, i32 0, i32 1
  store i64 %7, i64* %8, align 8, !tbaa !51
  %9 = icmp ugt i64 %7, 2305843009213693951
  br i1 %9, label %10, label %11

; <label>:10:                                     ; preds = %2
  tail call void @_ZSt17__throw_bad_allocv() #23
  unreachable

; <label>:11:                                     ; preds = %2
  %12 = shl nuw nsw i64 %7, 3
  %13 = tail call i8* @_Znwm(i64 %12)
  %14 = bitcast i8* %13 to i16**
  %15 = bitcast %"class.std::_Deque_base.6"* %0 to i8**
  store i8* %13, i8** %15, align 8, !tbaa !53
  %16 = load i64, i64* %8, align 8, !tbaa !51
  %17 = sub i64 %16, %4
  %18 = lshr i64 %17, 1
  %19 = getelementptr inbounds i16*, i16** %14, i64 %18
  %20 = getelementptr inbounds i16*, i16** %19, i64 %4
  br label %21

; <label>:21:                                     ; preds = %24, %11
  %22 = phi i16** [ %19, %11 ], [ %26, %24 ]
  %23 = invoke i8* @_Znwm(i64 512)
          to label %24 unwind label %28

; <label>:24:                                     ; preds = %21
  %25 = bitcast i16** %22 to i8**
  store i8* %23, i8** %25, align 8, !tbaa !27
  %26 = getelementptr inbounds i16*, i16** %22, i64 1
  %27 = icmp ult i16** %26, %20
  br i1 %27, label %21, label %55

; <label>:28:                                     ; preds = %21
  %29 = landingpad { i8*, i32 }
          catch i8* null
  %30 = extractvalue { i8*, i32 } %29, 0
  %31 = tail call i8* @__cxa_begin_catch(i8* %30) #8
  %32 = icmp ugt i16** %22, %19
  br i1 %32, label %33, label %40

; <label>:33:                                     ; preds = %28
  br label %34

; <label>:34:                                     ; preds = %33, %34
  %35 = phi i16** [ %38, %34 ], [ %19, %33 ]
  %36 = bitcast i16** %35 to i8**
  %37 = load i8*, i8** %36, align 8, !tbaa !27
  tail call void @_ZdlPv(i8* %37) #8
  %38 = getelementptr inbounds i16*, i16** %35, i64 1
  %39 = icmp ult i16** %38, %22
  br i1 %39, label %34, label %40

; <label>:40:                                     ; preds = %34, %28
  invoke void @__cxa_rethrow() #23
          to label %46 unwind label %41

; <label>:41:                                     ; preds = %40
  %42 = landingpad { i8*, i32 }
          catch i8* null
  invoke void @__cxa_end_catch()
          to label %47 unwind label %43

; <label>:43:                                     ; preds = %41
  %44 = landingpad { i8*, i32 }
          catch i8* null
  %45 = extractvalue { i8*, i32 } %44, 0
  tail call void @__clang_call_terminate(i8* %45) #22
  unreachable

; <label>:46:                                     ; preds = %40
  unreachable

; <label>:47:                                     ; preds = %41
  %48 = extractvalue { i8*, i32 } %42, 0
  %49 = tail call i8* @__cxa_begin_catch(i8* %48) #8
  %50 = load i8*, i8** %15, align 8, !tbaa !53
  tail call void @_ZdlPv(i8* %50) #8
  %51 = bitcast %"class.std::_Deque_base.6"* %0 to i8*
  call void @llvm.memset.p0i8.i64(i8* %51, i8 0, i64 16, i32 8, i1 false)
  invoke void @__cxa_rethrow() #23
          to label %76 unwind label %52

; <label>:52:                                     ; preds = %47
  %53 = landingpad { i8*, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %54 unwind label %73

; <label>:54:                                     ; preds = %52
  resume { i8*, i32 } %53

; <label>:55:                                     ; preds = %24
  %56 = getelementptr inbounds %"class.std::_Deque_base.6", %"class.std::_Deque_base.6"* %0, i64 0, i32 0, i32 2
  %57 = getelementptr inbounds %"class.std::_Deque_base.6", %"class.std::_Deque_base.6"* %0, i64 0, i32 0, i32 2, i32 3
  store i16** %19, i16*** %57, align 8, !tbaa !54
  %58 = load i16*, i16** %19, align 8, !tbaa !27
  %59 = getelementptr inbounds %"class.std::_Deque_base.6", %"class.std::_Deque_base.6"* %0, i64 0, i32 0, i32 2, i32 1
  store i16* %58, i16** %59, align 8, !tbaa !55
  %60 = getelementptr inbounds i16, i16* %58, i64 256
  %61 = getelementptr inbounds %"class.std::_Deque_base.6", %"class.std::_Deque_base.6"* %0, i64 0, i32 0, i32 2, i32 2
  store i16* %60, i16** %61, align 8, !tbaa !56
  %62 = getelementptr inbounds i16*, i16** %20, i64 -1
  %63 = getelementptr inbounds %"class.std::_Deque_base.6", %"class.std::_Deque_base.6"* %0, i64 0, i32 0, i32 3, i32 3
  store i16** %62, i16*** %63, align 8, !tbaa !54
  %64 = load i16*, i16** %62, align 8, !tbaa !27
  %65 = getelementptr inbounds %"class.std::_Deque_base.6", %"class.std::_Deque_base.6"* %0, i64 0, i32 0, i32 3, i32 1
  store i16* %64, i16** %65, align 8, !tbaa !55
  %66 = getelementptr inbounds i16, i16* %64, i64 256
  %67 = getelementptr inbounds %"class.std::_Deque_base.6", %"class.std::_Deque_base.6"* %0, i64 0, i32 0, i32 3, i32 2
  store i16* %66, i16** %67, align 8, !tbaa !56
  %68 = ptrtoint i16* %58 to i64
  %69 = bitcast %"struct.std::_Deque_iterator.10"* %56 to i64*
  store i64 %68, i64* %69, align 8, !tbaa !63
  %70 = and i64 %1, 255
  %71 = getelementptr inbounds i16, i16* %64, i64 %70
  %72 = getelementptr inbounds %"class.std::_Deque_base.6", %"class.std::_Deque_base.6"* %0, i64 0, i32 0, i32 3, i32 0
  store i16* %71, i16** %72, align 8, !tbaa !46
  ret void

; <label>:73:                                     ; preds = %52
  %74 = landingpad { i8*, i32 }
          catch i8* null
  %75 = extractvalue { i8*, i32 } %74, 0
  tail call void @__clang_call_terminate(i8* %75) #22
  unreachable

; <label>:76:                                     ; preds = %47
  unreachable
}

; Function Attrs: uwtable
define linkonce_odr void @_ZNSt11_Deque_baseIaSaIaEE17_M_initialize_mapEm(%"class.std::_Deque_base"*, i64) local_unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %3 = lshr i64 %1, 9
  %4 = add nuw nsw i64 %3, 1
  %5 = add nuw nsw i64 %3, 3
  %6 = icmp ugt i64 %5, 8
  %7 = select i1 %6, i64 %5, i64 8
  %8 = getelementptr inbounds %"class.std::_Deque_base", %"class.std::_Deque_base"* %0, i64 0, i32 0, i32 1
  store i64 %7, i64* %8, align 8, !tbaa !24
  %9 = icmp ugt i64 %7, 2305843009213693951
  br i1 %9, label %10, label %11

; <label>:10:                                     ; preds = %2
  tail call void @_ZSt17__throw_bad_allocv() #23
  unreachable

; <label>:11:                                     ; preds = %2
  %12 = shl nuw nsw i64 %7, 3
  %13 = tail call i8* @_Znwm(i64 %12)
  %14 = bitcast i8* %13 to i8**
  %15 = bitcast %"class.std::_Deque_base"* %0 to i8**
  store i8* %13, i8** %15, align 8, !tbaa !26
  %16 = load i64, i64* %8, align 8, !tbaa !24
  %17 = sub i64 %16, %4
  %18 = lshr i64 %17, 1
  %19 = getelementptr inbounds i8*, i8** %14, i64 %18
  %20 = getelementptr inbounds i8*, i8** %19, i64 %4
  br label %21

; <label>:21:                                     ; preds = %24, %11
  %22 = phi i8** [ %19, %11 ], [ %25, %24 ]
  %23 = invoke i8* @_Znwm(i64 512)
          to label %24 unwind label %27

; <label>:24:                                     ; preds = %21
  store i8* %23, i8** %22, align 8, !tbaa !27
  %25 = getelementptr inbounds i8*, i8** %22, i64 1
  %26 = icmp ult i8** %25, %20
  br i1 %26, label %21, label %53

; <label>:27:                                     ; preds = %21
  %28 = landingpad { i8*, i32 }
          catch i8* null
  %29 = extractvalue { i8*, i32 } %28, 0
  %30 = tail call i8* @__cxa_begin_catch(i8* %29) #8
  %31 = icmp ugt i8** %22, %19
  br i1 %31, label %32, label %38

; <label>:32:                                     ; preds = %27
  br label %33

; <label>:33:                                     ; preds = %32, %33
  %34 = phi i8** [ %36, %33 ], [ %19, %32 ]
  %35 = load i8*, i8** %34, align 8, !tbaa !27
  tail call void @_ZdlPv(i8* %35) #8
  %36 = getelementptr inbounds i8*, i8** %34, i64 1
  %37 = icmp ult i8** %36, %22
  br i1 %37, label %33, label %38

; <label>:38:                                     ; preds = %33, %27
  invoke void @__cxa_rethrow() #23
          to label %44 unwind label %39

; <label>:39:                                     ; preds = %38
  %40 = landingpad { i8*, i32 }
          catch i8* null
  invoke void @__cxa_end_catch()
          to label %45 unwind label %41

; <label>:41:                                     ; preds = %39
  %42 = landingpad { i8*, i32 }
          catch i8* null
  %43 = extractvalue { i8*, i32 } %42, 0
  tail call void @__clang_call_terminate(i8* %43) #22
  unreachable

; <label>:44:                                     ; preds = %38
  unreachable

; <label>:45:                                     ; preds = %39
  %46 = extractvalue { i8*, i32 } %40, 0
  %47 = tail call i8* @__cxa_begin_catch(i8* %46) #8
  %48 = load i8*, i8** %15, align 8, !tbaa !26
  tail call void @_ZdlPv(i8* %48) #8
  %49 = bitcast %"class.std::_Deque_base"* %0 to i8*
  call void @llvm.memset.p0i8.i64(i8* %49, i8 0, i64 16, i32 8, i1 false)
  invoke void @__cxa_rethrow() #23
          to label %74 unwind label %50

; <label>:50:                                     ; preds = %45
  %51 = landingpad { i8*, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %52 unwind label %71

; <label>:52:                                     ; preds = %50
  resume { i8*, i32 } %51

; <label>:53:                                     ; preds = %24
  %54 = getelementptr inbounds %"class.std::_Deque_base", %"class.std::_Deque_base"* %0, i64 0, i32 0, i32 2
  %55 = getelementptr inbounds %"class.std::_Deque_base", %"class.std::_Deque_base"* %0, i64 0, i32 0, i32 2, i32 3
  store i8** %19, i8*** %55, align 8, !tbaa !28
  %56 = load i8*, i8** %19, align 8, !tbaa !27
  %57 = getelementptr inbounds %"class.std::_Deque_base", %"class.std::_Deque_base"* %0, i64 0, i32 0, i32 2, i32 1
  store i8* %56, i8** %57, align 8, !tbaa !29
  %58 = getelementptr inbounds i8, i8* %56, i64 512
  %59 = getelementptr inbounds %"class.std::_Deque_base", %"class.std::_Deque_base"* %0, i64 0, i32 0, i32 2, i32 2
  store i8* %58, i8** %59, align 8, !tbaa !30
  %60 = getelementptr inbounds i8*, i8** %20, i64 -1
  %61 = getelementptr inbounds %"class.std::_Deque_base", %"class.std::_Deque_base"* %0, i64 0, i32 0, i32 3, i32 3
  store i8** %60, i8*** %61, align 8, !tbaa !28
  %62 = load i8*, i8** %60, align 8, !tbaa !27
  %63 = getelementptr inbounds %"class.std::_Deque_base", %"class.std::_Deque_base"* %0, i64 0, i32 0, i32 3, i32 1
  store i8* %62, i8** %63, align 8, !tbaa !29
  %64 = getelementptr inbounds i8, i8* %62, i64 512
  %65 = getelementptr inbounds %"class.std::_Deque_base", %"class.std::_Deque_base"* %0, i64 0, i32 0, i32 3, i32 2
  store i8* %64, i8** %65, align 8, !tbaa !30
  %66 = ptrtoint i8* %56 to i64
  %67 = bitcast %"struct.std::_Deque_iterator"* %54 to i64*
  store i64 %66, i64* %67, align 8, !tbaa !40
  %68 = and i64 %1, 511
  %69 = getelementptr inbounds i8, i8* %62, i64 %68
  %70 = getelementptr inbounds %"class.std::_Deque_base", %"class.std::_Deque_base"* %0, i64 0, i32 0, i32 3, i32 0
  store i8* %69, i8** %70, align 8, !tbaa !19
  ret void

; <label>:71:                                     ; preds = %50
  %72 = landingpad { i8*, i32 }
          catch i8* null
  %73 = extractvalue { i8*, i32 } %72, 0
  tail call void @__clang_call_terminate(i8* %73) #22
  unreachable

; <label>:74:                                     ; preds = %45
  unreachable
}

; Function Attrs: uwtable
define linkonce_odr void @_ZN4MARC15ThreadSafeQueueISt10unique_ptrINS_11IThreadTaskESt14default_deleteIS2_EEEC2Ev(%"class.MARC::ThreadSafeQueue.32"*) unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %2 = alloca %"class.std::deque.34", align 16
  %3 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 0, i32 0, i32 0
  store i8 1, i8* %3, align 1, !tbaa !135
  %4 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 1
  %5 = bitcast %"class.std::mutex"* %4 to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %5, i8 0, i64 40, i32 8, i1 false) #8
  %6 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 2
  %7 = bitcast %"class.std::deque.34"* %2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 80, i8* nonnull %7) #8
  %8 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %2, i64 0, i32 0
  call void @llvm.memset.p0i8.i64(i8* nonnull %7, i8 0, i64 80, i32 16, i1 false) #8
  call void @_ZNSt11_Deque_baseISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EESaIS5_EE17_M_initialize_mapEm(%"class.std::_Deque_base.35"* nonnull %8, i64 0)
  %9 = getelementptr inbounds %"class.std::queue.33", %"class.std::queue.33"* %6, i64 0, i32 0, i32 0
  %10 = bitcast %"class.std::queue.33"* %6 to i8*
  call void @llvm.memset.p0i8.i64(i8* %10, i8 0, i64 80, i32 8, i1 false) #8
  invoke void @_ZNSt11_Deque_baseISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EESaIS5_EE17_M_initialize_mapEm(%"class.std::_Deque_base.35"* %9, i64 0)
          to label %11 unwind label %51

; <label>:11:                                     ; preds = %1
  %12 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %2, i64 0, i32 0, i32 0, i32 0
  %13 = load %"class.std::unique_ptr"**, %"class.std::unique_ptr"*** %12, align 16, !tbaa !201
  %14 = icmp eq %"class.std::unique_ptr"** %13, null
  br i1 %14, label %48, label %15

; <label>:15:                                     ; preds = %11
  %16 = ptrtoint %"class.std::unique_ptr"** %13 to i64
  %17 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2
  %18 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %2, i64 0, i32 0, i32 0, i32 2
  %19 = bitcast %"struct.std::_Deque_iterator.41"* %17 to <2 x i64>*
  %20 = load <2 x i64>, <2 x i64>* %19, align 8, !tbaa !27
  %21 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 2
  %22 = bitcast %"class.std::unique_ptr"** %21 to <2 x i64>*
  %23 = load <2 x i64>, <2 x i64>* %22, align 8, !tbaa !27
  %24 = bitcast %"struct.std::_Deque_iterator.41"* %17 to i8*
  %25 = bitcast %"struct.std::_Deque_iterator.41"* %18 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %24, i8* nonnull %25, i64 32, i32 8, i1 false) #8, !tbaa.struct !155
  %26 = bitcast %"struct.std::_Deque_iterator.41"* %18 to <2 x i64>*
  store <2 x i64> %20, <2 x i64>* %26, align 16
  %27 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %2, i64 0, i32 0, i32 0, i32 2, i32 2
  %28 = bitcast %"class.std::unique_ptr"** %27 to <2 x i64>*
  store <2 x i64> %23, <2 x i64>* %28, align 16
  %29 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3
  %30 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %2, i64 0, i32 0, i32 0, i32 3
  %31 = bitcast %"struct.std::_Deque_iterator.41"* %29 to <2 x i64>*
  %32 = load <2 x i64>, <2 x i64>* %31, align 8, !tbaa !27
  %33 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 2
  %34 = bitcast %"class.std::unique_ptr"** %33 to <2 x i64>*
  %35 = load <2 x i64>, <2 x i64>* %34, align 8, !tbaa !27
  %36 = bitcast %"struct.std::_Deque_iterator.41"* %29 to i8*
  %37 = bitcast %"struct.std::_Deque_iterator.41"* %30 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %36, i8* nonnull %37, i64 32, i32 8, i1 false) #8, !tbaa.struct !155
  %38 = bitcast %"struct.std::_Deque_iterator.41"* %30 to <2 x i64>*
  store <2 x i64> %32, <2 x i64>* %38, align 16
  %39 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %2, i64 0, i32 0, i32 0, i32 3, i32 2
  %40 = bitcast %"class.std::unique_ptr"** %39 to <2 x i64>*
  store <2 x i64> %35, <2 x i64>* %40, align 16
  %41 = bitcast %"class.std::queue.33"* %6 to i64*
  %42 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 1
  %43 = bitcast %"class.std::queue.33"* %6 to <2 x i64>*
  %44 = load <2 x i64>, <2 x i64>* %43, align 8, !tbaa !18
  store i64 %16, i64* %41, align 8, !tbaa !27
  %45 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %2, i64 0, i32 0, i32 0, i32 1
  %46 = load i64, i64* %45, align 8, !tbaa !84
  store i64 %46, i64* %42, align 8, !tbaa !84
  %47 = bitcast %"class.std::deque.34"* %2 to <2 x i64>*
  store <2 x i64> %44, <2 x i64>* %47, align 16, !tbaa !18
  br label %48

; <label>:48:                                     ; preds = %15, %11
  call void @_ZNSt5dequeISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EESaIS5_EED2Ev(%"class.std::deque.34"* nonnull %2) #8
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %7) #8
  %49 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 3
  call void @_ZNSt18condition_variableC1Ev(%"class.std::condition_variable"* %49) #8
  %50 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 4
  call void @_ZNSt18condition_variableC1Ev(%"class.std::condition_variable"* %50) #8
  ret void

; <label>:51:                                     ; preds = %1
  %52 = landingpad { i8*, i32 }
          cleanup
  call void @_ZNSt5dequeISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EESaIS5_EED2Ev(%"class.std::deque.34"* nonnull %2) #8
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %7) #8
  resume { i8*, i32 } %52
}

; Function Attrs: uwtable
define linkonce_odr void @_ZN4MARC15ThreadSafeQueueISt8functionIFvvEEEC2Ev(%"class.MARC::ThreadSafeQueue.45"*) unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %2 = alloca %"class.std::deque.47", align 16
  %3 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 0, i32 0, i32 0
  store i8 1, i8* %3, align 1, !tbaa !135
  %4 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 1
  %5 = bitcast %"class.std::mutex"* %4 to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %5, i8 0, i64 40, i32 8, i1 false) #8
  %6 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 2
  %7 = bitcast %"class.std::deque.47"* %2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 80, i8* nonnull %7) #8
  %8 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %2, i64 0, i32 0
  call void @llvm.memset.p0i8.i64(i8* nonnull %7, i8 0, i64 80, i32 16, i1 false) #8
  call void @_ZNSt11_Deque_baseISt8functionIFvvEESaIS2_EE17_M_initialize_mapEm(%"class.std::_Deque_base.48"* nonnull %8, i64 0)
  %9 = getelementptr inbounds %"class.std::queue.46", %"class.std::queue.46"* %6, i64 0, i32 0, i32 0
  %10 = bitcast %"class.std::queue.46"* %6 to i8*
  call void @llvm.memset.p0i8.i64(i8* %10, i8 0, i64 80, i32 8, i1 false) #8
  invoke void @_ZNSt11_Deque_baseISt8functionIFvvEESaIS2_EE17_M_initialize_mapEm(%"class.std::_Deque_base.48"* %9, i64 0)
          to label %11 unwind label %51

; <label>:11:                                     ; preds = %1
  %12 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %2, i64 0, i32 0, i32 0, i32 0
  %13 = load %"class.std::function"**, %"class.std::function"*** %12, align 16, !tbaa !228
  %14 = icmp eq %"class.std::function"** %13, null
  br i1 %14, label %48, label %15

; <label>:15:                                     ; preds = %11
  %16 = ptrtoint %"class.std::function"** %13 to i64
  %17 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2
  %18 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %2, i64 0, i32 0, i32 0, i32 2
  %19 = bitcast %"struct.std::_Deque_iterator.52"* %17 to <2 x i64>*
  %20 = load <2 x i64>, <2 x i64>* %19, align 8, !tbaa !27
  %21 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 2
  %22 = bitcast %"class.std::function"** %21 to <2 x i64>*
  %23 = load <2 x i64>, <2 x i64>* %22, align 8, !tbaa !27
  %24 = bitcast %"struct.std::_Deque_iterator.52"* %17 to i8*
  %25 = bitcast %"struct.std::_Deque_iterator.52"* %18 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %24, i8* nonnull %25, i64 32, i32 8, i1 false) #8, !tbaa.struct !155
  %26 = bitcast %"struct.std::_Deque_iterator.52"* %18 to <2 x i64>*
  store <2 x i64> %20, <2 x i64>* %26, align 16
  %27 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %2, i64 0, i32 0, i32 0, i32 2, i32 2
  %28 = bitcast %"class.std::function"** %27 to <2 x i64>*
  store <2 x i64> %23, <2 x i64>* %28, align 16
  %29 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3
  %30 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %2, i64 0, i32 0, i32 0, i32 3
  %31 = bitcast %"struct.std::_Deque_iterator.52"* %29 to <2 x i64>*
  %32 = load <2 x i64>, <2 x i64>* %31, align 8, !tbaa !27
  %33 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 2
  %34 = bitcast %"class.std::function"** %33 to <2 x i64>*
  %35 = load <2 x i64>, <2 x i64>* %34, align 8, !tbaa !27
  %36 = bitcast %"struct.std::_Deque_iterator.52"* %29 to i8*
  %37 = bitcast %"struct.std::_Deque_iterator.52"* %30 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %36, i8* nonnull %37, i64 32, i32 8, i1 false) #8, !tbaa.struct !155
  %38 = bitcast %"struct.std::_Deque_iterator.52"* %30 to <2 x i64>*
  store <2 x i64> %32, <2 x i64>* %38, align 16
  %39 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %2, i64 0, i32 0, i32 0, i32 3, i32 2
  %40 = bitcast %"class.std::function"** %39 to <2 x i64>*
  store <2 x i64> %35, <2 x i64>* %40, align 16
  %41 = bitcast %"class.std::queue.46"* %6 to i64*
  %42 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 1
  %43 = bitcast %"class.std::queue.46"* %6 to <2 x i64>*
  %44 = load <2 x i64>, <2 x i64>* %43, align 8, !tbaa !18
  store i64 %16, i64* %41, align 8, !tbaa !27
  %45 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %2, i64 0, i32 0, i32 0, i32 1
  %46 = load i64, i64* %45, align 8, !tbaa !84
  store i64 %46, i64* %42, align 8, !tbaa !84
  %47 = bitcast %"class.std::deque.47"* %2 to <2 x i64>*
  store <2 x i64> %44, <2 x i64>* %47, align 16, !tbaa !18
  br label %48

; <label>:48:                                     ; preds = %15, %11
  call void @_ZNSt5dequeISt8functionIFvvEESaIS2_EED2Ev(%"class.std::deque.47"* nonnull %2) #8
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %7) #8
  %49 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 3
  call void @_ZNSt18condition_variableC1Ev(%"class.std::condition_variable"* %49) #8
  %50 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 4
  call void @_ZNSt18condition_variableC1Ev(%"class.std::condition_variable"* %50) #8
  ret void

; <label>:51:                                     ; preds = %1
  %52 = landingpad { i8*, i32 }
          cleanup
  call void @_ZNSt5dequeISt8functionIFvvEESaIS2_EED2Ev(%"class.std::deque.47"* nonnull %2) #8
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %7) #8
  resume { i8*, i32 } %52
}

; Function Attrs: nobuiltin
declare noalias nonnull i8* @_Znam(i64) local_unnamed_addr #12

; Function Attrs: uwtable
define linkonce_odr void @_ZN4MARC10ThreadPool6workerEPSt6atomicIbE(%"class.MARC::ThreadPool"*, %"struct.std::atomic"*) #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %3 = alloca %"class.std::unique_ptr", align 8
  %4 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 0, i32 0, i32 0
  %5 = load atomic i8, i8* %4 seq_cst, align 1
  %6 = and i8 %5, 1
  %7 = icmp eq i8 %6, 0
  br i1 %7, label %8, label %43

; <label>:8:                                      ; preds = %2
  %9 = getelementptr inbounds %"struct.std::atomic", %"struct.std::atomic"* %1, i64 0, i32 0, i32 0
  %10 = bitcast %"class.std::unique_ptr"* %3 to i8*
  %11 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %3, i64 0, i32 0, i32 0, i32 0, i32 0
  %12 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 1
  br label %13

; <label>:13:                                     ; preds = %8, %39
  store atomic i8 1, i8* %9 seq_cst, align 1
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %10) #8
  store %"class.MARC::IThreadTask"* null, %"class.MARC::IThreadTask"** %11, align 8, !tbaa !197
  %14 = invoke zeroext i1 @_ZN4MARC15ThreadSafeQueueISt10unique_ptrINS_11IThreadTaskESt14default_deleteIS2_EEE7waitPopERS5_(%"class.MARC::ThreadSafeQueue.32"* %12, %"class.std::unique_ptr"* nonnull dereferenceable(8) %3)
          to label %15 unwind label %21

; <label>:15:                                     ; preds = %13
  br i1 %14, label %16, label %31

; <label>:16:                                     ; preds = %15
  store atomic i8 0, i8* %9 seq_cst, align 1
  %17 = load %"class.MARC::IThreadTask"*, %"class.MARC::IThreadTask"** %11, align 8, !tbaa !27
  %18 = bitcast %"class.MARC::IThreadTask"* %17 to void (%"class.MARC::IThreadTask"*)***
  %19 = load void (%"class.MARC::IThreadTask"*)**, void (%"class.MARC::IThreadTask"*)*** %18, align 8, !tbaa !133
  %20 = load void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %19, align 8
  invoke void %20(%"class.MARC::IThreadTask"* %17)
          to label %31 unwind label %21

; <label>:21:                                     ; preds = %16, %13
  %22 = landingpad { i8*, i32 }
          cleanup
  %23 = load %"class.MARC::IThreadTask"*, %"class.MARC::IThreadTask"** %11, align 8, !tbaa !27
  %24 = icmp eq %"class.MARC::IThreadTask"* %23, null
  br i1 %24, label %30, label %25

; <label>:25:                                     ; preds = %21
  %26 = bitcast %"class.MARC::IThreadTask"* %23 to void (%"class.MARC::IThreadTask"*)***
  %27 = load void (%"class.MARC::IThreadTask"*)**, void (%"class.MARC::IThreadTask"*)*** %26, align 8, !tbaa !133
  %28 = getelementptr inbounds void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %27, i64 2
  %29 = load void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %28, align 8
  call void %29(%"class.MARC::IThreadTask"* nonnull %23) #8
  br label %30

; <label>:30:                                     ; preds = %21, %25
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %10) #8
  resume { i8*, i32 } %22

; <label>:31:                                     ; preds = %16, %15
  %32 = load %"class.MARC::IThreadTask"*, %"class.MARC::IThreadTask"** %11, align 8, !tbaa !27
  %33 = icmp eq %"class.MARC::IThreadTask"* %32, null
  br i1 %33, label %39, label %34

; <label>:34:                                     ; preds = %31
  %35 = bitcast %"class.MARC::IThreadTask"* %32 to void (%"class.MARC::IThreadTask"*)***
  %36 = load void (%"class.MARC::IThreadTask"*)**, void (%"class.MARC::IThreadTask"*)*** %35, align 8, !tbaa !133
  %37 = getelementptr inbounds void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %36, i64 2
  %38 = load void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %37, align 8
  call void %38(%"class.MARC::IThreadTask"* nonnull %32) #8
  br label %39

; <label>:39:                                     ; preds = %31, %34
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %10) #8
  %40 = load atomic i8, i8* %4 seq_cst, align 1
  %41 = and i8 %40, 1
  %42 = icmp eq i8 %41, 0
  br i1 %42, label %13, label %43

; <label>:43:                                     ; preds = %39, %2
  ret void
}

; Function Attrs: uwtable
define linkonce_odr void @_ZNSt6vectorISt6threadSaIS0_EE12emplace_backIJMN4MARC10ThreadPoolEFvPSt6atomicIbEEPS5_S8_EEEvDpOT_(%"class.std::vector"*, { i64, i64 }* dereferenceable(16), %"class.MARC::ThreadPool"** dereferenceable(8), %"struct.std::atomic"** dereferenceable(8)) local_unnamed_addr #5 comdat align 2 personality i32 (...)* @__gxx_personality_v0 {
  %5 = alloca %"class.std::unique_ptr", align 8
  %6 = getelementptr inbounds %"class.std::vector", %"class.std::vector"* %0, i64 0, i32 0, i32 0, i32 1
  %7 = load %"class.std::thread"*, %"class.std::thread"** %6, align 8, !tbaa !154
  %8 = getelementptr inbounds %"class.std::vector", %"class.std::vector"* %0, i64 0, i32 0, i32 0, i32 2
  %9 = load %"class.std::thread"*, %"class.std::thread"** %8, align 8, !tbaa !286
  %10 = icmp eq %"class.std::thread"* %7, %9
  br i1 %10, label %52, label %11

; <label>:11:                                     ; preds = %4
  %12 = bitcast %"class.std::unique_ptr"* %5 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %12)
  %13 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %7, i64 0, i32 0, i32 0
  store i64 0, i64* %13, align 8, !tbaa !287
  %14 = bitcast { i64, i64 }* %1 to <2 x i64>*
  %15 = load <2 x i64>, <2 x i64>* %14, align 8, !tbaa !18, !noalias !289
  %16 = bitcast %"struct.std::atomic"** %3 to i64*
  %17 = load i64, i64* %16, align 8, !tbaa !27, !noalias !289
  %18 = bitcast %"class.MARC::ThreadPool"** %2 to i64*
  %19 = load i64, i64* %18, align 8, !tbaa !27, !noalias !289
  %20 = tail call i8* @_Znwm(i64 40) #24, !noalias !292
  %21 = bitcast i8* %20 to i32 (...)***
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [5 x i8*] }, { [5 x i8*] }* @_ZTVNSt6thread11_State_implISt12_Bind_simpleIFSt7_Mem_fnIMN4MARC10ThreadPoolEFvPSt6atomicIbEEEPS4_S7_EEEE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %21, align 8, !tbaa !133, !noalias !292
  %22 = getelementptr inbounds i8, i8* %20, i64 8
  %23 = bitcast i8* %22 to i64*
  store i64 %17, i64* %23, align 8, !tbaa !295, !noalias !292
  %24 = getelementptr inbounds i8, i8* %20, i64 16
  %25 = bitcast i8* %24 to i64*
  store i64 %19, i64* %25, align 8, !tbaa !297, !noalias !292
  %26 = getelementptr inbounds i8, i8* %20, i64 24
  %27 = bitcast i8* %26 to <2 x i64>*
  store <2 x i64> %15, <2 x i64>* %27, align 8
  %28 = bitcast %"class.std::unique_ptr"* %5 to i8**
  store i8* %20, i8** %28, align 8, !tbaa !27, !alias.scope !292
  invoke void @_ZNSt6thread15_M_start_threadESt10unique_ptrINS_6_StateESt14default_deleteIS1_EEPFvvE(%"class.std::thread"* %7, %"class.std::unique_ptr"* nonnull %5, void ()* bitcast (i32 (i64*, %union.pthread_attr_t*, i8* (i8*)*, i8*)* @pthread_create to void ()*))
          to label %29 unwind label %38

; <label>:29:                                     ; preds = %11
  %30 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %5, i64 0, i32 0, i32 0, i32 0, i32 0
  %31 = load %"class.MARC::IThreadTask"*, %"class.MARC::IThreadTask"** %30, align 8, !tbaa !27
  %32 = icmp eq %"class.MARC::IThreadTask"* %31, null
  br i1 %32, label %49, label %33

; <label>:33:                                     ; preds = %29
  %34 = bitcast %"class.MARC::IThreadTask"* %31 to void (%"class.MARC::IThreadTask"*)***
  %35 = load void (%"class.MARC::IThreadTask"*)**, void (%"class.MARC::IThreadTask"*)*** %34, align 8, !tbaa !133
  %36 = getelementptr inbounds void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %35, i64 1
  %37 = load void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %36, align 8
  call void %37(%"class.MARC::IThreadTask"* nonnull %31) #8
  br label %49

; <label>:38:                                     ; preds = %11
  %39 = landingpad { i8*, i32 }
          cleanup
  %40 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %5, i64 0, i32 0, i32 0, i32 0, i32 0
  %41 = load %"class.MARC::IThreadTask"*, %"class.MARC::IThreadTask"** %40, align 8, !tbaa !27
  %42 = icmp eq %"class.MARC::IThreadTask"* %41, null
  br i1 %42, label %48, label %43

; <label>:43:                                     ; preds = %38
  %44 = bitcast %"class.MARC::IThreadTask"* %41 to void (%"class.MARC::IThreadTask"*)***
  %45 = load void (%"class.MARC::IThreadTask"*)**, void (%"class.MARC::IThreadTask"*)*** %44, align 8, !tbaa !133
  %46 = getelementptr inbounds void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %45, i64 1
  %47 = load void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %46, align 8
  call void %47(%"class.MARC::IThreadTask"* nonnull %41) #8
  br label %48

; <label>:48:                                     ; preds = %43, %38
  resume { i8*, i32 } %39

; <label>:49:                                     ; preds = %29, %33
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %12)
  %50 = load %"class.std::thread"*, %"class.std::thread"** %6, align 8, !tbaa !154
  %51 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %50, i64 1
  store %"class.std::thread"* %51, %"class.std::thread"** %6, align 8, !tbaa !154
  br label %53

; <label>:52:                                     ; preds = %4
  tail call void @_ZNSt6vectorISt6threadSaIS0_EE19_M_emplace_back_auxIJMN4MARC10ThreadPoolEFvPSt6atomicIbEEPS5_S8_EEEvDpOT_(%"class.std::vector"* nonnull %0, { i64, i64 }* nonnull dereferenceable(16) %1, %"class.MARC::ThreadPool"** nonnull dereferenceable(8) %2, %"struct.std::atomic"** nonnull dereferenceable(8) %3)
  br label %53

; <label>:53:                                     ; preds = %52, %49
  ret void
}

; Function Attrs: uwtable
define linkonce_odr void @_ZN4MARC15ThreadSafeQueueISt8functionIFvvEEE4pushES3_(%"class.MARC::ThreadSafeQueue.45"*, %"class.std::function"*) local_unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %3 = alloca { i64, i64 }, align 8
  %4 = alloca { i64, i64 }, align 8
  %5 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %6, label %11

; <label>:6:                                      ; preds = %2
  %7 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %5, i64 0, i32 0, i32 0
  %8 = tail call i32 @pthread_mutex_lock(%union.pthread_mutex_t* nonnull %7) #8
  %9 = icmp eq i32 %8, 0
  br i1 %9, label %11, label %10

; <label>:10:                                     ; preds = %6
  tail call void @_ZSt20__throw_system_errori(i32 %8) #23
  unreachable

; <label>:11:                                     ; preds = %2, %6
  %12 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 0
  %13 = load %"class.std::function"*, %"class.std::function"** %12, align 8, !tbaa !299
  %14 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 2
  %15 = load %"class.std::function"*, %"class.std::function"** %14, align 8, !tbaa !300
  %16 = getelementptr inbounds %"class.std::function", %"class.std::function"* %15, i64 -1
  %17 = icmp eq %"class.std::function"* %13, %16
  br i1 %17, label %36, label %18

; <label>:18:                                     ; preds = %11
  %19 = getelementptr inbounds %"class.std::function", %"class.std::function"* %13, i64 0, i32 0, i32 1
  store i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* null, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %19, align 8, !tbaa !123
  %20 = bitcast { i64, i64 }* %4 to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %20)
  %21 = bitcast %"class.std::function"* %1 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %20, i8* nonnull %21, i64 16, i32 8, i1 false) #8
  %22 = bitcast %"class.std::function"* %13 to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %21, i8* nonnull %22, i64 16, i32 8, i1 false) #8, !tbaa.struct !236
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %22, i8* nonnull %20, i64 16, i32 8, i1 false) #8
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %20)
  %23 = getelementptr inbounds %"class.std::function", %"class.std::function"* %1, i64 0, i32 0, i32 1
  %24 = bitcast i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %23 to i64*
  %25 = load i64, i64* %24, align 8, !tbaa !27
  %26 = bitcast i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %19 to i64*
  %27 = load i64, i64* %26, align 8, !tbaa !27
  store i64 %27, i64* %24, align 8, !tbaa !27
  store i64 %25, i64* %26, align 8, !tbaa !27
  %28 = getelementptr inbounds %"class.std::function", %"class.std::function"* %1, i64 0, i32 1
  %29 = getelementptr inbounds %"class.std::function", %"class.std::function"* %13, i64 0, i32 1
  %30 = bitcast void (%"union.std::_Any_data"*)** %28 to i64*
  %31 = load i64, i64* %30, align 8, !tbaa !27
  %32 = bitcast void (%"union.std::_Any_data"*)** %29 to i64*
  %33 = load i64, i64* %32, align 8, !tbaa !27
  store i64 %33, i64* %30, align 8, !tbaa !27
  store i64 %31, i64* %32, align 8, !tbaa !27
  %34 = load %"class.std::function"*, %"class.std::function"** %12, align 8, !tbaa !299
  %35 = getelementptr inbounds %"class.std::function", %"class.std::function"* %34, i64 1
  store %"class.std::function"* %35, %"class.std::function"** %12, align 8, !tbaa !299
  br label %81

; <label>:36:                                     ; preds = %11
  %37 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 2, i32 0
  %38 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 1
  %39 = load i64, i64* %38, align 8, !tbaa !301
  %40 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 3
  %41 = bitcast %"class.std::function"*** %40 to i64*
  %42 = load i64, i64* %41, align 8, !tbaa !232
  %43 = bitcast %"class.std::deque.47"* %37 to i64*
  %44 = load i64, i64* %43, align 8, !tbaa !228
  %45 = sub i64 %42, %44
  %46 = ashr exact i64 %45, 3
  %47 = sub i64 %39, %46
  %48 = icmp ult i64 %47, 2
  br i1 %48, label %49, label %50

; <label>:49:                                     ; preds = %36
  invoke void @_ZNSt5dequeISt8functionIFvvEESaIS2_EE17_M_reallocate_mapEmb(%"class.std::deque.47"* nonnull %37, i64 1, i1 zeroext false)
          to label %50 unwind label %87

; <label>:50:                                     ; preds = %49, %36
  %51 = invoke i8* @_Znwm(i64 512)
          to label %52 unwind label %87

; <label>:52:                                     ; preds = %50
  %53 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3
  %54 = load %"class.std::function"**, %"class.std::function"*** %40, align 8, !tbaa !232
  %55 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %54, i64 1
  %56 = bitcast %"class.std::function"** %55 to i8**
  store i8* %51, i8** %56, align 8, !tbaa !27
  %57 = getelementptr inbounds %"struct.std::_Deque_iterator.52", %"struct.std::_Deque_iterator.52"* %53, i64 0, i32 0
  %58 = load %"class.std::function"*, %"class.std::function"** %57, align 8, !tbaa !299
  %59 = getelementptr inbounds %"class.std::function", %"class.std::function"* %58, i64 0, i32 0, i32 1
  store i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* null, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %59, align 8, !tbaa !123
  %60 = bitcast { i64, i64 }* %3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %60)
  %61 = bitcast %"class.std::function"* %1 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %60, i8* nonnull %61, i64 16, i32 8, i1 false) #8
  %62 = bitcast %"class.std::function"* %58 to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %61, i8* nonnull %62, i64 16, i32 8, i1 false) #8, !tbaa.struct !236
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %62, i8* nonnull %60, i64 16, i32 8, i1 false) #8
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %60)
  %63 = getelementptr inbounds %"class.std::function", %"class.std::function"* %1, i64 0, i32 0, i32 1
  %64 = bitcast i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %63 to i64*
  %65 = load i64, i64* %64, align 8, !tbaa !27
  %66 = bitcast i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %59 to i64*
  %67 = load i64, i64* %66, align 8, !tbaa !27
  store i64 %67, i64* %64, align 8, !tbaa !27
  store i64 %65, i64* %66, align 8, !tbaa !27
  %68 = getelementptr inbounds %"class.std::function", %"class.std::function"* %1, i64 0, i32 1
  %69 = getelementptr inbounds %"class.std::function", %"class.std::function"* %58, i64 0, i32 1
  %70 = bitcast void (%"union.std::_Any_data"*)** %68 to i64*
  %71 = load i64, i64* %70, align 8, !tbaa !27
  %72 = bitcast void (%"union.std::_Any_data"*)** %69 to i64*
  %73 = load i64, i64* %72, align 8, !tbaa !27
  store i64 %73, i64* %70, align 8, !tbaa !27
  store i64 %71, i64* %72, align 8, !tbaa !27
  %74 = load %"class.std::function"**, %"class.std::function"*** %40, align 8, !tbaa !232
  %75 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %74, i64 1
  store %"class.std::function"** %75, %"class.std::function"*** %40, align 8, !tbaa !207
  %76 = load %"class.std::function"*, %"class.std::function"** %75, align 8, !tbaa !27
  %77 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 1
  store %"class.std::function"* %76, %"class.std::function"** %77, align 8, !tbaa !210
  %78 = getelementptr inbounds %"class.std::function", %"class.std::function"* %76, i64 16
  store %"class.std::function"* %78, %"class.std::function"** %14, align 8, !tbaa !211
  %79 = ptrtoint %"class.std::function"* %76 to i64
  %80 = bitcast %"struct.std::_Deque_iterator.52"* %53 to i64*
  store i64 %79, i64* %80, align 8, !tbaa !299
  br label %81

; <label>:81:                                     ; preds = %52, %18
  %82 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 3
  tail call void @_ZNSt18condition_variable10notify_oneEv(%"class.std::condition_variable"* %82) #8
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %83, label %86

; <label>:83:                                     ; preds = %81
  %84 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %5, i64 0, i32 0, i32 0
  %85 = tail call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %84) #8
  br label %86

; <label>:86:                                     ; preds = %81, %83
  ret void

; <label>:87:                                     ; preds = %50, %49
  %88 = landingpad { i8*, i32 }
          cleanup
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %89, label %92

; <label>:89:                                     ; preds = %87
  %90 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %5, i64 0, i32 0, i32 0
  %91 = tail call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %90) #8
  br label %92

; <label>:92:                                     ; preds = %87, %89
  resume { i8*, i32 } %88
}

; Function Attrs: uwtable
define linkonce_odr void @_ZNSt5dequeISt8functionIFvvEESaIS2_EE17_M_reallocate_mapEmb(%"class.std::deque.47"*, i64, i1 zeroext) local_unnamed_addr #5 comdat align 2 personality i32 (...)* @__gxx_personality_v0 {
  %4 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %0, i64 0, i32 0, i32 0, i32 3, i32 3
  %5 = load %"class.std::function"**, %"class.std::function"*** %4, align 8, !tbaa !232
  %6 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %0, i64 0, i32 0, i32 0, i32 2, i32 3
  %7 = load %"class.std::function"**, %"class.std::function"*** %6, align 8, !tbaa !231
  %8 = ptrtoint %"class.std::function"** %5 to i64
  %9 = ptrtoint %"class.std::function"** %7 to i64
  %10 = sub i64 %8, %9
  %11 = ashr exact i64 %10, 3
  %12 = add nsw i64 %11, 1
  %13 = add i64 %12, %1
  %14 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %0, i64 0, i32 0, i32 0, i32 1
  %15 = load i64, i64* %14, align 8, !tbaa !301
  %16 = shl i64 %13, 1
  %17 = icmp ugt i64 %15, %16
  br i1 %17, label %18, label %44

; <label>:18:                                     ; preds = %3
  %19 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %0, i64 0, i32 0, i32 0, i32 0
  %20 = load %"class.std::function"**, %"class.std::function"*** %19, align 8, !tbaa !228
  %21 = sub i64 %15, %13
  %22 = lshr i64 %21, 1
  %23 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %20, i64 %22
  %24 = select i1 %2, i64 %1, i64 0
  %25 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %23, i64 %24
  %26 = icmp ult %"class.std::function"** %25, %7
  %27 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %5, i64 1
  %28 = ptrtoint %"class.std::function"** %27 to i64
  %29 = sub i64 %28, %9
  br i1 %26, label %30, label %35

; <label>:30:                                     ; preds = %18
  %31 = icmp eq i64 %29, 0
  br i1 %31, label %73, label %32

; <label>:32:                                     ; preds = %30
  %33 = bitcast %"class.std::function"** %25 to i8*
  %34 = bitcast %"class.std::function"** %7 to i8*
  tail call void @llvm.memmove.p0i8.p0i8.i64(i8* %33, i8* %34, i64 %29, i32 8, i1 false) #8
  br label %73

; <label>:35:                                     ; preds = %18
  %36 = ashr exact i64 %29, 3
  %37 = icmp eq i64 %36, 0
  br i1 %37, label %73, label %38

; <label>:38:                                     ; preds = %35
  %39 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %25, i64 %12
  %40 = sub nsw i64 0, %36
  %41 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %39, i64 %40
  %42 = bitcast %"class.std::function"** %41 to i8*
  %43 = bitcast %"class.std::function"** %7 to i8*
  tail call void @llvm.memmove.p0i8.p0i8.i64(i8* %42, i8* %43, i64 %29, i32 8, i1 false) #8
  br label %73

; <label>:44:                                     ; preds = %3
  %45 = icmp ult i64 %15, %1
  %46 = select i1 %45, i64 %1, i64 %15
  %47 = add i64 %15, 2
  %48 = add i64 %47, %46
  %49 = icmp ugt i64 %48, 2305843009213693951
  br i1 %49, label %50, label %51

; <label>:50:                                     ; preds = %44
  tail call void @_ZSt17__throw_bad_allocv() #23
  unreachable

; <label>:51:                                     ; preds = %44
  %52 = shl i64 %48, 3
  %53 = tail call i8* @_Znwm(i64 %52)
  %54 = bitcast i8* %53 to %"class.std::function"**
  %55 = sub i64 %48, %13
  %56 = lshr i64 %55, 1
  %57 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %54, i64 %56
  %58 = select i1 %2, i64 %1, i64 0
  %59 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %57, i64 %58
  %60 = load %"class.std::function"**, %"class.std::function"*** %6, align 8, !tbaa !231
  %61 = load %"class.std::function"**, %"class.std::function"*** %4, align 8, !tbaa !232
  %62 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %61, i64 1
  %63 = ptrtoint %"class.std::function"** %62 to i64
  %64 = ptrtoint %"class.std::function"** %60 to i64
  %65 = sub i64 %63, %64
  %66 = icmp eq i64 %65, 0
  br i1 %66, label %70, label %67

; <label>:67:                                     ; preds = %51
  %68 = bitcast %"class.std::function"** %59 to i8*
  %69 = bitcast %"class.std::function"** %60 to i8*
  tail call void @llvm.memmove.p0i8.p0i8.i64(i8* %68, i8* %69, i64 %65, i32 8, i1 false) #8
  br label %70

; <label>:70:                                     ; preds = %51, %67
  %71 = bitcast %"class.std::deque.47"* %0 to i8**
  %72 = load i8*, i8** %71, align 8, !tbaa !228
  tail call void @_ZdlPv(i8* %72) #8
  store i8* %53, i8** %71, align 8, !tbaa !228
  store i64 %48, i64* %14, align 8, !tbaa !301
  br label %73

; <label>:73:                                     ; preds = %38, %35, %32, %30, %70
  %74 = phi %"class.std::function"** [ %59, %70 ], [ %25, %30 ], [ %25, %32 ], [ %25, %35 ], [ %25, %38 ]
  store %"class.std::function"** %74, %"class.std::function"*** %6, align 8, !tbaa !207
  %75 = load %"class.std::function"*, %"class.std::function"** %74, align 8, !tbaa !27
  %76 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %0, i64 0, i32 0, i32 0, i32 2, i32 1
  store %"class.std::function"* %75, %"class.std::function"** %76, align 8, !tbaa !210
  %77 = getelementptr inbounds %"class.std::function", %"class.std::function"* %75, i64 16
  %78 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %0, i64 0, i32 0, i32 0, i32 2, i32 2
  store %"class.std::function"* %77, %"class.std::function"** %78, align 8, !tbaa !211
  %79 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %74, i64 %11
  store %"class.std::function"** %79, %"class.std::function"*** %4, align 8, !tbaa !207
  %80 = load %"class.std::function"*, %"class.std::function"** %79, align 8, !tbaa !27
  %81 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %0, i64 0, i32 0, i32 0, i32 3, i32 1
  store %"class.std::function"* %80, %"class.std::function"** %81, align 8, !tbaa !210
  %82 = getelementptr inbounds %"class.std::function", %"class.std::function"* %80, i64 16
  %83 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %0, i64 0, i32 0, i32 0, i32 3, i32 2
  store %"class.std::function"* %82, %"class.std::function"** %83, align 8, !tbaa !211
  ret void
}

; Function Attrs: nounwind
declare i32 @pthread_create(i64*, %union.pthread_attr_t*, i8* (i8*)*, i8*) #7

declare void @_ZNSt6thread15_M_start_threadESt10unique_ptrINS_6_StateESt14default_deleteIS1_EEPFvvE(%"class.std::thread"*, %"class.std::unique_ptr"*, void ()*) local_unnamed_addr #6

; Function Attrs: uwtable
define linkonce_odr void @_ZNSt6vectorISt6threadSaIS0_EE19_M_emplace_back_auxIJMN4MARC10ThreadPoolEFvPSt6atomicIbEEPS5_S8_EEEvDpOT_(%"class.std::vector"*, { i64, i64 }* dereferenceable(16), %"class.MARC::ThreadPool"** dereferenceable(8), %"struct.std::atomic"** dereferenceable(8)) local_unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %5 = alloca %"class.std::unique_ptr", align 8
  %6 = getelementptr inbounds %"class.std::vector", %"class.std::vector"* %0, i64 0, i32 0, i32 0, i32 1
  %7 = bitcast %"class.std::thread"** %6 to i64*
  %8 = load i64, i64* %7, align 8, !tbaa !154
  %9 = bitcast %"class.std::vector"* %0 to i64*
  %10 = load i64, i64* %9, align 8, !tbaa !151
  %11 = sub i64 %8, %10
  %12 = ashr exact i64 %11, 3
  %13 = icmp eq i64 %12, 0
  %14 = select i1 %13, i64 1, i64 %12
  %15 = add nsw i64 %14, %12
  %16 = icmp ult i64 %15, %12
  %17 = icmp ugt i64 %15, 2305843009213693951
  %18 = or i1 %16, %17
  %19 = select i1 %18, i64 2305843009213693951, i64 %15
  %20 = icmp eq i64 %19, 0
  br i1 %20, label %30, label %21

; <label>:21:                                     ; preds = %4
  %22 = icmp ugt i64 %19, 2305843009213693951
  br i1 %22, label %23, label %24

; <label>:23:                                     ; preds = %21
  tail call void @_ZSt17__throw_bad_allocv() #23
  unreachable

; <label>:24:                                     ; preds = %21
  %25 = shl i64 %19, 3
  %26 = tail call i8* @_Znwm(i64 %25)
  %27 = bitcast i8* %26 to %"class.std::thread"*
  %28 = load i64, i64* %7, align 8, !tbaa !154
  %29 = load i64, i64* %9, align 8, !tbaa !151
  br label %30

; <label>:30:                                     ; preds = %4, %24
  %31 = phi i64 [ %29, %24 ], [ %10, %4 ]
  %32 = phi i64 [ %28, %24 ], [ %8, %4 ]
  %33 = phi i8* [ %26, %24 ], [ null, %4 ]
  %34 = phi %"class.std::thread"* [ %27, %24 ], [ null, %4 ]
  %35 = sub i64 %32, %31
  %36 = ashr exact i64 %35, 3
  %37 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %34, i64 %36
  %38 = bitcast %"class.std::unique_ptr"* %5 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %38)
  %39 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %37, i64 0, i32 0, i32 0
  store i64 0, i64* %39, align 8, !tbaa !287
  %40 = bitcast { i64, i64 }* %1 to <2 x i64>*
  %41 = load <2 x i64>, <2 x i64>* %40, align 8, !tbaa !18, !noalias !302
  %42 = bitcast %"struct.std::atomic"** %3 to i64*
  %43 = load i64, i64* %42, align 8, !tbaa !27, !noalias !302
  %44 = bitcast %"class.MARC::ThreadPool"** %2 to i64*
  %45 = load i64, i64* %44, align 8, !tbaa !27, !noalias !302
  %46 = invoke i8* @_Znwm(i64 40) #24
          to label %47 unwind label %238

; <label>:47:                                     ; preds = %30
  %48 = bitcast i8* %46 to i32 (...)***
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [5 x i8*] }, { [5 x i8*] }* @_ZTVNSt6thread11_State_implISt12_Bind_simpleIFSt7_Mem_fnIMN4MARC10ThreadPoolEFvPSt6atomicIbEEEPS4_S7_EEEE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %48, align 8, !tbaa !133, !noalias !305
  %49 = getelementptr inbounds i8, i8* %46, i64 8
  %50 = bitcast i8* %49 to i64*
  store i64 %43, i64* %50, align 8, !tbaa !295, !noalias !305
  %51 = getelementptr inbounds i8, i8* %46, i64 16
  %52 = bitcast i8* %51 to i64*
  store i64 %45, i64* %52, align 8, !tbaa !297, !noalias !305
  %53 = getelementptr inbounds i8, i8* %46, i64 24
  %54 = bitcast i8* %53 to <2 x i64>*
  store <2 x i64> %41, <2 x i64>* %54, align 8
  %55 = bitcast %"class.std::unique_ptr"* %5 to i8**
  store i8* %46, i8** %55, align 8, !tbaa !27, !alias.scope !305
  invoke void @_ZNSt6thread15_M_start_threadESt10unique_ptrINS_6_StateESt14default_deleteIS1_EEPFvvE(%"class.std::thread"* %37, %"class.std::unique_ptr"* nonnull %5, void ()* bitcast (i32 (i64*, %union.pthread_attr_t*, i8* (i8*)*, i8*)* @pthread_create to void ()*))
          to label %56 unwind label %65

; <label>:56:                                     ; preds = %47
  %57 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %5, i64 0, i32 0, i32 0, i32 0, i32 0
  %58 = load %"class.MARC::IThreadTask"*, %"class.MARC::IThreadTask"** %57, align 8, !tbaa !27
  %59 = icmp eq %"class.MARC::IThreadTask"* %58, null
  br i1 %59, label %75, label %60

; <label>:60:                                     ; preds = %56
  %61 = bitcast %"class.MARC::IThreadTask"* %58 to void (%"class.MARC::IThreadTask"*)***
  %62 = load void (%"class.MARC::IThreadTask"*)**, void (%"class.MARC::IThreadTask"*)*** %61, align 8, !tbaa !133
  %63 = getelementptr inbounds void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %62, i64 1
  %64 = load void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %63, align 8
  call void %64(%"class.MARC::IThreadTask"* nonnull %58) #8
  br label %75

; <label>:65:                                     ; preds = %47
  %66 = landingpad { i8*, i32 }
          catch i8* null
  %67 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %5, i64 0, i32 0, i32 0, i32 0, i32 0
  %68 = load %"class.MARC::IThreadTask"*, %"class.MARC::IThreadTask"** %67, align 8, !tbaa !27
  %69 = icmp eq %"class.MARC::IThreadTask"* %68, null
  br i1 %69, label %240, label %70

; <label>:70:                                     ; preds = %65
  %71 = bitcast %"class.MARC::IThreadTask"* %68 to void (%"class.MARC::IThreadTask"*)***
  %72 = load void (%"class.MARC::IThreadTask"*)**, void (%"class.MARC::IThreadTask"*)*** %71, align 8, !tbaa !133
  %73 = getelementptr inbounds void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %72, i64 1
  %74 = load void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %73, align 8
  call void %74(%"class.MARC::IThreadTask"* nonnull %68) #8
  br label %240

; <label>:75:                                     ; preds = %60, %56
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %38)
  %76 = getelementptr inbounds %"class.std::vector", %"class.std::vector"* %0, i64 0, i32 0, i32 0, i32 0
  %77 = load %"class.std::thread"*, %"class.std::thread"** %76, align 8, !tbaa !151
  %78 = bitcast %"class.std::thread"* %77 to i8*
  %79 = ptrtoint %"class.std::thread"* %77 to i64
  %80 = load %"class.std::thread"*, %"class.std::thread"** %6, align 8, !tbaa !154
  %81 = icmp eq %"class.std::thread"* %77, %80
  br i1 %81, label %229, label %82

; <label>:82:                                     ; preds = %75
  %83 = getelementptr %"class.std::thread", %"class.std::thread"* %80, i64 -1, i32 0, i32 0
  %84 = getelementptr %"class.std::thread", %"class.std::thread"* %80, i64 -1, i32 0, i32 0
  %85 = ptrtoint i64* %84 to i64
  %86 = sub i64 %85, %79
  %87 = lshr i64 %86, 3
  %88 = add nuw nsw i64 %87, 1
  %89 = icmp ult i64 %88, 4
  br i1 %89, label %168, label %90

; <label>:90:                                     ; preds = %82
  %91 = getelementptr %"class.std::thread", %"class.std::thread"* %80, i64 -1, i32 0, i32 0
  %92 = ptrtoint i64* %91 to i64
  %93 = sub i64 %92, %79
  %94 = lshr i64 %93, 3
  %95 = add nuw nsw i64 %94, 1
  %96 = getelementptr %"class.std::thread", %"class.std::thread"* %34, i64 %95
  %97 = getelementptr %"class.std::thread", %"class.std::thread"* %77, i64 %95
  %98 = bitcast %"class.std::thread"* %97 to i8*
  %99 = icmp ult i8* %33, %98
  %100 = icmp ult %"class.std::thread"* %77, %96
  %101 = and i1 %99, %100
  br i1 %101, label %168, label %102

; <label>:102:                                    ; preds = %90
  %103 = and i64 %88, 4611686018427387900
  %104 = getelementptr %"class.std::thread", %"class.std::thread"* %34, i64 %103
  %105 = getelementptr %"class.std::thread", %"class.std::thread"* %77, i64 %103
  %106 = add nsw i64 %103, -4
  %107 = lshr exact i64 %106, 2
  %108 = add nuw nsw i64 %107, 1
  %109 = and i64 %108, 1
  %110 = icmp eq i64 %107, 0
  br i1 %110, label %148, label %111

; <label>:111:                                    ; preds = %102
  %112 = sub nsw i64 %108, %109
  br label %113

; <label>:113:                                    ; preds = %113, %111
  %114 = phi i64 [ 0, %111 ], [ %145, %113 ]
  %115 = phi i64 [ %112, %111 ], [ %146, %113 ]
  %116 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %34, i64 %114, i32 0, i32 0
  %117 = bitcast i64* %116 to <2 x i64>*
  store <2 x i64> zeroinitializer, <2 x i64>* %117, align 8, !tbaa !287, !alias.scope !308, !noalias !311
  %118 = getelementptr i64, i64* %116, i64 2
  %119 = bitcast i64* %118 to <2 x i64>*
  store <2 x i64> zeroinitializer, <2 x i64>* %119, align 8, !tbaa !287, !alias.scope !308, !noalias !311
  %120 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %77, i64 %114, i32 0, i32 0
  %121 = bitcast i64* %120 to <2 x i64>*
  %122 = load <2 x i64>, <2 x i64>* %121, align 8, !tbaa !84, !alias.scope !311
  %123 = getelementptr i64, i64* %120, i64 2
  %124 = bitcast i64* %123 to <2 x i64>*
  %125 = load <2 x i64>, <2 x i64>* %124, align 8, !tbaa !84, !alias.scope !311
  %126 = bitcast i64* %116 to <2 x i64>*
  store <2 x i64> %122, <2 x i64>* %126, align 8, !tbaa !84, !alias.scope !308, !noalias !311
  %127 = bitcast i64* %118 to <2 x i64>*
  store <2 x i64> %125, <2 x i64>* %127, align 8, !tbaa !84, !alias.scope !308, !noalias !311
  %128 = bitcast i64* %120 to <2 x i64>*
  store <2 x i64> zeroinitializer, <2 x i64>* %128, align 8, !tbaa !84, !alias.scope !311
  %129 = bitcast i64* %123 to <2 x i64>*
  store <2 x i64> zeroinitializer, <2 x i64>* %129, align 8, !tbaa !84, !alias.scope !311
  %130 = or i64 %114, 4
  %131 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %34, i64 %130, i32 0, i32 0
  %132 = bitcast i64* %131 to <2 x i64>*
  store <2 x i64> zeroinitializer, <2 x i64>* %132, align 8, !tbaa !287, !alias.scope !308, !noalias !311
  %133 = getelementptr i64, i64* %131, i64 2
  %134 = bitcast i64* %133 to <2 x i64>*
  store <2 x i64> zeroinitializer, <2 x i64>* %134, align 8, !tbaa !287, !alias.scope !308, !noalias !311
  %135 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %77, i64 %130, i32 0, i32 0
  %136 = bitcast i64* %135 to <2 x i64>*
  %137 = load <2 x i64>, <2 x i64>* %136, align 8, !tbaa !84, !alias.scope !311
  %138 = getelementptr i64, i64* %135, i64 2
  %139 = bitcast i64* %138 to <2 x i64>*
  %140 = load <2 x i64>, <2 x i64>* %139, align 8, !tbaa !84, !alias.scope !311
  %141 = bitcast i64* %131 to <2 x i64>*
  store <2 x i64> %137, <2 x i64>* %141, align 8, !tbaa !84, !alias.scope !308, !noalias !311
  %142 = bitcast i64* %133 to <2 x i64>*
  store <2 x i64> %140, <2 x i64>* %142, align 8, !tbaa !84, !alias.scope !308, !noalias !311
  %143 = bitcast i64* %135 to <2 x i64>*
  store <2 x i64> zeroinitializer, <2 x i64>* %143, align 8, !tbaa !84, !alias.scope !311
  %144 = bitcast i64* %138 to <2 x i64>*
  store <2 x i64> zeroinitializer, <2 x i64>* %144, align 8, !tbaa !84, !alias.scope !311
  %145 = add i64 %114, 8
  %146 = add i64 %115, -2
  %147 = icmp eq i64 %146, 0
  br i1 %147, label %148, label %113, !llvm.loop !313

; <label>:148:                                    ; preds = %113, %102
  %149 = phi i64 [ 0, %102 ], [ %145, %113 ]
  %150 = icmp eq i64 %109, 0
  br i1 %150, label %166, label %151

; <label>:151:                                    ; preds = %148
  %152 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %34, i64 %149, i32 0, i32 0
  %153 = bitcast i64* %152 to <2 x i64>*
  store <2 x i64> zeroinitializer, <2 x i64>* %153, align 8, !tbaa !287, !alias.scope !308, !noalias !311
  %154 = getelementptr i64, i64* %152, i64 2
  %155 = bitcast i64* %154 to <2 x i64>*
  store <2 x i64> zeroinitializer, <2 x i64>* %155, align 8, !tbaa !287, !alias.scope !308, !noalias !311
  %156 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %77, i64 %149, i32 0, i32 0
  %157 = bitcast i64* %156 to <2 x i64>*
  %158 = load <2 x i64>, <2 x i64>* %157, align 8, !tbaa !84, !alias.scope !311
  %159 = getelementptr i64, i64* %156, i64 2
  %160 = bitcast i64* %159 to <2 x i64>*
  %161 = load <2 x i64>, <2 x i64>* %160, align 8, !tbaa !84, !alias.scope !311
  %162 = bitcast i64* %152 to <2 x i64>*
  store <2 x i64> %158, <2 x i64>* %162, align 8, !tbaa !84, !alias.scope !308, !noalias !311
  %163 = bitcast i64* %154 to <2 x i64>*
  store <2 x i64> %161, <2 x i64>* %163, align 8, !tbaa !84, !alias.scope !308, !noalias !311
  %164 = bitcast i64* %156 to <2 x i64>*
  store <2 x i64> zeroinitializer, <2 x i64>* %164, align 8, !tbaa !84, !alias.scope !311
  %165 = bitcast i64* %159 to <2 x i64>*
  store <2 x i64> zeroinitializer, <2 x i64>* %165, align 8, !tbaa !84, !alias.scope !311
  br label %166

; <label>:166:                                    ; preds = %148, %151
  %167 = icmp eq i64 %88, %103
  br i1 %167, label %214, label %168

; <label>:168:                                    ; preds = %166, %90, %82
  %169 = phi %"class.std::thread"* [ %34, %90 ], [ %34, %82 ], [ %104, %166 ]
  %170 = phi %"class.std::thread"* [ %77, %90 ], [ %77, %82 ], [ %105, %166 ]
  %171 = ptrtoint %"class.std::thread"* %170 to i64
  %172 = getelementptr %"class.std::thread", %"class.std::thread"* %80, i64 -1, i32 0, i32 0
  %173 = ptrtoint i64* %172 to i64
  %174 = sub i64 %173, %171
  %175 = lshr i64 %174, 3
  %176 = add nuw nsw i64 %175, 1
  %177 = and i64 %176, 3
  %178 = icmp eq i64 %177, 0
  br i1 %178, label %191, label %179

; <label>:179:                                    ; preds = %168
  br label %180

; <label>:180:                                    ; preds = %180, %179
  %181 = phi %"class.std::thread"* [ %188, %180 ], [ %169, %179 ]
  %182 = phi %"class.std::thread"* [ %187, %180 ], [ %170, %179 ]
  %183 = phi i64 [ %189, %180 ], [ %177, %179 ]
  %184 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %181, i64 0, i32 0, i32 0
  store i64 0, i64* %184, align 8, !tbaa !287
  %185 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %182, i64 0, i32 0, i32 0
  %186 = load i64, i64* %185, align 8, !tbaa !84
  store i64 %186, i64* %184, align 8, !tbaa !84
  store i64 0, i64* %185, align 8, !tbaa !84
  %187 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %182, i64 1
  %188 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %181, i64 1
  %189 = add i64 %183, -1
  %190 = icmp eq i64 %189, 0
  br i1 %190, label %191, label %180, !llvm.loop !316

; <label>:191:                                    ; preds = %180, %168
  %192 = phi %"class.std::thread"* [ %169, %168 ], [ %188, %180 ]
  %193 = phi %"class.std::thread"* [ %170, %168 ], [ %187, %180 ]
  %194 = icmp ult i64 %174, 24
  br i1 %194, label %214, label %195

; <label>:195:                                    ; preds = %191
  br label %196

; <label>:196:                                    ; preds = %196, %195
  %197 = phi %"class.std::thread"* [ %192, %195 ], [ %212, %196 ]
  %198 = phi %"class.std::thread"* [ %193, %195 ], [ %211, %196 ]
  %199 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %197, i64 0, i32 0, i32 0
  store i64 0, i64* %199, align 8, !tbaa !287
  %200 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %198, i64 0, i32 0, i32 0
  %201 = load i64, i64* %200, align 8, !tbaa !84
  store i64 %201, i64* %199, align 8, !tbaa !84
  store i64 0, i64* %200, align 8, !tbaa !84
  %202 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %197, i64 1, i32 0, i32 0
  store i64 0, i64* %202, align 8, !tbaa !287
  %203 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %198, i64 1, i32 0, i32 0
  %204 = load i64, i64* %203, align 8, !tbaa !84
  store i64 %204, i64* %202, align 8, !tbaa !84
  store i64 0, i64* %203, align 8, !tbaa !84
  %205 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %197, i64 2, i32 0, i32 0
  store i64 0, i64* %205, align 8, !tbaa !287
  %206 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %198, i64 2, i32 0, i32 0
  %207 = load i64, i64* %206, align 8, !tbaa !84
  store i64 %207, i64* %205, align 8, !tbaa !84
  store i64 0, i64* %206, align 8, !tbaa !84
  %208 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %197, i64 3, i32 0, i32 0
  store i64 0, i64* %208, align 8, !tbaa !287
  %209 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %198, i64 3, i32 0, i32 0
  %210 = load i64, i64* %209, align 8, !tbaa !84
  store i64 %210, i64* %208, align 8, !tbaa !84
  store i64 0, i64* %209, align 8, !tbaa !84
  %211 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %198, i64 4
  %212 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %197, i64 4
  %213 = icmp eq %"class.std::thread"* %211, %80
  br i1 %213, label %214, label %196, !llvm.loop !317

; <label>:214:                                    ; preds = %191, %196, %166
  %215 = ptrtoint i64* %83 to i64
  %216 = sub i64 %215, %79
  %217 = lshr i64 %216, 3
  %218 = add nuw nsw i64 %217, 1
  %219 = getelementptr %"class.std::thread", %"class.std::thread"* %34, i64 %218
  br label %222

; <label>:220:                                    ; preds = %222
  %221 = icmp eq %"class.std::thread"* %227, %80
  br i1 %221, label %229, label %222

; <label>:222:                                    ; preds = %220, %214
  %223 = phi %"class.std::thread"* [ %77, %214 ], [ %227, %220 ]
  %224 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %223, i64 0, i32 0, i32 0
  %225 = load i64, i64* %224, align 8
  %226 = icmp eq i64 %225, 0
  %227 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %223, i64 1
  br i1 %226, label %220, label %228

; <label>:228:                                    ; preds = %222
  call void @_ZSt9terminatev() #22
  unreachable

; <label>:229:                                    ; preds = %220, %75
  %230 = phi %"class.std::thread"* [ %34, %75 ], [ %219, %220 ]
  %231 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %230, i64 1
  %232 = getelementptr inbounds %"class.std::vector", %"class.std::vector"* %0, i64 0, i32 0, i32 0, i32 2
  %233 = icmp eq %"class.std::thread"* %77, null
  br i1 %233, label %235, label %234

; <label>:234:                                    ; preds = %229
  call void @_ZdlPv(i8* %78) #8
  br label %235

; <label>:235:                                    ; preds = %229, %234
  %236 = bitcast %"class.std::vector"* %0 to i8**
  store i8* %33, i8** %236, align 8, !tbaa !151
  store %"class.std::thread"* %231, %"class.std::thread"** %6, align 8, !tbaa !154
  %237 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %34, i64 %19
  store %"class.std::thread"* %237, %"class.std::thread"** %232, align 8, !tbaa !286
  ret void

; <label>:238:                                    ; preds = %30
  %239 = landingpad { i8*, i32 }
          catch i8* null
  br label %240

; <label>:240:                                    ; preds = %65, %70, %238
  %241 = phi { i8*, i32 } [ %239, %238 ], [ %66, %70 ], [ %66, %65 ]
  %242 = extractvalue { i8*, i32 } %241, 0
  %243 = call i8* @__cxa_begin_catch(i8* %242) #8
  %244 = icmp eq %"class.std::thread"* %34, null
  br i1 %244, label %245, label %248

; <label>:245:                                    ; preds = %240
  call void @llvm.trap()
  unreachable

; <label>:246:                                    ; preds = %248
  %247 = landingpad { i8*, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %249 unwind label %250

; <label>:248:                                    ; preds = %240
  call void @_ZdlPv(i8* %33) #8
  invoke void @__cxa_rethrow() #23
          to label %253 unwind label %246

; <label>:249:                                    ; preds = %246
  resume { i8*, i32 } %247

; <label>:250:                                    ; preds = %246
  %251 = landingpad { i8*, i32 }
          catch i8* null
  %252 = extractvalue { i8*, i32 } %251, 0
  call void @__clang_call_terminate(i8* %252) #22
  unreachable

; <label>:253:                                    ; preds = %248
  unreachable
}

; Function Attrs: nounwind
declare void @_ZNSt6thread6_StateD2Ev(%"class.MARC::IThreadTask"*) unnamed_addr #7

; Function Attrs: inlinehint nounwind uwtable
define linkonce_odr void @_ZNSt6thread11_State_implISt12_Bind_simpleIFSt7_Mem_fnIMN4MARC10ThreadPoolEFvPSt6atomicIbEEEPS4_S7_EEED0Ev(%"struct.std::thread::_State_impl"*) unnamed_addr #17 comdat align 2 {
  %2 = getelementptr inbounds %"struct.std::thread::_State_impl", %"struct.std::thread::_State_impl"* %0, i64 0, i32 0
  tail call void @_ZNSt6thread6_StateD2Ev(%"class.MARC::IThreadTask"* %2) #8
  %3 = bitcast %"struct.std::thread::_State_impl"* %0 to i8*
  tail call void @_ZdlPv(i8* %3) #25
  ret void
}

; Function Attrs: uwtable
define linkonce_odr void @_ZNSt6thread11_State_implISt12_Bind_simpleIFSt7_Mem_fnIMN4MARC10ThreadPoolEFvPSt6atomicIbEEEPS4_S7_EEE6_M_runEv(%"struct.std::thread::_State_impl"*) unnamed_addr #5 comdat align 2 {
  %2 = getelementptr inbounds %"struct.std::thread::_State_impl", %"struct.std::thread::_State_impl"* %0, i64 0, i32 1, i32 0, i32 0, i32 0, i32 1, i32 0
  %3 = getelementptr inbounds %"struct.std::thread::_State_impl", %"struct.std::thread::_State_impl"* %0, i64 0, i32 1, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0
  %4 = bitcast %"class.MARC::ThreadPool"** %2 to i8**
  %5 = load i8*, i8** %4, align 8, !tbaa !27
  %6 = getelementptr inbounds %"struct.std::thread::_State_impl", %"struct.std::thread::_State_impl"* %0, i64 0, i32 1, i32 0, i32 0, i32 1, i32 0, i32 0, i32 0, i32 0
  %7 = load i64, i64* %6, align 8, !tbaa !18
  %8 = getelementptr inbounds %"struct.std::thread::_State_impl", %"struct.std::thread::_State_impl"* %0, i64 0, i32 1, i32 0, i32 0, i32 1, i32 0, i32 0, i32 0, i32 1
  %9 = load i64, i64* %8, align 8, !tbaa !18
  %10 = getelementptr inbounds i8, i8* %5, i64 %9
  %11 = bitcast i8* %10 to %"class.MARC::ThreadPool"*
  %12 = and i64 %7, 1
  %13 = icmp eq i64 %12, 0
  br i1 %13, label %21, label %14

; <label>:14:                                     ; preds = %1
  %15 = bitcast i8* %10 to i8**
  %16 = load i8*, i8** %15, align 8, !tbaa !133
  %17 = add i64 %7, -1
  %18 = getelementptr i8, i8* %16, i64 %17
  %19 = bitcast i8* %18 to void (%"class.MARC::ThreadPool"*, %"struct.std::atomic"*)**
  %20 = load void (%"class.MARC::ThreadPool"*, %"struct.std::atomic"*)*, void (%"class.MARC::ThreadPool"*, %"struct.std::atomic"*)** %19, align 8
  br label %23

; <label>:21:                                     ; preds = %1
  %22 = inttoptr i64 %7 to void (%"class.MARC::ThreadPool"*, %"struct.std::atomic"*)*
  br label %23

; <label>:23:                                     ; preds = %14, %21
  %24 = phi void (%"class.MARC::ThreadPool"*, %"struct.std::atomic"*)* [ %20, %14 ], [ %22, %21 ]
  %25 = load %"struct.std::atomic"*, %"struct.std::atomic"** %3, align 8, !tbaa !27
  tail call void %24(%"class.MARC::ThreadPool"* %11, %"struct.std::atomic"* %25)
  ret void
}

; Function Attrs: uwtable
define linkonce_odr zeroext i1 @_ZN4MARC15ThreadSafeQueueISt10unique_ptrINS_11IThreadTaskESt14default_deleteIS2_EEE7waitPopERS5_(%"class.MARC::ThreadSafeQueue.32"*, %"class.std::unique_ptr"* dereferenceable(8)) local_unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %3 = alloca %"class.std::unique_lock", align 8
  %4 = bitcast %"class.std::unique_lock"* %3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %4) #8
  %5 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 1
  %6 = getelementptr inbounds %"class.std::unique_lock", %"class.std::unique_lock"* %3, i64 0, i32 0
  store %"class.std::mutex"* %5, %"class.std::mutex"** %6, align 8, !tbaa !32
  %7 = getelementptr inbounds %"class.std::unique_lock", %"class.std::unique_lock"* %3, i64 0, i32 1
  store i8 0, i8* %7, align 8, !tbaa !35
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %8, label %13

; <label>:8:                                      ; preds = %2
  %9 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %5, i64 0, i32 0, i32 0
  %10 = tail call i32 @pthread_mutex_lock(%union.pthread_mutex_t* nonnull %9) #8
  %11 = icmp eq i32 %10, 0
  br i1 %11, label %13, label %12

; <label>:12:                                     ; preds = %8
  tail call void @_ZSt20__throw_system_errori(i32 %10) #23
  unreachable

; <label>:13:                                     ; preds = %2, %8
  store i8 1, i8* %7, align 8, !tbaa !35
  %14 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 0, i32 0, i32 0
  %15 = load atomic i8, i8* %14 seq_cst, align 1
  %16 = and i8 %15, 1
  %17 = icmp eq i8 %16, 0
  br i1 %17, label %93, label %18

; <label>:18:                                     ; preds = %13
  %19 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 0
  %20 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %19, align 8, !tbaa !214
  %21 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 0
  %22 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %21, align 8, !tbaa !214
  %23 = icmp eq %"class.std::unique_ptr"* %20, %22
  br i1 %23, label %24, label %34

; <label>:24:                                     ; preds = %18
  %25 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 3
  br label %26

; <label>:26:                                     ; preds = %30, %24
  %27 = load atomic i8, i8* %14 seq_cst, align 1
  %28 = and i8 %27, 1
  %29 = icmp eq i8 %28, 0
  br i1 %29, label %34, label %30

; <label>:30:                                     ; preds = %26
  call void @_ZNSt18condition_variable4waitERSt11unique_lockISt5mutexE(%"class.std::condition_variable"* nonnull %25, %"class.std::unique_lock"* nonnull dereferenceable(16) %3) #8
  %31 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %19, align 8, !tbaa !214
  %32 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %21, align 8, !tbaa !214
  %33 = icmp eq %"class.std::unique_ptr"* %31, %32
  br i1 %33, label %26, label %34

; <label>:34:                                     ; preds = %30, %26, %18
  %35 = load atomic i8, i8* %14 seq_cst, align 1
  %36 = and i8 %35, 1
  %37 = icmp eq i8 %36, 0
  br i1 %37, label %93, label %38

; <label>:38:                                     ; preds = %34
  %39 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %21, align 8, !tbaa !214, !noalias !318
  %40 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %39, i64 0, i32 0, i32 0, i32 0, i32 0
  %41 = bitcast %"class.std::unique_ptr"* %39 to i64*
  %42 = load i64, i64* %41, align 8, !tbaa !27
  store %"class.MARC::IThreadTask"* null, %"class.MARC::IThreadTask"** %40, align 8, !tbaa !27
  %43 = bitcast %"class.std::unique_ptr"* %1 to i64*
  %44 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %1, i64 0, i32 0, i32 0, i32 0, i32 0
  %45 = load %"class.MARC::IThreadTask"*, %"class.MARC::IThreadTask"** %44, align 8, !tbaa !27
  store i64 %42, i64* %43, align 8, !tbaa !27
  %46 = icmp eq %"class.MARC::IThreadTask"* %45, null
  br i1 %46, label %52, label %47

; <label>:47:                                     ; preds = %38
  %48 = bitcast %"class.MARC::IThreadTask"* %45 to void (%"class.MARC::IThreadTask"*)***
  %49 = load void (%"class.MARC::IThreadTask"*)**, void (%"class.MARC::IThreadTask"*)*** %48, align 8, !tbaa !133
  %50 = getelementptr inbounds void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %49, i64 2
  %51 = load void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %50, align 8
  call void %51(%"class.MARC::IThreadTask"* nonnull %45) #8
  br label %52

; <label>:52:                                     ; preds = %47, %38
  %53 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %21, align 8, !tbaa !321
  %54 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 2
  %55 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %54, align 8, !tbaa !322
  %56 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %55, i64 -1
  %57 = icmp eq %"class.std::unique_ptr"* %53, %56
  br i1 %57, label %70, label %58

; <label>:58:                                     ; preds = %52
  %59 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %53, i64 0, i32 0, i32 0, i32 0, i32 0
  %60 = load %"class.MARC::IThreadTask"*, %"class.MARC::IThreadTask"** %59, align 8, !tbaa !27
  %61 = icmp eq %"class.MARC::IThreadTask"* %60, null
  br i1 %61, label %67, label %62

; <label>:62:                                     ; preds = %58
  %63 = bitcast %"class.MARC::IThreadTask"* %60 to void (%"class.MARC::IThreadTask"*)***
  %64 = load void (%"class.MARC::IThreadTask"*)**, void (%"class.MARC::IThreadTask"*)*** %63, align 8, !tbaa !133
  %65 = getelementptr inbounds void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %64, i64 2
  %66 = load void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %65, align 8
  call void %66(%"class.MARC::IThreadTask"* nonnull %60) #8
  br label %67

; <label>:67:                                     ; preds = %62, %58
  store %"class.MARC::IThreadTask"* null, %"class.MARC::IThreadTask"** %59, align 8, !tbaa !27
  %68 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %21, align 8, !tbaa !321
  %69 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %68, i64 1
  store %"class.std::unique_ptr"* %69, %"class.std::unique_ptr"** %21, align 8, !tbaa !321
  br label %91

; <label>:70:                                     ; preds = %52
  %71 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2
  %72 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %53, i64 0, i32 0, i32 0, i32 0, i32 0
  %73 = load %"class.MARC::IThreadTask"*, %"class.MARC::IThreadTask"** %72, align 8, !tbaa !27
  %74 = icmp eq %"class.MARC::IThreadTask"* %73, null
  br i1 %74, label %80, label %75

; <label>:75:                                     ; preds = %70
  %76 = bitcast %"class.MARC::IThreadTask"* %73 to void (%"class.MARC::IThreadTask"*)***
  %77 = load void (%"class.MARC::IThreadTask"*)**, void (%"class.MARC::IThreadTask"*)*** %76, align 8, !tbaa !133
  %78 = getelementptr inbounds void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %77, i64 2
  %79 = load void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %78, align 8
  call void %79(%"class.MARC::IThreadTask"* nonnull %73) #8
  br label %80

; <label>:80:                                     ; preds = %75, %70
  store %"class.MARC::IThreadTask"* null, %"class.MARC::IThreadTask"** %72, align 8, !tbaa !27
  %81 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 1
  %82 = bitcast %"class.std::unique_ptr"** %81 to i8**
  %83 = load i8*, i8** %82, align 8, !tbaa !323
  call void @_ZdlPv(i8* %83) #8
  %84 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 3
  %85 = load %"class.std::unique_ptr"**, %"class.std::unique_ptr"*** %84, align 8, !tbaa !221
  %86 = getelementptr inbounds %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %85, i64 1
  store %"class.std::unique_ptr"** %86, %"class.std::unique_ptr"*** %84, align 8, !tbaa !202
  %87 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %86, align 8, !tbaa !27
  store %"class.std::unique_ptr"* %87, %"class.std::unique_ptr"** %81, align 8, !tbaa !203
  %88 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %87, i64 64
  store %"class.std::unique_ptr"* %88, %"class.std::unique_ptr"** %54, align 8, !tbaa !204
  %89 = ptrtoint %"class.std::unique_ptr"* %87 to i64
  %90 = bitcast %"struct.std::_Deque_iterator.41"* %71 to i64*
  store i64 %89, i64* %90, align 8, !tbaa !321
  br label %91

; <label>:91:                                     ; preds = %67, %80
  %92 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 4
  call void @_ZNSt18condition_variable10notify_oneEv(%"class.std::condition_variable"* %92) #8
  br label %93

; <label>:93:                                     ; preds = %34, %13, %91
  %94 = phi i1 [ false, %13 ], [ false, %34 ], [ true, %91 ]
  %95 = load i8, i8* %7, align 8, !tbaa !35, !range !43
  %96 = icmp eq i8 %95, 0
  br i1 %96, label %105, label %97

; <label>:97:                                     ; preds = %93
  %98 = load %"class.std::mutex"*, %"class.std::mutex"** %6, align 8, !tbaa !32
  %99 = icmp eq %"class.std::mutex"* %98, null
  br i1 %99, label %105, label %100

; <label>:100:                                    ; preds = %97
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %101, label %104

; <label>:101:                                    ; preds = %100
  %102 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %98, i64 0, i32 0, i32 0
  %103 = call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %102) #8
  br label %104

; <label>:104:                                    ; preds = %101, %100
  store i8 0, i8* %7, align 8, !tbaa !35
  br label %105

; <label>:105:                                    ; preds = %93, %97, %104
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %4) #8
  ret i1 %94
}

; Function Attrs: uwtable
define linkonce_odr void @_ZNSt11_Deque_baseISt8functionIFvvEESaIS2_EE17_M_initialize_mapEm(%"class.std::_Deque_base.48"*, i64) local_unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %3 = lshr i64 %1, 4
  %4 = add nuw nsw i64 %3, 1
  %5 = add nuw nsw i64 %3, 3
  %6 = icmp ugt i64 %5, 8
  %7 = select i1 %6, i64 %5, i64 8
  %8 = getelementptr inbounds %"class.std::_Deque_base.48", %"class.std::_Deque_base.48"* %0, i64 0, i32 0, i32 1
  store i64 %7, i64* %8, align 8, !tbaa !301
  %9 = icmp ugt i64 %7, 2305843009213693951
  br i1 %9, label %10, label %11

; <label>:10:                                     ; preds = %2
  tail call void @_ZSt17__throw_bad_allocv() #23
  unreachable

; <label>:11:                                     ; preds = %2
  %12 = shl nuw i64 %7, 3
  %13 = tail call i8* @_Znwm(i64 %12)
  %14 = bitcast i8* %13 to %"class.std::function"**
  %15 = bitcast %"class.std::_Deque_base.48"* %0 to i8**
  store i8* %13, i8** %15, align 8, !tbaa !228
  %16 = load i64, i64* %8, align 8, !tbaa !301
  %17 = sub i64 %16, %4
  %18 = lshr i64 %17, 1
  %19 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %14, i64 %18
  %20 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %19, i64 %4
  br label %21

; <label>:21:                                     ; preds = %24, %11
  %22 = phi %"class.std::function"** [ %19, %11 ], [ %26, %24 ]
  %23 = invoke i8* @_Znwm(i64 512)
          to label %24 unwind label %28

; <label>:24:                                     ; preds = %21
  %25 = bitcast %"class.std::function"** %22 to i8**
  store i8* %23, i8** %25, align 8, !tbaa !27
  %26 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %22, i64 1
  %27 = icmp ult %"class.std::function"** %26, %20
  br i1 %27, label %21, label %55

; <label>:28:                                     ; preds = %21
  %29 = landingpad { i8*, i32 }
          catch i8* null
  %30 = extractvalue { i8*, i32 } %29, 0
  %31 = tail call i8* @__cxa_begin_catch(i8* %30) #8
  %32 = icmp ugt %"class.std::function"** %22, %19
  br i1 %32, label %33, label %40

; <label>:33:                                     ; preds = %28
  br label %34

; <label>:34:                                     ; preds = %33, %34
  %35 = phi %"class.std::function"** [ %38, %34 ], [ %19, %33 ]
  %36 = bitcast %"class.std::function"** %35 to i8**
  %37 = load i8*, i8** %36, align 8, !tbaa !27
  tail call void @_ZdlPv(i8* %37) #8
  %38 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %35, i64 1
  %39 = icmp ult %"class.std::function"** %38, %22
  br i1 %39, label %34, label %40

; <label>:40:                                     ; preds = %34, %28
  invoke void @__cxa_rethrow() #23
          to label %46 unwind label %41

; <label>:41:                                     ; preds = %40
  %42 = landingpad { i8*, i32 }
          catch i8* null
  invoke void @__cxa_end_catch()
          to label %47 unwind label %43

; <label>:43:                                     ; preds = %41
  %44 = landingpad { i8*, i32 }
          catch i8* null
  %45 = extractvalue { i8*, i32 } %44, 0
  tail call void @__clang_call_terminate(i8* %45) #22
  unreachable

; <label>:46:                                     ; preds = %40
  unreachable

; <label>:47:                                     ; preds = %41
  %48 = extractvalue { i8*, i32 } %42, 0
  %49 = tail call i8* @__cxa_begin_catch(i8* %48) #8
  %50 = load i8*, i8** %15, align 8, !tbaa !228
  tail call void @_ZdlPv(i8* %50) #8
  %51 = bitcast %"class.std::_Deque_base.48"* %0 to i8*
  call void @llvm.memset.p0i8.i64(i8* %51, i8 0, i64 16, i32 8, i1 false)
  invoke void @__cxa_rethrow() #23
          to label %76 unwind label %52

; <label>:52:                                     ; preds = %47
  %53 = landingpad { i8*, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %54 unwind label %73

; <label>:54:                                     ; preds = %52
  resume { i8*, i32 } %53

; <label>:55:                                     ; preds = %24
  %56 = getelementptr inbounds %"class.std::_Deque_base.48", %"class.std::_Deque_base.48"* %0, i64 0, i32 0, i32 2
  %57 = getelementptr inbounds %"class.std::_Deque_base.48", %"class.std::_Deque_base.48"* %0, i64 0, i32 0, i32 2, i32 3
  store %"class.std::function"** %19, %"class.std::function"*** %57, align 8, !tbaa !207
  %58 = load %"class.std::function"*, %"class.std::function"** %19, align 8, !tbaa !27
  %59 = getelementptr inbounds %"class.std::_Deque_base.48", %"class.std::_Deque_base.48"* %0, i64 0, i32 0, i32 2, i32 1
  store %"class.std::function"* %58, %"class.std::function"** %59, align 8, !tbaa !210
  %60 = getelementptr inbounds %"class.std::function", %"class.std::function"* %58, i64 16
  %61 = getelementptr inbounds %"class.std::_Deque_base.48", %"class.std::_Deque_base.48"* %0, i64 0, i32 0, i32 2, i32 2
  store %"class.std::function"* %60, %"class.std::function"** %61, align 8, !tbaa !211
  %62 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %20, i64 -1
  %63 = getelementptr inbounds %"class.std::_Deque_base.48", %"class.std::_Deque_base.48"* %0, i64 0, i32 0, i32 3, i32 3
  store %"class.std::function"** %62, %"class.std::function"*** %63, align 8, !tbaa !207
  %64 = load %"class.std::function"*, %"class.std::function"** %62, align 8, !tbaa !27
  %65 = getelementptr inbounds %"class.std::_Deque_base.48", %"class.std::_Deque_base.48"* %0, i64 0, i32 0, i32 3, i32 1
  store %"class.std::function"* %64, %"class.std::function"** %65, align 8, !tbaa !210
  %66 = getelementptr inbounds %"class.std::function", %"class.std::function"* %64, i64 16
  %67 = getelementptr inbounds %"class.std::_Deque_base.48", %"class.std::_Deque_base.48"* %0, i64 0, i32 0, i32 3, i32 2
  store %"class.std::function"* %66, %"class.std::function"** %67, align 8, !tbaa !211
  %68 = ptrtoint %"class.std::function"* %58 to i64
  %69 = bitcast %"struct.std::_Deque_iterator.52"* %56 to i64*
  store i64 %68, i64* %69, align 8, !tbaa !237
  %70 = and i64 %1, 15
  %71 = getelementptr inbounds %"class.std::function", %"class.std::function"* %64, i64 %70
  %72 = getelementptr inbounds %"class.std::_Deque_base.48", %"class.std::_Deque_base.48"* %0, i64 0, i32 0, i32 3, i32 0
  store %"class.std::function"* %71, %"class.std::function"** %72, align 8, !tbaa !299
  ret void

; <label>:73:                                     ; preds = %52
  %74 = landingpad { i8*, i32 }
          catch i8* null
  %75 = extractvalue { i8*, i32 } %74, 0
  tail call void @__clang_call_terminate(i8* %75) #22
  unreachable

; <label>:76:                                     ; preds = %47
  unreachable
}

; Function Attrs: uwtable
define linkonce_odr void @_ZNSt11_Deque_baseISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EESaIS5_EE17_M_initialize_mapEm(%"class.std::_Deque_base.35"*, i64) local_unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %3 = lshr i64 %1, 6
  %4 = add nuw nsw i64 %3, 1
  %5 = add nuw nsw i64 %3, 3
  %6 = icmp ugt i64 %5, 8
  %7 = select i1 %6, i64 %5, i64 8
  %8 = getelementptr inbounds %"class.std::_Deque_base.35", %"class.std::_Deque_base.35"* %0, i64 0, i32 0, i32 1
  store i64 %7, i64* %8, align 8, !tbaa !199
  %9 = icmp ugt i64 %7, 2305843009213693951
  br i1 %9, label %10, label %11

; <label>:10:                                     ; preds = %2
  tail call void @_ZSt17__throw_bad_allocv() #23
  unreachable

; <label>:11:                                     ; preds = %2
  %12 = shl nuw nsw i64 %7, 3
  %13 = tail call i8* @_Znwm(i64 %12)
  %14 = bitcast i8* %13 to %"class.std::unique_ptr"**
  %15 = bitcast %"class.std::_Deque_base.35"* %0 to i8**
  store i8* %13, i8** %15, align 8, !tbaa !201
  %16 = load i64, i64* %8, align 8, !tbaa !199
  %17 = sub i64 %16, %4
  %18 = lshr i64 %17, 1
  %19 = getelementptr inbounds %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %14, i64 %18
  %20 = getelementptr inbounds %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %19, i64 %4
  br label %21

; <label>:21:                                     ; preds = %24, %11
  %22 = phi %"class.std::unique_ptr"** [ %19, %11 ], [ %26, %24 ]
  %23 = invoke i8* @_Znwm(i64 512)
          to label %24 unwind label %28

; <label>:24:                                     ; preds = %21
  %25 = bitcast %"class.std::unique_ptr"** %22 to i8**
  store i8* %23, i8** %25, align 8, !tbaa !27
  %26 = getelementptr inbounds %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %22, i64 1
  %27 = icmp ult %"class.std::unique_ptr"** %26, %20
  br i1 %27, label %21, label %55

; <label>:28:                                     ; preds = %21
  %29 = landingpad { i8*, i32 }
          catch i8* null
  %30 = extractvalue { i8*, i32 } %29, 0
  %31 = tail call i8* @__cxa_begin_catch(i8* %30) #8
  %32 = icmp ugt %"class.std::unique_ptr"** %22, %19
  br i1 %32, label %33, label %40

; <label>:33:                                     ; preds = %28
  br label %34

; <label>:34:                                     ; preds = %33, %34
  %35 = phi %"class.std::unique_ptr"** [ %38, %34 ], [ %19, %33 ]
  %36 = bitcast %"class.std::unique_ptr"** %35 to i8**
  %37 = load i8*, i8** %36, align 8, !tbaa !27
  tail call void @_ZdlPv(i8* %37) #8
  %38 = getelementptr inbounds %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %35, i64 1
  %39 = icmp ult %"class.std::unique_ptr"** %38, %22
  br i1 %39, label %34, label %40

; <label>:40:                                     ; preds = %34, %28
  invoke void @__cxa_rethrow() #23
          to label %46 unwind label %41

; <label>:41:                                     ; preds = %40
  %42 = landingpad { i8*, i32 }
          catch i8* null
  invoke void @__cxa_end_catch()
          to label %47 unwind label %43

; <label>:43:                                     ; preds = %41
  %44 = landingpad { i8*, i32 }
          catch i8* null
  %45 = extractvalue { i8*, i32 } %44, 0
  tail call void @__clang_call_terminate(i8* %45) #22
  unreachable

; <label>:46:                                     ; preds = %40
  unreachable

; <label>:47:                                     ; preds = %41
  %48 = extractvalue { i8*, i32 } %42, 0
  %49 = tail call i8* @__cxa_begin_catch(i8* %48) #8
  %50 = load i8*, i8** %15, align 8, !tbaa !201
  tail call void @_ZdlPv(i8* %50) #8
  %51 = bitcast %"class.std::_Deque_base.35"* %0 to i8*
  call void @llvm.memset.p0i8.i64(i8* %51, i8 0, i64 16, i32 8, i1 false)
  invoke void @__cxa_rethrow() #23
          to label %76 unwind label %52

; <label>:52:                                     ; preds = %47
  %53 = landingpad { i8*, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %54 unwind label %73

; <label>:54:                                     ; preds = %52
  resume { i8*, i32 } %53

; <label>:55:                                     ; preds = %24
  %56 = getelementptr inbounds %"class.std::_Deque_base.35", %"class.std::_Deque_base.35"* %0, i64 0, i32 0, i32 2
  %57 = getelementptr inbounds %"class.std::_Deque_base.35", %"class.std::_Deque_base.35"* %0, i64 0, i32 0, i32 2, i32 3
  store %"class.std::unique_ptr"** %19, %"class.std::unique_ptr"*** %57, align 8, !tbaa !202
  %58 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %19, align 8, !tbaa !27
  %59 = getelementptr inbounds %"class.std::_Deque_base.35", %"class.std::_Deque_base.35"* %0, i64 0, i32 0, i32 2, i32 1
  store %"class.std::unique_ptr"* %58, %"class.std::unique_ptr"** %59, align 8, !tbaa !203
  %60 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %58, i64 64
  %61 = getelementptr inbounds %"class.std::_Deque_base.35", %"class.std::_Deque_base.35"* %0, i64 0, i32 0, i32 2, i32 2
  store %"class.std::unique_ptr"* %60, %"class.std::unique_ptr"** %61, align 8, !tbaa !204
  %62 = getelementptr inbounds %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %20, i64 -1
  %63 = getelementptr inbounds %"class.std::_Deque_base.35", %"class.std::_Deque_base.35"* %0, i64 0, i32 0, i32 3, i32 3
  store %"class.std::unique_ptr"** %62, %"class.std::unique_ptr"*** %63, align 8, !tbaa !202
  %64 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %62, align 8, !tbaa !27
  %65 = getelementptr inbounds %"class.std::_Deque_base.35", %"class.std::_Deque_base.35"* %0, i64 0, i32 0, i32 3, i32 1
  store %"class.std::unique_ptr"* %64, %"class.std::unique_ptr"** %65, align 8, !tbaa !203
  %66 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %64, i64 64
  %67 = getelementptr inbounds %"class.std::_Deque_base.35", %"class.std::_Deque_base.35"* %0, i64 0, i32 0, i32 3, i32 2
  store %"class.std::unique_ptr"* %66, %"class.std::unique_ptr"** %67, align 8, !tbaa !204
  %68 = ptrtoint %"class.std::unique_ptr"* %58 to i64
  %69 = bitcast %"struct.std::_Deque_iterator.41"* %56 to i64*
  store i64 %68, i64* %69, align 8, !tbaa !321
  %70 = and i64 %1, 63
  %71 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %64, i64 %70
  %72 = getelementptr inbounds %"class.std::_Deque_base.35", %"class.std::_Deque_base.35"* %0, i64 0, i32 0, i32 3, i32 0
  store %"class.std::unique_ptr"* %71, %"class.std::unique_ptr"** %72, align 8, !tbaa !192
  ret void

; <label>:73:                                     ; preds = %52
  %74 = landingpad { i8*, i32 }
          catch i8* null
  %75 = extractvalue { i8*, i32 } %74, 0
  tail call void @__clang_call_terminate(i8* %75) #22
  unreachable

; <label>:76:                                     ; preds = %47
  unreachable
}

attributes #0 = { noinline uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noinline norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #6 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #7 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #8 = { nounwind }
attributes #9 = { argmemonly nounwind }
attributes #10 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #11 = { noreturn "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #12 = { nobuiltin "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #13 = { nobuiltin nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #14 = { inlinehint uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #15 = { noreturn nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #16 = { noinline noreturn nounwind }
attributes #17 = { inlinehint nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #18 = { nounwind readonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #19 = { inlinehint norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #20 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #21 = { nounwind readnone }
attributes #22 = { noreturn nounwind }
attributes #23 = { noreturn }
attributes #24 = { builtin }
attributes #25 = { builtin nounwind }
attributes #26 = { nounwind readonly }

!llvm.ident = !{!0, !0}
!llvm.module.flags = !{!1}

!0 = !{!"clang version 5.0.0 (tags/RELEASE_500/final)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !4, i64 0}
!3 = !{!"_ZTSNSs12_Alloc_hiderE", !4, i64 0}
!4 = !{!"any pointer", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!8}
!8 = distinct !{!8, !9, !"_ZStplIcSt11char_traitsIcESaIcEESbIT_T0_T1_EPKS3_RKS6_: argument 0"}
!9 = distinct !{!9, !"_ZStplIcSt11char_traitsIcESaIcEESbIT_T0_T1_EPKS3_RKS6_"}
!10 = !{!11, !4, i64 0}
!11 = !{!"_ZTSSs", !3, i64 0}
!12 = !{!13, !14, i64 0}
!13 = !{!"_ZTSNSs9_Rep_baseE", !14, i64 0, !14, i64 8, !15, i64 16}
!14 = !{!"long", !5, i64 0}
!15 = !{!"int", !5, i64 0}
!16 = !{!"branch_weights", i32 2000, i32 1}
!17 = !{!15, !15, i64 0}
!18 = !{!5, !5, i64 0}
!19 = !{!20, !4, i64 48}
!20 = !{!"_ZTSSt11_Deque_baseIaSaIaEE", !21, i64 0}
!21 = !{!"_ZTSNSt11_Deque_baseIaSaIaEE11_Deque_implE", !4, i64 0, !14, i64 8, !22, i64 16, !22, i64 48}
!22 = !{!"_ZTSSt15_Deque_iteratorIaRaPaE", !4, i64 0, !4, i64 8, !4, i64 16, !4, i64 24}
!23 = !{!20, !4, i64 64}
!24 = !{!20, !14, i64 8}
!25 = !{!20, !4, i64 72}
!26 = !{!20, !4, i64 0}
!27 = !{!4, !4, i64 0}
!28 = !{!22, !4, i64 24}
!29 = !{!22, !4, i64 8}
!30 = !{!22, !4, i64 16}
!31 = !{!20, !4, i64 40}
!32 = !{!33, !4, i64 0}
!33 = !{!"_ZTSSt11unique_lockISt5mutexE", !4, i64 0, !34, i64 8}
!34 = !{!"bool", !5, i64 0}
!35 = !{!33, !34, i64 8}
!36 = !{!22, !4, i64 0}
!37 = !{!38}
!38 = distinct !{!38, !39, !"_ZNSt5dequeIaSaIaEE5beginEv: argument 0"}
!39 = distinct !{!39, !"_ZNSt5dequeIaSaIaEE5beginEv"}
!40 = !{!20, !4, i64 16}
!41 = !{!20, !4, i64 32}
!42 = !{!20, !4, i64 24}
!43 = !{i8 0, i8 2}
!44 = !{!45, !45, i64 0}
!45 = !{!"short", !5, i64 0}
!46 = !{!47, !4, i64 48}
!47 = !{!"_ZTSSt11_Deque_baseIsSaIsEE", !48, i64 0}
!48 = !{!"_ZTSNSt11_Deque_baseIsSaIsEE11_Deque_implE", !4, i64 0, !14, i64 8, !49, i64 16, !49, i64 48}
!49 = !{!"_ZTSSt15_Deque_iteratorIsRsPsE", !4, i64 0, !4, i64 8, !4, i64 16, !4, i64 24}
!50 = !{!47, !4, i64 64}
!51 = !{!47, !14, i64 8}
!52 = !{!47, !4, i64 72}
!53 = !{!47, !4, i64 0}
!54 = !{!49, !4, i64 24}
!55 = !{!49, !4, i64 8}
!56 = !{!49, !4, i64 16}
!57 = !{!47, !4, i64 40}
!58 = !{!49, !4, i64 0}
!59 = !{!60}
!60 = distinct !{!60, !61, !"_ZNSt5dequeIsSaIsEE5beginEv: argument 0"}
!61 = distinct !{!61, !"_ZNSt5dequeIsSaIsEE5beginEv"}
!62 = !{!47, !4, i64 32}
!63 = !{!47, !4, i64 16}
!64 = !{!47, !4, i64 24}
!65 = !{!66, !4, i64 48}
!66 = !{!"_ZTSSt11_Deque_baseIiSaIiEE", !67, i64 0}
!67 = !{!"_ZTSNSt11_Deque_baseIiSaIiEE11_Deque_implE", !4, i64 0, !14, i64 8, !68, i64 16, !68, i64 48}
!68 = !{!"_ZTSSt15_Deque_iteratorIiRiPiE", !4, i64 0, !4, i64 8, !4, i64 16, !4, i64 24}
!69 = !{!66, !4, i64 64}
!70 = !{!66, !14, i64 8}
!71 = !{!66, !4, i64 72}
!72 = !{!66, !4, i64 0}
!73 = !{!68, !4, i64 24}
!74 = !{!68, !4, i64 8}
!75 = !{!68, !4, i64 16}
!76 = !{!66, !4, i64 40}
!77 = !{!68, !4, i64 0}
!78 = !{!79}
!79 = distinct !{!79, !80, !"_ZNSt5dequeIiSaIiEE5beginEv: argument 0"}
!80 = distinct !{!80, !"_ZNSt5dequeIiSaIiEE5beginEv"}
!81 = !{!66, !4, i64 32}
!82 = !{!66, !4, i64 16}
!83 = !{!66, !4, i64 24}
!84 = !{!14, !14, i64 0}
!85 = !{!86, !4, i64 48}
!86 = !{!"_ZTSSt11_Deque_baseIlSaIlEE", !87, i64 0}
!87 = !{!"_ZTSNSt11_Deque_baseIlSaIlEE11_Deque_implE", !4, i64 0, !14, i64 8, !88, i64 16, !88, i64 48}
!88 = !{!"_ZTSSt15_Deque_iteratorIlRlPlE", !4, i64 0, !4, i64 8, !4, i64 16, !4, i64 24}
!89 = !{!86, !4, i64 64}
!90 = !{!86, !14, i64 8}
!91 = !{!86, !4, i64 72}
!92 = !{!86, !4, i64 0}
!93 = !{!88, !4, i64 24}
!94 = !{!88, !4, i64 8}
!95 = !{!88, !4, i64 16}
!96 = !{!86, !4, i64 40}
!97 = !{!88, !4, i64 0}
!98 = !{!99}
!99 = distinct !{!99, !100, !"_ZNSt5dequeIlSaIlEE5beginEv: argument 0"}
!100 = distinct !{!100, !"_ZNSt5dequeIlSaIlEE5beginEv"}
!101 = !{!86, !4, i64 32}
!102 = !{!86, !4, i64 16}
!103 = !{!86, !4, i64 24}
!104 = !{!105, !4, i64 48}
!105 = !{!"_ZTSSt11_Deque_baseIcSaIcEE", !106, i64 0}
!106 = !{!"_ZTSNSt11_Deque_baseIcSaIcEE11_Deque_implE", !4, i64 0, !14, i64 8, !107, i64 16, !107, i64 48}
!107 = !{!"_ZTSSt15_Deque_iteratorIcRcPcE", !4, i64 0, !4, i64 8, !4, i64 16, !4, i64 24}
!108 = !{!105, !4, i64 64}
!109 = !{!105, !14, i64 8}
!110 = !{!105, !4, i64 72}
!111 = !{!105, !4, i64 0}
!112 = !{!107, !4, i64 24}
!113 = !{!107, !4, i64 8}
!114 = !{!107, !4, i64 16}
!115 = !{!105, !4, i64 40}
!116 = !{!107, !4, i64 0}
!117 = !{!118}
!118 = distinct !{!118, !119, !"_ZNSt5dequeIcSaIcEE5beginEv: argument 0"}
!119 = distinct !{!119, !"_ZNSt5dequeIcSaIcEE5beginEv"}
!120 = !{!105, !4, i64 16}
!121 = !{!105, !4, i64 32}
!122 = !{!105, !4, i64 24}
!123 = !{!124, !4, i64 16}
!124 = !{!"_ZTSSt14_Function_base", !5, i64 0, !4, i64 16}
!125 = !{!126, !4, i64 8}
!126 = !{!"_ZTSSt12_Vector_baseIN4MARC10TaskFutureIvEESaIS2_EE", !127, i64 0}
!127 = !{!"_ZTSNSt12_Vector_baseIN4MARC10TaskFutureIvEESaIS2_EE12_Vector_implE", !4, i64 0, !4, i64 8, !4, i64 16}
!128 = !{!126, !4, i64 16}
!129 = !{!130, !4, i64 0}
!130 = !{!"_ZTSSt14__shared_countILN9__gnu_cxx12_Lock_policyE2EE", !4, i64 0}
!131 = !{!132, !4, i64 0}
!132 = !{!"_ZTSSt12__shared_ptrINSt13__future_base13_State_baseV2ELN9__gnu_cxx12_Lock_policyE2EE", !4, i64 0, !130, i64 8}
!133 = !{!134, !134, i64 0}
!134 = !{!"vtable pointer", !6, i64 0}
!135 = !{!136, !34, i64 0}
!136 = !{!"_ZTSSt13__atomic_baseIbE", !34, i64 0}
!137 = !{!138, !4, i64 256}
!138 = !{!"_ZTSN4MARC10ThreadPoolE", !139, i64 0, !140, i64 8, !145, i64 232, !4, i64 256, !146, i64 264}
!139 = !{!"_ZTSSt6atomicIbE", !136, i64 0}
!140 = !{!"_ZTSN4MARC15ThreadSafeQueueISt10unique_ptrINS_11IThreadTaskESt14default_deleteIS2_EEEE", !139, i64 0, !141, i64 8, !142, i64 48, !144, i64 128, !144, i64 176}
!141 = !{!"_ZTSSt5mutex"}
!142 = !{!"_ZTSSt5queueISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EESt5dequeIS5_SaIS5_EEE", !143, i64 0}
!143 = !{!"_ZTSSt5dequeISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EESaIS5_EE"}
!144 = !{!"_ZTSSt18condition_variable", !5, i64 0}
!145 = !{!"_ZTSSt6vectorISt6threadSaIS0_EE"}
!146 = !{!"_ZTSN4MARC15ThreadSafeQueueISt8functionIFvvEEEE", !139, i64 0, !141, i64 8, !147, i64 48, !144, i64 128, !144, i64 176}
!147 = !{!"_ZTSSt5queueISt8functionIFvvEESt5dequeIS2_SaIS2_EEE", !148, i64 0}
!148 = !{!"_ZTSSt5dequeISt8functionIFvvEESaIS2_EE"}
!149 = distinct !{!149, !150}
!150 = !{!"llvm.loop.unroll.disable"}
!151 = !{!152, !4, i64 0}
!152 = !{!"_ZTSSt12_Vector_baseISt6threadSaIS0_EE", !153, i64 0}
!153 = !{!"_ZTSNSt12_Vector_baseISt6threadSaIS0_EE12_Vector_implE", !4, i64 0, !4, i64 8, !4, i64 16}
!154 = !{!152, !4, i64 8}
!155 = !{i64 0, i64 8, !27, i64 8, i64 8, !27, i64 16, i64 8, !27, i64 24, i64 8, !27}
!156 = !{!157}
!157 = distinct !{!157, !158, !"_ZSt4bindIRPFvPvS0_EJRS0_S0_EENSt12_Bind_helperIXsr15__is_socketlikeIT_EE5valueES6_JDpT0_EE4typeEOS6_DpOS7_: argument 0"}
!158 = distinct !{!158, !"_ZSt4bindIRPFvPvS0_EJRS0_S0_EENSt12_Bind_helperIXsr15__is_socketlikeIT_EE5valueES6_JDpT0_EE4typeEOS6_DpOS7_"}
!159 = !{!160}
!160 = distinct !{!160, !161, !"_ZStL19__create_task_stateIFvvESt5_BindIFPFvPvS2_ES2_S2_EESaIiEESt10shared_ptrINSt13__future_base16_Task_state_baseIT_EEEOT0_RKT1_: argument 0"}
!161 = distinct !{!161, !"_ZStL19__create_task_stateIFvvESt5_BindIFPFvPvS2_ES2_S2_EESaIiEESt10shared_ptrINSt13__future_base16_Task_state_baseIT_EEEOT0_RKT1_"}
!162 = !{!163}
!163 = distinct !{!163, !164, !"_ZSt15allocate_sharedINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_JS7_RKS8_EESt10shared_ptrIT_ERKT0_DpOT1_: argument 0"}
!164 = distinct !{!164, !"_ZSt15allocate_sharedINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_JS7_RKS8_EESt10shared_ptrIT_ERKT0_DpOT1_"}
!165 = !{!166, !15, i64 8}
!166 = !{!"_ZTSSt16_Sp_counted_baseILN9__gnu_cxx12_Lock_policyE2EE", !15, i64 8, !15, i64 12}
!167 = !{!166, !15, i64 12}
!168 = !{!169, !4, i64 0}
!169 = !{!"_ZTSSt10_Head_baseILm0EPNSt13__future_base12_Result_baseELb0EE", !4, i64 0}
!170 = !{!171, !15, i64 0}
!171 = !{!"_ZTSSt13__atomic_baseIjE", !15, i64 0}
!172 = !{!173, !34, i64 0}
!173 = !{!"_ZTSSt18__atomic_flag_base", !34, i64 0}
!174 = !{!175, !15, i64 0}
!175 = !{!"_ZTSSt9once_flag", !15, i64 0}
!176 = !{!177, !163}
!177 = distinct !{!177, !178, !"_ZNSt13__future_base18_S_allocate_resultIviEESt10unique_ptrINS_7_ResultIT_EENS_12_Result_base8_DeleterEERKSaIT0_E: argument 0"}
!178 = distinct !{!178, !"_ZNSt13__future_base18_S_allocate_resultIviEESt10unique_ptrINS_7_ResultIT_EENS_12_Result_base8_DeleterEERKSaIT0_E"}
!179 = !{!177}
!180 = !{!181, !4, i64 0}
!181 = !{!"_ZTSSt5_BindIFPFvPvS0_ES0_S0_EE", !4, i64 0, !182, i64 8}
!182 = !{!"_ZTSSt5tupleIJPvS0_EE"}
!183 = !{!184, !4, i64 0}
!184 = !{!"_ZTSSt10_Head_baseILm1EPvLb0EE", !4, i64 0}
!185 = !{!186, !4, i64 0}
!186 = !{!"_ZTSSt10_Head_baseILm0EPvLb0EE", !4, i64 0}
!187 = !{!188, !4, i64 0}
!188 = !{!"_ZTSSt12__shared_ptrINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEELN9__gnu_cxx12_Lock_policyE2EE", !4, i64 0, !130, i64 8}
!189 = !{!190}
!190 = distinct !{!190, !191, !"_ZSt11make_uniqueIN4MARC10ThreadTaskISt13packaged_taskIFvvEEEEJS4_EENSt9_MakeUniqIT_E15__single_objectEDpOT0_: argument 0"}
!191 = distinct !{!191, !"_ZSt11make_uniqueIN4MARC10ThreadTaskISt13packaged_taskIFvvEEEEJS4_EENSt9_MakeUniqIT_E15__single_objectEDpOT0_"}
!192 = !{!193, !4, i64 48}
!193 = !{!"_ZTSSt11_Deque_baseISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EESaIS5_EE", !194, i64 0}
!194 = !{!"_ZTSNSt11_Deque_baseISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EESaIS5_EE11_Deque_implE", !4, i64 0, !14, i64 8, !195, i64 16, !195, i64 48}
!195 = !{!"_ZTSSt15_Deque_iteratorISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EERS5_PS5_E", !4, i64 0, !4, i64 8, !4, i64 16, !4, i64 24}
!196 = !{!193, !4, i64 64}
!197 = !{!198, !4, i64 0}
!198 = !{!"_ZTSSt10_Head_baseILm0EPN4MARC11IThreadTaskELb0EE", !4, i64 0}
!199 = !{!193, !14, i64 8}
!200 = !{!193, !4, i64 72}
!201 = !{!193, !4, i64 0}
!202 = !{!195, !4, i64 24}
!203 = !{!195, !4, i64 8}
!204 = !{!195, !4, i64 16}
!205 = !{!126, !4, i64 0}
!206 = distinct !{!206, !150}
!207 = !{!208, !4, i64 24}
!208 = !{!"_ZTSSt15_Deque_iteratorISt8functionIFvvEERS2_PS2_E", !4, i64 0, !4, i64 8, !4, i64 16, !4, i64 24}
!209 = !{!208, !4, i64 0}
!210 = !{!208, !4, i64 8}
!211 = !{!208, !4, i64 16}
!212 = !{!213, !4, i64 24}
!213 = !{!"_ZTSSt8functionIFvvEE", !4, i64 24}
!214 = !{!195, !4, i64 0}
!215 = !{!216}
!216 = distinct !{!216, !217, !"_ZNSt5dequeISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EESaIS5_EE5beginEv: argument 0"}
!217 = distinct !{!217, !"_ZNSt5dequeISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EESaIS5_EE5beginEv"}
!218 = !{!219}
!219 = distinct !{!219, !220, !"_ZNSt5dequeISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EESaIS5_EE3endEv: argument 0"}
!220 = distinct !{!220, !"_ZNSt5dequeISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EESaIS5_EE3endEv"}
!221 = !{!193, !4, i64 40}
!222 = !{!223}
!223 = distinct !{!223, !224, !"_ZNSt5dequeISt8functionIFvvEESaIS2_EE5beginEv: argument 0"}
!224 = distinct !{!224, !"_ZNSt5dequeISt8functionIFvvEESaIS2_EE5beginEv"}
!225 = !{!226}
!226 = distinct !{!226, !227, !"_ZNSt5dequeISt8functionIFvvEESaIS2_EE3endEv: argument 0"}
!227 = distinct !{!227, !"_ZNSt5dequeISt8functionIFvvEESaIS2_EE3endEv"}
!228 = !{!229, !4, i64 0}
!229 = !{!"_ZTSSt11_Deque_baseISt8functionIFvvEESaIS2_EE", !230, i64 0}
!230 = !{!"_ZTSNSt11_Deque_baseISt8functionIFvvEESaIS2_EE11_Deque_implE", !4, i64 0, !14, i64 8, !208, i64 16, !208, i64 48}
!231 = !{!229, !4, i64 40}
!232 = !{!229, !4, i64 72}
!233 = !{!234}
!234 = distinct !{!234, !235, !"_ZNSt5dequeISt8functionIFvvEESaIS2_EE5beginEv: argument 0"}
!235 = distinct !{!235, !"_ZNSt5dequeISt8functionIFvvEESaIS2_EE5beginEv"}
!236 = !{i64 0, i64 8, !18, i64 0, i64 8, !18, i64 0, i64 8, !18, i64 0, i64 16, !18, i64 0, i64 16, !18}
!237 = !{!229, !4, i64 16}
!238 = !{!229, !4, i64 32}
!239 = !{!229, !4, i64 24}
!240 = !{!241, !4, i64 0}
!241 = !{!"_ZTSNSt15__exception_ptr13exception_ptrE", !4, i64 0}
!242 = !{!243, !4, i64 0}
!243 = !{!"_ZTSSt12__shared_ptrINSt13__future_base16_Task_state_baseIFvvEEELN9__gnu_cxx12_Lock_policyE2EE", !4, i64 0, !130, i64 8}
!244 = !{!245}
!245 = distinct !{!245, !246, !"_ZNKSt10error_code7messageEv: argument 0"}
!246 = distinct !{!246, !"_ZNKSt10error_code7messageEv"}
!247 = !{!248}
!248 = distinct !{!248, !249, !"_ZStplIcSt11char_traitsIcESaIcEESbIT_T0_T1_EPKS3_OS6_: argument 0"}
!249 = distinct !{!249, !"_ZStplIcSt11char_traitsIcESaIcEESbIT_T0_T1_EPKS3_OS6_"}
!250 = !{i64 0, i64 4, !17, i64 8, i64 8, !27}
!251 = !{!252}
!252 = distinct !{!252, !253, !"_ZSt13__bind_simpleISt17reference_wrapperISt5_BindIFPFvPvS2_ES2_S2_EEEJEENSt19_Bind_simple_helperIT_JDpT0_EE6__typeEOS9_DpOSA_: argument 0"}
!253 = distinct !{!253, !"_ZSt13__bind_simpleISt17reference_wrapperISt5_BindIFPFvPvS2_ES2_S2_EEEJEENSt19_Bind_simple_helperIT_JDpT0_EE6__typeEOS9_DpOSA_"}
!254 = !{!255, !4, i64 24}
!255 = !{!"_ZTSSt8functionIFSt10unique_ptrINSt13__future_base12_Result_baseENS2_8_DeleterEEvEE", !4, i64 24}
!256 = !{!34, !34, i64 0}
!257 = !{!258}
!258 = distinct !{!258, !259, !"_ZSt13__bind_simpleISt17reference_wrapperISt5_BindIFPFvPvS2_ES2_S2_EEEJEENSt19_Bind_simple_helperIT_JDpT0_EE6__typeEOS9_DpOSA_: argument 0"}
!259 = distinct !{!259, !"_ZSt13__bind_simpleISt17reference_wrapperISt5_BindIFPFvPvS2_ES2_S2_EEEJEENSt19_Bind_simple_helperIT_JDpT0_EE6__typeEOS9_DpOSA_"}
!260 = !{!261, !4, i64 0}
!261 = !{!"_ZTSSt12__weak_countILN9__gnu_cxx12_Lock_policyE2EE", !4, i64 0}
!262 = !{!263}
!263 = distinct !{!263, !264, !"_ZStL19__create_task_stateIFvvESt5_BindIFPFvPvS2_ES2_S2_EESaIiEESt10shared_ptrINSt13__future_base16_Task_state_baseIT_EEEOT0_RKT1_: argument 0"}
!264 = distinct !{!264, !"_ZStL19__create_task_stateIFvvESt5_BindIFPFvPvS2_ES2_S2_EESaIiEESt10shared_ptrINSt13__future_base16_Task_state_baseIT_EEEOT0_RKT1_"}
!265 = !{!266}
!266 = distinct !{!266, !267, !"_ZSt15allocate_sharedINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_JS7_RKS8_EESt10shared_ptrIT_ERKT0_DpOT1_: argument 0"}
!267 = distinct !{!267, !"_ZSt15allocate_sharedINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_JS7_RKS8_EESt10shared_ptrIT_ERKT0_DpOT1_"}
!268 = !{!269}
!269 = distinct !{!269, !270, !"_ZNSt13__future_base18_S_allocate_resultIviEESt10unique_ptrINS_7_ResultIT_EENS_12_Result_base8_DeleterEERKSaIT0_E: argument 0"}
!270 = distinct !{!270, !"_ZNSt13__future_base18_S_allocate_resultIviEESt10unique_ptrINS_7_ResultIT_EENS_12_Result_base8_DeleterEERKSaIT0_E"}
!271 = !{i64 0, i64 8, !27, i64 8, i64 8, !27}
!272 = !{!273, !4, i64 0}
!273 = !{!"_ZTSSt10__weak_ptrINSt13__future_base13_State_baseV2ELN9__gnu_cxx12_Lock_policyE2EE", !4, i64 0, !261, i64 8}
!274 = !{!275}
!275 = distinct !{!275, !276, !"_ZNKSt8functionIFSt10unique_ptrINSt13__future_base12_Result_baseENS2_8_DeleterEEvEEclEv: argument 0"}
!276 = distinct !{!276, !"_ZNKSt8functionIFSt10unique_ptrINSt13__future_base12_Result_baseENS2_8_DeleterEEvEEclEv"}
!277 = !{!278, !4, i64 8}
!278 = !{!"_ZTSNSt13__future_base12_Task_setterISt10unique_ptrINS_7_ResultIvEENS_12_Result_base8_DeleterEESt12_Bind_simpleIFSt17reference_wrapperISt5_BindIFPFvPvSA_ESA_SA_EEEvEEvEE", !4, i64 0, !4, i64 8}
!279 = !{!280, !4, i64 0}
!280 = !{!"_ZTSSt17reference_wrapperISt5_BindIFPFvPvS1_ES1_S1_EEE", !4, i64 0}
!281 = !{!278, !4, i64 0}
!282 = !{!283, !4, i64 0}
!283 = !{!"_ZTSSt10_Head_baseILm0EPNSt13__future_base7_ResultIvEELb0EE", !4, i64 0}
!284 = !{!285, !4, i64 8}
!285 = !{!"_ZTSSt9type_info", !4, i64 8}
!286 = !{!152, !4, i64 16}
!287 = !{!288, !14, i64 0}
!288 = !{!"_ZTSNSt6thread2idE", !14, i64 0}
!289 = !{!290}
!290 = distinct !{!290, !291, !"_ZSt13__bind_simpleIMN4MARC10ThreadPoolEFvPSt6atomicIbEEJPS1_S4_EENSt19_Bind_simple_helperIT_JDpT0_EE6__typeEOS9_DpOSA_: argument 0"}
!291 = distinct !{!291, !"_ZSt13__bind_simpleIMN4MARC10ThreadPoolEFvPSt6atomicIbEEJPS1_S4_EENSt19_Bind_simple_helperIT_JDpT0_EE6__typeEOS9_DpOSA_"}
!292 = !{!293}
!293 = distinct !{!293, !294, !"_ZNSt6thread13_S_make_stateISt12_Bind_simpleIFSt7_Mem_fnIMN4MARC10ThreadPoolEFvPSt6atomicIbEEEPS4_S7_EEEESt10unique_ptrINS_6_StateESt14default_deleteISF_EEOT_: argument 0"}
!294 = distinct !{!294, !"_ZNSt6thread13_S_make_stateISt12_Bind_simpleIFSt7_Mem_fnIMN4MARC10ThreadPoolEFvPSt6atomicIbEEEPS4_S7_EEEESt10unique_ptrINS_6_StateESt14default_deleteISF_EEOT_"}
!295 = !{!296, !4, i64 0}
!296 = !{!"_ZTSSt10_Head_baseILm2EPSt6atomicIbELb0EE", !4, i64 0}
!297 = !{!298, !4, i64 0}
!298 = !{!"_ZTSSt10_Head_baseILm1EPN4MARC10ThreadPoolELb0EE", !4, i64 0}
!299 = !{!229, !4, i64 48}
!300 = !{!229, !4, i64 64}
!301 = !{!229, !14, i64 8}
!302 = !{!303}
!303 = distinct !{!303, !304, !"_ZSt13__bind_simpleIMN4MARC10ThreadPoolEFvPSt6atomicIbEEJPS1_S4_EENSt19_Bind_simple_helperIT_JDpT0_EE6__typeEOS9_DpOSA_: argument 0"}
!304 = distinct !{!304, !"_ZSt13__bind_simpleIMN4MARC10ThreadPoolEFvPSt6atomicIbEEJPS1_S4_EENSt19_Bind_simple_helperIT_JDpT0_EE6__typeEOS9_DpOSA_"}
!305 = !{!306}
!306 = distinct !{!306, !307, !"_ZNSt6thread13_S_make_stateISt12_Bind_simpleIFSt7_Mem_fnIMN4MARC10ThreadPoolEFvPSt6atomicIbEEEPS4_S7_EEEESt10unique_ptrINS_6_StateESt14default_deleteISF_EEOT_: argument 0"}
!307 = distinct !{!307, !"_ZNSt6thread13_S_make_stateISt12_Bind_simpleIFSt7_Mem_fnIMN4MARC10ThreadPoolEFvPSt6atomicIbEEEPS4_S7_EEEESt10unique_ptrINS_6_StateESt14default_deleteISF_EEOT_"}
!308 = !{!309}
!309 = distinct !{!309, !310}
!310 = distinct !{!310, !"LVerDomain"}
!311 = !{!312}
!312 = distinct !{!312, !310}
!313 = distinct !{!313, !314, !315}
!314 = !{!"llvm.loop.vectorize.width", i32 1}
!315 = !{!"llvm.loop.interleave.count", i32 1}
!316 = distinct !{!316, !150}
!317 = distinct !{!317, !314, !315}
!318 = !{!319}
!319 = distinct !{!319, !320, !"_ZNSt5dequeISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EESaIS5_EE5beginEv: argument 0"}
!320 = distinct !{!320, !"_ZNSt5dequeISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EESaIS5_EE5beginEv"}
!321 = !{!193, !4, i64 16}
!322 = !{!193, !4, i64 32}
!323 = !{!193, !4, i64 24}
