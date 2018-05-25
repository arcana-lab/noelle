; ModuleID = 'test_dswp.bc'
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
@0 = internal unnamed_addr global i1 false

; Function Attrs: noinline uwtable
define float @_Z16heavyComputationff(float, float) local_unnamed_addr #0 {
  %3 = alloca [4 x i8*], align 8
  %4 = alloca float, align 4
  %5 = bitcast [4 x i8*]* %3 to float**
  store float* %4, float** %5, align 8
  %6 = alloca float, align 4
  %7 = getelementptr inbounds [4 x i8*], [4 x i8*]* %3, i64 0, i64 1
  %8 = bitcast i8** %7 to float**
  store float* %6, float** %8, align 8
  %9 = alloca float, align 4
  %10 = getelementptr inbounds [4 x i8*], [4 x i8*]* %3, i64 0, i64 2
  %11 = bitcast i8** %10 to float**
  store float* %9, float** %11, align 8
  %12 = alloca i32, align 4
  %13 = getelementptr inbounds [4 x i8*], [4 x i8*]* %3, i64 0, i64 3
  %14 = bitcast i8** %13 to i32**
  store i32* %12, i32** %14, align 8
  %15 = alloca [11 x i8*], align 16
  %16 = bitcast [11 x i8*]* %15 to <2 x void (i8*, i8*)*>*
  store <2 x void (i8*, i8*)*> <void (i8*, i8*)* @1, void (i8*, i8*)* @2>, <2 x void (i8*, i8*)*>* %16, align 16
  %17 = getelementptr inbounds [11 x i8*], [11 x i8*]* %15, i64 0, i64 2
  %18 = bitcast i8** %17 to <2 x void (i8*, i8*)*>*
  store <2 x void (i8*, i8*)*> <void (i8*, i8*)* @3, void (i8*, i8*)* @4>, <2 x void (i8*, i8*)*>* %18, align 16
  %19 = getelementptr inbounds [11 x i8*], [11 x i8*]* %15, i64 0, i64 4
  %20 = bitcast i8** %19 to <2 x void (i8*, i8*)*>*
  store <2 x void (i8*, i8*)*> <void (i8*, i8*)* @5, void (i8*, i8*)* @6>, <2 x void (i8*, i8*)*>* %20, align 16
  %21 = getelementptr inbounds [11 x i8*], [11 x i8*]* %15, i64 0, i64 6
  %22 = bitcast i8** %21 to <2 x void (i8*, i8*)*>*
  store <2 x void (i8*, i8*)*> <void (i8*, i8*)* @7, void (i8*, i8*)* @8>, <2 x void (i8*, i8*)*>* %22, align 16
  %23 = getelementptr inbounds [11 x i8*], [11 x i8*]* %15, i64 0, i64 8
  %24 = bitcast i8** %23 to <2 x void (i8*, i8*)*>*
  store <2 x void (i8*, i8*)*> <void (i8*, i8*)* @9, void (i8*, i8*)* @10>, <2 x void (i8*, i8*)*>* %24, align 16
  %25 = getelementptr inbounds [11 x i8*], [11 x i8*]* %15, i64 0, i64 10
  %26 = bitcast i8** %25 to void (i8*, i8*)**
  store void (i8*, i8*)* @11, void (i8*, i8*)** %26, align 16
  %27 = alloca [65 x i64], align 16
  %28 = bitcast [65 x i64]* %27 to <2 x i64>*
  store <2 x i64> <i64 32, i64 1>, <2 x i64>* %28, align 16
  %29 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 2
  %30 = bitcast i64* %29 to <2 x i64>*
  store <2 x i64> <i64 1, i64 1>, <2 x i64>* %30, align 16
  %31 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 4
  %32 = bitcast i64* %31 to <2 x i64>*
  store <2 x i64> <i64 32, i64 32>, <2 x i64>* %32, align 16
  %33 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 6
  %34 = bitcast i64* %33 to <2 x i64>*
  store <2 x i64> <i64 32, i64 1>, <2 x i64>* %34, align 16
  %35 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 8
  %36 = bitcast i64* %35 to <2 x i64>*
  store <2 x i64> <i64 1, i64 1>, <2 x i64>* %36, align 16
  %37 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 10
  %38 = bitcast i64* %37 to <2 x i64>*
  store <2 x i64> <i64 1, i64 1>, <2 x i64>* %38, align 16
  %39 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 12
  %40 = bitcast i64* %39 to <2 x i64>*
  store <2 x i64> <i64 1, i64 1>, <2 x i64>* %40, align 16
  %41 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 14
  %42 = bitcast i64* %41 to <2 x i64>*
  store <2 x i64> <i64 1, i64 1>, <2 x i64>* %42, align 16
  %43 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 16
  %44 = bitcast i64* %43 to <2 x i64>*
  store <2 x i64> <i64 1, i64 1>, <2 x i64>* %44, align 16
  %45 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 18
  %46 = bitcast i64* %45 to <2 x i64>*
  store <2 x i64> <i64 1, i64 1>, <2 x i64>* %46, align 16
  %47 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 20
  %48 = bitcast i64* %47 to <2 x i64>*
  store <2 x i64> <i64 1, i64 1>, <2 x i64>* %48, align 16
  %49 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 22
  %50 = bitcast i64* %49 to <2 x i64>*
  store <2 x i64> <i64 1, i64 1>, <2 x i64>* %50, align 16
  %51 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 24
  %52 = bitcast i64* %51 to <2 x i64>*
  store <2 x i64> <i64 1, i64 1>, <2 x i64>* %52, align 16
  %53 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 26
  %54 = bitcast i64* %53 to <2 x i64>*
  store <2 x i64> <i64 1, i64 1>, <2 x i64>* %54, align 16
  %55 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 28
  %56 = bitcast i64* %55 to <2 x i64>*
  store <2 x i64> <i64 1, i64 1>, <2 x i64>* %56, align 16
  %57 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 30
  %58 = bitcast i64* %57 to <2 x i64>*
  store <2 x i64> <i64 1, i64 1>, <2 x i64>* %58, align 16
  %59 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 32
  %60 = bitcast i64* %59 to <2 x i64>*
  store <2 x i64> <i64 1, i64 1>, <2 x i64>* %60, align 16
  %61 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 34
  %62 = bitcast i64* %61 to <2 x i64>*
  store <2 x i64> <i64 1, i64 1>, <2 x i64>* %62, align 16
  %63 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 36
  %64 = bitcast i64* %63 to <2 x i64>*
  store <2 x i64> <i64 1, i64 1>, <2 x i64>* %64, align 16
  %65 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 38
  %66 = bitcast i64* %65 to <2 x i64>*
  store <2 x i64> <i64 1, i64 1>, <2 x i64>* %66, align 16
  %67 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 40
  %68 = bitcast i64* %67 to <2 x i64>*
  store <2 x i64> <i64 1, i64 1>, <2 x i64>* %68, align 16
  %69 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 42
  %70 = bitcast i64* %69 to <2 x i64>*
  store <2 x i64> <i64 1, i64 1>, <2 x i64>* %70, align 16
  %71 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 44
  %72 = bitcast i64* %71 to <2 x i64>*
  store <2 x i64> <i64 1, i64 1>, <2 x i64>* %72, align 16
  %73 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 46
  %74 = bitcast i64* %73 to <2 x i64>*
  store <2 x i64> <i64 1, i64 1>, <2 x i64>* %74, align 16
  %75 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 48
  %76 = bitcast i64* %75 to <2 x i64>*
  store <2 x i64> <i64 1, i64 1>, <2 x i64>* %76, align 16
  %77 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 50
  %78 = bitcast i64* %77 to <2 x i64>*
  store <2 x i64> <i64 1, i64 1>, <2 x i64>* %78, align 16
  %79 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 52
  %80 = bitcast i64* %79 to <2 x i64>*
  store <2 x i64> <i64 1, i64 1>, <2 x i64>* %80, align 16
  %81 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 54
  %82 = bitcast i64* %81 to <2 x i64>*
  store <2 x i64> <i64 1, i64 1>, <2 x i64>* %82, align 16
  %83 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 56
  %84 = bitcast i64* %83 to <2 x i64>*
  store <2 x i64> <i64 1, i64 1>, <2 x i64>* %84, align 16
  %85 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 58
  %86 = bitcast i64* %85 to <2 x i64>*
  store <2 x i64> <i64 1, i64 1>, <2 x i64>* %86, align 16
  %87 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 60
  %88 = bitcast i64* %87 to <2 x i64>*
  store <2 x i64> <i64 1, i64 1>, <2 x i64>* %88, align 16
  %89 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 62
  %90 = bitcast i64* %89 to <2 x i64>*
  store <2 x i64> <i64 1, i64 1>, <2 x i64>* %90, align 16
  %91 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 64
  store i64 1, i64* %91, align 16
  %.b = load i1, i1* @0, align 1
  br i1 %.b, label %.preheader.preheader, label %178

.preheader.preheader:                             ; preds = %2
  br label %.preheader

.preheader:                                       ; preds = %.preheader.backedge, %.preheader.preheader
  %.016 = phi float [ %0, %.preheader.preheader ], [ %.016.be, %.preheader.backedge ]
  %.0115 = phi float [ %1, %.preheader.preheader ], [ %.0115.be, %.preheader.backedge ]
  %.0414 = phi i32 [ 0, %.preheader.preheader ], [ %.0414.be, %.preheader.backedge ]
  %92 = icmp ugt i32 %.0414, 3
  br i1 %92, label %.loopexit918, label %.loopexit.thread

.loopexit918:                                     ; preds = %.preheader
  %93 = fpext float %.016 to double
  %94 = fadd double %93, 1.430000e-01
  %95 = fptrunc double %94 to float
  %96 = tail call float @_ZSt4sqrtf(float %95)
  %97 = tail call float @_ZSt4sqrtf(float %96)
  %98 = tail call float @_ZSt4sqrtf(float %97)
  %99 = fpext float %98 to double
  %100 = fadd double %99, 1.430000e-01
  %101 = fptrunc double %100 to float
  %102 = tail call float @_ZSt4sqrtf(float %101)
  %103 = tail call float @_ZSt4sqrtf(float %102)
  %104 = tail call float @_ZSt4sqrtf(float %103)
  %105 = fpext float %104 to double
  %106 = fadd double %105, 1.430000e-01
  %107 = fptrunc double %106 to float
  %108 = tail call float @_ZSt4sqrtf(float %107)
  %109 = tail call float @_ZSt4sqrtf(float %108)
  %110 = tail call float @_ZSt4sqrtf(float %109)
  %111 = fpext float %110 to double
  %112 = fadd double %111, 1.430000e-01
  %113 = fptrunc double %112 to float
  %114 = tail call float @_ZSt4sqrtf(float %113)
  %115 = tail call float @_ZSt4sqrtf(float %114)
  %116 = tail call float @_ZSt4sqrtf(float %115)
  %117 = fpext float %116 to double
  %118 = fadd double %117, 1.430000e-01
  %119 = fptrunc double %118 to float
  %120 = tail call float @_ZSt4sqrtf(float %119)
  %121 = tail call float @_ZSt4sqrtf(float %120)
  %122 = tail call float @_ZSt4sqrtf(float %121)
  %123 = fpext float %122 to double
  %124 = fadd double %123, 1.430000e-01
  %125 = fptrunc double %124 to float
  %126 = tail call float @_ZSt4sqrtf(float %125)
  %127 = tail call float @_ZSt4sqrtf(float %126)
  %128 = tail call float @_ZSt4sqrtf(float %127)
  %129 = fpext float %128 to double
  %130 = fadd double %129, 1.430000e-01
  %131 = fptrunc double %130 to float
  %132 = tail call float @_ZSt4sqrtf(float %131)
  %133 = tail call float @_ZSt4sqrtf(float %132)
  %134 = tail call float @_ZSt4sqrtf(float %133)
  %135 = fpext float %134 to double
  %136 = fadd double %135, 1.430000e-01
  %137 = fptrunc double %136 to float
  %138 = tail call float @_ZSt4sqrtf(float %137)
  %139 = tail call float @_ZSt4sqrtf(float %138)
  %140 = tail call float @_ZSt4sqrtf(float %139)
  %141 = fpext float %140 to double
  %142 = fadd double %141, 1.430000e-01
  %143 = fptrunc double %142 to float
  %144 = tail call float @_ZSt4sqrtf(float %143)
  %145 = tail call float @_ZSt4sqrtf(float %144)
  %146 = tail call float @_ZSt4sqrtf(float %145)
  %147 = fpext float %146 to double
  %148 = fadd double %147, 1.430000e-01
  %149 = fptrunc double %148 to float
  %150 = tail call float @_ZSt4sqrtf(float %149)
  %151 = tail call float @_ZSt4sqrtf(float %150)
  %152 = tail call float @_ZSt4sqrtf(float %151)
  %153 = icmp eq i32 %.0414, 9
  %154 = fadd float %.0115, %152
  %..01 = select i1 %153, float %154, float %.0115
  %155 = icmp ult i32 %.0414, 7
  br i1 %155, label %.loopexit.thread, label %.loopexit

.loopexit.thread:                                 ; preds = %.loopexit918, %.preheader
  %..015 = phi float [ %..01, %.loopexit918 ], [ %.0115, %.preheader ]
  %.24 = phi float [ %152, %.loopexit918 ], [ %.016, %.preheader ]
  %156 = fpext float %..015 to double
  %157 = fadd double %156, -1.980000e-01
  %158 = fptrunc double %157 to float
  %159 = tail call float @_ZSt4sqrtf(float %158)
  %160 = fpext float %159 to double
  %161 = fadd double %160, -1.980000e-01
  %162 = fptrunc double %161 to float
  %163 = tail call float @_ZSt4sqrtf(float %162)
  %164 = fpext float %163 to double
  %165 = fadd double %164, -1.980000e-01
  %166 = fptrunc double %165 to float
  %167 = tail call float @_ZSt4sqrtf(float %166)
  %168 = fpext float %167 to double
  %169 = fadd double %168, -1.980000e-01
  %170 = fptrunc double %169 to float
  %171 = tail call float @_ZSt4sqrtf(float %170)
  %172 = fpext float %171 to double
  %173 = fadd double %172, -1.980000e-01
  %174 = fptrunc double %173 to float
  %175 = tail call float @_ZSt4sqrtf(float %174)
  %176 = add nuw nsw i32 %.0414, 1
  br label %.preheader.backedge

.loopexit:                                        ; preds = %.loopexit918
  %177 = add nuw nsw i32 %.0414, 1
  %exitcond12 = icmp eq i32 %177, 10
  br i1 %exitcond12, label %.loopexit6, label %.preheader.backedge

.preheader.backedge:                              ; preds = %.loopexit, %.loopexit.thread
  %.016.be = phi float [ %152, %.loopexit ], [ %.24, %.loopexit.thread ]
  %.0115.be = phi float [ %..01, %.loopexit ], [ %175, %.loopexit.thread ]
  %.0414.be = phi i32 [ %177, %.loopexit ], [ %176, %.loopexit.thread ]
  br label %.preheader

.loopexit6:                                       ; preds = %.loopexit, %178
  %.0.lcssa = phi float [ %183, %178 ], [ %152, %.loopexit ]
  ret float %.0.lcssa

; <label>:178:                                    ; preds = %2
  %179 = getelementptr inbounds [65 x i64], [65 x i64]* %27, i64 0, i64 0
  %180 = bitcast [11 x i8*]* %15 to i8*
  store i1 true, i1* @0, align 1
  store float %1, float* %6, align 4
  store float %0, float* %9, align 4
  %181 = bitcast [4 x i8*]* %3 to i8*
  call void @stageDispatcher(i8* nonnull %181, i8* undef, i64* nonnull %179, i8* nonnull %180, i64 11, i64 65)
  %182 = load float*, float** %5, align 8
  %183 = load float, float* %182, align 4
  store i1 false, i1* @0, align 1
  br label %.loopexit6
}

; Function Attrs: noinline nounwind uwtable
define linkonce_odr float @_ZSt4sqrtf(float) local_unnamed_addr #1 comdat {
  %2 = tail call float @sqrtf(float %0) #21
  ret float %2
}

; Function Attrs: nounwind readnone
declare float @sqrtf(float) local_unnamed_addr #2

; Function Attrs: noinline norecurse uwtable
define i32 @main() local_unnamed_addr #3 {
  %1 = tail call float @_Z16heavyComputationff(float 0x4041A66660000000, float 0x4038B33340000000)
  %2 = fpext float %1 to double
  %3 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), double %2)
  ret i32 0
}

; Function Attrs: nounwind
declare i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #4

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
declare i32 @puts(i8* nocapture readonly) local_unnamed_addr #8

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
  br i1 %17, label %60, label %18

; <label>:18:                                     ; preds = %13
  %19 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 0
  %20 = load i8*, i8** %19, align 8, !tbaa !36
  %21 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 0
  %22 = load i8*, i8** %21, align 8, !tbaa !36
  %23 = icmp eq i8* %20, %22
  br i1 %23, label %24, label %.loopexit

; <label>:24:                                     ; preds = %18
  %25 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 3
  br label %26

; <label>:26:                                     ; preds = %30, %24
  %27 = load atomic i8, i8* %14 seq_cst, align 1
  %28 = and i8 %27, 1
  %29 = icmp eq i8 %28, 0
  br i1 %29, label %.loopexit, label %30

; <label>:30:                                     ; preds = %26
  call void @_ZNSt18condition_variable4waitERSt11unique_lockISt5mutexE(%"class.std::condition_variable"* nonnull %25, %"class.std::unique_lock"* nonnull dereferenceable(16) %3) #8
  %31 = load i8*, i8** %19, align 8, !tbaa !36
  %32 = load i8*, i8** %21, align 8, !tbaa !36
  %33 = icmp eq i8* %31, %32
  br i1 %33, label %26, label %.loopexit

.loopexit:                                        ; preds = %30, %26, %18
  %34 = load atomic i8, i8* %14 seq_cst, align 1
  %35 = and i8 %34, 1
  %36 = icmp eq i8 %35, 0
  br i1 %36, label %60, label %37

; <label>:37:                                     ; preds = %.loopexit
  %38 = load i8*, i8** %21, align 8, !tbaa !36, !noalias !37
  %39 = load i8, i8* %38, align 1, !tbaa !18
  store i8 %39, i8* %1, align 1, !tbaa !18
  %40 = load i8*, i8** %21, align 8, !tbaa !40
  %41 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 2
  %42 = load i8*, i8** %41, align 8, !tbaa !41
  %43 = getelementptr inbounds i8, i8* %42, i64 -1
  %44 = icmp eq i8* %40, %43
  br i1 %44, label %47, label %45

; <label>:45:                                     ; preds = %37
  %46 = getelementptr inbounds i8, i8* %40, i64 1
  store i8* %46, i8** %21, align 8, !tbaa !40
  br label %58

; <label>:47:                                     ; preds = %37
  %48 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2
  %49 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 1
  %50 = load i8*, i8** %49, align 8, !tbaa !42
  call void @_ZdlPv(i8* %50) #8
  %51 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 3
  %52 = load i8**, i8*** %51, align 8, !tbaa !31
  %53 = getelementptr inbounds i8*, i8** %52, i64 1
  store i8** %53, i8*** %51, align 8, !tbaa !28
  %54 = load i8*, i8** %53, align 8, !tbaa !27
  store i8* %54, i8** %49, align 8, !tbaa !29
  %55 = getelementptr inbounds i8, i8* %54, i64 512
  store i8* %55, i8** %41, align 8, !tbaa !30
  %56 = ptrtoint i8* %54 to i64
  %57 = bitcast %"struct.std::_Deque_iterator"* %48 to i64*
  store i64 %56, i64* %57, align 8, !tbaa !40
  br label %58

; <label>:58:                                     ; preds = %45, %47
  %59 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 4
  call void @_ZNSt18condition_variable10notify_oneEv(%"class.std::condition_variable"* %59) #8
  br label %60

; <label>:60:                                     ; preds = %.loopexit, %13, %58
  %61 = phi i1 [ false, %13 ], [ false, %.loopexit ], [ true, %58 ]
  %62 = load i8, i8* %7, align 8, !tbaa !35, !range !43
  %63 = icmp eq i8 %62, 0
  br i1 %63, label %72, label %64

; <label>:64:                                     ; preds = %60
  %65 = load %"class.std::mutex"*, %"class.std::mutex"** %6, align 8, !tbaa !32
  %66 = icmp eq %"class.std::mutex"* %65, null
  br i1 %66, label %72, label %67

; <label>:67:                                     ; preds = %64
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %68, label %71

; <label>:68:                                     ; preds = %67
  %69 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %65, i64 0, i32 0, i32 0
  %70 = call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %69) #8
  br label %71

; <label>:71:                                     ; preds = %68, %67
  store i8 0, i8* %7, align 8, !tbaa !35
  br label %72

; <label>:72:                                     ; preds = %60, %64, %71
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %4) #8
  ret i1 %61
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
  br i1 %17, label %60, label %18

; <label>:18:                                     ; preds = %13
  %19 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 0
  %20 = load i16*, i16** %19, align 8, !tbaa !58
  %21 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 0
  %22 = load i16*, i16** %21, align 8, !tbaa !58
  %23 = icmp eq i16* %20, %22
  br i1 %23, label %24, label %.loopexit

; <label>:24:                                     ; preds = %18
  %25 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 3
  br label %26

; <label>:26:                                     ; preds = %30, %24
  %27 = load atomic i8, i8* %14 seq_cst, align 1
  %28 = and i8 %27, 1
  %29 = icmp eq i8 %28, 0
  br i1 %29, label %.loopexit, label %30

; <label>:30:                                     ; preds = %26
  call void @_ZNSt18condition_variable4waitERSt11unique_lockISt5mutexE(%"class.std::condition_variable"* nonnull %25, %"class.std::unique_lock"* nonnull dereferenceable(16) %3) #8
  %31 = load i16*, i16** %19, align 8, !tbaa !58
  %32 = load i16*, i16** %21, align 8, !tbaa !58
  %33 = icmp eq i16* %31, %32
  br i1 %33, label %26, label %.loopexit

.loopexit:                                        ; preds = %30, %26, %18
  %34 = load atomic i8, i8* %14 seq_cst, align 1
  %35 = and i8 %34, 1
  %36 = icmp eq i8 %35, 0
  br i1 %36, label %60, label %37

; <label>:37:                                     ; preds = %.loopexit
  %38 = load i16*, i16** %21, align 8, !tbaa !58, !noalias !59
  %39 = load i16, i16* %38, align 2, !tbaa !44
  store i16 %39, i16* %1, align 2, !tbaa !44
  %40 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 2
  %41 = load i16*, i16** %40, align 8, !tbaa !62
  %42 = getelementptr inbounds i16, i16* %41, i64 -1
  %43 = icmp eq i16* %38, %42
  br i1 %43, label %46, label %44

; <label>:44:                                     ; preds = %37
  %45 = getelementptr inbounds i16, i16* %38, i64 1
  store i16* %45, i16** %21, align 8, !tbaa !63
  br label %58

; <label>:46:                                     ; preds = %37
  %47 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2
  %48 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 1
  %49 = bitcast i16** %48 to i8**
  %50 = load i8*, i8** %49, align 8, !tbaa !64
  call void @_ZdlPv(i8* %50) #8
  %51 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 3
  %52 = load i16**, i16*** %51, align 8, !tbaa !57
  %53 = getelementptr inbounds i16*, i16** %52, i64 1
  store i16** %53, i16*** %51, align 8, !tbaa !54
  %54 = load i16*, i16** %53, align 8, !tbaa !27
  store i16* %54, i16** %48, align 8, !tbaa !55
  %55 = getelementptr inbounds i16, i16* %54, i64 256
  store i16* %55, i16** %40, align 8, !tbaa !56
  %56 = ptrtoint i16* %54 to i64
  %57 = bitcast %"struct.std::_Deque_iterator.10"* %47 to i64*
  store i64 %56, i64* %57, align 8, !tbaa !63
  br label %58

; <label>:58:                                     ; preds = %44, %46
  %59 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 4
  call void @_ZNSt18condition_variable10notify_oneEv(%"class.std::condition_variable"* %59) #8
  br label %60

; <label>:60:                                     ; preds = %.loopexit, %13, %58
  %61 = phi i1 [ false, %13 ], [ false, %.loopexit ], [ true, %58 ]
  %62 = load i8, i8* %7, align 8, !tbaa !35, !range !43
  %63 = icmp eq i8 %62, 0
  br i1 %63, label %72, label %64

; <label>:64:                                     ; preds = %60
  %65 = load %"class.std::mutex"*, %"class.std::mutex"** %6, align 8, !tbaa !32
  %66 = icmp eq %"class.std::mutex"* %65, null
  br i1 %66, label %72, label %67

; <label>:67:                                     ; preds = %64
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %68, label %71

; <label>:68:                                     ; preds = %67
  %69 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %65, i64 0, i32 0, i32 0
  %70 = call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %69) #8
  br label %71

; <label>:71:                                     ; preds = %68, %67
  store i8 0, i8* %7, align 8, !tbaa !35
  br label %72

; <label>:72:                                     ; preds = %60, %64, %71
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %4) #8
  ret i1 %61
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
  br i1 %17, label %60, label %18

; <label>:18:                                     ; preds = %13
  %19 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 0
  %20 = load i32*, i32** %19, align 8, !tbaa !77
  %21 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 0
  %22 = load i32*, i32** %21, align 8, !tbaa !77
  %23 = icmp eq i32* %20, %22
  br i1 %23, label %24, label %.loopexit

; <label>:24:                                     ; preds = %18
  %25 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 3
  br label %26

; <label>:26:                                     ; preds = %30, %24
  %27 = load atomic i8, i8* %14 seq_cst, align 1
  %28 = and i8 %27, 1
  %29 = icmp eq i8 %28, 0
  br i1 %29, label %.loopexit, label %30

; <label>:30:                                     ; preds = %26
  call void @_ZNSt18condition_variable4waitERSt11unique_lockISt5mutexE(%"class.std::condition_variable"* nonnull %25, %"class.std::unique_lock"* nonnull dereferenceable(16) %3) #8
  %31 = load i32*, i32** %19, align 8, !tbaa !77
  %32 = load i32*, i32** %21, align 8, !tbaa !77
  %33 = icmp eq i32* %31, %32
  br i1 %33, label %26, label %.loopexit

.loopexit:                                        ; preds = %30, %26, %18
  %34 = load atomic i8, i8* %14 seq_cst, align 1
  %35 = and i8 %34, 1
  %36 = icmp eq i8 %35, 0
  br i1 %36, label %60, label %37

; <label>:37:                                     ; preds = %.loopexit
  %38 = load i32*, i32** %21, align 8, !tbaa !77, !noalias !78
  %39 = load i32, i32* %38, align 4, !tbaa !17
  store i32 %39, i32* %1, align 4, !tbaa !17
  %40 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 2
  %41 = load i32*, i32** %40, align 8, !tbaa !81
  %42 = getelementptr inbounds i32, i32* %41, i64 -1
  %43 = icmp eq i32* %38, %42
  br i1 %43, label %46, label %44

; <label>:44:                                     ; preds = %37
  %45 = getelementptr inbounds i32, i32* %38, i64 1
  store i32* %45, i32** %21, align 8, !tbaa !82
  br label %58

; <label>:46:                                     ; preds = %37
  %47 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2
  %48 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 1
  %49 = bitcast i32** %48 to i8**
  %50 = load i8*, i8** %49, align 8, !tbaa !83
  call void @_ZdlPv(i8* %50) #8
  %51 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 3
  %52 = load i32**, i32*** %51, align 8, !tbaa !76
  %53 = getelementptr inbounds i32*, i32** %52, i64 1
  store i32** %53, i32*** %51, align 8, !tbaa !73
  %54 = load i32*, i32** %53, align 8, !tbaa !27
  store i32* %54, i32** %48, align 8, !tbaa !74
  %55 = getelementptr inbounds i32, i32* %54, i64 128
  store i32* %55, i32** %40, align 8, !tbaa !75
  %56 = ptrtoint i32* %54 to i64
  %57 = bitcast %"struct.std::_Deque_iterator.18"* %47 to i64*
  store i64 %56, i64* %57, align 8, !tbaa !82
  br label %58

; <label>:58:                                     ; preds = %44, %46
  %59 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 4
  call void @_ZNSt18condition_variable10notify_oneEv(%"class.std::condition_variable"* %59) #8
  br label %60

; <label>:60:                                     ; preds = %.loopexit, %13, %58
  %61 = phi i1 [ false, %13 ], [ false, %.loopexit ], [ true, %58 ]
  %62 = load i8, i8* %7, align 8, !tbaa !35, !range !43
  %63 = icmp eq i8 %62, 0
  br i1 %63, label %72, label %64

; <label>:64:                                     ; preds = %60
  %65 = load %"class.std::mutex"*, %"class.std::mutex"** %6, align 8, !tbaa !32
  %66 = icmp eq %"class.std::mutex"* %65, null
  br i1 %66, label %72, label %67

; <label>:67:                                     ; preds = %64
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %68, label %71

; <label>:68:                                     ; preds = %67
  %69 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %65, i64 0, i32 0, i32 0
  %70 = call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %69) #8
  br label %71

; <label>:71:                                     ; preds = %68, %67
  store i8 0, i8* %7, align 8, !tbaa !35
  br label %72

; <label>:72:                                     ; preds = %60, %64, %71
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %4) #8
  ret i1 %61
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
  br i1 %17, label %60, label %18

; <label>:18:                                     ; preds = %13
  %19 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 0
  %20 = load i64*, i64** %19, align 8, !tbaa !97
  %21 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 0
  %22 = load i64*, i64** %21, align 8, !tbaa !97
  %23 = icmp eq i64* %20, %22
  br i1 %23, label %24, label %.loopexit

; <label>:24:                                     ; preds = %18
  %25 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 3
  br label %26

; <label>:26:                                     ; preds = %30, %24
  %27 = load atomic i8, i8* %14 seq_cst, align 1
  %28 = and i8 %27, 1
  %29 = icmp eq i8 %28, 0
  br i1 %29, label %.loopexit, label %30

; <label>:30:                                     ; preds = %26
  call void @_ZNSt18condition_variable4waitERSt11unique_lockISt5mutexE(%"class.std::condition_variable"* nonnull %25, %"class.std::unique_lock"* nonnull dereferenceable(16) %3) #8
  %31 = load i64*, i64** %19, align 8, !tbaa !97
  %32 = load i64*, i64** %21, align 8, !tbaa !97
  %33 = icmp eq i64* %31, %32
  br i1 %33, label %26, label %.loopexit

.loopexit:                                        ; preds = %30, %26, %18
  %34 = load atomic i8, i8* %14 seq_cst, align 1
  %35 = and i8 %34, 1
  %36 = icmp eq i8 %35, 0
  br i1 %36, label %60, label %37

; <label>:37:                                     ; preds = %.loopexit
  %38 = load i64*, i64** %21, align 8, !tbaa !97, !noalias !98
  %39 = load i64, i64* %38, align 8, !tbaa !84
  store i64 %39, i64* %1, align 8, !tbaa !84
  %40 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 2
  %41 = load i64*, i64** %40, align 8, !tbaa !101
  %42 = getelementptr inbounds i64, i64* %41, i64 -1
  %43 = icmp eq i64* %38, %42
  br i1 %43, label %46, label %44

; <label>:44:                                     ; preds = %37
  %45 = getelementptr inbounds i64, i64* %38, i64 1
  store i64* %45, i64** %21, align 8, !tbaa !102
  br label %58

; <label>:46:                                     ; preds = %37
  %47 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2
  %48 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 1
  %49 = bitcast i64** %48 to i8**
  %50 = load i8*, i8** %49, align 8, !tbaa !103
  call void @_ZdlPv(i8* %50) #8
  %51 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 3
  %52 = load i64**, i64*** %51, align 8, !tbaa !96
  %53 = getelementptr inbounds i64*, i64** %52, i64 1
  store i64** %53, i64*** %51, align 8, !tbaa !93
  %54 = load i64*, i64** %53, align 8, !tbaa !27
  store i64* %54, i64** %48, align 8, !tbaa !94
  %55 = getelementptr inbounds i64, i64* %54, i64 64
  store i64* %55, i64** %40, align 8, !tbaa !95
  %56 = ptrtoint i64* %54 to i64
  %57 = bitcast %"struct.std::_Deque_iterator.26"* %47 to i64*
  store i64 %56, i64* %57, align 8, !tbaa !102
  br label %58

; <label>:58:                                     ; preds = %44, %46
  %59 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 4
  call void @_ZNSt18condition_variable10notify_oneEv(%"class.std::condition_variable"* %59) #8
  br label %60

; <label>:60:                                     ; preds = %.loopexit, %13, %58
  %61 = phi i1 [ false, %13 ], [ false, %.loopexit ], [ true, %58 ]
  %62 = load i8, i8* %7, align 8, !tbaa !35, !range !43
  %63 = icmp eq i8 %62, 0
  br i1 %63, label %72, label %64

; <label>:64:                                     ; preds = %60
  %65 = load %"class.std::mutex"*, %"class.std::mutex"** %6, align 8, !tbaa !32
  %66 = icmp eq %"class.std::mutex"* %65, null
  br i1 %66, label %72, label %67

; <label>:67:                                     ; preds = %64
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %68, label %71

; <label>:68:                                     ; preds = %67
  %69 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %65, i64 0, i32 0, i32 0
  %70 = call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %69) #8
  br label %71

; <label>:71:                                     ; preds = %68, %67
  store i8 0, i8* %7, align 8, !tbaa !35
  br label %72

; <label>:72:                                     ; preds = %60, %64, %71
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %4) #8
  ret i1 %61
}

; Function Attrs: uwtable
define void @queuePush(%"class.MARC::ThreadSafeQueue"*, i8* nocapture readonly, i64) local_unnamed_addr #5 {
  %4 = icmp sgt i64 %2, 0
  br i1 %4, label %.preheader, label %.loopexit

.preheader:                                       ; preds = %3
  br label %5

.loopexit:                                        ; preds = %5, %3
  ret void

; <label>:5:                                      ; preds = %.preheader, %5
  %6 = phi i64 [ %9, %5 ], [ 0, %.preheader ]
  %7 = phi i8* [ %10, %5 ], [ %1, %.preheader ]
  %8 = load i8, i8* %7, align 1, !tbaa !18
  tail call void @_ZN4MARC15ThreadSafeQueueIcE4pushEc(%"class.MARC::ThreadSafeQueue"* %0, i8 signext %8)
  %9 = add nuw nsw i64 %6, 1
  %10 = getelementptr inbounds i8, i8* %7, i64 1
  %exitcond = icmp eq i64 %9, %2
  br i1 %exitcond, label %.loopexit, label %5
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
  br i1 %4, label %.preheader1, label %.loopexit1

.preheader1:                                      ; preds = %3
  br label %5

.loopexit1:                                       ; preds = %.loopexit, %3
  ret void

; <label>:5:                                      ; preds = %.preheader1, %.loopexit
  %6 = phi i64 [ %12, %.loopexit ], [ 0, %.preheader1 ]
  %7 = phi i8* [ %13, %.loopexit ], [ %1, %.preheader1 ]
  %8 = tail call zeroext i1 @_ZN4MARC15ThreadSafeQueueIcE7waitPopERc(%"class.MARC::ThreadSafeQueue"* %0, i8* dereferenceable(1) %7)
  br i1 %8, label %.loopexit, label %.preheader

.preheader:                                       ; preds = %5
  br label %9

; <label>:9:                                      ; preds = %.preheader, %9
  %10 = tail call i32 @puts(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @str, i64 0, i64 0))
  %11 = tail call zeroext i1 @_ZN4MARC15ThreadSafeQueueIcE7waitPopERc(%"class.MARC::ThreadSafeQueue"* %0, i8* nonnull dereferenceable(1) %7)
  br i1 %11, label %.loopexit, label %9

.loopexit:                                        ; preds = %9, %5
  %12 = add nuw nsw i64 %6, 1
  %13 = getelementptr inbounds i8, i8* %7, i64 1
  %exitcond = icmp eq i64 %12, %2
  br i1 %exitcond, label %.loopexit1, label %5
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
  br i1 %17, label %60, label %18

; <label>:18:                                     ; preds = %13
  %19 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 0
  %20 = load i8*, i8** %19, align 8, !tbaa !116
  %21 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 0
  %22 = load i8*, i8** %21, align 8, !tbaa !116
  %23 = icmp eq i8* %20, %22
  br i1 %23, label %24, label %.loopexit

; <label>:24:                                     ; preds = %18
  %25 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 3
  br label %26

; <label>:26:                                     ; preds = %30, %24
  %27 = load atomic i8, i8* %14 seq_cst, align 1
  %28 = and i8 %27, 1
  %29 = icmp eq i8 %28, 0
  br i1 %29, label %.loopexit, label %30

; <label>:30:                                     ; preds = %26
  call void @_ZNSt18condition_variable4waitERSt11unique_lockISt5mutexE(%"class.std::condition_variable"* nonnull %25, %"class.std::unique_lock"* nonnull dereferenceable(16) %3) #8
  %31 = load i8*, i8** %19, align 8, !tbaa !116
  %32 = load i8*, i8** %21, align 8, !tbaa !116
  %33 = icmp eq i8* %31, %32
  br i1 %33, label %26, label %.loopexit

.loopexit:                                        ; preds = %30, %26, %18
  %34 = load atomic i8, i8* %14 seq_cst, align 1
  %35 = and i8 %34, 1
  %36 = icmp eq i8 %35, 0
  br i1 %36, label %60, label %37

; <label>:37:                                     ; preds = %.loopexit
  %38 = load i8*, i8** %21, align 8, !tbaa !116, !noalias !117
  %39 = load i8, i8* %38, align 1, !tbaa !18
  store i8 %39, i8* %1, align 1, !tbaa !18
  %40 = load i8*, i8** %21, align 8, !tbaa !120
  %41 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 2
  %42 = load i8*, i8** %41, align 8, !tbaa !121
  %43 = getelementptr inbounds i8, i8* %42, i64 -1
  %44 = icmp eq i8* %40, %43
  br i1 %44, label %47, label %45

; <label>:45:                                     ; preds = %37
  %46 = getelementptr inbounds i8, i8* %40, i64 1
  store i8* %46, i8** %21, align 8, !tbaa !120
  br label %58

; <label>:47:                                     ; preds = %37
  %48 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2
  %49 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 1
  %50 = load i8*, i8** %49, align 8, !tbaa !122
  call void @_ZdlPv(i8* %50) #8
  %51 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 3
  %52 = load i8**, i8*** %51, align 8, !tbaa !115
  %53 = getelementptr inbounds i8*, i8** %52, i64 1
  store i8** %53, i8*** %51, align 8, !tbaa !112
  %54 = load i8*, i8** %53, align 8, !tbaa !27
  store i8* %54, i8** %49, align 8, !tbaa !113
  %55 = getelementptr inbounds i8, i8* %54, i64 512
  store i8* %55, i8** %41, align 8, !tbaa !114
  %56 = ptrtoint i8* %54 to i64
  %57 = bitcast %"struct.std::_Deque_iterator"* %48 to i64*
  store i64 %56, i64* %57, align 8, !tbaa !120
  br label %58

; <label>:58:                                     ; preds = %45, %47
  %59 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 4
  call void @_ZNSt18condition_variable10notify_oneEv(%"class.std::condition_variable"* %59) #8
  br label %60

; <label>:60:                                     ; preds = %.loopexit, %13, %58
  %61 = phi i1 [ false, %13 ], [ false, %.loopexit ], [ true, %58 ]
  %62 = load i8, i8* %7, align 8, !tbaa !35, !range !43
  %63 = icmp eq i8 %62, 0
  br i1 %63, label %72, label %64

; <label>:64:                                     ; preds = %60
  %65 = load %"class.std::mutex"*, %"class.std::mutex"** %6, align 8, !tbaa !32
  %66 = icmp eq %"class.std::mutex"* %65, null
  br i1 %66, label %72, label %67

; <label>:67:                                     ; preds = %64
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %68, label %71

; <label>:68:                                     ; preds = %67
  %69 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %65, i64 0, i32 0, i32 0
  %70 = call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %69) #8
  br label %71

; <label>:71:                                     ; preds = %68, %67
  store i8 0, i8* %7, align 8, !tbaa !35
  br label %72

; <label>:72:                                     ; preds = %60, %64, %71
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %4) #8
  ret i1 %61
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
  br i1 %16, label %.preheader18, label %.loopexit2

.preheader18:                                     ; preds = %6
  br label %20

.loopexit2:                                       ; preds = %61, %6
  %17 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %8, i64 0, i32 0, i32 0, i32 0
  call void @llvm.lifetime.start.p0i8(i64 488, i8* nonnull %17) #8
  %18 = trunc i64 %4 to i32
  %19 = getelementptr inbounds %"class.std::function", %"class.std::function"* %9, i64 0, i32 0, i32 1
  store i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* null, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %19, align 8, !tbaa !123
  invoke void @_ZN4MARC10ThreadPoolC2EjSt8functionIFvvEE(%"class.MARC::ThreadPool"* nonnull %8, i32 %18, %"class.std::function"* nonnull %9)
          to label %66 unwind label %97

; <label>:20:                                     ; preds = %.preheader18, %61
  %21 = phi i64 [ %64, %61 ], [ 0, %.preheader18 ]
  %22 = getelementptr inbounds i64, i64* %2, i64 %21
  %23 = load i64, i64* %22, align 8, !tbaa !84
  switch i64 %23, label %59 [
    i64 1, label %24
    i64 8, label %31
    i64 16, label %38
    i64 32, label %45
    i64 64, label %52
  ]

; <label>:24:                                     ; preds = %20
  %25 = tail call i8* @_Znwm(i64 224) #24
  %26 = bitcast i8* %25 to %"class.MARC::ThreadSafeQueue"*
  tail call void @llvm.memset.p0i8.i64(i8* nonnull %25, i8 0, i64 224, i32 16, i1 false)
  invoke void @_ZN4MARC15ThreadSafeQueueIaEC2Ev(%"class.MARC::ThreadSafeQueue"* nonnull %26)
          to label %61 unwind label %27

; <label>:27:                                     ; preds = %24
  %28 = landingpad { i8*, i32 }
          cleanup
  %29 = extractvalue { i8*, i32 } %28, 0
  %30 = extractvalue { i8*, i32 } %28, 1
  tail call void @_ZdlPv(i8* nonnull %25) #25
  br label %267

; <label>:31:                                     ; preds = %20
  %32 = tail call i8* @_Znwm(i64 224) #24
  %33 = bitcast i8* %32 to %"class.MARC::ThreadSafeQueue"*
  tail call void @llvm.memset.p0i8.i64(i8* nonnull %32, i8 0, i64 224, i32 16, i1 false)
  invoke void @_ZN4MARC15ThreadSafeQueueIaEC2Ev(%"class.MARC::ThreadSafeQueue"* nonnull %33)
          to label %61 unwind label %34

; <label>:34:                                     ; preds = %31
  %35 = landingpad { i8*, i32 }
          cleanup
  %36 = extractvalue { i8*, i32 } %35, 0
  %37 = extractvalue { i8*, i32 } %35, 1
  tail call void @_ZdlPv(i8* nonnull %32) #25
  br label %267

; <label>:38:                                     ; preds = %20
  %39 = tail call i8* @_Znwm(i64 224) #24
  %40 = bitcast i8* %39 to %"class.MARC::ThreadSafeQueue.3"*
  tail call void @llvm.memset.p0i8.i64(i8* nonnull %39, i8 0, i64 224, i32 16, i1 false)
  invoke void @_ZN4MARC15ThreadSafeQueueIsEC2Ev(%"class.MARC::ThreadSafeQueue.3"* nonnull %40)
          to label %61 unwind label %41

; <label>:41:                                     ; preds = %38
  %42 = landingpad { i8*, i32 }
          cleanup
  %43 = extractvalue { i8*, i32 } %42, 0
  %44 = extractvalue { i8*, i32 } %42, 1
  tail call void @_ZdlPv(i8* nonnull %39) #25
  br label %267

; <label>:45:                                     ; preds = %20
  %46 = tail call i8* @_Znwm(i64 224) #24
  %47 = bitcast i8* %46 to %"class.MARC::ThreadSafeQueue.11"*
  tail call void @llvm.memset.p0i8.i64(i8* nonnull %46, i8 0, i64 224, i32 16, i1 false)
  invoke void @_ZN4MARC15ThreadSafeQueueIiEC2Ev(%"class.MARC::ThreadSafeQueue.11"* nonnull %47)
          to label %61 unwind label %48

; <label>:48:                                     ; preds = %45
  %49 = landingpad { i8*, i32 }
          cleanup
  %50 = extractvalue { i8*, i32 } %49, 0
  %51 = extractvalue { i8*, i32 } %49, 1
  tail call void @_ZdlPv(i8* nonnull %46) #25
  br label %267

; <label>:52:                                     ; preds = %20
  %53 = tail call i8* @_Znwm(i64 224) #24
  %54 = bitcast i8* %53 to %"class.MARC::ThreadSafeQueue.19"*
  tail call void @llvm.memset.p0i8.i64(i8* nonnull %53, i8 0, i64 224, i32 16, i1 false)
  invoke void @_ZN4MARC15ThreadSafeQueueIlEC2Ev(%"class.MARC::ThreadSafeQueue.19"* nonnull %54)
          to label %61 unwind label %55

; <label>:55:                                     ; preds = %52
  %56 = landingpad { i8*, i32 }
          cleanup
  %57 = extractvalue { i8*, i32 } %56, 0
  %58 = extractvalue { i8*, i32 } %56, 1
  tail call void @_ZdlPv(i8* nonnull %53) #25
  br label %267

; <label>:59:                                     ; preds = %20
  %60 = tail call dereferenceable(272) %"class.std::basic_ostream"* @_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(%"class.std::basic_ostream"* nonnull dereferenceable(272) @_ZSt4cerr, i8* getelementptr inbounds ([23 x i8], [23 x i8]* @.str.4, i64 0, i64 0))
  tail call void @abort() #22
  unreachable

; <label>:61:                                     ; preds = %52, %45, %38, %31, %24
  %62 = phi i8* [ %25, %24 ], [ %32, %31 ], [ %39, %38 ], [ %46, %45 ], [ %53, %52 ]
  %63 = getelementptr inbounds i8*, i8** %15, i64 %21
  store i8* %62, i8** %63, align 8, !tbaa !27
  %64 = add nuw nsw i64 %21, 1
  %65 = icmp slt i64 %64, %5
  br i1 %65, label %20, label %.loopexit2

; <label>:66:                                     ; preds = %.loopexit2
  %67 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %19, align 8, !tbaa !123
  %68 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %67, null
  br i1 %68, label %75, label %69

; <label>:69:                                     ; preds = %66
  %70 = getelementptr inbounds %"class.std::function", %"class.std::function"* %9, i64 0, i32 0, i32 0
  %71 = invoke zeroext i1 %67(%"union.std::_Any_data"* nonnull dereferenceable(16) %70, %"union.std::_Any_data"* nonnull dereferenceable(16) %70, i32 3)
          to label %75 unwind label %72

; <label>:72:                                     ; preds = %69
  %73 = landingpad { i8*, i32 }
          catch i8* null
  %74 = extractvalue { i8*, i32 } %73, 0
  call void @__clang_call_terminate(i8* %74) #22
  unreachable

; <label>:75:                                     ; preds = %66, %69
  %76 = bitcast %"class.std::vector.53"* %10 to i8*
  call void @llvm.lifetime.start.p0i8(i64 24, i8* nonnull %76) #8
  call void @llvm.memset.p0i8.i64(i8* nonnull %76, i8 0, i64 24, i32 8, i1 false) #8
  %77 = icmp sgt i64 %4, 0
  br i1 %77, label %78, label %.loopexit1

; <label>:78:                                     ; preds = %75
  %79 = bitcast void (i8*, i8*)** %11 to i8*
  %80 = bitcast i8* %3 to void (i8*, i8*)**
  %81 = bitcast void (i8*, i8*)** %11 to i64*
  %82 = bitcast <2 x i64>* %12 to i8*
  %83 = bitcast i8** %14 to i8*
  %84 = bitcast i8** %14 to i8***
  %85 = getelementptr inbounds %"class.std::vector.53", %"class.std::vector.53"* %10, i64 0, i32 0, i32 0, i32 1
  %86 = getelementptr inbounds %"class.std::vector.53", %"class.std::vector.53"* %10, i64 0, i32 0, i32 0, i32 2
  %87 = bitcast <2 x i64>* %12 to %"class.std::__future_base::_State_baseV2"**
  %88 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %13, i64 0, i32 0, i32 0, i32 0, i32 0, i32 1, i32 0
  %89 = bitcast <2 x i64>* %12 to %"class.std::future"*
  %90 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %13, i64 0, i32 0, i32 0, i32 0, i32 0, i32 1
  %91 = bitcast %"class.std::__shared_count"* %90 to i64*
  br label %109

; <label>:92:                                     ; preds = %163
  %93 = getelementptr inbounds %"class.std::vector.53", %"class.std::vector.53"* %10, i64 0, i32 0, i32 0, i32 0
  %94 = load %"class.MARC::TaskFuture"*, %"class.MARC::TaskFuture"** %93, align 8, !tbaa !27
  %95 = load %"class.MARC::TaskFuture"*, %"class.MARC::TaskFuture"** %85, align 8, !tbaa !27
  %96 = icmp eq %"class.MARC::TaskFuture"* %94, %95
  br i1 %96, label %.loopexit1, label %.preheader17

.preheader17:                                     ; preds = %92
  br label %215

; <label>:97:                                     ; preds = %.loopexit2
  %98 = landingpad { i8*, i32 }
          cleanup
  %99 = extractvalue { i8*, i32 } %98, 0
  %100 = extractvalue { i8*, i32 } %98, 1
  %101 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %19, align 8, !tbaa !123
  %102 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %101, null
  br i1 %102, label %264, label %103

; <label>:103:                                    ; preds = %97
  %104 = getelementptr inbounds %"class.std::function", %"class.std::function"* %9, i64 0, i32 0, i32 0
  %105 = invoke zeroext i1 %101(%"union.std::_Any_data"* nonnull dereferenceable(16) %104, %"union.std::_Any_data"* nonnull dereferenceable(16) %104, i32 3)
          to label %264 unwind label %106

; <label>:106:                                    ; preds = %103
  %107 = landingpad { i8*, i32 }
          catch i8* null
  %108 = extractvalue { i8*, i32 } %107, 0
  call void @__clang_call_terminate(i8* %108) #22
  unreachable

; <label>:109:                                    ; preds = %78, %163
  %110 = phi i64 [ 0, %78 ], [ %164, %163 ]
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %79) #8
  %111 = getelementptr inbounds void (i8*, i8*)*, void (i8*, i8*)** %80, i64 %110
  %112 = bitcast void (i8*, i8*)** %111 to i64*
  %113 = load i64, i64* %112, align 8, !tbaa !27
  store i64 %113, i64* %81, align 8, !tbaa !27
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %82) #8
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %83) #8
  store i8** %15, i8*** %84, align 8, !tbaa !27
  invoke void @_ZN4MARC10ThreadPool6submitIRPFvPvS2_EJRS2_S2_EEEDaOT_DpOT0_(%"class.MARC::TaskFuture"* nonnull sret %13, %"class.MARC::ThreadPool"* nonnull %8, void (i8*, i8*)** nonnull dereferenceable(8) %11, i8** nonnull dereferenceable(8) %7, i8** nonnull dereferenceable(8) %14)
          to label %114 unwind label %166

; <label>:114:                                    ; preds = %109
  %115 = load %"class.MARC::TaskFuture"*, %"class.MARC::TaskFuture"** %85, align 8, !tbaa !125
  %116 = load %"class.MARC::TaskFuture"*, %"class.MARC::TaskFuture"** %86, align 8, !tbaa !128
  %117 = icmp eq %"class.MARC::TaskFuture"* %115, %116
  br i1 %117, label %123, label %118

; <label>:118:                                    ; preds = %114
  %119 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %115, i64 0, i32 0, i32 0, i32 0, i32 0, i32 1, i32 0
  store %"class.std::_Sp_counted_base"* null, %"class.std::_Sp_counted_base"** %119, align 8, !tbaa !129
  %120 = load <2 x i64>, <2 x i64>* %12, align 16, !tbaa !27
  store i64 0, i64* %91, align 8, !tbaa !129
  %121 = bitcast %"class.MARC::TaskFuture"* %115 to <2 x i64>*
  store <2 x i64> %120, <2 x i64>* %121, align 8, !tbaa !27
  store %"class.std::__future_base::_State_baseV2"* null, %"class.std::__future_base::_State_baseV2"** %87, align 16, !tbaa !131
  %122 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %115, i64 1
  store %"class.MARC::TaskFuture"* %122, %"class.MARC::TaskFuture"** %85, align 8, !tbaa !125
  br label %132

; <label>:123:                                    ; preds = %114
  invoke void @_ZNSt6vectorIN4MARC10TaskFutureIvEESaIS2_EE19_M_emplace_back_auxIJS2_EEEvDpOT_(%"class.std::vector.53"* nonnull %10, %"class.MARC::TaskFuture"* nonnull dereferenceable(16) %13)
          to label %124 unwind label %170

; <label>:124:                                    ; preds = %123
  %125 = load %"class.std::__future_base::_State_baseV2"*, %"class.std::__future_base::_State_baseV2"** %87, align 16, !tbaa !131
  %126 = icmp eq %"class.std::__future_base::_State_baseV2"* %125, null
  br i1 %126, label %132, label %127

; <label>:127:                                    ; preds = %124
  invoke void @_ZNSt6futureIvE3getEv(%"class.std::future"* nonnull %89)
          to label %132 unwind label %128

; <label>:128:                                    ; preds = %127
  %129 = landingpad { i8*, i32 }
          catch i8* null
  %130 = bitcast <2 x i64>* %12 to %"class.std::__basic_future"*
  %131 = extractvalue { i8*, i32 } %129, 0
  call void @_ZNSt14__basic_futureIvED2Ev(%"class.std::__basic_future"* nonnull %130) #8
  call void @__clang_call_terminate(i8* %131) #22
  unreachable

; <label>:132:                                    ; preds = %118, %127, %124
  %133 = load %"class.std::_Sp_counted_base"*, %"class.std::_Sp_counted_base"** %88, align 8, !tbaa !129
  %134 = icmp eq %"class.std::_Sp_counted_base"* %133, null
  br i1 %134, label %163, label %135

; <label>:135:                                    ; preds = %132
  %136 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %133, i64 0, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %137, label %139

; <label>:137:                                    ; preds = %135
  %138 = atomicrmw volatile add i32* %136, i32 -1 acq_rel
  br label %142

; <label>:139:                                    ; preds = %135
  %140 = load i32, i32* %136, align 4, !tbaa !17
  %141 = add nsw i32 %140, -1
  store i32 %141, i32* %136, align 4, !tbaa !17
  br label %142

; <label>:142:                                    ; preds = %139, %137
  %143 = phi i32 [ %138, %137 ], [ %140, %139 ]
  %144 = icmp eq i32 %143, 1
  br i1 %144, label %145, label %163

; <label>:145:                                    ; preds = %142
  %146 = bitcast %"class.std::_Sp_counted_base"* %133 to void (%"class.std::_Sp_counted_base"*)***
  %147 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %146, align 8, !tbaa !133
  %148 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %147, i64 2
  %149 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %148, align 8
  call void %149(%"class.std::_Sp_counted_base"* nonnull %133) #8
  %150 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %133, i64 0, i32 2
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %151, label %153

; <label>:151:                                    ; preds = %145
  %152 = atomicrmw volatile add i32* %150, i32 -1 acq_rel
  br label %156

; <label>:153:                                    ; preds = %145
  %154 = load i32, i32* %150, align 4, !tbaa !17
  %155 = add nsw i32 %154, -1
  store i32 %155, i32* %150, align 4, !tbaa !17
  br label %156

; <label>:156:                                    ; preds = %153, %151
  %157 = phi i32 [ %152, %151 ], [ %154, %153 ]
  %158 = icmp eq i32 %157, 1
  br i1 %158, label %159, label %163

; <label>:159:                                    ; preds = %156
  %160 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %146, align 8, !tbaa !133
  %161 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %160, i64 3
  %162 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %161, align 8
  call void %162(%"class.std::_Sp_counted_base"* nonnull %133) #8
  br label %163

; <label>:163:                                    ; preds = %132, %142, %156, %159
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %83) #8
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %82) #8
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %79) #8
  %164 = add nuw nsw i64 %110, 1
  %165 = icmp slt i64 %164, %4
  br i1 %165, label %109, label %92

; <label>:166:                                    ; preds = %109
  %167 = landingpad { i8*, i32 }
          cleanup
  %168 = extractvalue { i8*, i32 } %167, 0
  %169 = extractvalue { i8*, i32 } %167, 1
  br label %212

; <label>:170:                                    ; preds = %123
  %171 = landingpad { i8*, i32 }
          cleanup
  %172 = extractvalue { i8*, i32 } %171, 0
  %173 = extractvalue { i8*, i32 } %171, 1
  %174 = bitcast <2 x i64>* %12 to %"class.std::__basic_future"*
  %175 = load %"class.std::__future_base::_State_baseV2"*, %"class.std::__future_base::_State_baseV2"** %87, align 16, !tbaa !131
  %176 = icmp eq %"class.std::__future_base::_State_baseV2"* %175, null
  br i1 %176, label %181, label %177

; <label>:177:                                    ; preds = %170
  invoke void @_ZNSt6futureIvE3getEv(%"class.std::future"* nonnull %89)
          to label %181 unwind label %178

; <label>:178:                                    ; preds = %177
  %179 = landingpad { i8*, i32 }
          catch i8* null
  %180 = extractvalue { i8*, i32 } %179, 0
  call void @_ZNSt14__basic_futureIvED2Ev(%"class.std::__basic_future"* nonnull %174) #8
  call void @__clang_call_terminate(i8* %180) #22
  unreachable

; <label>:181:                                    ; preds = %177, %170
  %182 = load %"class.std::_Sp_counted_base"*, %"class.std::_Sp_counted_base"** %88, align 8, !tbaa !129
  %183 = icmp eq %"class.std::_Sp_counted_base"* %182, null
  br i1 %183, label %212, label %184

; <label>:184:                                    ; preds = %181
  %185 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %182, i64 0, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %186, label %188

; <label>:186:                                    ; preds = %184
  %187 = atomicrmw volatile add i32* %185, i32 -1 acq_rel
  br label %191

; <label>:188:                                    ; preds = %184
  %189 = load i32, i32* %185, align 4, !tbaa !17
  %190 = add nsw i32 %189, -1
  store i32 %190, i32* %185, align 4, !tbaa !17
  br label %191

; <label>:191:                                    ; preds = %188, %186
  %192 = phi i32 [ %187, %186 ], [ %189, %188 ]
  %193 = icmp eq i32 %192, 1
  br i1 %193, label %194, label %212

; <label>:194:                                    ; preds = %191
  %195 = bitcast %"class.std::_Sp_counted_base"* %182 to void (%"class.std::_Sp_counted_base"*)***
  %196 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %195, align 8, !tbaa !133
  %197 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %196, i64 2
  %198 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %197, align 8
  call void %198(%"class.std::_Sp_counted_base"* nonnull %182) #8
  %199 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %182, i64 0, i32 2
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %200, label %202

; <label>:200:                                    ; preds = %194
  %201 = atomicrmw volatile add i32* %199, i32 -1 acq_rel
  br label %205

; <label>:202:                                    ; preds = %194
  %203 = load i32, i32* %199, align 4, !tbaa !17
  %204 = add nsw i32 %203, -1
  store i32 %204, i32* %199, align 4, !tbaa !17
  br label %205

; <label>:205:                                    ; preds = %202, %200
  %206 = phi i32 [ %201, %200 ], [ %203, %202 ]
  %207 = icmp eq i32 %206, 1
  br i1 %207, label %208, label %212

; <label>:208:                                    ; preds = %205
  %209 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %195, align 8, !tbaa !133
  %210 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %209, i64 3
  %211 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %210, align 8
  call void %211(%"class.std::_Sp_counted_base"* nonnull %182) #8
  br label %212

; <label>:212:                                    ; preds = %208, %205, %191, %181, %166
  %213 = phi i32 [ %169, %166 ], [ %173, %181 ], [ %173, %191 ], [ %173, %205 ], [ %173, %208 ]
  %214 = phi i8* [ %168, %166 ], [ %172, %181 ], [ %172, %191 ], [ %172, %205 ], [ %172, %208 ]
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %83) #8
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %82) #8
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %79) #8
  br label %261

.loopexit1:                                       ; preds = %218, %75, %92
  br i1 %16, label %.preheader, label %.loopexit

.preheader:                                       ; preds = %.loopexit1
  br label %225

; <label>:215:                                    ; preds = %.preheader17, %218
  %216 = phi %"class.MARC::TaskFuture"* [ %219, %218 ], [ %94, %.preheader17 ]
  %217 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %216, i64 0, i32 0
  invoke void @_ZNSt6futureIvE3getEv(%"class.std::future"* %217)
          to label %218 unwind label %221

; <label>:218:                                    ; preds = %215
  %219 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %216, i64 1
  %220 = icmp eq %"class.MARC::TaskFuture"* %219, %95
  br i1 %220, label %.loopexit1, label %215

; <label>:221:                                    ; preds = %215
  %222 = landingpad { i8*, i32 }
          cleanup
  %223 = extractvalue { i8*, i32 } %222, 0
  %224 = extractvalue { i8*, i32 } %222, 1
  br label %261

.loopexit:                                        ; preds = %259, %.loopexit1
  call void @_ZNSt6vectorIN4MARC10TaskFutureIvEESaIS2_EED2Ev(%"class.std::vector.53"* nonnull %10) #8
  call void @llvm.lifetime.end.p0i8(i64 24, i8* nonnull %76) #8
  call void @_ZN4MARC10ThreadPoolD2Ev(%"class.MARC::ThreadPool"* nonnull %8) #8
  call void @llvm.lifetime.end.p0i8(i64 488, i8* nonnull %17) #8
  ret void

; <label>:225:                                    ; preds = %.preheader, %259
  %226 = phi i64 [ %260, %259 ], [ 0, %.preheader ]
  %227 = getelementptr inbounds i64, i64* %2, i64 %226
  %228 = load i64, i64* %227, align 8, !tbaa !84
  switch i64 %228, label %259 [
    i64 1, label %229
    i64 8, label %235
    i64 16, label %241
    i64 32, label %247
    i64 64, label %253
  ]

; <label>:229:                                    ; preds = %225
  %230 = getelementptr inbounds i8*, i8** %15, i64 %226
  %231 = load i8*, i8** %230, align 8, !tbaa !27
  %232 = icmp eq i8* %231, null
  br i1 %232, label %259, label %233

; <label>:233:                                    ; preds = %229
  %234 = bitcast i8* %231 to %"class.MARC::ThreadSafeQueue"*
  call void @_ZN4MARC15ThreadSafeQueueIaED2Ev(%"class.MARC::ThreadSafeQueue"* %234) #8
  call void @_ZdlPv(i8* nonnull %231) #25
  br label %259

; <label>:235:                                    ; preds = %225
  %236 = getelementptr inbounds i8*, i8** %15, i64 %226
  %237 = load i8*, i8** %236, align 8, !tbaa !27
  %238 = icmp eq i8* %237, null
  br i1 %238, label %259, label %239

; <label>:239:                                    ; preds = %235
  %240 = bitcast i8* %237 to %"class.MARC::ThreadSafeQueue"*
  call void @_ZN4MARC15ThreadSafeQueueIaED2Ev(%"class.MARC::ThreadSafeQueue"* %240) #8
  call void @_ZdlPv(i8* nonnull %237) #25
  br label %259

; <label>:241:                                    ; preds = %225
  %242 = getelementptr inbounds i8*, i8** %15, i64 %226
  %243 = load i8*, i8** %242, align 8, !tbaa !27
  %244 = icmp eq i8* %243, null
  br i1 %244, label %259, label %245

; <label>:245:                                    ; preds = %241
  %246 = bitcast i8* %243 to %"class.MARC::ThreadSafeQueue.3"*
  call void @_ZN4MARC15ThreadSafeQueueIsED2Ev(%"class.MARC::ThreadSafeQueue.3"* %246) #8
  call void @_ZdlPv(i8* nonnull %243) #25
  br label %259

; <label>:247:                                    ; preds = %225
  %248 = getelementptr inbounds i8*, i8** %15, i64 %226
  %249 = load i8*, i8** %248, align 8, !tbaa !27
  %250 = icmp eq i8* %249, null
  br i1 %250, label %259, label %251

; <label>:251:                                    ; preds = %247
  %252 = bitcast i8* %249 to %"class.MARC::ThreadSafeQueue.11"*
  call void @_ZN4MARC15ThreadSafeQueueIiED2Ev(%"class.MARC::ThreadSafeQueue.11"* %252) #8
  call void @_ZdlPv(i8* nonnull %249) #25
  br label %259

; <label>:253:                                    ; preds = %225
  %254 = getelementptr inbounds i8*, i8** %15, i64 %226
  %255 = load i8*, i8** %254, align 8, !tbaa !27
  %256 = icmp eq i8* %255, null
  br i1 %256, label %259, label %257

; <label>:257:                                    ; preds = %253
  %258 = bitcast i8* %255 to %"class.MARC::ThreadSafeQueue.19"*
  call void @_ZN4MARC15ThreadSafeQueueIlED2Ev(%"class.MARC::ThreadSafeQueue.19"* %258) #8
  call void @_ZdlPv(i8* nonnull %255) #25
  br label %259

; <label>:259:                                    ; preds = %225, %233, %229, %239, %235, %245, %241, %251, %247, %257, %253
  %260 = add nuw nsw i64 %226, 1
  %exitcond = icmp eq i64 %260, %5
  br i1 %exitcond, label %.loopexit, label %225

; <label>:261:                                    ; preds = %221, %212
  %262 = phi i32 [ %213, %212 ], [ %224, %221 ]
  %263 = phi i8* [ %214, %212 ], [ %223, %221 ]
  call void @_ZNSt6vectorIN4MARC10TaskFutureIvEESaIS2_EED2Ev(%"class.std::vector.53"* nonnull %10) #8
  call void @llvm.lifetime.end.p0i8(i64 24, i8* nonnull %76) #8
  call void @_ZN4MARC10ThreadPoolD2Ev(%"class.MARC::ThreadPool"* nonnull %8) #8
  br label %264

; <label>:264:                                    ; preds = %103, %97, %261
  %265 = phi i32 [ %262, %261 ], [ %100, %97 ], [ %100, %103 ]
  %266 = phi i8* [ %263, %261 ], [ %99, %97 ], [ %99, %103 ]
  call void @llvm.lifetime.end.p0i8(i64 488, i8* nonnull %17) #8
  br label %267

; <label>:267:                                    ; preds = %27, %34, %41, %48, %55, %264
  %268 = phi i32 [ %265, %264 ], [ %58, %55 ], [ %51, %48 ], [ %44, %41 ], [ %37, %34 ], [ %30, %27 ]
  %269 = phi i8* [ %266, %264 ], [ %57, %55 ], [ %50, %48 ], [ %43, %41 ], [ %36, %34 ], [ %29, %27 ]
  %270 = insertvalue { i8*, i32 } undef, i8* %269, 0
  %271 = insertvalue { i8*, i32 } %270, i32 %268, 1
  resume { i8*, i32 } %271
}

; Function Attrs: uwtable
define linkonce_odr void @_ZN4MARC10ThreadPoolC2EjSt8functionIFvvEE(%"class.MARC::ThreadPool"*, i32, %"class.std::function"*) unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %4 = alloca %"class.std::unique_ptr", align 8
  %5 = alloca <2 x i64>, align 16
  %6 = bitcast <2 x i64>* %5 to { i64, i64 }*
  %7 = alloca %"class.MARC::ThreadPool"*, align 8
  %8 = alloca %"struct.std::atomic"*, align 8
  %9 = alloca %"class.std::function", align 8
  %10 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 0, i32 0, i32 0
  store i8 0, i8* %10, align 1, !tbaa !135
  %11 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 1
  %12 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %11, i64 0, i32 0, i32 0, i32 0
  tail call void @llvm.memset.p0i8.i64(i8* %12, i8 0, i64 224, i32 8, i1 false)
  tail call void @_ZN4MARC15ThreadSafeQueueISt10unique_ptrINS_11IThreadTaskESt14default_deleteIS2_EEEC2Ev(%"class.MARC::ThreadSafeQueue.32"* %11)
  %13 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 2
  %14 = bitcast %"class.std::vector"* %13 to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %14, i8 0, i64 24, i32 8, i1 false) #8
  %15 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 4
  invoke void @_ZN4MARC15ThreadSafeQueueISt8functionIFvvEEEC2Ev(%"class.MARC::ThreadSafeQueue.45"* %15)
          to label %16 unwind label %53

; <label>:16:                                     ; preds = %3
  %17 = zext i32 %1 to i64
  %18 = invoke i8* @_Znam(i64 %17) #24
          to label %19 unwind label %57

; <label>:19:                                     ; preds = %16
  %20 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 3
  %21 = bitcast %"struct.std::atomic"** %20 to i8**
  store i8* %18, i8** %21, align 8, !tbaa !137
  %22 = icmp eq i32 %1, 0
  br i1 %22, label %.loopexit1, label %23

; <label>:23:                                     ; preds = %19
  store atomic i8 1, i8* %18 seq_cst, align 1
  %24 = icmp eq i32 %1, 1
  br i1 %24, label %.loopexit2, label %25

; <label>:25:                                     ; preds = %23
  %26 = add nsw i64 %17, -2
  %27 = add i32 %1, 7
  %28 = and i32 %27, 7
  %29 = zext i32 %28 to i64
  %30 = icmp ult i64 %26, 7
  br i1 %30, label %.loopexit3, label %31

; <label>:31:                                     ; preds = %25
  %32 = add nsw i64 %17, -1
  %33 = sub nsw i64 %32, %29
  br label %60

.loopexit3:                                       ; preds = %60, %25
  %34 = phi i64 [ 1, %25 ], [ %86, %60 ]
  %35 = icmp eq i32 %28, 0
  br i1 %35, label %.loopexit2, label %.preheader2

.preheader2:                                      ; preds = %.loopexit3
  br label %36

; <label>:36:                                     ; preds = %.preheader2, %36
  %37 = phi i64 [ %41, %36 ], [ %34, %.preheader2 ]
  %38 = phi i64 [ %42, %36 ], [ %29, %.preheader2 ]
  %39 = load %"struct.std::atomic"*, %"struct.std::atomic"** %20, align 8, !tbaa !137
  %40 = getelementptr inbounds %"struct.std::atomic", %"struct.std::atomic"* %39, i64 %37, i32 0, i32 0
  store atomic i8 1, i8* %40 seq_cst, align 1
  %41 = add nuw nsw i64 %37, 1
  %42 = add nsw i64 %38, -1
  %43 = icmp eq i64 %42, 0
  br i1 %43, label %.loopexit2, label %36, !llvm.loop !149

.loopexit2:                                       ; preds = %36, %23, %.loopexit3
  %44 = bitcast <2 x i64>* %5 to i8*
  %45 = bitcast %"class.MARC::ThreadPool"** %7 to i8*
  %46 = bitcast %"struct.std::atomic"** %8 to i8*
  %47 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 2, i32 0, i32 0, i32 1
  %48 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 2, i32 0, i32 0, i32 2
  %49 = bitcast %"class.std::unique_ptr"* %4 to i8*
  %50 = bitcast %"class.std::unique_ptr"* %4 to i8**
  %51 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %4, i64 0, i32 0, i32 0, i32 0, i32 0
  %52 = ptrtoint %"class.MARC::ThreadPool"* %0 to i64
  br label %92

; <label>:53:                                     ; preds = %3
  %54 = landingpad { i8*, i32 }
          cleanup
  %55 = extractvalue { i8*, i32 } %54, 0
  %56 = extractvalue { i8*, i32 } %54, 1
  br label %189

; <label>:57:                                     ; preds = %16
  %58 = landingpad { i8*, i32 }
          catch i8* null
  %59 = extractvalue { i8*, i32 } %58, 0
  br label %137

; <label>:60:                                     ; preds = %60, %31
  %61 = phi i64 [ 1, %31 ], [ %86, %60 ]
  %62 = phi i64 [ %33, %31 ], [ %87, %60 ]
  %63 = load %"struct.std::atomic"*, %"struct.std::atomic"** %20, align 8, !tbaa !137
  %64 = getelementptr inbounds %"struct.std::atomic", %"struct.std::atomic"* %63, i64 %61, i32 0, i32 0
  store atomic i8 1, i8* %64 seq_cst, align 1
  %65 = add nuw nsw i64 %61, 1
  %66 = load %"struct.std::atomic"*, %"struct.std::atomic"** %20, align 8, !tbaa !137
  %67 = getelementptr inbounds %"struct.std::atomic", %"struct.std::atomic"* %66, i64 %65, i32 0, i32 0
  store atomic i8 1, i8* %67 seq_cst, align 1
  %68 = add nuw nsw i64 %61, 2
  %69 = load %"struct.std::atomic"*, %"struct.std::atomic"** %20, align 8, !tbaa !137
  %70 = getelementptr inbounds %"struct.std::atomic", %"struct.std::atomic"* %69, i64 %68, i32 0, i32 0
  store atomic i8 1, i8* %70 seq_cst, align 1
  %71 = add nuw nsw i64 %61, 3
  %72 = load %"struct.std::atomic"*, %"struct.std::atomic"** %20, align 8, !tbaa !137
  %73 = getelementptr inbounds %"struct.std::atomic", %"struct.std::atomic"* %72, i64 %71, i32 0, i32 0
  store atomic i8 1, i8* %73 seq_cst, align 1
  %74 = add nuw nsw i64 %61, 4
  %75 = load %"struct.std::atomic"*, %"struct.std::atomic"** %20, align 8, !tbaa !137
  %76 = getelementptr inbounds %"struct.std::atomic", %"struct.std::atomic"* %75, i64 %74, i32 0, i32 0
  store atomic i8 1, i8* %76 seq_cst, align 1
  %77 = add nuw nsw i64 %61, 5
  %78 = load %"struct.std::atomic"*, %"struct.std::atomic"** %20, align 8, !tbaa !137
  %79 = getelementptr inbounds %"struct.std::atomic", %"struct.std::atomic"* %78, i64 %77, i32 0, i32 0
  store atomic i8 1, i8* %79 seq_cst, align 1
  %80 = add nuw nsw i64 %61, 6
  %81 = load %"struct.std::atomic"*, %"struct.std::atomic"** %20, align 8, !tbaa !137
  %82 = getelementptr inbounds %"struct.std::atomic", %"struct.std::atomic"* %81, i64 %80, i32 0, i32 0
  store atomic i8 1, i8* %82 seq_cst, align 1
  %83 = add nuw nsw i64 %61, 7
  %84 = load %"struct.std::atomic"*, %"struct.std::atomic"** %20, align 8, !tbaa !137
  %85 = getelementptr inbounds %"struct.std::atomic", %"struct.std::atomic"* %84, i64 %83, i32 0, i32 0
  store atomic i8 1, i8* %85 seq_cst, align 1
  %86 = add nuw nsw i64 %61, 8
  %87 = add i64 %62, -8
  %88 = icmp eq i64 %87, 0
  br i1 %88, label %.loopexit3, label %60

.loopexit1:                                       ; preds = %_ZNSt6vectorISt6threadSaIS0_EE12emplace_backIJMN4MARC10ThreadPoolEFvPSt6atomicIbEEPS5_S8_EEEvDpOT_.exit, %19
  %89 = getelementptr inbounds %"class.std::function", %"class.std::function"* %2, i64 0, i32 0, i32 1
  %90 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %89, align 8, !tbaa !123
  %91 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %90, null
  br i1 %91, label %185, label %145

; <label>:92:                                     ; preds = %.loopexit2, %_ZNSt6vectorISt6threadSaIS0_EE12emplace_backIJMN4MARC10ThreadPoolEFvPSt6atomicIbEEPS5_S8_EEEvDpOT_.exit
  %93 = phi i64 [ 0, %.loopexit2 ], [ %132, %_ZNSt6vectorISt6threadSaIS0_EE12emplace_backIJMN4MARC10ThreadPoolEFvPSt6atomicIbEEPS5_S8_EEEvDpOT_.exit ]
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %44) #8
  store <2 x i64> <i64 ptrtoint (void (%"class.MARC::ThreadPool"*, %"struct.std::atomic"*)* @_ZN4MARC10ThreadPool6workerEPSt6atomicIbE to i64), i64 0>, <2 x i64>* %5, align 16
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %45) #8
  store %"class.MARC::ThreadPool"* %0, %"class.MARC::ThreadPool"** %7, align 8, !tbaa !27
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %46) #8
  %94 = load %"struct.std::atomic"*, %"struct.std::atomic"** %20, align 8, !tbaa !137
  %95 = getelementptr inbounds %"struct.std::atomic", %"struct.std::atomic"* %94, i64 %93
  store %"struct.std::atomic"* %95, %"struct.std::atomic"** %8, align 8, !tbaa !27
  %96 = load %"class.std::thread"*, %"class.std::thread"** %47, align 8, !tbaa !151
  %97 = load %"class.std::thread"*, %"class.std::thread"** %48, align 8, !tbaa !154
  %98 = icmp eq %"class.std::thread"* %96, %97
  %99 = ptrtoint %"struct.std::atomic"* %95 to i64
  br i1 %98, label %131, label %100

; <label>:100:                                    ; preds = %92
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %49)
  %101 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %96, i64 0, i32 0, i32 0
  store i64 0, i64* %101, align 8, !tbaa !155
  %102 = load <2 x i64>, <2 x i64>* %5, align 16, !tbaa !18, !noalias !157
  %103 = invoke i8* @_Znwm(i64 40) #24
          to label %.noexc unwind label %134

.noexc:                                           ; preds = %100
  %104 = bitcast i8* %103 to i32 (...)***
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [5 x i8*] }, { [5 x i8*] }* @_ZTVNSt6thread11_State_implISt12_Bind_simpleIFSt7_Mem_fnIMN4MARC10ThreadPoolEFvPSt6atomicIbEEEPS4_S7_EEEE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %104, align 8, !tbaa !133, !noalias !160
  %105 = getelementptr inbounds i8, i8* %103, i64 8
  %106 = bitcast i8* %105 to i64*
  store i64 %99, i64* %106, align 8, !tbaa !163, !noalias !160
  %107 = getelementptr inbounds i8, i8* %103, i64 16
  %108 = bitcast i8* %107 to i64*
  store i64 %52, i64* %108, align 8, !tbaa !165, !noalias !160
  %109 = getelementptr inbounds i8, i8* %103, i64 24
  %110 = bitcast i8* %109 to <2 x i64>*
  store <2 x i64> %102, <2 x i64>* %110, align 8
  store i8* %103, i8** %50, align 8, !tbaa !27, !alias.scope !160
  invoke void @_ZNSt6thread15_M_start_threadESt10unique_ptrINS_6_StateESt14default_deleteIS1_EEPFvvE(%"class.std::thread"* nonnull %96, %"class.std::unique_ptr"* nonnull %4, void ()* bitcast (i32 (i64*, %union.pthread_attr_t*, i8* (i8*)*, i8*)* @pthread_create to void ()*))
          to label %111 unwind label %119

; <label>:111:                                    ; preds = %.noexc
  %112 = load %"class.MARC::IThreadTask"*, %"class.MARC::IThreadTask"** %51, align 8, !tbaa !27
  %113 = icmp eq %"class.MARC::IThreadTask"* %112, null
  br i1 %113, label %128, label %114

; <label>:114:                                    ; preds = %111
  %115 = bitcast %"class.MARC::IThreadTask"* %112 to void (%"class.MARC::IThreadTask"*)***
  %116 = load void (%"class.MARC::IThreadTask"*)**, void (%"class.MARC::IThreadTask"*)*** %115, align 8, !tbaa !133
  %117 = getelementptr inbounds void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %116, i64 1
  %118 = load void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %117, align 8
  call void %118(%"class.MARC::IThreadTask"* nonnull %112) #8
  br label %128

; <label>:119:                                    ; preds = %.noexc
  %120 = landingpad { i8*, i32 }
          catch i8* null
  %121 = load %"class.MARC::IThreadTask"*, %"class.MARC::IThreadTask"** %51, align 8, !tbaa !27
  %122 = icmp eq %"class.MARC::IThreadTask"* %121, null
  br i1 %122, label %.body, label %123

; <label>:123:                                    ; preds = %119
  %124 = bitcast %"class.MARC::IThreadTask"* %121 to void (%"class.MARC::IThreadTask"*)***
  %125 = load void (%"class.MARC::IThreadTask"*)**, void (%"class.MARC::IThreadTask"*)*** %124, align 8, !tbaa !133
  %126 = getelementptr inbounds void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %125, i64 1
  %127 = load void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %126, align 8
  call void %127(%"class.MARC::IThreadTask"* nonnull %121) #8
  br label %.body

; <label>:128:                                    ; preds = %114, %111
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %49)
  %129 = load %"class.std::thread"*, %"class.std::thread"** %47, align 8, !tbaa !151
  %130 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %129, i64 1
  store %"class.std::thread"* %130, %"class.std::thread"** %47, align 8, !tbaa !151
  br label %_ZNSt6vectorISt6threadSaIS0_EE12emplace_backIJMN4MARC10ThreadPoolEFvPSt6atomicIbEEPS5_S8_EEEvDpOT_.exit

; <label>:131:                                    ; preds = %92
  invoke void @_ZNSt6vectorISt6threadSaIS0_EE19_M_emplace_back_auxIJMN4MARC10ThreadPoolEFvPSt6atomicIbEEPS5_S8_EEEvDpOT_(%"class.std::vector"* nonnull %13, { i64, i64 }* nonnull dereferenceable(16) %6, %"class.MARC::ThreadPool"** nonnull dereferenceable(8) %7, %"struct.std::atomic"** nonnull dereferenceable(8) %8)
          to label %_ZNSt6vectorISt6threadSaIS0_EE12emplace_backIJMN4MARC10ThreadPoolEFvPSt6atomicIbEEPS5_S8_EEEvDpOT_.exit unwind label %134

_ZNSt6vectorISt6threadSaIS0_EE12emplace_backIJMN4MARC10ThreadPoolEFvPSt6atomicIbEEPS5_S8_EEEvDpOT_.exit: ; preds = %128, %131
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %46) #8
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %45) #8
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %44) #8
  %132 = add nuw nsw i64 %93, 1
  %133 = icmp ult i64 %132, %17
  br i1 %133, label %92, label %.loopexit1

; <label>:134:                                    ; preds = %131, %100
  %135 = landingpad { i8*, i32 }
          catch i8* null
  br label %.body

.body:                                            ; preds = %119, %123, %134
  %eh.lpad-body = phi { i8*, i32 } [ %135, %134 ], [ %120, %123 ], [ %120, %119 ]
  %136 = extractvalue { i8*, i32 } %eh.lpad-body, 0
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %46) #8
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %45) #8
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %44) #8
  br label %137

; <label>:137:                                    ; preds = %.body, %57
  %138 = phi i8* [ %136, %.body ], [ %59, %57 ]
  %139 = call i8* @__cxa_begin_catch(i8* %138) #8
  invoke void @_ZN4MARC10ThreadPool7destroyEv(%"class.MARC::ThreadPool"* nonnull %0)
          to label %140 unwind label %141

; <label>:140:                                    ; preds = %137
  invoke void @__cxa_rethrow() #23
          to label %215 unwind label %141

; <label>:141:                                    ; preds = %140, %137
  %142 = landingpad { i8*, i32 }
          cleanup
  %143 = extractvalue { i8*, i32 } %142, 0
  %144 = extractvalue { i8*, i32 } %142, 1
  invoke void @__cxa_end_catch()
          to label %186 unwind label %212

; <label>:145:                                    ; preds = %.loopexit1
  %146 = getelementptr inbounds %"class.std::function", %"class.std::function"* %9, i64 0, i32 0, i32 1
  store i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* null, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %146, align 8, !tbaa !123
  %147 = getelementptr inbounds %"class.std::function", %"class.std::function"* %9, i64 0, i32 0, i32 0
  %148 = getelementptr inbounds %"class.std::function", %"class.std::function"* %2, i64 0, i32 0, i32 0
  %149 = invoke zeroext i1 %90(%"union.std::_Any_data"* nonnull dereferenceable(16) %147, %"union.std::_Any_data"* nonnull dereferenceable(16) %148, i32 2)
          to label %159 unwind label %150

; <label>:150:                                    ; preds = %145
  %151 = landingpad { i8*, i32 }
          cleanup
  %152 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %146, align 8, !tbaa !123
  %153 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %152, null
  br i1 %153, label %171, label %154

; <label>:154:                                    ; preds = %150
  %155 = invoke zeroext i1 %152(%"union.std::_Any_data"* nonnull dereferenceable(16) %147, %"union.std::_Any_data"* nonnull dereferenceable(16) %147, i32 3)
          to label %171 unwind label %156

; <label>:156:                                    ; preds = %154
  %157 = landingpad { i8*, i32 }
          catch i8* null
  %158 = extractvalue { i8*, i32 } %157, 0
  call void @__clang_call_terminate(i8* %158) #22
  unreachable

; <label>:159:                                    ; preds = %145
  %160 = bitcast i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %89 to <2 x i64>*
  %161 = load <2 x i64>, <2 x i64>* %160, align 8, !tbaa !27
  %162 = bitcast i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %146 to <2 x i64>*
  store <2 x i64> %161, <2 x i64>* %162, align 8, !tbaa !27
  invoke void @_ZN4MARC15ThreadSafeQueueISt8functionIFvvEEE4pushES3_(%"class.MARC::ThreadSafeQueue.45"* nonnull %15, %"class.std::function"* nonnull %9)
          to label %163 unwind label %174

; <label>:163:                                    ; preds = %159
  %164 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %146, align 8, !tbaa !123
  %165 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %164, null
  br i1 %165, label %185, label %166

; <label>:166:                                    ; preds = %163
  %167 = invoke zeroext i1 %164(%"union.std::_Any_data"* nonnull dereferenceable(16) %147, %"union.std::_Any_data"* nonnull dereferenceable(16) %147, i32 3)
          to label %185 unwind label %168

; <label>:168:                                    ; preds = %166
  %169 = landingpad { i8*, i32 }
          catch i8* null
  %170 = extractvalue { i8*, i32 } %169, 0
  call void @__clang_call_terminate(i8* %170) #22
  unreachable

; <label>:171:                                    ; preds = %150, %154
  %172 = extractvalue { i8*, i32 } %151, 0
  %173 = extractvalue { i8*, i32 } %151, 1
  br label %186

; <label>:174:                                    ; preds = %159
  %175 = landingpad { i8*, i32 }
          cleanup
  %176 = extractvalue { i8*, i32 } %175, 0
  %177 = extractvalue { i8*, i32 } %175, 1
  %178 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %146, align 8, !tbaa !123
  %179 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %178, null
  br i1 %179, label %186, label %180

; <label>:180:                                    ; preds = %174
  %181 = invoke zeroext i1 %178(%"union.std::_Any_data"* nonnull dereferenceable(16) %147, %"union.std::_Any_data"* nonnull dereferenceable(16) %147, i32 3)
          to label %186 unwind label %182

; <label>:182:                                    ; preds = %180
  %183 = landingpad { i8*, i32 }
          catch i8* null
  %184 = extractvalue { i8*, i32 } %183, 0
  call void @__clang_call_terminate(i8* %184) #22
  unreachable

; <label>:185:                                    ; preds = %.loopexit1, %166, %163
  ret void

; <label>:186:                                    ; preds = %180, %174, %141, %171
  %187 = phi i8* [ %143, %141 ], [ %172, %171 ], [ %176, %174 ], [ %176, %180 ]
  %188 = phi i32 [ %144, %141 ], [ %173, %171 ], [ %177, %174 ], [ %177, %180 ]
  call void @_ZN4MARC15ThreadSafeQueueISt8functionIFvvEEED2Ev(%"class.MARC::ThreadSafeQueue.45"* %15) #8
  br label %189

; <label>:189:                                    ; preds = %186, %53
  %190 = phi i8* [ %187, %186 ], [ %55, %53 ]
  %191 = phi i32 [ %188, %186 ], [ %56, %53 ]
  %192 = getelementptr inbounds %"class.std::vector", %"class.std::vector"* %13, i64 0, i32 0, i32 0, i32 0
  %193 = load %"class.std::thread"*, %"class.std::thread"** %192, align 8, !tbaa !167
  %194 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 2, i32 0, i32 0, i32 1
  %195 = load %"class.std::thread"*, %"class.std::thread"** %194, align 8, !tbaa !151
  %196 = icmp eq %"class.std::thread"* %193, %195
  br i1 %196, label %.loopexit, label %.preheader

.preheader:                                       ; preds = %189
  br label %199

; <label>:197:                                    ; preds = %199
  %198 = icmp eq %"class.std::thread"* %204, %195
  br i1 %198, label %.loopexit, label %199

; <label>:199:                                    ; preds = %.preheader, %197
  %200 = phi %"class.std::thread"* [ %204, %197 ], [ %193, %.preheader ]
  %201 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %200, i64 0, i32 0, i32 0
  %202 = load i64, i64* %201, align 8
  %203 = icmp eq i64 %202, 0
  %204 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %200, i64 1
  br i1 %203, label %197, label %205

; <label>:205:                                    ; preds = %199
  call void @_ZSt9terminatev() #22
  unreachable

.loopexit:                                        ; preds = %197, %189
  %206 = icmp eq %"class.std::thread"* %193, null
  br i1 %206, label %209, label %207

; <label>:207:                                    ; preds = %.loopexit
  %208 = bitcast %"class.std::thread"* %193 to i8*
  call void @_ZdlPv(i8* %208) #8
  br label %209

; <label>:209:                                    ; preds = %.loopexit, %207
  call void @_ZN4MARC15ThreadSafeQueueISt10unique_ptrINS_11IThreadTaskESt14default_deleteIS2_EEED2Ev(%"class.MARC::ThreadSafeQueue.32"* %11) #8
  %210 = insertvalue { i8*, i32 } undef, i8* %190, 0
  %211 = insertvalue { i8*, i32 } %210, i32 %191, 1
  resume { i8*, i32 } %211

; <label>:212:                                    ; preds = %141
  %213 = landingpad { i8*, i32 }
          catch i8* null
  %214 = extractvalue { i8*, i32 } %213, 0
  call void @__clang_call_terminate(i8* %214) #22
  unreachable

; <label>:215:                                    ; preds = %140
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
          to label %11 unwind label %71

; <label>:11:                                     ; preds = %1
  %12 = getelementptr inbounds %"class.std::deque", %"class.std::deque"* %2, i64 0, i32 0, i32 0, i32 0
  %13 = load i8**, i8*** %12, align 16, !tbaa !26
  %14 = icmp eq i8** %13, null
  br i1 %14, label %68, label %15

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
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %24, i8* nonnull %25, i64 32, i32 8, i1 false) #8, !tbaa.struct !168
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
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %36, i8* nonnull %37, i64 32, i32 8, i1 false) #8, !tbaa.struct !168
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
  br i1 %49, label %68, label %54

; <label>:54:                                     ; preds = %15
  %55 = inttoptr i64 %48 to i8*
  %56 = getelementptr inbounds i8*, i8** %53, i64 1
  %57 = icmp ugt i8** %56, %51
  br i1 %57, label %.preheader, label %66

.preheader:                                       ; preds = %54
  br label %58

; <label>:58:                                     ; preds = %.preheader, %58
  %59 = phi i8** [ %61, %58 ], [ %51, %.preheader ]
  %60 = load i8*, i8** %59, align 8, !tbaa !27
  call void @_ZdlPv(i8* %60) #8
  %61 = getelementptr inbounds i8*, i8** %59, i64 1
  %62 = icmp ult i8** %59, %53
  br i1 %62, label %58, label %63

; <label>:63:                                     ; preds = %58
  %64 = bitcast %"class.std::deque"* %2 to i8**
  %65 = load i8*, i8** %64, align 16, !tbaa !26
  br label %66

; <label>:66:                                     ; preds = %63, %54
  %67 = phi i8* [ %65, %63 ], [ %55, %54 ]
  call void @_ZdlPv(i8* %67) #8
  br label %68

; <label>:68:                                     ; preds = %11, %15, %66
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %7) #8
  %69 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 3
  call void @_ZNSt18condition_variableC1Ev(%"class.std::condition_variable"* %69) #8
  %70 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 4
  call void @_ZNSt18condition_variableC1Ev(%"class.std::condition_variable"* %70) #8
  ret void

; <label>:71:                                     ; preds = %1
  %72 = landingpad { i8*, i32 }
          cleanup
  %73 = getelementptr inbounds %"class.std::deque", %"class.std::deque"* %2, i64 0, i32 0, i32 0, i32 0
  %74 = load i8**, i8*** %73, align 16, !tbaa !26
  %75 = icmp eq i8** %74, null
  br i1 %75, label %94, label %76

; <label>:76:                                     ; preds = %71
  %77 = bitcast i8** %74 to i8*
  %78 = getelementptr inbounds %"class.std::deque", %"class.std::deque"* %2, i64 0, i32 0, i32 0, i32 2, i32 3
  %79 = load i8**, i8*** %78, align 8, !tbaa !31
  %80 = getelementptr inbounds %"class.std::deque", %"class.std::deque"* %2, i64 0, i32 0, i32 0, i32 3, i32 3
  %81 = load i8**, i8*** %80, align 8, !tbaa !25
  %82 = getelementptr inbounds i8*, i8** %81, i64 1
  %83 = icmp ult i8** %79, %82
  br i1 %83, label %.preheader1, label %92

.preheader1:                                      ; preds = %76
  br label %84

; <label>:84:                                     ; preds = %.preheader1, %84
  %85 = phi i8** [ %87, %84 ], [ %79, %.preheader1 ]
  %86 = load i8*, i8** %85, align 8, !tbaa !27
  call void @_ZdlPv(i8* %86) #8
  %87 = getelementptr inbounds i8*, i8** %85, i64 1
  %88 = icmp ult i8** %85, %81
  br i1 %88, label %84, label %89

; <label>:89:                                     ; preds = %84
  %90 = bitcast %"class.std::deque"* %2 to i8**
  %91 = load i8*, i8** %90, align 16, !tbaa !26
  br label %92

; <label>:92:                                     ; preds = %89, %76
  %93 = phi i8* [ %91, %89 ], [ %77, %76 ]
  call void @_ZdlPv(i8* %93) #8
  br label %94

; <label>:94:                                     ; preds = %71, %92
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %7) #8
  resume { i8*, i32 } %72
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
          to label %11 unwind label %72

; <label>:11:                                     ; preds = %1
  %12 = getelementptr inbounds %"class.std::deque.5", %"class.std::deque.5"* %2, i64 0, i32 0, i32 0, i32 0
  %13 = load i16**, i16*** %12, align 16, !tbaa !53
  %14 = icmp eq i16** %13, null
  br i1 %14, label %69, label %15

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
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %24, i8* nonnull %25, i64 32, i32 8, i1 false) #8, !tbaa.struct !168
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
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %36, i8* nonnull %37, i64 32, i32 8, i1 false) #8, !tbaa.struct !168
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
  br i1 %49, label %69, label %54

; <label>:54:                                     ; preds = %15
  %55 = inttoptr i64 %48 to i8*
  %56 = getelementptr inbounds i16*, i16** %53, i64 1
  %57 = icmp ugt i16** %56, %51
  br i1 %57, label %.preheader, label %67

.preheader:                                       ; preds = %54
  br label %58

; <label>:58:                                     ; preds = %.preheader, %58
  %59 = phi i16** [ %62, %58 ], [ %51, %.preheader ]
  %60 = bitcast i16** %59 to i8**
  %61 = load i8*, i8** %60, align 8, !tbaa !27
  call void @_ZdlPv(i8* %61) #8
  %62 = getelementptr inbounds i16*, i16** %59, i64 1
  %63 = icmp ult i16** %59, %53
  br i1 %63, label %58, label %64

; <label>:64:                                     ; preds = %58
  %65 = bitcast %"class.std::deque.5"* %2 to i8**
  %66 = load i8*, i8** %65, align 16, !tbaa !53
  br label %67

; <label>:67:                                     ; preds = %64, %54
  %68 = phi i8* [ %66, %64 ], [ %55, %54 ]
  call void @_ZdlPv(i8* %68) #8
  br label %69

; <label>:69:                                     ; preds = %11, %15, %67
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %7) #8
  %70 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 3
  call void @_ZNSt18condition_variableC1Ev(%"class.std::condition_variable"* %70) #8
  %71 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 4
  call void @_ZNSt18condition_variableC1Ev(%"class.std::condition_variable"* %71) #8
  ret void

; <label>:72:                                     ; preds = %1
  %73 = landingpad { i8*, i32 }
          cleanup
  %74 = getelementptr inbounds %"class.std::deque.5", %"class.std::deque.5"* %2, i64 0, i32 0, i32 0, i32 0
  %75 = load i16**, i16*** %74, align 16, !tbaa !53
  %76 = icmp eq i16** %75, null
  br i1 %76, label %96, label %77

; <label>:77:                                     ; preds = %72
  %78 = bitcast i16** %75 to i8*
  %79 = getelementptr inbounds %"class.std::deque.5", %"class.std::deque.5"* %2, i64 0, i32 0, i32 0, i32 2, i32 3
  %80 = load i16**, i16*** %79, align 8, !tbaa !57
  %81 = getelementptr inbounds %"class.std::deque.5", %"class.std::deque.5"* %2, i64 0, i32 0, i32 0, i32 3, i32 3
  %82 = load i16**, i16*** %81, align 8, !tbaa !52
  %83 = getelementptr inbounds i16*, i16** %82, i64 1
  %84 = icmp ult i16** %80, %83
  br i1 %84, label %.preheader1, label %94

.preheader1:                                      ; preds = %77
  br label %85

; <label>:85:                                     ; preds = %.preheader1, %85
  %86 = phi i16** [ %89, %85 ], [ %80, %.preheader1 ]
  %87 = bitcast i16** %86 to i8**
  %88 = load i8*, i8** %87, align 8, !tbaa !27
  call void @_ZdlPv(i8* %88) #8
  %89 = getelementptr inbounds i16*, i16** %86, i64 1
  %90 = icmp ult i16** %86, %82
  br i1 %90, label %85, label %91

; <label>:91:                                     ; preds = %85
  %92 = bitcast %"class.std::deque.5"* %2 to i8**
  %93 = load i8*, i8** %92, align 16, !tbaa !53
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
          to label %11 unwind label %72

; <label>:11:                                     ; preds = %1
  %12 = getelementptr inbounds %"class.std::deque.13", %"class.std::deque.13"* %2, i64 0, i32 0, i32 0, i32 0
  %13 = load i32**, i32*** %12, align 16, !tbaa !72
  %14 = icmp eq i32** %13, null
  br i1 %14, label %69, label %15

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
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %24, i8* nonnull %25, i64 32, i32 8, i1 false) #8, !tbaa.struct !168
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
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %36, i8* nonnull %37, i64 32, i32 8, i1 false) #8, !tbaa.struct !168
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
  br i1 %49, label %69, label %54

; <label>:54:                                     ; preds = %15
  %55 = inttoptr i64 %48 to i8*
  %56 = getelementptr inbounds i32*, i32** %53, i64 1
  %57 = icmp ugt i32** %56, %51
  br i1 %57, label %.preheader, label %67

.preheader:                                       ; preds = %54
  br label %58

; <label>:58:                                     ; preds = %.preheader, %58
  %59 = phi i32** [ %62, %58 ], [ %51, %.preheader ]
  %60 = bitcast i32** %59 to i8**
  %61 = load i8*, i8** %60, align 8, !tbaa !27
  call void @_ZdlPv(i8* %61) #8
  %62 = getelementptr inbounds i32*, i32** %59, i64 1
  %63 = icmp ult i32** %59, %53
  br i1 %63, label %58, label %64

; <label>:64:                                     ; preds = %58
  %65 = bitcast %"class.std::deque.13"* %2 to i8**
  %66 = load i8*, i8** %65, align 16, !tbaa !72
  br label %67

; <label>:67:                                     ; preds = %64, %54
  %68 = phi i8* [ %66, %64 ], [ %55, %54 ]
  call void @_ZdlPv(i8* %68) #8
  br label %69

; <label>:69:                                     ; preds = %11, %15, %67
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %7) #8
  %70 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 3
  call void @_ZNSt18condition_variableC1Ev(%"class.std::condition_variable"* %70) #8
  %71 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 4
  call void @_ZNSt18condition_variableC1Ev(%"class.std::condition_variable"* %71) #8
  ret void

; <label>:72:                                     ; preds = %1
  %73 = landingpad { i8*, i32 }
          cleanup
  %74 = getelementptr inbounds %"class.std::deque.13", %"class.std::deque.13"* %2, i64 0, i32 0, i32 0, i32 0
  %75 = load i32**, i32*** %74, align 16, !tbaa !72
  %76 = icmp eq i32** %75, null
  br i1 %76, label %96, label %77

; <label>:77:                                     ; preds = %72
  %78 = bitcast i32** %75 to i8*
  %79 = getelementptr inbounds %"class.std::deque.13", %"class.std::deque.13"* %2, i64 0, i32 0, i32 0, i32 2, i32 3
  %80 = load i32**, i32*** %79, align 8, !tbaa !76
  %81 = getelementptr inbounds %"class.std::deque.13", %"class.std::deque.13"* %2, i64 0, i32 0, i32 0, i32 3, i32 3
  %82 = load i32**, i32*** %81, align 8, !tbaa !71
  %83 = getelementptr inbounds i32*, i32** %82, i64 1
  %84 = icmp ult i32** %80, %83
  br i1 %84, label %.preheader1, label %94

.preheader1:                                      ; preds = %77
  br label %85

; <label>:85:                                     ; preds = %.preheader1, %85
  %86 = phi i32** [ %89, %85 ], [ %80, %.preheader1 ]
  %87 = bitcast i32** %86 to i8**
  %88 = load i8*, i8** %87, align 8, !tbaa !27
  call void @_ZdlPv(i8* %88) #8
  %89 = getelementptr inbounds i32*, i32** %86, i64 1
  %90 = icmp ult i32** %86, %82
  br i1 %90, label %85, label %91

; <label>:91:                                     ; preds = %85
  %92 = bitcast %"class.std::deque.13"* %2 to i8**
  %93 = load i8*, i8** %92, align 16, !tbaa !72
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
          to label %11 unwind label %72

; <label>:11:                                     ; preds = %1
  %12 = getelementptr inbounds %"class.std::deque.21", %"class.std::deque.21"* %2, i64 0, i32 0, i32 0, i32 0
  %13 = load i64**, i64*** %12, align 16, !tbaa !92
  %14 = icmp eq i64** %13, null
  br i1 %14, label %69, label %15

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
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %24, i8* nonnull %25, i64 32, i32 8, i1 false) #8, !tbaa.struct !168
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
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %36, i8* nonnull %37, i64 32, i32 8, i1 false) #8, !tbaa.struct !168
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
  br i1 %49, label %69, label %54

; <label>:54:                                     ; preds = %15
  %55 = inttoptr i64 %48 to i8*
  %56 = getelementptr inbounds i64*, i64** %53, i64 1
  %57 = icmp ugt i64** %56, %51
  br i1 %57, label %.preheader, label %67

.preheader:                                       ; preds = %54
  br label %58

; <label>:58:                                     ; preds = %.preheader, %58
  %59 = phi i64** [ %62, %58 ], [ %51, %.preheader ]
  %60 = bitcast i64** %59 to i8**
  %61 = load i8*, i8** %60, align 8, !tbaa !27
  call void @_ZdlPv(i8* %61) #8
  %62 = getelementptr inbounds i64*, i64** %59, i64 1
  %63 = icmp ult i64** %59, %53
  br i1 %63, label %58, label %64

; <label>:64:                                     ; preds = %58
  %65 = bitcast %"class.std::deque.21"* %2 to i8**
  %66 = load i8*, i8** %65, align 16, !tbaa !92
  br label %67

; <label>:67:                                     ; preds = %64, %54
  %68 = phi i8* [ %66, %64 ], [ %55, %54 ]
  call void @_ZdlPv(i8* %68) #8
  br label %69

; <label>:69:                                     ; preds = %11, %15, %67
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %7) #8
  %70 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 3
  call void @_ZNSt18condition_variableC1Ev(%"class.std::condition_variable"* %70) #8
  %71 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 4
  call void @_ZNSt18condition_variableC1Ev(%"class.std::condition_variable"* %71) #8
  ret void

; <label>:72:                                     ; preds = %1
  %73 = landingpad { i8*, i32 }
          cleanup
  %74 = getelementptr inbounds %"class.std::deque.21", %"class.std::deque.21"* %2, i64 0, i32 0, i32 0, i32 0
  %75 = load i64**, i64*** %74, align 16, !tbaa !92
  %76 = icmp eq i64** %75, null
  br i1 %76, label %96, label %77

; <label>:77:                                     ; preds = %72
  %78 = bitcast i64** %75 to i8*
  %79 = getelementptr inbounds %"class.std::deque.21", %"class.std::deque.21"* %2, i64 0, i32 0, i32 0, i32 2, i32 3
  %80 = load i64**, i64*** %79, align 8, !tbaa !96
  %81 = getelementptr inbounds %"class.std::deque.21", %"class.std::deque.21"* %2, i64 0, i32 0, i32 0, i32 3, i32 3
  %82 = load i64**, i64*** %81, align 8, !tbaa !91
  %83 = getelementptr inbounds i64*, i64** %82, i64 1
  %84 = icmp ult i64** %80, %83
  br i1 %84, label %.preheader1, label %94

.preheader1:                                      ; preds = %77
  br label %85

; <label>:85:                                     ; preds = %.preheader1, %85
  %86 = phi i64** [ %89, %85 ], [ %80, %.preheader1 ]
  %87 = bitcast i64** %86 to i8**
  %88 = load i8*, i8** %87, align 8, !tbaa !27
  call void @_ZdlPv(i8* %88) #8
  %89 = getelementptr inbounds i64*, i64** %86, i64 1
  %90 = icmp ult i64** %86, %82
  br i1 %90, label %85, label %91

; <label>:91:                                     ; preds = %85
  %92 = bitcast %"class.std::deque.21"* %2 to i8**
  %93 = load i8*, i8** %92, align 16, !tbaa !92
  br label %94

; <label>:94:                                     ; preds = %91, %77
  %95 = phi i8* [ %93, %91 ], [ %78, %77 ]
  call void @_ZdlPv(i8* %95) #8
  br label %96

; <label>:96:                                     ; preds = %72, %94
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %7) #8
  resume { i8*, i32 } %73
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
  %15 = load i64, i64* %14, align 8, !tbaa !27, !noalias !169
  %16 = bitcast i8** %4 to i64*
  %17 = load i64, i64* %16, align 8, !tbaa !27, !noalias !169
  %18 = bitcast i8** %3 to i64*
  %19 = load i64, i64* %18, align 8, !tbaa !27, !noalias !169
  %20 = bitcast <2 x i64>* %8 to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %20) #8
  %21 = bitcast <2 x i64>* %6 to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %21) #8, !noalias !172
  %22 = getelementptr inbounds %"class.std::shared_ptr.121", %"class.std::shared_ptr.121"* %7, i64 0, i32 0, i32 1
  call void @llvm.memset.p0i8.i64(i8* nonnull %21, i8 0, i64 16, i32 16, i1 false)
  %23 = invoke i8* @_Znwm(i64 80)
          to label %24 unwind label %56, !noalias !175

; <label>:24:                                     ; preds = %5
  %25 = bitcast i8* %23 to i32 (...)***
  %26 = getelementptr inbounds i8, i8* %23, i64 8
  %27 = bitcast i8* %26 to i32*
  store i32 1, i32* %27, align 8, !tbaa !178, !noalias !175
  %28 = getelementptr inbounds i8, i8* %23, i64 12
  %29 = bitcast i8* %28 to i32*
  store i32 1, i32* %29, align 4, !tbaa !180, !noalias !175
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [7 x i8*] }, { [7 x i8*] }* @_ZTVSt23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %25, align 8, !tbaa !133, !noalias !175
  %30 = getelementptr inbounds i8, i8* %23, i64 16
  %31 = bitcast i8* %30 to i32 (...)***
  %32 = getelementptr inbounds i8, i8* %23, i64 24
  %33 = bitcast i8* %32 to %"struct.std::__future_base::_Result_base"**
  store %"struct.std::__future_base::_Result_base"* null, %"struct.std::__future_base::_Result_base"** %33, align 8, !tbaa !181, !noalias !175
  %34 = getelementptr inbounds i8, i8* %23, i64 32
  %35 = bitcast i8* %34 to i32*
  store i32 0, i32* %35, align 4, !tbaa !183, !noalias !175
  %36 = getelementptr inbounds i8, i8* %23, i64 36
  store i8 0, i8* %36, align 1, !tbaa !185, !noalias !175
  %37 = getelementptr inbounds i8, i8* %23, i64 40
  %38 = bitcast i8* %37 to i32*
  store i32 0, i32* %38, align 4, !tbaa !187, !noalias !175
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [9 x i8*] }, { [9 x i8*] }* @_ZTVNSt13__future_base16_Task_state_baseIFvvEEE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %31, align 8, !tbaa !133, !noalias !175
  %39 = getelementptr inbounds i8, i8* %23, i64 48
  %40 = invoke i8* @_Znwm(i64 16) #24
          to label %41 unwind label %43, !noalias !175

; <label>:41:                                     ; preds = %24
  %42 = bitcast i8* %40 to %"struct.std::__future_base::_Result_base"*
  invoke void @_ZNSt13__future_base12_Result_baseC2Ev(%"struct.std::__future_base::_Result_base"* nonnull %42)
          to label %69 unwind label %45, !noalias !189

; <label>:43:                                     ; preds = %24
  %44 = landingpad { i8*, i32 }
          cleanup
  br label %60

; <label>:45:                                     ; preds = %41
  %46 = landingpad { i8*, i32 }
          cleanup
  tail call void @_ZdlPv(i8* nonnull %40) #25, !noalias !189
  %47 = load %"struct.std::__future_base::_Result_base"*, %"struct.std::__future_base::_Result_base"** %33, align 8, !tbaa !27, !noalias !175
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [6 x i8*] }, { [6 x i8*] }* @_ZTVNSt13__future_base13_State_baseV2E, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %31, align 8, !tbaa !133, !noalias !175
  %48 = icmp eq %"struct.std::__future_base::_Result_base"* %47, null
  br i1 %48, label %60, label %49

; <label>:49:                                     ; preds = %45
  %50 = bitcast %"struct.std::__future_base::_Result_base"* %47 to void (%"struct.std::__future_base::_Result_base"*)***
  %51 = load void (%"struct.std::__future_base::_Result_base"*)**, void (%"struct.std::__future_base::_Result_base"*)*** %50, align 8, !tbaa !133, !noalias !175
  %52 = load void (%"struct.std::__future_base::_Result_base"*)*, void (%"struct.std::__future_base::_Result_base"*)** %51, align 8, !noalias !175
  invoke void %52(%"struct.std::__future_base::_Result_base"* nonnull %47)
          to label %60 unwind label %53, !noalias !175

; <label>:53:                                     ; preds = %49
  %54 = landingpad { i8*, i32 }
          catch i8* null
  %55 = extractvalue { i8*, i32 } %54, 0
  tail call void @__clang_call_terminate(i8* %55) #22, !noalias !175
  unreachable

; <label>:56:                                     ; preds = %5
  %57 = landingpad { i8*, i32 }
          cleanup
  %58 = extractvalue { i8*, i32 } %57, 0
  %59 = extractvalue { i8*, i32 } %57, 1
  br label %64

; <label>:60:                                     ; preds = %43, %45, %49
  %61 = phi { i8*, i32 } [ %44, %43 ], [ %46, %45 ], [ %46, %49 ]
  %62 = extractvalue { i8*, i32 } %61, 0
  %63 = extractvalue { i8*, i32 } %61, 1
  tail call void @_ZdlPv(i8* nonnull %23) #8
  br label %64

; <label>:64:                                     ; preds = %56, %60
  %65 = phi i32 [ %63, %60 ], [ %59, %56 ]
  %66 = phi i8* [ %62, %60 ], [ %58, %56 ]
  %67 = insertvalue { i8*, i32 } undef, i8* %66, 0
  %68 = insertvalue { i8*, i32 } %67, i32 %65, 1
  resume { i8*, i32 } %68

; <label>:69:                                     ; preds = %41
  %70 = bitcast i8* %40 to i32 (...)***
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [5 x i8*] }, { [5 x i8*] }* @_ZTVNSt13__future_base7_ResultIvEE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %70, align 8, !tbaa !133, !noalias !189
  %71 = bitcast i8* %39 to i8**
  store i8* %40, i8** %71, align 8, !tbaa !27, !alias.scope !192, !noalias !175
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [9 x i8*] }, { [9 x i8*] }* @_ZTVNSt13__future_base11_Task_stateISt5_BindIFPFvPvS2_ES2_S2_EESaIiEFvvEEE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %31, align 8, !tbaa !133, !noalias !175
  %72 = getelementptr inbounds i8, i8* %23, i64 56
  %73 = bitcast i8* %72 to i64*
  store i64 %15, i64* %73, align 8, !tbaa !193, !noalias !175
  %74 = getelementptr inbounds i8, i8* %23, i64 64
  %75 = bitcast i8* %74 to i64*
  store i64 %17, i64* %75, align 8, !tbaa !196, !noalias !175
  %76 = getelementptr inbounds i8, i8* %23, i64 72
  %77 = bitcast i8* %76 to i64*
  store i64 %19, i64* %77, align 8, !tbaa !198, !noalias !175
  %78 = bitcast %"class.std::__shared_count"* %22 to i8**
  store i8* %23, i8** %78, align 8, !tbaa !129, !alias.scope !175
  %79 = bitcast i8* %23 to %"class.std::_Sp_counted_base"*
  %80 = bitcast i8* %23 to i8* (%"class.std::_Sp_counted_base"*, %"class.std::type_info"*)***
  %81 = load i8* (%"class.std::_Sp_counted_base"*, %"class.std::type_info"*)**, i8* (%"class.std::_Sp_counted_base"*, %"class.std::type_info"*)*** %80, align 8, !tbaa !133, !noalias !175
  %82 = getelementptr inbounds i8* (%"class.std::_Sp_counted_base"*, %"class.std::type_info"*)*, i8* (%"class.std::_Sp_counted_base"*, %"class.std::type_info"*)** %81, i64 4
  %83 = load i8* (%"class.std::_Sp_counted_base"*, %"class.std::type_info"*)*, i8* (%"class.std::_Sp_counted_base"*, %"class.std::type_info"*)** %82, align 8, !noalias !175
  %84 = tail call i8* %83(%"class.std::_Sp_counted_base"* nonnull %79, %"class.std::type_info"* nonnull dereferenceable(16) bitcast ({ i8*, i8* }* @_ZTISt19_Sp_make_shared_tag to %"class.std::type_info"*)) #8, !noalias !175
  %85 = bitcast <2 x i64>* %6 to i8**
  store i8* %84, i8** %85, align 16, !tbaa !200, !alias.scope !175
  call void (%"class.std::__shared_count"*, ...) @_ZSt32__enable_shared_from_this_helperILN9__gnu_cxx12_Lock_policyE2EEvRKSt14__shared_countIXT_EEz(%"class.std::__shared_count"* nonnull dereferenceable(8) %22, i8* %84, i8* %84) #8
  %86 = load <2 x i64>, <2 x i64>* %6, align 16, !tbaa !27, !noalias !172
  store <2 x i64> %86, <2 x i64>* %8, align 16, !tbaa !27, !alias.scope !172
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %21) #8, !noalias !172
  %87 = bitcast <2 x i64>* %10 to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %87) #8
  %88 = bitcast <2 x i64>* %12 to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %88) #8
  invoke void @_ZNSt13packaged_taskIFvvEE10get_futureEv(%"class.std::future"* nonnull sret %13, %"class.std::packaged_task"* nonnull %9)
          to label %89 unwind label %159

; <label>:89:                                     ; preds = %69
  %90 = load <2 x i64>, <2 x i64>* %12, align 16, !tbaa !27
  store <2 x i64> %90, <2 x i64>* %10, align 16, !tbaa !27
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %88) #8
  %91 = invoke i8* @_Znwm(i64 24) #24
          to label %92 unwind label %163

; <label>:92:                                     ; preds = %89
  %93 = bitcast i8* %91 to i32 (...)***
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [5 x i8*] }, { [5 x i8*] }* @_ZTVN4MARC10ThreadTaskISt13packaged_taskIFvvEEEE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %93, align 8, !tbaa !133, !noalias !202
  %94 = getelementptr inbounds i8, i8* %91, i64 8
  %95 = load <2 x i64>, <2 x i64>* %8, align 16, !tbaa !27, !noalias !202
  store <2 x i64> zeroinitializer, <2 x i64>* %8, align 16, !tbaa !27, !noalias !202
  %96 = bitcast i8* %94 to <2 x i64>*
  store <2 x i64> %95, <2 x i64>* %96, align 8, !tbaa !27, !noalias !202
  %97 = ptrtoint i8* %91 to i64
  %98 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %1, i64 0, i32 1, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %99, label %105

; <label>:99:                                     ; preds = %92
  %100 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %98, i64 0, i32 0, i32 0
  %101 = call i32 @pthread_mutex_lock(%union.pthread_mutex_t* nonnull %100) #8
  %102 = icmp eq i32 %101, 0
  br i1 %102, label %105, label %103

; <label>:103:                                    ; preds = %99
  invoke void @_ZSt20__throw_system_errori(i32 %101) #23
          to label %104 unwind label %169

; <label>:104:                                    ; preds = %103
  unreachable

; <label>:105:                                    ; preds = %99, %92
  %106 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %1, i64 0, i32 1, i32 2
  %107 = getelementptr inbounds %"class.std::queue.33", %"class.std::queue.33"* %106, i64 0, i32 0
  %108 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %1, i64 0, i32 1, i32 2, i32 0, i32 0, i32 0, i32 3, i32 0
  %109 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %108, align 8, !tbaa !205
  %110 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %1, i64 0, i32 1, i32 2, i32 0, i32 0, i32 0, i32 3, i32 2
  %111 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %110, align 8, !tbaa !209
  %112 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %111, i64 -1
  %113 = icmp eq %"class.std::unique_ptr"* %109, %112
  br i1 %113, label %117, label %114

; <label>:114:                                    ; preds = %105
  %115 = bitcast %"class.std::unique_ptr"* %109 to i64*
  store i64 %97, i64* %115, align 8, !tbaa !210
  %116 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %109, i64 1
  store %"class.std::unique_ptr"* %116, %"class.std::unique_ptr"** %108, align 8, !tbaa !205
  br label %146

; <label>:117:                                    ; preds = %105
  %118 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %1, i64 0, i32 1, i32 2, i32 0, i32 0, i32 0, i32 1
  %119 = load i64, i64* %118, align 8, !tbaa !212
  %120 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %1, i64 0, i32 1, i32 2, i32 0, i32 0, i32 0, i32 3, i32 3
  %121 = bitcast %"class.std::unique_ptr"*** %120 to i64*
  %122 = load i64, i64* %121, align 8, !tbaa !213
  %123 = bitcast %"class.std::queue.33"* %106 to i64*
  %124 = load i64, i64* %123, align 8, !tbaa !214
  %125 = sub i64 %122, %124
  %126 = ashr exact i64 %125, 3
  %127 = sub i64 %119, %126
  %128 = icmp ult i64 %127, 2
  br i1 %128, label %129, label %130

; <label>:129:                                    ; preds = %117
  invoke void @_ZNSt5dequeISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EESaIS5_EE17_M_reallocate_mapEmb(%"class.std::deque.34"* nonnull %107, i64 1, i1 zeroext false)
          to label %130 unwind label %151

; <label>:130:                                    ; preds = %129, %117
  %131 = invoke i8* @_Znwm(i64 512)
          to label %132 unwind label %151

; <label>:132:                                    ; preds = %130
  %133 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %1, i64 0, i32 1, i32 2, i32 0, i32 0, i32 0, i32 3
  %134 = load %"class.std::unique_ptr"**, %"class.std::unique_ptr"*** %120, align 8, !tbaa !213
  %135 = getelementptr inbounds %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %134, i64 1
  %136 = bitcast %"class.std::unique_ptr"** %135 to i8**
  store i8* %131, i8** %136, align 8, !tbaa !27
  %137 = bitcast %"struct.std::_Deque_iterator.41"* %133 to i64**
  %138 = load i64*, i64** %137, align 8, !tbaa !205
  store i64 %97, i64* %138, align 8, !tbaa !210
  %139 = load %"class.std::unique_ptr"**, %"class.std::unique_ptr"*** %120, align 8, !tbaa !213
  %140 = getelementptr inbounds %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %139, i64 1
  store %"class.std::unique_ptr"** %140, %"class.std::unique_ptr"*** %120, align 8, !tbaa !215
  %141 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %140, align 8, !tbaa !27
  %142 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %1, i64 0, i32 1, i32 2, i32 0, i32 0, i32 0, i32 3, i32 1
  store %"class.std::unique_ptr"* %141, %"class.std::unique_ptr"** %142, align 8, !tbaa !216
  %143 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %141, i64 64
  store %"class.std::unique_ptr"* %143, %"class.std::unique_ptr"** %110, align 8, !tbaa !217
  %144 = ptrtoint %"class.std::unique_ptr"* %141 to i64
  %145 = bitcast %"struct.std::_Deque_iterator.41"* %133 to i64*
  store i64 %144, i64* %145, align 8, !tbaa !205
  br label %146

; <label>:146:                                    ; preds = %132, %114
  %147 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %1, i64 0, i32 1, i32 3
  call void @_ZNSt18condition_variable10notify_oneEv(%"class.std::condition_variable"* %147) #8
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %148, label %156

; <label>:148:                                    ; preds = %146
  %149 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %98, i64 0, i32 0, i32 0
  %150 = call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %149) #8
  br label %156

; <label>:151:                                    ; preds = %130, %129
  %152 = landingpad { i8*, i32 }
          cleanup
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %153, label %171

; <label>:153:                                    ; preds = %151
  %154 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %98, i64 0, i32 0, i32 0
  %155 = call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %154) #8
  br label %171

; <label>:156:                                    ; preds = %148, %146
  %157 = load <2 x i64>, <2 x i64>* %10, align 16, !tbaa !27
  %158 = bitcast %"class.MARC::TaskFuture"* %0 to <2 x i64>*
  store <2 x i64> %157, <2 x i64>* %158, align 8, !tbaa !27
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %87) #8
  call void @_ZNSt13packaged_taskIFvvEED2Ev(%"class.std::packaged_task"* nonnull %9) #8
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %20) #8
  ret void

; <label>:159:                                    ; preds = %69
  %160 = landingpad { i8*, i32 }
          cleanup
  %161 = extractvalue { i8*, i32 } %160, 0
  %162 = extractvalue { i8*, i32 } %160, 1
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %88) #8
  br label %225

; <label>:163:                                    ; preds = %89
  %164 = landingpad { i8*, i32 }
          cleanup
  %165 = extractelement <2 x i64> %90, i32 0
  %166 = inttoptr i64 %165 to %"class.std::__future_base::_State_baseV2"*
  %167 = extractvalue { i8*, i32 } %164, 0
  %168 = extractvalue { i8*, i32 } %164, 1
  br label %182

; <label>:169:                                    ; preds = %103
  %170 = landingpad { i8*, i32 }
          cleanup
  br label %171

; <label>:171:                                    ; preds = %151, %153, %169
  %172 = phi { i8*, i32 } [ %170, %169 ], [ %152, %153 ], [ %152, %151 ]
  %173 = extractvalue { i8*, i32 } %172, 0
  %174 = extractvalue { i8*, i32 } %172, 1
  %175 = bitcast i8* %91 to %"class.MARC::IThreadTask"*
  %176 = bitcast i8* %91 to void (%"class.MARC::IThreadTask"*)***
  %177 = load void (%"class.MARC::IThreadTask"*)**, void (%"class.MARC::IThreadTask"*)*** %176, align 8, !tbaa !133
  %178 = getelementptr inbounds void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %177, i64 2
  %179 = load void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %178, align 8
  call void %179(%"class.MARC::IThreadTask"* nonnull %175) #8
  %180 = bitcast <2 x i64>* %10 to %"class.std::__future_base::_State_baseV2"**
  %181 = load %"class.std::__future_base::_State_baseV2"*, %"class.std::__future_base::_State_baseV2"** %180, align 16, !tbaa !131
  br label %182

; <label>:182:                                    ; preds = %171, %163
  %183 = phi %"class.std::__future_base::_State_baseV2"* [ %166, %163 ], [ %181, %171 ]
  %184 = phi i8* [ %167, %163 ], [ %173, %171 ]
  %185 = phi i32 [ %168, %163 ], [ %174, %171 ]
  %186 = bitcast <2 x i64>* %10 to %"class.std::__basic_future"*
  %187 = icmp eq %"class.std::__future_base::_State_baseV2"* %183, null
  br i1 %187, label %193, label %188

; <label>:188:                                    ; preds = %182
  %189 = bitcast <2 x i64>* %10 to %"class.std::future"*
  invoke void @_ZNSt6futureIvE3getEv(%"class.std::future"* nonnull %189)
          to label %193 unwind label %190

; <label>:190:                                    ; preds = %188
  %191 = landingpad { i8*, i32 }
          catch i8* null
  %192 = extractvalue { i8*, i32 } %191, 0
  call void @_ZNSt14__basic_futureIvED2Ev(%"class.std::__basic_future"* nonnull %186) #8
  call void @__clang_call_terminate(i8* %192) #22
  unreachable

; <label>:193:                                    ; preds = %188, %182
  %194 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %11, i64 0, i32 0, i32 0, i32 0, i32 0, i32 1, i32 0
  %195 = load %"class.std::_Sp_counted_base"*, %"class.std::_Sp_counted_base"** %194, align 8, !tbaa !129
  %196 = icmp eq %"class.std::_Sp_counted_base"* %195, null
  br i1 %196, label %225, label %197

; <label>:197:                                    ; preds = %193
  %198 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %195, i64 0, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %199, label %201

; <label>:199:                                    ; preds = %197
  %200 = atomicrmw volatile add i32* %198, i32 -1 acq_rel
  br label %204

; <label>:201:                                    ; preds = %197
  %202 = load i32, i32* %198, align 4, !tbaa !17
  %203 = add nsw i32 %202, -1
  store i32 %203, i32* %198, align 4, !tbaa !17
  br label %204

; <label>:204:                                    ; preds = %201, %199
  %205 = phi i32 [ %200, %199 ], [ %202, %201 ]
  %206 = icmp eq i32 %205, 1
  br i1 %206, label %207, label %225

; <label>:207:                                    ; preds = %204
  %208 = bitcast %"class.std::_Sp_counted_base"* %195 to void (%"class.std::_Sp_counted_base"*)***
  %209 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %208, align 8, !tbaa !133
  %210 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %209, i64 2
  %211 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %210, align 8
  call void %211(%"class.std::_Sp_counted_base"* nonnull %195) #8
  %212 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %195, i64 0, i32 2
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %213, label %215

; <label>:213:                                    ; preds = %207
  %214 = atomicrmw volatile add i32* %212, i32 -1 acq_rel
  br label %218

; <label>:215:                                    ; preds = %207
  %216 = load i32, i32* %212, align 4, !tbaa !17
  %217 = add nsw i32 %216, -1
  store i32 %217, i32* %212, align 4, !tbaa !17
  br label %218

; <label>:218:                                    ; preds = %215, %213
  %219 = phi i32 [ %214, %213 ], [ %216, %215 ]
  %220 = icmp eq i32 %219, 1
  br i1 %220, label %221, label %225

; <label>:221:                                    ; preds = %218
  %222 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %208, align 8, !tbaa !133
  %223 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %222, i64 3
  %224 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %223, align 8
  call void %224(%"class.std::_Sp_counted_base"* nonnull %195) #8
  br label %225

; <label>:225:                                    ; preds = %221, %218, %204, %193, %159
  %226 = phi i8* [ %161, %159 ], [ %184, %193 ], [ %184, %204 ], [ %184, %218 ], [ %184, %221 ]
  %227 = phi i32 [ %162, %159 ], [ %185, %193 ], [ %185, %204 ], [ %185, %218 ], [ %185, %221 ]
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %87) #8
  call void @_ZNSt13packaged_taskIFvvEED2Ev(%"class.std::packaged_task"* nonnull %9) #8
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %20) #8
  %228 = insertvalue { i8*, i32 } undef, i8* %226, 0
  %229 = insertvalue { i8*, i32 } %228, i32 %227, 1
  resume { i8*, i32 } %229
}

; Function Attrs: uwtable
define linkonce_odr void @_ZNSt6vectorIN4MARC10TaskFutureIvEESaIS2_EE19_M_emplace_back_auxIJS2_EEEvDpOT_(%"class.std::vector.53"*, %"class.MARC::TaskFuture"* dereferenceable(16)) local_unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %3 = getelementptr inbounds %"class.std::vector.53", %"class.std::vector.53"* %0, i64 0, i32 0, i32 0, i32 1
  %4 = bitcast %"class.MARC::TaskFuture"** %3 to i64*
  %5 = load i64, i64* %4, align 8, !tbaa !125
  %6 = bitcast %"class.std::vector.53"* %0 to i64*
  %7 = load i64, i64* %6, align 8, !tbaa !218
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
  %26 = load i64, i64* %6, align 8, !tbaa !218
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
  br i1 %43, label %166, label %44

; <label>:44:                                     ; preds = %27
  %45 = getelementptr %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %32, i64 -1, i32 0, i32 0, i32 0, i32 0, i32 0
  %46 = bitcast %"class.std::__future_base::_State_baseV2"** %45 to i8*
  %47 = sub i64 0, %29
  %48 = getelementptr i8, i8* %46, i64 %47
  %49 = ptrtoint i8* %48 to i64
  %50 = lshr i64 %49, 4
  %51 = add nuw nsw i64 %50, 1
  %52 = and i64 %51, 3
  %53 = icmp eq i64 %52, 0
  br i1 %53, label %.loopexit1, label %54

; <label>:54:                                     ; preds = %44
  %scevgep = getelementptr %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %31, i64 %52
  br label %55

; <label>:55:                                     ; preds = %55, %54
  %56 = phi %"class.MARC::TaskFuture"* [ %30, %54 ], [ %67, %55 ]
  %57 = phi %"class.MARC::TaskFuture"* [ %31, %54 ], [ %66, %55 ]
  %58 = phi i64 [ %52, %54 ], [ %68, %55 ]
  %59 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %57, i64 0, i32 0, i32 0, i32 0, i32 0, i32 0
  %60 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %56, i64 0, i32 0, i32 0, i32 0, i32 0, i32 1, i32 0
  store %"class.std::_Sp_counted_base"* null, %"class.std::_Sp_counted_base"** %60, align 8, !tbaa !129
  %61 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %57, i64 0, i32 0, i32 0, i32 0, i32 0, i32 1
  %62 = bitcast %"class.std::__shared_count"* %61 to i64*
  %63 = bitcast %"class.MARC::TaskFuture"* %57 to <2 x i64>*
  %64 = load <2 x i64>, <2 x i64>* %63, align 8, !tbaa !27
  store i64 0, i64* %62, align 8, !tbaa !129
  %65 = bitcast %"class.MARC::TaskFuture"* %56 to <2 x i64>*
  store <2 x i64> %64, <2 x i64>* %65, align 8, !tbaa !27
  store %"class.std::__future_base::_State_baseV2"* null, %"class.std::__future_base::_State_baseV2"** %59, align 8, !tbaa !131
  %66 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %57, i64 1
  %67 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %56, i64 1
  %68 = add nsw i64 %58, -1
  %69 = icmp eq i64 %68, 0
  br i1 %69, label %.loopexit1.loopexit, label %55, !llvm.loop !219

.loopexit1.loopexit:                              ; preds = %55
  %scevgep7 = getelementptr %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %30, i64 %52
  br label %.loopexit1

.loopexit1:                                       ; preds = %.loopexit1.loopexit, %44
  %70 = phi %"class.MARC::TaskFuture"* [ %30, %44 ], [ %scevgep7, %.loopexit1.loopexit ]
  %71 = phi %"class.MARC::TaskFuture"* [ %31, %44 ], [ %scevgep, %.loopexit1.loopexit ]
  %72 = icmp ult i8* %48, inttoptr (i64 48 to i8*)
  br i1 %72, label %.loopexit, label %.preheader

.preheader:                                       ; preds = %.loopexit1
  br label %73

; <label>:73:                                     ; preds = %.preheader, %73
  %74 = phi %"class.MARC::TaskFuture"* [ %111, %73 ], [ %70, %.preheader ]
  %75 = phi %"class.MARC::TaskFuture"* [ %110, %73 ], [ %71, %.preheader ]
  %76 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %75, i64 0, i32 0, i32 0, i32 0, i32 0, i32 0
  %77 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %74, i64 0, i32 0, i32 0, i32 0, i32 0, i32 1, i32 0
  store %"class.std::_Sp_counted_base"* null, %"class.std::_Sp_counted_base"** %77, align 8, !tbaa !129
  %78 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %75, i64 0, i32 0, i32 0, i32 0, i32 0, i32 1
  %79 = bitcast %"class.std::__shared_count"* %78 to i64*
  %80 = bitcast %"class.MARC::TaskFuture"* %75 to <2 x i64>*
  %81 = load <2 x i64>, <2 x i64>* %80, align 8, !tbaa !27
  store i64 0, i64* %79, align 8, !tbaa !129
  %82 = bitcast %"class.MARC::TaskFuture"* %74 to <2 x i64>*
  store <2 x i64> %81, <2 x i64>* %82, align 8, !tbaa !27
  store %"class.std::__future_base::_State_baseV2"* null, %"class.std::__future_base::_State_baseV2"** %76, align 8, !tbaa !131
  %83 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %75, i64 1
  %84 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %74, i64 1
  %85 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %83, i64 0, i32 0, i32 0, i32 0, i32 0, i32 0
  %86 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %74, i64 1, i32 0, i32 0, i32 0, i32 0, i32 1, i32 0
  store %"class.std::_Sp_counted_base"* null, %"class.std::_Sp_counted_base"** %86, align 8, !tbaa !129
  %87 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %75, i64 1, i32 0, i32 0, i32 0, i32 0, i32 1
  %88 = bitcast %"class.std::__shared_count"* %87 to i64*
  %89 = bitcast %"class.MARC::TaskFuture"* %83 to <2 x i64>*
  %90 = load <2 x i64>, <2 x i64>* %89, align 8, !tbaa !27
  store i64 0, i64* %88, align 8, !tbaa !129
  %91 = bitcast %"class.MARC::TaskFuture"* %84 to <2 x i64>*
  store <2 x i64> %90, <2 x i64>* %91, align 8, !tbaa !27
  store %"class.std::__future_base::_State_baseV2"* null, %"class.std::__future_base::_State_baseV2"** %85, align 8, !tbaa !131
  %92 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %75, i64 2
  %93 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %74, i64 2
  %94 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %92, i64 0, i32 0, i32 0, i32 0, i32 0, i32 0
  %95 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %74, i64 2, i32 0, i32 0, i32 0, i32 0, i32 1, i32 0
  store %"class.std::_Sp_counted_base"* null, %"class.std::_Sp_counted_base"** %95, align 8, !tbaa !129
  %96 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %75, i64 2, i32 0, i32 0, i32 0, i32 0, i32 1
  %97 = bitcast %"class.std::__shared_count"* %96 to i64*
  %98 = bitcast %"class.MARC::TaskFuture"* %92 to <2 x i64>*
  %99 = load <2 x i64>, <2 x i64>* %98, align 8, !tbaa !27
  store i64 0, i64* %97, align 8, !tbaa !129
  %100 = bitcast %"class.MARC::TaskFuture"* %93 to <2 x i64>*
  store <2 x i64> %99, <2 x i64>* %100, align 8, !tbaa !27
  store %"class.std::__future_base::_State_baseV2"* null, %"class.std::__future_base::_State_baseV2"** %94, align 8, !tbaa !131
  %101 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %75, i64 3
  %102 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %74, i64 3
  %103 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %101, i64 0, i32 0, i32 0, i32 0, i32 0, i32 0
  %104 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %74, i64 3, i32 0, i32 0, i32 0, i32 0, i32 1, i32 0
  store %"class.std::_Sp_counted_base"* null, %"class.std::_Sp_counted_base"** %104, align 8, !tbaa !129
  %105 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %75, i64 3, i32 0, i32 0, i32 0, i32 0, i32 1
  %106 = bitcast %"class.std::__shared_count"* %105 to i64*
  %107 = bitcast %"class.MARC::TaskFuture"* %101 to <2 x i64>*
  %108 = load <2 x i64>, <2 x i64>* %107, align 8, !tbaa !27
  store i64 0, i64* %106, align 8, !tbaa !129
  %109 = bitcast %"class.MARC::TaskFuture"* %102 to <2 x i64>*
  store <2 x i64> %108, <2 x i64>* %109, align 8, !tbaa !27
  store %"class.std::__future_base::_State_baseV2"* null, %"class.std::__future_base::_State_baseV2"** %103, align 8, !tbaa !131
  %110 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %75, i64 4
  %111 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %74, i64 4
  %112 = icmp eq %"class.MARC::TaskFuture"* %110, %32
  br i1 %112, label %.loopexit, label %73

.loopexit:                                        ; preds = %73, %.loopexit1
  %113 = ptrtoint %"class.std::__future_base::_State_baseV2"** %45 to i64
  %114 = sub i64 %113, %29
  %115 = lshr i64 %114, 4
  %116 = add nuw nsw i64 %115, 1
  %117 = getelementptr %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %30, i64 %116
  br label %118

; <label>:118:                                    ; preds = %161, %.loopexit
  %119 = phi %"class.MARC::TaskFuture"* [ %31, %.loopexit ], [ %162, %161 ]
  %120 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %119, i64 0, i32 0, i32 0, i32 0, i32 0, i32 0
  %121 = load %"class.std::__future_base::_State_baseV2"*, %"class.std::__future_base::_State_baseV2"** %120, align 8, !tbaa !131
  %122 = icmp eq %"class.std::__future_base::_State_baseV2"* %121, null
  br i1 %122, label %129, label %123

; <label>:123:                                    ; preds = %118
  %124 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %119, i64 0, i32 0
  invoke void @_ZNSt6futureIvE3getEv(%"class.std::future"* %124)
          to label %129 unwind label %125

; <label>:125:                                    ; preds = %123
  %126 = landingpad { i8*, i32 }
          catch i8* null
  %127 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %119, i64 0, i32 0, i32 0
  %128 = extractvalue { i8*, i32 } %126, 0
  tail call void @_ZNSt14__basic_futureIvED2Ev(%"class.std::__basic_future"* %127) #8
  tail call void @__clang_call_terminate(i8* %128) #22
  unreachable

; <label>:129:                                    ; preds = %123, %118
  %130 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %119, i64 0, i32 0, i32 0, i32 0, i32 0, i32 1, i32 0
  %131 = load %"class.std::_Sp_counted_base"*, %"class.std::_Sp_counted_base"** %130, align 8, !tbaa !129
  %132 = icmp eq %"class.std::_Sp_counted_base"* %131, null
  br i1 %132, label %161, label %133

; <label>:133:                                    ; preds = %129
  %134 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %131, i64 0, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %135, label %137

; <label>:135:                                    ; preds = %133
  %136 = atomicrmw volatile add i32* %134, i32 -1 acq_rel
  br label %140

; <label>:137:                                    ; preds = %133
  %138 = load i32, i32* %134, align 4, !tbaa !17
  %139 = add nsw i32 %138, -1
  store i32 %139, i32* %134, align 4, !tbaa !17
  br label %140

; <label>:140:                                    ; preds = %137, %135
  %141 = phi i32 [ %136, %135 ], [ %138, %137 ]
  %142 = icmp eq i32 %141, 1
  br i1 %142, label %143, label %161

; <label>:143:                                    ; preds = %140
  %144 = bitcast %"class.std::_Sp_counted_base"* %131 to void (%"class.std::_Sp_counted_base"*)***
  %145 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %144, align 8, !tbaa !133
  %146 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %145, i64 2
  %147 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %146, align 8
  tail call void %147(%"class.std::_Sp_counted_base"* nonnull %131) #8
  %148 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %131, i64 0, i32 2
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %149, label %151

; <label>:149:                                    ; preds = %143
  %150 = atomicrmw volatile add i32* %148, i32 -1 acq_rel
  br label %154

; <label>:151:                                    ; preds = %143
  %152 = load i32, i32* %148, align 4, !tbaa !17
  %153 = add nsw i32 %152, -1
  store i32 %153, i32* %148, align 4, !tbaa !17
  br label %154

; <label>:154:                                    ; preds = %151, %149
  %155 = phi i32 [ %150, %149 ], [ %152, %151 ]
  %156 = icmp eq i32 %155, 1
  br i1 %156, label %157, label %161

; <label>:157:                                    ; preds = %154
  %158 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %144, align 8, !tbaa !133
  %159 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %158, i64 3
  %160 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %159, align 8
  tail call void %160(%"class.std::_Sp_counted_base"* nonnull %131) #8
  br label %161

; <label>:161:                                    ; preds = %157, %154, %140, %129
  %162 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %119, i64 1
  %163 = icmp eq %"class.MARC::TaskFuture"* %162, %32
  br i1 %163, label %164, label %118

; <label>:164:                                    ; preds = %161
  %165 = load %"class.MARC::TaskFuture"*, %"class.MARC::TaskFuture"** %42, align 8, !tbaa !218
  br label %166

; <label>:166:                                    ; preds = %27, %164
  %167 = phi %"class.MARC::TaskFuture"* [ %165, %164 ], [ %31, %27 ]
  %168 = phi %"class.MARC::TaskFuture"* [ %117, %164 ], [ %30, %27 ]
  %169 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %168, i64 1
  %170 = getelementptr inbounds %"class.std::vector.53", %"class.std::vector.53"* %0, i64 0, i32 0, i32 0, i32 2
  %171 = icmp eq %"class.MARC::TaskFuture"* %167, null
  br i1 %171, label %174, label %172

; <label>:172:                                    ; preds = %166
  %173 = bitcast %"class.MARC::TaskFuture"* %167 to i8*
  tail call void @_ZdlPv(i8* %173) #8
  br label %174

; <label>:174:                                    ; preds = %166, %172
  store %"class.MARC::TaskFuture"* %30, %"class.MARC::TaskFuture"** %42, align 8, !tbaa !218
  store %"class.MARC::TaskFuture"* %169, %"class.MARC::TaskFuture"** %3, align 8, !tbaa !125
  %175 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %30, i64 %16
  store %"class.MARC::TaskFuture"* %175, %"class.MARC::TaskFuture"** %170, align 8, !tbaa !128
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
  %3 = load %"class.MARC::TaskFuture"*, %"class.MARC::TaskFuture"** %2, align 8, !tbaa !218
  %4 = getelementptr inbounds %"class.std::vector.53", %"class.std::vector.53"* %0, i64 0, i32 0, i32 0, i32 1
  %5 = load %"class.MARC::TaskFuture"*, %"class.MARC::TaskFuture"** %4, align 8, !tbaa !125
  %6 = icmp eq %"class.MARC::TaskFuture"* %3, %5
  br i1 %6, label %55, label %.preheader

.preheader:                                       ; preds = %1
  br label %7

; <label>:7:                                      ; preds = %.preheader, %50
  %8 = phi %"class.MARC::TaskFuture"* [ %51, %50 ], [ %3, %.preheader ]
  %9 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %8, i64 0, i32 0, i32 0, i32 0, i32 0, i32 0
  %10 = load %"class.std::__future_base::_State_baseV2"*, %"class.std::__future_base::_State_baseV2"** %9, align 8, !tbaa !131
  %11 = icmp eq %"class.std::__future_base::_State_baseV2"* %10, null
  br i1 %11, label %18, label %12

; <label>:12:                                     ; preds = %7
  %13 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %8, i64 0, i32 0
  invoke void @_ZNSt6futureIvE3getEv(%"class.std::future"* %13)
          to label %18 unwind label %14

; <label>:14:                                     ; preds = %12
  %15 = landingpad { i8*, i32 }
          catch i8* null
  %16 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %8, i64 0, i32 0, i32 0
  %17 = extractvalue { i8*, i32 } %15, 0
  tail call void @_ZNSt14__basic_futureIvED2Ev(%"class.std::__basic_future"* %16) #8
  tail call void @__clang_call_terminate(i8* %17) #22
  unreachable

; <label>:18:                                     ; preds = %12, %7
  %19 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %8, i64 0, i32 0, i32 0, i32 0, i32 0, i32 1, i32 0
  %20 = load %"class.std::_Sp_counted_base"*, %"class.std::_Sp_counted_base"** %19, align 8, !tbaa !129
  %21 = icmp eq %"class.std::_Sp_counted_base"* %20, null
  br i1 %21, label %50, label %22

; <label>:22:                                     ; preds = %18
  %23 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %20, i64 0, i32 1
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %24, label %26

; <label>:24:                                     ; preds = %22
  %25 = atomicrmw volatile add i32* %23, i32 -1 acq_rel
  br label %29

; <label>:26:                                     ; preds = %22
  %27 = load i32, i32* %23, align 4, !tbaa !17
  %28 = add nsw i32 %27, -1
  store i32 %28, i32* %23, align 4, !tbaa !17
  br label %29

; <label>:29:                                     ; preds = %26, %24
  %30 = phi i32 [ %25, %24 ], [ %27, %26 ]
  %31 = icmp eq i32 %30, 1
  br i1 %31, label %32, label %50

; <label>:32:                                     ; preds = %29
  %33 = bitcast %"class.std::_Sp_counted_base"* %20 to void (%"class.std::_Sp_counted_base"*)***
  %34 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %33, align 8, !tbaa !133
  %35 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %34, i64 2
  %36 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %35, align 8
  tail call void %36(%"class.std::_Sp_counted_base"* nonnull %20) #8
  %37 = getelementptr inbounds %"class.std::_Sp_counted_base", %"class.std::_Sp_counted_base"* %20, i64 0, i32 2
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %38, label %40

; <label>:38:                                     ; preds = %32
  %39 = atomicrmw volatile add i32* %37, i32 -1 acq_rel
  br label %43

; <label>:40:                                     ; preds = %32
  %41 = load i32, i32* %37, align 4, !tbaa !17
  %42 = add nsw i32 %41, -1
  store i32 %42, i32* %37, align 4, !tbaa !17
  br label %43

; <label>:43:                                     ; preds = %40, %38
  %44 = phi i32 [ %39, %38 ], [ %41, %40 ]
  %45 = icmp eq i32 %44, 1
  br i1 %45, label %46, label %50

; <label>:46:                                     ; preds = %43
  %47 = load void (%"class.std::_Sp_counted_base"*)**, void (%"class.std::_Sp_counted_base"*)*** %33, align 8, !tbaa !133
  %48 = getelementptr inbounds void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %47, i64 3
  %49 = load void (%"class.std::_Sp_counted_base"*)*, void (%"class.std::_Sp_counted_base"*)** %48, align 8
  tail call void %49(%"class.std::_Sp_counted_base"* nonnull %20) #8
  br label %50

; <label>:50:                                     ; preds = %46, %43, %29, %18
  %51 = getelementptr inbounds %"class.MARC::TaskFuture", %"class.MARC::TaskFuture"* %8, i64 1
  %52 = icmp eq %"class.MARC::TaskFuture"* %51, %5
  br i1 %52, label %53, label %7

; <label>:53:                                     ; preds = %50
  %54 = load %"class.MARC::TaskFuture"*, %"class.MARC::TaskFuture"** %2, align 8, !tbaa !218
  br label %55

; <label>:55:                                     ; preds = %53, %1
  %56 = phi %"class.MARC::TaskFuture"* [ %54, %53 ], [ %3, %1 ]
  %57 = icmp eq %"class.MARC::TaskFuture"* %56, null
  br i1 %57, label %60, label %58

; <label>:58:                                     ; preds = %55
  %59 = bitcast %"class.MARC::TaskFuture"* %56 to i8*
  tail call void @_ZdlPv(i8* %59) #8
  br label %60

; <label>:60:                                     ; preds = %55, %58
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN4MARC10ThreadPoolD2Ev(%"class.MARC::ThreadPool"*) unnamed_addr #10 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  invoke void @_ZN4MARC10ThreadPool7destroyEv(%"class.MARC::ThreadPool"* %0)
          to label %2 unwind label %23

; <label>:2:                                      ; preds = %1
  %3 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 4
  tail call void @_ZN4MARC15ThreadSafeQueueISt8functionIFvvEEED2Ev(%"class.MARC::ThreadSafeQueue.45"* %3) #8
  %4 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 2, i32 0, i32 0, i32 0
  %5 = load %"class.std::thread"*, %"class.std::thread"** %4, align 8, !tbaa !167
  %6 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 2, i32 0, i32 0, i32 1
  %7 = load %"class.std::thread"*, %"class.std::thread"** %6, align 8, !tbaa !151
  %8 = icmp eq %"class.std::thread"* %5, %7
  br i1 %8, label %.loopexit, label %.preheader

.preheader:                                       ; preds = %2
  br label %11

; <label>:9:                                      ; preds = %11
  %10 = icmp eq %"class.std::thread"* %16, %7
  br i1 %10, label %.loopexit, label %11

; <label>:11:                                     ; preds = %.preheader, %9
  %12 = phi %"class.std::thread"* [ %16, %9 ], [ %5, %.preheader ]
  %13 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %12, i64 0, i32 0, i32 0
  %14 = load i64, i64* %13, align 8
  %15 = icmp eq i64 %14, 0
  %16 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %12, i64 1
  br i1 %15, label %9, label %17

; <label>:17:                                     ; preds = %11
  tail call void @_ZSt9terminatev() #22
  unreachable

.loopexit:                                        ; preds = %9, %2
  %18 = icmp eq %"class.std::thread"* %5, null
  br i1 %18, label %21, label %19

; <label>:19:                                     ; preds = %.loopexit
  %20 = bitcast %"class.std::thread"* %5 to i8*
  tail call void @_ZdlPv(i8* %20) #8
  br label %21

; <label>:21:                                     ; preds = %.loopexit, %19
  %22 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 1
  tail call void @_ZN4MARC15ThreadSafeQueueISt10unique_ptrINS_11IThreadTaskESt14default_deleteIS2_EEED2Ev(%"class.MARC::ThreadSafeQueue.32"* %22) #8
  ret void

; <label>:23:                                     ; preds = %1
  %24 = landingpad { i8*, i32 }
          catch i8* null
  %25 = extractvalue { i8*, i32 } %24, 0
  %26 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 4
  tail call void @_ZN4MARC15ThreadSafeQueueISt8functionIFvvEEED2Ev(%"class.MARC::ThreadSafeQueue.45"* %26) #8
  %27 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 2
  tail call void @_ZNSt6vectorISt6threadSaIS0_EED2Ev(%"class.std::vector"* %27) #8
  %28 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 1
  tail call void @_ZN4MARC15ThreadSafeQueueISt10unique_ptrINS_11IThreadTaskESt14default_deleteIS2_EEED2Ev(%"class.MARC::ThreadSafeQueue.32"* %28) #8
  tail call void @__clang_call_terminate(i8* %25) #22
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
          to label %8 unwind label %47

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
  br i1 %27, label %46, label %28

; <label>:28:                                     ; preds = %21
  %29 = bitcast i8** %26 to i8*
  %30 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 3
  %31 = load i8**, i8*** %30, align 8, !tbaa !31
  %32 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 3
  %33 = load i8**, i8*** %32, align 8, !tbaa !25
  %34 = getelementptr inbounds i8*, i8** %33, i64 1
  %35 = icmp ult i8** %31, %34
  br i1 %35, label %.preheader, label %44

.preheader:                                       ; preds = %28
  br label %36

; <label>:36:                                     ; preds = %.preheader, %36
  %37 = phi i8** [ %39, %36 ], [ %31, %.preheader ]
  %38 = load i8*, i8** %37, align 8, !tbaa !27
  tail call void @_ZdlPv(i8* %38) #8
  %39 = getelementptr inbounds i8*, i8** %37, i64 1
  %40 = icmp ult i8** %37, %33
  br i1 %40, label %36, label %41

; <label>:41:                                     ; preds = %36
  %42 = bitcast %"class.std::queue"* %24 to i8**
  %43 = load i8*, i8** %42, align 8, !tbaa !26
  br label %44

; <label>:44:                                     ; preds = %41, %28
  %45 = phi i8* [ %43, %41 ], [ %29, %28 ]
  tail call void @_ZdlPv(i8* %45) #8
  br label %46

; <label>:46:                                     ; preds = %21, %44
  ret void

; <label>:47:                                     ; preds = %7
  %48 = landingpad { i8*, i32 }
          catch i8* null
  %49 = extractvalue { i8*, i32 } %48, 0
  %50 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 4
  tail call void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"* %50) #8
  %51 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 3
  tail call void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"* %51) #8
  %52 = getelementptr inbounds %"class.MARC::ThreadSafeQueue", %"class.MARC::ThreadSafeQueue"* %0, i64 0, i32 2
  tail call void @_ZNSt5queueIaSt5dequeIaSaIaEEED2Ev(%"class.std::queue"* %52) #8
  tail call void @__clang_call_terminate(i8* %49) #22
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
          to label %8 unwind label %48

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
  br i1 %27, label %47, label %28

; <label>:28:                                     ; preds = %21
  %29 = bitcast i16** %26 to i8*
  %30 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 3
  %31 = load i16**, i16*** %30, align 8, !tbaa !57
  %32 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 3
  %33 = load i16**, i16*** %32, align 8, !tbaa !52
  %34 = getelementptr inbounds i16*, i16** %33, i64 1
  %35 = icmp ult i16** %31, %34
  br i1 %35, label %.preheader, label %45

.preheader:                                       ; preds = %28
  br label %36

; <label>:36:                                     ; preds = %.preheader, %36
  %37 = phi i16** [ %40, %36 ], [ %31, %.preheader ]
  %38 = bitcast i16** %37 to i8**
  %39 = load i8*, i8** %38, align 8, !tbaa !27
  tail call void @_ZdlPv(i8* %39) #8
  %40 = getelementptr inbounds i16*, i16** %37, i64 1
  %41 = icmp ult i16** %37, %33
  br i1 %41, label %36, label %42

; <label>:42:                                     ; preds = %36
  %43 = bitcast %"class.std::queue.4"* %24 to i8**
  %44 = load i8*, i8** %43, align 8, !tbaa !53
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
  %51 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 4
  tail call void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"* %51) #8
  %52 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 3
  tail call void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"* %52) #8
  %53 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.3", %"class.MARC::ThreadSafeQueue.3"* %0, i64 0, i32 2
  tail call void @_ZNSt5queueIsSt5dequeIsSaIsEEED2Ev(%"class.std::queue.4"* %53) #8
  tail call void @__clang_call_terminate(i8* %50) #22
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
          to label %8 unwind label %48

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
  br i1 %27, label %47, label %28

; <label>:28:                                     ; preds = %21
  %29 = bitcast i32** %26 to i8*
  %30 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 3
  %31 = load i32**, i32*** %30, align 8, !tbaa !76
  %32 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 3
  %33 = load i32**, i32*** %32, align 8, !tbaa !71
  %34 = getelementptr inbounds i32*, i32** %33, i64 1
  %35 = icmp ult i32** %31, %34
  br i1 %35, label %.preheader, label %45

.preheader:                                       ; preds = %28
  br label %36

; <label>:36:                                     ; preds = %.preheader, %36
  %37 = phi i32** [ %40, %36 ], [ %31, %.preheader ]
  %38 = bitcast i32** %37 to i8**
  %39 = load i8*, i8** %38, align 8, !tbaa !27
  tail call void @_ZdlPv(i8* %39) #8
  %40 = getelementptr inbounds i32*, i32** %37, i64 1
  %41 = icmp ult i32** %37, %33
  br i1 %41, label %36, label %42

; <label>:42:                                     ; preds = %36
  %43 = bitcast %"class.std::queue.12"* %24 to i8**
  %44 = load i8*, i8** %43, align 8, !tbaa !72
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
  %51 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 4
  tail call void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"* %51) #8
  %52 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 3
  tail call void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"* %52) #8
  %53 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.11", %"class.MARC::ThreadSafeQueue.11"* %0, i64 0, i32 2
  tail call void @_ZNSt5queueIiSt5dequeIiSaIiEEED2Ev(%"class.std::queue.12"* %53) #8
  tail call void @__clang_call_terminate(i8* %50) #22
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
          to label %8 unwind label %48

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
  br i1 %27, label %47, label %28

; <label>:28:                                     ; preds = %21
  %29 = bitcast i64** %26 to i8*
  %30 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 3
  %31 = load i64**, i64*** %30, align 8, !tbaa !96
  %32 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 3
  %33 = load i64**, i64*** %32, align 8, !tbaa !91
  %34 = getelementptr inbounds i64*, i64** %33, i64 1
  %35 = icmp ult i64** %31, %34
  br i1 %35, label %.preheader, label %45

.preheader:                                       ; preds = %28
  br label %36

; <label>:36:                                     ; preds = %.preheader, %36
  %37 = phi i64** [ %40, %36 ], [ %31, %.preheader ]
  %38 = bitcast i64** %37 to i8**
  %39 = load i8*, i8** %38, align 8, !tbaa !27
  tail call void @_ZdlPv(i8* %39) #8
  %40 = getelementptr inbounds i64*, i64** %37, i64 1
  %41 = icmp ult i64** %37, %33
  br i1 %41, label %36, label %42

; <label>:42:                                     ; preds = %36
  %43 = bitcast %"class.std::queue.20"* %24 to i8**
  %44 = load i8*, i8** %43, align 8, !tbaa !92
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
  %51 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 4
  tail call void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"* %51) #8
  %52 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 3
  tail call void @_ZNSt18condition_variableD1Ev(%"class.std::condition_variable"* %52) #8
  %53 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.19", %"class.MARC::ThreadSafeQueue.19"* %0, i64 0, i32 2
  tail call void @_ZNSt5queueIlSt5dequeIlSaIlEEED2Ev(%"class.std::queue.20"* %53) #8
  tail call void @__clang_call_terminate(i8* %50) #22
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
  br i1 %4, label %24, label %5

; <label>:5:                                      ; preds = %1
  %6 = bitcast i64** %3 to i8*
  %7 = getelementptr inbounds %"class.std::queue.20", %"class.std::queue.20"* %0, i64 0, i32 0, i32 0, i32 0, i32 2, i32 3
  %8 = load i64**, i64*** %7, align 8, !tbaa !96
  %9 = getelementptr inbounds %"class.std::queue.20", %"class.std::queue.20"* %0, i64 0, i32 0, i32 0, i32 0, i32 3, i32 3
  %10 = load i64**, i64*** %9, align 8, !tbaa !91
  %11 = getelementptr inbounds i64*, i64** %10, i64 1
  %12 = icmp ult i64** %8, %11
  br i1 %12, label %.preheader, label %22

.preheader:                                       ; preds = %5
  br label %13

; <label>:13:                                     ; preds = %.preheader, %13
  %14 = phi i64** [ %17, %13 ], [ %8, %.preheader ]
  %15 = bitcast i64** %14 to i8**
  %16 = load i8*, i8** %15, align 8, !tbaa !27
  tail call void @_ZdlPv(i8* %16) #8
  %17 = getelementptr inbounds i64*, i64** %14, i64 1
  %18 = icmp ult i64** %14, %10
  br i1 %18, label %13, label %19

; <label>:19:                                     ; preds = %13
  %20 = bitcast %"class.std::queue.20"* %0 to i8**
  %21 = load i8*, i8** %20, align 8, !tbaa !92
  br label %22

; <label>:22:                                     ; preds = %19, %5
  %23 = phi i8* [ %21, %19 ], [ %6, %5 ]
  tail call void @_ZdlPv(i8* %23) #8
  br label %24

; <label>:24:                                     ; preds = %1, %22
  ret void
}

; Function Attrs: inlinehint nounwind uwtable
define linkonce_odr void @_ZNSt5queueIiSt5dequeIiSaIiEEED2Ev(%"class.std::queue.12"*) unnamed_addr #17 comdat align 2 personality i32 (...)* @__gxx_personality_v0 {
  %2 = getelementptr inbounds %"class.std::queue.12", %"class.std::queue.12"* %0, i64 0, i32 0, i32 0, i32 0, i32 0
  %3 = load i32**, i32*** %2, align 8, !tbaa !72
  %4 = icmp eq i32** %3, null
  br i1 %4, label %24, label %5

; <label>:5:                                      ; preds = %1
  %6 = bitcast i32** %3 to i8*
  %7 = getelementptr inbounds %"class.std::queue.12", %"class.std::queue.12"* %0, i64 0, i32 0, i32 0, i32 0, i32 2, i32 3
  %8 = load i32**, i32*** %7, align 8, !tbaa !76
  %9 = getelementptr inbounds %"class.std::queue.12", %"class.std::queue.12"* %0, i64 0, i32 0, i32 0, i32 0, i32 3, i32 3
  %10 = load i32**, i32*** %9, align 8, !tbaa !71
  %11 = getelementptr inbounds i32*, i32** %10, i64 1
  %12 = icmp ult i32** %8, %11
  br i1 %12, label %.preheader, label %22

.preheader:                                       ; preds = %5
  br label %13

; <label>:13:                                     ; preds = %.preheader, %13
  %14 = phi i32** [ %17, %13 ], [ %8, %.preheader ]
  %15 = bitcast i32** %14 to i8**
  %16 = load i8*, i8** %15, align 8, !tbaa !27
  tail call void @_ZdlPv(i8* %16) #8
  %17 = getelementptr inbounds i32*, i32** %14, i64 1
  %18 = icmp ult i32** %14, %10
  br i1 %18, label %13, label %19

; <label>:19:                                     ; preds = %13
  %20 = bitcast %"class.std::queue.12"* %0 to i8**
  %21 = load i8*, i8** %20, align 8, !tbaa !72
  br label %22

; <label>:22:                                     ; preds = %19, %5
  %23 = phi i8* [ %21, %19 ], [ %6, %5 ]
  tail call void @_ZdlPv(i8* %23) #8
  br label %24

; <label>:24:                                     ; preds = %1, %22
  ret void
}

; Function Attrs: inlinehint nounwind uwtable
define linkonce_odr void @_ZNSt5queueIsSt5dequeIsSaIsEEED2Ev(%"class.std::queue.4"*) unnamed_addr #17 comdat align 2 personality i32 (...)* @__gxx_personality_v0 {
  %2 = getelementptr inbounds %"class.std::queue.4", %"class.std::queue.4"* %0, i64 0, i32 0, i32 0, i32 0, i32 0
  %3 = load i16**, i16*** %2, align 8, !tbaa !53
  %4 = icmp eq i16** %3, null
  br i1 %4, label %24, label %5

; <label>:5:                                      ; preds = %1
  %6 = bitcast i16** %3 to i8*
  %7 = getelementptr inbounds %"class.std::queue.4", %"class.std::queue.4"* %0, i64 0, i32 0, i32 0, i32 0, i32 2, i32 3
  %8 = load i16**, i16*** %7, align 8, !tbaa !57
  %9 = getelementptr inbounds %"class.std::queue.4", %"class.std::queue.4"* %0, i64 0, i32 0, i32 0, i32 0, i32 3, i32 3
  %10 = load i16**, i16*** %9, align 8, !tbaa !52
  %11 = getelementptr inbounds i16*, i16** %10, i64 1
  %12 = icmp ult i16** %8, %11
  br i1 %12, label %.preheader, label %22

.preheader:                                       ; preds = %5
  br label %13

; <label>:13:                                     ; preds = %.preheader, %13
  %14 = phi i16** [ %17, %13 ], [ %8, %.preheader ]
  %15 = bitcast i16** %14 to i8**
  %16 = load i8*, i8** %15, align 8, !tbaa !27
  tail call void @_ZdlPv(i8* %16) #8
  %17 = getelementptr inbounds i16*, i16** %14, i64 1
  %18 = icmp ult i16** %14, %10
  br i1 %18, label %13, label %19

; <label>:19:                                     ; preds = %13
  %20 = bitcast %"class.std::queue.4"* %0 to i8**
  %21 = load i8*, i8** %20, align 8, !tbaa !53
  br label %22

; <label>:22:                                     ; preds = %19, %5
  %23 = phi i8* [ %21, %19 ], [ %6, %5 ]
  tail call void @_ZdlPv(i8* %23) #8
  br label %24

; <label>:24:                                     ; preds = %1, %22
  ret void
}

; Function Attrs: inlinehint nounwind uwtable
define linkonce_odr void @_ZNSt5queueIaSt5dequeIaSaIaEEED2Ev(%"class.std::queue"*) unnamed_addr #17 comdat align 2 personality i32 (...)* @__gxx_personality_v0 {
  %2 = getelementptr inbounds %"class.std::queue", %"class.std::queue"* %0, i64 0, i32 0, i32 0, i32 0, i32 0
  %3 = load i8**, i8*** %2, align 8, !tbaa !26
  %4 = icmp eq i8** %3, null
  br i1 %4, label %23, label %5

; <label>:5:                                      ; preds = %1
  %6 = bitcast i8** %3 to i8*
  %7 = getelementptr inbounds %"class.std::queue", %"class.std::queue"* %0, i64 0, i32 0, i32 0, i32 0, i32 2, i32 3
  %8 = load i8**, i8*** %7, align 8, !tbaa !31
  %9 = getelementptr inbounds %"class.std::queue", %"class.std::queue"* %0, i64 0, i32 0, i32 0, i32 0, i32 3, i32 3
  %10 = load i8**, i8*** %9, align 8, !tbaa !25
  %11 = getelementptr inbounds i8*, i8** %10, i64 1
  %12 = icmp ult i8** %8, %11
  br i1 %12, label %.preheader, label %21

.preheader:                                       ; preds = %5
  br label %13

; <label>:13:                                     ; preds = %.preheader, %13
  %14 = phi i8** [ %16, %13 ], [ %8, %.preheader ]
  %15 = load i8*, i8** %14, align 8, !tbaa !27
  tail call void @_ZdlPv(i8* %15) #8
  %16 = getelementptr inbounds i8*, i8** %14, i64 1
  %17 = icmp ult i8** %14, %10
  br i1 %17, label %13, label %18

; <label>:18:                                     ; preds = %13
  %19 = bitcast %"class.std::queue"* %0 to i8**
  %20 = load i8*, i8** %19, align 8, !tbaa !26
  br label %21

; <label>:21:                                     ; preds = %18, %5
  %22 = phi i8* [ %20, %18 ], [ %6, %5 ]
  tail call void @_ZdlPv(i8* %22) #8
  br label %23

; <label>:23:                                     ; preds = %1, %21
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
  %27 = load i64, i64* %8, align 8, !tbaa !220
  %28 = load i64, i64* %10, align 8, !tbaa !220
  %29 = sub i64 %27, %28
  %30 = shl i64 %29, 1
  %31 = add i64 %30, -16
  %32 = load i64, i64* %11, align 8, !tbaa !222
  %33 = load i64, i64* %13, align 8, !tbaa !223
  %34 = sub i64 %32, %33
  %35 = ashr exact i64 %34, 5
  %36 = add nsw i64 %31, %35
  %37 = load i64, i64* %15, align 8, !tbaa !224
  %38 = load i64, i64* %16, align 8, !tbaa !222
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
  %54 = load void (%"union.std::_Any_data"*)*, void (%"union.std::_Any_data"*)** %19, align 8, !tbaa !225
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
  br i1 %103, label %.loopexit, label %.preheader

.preheader:                                       ; preds = %98
  br label %107

.loopexit:                                        ; preds = %113, %98
  %104 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 3
  %105 = load %"struct.std::atomic"*, %"struct.std::atomic"** %104, align 8, !tbaa !137
  %106 = icmp eq %"struct.std::atomic"* %105, null
  br i1 %106, label %118, label %116

; <label>:107:                                    ; preds = %.preheader, %113
  %108 = phi %"class.std::thread"* [ %114, %113 ], [ %100, %.preheader ]
  %109 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %108, i64 0, i32 0, i32 0
  %110 = load i64, i64* %109, align 8
  %111 = icmp eq i64 %110, 0
  br i1 %111, label %113, label %112

; <label>:112:                                    ; preds = %107
  call void @_ZNSt6thread4joinEv(%"class.std::thread"* nonnull %108)
  br label %113

; <label>:113:                                    ; preds = %107, %112
  %114 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %108, i64 1
  %115 = icmp eq %"class.std::thread"* %114, %102
  br i1 %115, label %.loopexit, label %107

; <label>:116:                                    ; preds = %.loopexit
  %117 = getelementptr inbounds %"struct.std::atomic", %"struct.std::atomic"* %105, i64 0, i32 0, i32 0
  call void @_ZdaPv(i8* %117) #25
  br label %118

; <label>:118:                                    ; preds = %116, %.loopexit
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
  %3 = load %"class.std::thread"*, %"class.std::thread"** %2, align 8, !tbaa !167
  %4 = getelementptr inbounds %"class.std::vector", %"class.std::vector"* %0, i64 0, i32 0, i32 0, i32 1
  %5 = load %"class.std::thread"*, %"class.std::thread"** %4, align 8, !tbaa !151
  %6 = icmp eq %"class.std::thread"* %3, %5
  br i1 %6, label %.loopexit, label %.preheader

.preheader:                                       ; preds = %1
  br label %9

; <label>:7:                                      ; preds = %9
  %8 = icmp eq %"class.std::thread"* %14, %5
  br i1 %8, label %.loopexit, label %9

; <label>:9:                                      ; preds = %.preheader, %7
  %10 = phi %"class.std::thread"* [ %14, %7 ], [ %3, %.preheader ]
  %11 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %10, i64 0, i32 0, i32 0
  %12 = load i64, i64* %11, align 8
  %13 = icmp eq i64 %12, 0
  %14 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %10, i64 1
  br i1 %13, label %7, label %15

; <label>:15:                                     ; preds = %9
  tail call void @_ZSt9terminatev() #22
  unreachable

.loopexit:                                        ; preds = %7, %1
  %16 = icmp eq %"class.std::thread"* %3, null
  br i1 %16, label %19, label %17

; <label>:17:                                     ; preds = %.loopexit
  %18 = bitcast %"class.std::thread"* %3 to i8*
  tail call void @_ZdlPv(i8* %18) #8
  br label %19

; <label>:19:                                     ; preds = %.loopexit, %17
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZNSt5dequeISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EESaIS5_EED2Ev(%"class.std::deque.34"*) unnamed_addr #10 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %2 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %0, i64 0, i32 0, i32 0, i32 2, i32 0
  %3 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %2, align 8, !tbaa !227, !noalias !228
  %4 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %0, i64 0, i32 0, i32 0, i32 2, i32 2
  %5 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %4, align 8, !tbaa !217, !noalias !228
  %6 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %0, i64 0, i32 0, i32 0, i32 2, i32 3
  %7 = load %"class.std::unique_ptr"**, %"class.std::unique_ptr"*** %6, align 8, !tbaa !215, !noalias !228
  %8 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %0, i64 0, i32 0, i32 0, i32 3
  %9 = bitcast %"struct.std::_Deque_iterator.41"* %8 to i64*
  %10 = load i64, i64* %9, align 8, !tbaa !227, !noalias !231
  %11 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %0, i64 0, i32 0, i32 0, i32 3, i32 1
  %12 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %11, align 8, !tbaa !216, !noalias !231
  %13 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %0, i64 0, i32 0, i32 0, i32 3, i32 3
  %14 = load %"class.std::unique_ptr"**, %"class.std::unique_ptr"*** %13, align 8, !tbaa !215, !noalias !231
  %15 = getelementptr inbounds %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %7, i64 1
  %16 = icmp ult %"class.std::unique_ptr"** %15, %14
  br i1 %16, label %.preheader9, label %.loopexit3

.preheader9:                                      ; preds = %1
  br label %18

.loopexit3:                                       ; preds = %34, %1
  %17 = icmp eq %"class.std::unique_ptr"** %7, %14
  br i1 %17, label %67, label %37

; <label>:18:                                     ; preds = %.preheader9, %34
  %19 = phi %"class.std::unique_ptr"** [ %35, %34 ], [ %15, %.preheader9 ]
  %20 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %19, align 8, !tbaa !27
  br label %21

; <label>:21:                                     ; preds = %31, %18
  %22 = phi i64 [ 0, %18 ], [ %32, %31 ]
  %23 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %20, i64 %22, i32 0, i32 0, i32 0, i32 0
  %24 = load %"class.MARC::IThreadTask"*, %"class.MARC::IThreadTask"** %23, align 8, !tbaa !27
  %25 = icmp eq %"class.MARC::IThreadTask"* %24, null
  br i1 %25, label %31, label %26

; <label>:26:                                     ; preds = %21
  %27 = bitcast %"class.MARC::IThreadTask"* %24 to void (%"class.MARC::IThreadTask"*)***
  %28 = load void (%"class.MARC::IThreadTask"*)**, void (%"class.MARC::IThreadTask"*)*** %27, align 8, !tbaa !133
  %29 = getelementptr inbounds void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %28, i64 2
  %30 = load void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %29, align 8
  tail call void %30(%"class.MARC::IThreadTask"* nonnull %24) #8
  br label %31

; <label>:31:                                     ; preds = %26, %21
  store %"class.MARC::IThreadTask"* null, %"class.MARC::IThreadTask"** %23, align 8, !tbaa !27
  %32 = add nuw nsw i64 %22, 1
  %33 = icmp eq i64 %32, 64
  br i1 %33, label %34, label %21

; <label>:34:                                     ; preds = %31
  %35 = getelementptr inbounds %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %19, i64 1
  %36 = icmp ult %"class.std::unique_ptr"** %35, %14
  br i1 %36, label %18, label %.loopexit3

; <label>:37:                                     ; preds = %.loopexit3
  %38 = icmp eq %"class.std::unique_ptr"* %3, %5
  br i1 %38, label %.loopexit2, label %.preheader8

.preheader8:                                      ; preds = %37
  br label %39

; <label>:39:                                     ; preds = %.preheader8, %49
  %40 = phi %"class.std::unique_ptr"* [ %50, %49 ], [ %3, %.preheader8 ]
  %41 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %40, i64 0, i32 0, i32 0, i32 0, i32 0
  %42 = load %"class.MARC::IThreadTask"*, %"class.MARC::IThreadTask"** %41, align 8, !tbaa !27
  %43 = icmp eq %"class.MARC::IThreadTask"* %42, null
  br i1 %43, label %49, label %44

; <label>:44:                                     ; preds = %39
  %45 = bitcast %"class.MARC::IThreadTask"* %42 to void (%"class.MARC::IThreadTask"*)***
  %46 = load void (%"class.MARC::IThreadTask"*)**, void (%"class.MARC::IThreadTask"*)*** %45, align 8, !tbaa !133
  %47 = getelementptr inbounds void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %46, i64 2
  %48 = load void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %47, align 8
  tail call void %48(%"class.MARC::IThreadTask"* nonnull %42) #8
  br label %49

; <label>:49:                                     ; preds = %44, %39
  store %"class.MARC::IThreadTask"* null, %"class.MARC::IThreadTask"** %41, align 8, !tbaa !27
  %50 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %40, i64 1
  %51 = icmp eq %"class.std::unique_ptr"* %50, %5
  br i1 %51, label %.loopexit2, label %39

.loopexit2:                                       ; preds = %49, %37
  %52 = inttoptr i64 %10 to %"class.std::unique_ptr"*
  %53 = icmp eq %"class.std::unique_ptr"* %12, %52
  br i1 %53, label %.loopexit, label %.preheader6

.preheader6:                                      ; preds = %.loopexit2
  br label %54

; <label>:54:                                     ; preds = %.preheader6, %64
  %55 = phi %"class.std::unique_ptr"* [ %65, %64 ], [ %12, %.preheader6 ]
  %56 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %55, i64 0, i32 0, i32 0, i32 0, i32 0
  %57 = load %"class.MARC::IThreadTask"*, %"class.MARC::IThreadTask"** %56, align 8, !tbaa !27
  %58 = icmp eq %"class.MARC::IThreadTask"* %57, null
  br i1 %58, label %64, label %59

; <label>:59:                                     ; preds = %54
  %60 = bitcast %"class.MARC::IThreadTask"* %57 to void (%"class.MARC::IThreadTask"*)***
  %61 = load void (%"class.MARC::IThreadTask"*)**, void (%"class.MARC::IThreadTask"*)*** %60, align 8, !tbaa !133
  %62 = getelementptr inbounds void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %61, i64 2
  %63 = load void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %62, align 8
  tail call void %63(%"class.MARC::IThreadTask"* nonnull %57) #8
  br label %64

; <label>:64:                                     ; preds = %59, %54
  store %"class.MARC::IThreadTask"* null, %"class.MARC::IThreadTask"** %56, align 8, !tbaa !27
  %65 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %55, i64 1
  %66 = icmp eq %"class.std::unique_ptr"* %65, %52
  br i1 %66, label %.loopexit, label %54

; <label>:67:                                     ; preds = %.loopexit3
  %68 = inttoptr i64 %10 to %"class.std::unique_ptr"*
  %69 = icmp eq %"class.std::unique_ptr"* %3, %68
  br i1 %69, label %.loopexit, label %.preheader5

.preheader5:                                      ; preds = %67
  br label %70

; <label>:70:                                     ; preds = %.preheader5, %80
  %71 = phi %"class.std::unique_ptr"* [ %81, %80 ], [ %3, %.preheader5 ]
  %72 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %71, i64 0, i32 0, i32 0, i32 0, i32 0
  %73 = load %"class.MARC::IThreadTask"*, %"class.MARC::IThreadTask"** %72, align 8, !tbaa !27
  %74 = icmp eq %"class.MARC::IThreadTask"* %73, null
  br i1 %74, label %80, label %75

; <label>:75:                                     ; preds = %70
  %76 = bitcast %"class.MARC::IThreadTask"* %73 to void (%"class.MARC::IThreadTask"*)***
  %77 = load void (%"class.MARC::IThreadTask"*)**, void (%"class.MARC::IThreadTask"*)*** %76, align 8, !tbaa !133
  %78 = getelementptr inbounds void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %77, i64 2
  %79 = load void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %78, align 8
  tail call void %79(%"class.MARC::IThreadTask"* nonnull %73) #8
  br label %80

; <label>:80:                                     ; preds = %75, %70
  store %"class.MARC::IThreadTask"* null, %"class.MARC::IThreadTask"** %72, align 8, !tbaa !27
  %81 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %71, i64 1
  %82 = icmp eq %"class.std::unique_ptr"* %81, %68
  br i1 %82, label %.loopexit, label %70

.loopexit:                                        ; preds = %64, %80, %67, %.loopexit2
  %83 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %0, i64 0, i32 0, i32 0, i32 0
  %84 = load %"class.std::unique_ptr"**, %"class.std::unique_ptr"*** %83, align 8, !tbaa !214
  %85 = icmp eq %"class.std::unique_ptr"** %84, null
  br i1 %85, label %103, label %86

; <label>:86:                                     ; preds = %.loopexit
  %87 = bitcast %"class.std::unique_ptr"** %84 to i8*
  %88 = load %"class.std::unique_ptr"**, %"class.std::unique_ptr"*** %6, align 8, !tbaa !234
  %89 = load %"class.std::unique_ptr"**, %"class.std::unique_ptr"*** %13, align 8, !tbaa !213
  %90 = getelementptr inbounds %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %89, i64 1
  %91 = icmp ult %"class.std::unique_ptr"** %88, %90
  br i1 %91, label %.preheader, label %101

.preheader:                                       ; preds = %86
  br label %92

; <label>:92:                                     ; preds = %.preheader, %92
  %93 = phi %"class.std::unique_ptr"** [ %96, %92 ], [ %88, %.preheader ]
  %94 = bitcast %"class.std::unique_ptr"** %93 to i8**
  %95 = load i8*, i8** %94, align 8, !tbaa !27
  tail call void @_ZdlPv(i8* %95) #8
  %96 = getelementptr inbounds %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %93, i64 1
  %97 = icmp ult %"class.std::unique_ptr"** %93, %89
  br i1 %97, label %92, label %98

; <label>:98:                                     ; preds = %92
  %99 = bitcast %"class.std::deque.34"* %0 to i8**
  %100 = load i8*, i8** %99, align 8, !tbaa !214
  br label %101

; <label>:101:                                    ; preds = %98, %86
  %102 = phi i8* [ %100, %98 ], [ %87, %86 ]
  tail call void @_ZdlPv(i8* %102) #8
  br label %103

; <label>:103:                                    ; preds = %.loopexit, %101
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZNSt5dequeISt8functionIFvvEESaIS2_EED2Ev(%"class.std::deque.47"*) unnamed_addr #10 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %2 = alloca %"struct.std::_Deque_iterator.52", align 16
  %3 = alloca %"struct.std::_Deque_iterator.52", align 16
  %4 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %0, i64 0, i32 0, i32 0, i32 2
  %5 = bitcast %"struct.std::_Deque_iterator.52"* %4 to <2 x i64>*
  %6 = load <2 x i64>, <2 x i64>* %5, align 8, !tbaa !27, !noalias !235
  %7 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %0, i64 0, i32 0, i32 0, i32 2, i32 2
  %8 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %0, i64 0, i32 0, i32 0, i32 2, i32 3
  %9 = bitcast %"class.std::function"** %7 to <2 x i64>*
  %10 = load <2 x i64>, <2 x i64>* %9, align 8, !tbaa !27, !noalias !235
  %11 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %0, i64 0, i32 0, i32 0, i32 3
  %12 = bitcast %"struct.std::_Deque_iterator.52"* %11 to <2 x i64>*
  %13 = load <2 x i64>, <2 x i64>* %12, align 8, !tbaa !27, !noalias !238
  %14 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %0, i64 0, i32 0, i32 0, i32 3, i32 2
  %15 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %0, i64 0, i32 0, i32 0, i32 3, i32 3
  %16 = bitcast %"class.std::function"** %14 to <2 x i64>*
  %17 = load <2 x i64>, <2 x i64>* %16, align 8, !tbaa !27, !noalias !238
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
          to label %26 unwind label %48

; <label>:26:                                     ; preds = %1
  call void @llvm.lifetime.end.p0i8(i64 32, i8* nonnull %18)
  call void @llvm.lifetime.end.p0i8(i64 32, i8* nonnull %19)
  %27 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %0, i64 0, i32 0, i32 0, i32 0
  %28 = load %"class.std::function"**, %"class.std::function"*** %27, align 8, !tbaa !241
  %29 = icmp eq %"class.std::function"** %28, null
  br i1 %29, label %47, label %30

; <label>:30:                                     ; preds = %26
  %31 = bitcast %"class.std::function"** %28 to i8*
  %32 = load %"class.std::function"**, %"class.std::function"*** %8, align 8, !tbaa !244
  %33 = load %"class.std::function"**, %"class.std::function"*** %15, align 8, !tbaa !245
  %34 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %33, i64 1
  %35 = icmp ult %"class.std::function"** %32, %34
  br i1 %35, label %.preheader, label %45

.preheader:                                       ; preds = %30
  br label %36

; <label>:36:                                     ; preds = %.preheader, %36
  %37 = phi %"class.std::function"** [ %40, %36 ], [ %32, %.preheader ]
  %38 = bitcast %"class.std::function"** %37 to i8**
  %39 = load i8*, i8** %38, align 8, !tbaa !27
  call void @_ZdlPv(i8* %39) #8
  %40 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %37, i64 1
  %41 = icmp ult %"class.std::function"** %37, %33
  br i1 %41, label %36, label %42

; <label>:42:                                     ; preds = %36
  %43 = bitcast %"class.std::deque.47"* %0 to i8**
  %44 = load i8*, i8** %43, align 8, !tbaa !241
  br label %45

; <label>:45:                                     ; preds = %42, %30
  %46 = phi i8* [ %44, %42 ], [ %31, %30 ]
  call void @_ZdlPv(i8* %46) #8
  br label %47

; <label>:47:                                     ; preds = %26, %45
  ret void

; <label>:48:                                     ; preds = %1
  %49 = landingpad { i8*, i32 }
          catch i8* null
  %50 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %0, i64 0, i32 0
  %51 = extractvalue { i8*, i32 } %49, 0
  call void @_ZNSt11_Deque_baseISt8functionIFvvEESaIS2_EED2Ev(%"class.std::_Deque_base.48"* %50) #8
  call void @__clang_call_terminate(i8* %51) #22
  unreachable
}

; Function Attrs: uwtable
define linkonce_odr void @_ZNSt5dequeISt8functionIFvvEESaIS2_EE19_M_destroy_data_auxESt15_Deque_iteratorIS2_RS2_PS2_ES8_(%"class.std::deque.47"*, %"struct.std::_Deque_iterator.52"*, %"struct.std::_Deque_iterator.52"*) local_unnamed_addr #5 comdat align 2 personality i32 (...)* @__gxx_personality_v0 {
  %4 = getelementptr inbounds %"struct.std::_Deque_iterator.52", %"struct.std::_Deque_iterator.52"* %1, i64 0, i32 3
  %5 = load %"class.std::function"**, %"class.std::function"*** %4, align 8, !tbaa !220
  %6 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %5, i64 1
  %7 = getelementptr inbounds %"struct.std::_Deque_iterator.52", %"struct.std::_Deque_iterator.52"* %2, i64 0, i32 3
  %8 = load %"class.std::function"**, %"class.std::function"*** %7, align 8, !tbaa !220
  %9 = icmp ult %"class.std::function"** %6, %8
  br i1 %9, label %.preheader8, label %12

.preheader8:                                      ; preds = %3
  br label %18

; <label>:10:                                     ; preds = %190
  %11 = load %"class.std::function"**, %"class.std::function"*** %4, align 8, !tbaa !220
  br label %12

; <label>:12:                                     ; preds = %10, %3
  %13 = phi %"class.std::function"** [ %5, %3 ], [ %11, %10 ]
  %14 = phi %"class.std::function"** [ %8, %3 ], [ %192, %10 ]
  %15 = icmp eq %"class.std::function"** %13, %14
  %16 = getelementptr inbounds %"struct.std::_Deque_iterator.52", %"struct.std::_Deque_iterator.52"* %1, i64 0, i32 0
  %17 = load %"class.std::function"*, %"class.std::function"** %16, align 8, !tbaa !222
  br i1 %15, label %71, label %34

; <label>:18:                                     ; preds = %.preheader8, %190
  %19 = phi %"class.std::function"** [ %191, %190 ], [ %6, %.preheader8 ]
  %20 = load %"class.std::function"*, %"class.std::function"** %19, align 8, !tbaa !27
  %21 = getelementptr inbounds %"class.std::function", %"class.std::function"* %20, i64 0, i32 0, i32 1
  %22 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %21, align 8, !tbaa !123
  %23 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %22, null
  br i1 %23, label %30, label %24

; <label>:24:                                     ; preds = %18
  %25 = getelementptr inbounds %"class.std::function", %"class.std::function"* %20, i64 0, i32 0, i32 0
  %26 = invoke zeroext i1 %22(%"union.std::_Any_data"* dereferenceable(16) %25, %"union.std::_Any_data"* dereferenceable(16) %25, i32 3)
          to label %30 unwind label %27

; <label>:27:                                     ; preds = %187, %180, %173, %166, %159, %152, %145, %138, %131, %124, %117, %110, %103, %96, %89, %24
  %28 = landingpad { i8*, i32 }
          catch i8* null
  %29 = extractvalue { i8*, i32 } %28, 0
  tail call void @__clang_call_terminate(i8* %29) #22
  unreachable

; <label>:30:                                     ; preds = %24, %18
  %31 = getelementptr inbounds %"class.std::function", %"class.std::function"* %20, i64 1, i32 0, i32 1
  %32 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %31, align 8, !tbaa !123
  %33 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %32, null
  br i1 %33, label %92, label %89

; <label>:34:                                     ; preds = %12
  %35 = getelementptr inbounds %"struct.std::_Deque_iterator.52", %"struct.std::_Deque_iterator.52"* %1, i64 0, i32 2
  %36 = load %"class.std::function"*, %"class.std::function"** %35, align 8, !tbaa !224
  %37 = icmp eq %"class.std::function"* %17, %36
  br i1 %37, label %.loopexit2, label %.preheader7

.preheader7:                                      ; preds = %34
  br label %38

; <label>:38:                                     ; preds = %.preheader7, %49
  %39 = phi %"class.std::function"* [ %50, %49 ], [ %17, %.preheader7 ]
  %40 = getelementptr inbounds %"class.std::function", %"class.std::function"* %39, i64 0, i32 0, i32 1
  %41 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %40, align 8, !tbaa !123
  %42 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %41, null
  br i1 %42, label %49, label %43

; <label>:43:                                     ; preds = %38
  %44 = getelementptr inbounds %"class.std::function", %"class.std::function"* %39, i64 0, i32 0, i32 0
  %45 = invoke zeroext i1 %41(%"union.std::_Any_data"* dereferenceable(16) %44, %"union.std::_Any_data"* dereferenceable(16) %44, i32 3)
          to label %49 unwind label %46

; <label>:46:                                     ; preds = %43
  %47 = landingpad { i8*, i32 }
          catch i8* null
  %48 = extractvalue { i8*, i32 } %47, 0
  tail call void @__clang_call_terminate(i8* %48) #22
  unreachable

; <label>:49:                                     ; preds = %43, %38
  %50 = getelementptr inbounds %"class.std::function", %"class.std::function"* %39, i64 1
  %51 = icmp eq %"class.std::function"* %50, %36
  br i1 %51, label %.loopexit2, label %38

.loopexit2:                                       ; preds = %49, %34
  %52 = getelementptr inbounds %"struct.std::_Deque_iterator.52", %"struct.std::_Deque_iterator.52"* %2, i64 0, i32 1
  %53 = load %"class.std::function"*, %"class.std::function"** %52, align 8, !tbaa !223
  %54 = getelementptr inbounds %"struct.std::_Deque_iterator.52", %"struct.std::_Deque_iterator.52"* %2, i64 0, i32 0
  %55 = load %"class.std::function"*, %"class.std::function"** %54, align 8, !tbaa !222
  %56 = icmp eq %"class.std::function"* %53, %55
  br i1 %56, label %.loopexit, label %.preheader5

.preheader5:                                      ; preds = %.loopexit2
  br label %57

; <label>:57:                                     ; preds = %.preheader5, %68
  %58 = phi %"class.std::function"* [ %69, %68 ], [ %53, %.preheader5 ]
  %59 = getelementptr inbounds %"class.std::function", %"class.std::function"* %58, i64 0, i32 0, i32 1
  %60 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %59, align 8, !tbaa !123
  %61 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %60, null
  br i1 %61, label %68, label %62

; <label>:62:                                     ; preds = %57
  %63 = getelementptr inbounds %"class.std::function", %"class.std::function"* %58, i64 0, i32 0, i32 0
  %64 = invoke zeroext i1 %60(%"union.std::_Any_data"* dereferenceable(16) %63, %"union.std::_Any_data"* dereferenceable(16) %63, i32 3)
          to label %68 unwind label %65

; <label>:65:                                     ; preds = %62
  %66 = landingpad { i8*, i32 }
          catch i8* null
  %67 = extractvalue { i8*, i32 } %66, 0
  tail call void @__clang_call_terminate(i8* %67) #22
  unreachable

; <label>:68:                                     ; preds = %62, %57
  %69 = getelementptr inbounds %"class.std::function", %"class.std::function"* %58, i64 1
  %70 = icmp eq %"class.std::function"* %69, %55
  br i1 %70, label %.loopexit, label %57

; <label>:71:                                     ; preds = %12
  %72 = getelementptr inbounds %"struct.std::_Deque_iterator.52", %"struct.std::_Deque_iterator.52"* %2, i64 0, i32 0
  %73 = load %"class.std::function"*, %"class.std::function"** %72, align 8, !tbaa !222
  %74 = icmp eq %"class.std::function"* %17, %73
  br i1 %74, label %.loopexit, label %.preheader

.preheader:                                       ; preds = %71
  br label %75

; <label>:75:                                     ; preds = %.preheader, %86
  %76 = phi %"class.std::function"* [ %87, %86 ], [ %17, %.preheader ]
  %77 = getelementptr inbounds %"class.std::function", %"class.std::function"* %76, i64 0, i32 0, i32 1
  %78 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %77, align 8, !tbaa !123
  %79 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %78, null
  br i1 %79, label %86, label %80

; <label>:80:                                     ; preds = %75
  %81 = getelementptr inbounds %"class.std::function", %"class.std::function"* %76, i64 0, i32 0, i32 0
  %82 = invoke zeroext i1 %78(%"union.std::_Any_data"* dereferenceable(16) %81, %"union.std::_Any_data"* dereferenceable(16) %81, i32 3)
          to label %86 unwind label %83

; <label>:83:                                     ; preds = %80
  %84 = landingpad { i8*, i32 }
          catch i8* null
  %85 = extractvalue { i8*, i32 } %84, 0
  tail call void @__clang_call_terminate(i8* %85) #22
  unreachable

; <label>:86:                                     ; preds = %80, %75
  %87 = getelementptr inbounds %"class.std::function", %"class.std::function"* %76, i64 1
  %88 = icmp eq %"class.std::function"* %87, %73
  br i1 %88, label %.loopexit, label %75

.loopexit:                                        ; preds = %68, %86, %71, %.loopexit2
  ret void

; <label>:89:                                     ; preds = %30
  %90 = getelementptr inbounds %"class.std::function", %"class.std::function"* %20, i64 1, i32 0, i32 0
  %91 = invoke zeroext i1 %32(%"union.std::_Any_data"* dereferenceable(16) %90, %"union.std::_Any_data"* dereferenceable(16) %90, i32 3)
          to label %92 unwind label %27

; <label>:92:                                     ; preds = %89, %30
  %93 = getelementptr inbounds %"class.std::function", %"class.std::function"* %20, i64 2, i32 0, i32 1
  %94 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %93, align 8, !tbaa !123
  %95 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %94, null
  br i1 %95, label %99, label %96

; <label>:96:                                     ; preds = %92
  %97 = getelementptr inbounds %"class.std::function", %"class.std::function"* %20, i64 2, i32 0, i32 0
  %98 = invoke zeroext i1 %94(%"union.std::_Any_data"* dereferenceable(16) %97, %"union.std::_Any_data"* dereferenceable(16) %97, i32 3)
          to label %99 unwind label %27

; <label>:99:                                     ; preds = %96, %92
  %100 = getelementptr inbounds %"class.std::function", %"class.std::function"* %20, i64 3, i32 0, i32 1
  %101 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %100, align 8, !tbaa !123
  %102 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %101, null
  br i1 %102, label %106, label %103

; <label>:103:                                    ; preds = %99
  %104 = getelementptr inbounds %"class.std::function", %"class.std::function"* %20, i64 3, i32 0, i32 0
  %105 = invoke zeroext i1 %101(%"union.std::_Any_data"* dereferenceable(16) %104, %"union.std::_Any_data"* dereferenceable(16) %104, i32 3)
          to label %106 unwind label %27

; <label>:106:                                    ; preds = %103, %99
  %107 = getelementptr inbounds %"class.std::function", %"class.std::function"* %20, i64 4, i32 0, i32 1
  %108 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %107, align 8, !tbaa !123
  %109 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %108, null
  br i1 %109, label %113, label %110

; <label>:110:                                    ; preds = %106
  %111 = getelementptr inbounds %"class.std::function", %"class.std::function"* %20, i64 4, i32 0, i32 0
  %112 = invoke zeroext i1 %108(%"union.std::_Any_data"* dereferenceable(16) %111, %"union.std::_Any_data"* dereferenceable(16) %111, i32 3)
          to label %113 unwind label %27

; <label>:113:                                    ; preds = %110, %106
  %114 = getelementptr inbounds %"class.std::function", %"class.std::function"* %20, i64 5, i32 0, i32 1
  %115 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %114, align 8, !tbaa !123
  %116 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %115, null
  br i1 %116, label %120, label %117

; <label>:117:                                    ; preds = %113
  %118 = getelementptr inbounds %"class.std::function", %"class.std::function"* %20, i64 5, i32 0, i32 0
  %119 = invoke zeroext i1 %115(%"union.std::_Any_data"* dereferenceable(16) %118, %"union.std::_Any_data"* dereferenceable(16) %118, i32 3)
          to label %120 unwind label %27

; <label>:120:                                    ; preds = %117, %113
  %121 = getelementptr inbounds %"class.std::function", %"class.std::function"* %20, i64 6, i32 0, i32 1
  %122 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %121, align 8, !tbaa !123
  %123 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %122, null
  br i1 %123, label %127, label %124

; <label>:124:                                    ; preds = %120
  %125 = getelementptr inbounds %"class.std::function", %"class.std::function"* %20, i64 6, i32 0, i32 0
  %126 = invoke zeroext i1 %122(%"union.std::_Any_data"* dereferenceable(16) %125, %"union.std::_Any_data"* dereferenceable(16) %125, i32 3)
          to label %127 unwind label %27

; <label>:127:                                    ; preds = %124, %120
  %128 = getelementptr inbounds %"class.std::function", %"class.std::function"* %20, i64 7, i32 0, i32 1
  %129 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %128, align 8, !tbaa !123
  %130 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %129, null
  br i1 %130, label %134, label %131

; <label>:131:                                    ; preds = %127
  %132 = getelementptr inbounds %"class.std::function", %"class.std::function"* %20, i64 7, i32 0, i32 0
  %133 = invoke zeroext i1 %129(%"union.std::_Any_data"* dereferenceable(16) %132, %"union.std::_Any_data"* dereferenceable(16) %132, i32 3)
          to label %134 unwind label %27

; <label>:134:                                    ; preds = %131, %127
  %135 = getelementptr inbounds %"class.std::function", %"class.std::function"* %20, i64 8, i32 0, i32 1
  %136 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %135, align 8, !tbaa !123
  %137 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %136, null
  br i1 %137, label %141, label %138

; <label>:138:                                    ; preds = %134
  %139 = getelementptr inbounds %"class.std::function", %"class.std::function"* %20, i64 8, i32 0, i32 0
  %140 = invoke zeroext i1 %136(%"union.std::_Any_data"* dereferenceable(16) %139, %"union.std::_Any_data"* dereferenceable(16) %139, i32 3)
          to label %141 unwind label %27

; <label>:141:                                    ; preds = %138, %134
  %142 = getelementptr inbounds %"class.std::function", %"class.std::function"* %20, i64 9, i32 0, i32 1
  %143 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %142, align 8, !tbaa !123
  %144 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %143, null
  br i1 %144, label %148, label %145

; <label>:145:                                    ; preds = %141
  %146 = getelementptr inbounds %"class.std::function", %"class.std::function"* %20, i64 9, i32 0, i32 0
  %147 = invoke zeroext i1 %143(%"union.std::_Any_data"* dereferenceable(16) %146, %"union.std::_Any_data"* dereferenceable(16) %146, i32 3)
          to label %148 unwind label %27

; <label>:148:                                    ; preds = %145, %141
  %149 = getelementptr inbounds %"class.std::function", %"class.std::function"* %20, i64 10, i32 0, i32 1
  %150 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %149, align 8, !tbaa !123
  %151 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %150, null
  br i1 %151, label %155, label %152

; <label>:152:                                    ; preds = %148
  %153 = getelementptr inbounds %"class.std::function", %"class.std::function"* %20, i64 10, i32 0, i32 0
  %154 = invoke zeroext i1 %150(%"union.std::_Any_data"* dereferenceable(16) %153, %"union.std::_Any_data"* dereferenceable(16) %153, i32 3)
          to label %155 unwind label %27

; <label>:155:                                    ; preds = %152, %148
  %156 = getelementptr inbounds %"class.std::function", %"class.std::function"* %20, i64 11, i32 0, i32 1
  %157 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %156, align 8, !tbaa !123
  %158 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %157, null
  br i1 %158, label %162, label %159

; <label>:159:                                    ; preds = %155
  %160 = getelementptr inbounds %"class.std::function", %"class.std::function"* %20, i64 11, i32 0, i32 0
  %161 = invoke zeroext i1 %157(%"union.std::_Any_data"* dereferenceable(16) %160, %"union.std::_Any_data"* dereferenceable(16) %160, i32 3)
          to label %162 unwind label %27

; <label>:162:                                    ; preds = %159, %155
  %163 = getelementptr inbounds %"class.std::function", %"class.std::function"* %20, i64 12, i32 0, i32 1
  %164 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %163, align 8, !tbaa !123
  %165 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %164, null
  br i1 %165, label %169, label %166

; <label>:166:                                    ; preds = %162
  %167 = getelementptr inbounds %"class.std::function", %"class.std::function"* %20, i64 12, i32 0, i32 0
  %168 = invoke zeroext i1 %164(%"union.std::_Any_data"* dereferenceable(16) %167, %"union.std::_Any_data"* dereferenceable(16) %167, i32 3)
          to label %169 unwind label %27

; <label>:169:                                    ; preds = %166, %162
  %170 = getelementptr inbounds %"class.std::function", %"class.std::function"* %20, i64 13, i32 0, i32 1
  %171 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %170, align 8, !tbaa !123
  %172 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %171, null
  br i1 %172, label %176, label %173

; <label>:173:                                    ; preds = %169
  %174 = getelementptr inbounds %"class.std::function", %"class.std::function"* %20, i64 13, i32 0, i32 0
  %175 = invoke zeroext i1 %171(%"union.std::_Any_data"* dereferenceable(16) %174, %"union.std::_Any_data"* dereferenceable(16) %174, i32 3)
          to label %176 unwind label %27

; <label>:176:                                    ; preds = %173, %169
  %177 = getelementptr inbounds %"class.std::function", %"class.std::function"* %20, i64 14, i32 0, i32 1
  %178 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %177, align 8, !tbaa !123
  %179 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %178, null
  br i1 %179, label %183, label %180

; <label>:180:                                    ; preds = %176
  %181 = getelementptr inbounds %"class.std::function", %"class.std::function"* %20, i64 14, i32 0, i32 0
  %182 = invoke zeroext i1 %178(%"union.std::_Any_data"* dereferenceable(16) %181, %"union.std::_Any_data"* dereferenceable(16) %181, i32 3)
          to label %183 unwind label %27

; <label>:183:                                    ; preds = %180, %176
  %184 = getelementptr inbounds %"class.std::function", %"class.std::function"* %20, i64 15, i32 0, i32 1
  %185 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %184, align 8, !tbaa !123
  %186 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %185, null
  br i1 %186, label %190, label %187

; <label>:187:                                    ; preds = %183
  %188 = getelementptr inbounds %"class.std::function", %"class.std::function"* %20, i64 15, i32 0, i32 0
  %189 = invoke zeroext i1 %185(%"union.std::_Any_data"* dereferenceable(16) %188, %"union.std::_Any_data"* dereferenceable(16) %188, i32 3)
          to label %190 unwind label %27

; <label>:190:                                    ; preds = %187, %183
  %191 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %19, i64 1
  %192 = load %"class.std::function"**, %"class.std::function"*** %7, align 8, !tbaa !220
  %193 = icmp ult %"class.std::function"** %191, %192
  br i1 %193, label %18, label %10
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZNSt11_Deque_baseISt8functionIFvvEESaIS2_EED2Ev(%"class.std::_Deque_base.48"*) unnamed_addr #10 comdat align 2 personality i32 (...)* @__gxx_personality_v0 {
  %2 = getelementptr inbounds %"class.std::_Deque_base.48", %"class.std::_Deque_base.48"* %0, i64 0, i32 0, i32 0
  %3 = load %"class.std::function"**, %"class.std::function"*** %2, align 8, !tbaa !241
  %4 = icmp eq %"class.std::function"** %3, null
  br i1 %4, label %24, label %5

; <label>:5:                                      ; preds = %1
  %6 = bitcast %"class.std::function"** %3 to i8*
  %7 = getelementptr inbounds %"class.std::_Deque_base.48", %"class.std::_Deque_base.48"* %0, i64 0, i32 0, i32 2, i32 3
  %8 = load %"class.std::function"**, %"class.std::function"*** %7, align 8, !tbaa !244
  %9 = getelementptr inbounds %"class.std::_Deque_base.48", %"class.std::_Deque_base.48"* %0, i64 0, i32 0, i32 3, i32 3
  %10 = load %"class.std::function"**, %"class.std::function"*** %9, align 8, !tbaa !245
  %11 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %10, i64 1
  %12 = icmp ult %"class.std::function"** %8, %11
  br i1 %12, label %.preheader, label %22

.preheader:                                       ; preds = %5
  br label %13

; <label>:13:                                     ; preds = %.preheader, %13
  %14 = phi %"class.std::function"** [ %17, %13 ], [ %8, %.preheader ]
  %15 = bitcast %"class.std::function"** %14 to i8**
  %16 = load i8*, i8** %15, align 8, !tbaa !27
  tail call void @_ZdlPv(i8* %16) #8
  %17 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %14, i64 1
  %18 = icmp ult %"class.std::function"** %14, %10
  br i1 %18, label %13, label %19

; <label>:19:                                     ; preds = %13
  %20 = bitcast %"class.std::_Deque_base.48"* %0 to i8**
  %21 = load i8*, i8** %20, align 8, !tbaa !241
  br label %22

; <label>:22:                                     ; preds = %5, %19
  %23 = phi i8* [ %21, %19 ], [ %6, %5 ]
  tail call void @_ZdlPv(i8* %23) #8
  br label %24

; <label>:24:                                     ; preds = %1, %22
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
  br i1 %17, label %51, label %18

; <label>:18:                                     ; preds = %13
  %19 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 0
  %20 = load %"class.std::function"*, %"class.std::function"** %19, align 8, !tbaa !222
  %21 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 0
  %22 = load %"class.std::function"*, %"class.std::function"** %21, align 8, !tbaa !222
  %23 = icmp eq %"class.std::function"* %20, %22
  br i1 %23, label %24, label %.loopexit

; <label>:24:                                     ; preds = %18
  %25 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 3
  br label %26

; <label>:26:                                     ; preds = %30, %24
  %27 = load atomic i8, i8* %14 seq_cst, align 1
  %28 = and i8 %27, 1
  %29 = icmp eq i8 %28, 0
  br i1 %29, label %.loopexit, label %30

; <label>:30:                                     ; preds = %26
  call void @_ZNSt18condition_variable4waitERSt11unique_lockISt5mutexE(%"class.std::condition_variable"* nonnull %25, %"class.std::unique_lock"* nonnull dereferenceable(16) %3) #8
  %31 = load %"class.std::function"*, %"class.std::function"** %19, align 8, !tbaa !222
  %32 = load %"class.std::function"*, %"class.std::function"** %21, align 8, !tbaa !222
  %33 = icmp eq %"class.std::function"* %31, %32
  br i1 %33, label %26, label %.loopexit

; <label>:34:                                     ; preds = %50
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

.loopexit:                                        ; preds = %30, %26, %18
  %47 = load atomic i8, i8* %14 seq_cst, align 1
  %48 = and i8 %47, 1
  %49 = icmp eq i8 %48, 0
  br i1 %49, label %51, label %50

; <label>:50:                                     ; preds = %.loopexit
  invoke void @_ZN4MARC15ThreadSafeQueueISt8functionIFvvEEE12internal_popERS3_(%"class.MARC::ThreadSafeQueue.45"* nonnull %0, %"class.std::function"* nonnull dereferenceable(32) %1)
          to label %51 unwind label %34

; <label>:51:                                     ; preds = %.loopexit, %13, %50
  %52 = phi i1 [ false, %13 ], [ false, %.loopexit ], [ true, %50 ]
  %53 = load i8, i8* %7, align 8, !tbaa !35, !range !43
  %54 = icmp eq i8 %53, 0
  br i1 %54, label %63, label %55

; <label>:55:                                     ; preds = %51
  %56 = load %"class.std::mutex"*, %"class.std::mutex"** %6, align 8, !tbaa !32
  %57 = icmp eq %"class.std::mutex"* %56, null
  br i1 %57, label %63, label %58

; <label>:58:                                     ; preds = %55
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %59, label %62

; <label>:59:                                     ; preds = %58
  %60 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %56, i64 0, i32 0, i32 0
  %61 = call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %60) #8
  br label %62

; <label>:62:                                     ; preds = %59, %58
  store i8 0, i8* %7, align 8, !tbaa !35
  br label %63

; <label>:63:                                     ; preds = %51, %55, %62
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %4) #8
  ret i1 %52
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
  %6 = load %"class.std::function"*, %"class.std::function"** %5, align 8, !tbaa !222, !noalias !246
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
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %7, i8* nonnull %15, i64 16, i32 8, i1 false) #8, !tbaa.struct !249
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %15, i8* nonnull %14, i64 16, i32 8, i1 false) #8
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %14)
  %16 = getelementptr inbounds %"class.std::function", %"class.std::function"* %1, i64 0, i32 0, i32 1
  %17 = bitcast i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %16 to <2 x i64>*
  %18 = load <2 x i64>, <2 x i64>* %17, align 8, !tbaa !27
  %19 = bitcast i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %8 to <2 x i64>*
  store <2 x i64> %18, <2 x i64>* %19, align 8, !tbaa !27
  store <2 x i64> %13, <2 x i64>* %17, align 8, !tbaa !27
  %20 = extractelement <2 x i64> %18, i32 0
  %21 = icmp eq i64 %20, 0
  br i1 %21, label %29, label %22

; <label>:22:                                     ; preds = %2
  %23 = inttoptr i64 %20 to i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*
  %24 = getelementptr inbounds %"class.std::function", %"class.std::function"* %4, i64 0, i32 0, i32 0
  %25 = invoke zeroext i1 %23(%"union.std::_Any_data"* nonnull dereferenceable(16) %24, %"union.std::_Any_data"* nonnull dereferenceable(16) %24, i32 3)
          to label %29 unwind label %26

; <label>:26:                                     ; preds = %22
  %27 = landingpad { i8*, i32 }
          catch i8* null
  %28 = extractvalue { i8*, i32 } %27, 0
  call void @__clang_call_terminate(i8* %28) #22
  unreachable

; <label>:29:                                     ; preds = %2, %22
  call void @llvm.lifetime.end.p0i8(i64 32, i8* nonnull %7) #8
  %30 = load %"class.std::function"*, %"class.std::function"** %5, align 8, !tbaa !250
  %31 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 2
  %32 = load %"class.std::function"*, %"class.std::function"** %31, align 8, !tbaa !251
  %33 = getelementptr inbounds %"class.std::function", %"class.std::function"* %32, i64 -1
  %34 = icmp eq %"class.std::function"* %30, %33
  br i1 %34, label %50, label %35

; <label>:35:                                     ; preds = %29
  %36 = getelementptr inbounds %"class.std::function", %"class.std::function"* %30, i64 0, i32 0, i32 1
  %37 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %36, align 8, !tbaa !123
  %38 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %37, null
  br i1 %38, label %47, label %39

; <label>:39:                                     ; preds = %35
  %40 = getelementptr inbounds %"class.std::function", %"class.std::function"* %30, i64 0, i32 0, i32 0
  %41 = invoke zeroext i1 %37(%"union.std::_Any_data"* dereferenceable(16) %40, %"union.std::_Any_data"* dereferenceable(16) %40, i32 3)
          to label %42 unwind label %44

; <label>:42:                                     ; preds = %39
  %43 = load %"class.std::function"*, %"class.std::function"** %5, align 8, !tbaa !250
  br label %47

; <label>:44:                                     ; preds = %39
  %45 = landingpad { i8*, i32 }
          catch i8* null
  %46 = extractvalue { i8*, i32 } %45, 0
  call void @__clang_call_terminate(i8* %46) #22
  unreachable

; <label>:47:                                     ; preds = %42, %35
  %48 = phi %"class.std::function"* [ %43, %42 ], [ %30, %35 ]
  %49 = getelementptr inbounds %"class.std::function", %"class.std::function"* %48, i64 1
  store %"class.std::function"* %49, %"class.std::function"** %5, align 8, !tbaa !250
  br label %72

; <label>:50:                                     ; preds = %29
  %51 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2
  %52 = getelementptr inbounds %"class.std::function", %"class.std::function"* %30, i64 0, i32 0, i32 1
  %53 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %52, align 8, !tbaa !123
  %54 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %53, null
  br i1 %54, label %61, label %55

; <label>:55:                                     ; preds = %50
  %56 = getelementptr inbounds %"class.std::function", %"class.std::function"* %30, i64 0, i32 0, i32 0
  %57 = invoke zeroext i1 %53(%"union.std::_Any_data"* dereferenceable(16) %56, %"union.std::_Any_data"* dereferenceable(16) %56, i32 3)
          to label %61 unwind label %58

; <label>:58:                                     ; preds = %55
  %59 = landingpad { i8*, i32 }
          catch i8* null
  %60 = extractvalue { i8*, i32 } %59, 0
  call void @__clang_call_terminate(i8* %60) #22
  unreachable

; <label>:61:                                     ; preds = %55, %50
  %62 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 1
  %63 = bitcast %"class.std::function"** %62 to i8**
  %64 = load i8*, i8** %63, align 8, !tbaa !252
  call void @_ZdlPv(i8* %64) #8
  %65 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 3
  %66 = load %"class.std::function"**, %"class.std::function"*** %65, align 8, !tbaa !244
  %67 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %66, i64 1
  store %"class.std::function"** %67, %"class.std::function"*** %65, align 8, !tbaa !220
  %68 = load %"class.std::function"*, %"class.std::function"** %67, align 8, !tbaa !27
  store %"class.std::function"* %68, %"class.std::function"** %62, align 8, !tbaa !223
  %69 = getelementptr inbounds %"class.std::function", %"class.std::function"* %68, i64 16
  store %"class.std::function"* %69, %"class.std::function"** %31, align 8, !tbaa !224
  %70 = ptrtoint %"class.std::function"* %68 to i64
  %71 = bitcast %"struct.std::_Deque_iterator.52"* %51 to i64*
  store i64 %70, i64* %71, align 8, !tbaa !250
  br label %72

; <label>:72:                                     ; preds = %47, %61
  %73 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 4
  call void @_ZNSt18condition_variable10notify_oneEv(%"class.std::condition_variable"* %73) #8
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
  br i1 %17, label %.loopexit, label %18

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
  br i1 %28, label %20, label %.loopexit

.loopexit:                                        ; preds = %20, %8
  %29 = getelementptr inbounds %"class.std::__future_base::_State_baseV2", %"class.std::__future_base::_State_baseV2"* %5, i64 0, i32 1, i32 0, i32 0, i32 0, i32 0
  %30 = load %"struct.std::__future_base::_Result_base"*, %"struct.std::__future_base::_Result_base"** %29, align 8, !tbaa !27
  %31 = getelementptr inbounds %"struct.std::__future_base::_Result_base", %"struct.std::__future_base::_Result_base"* %30, i64 0, i32 1
  %32 = bitcast %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* %2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %32) #8
  %33 = getelementptr inbounds %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider", %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* %2, i64 0, i32 0
  store i8* null, i8** %33, align 8, !tbaa !253
  %34 = call zeroext i1 @_ZNSt15__exception_ptreqERKNS_13exception_ptrES2_(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* dereferenceable(8) %31, %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* nonnull dereferenceable(8) %2) #26
  call void @_ZNSt15__exception_ptr13exception_ptrD1Ev(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* nonnull %2) #8
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %32) #8
  br i1 %34, label %39, label %35

; <label>:35:                                     ; preds = %.loopexit
  call void @_ZNSt15__exception_ptr13exception_ptrC1ERKS0_(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* nonnull %3, %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* nonnull dereferenceable(8) %31) #8
  invoke void @_ZSt17rethrow_exceptionNSt15__exception_ptr13exception_ptrE(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* nonnull %3) #23
          to label %36 unwind label %37

; <label>:36:                                     ; preds = %35
  unreachable

; <label>:37:                                     ; preds = %35
  %38 = landingpad { i8*, i32 }
          cleanup
  call void @_ZNSt15__exception_ptr13exception_ptrD1Ev(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* nonnull %3) #8
  resume { i8*, i32 } %38

; <label>:39:                                     ; preds = %.loopexit
  %40 = bitcast %"struct.std::__future_base::_Result_base"* %30 to %"struct.std::__future_base::_Result"*
  ret %"struct.std::__future_base::_Result"* %40
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
  %4 = load i64, i64* %3, align 8, !tbaa !255
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
  %5 = load %"class.std::unique_ptr"**, %"class.std::unique_ptr"*** %4, align 8, !tbaa !213
  %6 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %0, i64 0, i32 0, i32 0, i32 2, i32 3
  %7 = load %"class.std::unique_ptr"**, %"class.std::unique_ptr"*** %6, align 8, !tbaa !234
  %8 = ptrtoint %"class.std::unique_ptr"** %5 to i64
  %9 = ptrtoint %"class.std::unique_ptr"** %7 to i64
  %10 = sub i64 %8, %9
  %11 = ashr exact i64 %10, 3
  %12 = add nsw i64 %11, 1
  %13 = add i64 %12, %1
  %14 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %0, i64 0, i32 0, i32 0, i32 1
  %15 = load i64, i64* %14, align 8, !tbaa !212
  %16 = shl i64 %13, 1
  %17 = icmp ugt i64 %15, %16
  br i1 %17, label %18, label %44

; <label>:18:                                     ; preds = %3
  %19 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %0, i64 0, i32 0, i32 0, i32 0
  %20 = load %"class.std::unique_ptr"**, %"class.std::unique_ptr"*** %19, align 8, !tbaa !214
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
  %60 = load %"class.std::unique_ptr"**, %"class.std::unique_ptr"*** %6, align 8, !tbaa !234
  %61 = load %"class.std::unique_ptr"**, %"class.std::unique_ptr"*** %4, align 8, !tbaa !213
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
  %72 = load i8*, i8** %71, align 8, !tbaa !214
  tail call void @_ZdlPv(i8* %72) #8
  store i8* %53, i8** %71, align 8, !tbaa !214
  store i64 %48, i64* %14, align 8, !tbaa !212
  br label %73

; <label>:73:                                     ; preds = %38, %35, %32, %30, %70
  %74 = phi %"class.std::unique_ptr"** [ %59, %70 ], [ %25, %30 ], [ %25, %32 ], [ %25, %35 ], [ %25, %38 ]
  store %"class.std::unique_ptr"** %74, %"class.std::unique_ptr"*** %6, align 8, !tbaa !215
  %75 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %74, align 8, !tbaa !27
  %76 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %0, i64 0, i32 0, i32 0, i32 2, i32 1
  store %"class.std::unique_ptr"* %75, %"class.std::unique_ptr"** %76, align 8, !tbaa !216
  %77 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %75, i64 64
  %78 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %0, i64 0, i32 0, i32 0, i32 2, i32 2
  store %"class.std::unique_ptr"* %77, %"class.std::unique_ptr"** %78, align 8, !tbaa !217
  %79 = getelementptr inbounds %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %74, i64 %11
  store %"class.std::unique_ptr"** %79, %"class.std::unique_ptr"*** %4, align 8, !tbaa !215
  %80 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %79, align 8, !tbaa !27
  %81 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %0, i64 0, i32 0, i32 0, i32 3, i32 1
  store %"class.std::unique_ptr"* %80, %"class.std::unique_ptr"** %81, align 8, !tbaa !216
  %82 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %80, i64 64
  %83 = getelementptr inbounds %"class.std::deque.34", %"class.std::deque.34"* %0, i64 0, i32 0, i32 0, i32 3, i32 2
  store %"class.std::unique_ptr"* %82, %"class.std::unique_ptr"** %83, align 8, !tbaa !217
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZNSt13packaged_taskIFvvEED2Ev(%"class.std::packaged_task"*) unnamed_addr #10 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %2 = alloca %"class.std::unique_ptr.58", align 8
  %3 = getelementptr inbounds %"class.std::packaged_task", %"class.std::packaged_task"* %0, i64 0, i32 0, i32 0
  %4 = getelementptr inbounds %"class.std::packaged_task", %"class.std::packaged_task"* %0, i64 0, i32 0, i32 0, i32 0
  %5 = load %"struct.std::__future_base::_Task_state_base"*, %"struct.std::__future_base::_Task_state_base"** %4, align 8, !tbaa !255
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
  store i64 %20, i64* %21, align 8, !tbaa !181
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
  %17 = load i64, i64* %16, align 8, !tbaa !253
  %18 = bitcast %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* %3 to i64*
  store i64 %17, i64* %18, align 8, !tbaa !253
  store i8* null, i8** %15, align 8, !tbaa !253
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
  %13 = load void (%"class.std::basic_string"*, %"class.MARC::IThreadTask"*, i32)**, void (%"class.std::basic_string"*, %"class.MARC::IThreadTask"*, i32)*** %12, align 8, !tbaa !133, !noalias !257
  %14 = getelementptr inbounds void (%"class.std::basic_string"*, %"class.MARC::IThreadTask"*, i32)*, void (%"class.std::basic_string"*, %"class.MARC::IThreadTask"*, i32)** %13, i64 3
  %15 = load void (%"class.std::basic_string"*, %"class.MARC::IThreadTask"*, i32)*, void (%"class.std::basic_string"*, %"class.MARC::IThreadTask"*, i32)** %14, align 8, !noalias !257
  call void %15(%"class.std::basic_string"* nonnull sret %9, %"class.MARC::IThreadTask"* nonnull %2, i32 %1)
  %16 = invoke dereferenceable(8) %"class.std::basic_string"* @_ZNSs6insertEmPKcm(%"class.std::basic_string"* nonnull %9, i64 0, i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.6, i64 0, i64 0), i64 19)
          to label %17 unwind label %65

; <label>:17:                                     ; preds = %3
  %18 = bitcast %"class.std::basic_string"* %16 to i64*
  %19 = load i64, i64* %18, align 8, !tbaa !2, !noalias !260
  %20 = bitcast %"class.std::basic_string"* %8 to i64*
  store i64 %19, i64* %20, align 8, !tbaa !2, !alias.scope !260
  %21 = getelementptr inbounds %"class.std::basic_string", %"class.std::basic_string"* %16, i64 0, i32 0, i32 0
  store i8* bitcast (i64* getelementptr inbounds ([0 x i64], [0 x i64]* @_ZNSs4_Rep20_S_empty_rep_storageE, i64 0, i64 3) to i8*), i8** %21, align 8, !tbaa !10, !noalias !260
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
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %9, i8* %11, i64 16, i32 8, i1 false), !tbaa.struct !263
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
  %3 = load %"struct.std::__future_base::_Task_state_base"*, %"struct.std::__future_base::_Task_state_base"** %2, align 8, !tbaa !255
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
  store i64 %12, i64* %13, align 8, !alias.scope !264
  %14 = getelementptr inbounds %"struct.std::__future_base::_Task_state", %"struct.std::__future_base::_Task_state"* %0, i64 0, i32 0, i32 1
  %15 = getelementptr inbounds %"class.std::function.133", %"class.std::function.133"* %9, i64 0, i32 0, i32 1
  %16 = bitcast %"class.std::function.133"* %9 to %"class.std::unique_ptr.109"**
  store %"class.std::unique_ptr.109"* %14, %"class.std::unique_ptr.109"** %16, align 8
  %17 = getelementptr inbounds %"class.std::function.133", %"class.std::function.133"* %9, i64 0, i32 0, i32 0, i32 0, i32 0, i32 1
  %18 = bitcast i64* %17 to %"struct.std::_Bind_simple.127"**
  store %"struct.std::_Bind_simple.127"* %8, %"struct.std::_Bind_simple.127"** %18, align 8
  %19 = getelementptr inbounds %"class.std::function.133", %"class.std::function.133"* %9, i64 0, i32 1
  store void (%"class.std::unique_ptr.58"*, %"union.std::_Any_data"*)* @_ZNSt17_Function_handlerIFSt10unique_ptrINSt13__future_base12_Result_baseENS2_8_DeleterEEvENS1_12_Task_setterIS0_INS1_7_ResultIvEES3_ESt12_Bind_simpleIFSt17reference_wrapperISt5_BindIFPFvPvSD_ESD_SD_EEEvEEvEEE9_M_invokeERKSt9_Any_data, void (%"class.std::unique_ptr.58"*, %"union.std::_Any_data"*)** %19, align 8, !tbaa !267
  store i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* @_ZNSt14_Function_base13_Base_managerINSt13__future_base12_Task_setterISt10unique_ptrINS1_7_ResultIvEENS1_12_Result_base8_DeleterEESt12_Bind_simpleIFSt17reference_wrapperISt5_BindIFPFvPvSC_ESC_SC_EEEvEEvEEE10_M_managerERSt9_Any_dataRKSM_St18_Manager_operation, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %15, align 8, !tbaa !123
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %3) #8
  store i8 0, i8* %3, align 1, !tbaa !269
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
  %39 = load i8, i8* %3, align 1, !tbaa !269, !range !43
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
  store i64 %9, i64* %10, align 8, !alias.scope !270
  %11 = bitcast %"struct.std::__future_base::_Task_state"* %0 to %"class.std::__future_base::_State_baseV2"*
  %12 = getelementptr inbounds %"struct.std::__future_base::_Task_state", %"struct.std::__future_base::_Task_state"* %0, i64 0, i32 0, i32 1
  %13 = getelementptr inbounds %"class.std::function.133", %"class.std::function.133"* %4, i64 0, i32 0, i32 1
  %14 = bitcast %"class.std::function.133"* %4 to %"class.std::unique_ptr.109"**
  store %"class.std::unique_ptr.109"* %12, %"class.std::unique_ptr.109"** %14, align 8
  %15 = getelementptr inbounds %"class.std::function.133", %"class.std::function.133"* %4, i64 0, i32 0, i32 0, i32 0, i32 0, i32 1
  %16 = bitcast i64* %15 to %"struct.std::_Bind_simple.127"**
  store %"struct.std::_Bind_simple.127"* %3, %"struct.std::_Bind_simple.127"** %16, align 8
  %17 = getelementptr inbounds %"class.std::function.133", %"class.std::function.133"* %4, i64 0, i32 1
  store void (%"class.std::unique_ptr.58"*, %"union.std::_Any_data"*)* @_ZNSt17_Function_handlerIFSt10unique_ptrINSt13__future_base12_Result_baseENS2_8_DeleterEEvENS1_12_Task_setterIS0_INS1_7_ResultIvEES3_ESt12_Bind_simpleIFSt17reference_wrapperISt5_BindIFPFvPvSD_ESD_SD_EEEvEEvEEE9_M_invokeERKSt9_Any_data, void (%"class.std::unique_ptr.58"*, %"union.std::_Any_data"*)** %17, align 8, !tbaa !267
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
  %23 = load %"class.std::_Sp_counted_base"*, %"class.std::_Sp_counted_base"** %22, align 8, !tbaa !273
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
  %53 = load %"class.std::_Sp_counted_base"*, %"class.std::_Sp_counted_base"** %52, align 8, !tbaa !273
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
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %7) #8, !noalias !275
  %8 = bitcast <2 x i64>* %4 to %"struct.std::__future_base::_Task_state"**
  store %"struct.std::__future_base::_Task_state"* null, %"struct.std::__future_base::_Task_state"** %8, align 16, !tbaa !200, !alias.scope !278
  %9 = getelementptr inbounds %"class.std::shared_ptr.121", %"class.std::shared_ptr.121"* %5, i64 0, i32 0, i32 1
  %10 = getelementptr inbounds %"class.std::ios_base::Init", %"class.std::ios_base::Init"* %3, i64 0, i32 0
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %10), !noalias !278
  %11 = getelementptr inbounds %"class.std::__shared_count", %"class.std::__shared_count"* %9, i64 0, i32 0
  store %"class.std::_Sp_counted_base"* null, %"class.std::_Sp_counted_base"** %11, align 8, !tbaa !129, !alias.scope !278
  %12 = invoke i8* @_Znwm(i64 80)
          to label %13 unwind label %17, !noalias !278

; <label>:13:                                     ; preds = %2
  %14 = bitcast %"struct.std::__future_base::_Task_state<std::_Bind<void (*(void *, void *))(void *, void *)>, std::allocator<int>, void ()>::_Impl"* %6 to %"class.std::ios_base::Init"*
  %15 = getelementptr inbounds %"struct.std::__future_base::_Task_state<std::_Bind<void (*(void *, void *))(void *, void *)>, std::allocator<int>, void ()>::_Impl", %"struct.std::__future_base::_Task_state<std::_Bind<void (*(void *, void *))(void *, void *)>, std::allocator<int>, void ()>::_Impl"* %6, i64 0, i32 0
  %16 = bitcast i8* %12 to %"class.std::_Sp_counted_ptr_inplace"*
  invoke void @_ZNSt23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EEC2IJS7_RKS8_EEES8_DpOT_(%"class.std::_Sp_counted_ptr_inplace"* nonnull %16, %"class.std::ios_base::Init"* nonnull %3, %"class.std::_Bind"* nonnull dereferenceable(24) %15, %"class.std::ios_base::Init"* nonnull dereferenceable(1) %14)
          to label %30 unwind label %21, !noalias !278

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
  store i8* %12, i8** %31, align 8, !tbaa !129, !alias.scope !278
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %10), !noalias !278
  %32 = bitcast i8* %12 to %"class.std::_Sp_counted_base"*
  %33 = bitcast i8* %12 to i8* (%"class.std::_Sp_counted_base"*, %"class.std::type_info"*)***
  %34 = load i8* (%"class.std::_Sp_counted_base"*, %"class.std::type_info"*)**, i8* (%"class.std::_Sp_counted_base"*, %"class.std::type_info"*)*** %33, align 8, !tbaa !133, !noalias !278
  %35 = getelementptr inbounds i8* (%"class.std::_Sp_counted_base"*, %"class.std::type_info"*)*, i8* (%"class.std::_Sp_counted_base"*, %"class.std::type_info"*)** %34, i64 4
  %36 = load i8* (%"class.std::_Sp_counted_base"*, %"class.std::type_info"*)*, i8* (%"class.std::_Sp_counted_base"*, %"class.std::type_info"*)** %35, align 8, !noalias !278
  %37 = call i8* %36(%"class.std::_Sp_counted_base"* nonnull %32, %"class.std::type_info"* nonnull dereferenceable(16) bitcast ({ i8*, i8* }* @_ZTISt19_Sp_make_shared_tag to %"class.std::type_info"*)) #8, !noalias !278
  %38 = bitcast <2 x i64>* %4 to i8**
  store i8* %37, i8** %38, align 16, !tbaa !200, !alias.scope !278
  call void (%"class.std::__shared_count"*, ...) @_ZSt32__enable_shared_from_this_helperILN9__gnu_cxx12_Lock_policyE2EEvRKSt14__shared_countIXT_EEz(%"class.std::__shared_count"* nonnull dereferenceable(8) %9, i8* %37, i8* %37) #8
  %39 = load <2 x i64>, <2 x i64>* %4, align 16, !tbaa !27, !noalias !275
  %40 = bitcast %"class.std::shared_ptr.107"* %0 to <2 x i64>*
  store <2 x i64> %39, <2 x i64>* %40, align 8, !tbaa !27, !alias.scope !275
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %7) #8, !noalias !275
  ret void
}

; Function Attrs: uwtable
define linkonce_odr void @_ZNSt23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EEC2IJS7_RKS8_EEES8_DpOT_(%"class.std::_Sp_counted_ptr_inplace"*, %"class.std::ios_base::Init"*, %"class.std::_Bind"* dereferenceable(24), %"class.std::ios_base::Init"* dereferenceable(1)) unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %5 = getelementptr inbounds %"class.std::_Sp_counted_ptr_inplace", %"class.std::_Sp_counted_ptr_inplace"* %0, i64 0, i32 0, i32 0
  %6 = getelementptr inbounds %"class.std::_Sp_counted_ptr_inplace", %"class.std::_Sp_counted_ptr_inplace"* %0, i64 0, i32 0, i32 1
  store i32 1, i32* %6, align 8, !tbaa !178
  %7 = getelementptr inbounds %"class.std::_Sp_counted_ptr_inplace", %"class.std::_Sp_counted_ptr_inplace"* %0, i64 0, i32 0, i32 2
  store i32 1, i32* %7, align 4, !tbaa !180
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [7 x i8*] }, { [7 x i8*] }* @_ZTVSt23_Sp_counted_ptr_inplaceINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_LN9__gnu_cxx12_Lock_policyE2EE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %5, align 8, !tbaa !133
  %8 = getelementptr inbounds %"class.std::_Sp_counted_ptr_inplace", %"class.std::_Sp_counted_ptr_inplace"* %0, i64 0, i32 1, i32 0
  %9 = bitcast %"struct.__gnu_cxx::__aligned_buffer"* %8 to i32 (...)***
  %10 = getelementptr inbounds %"class.std::_Sp_counted_ptr_inplace", %"class.std::_Sp_counted_ptr_inplace"* %0, i64 0, i32 1, i32 0, i32 0, i32 0, i64 8
  %11 = bitcast i8* %10 to %"struct.std::__future_base::_Result_base"**
  store %"struct.std::__future_base::_Result_base"* null, %"struct.std::__future_base::_Result_base"** %11, align 8, !tbaa !181
  %12 = getelementptr inbounds %"class.std::_Sp_counted_ptr_inplace", %"class.std::_Sp_counted_ptr_inplace"* %0, i64 0, i32 1, i32 0, i32 0, i32 0, i64 16
  %13 = bitcast i8* %12 to i32*
  store i32 0, i32* %13, align 4, !tbaa !183
  %14 = getelementptr inbounds %"class.std::_Sp_counted_ptr_inplace", %"class.std::_Sp_counted_ptr_inplace"* %0, i64 0, i32 1, i32 0, i32 0, i32 0, i64 20
  store i8 0, i8* %14, align 1, !tbaa !185
  %15 = getelementptr inbounds %"class.std::_Sp_counted_ptr_inplace", %"class.std::_Sp_counted_ptr_inplace"* %0, i64 0, i32 1, i32 0, i32 0, i32 0, i64 24
  %16 = bitcast i8* %15 to i32*
  store i32 0, i32* %16, align 4, !tbaa !187
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [9 x i8*] }, { [9 x i8*] }* @_ZTVNSt13__future_base16_Task_state_baseIFvvEEE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %9, align 8, !tbaa !133
  %17 = getelementptr inbounds %"class.std::_Sp_counted_ptr_inplace", %"class.std::_Sp_counted_ptr_inplace"* %0, i64 0, i32 1, i32 0, i32 0, i32 0, i64 32
  %18 = invoke i8* @_Znwm(i64 16) #24
          to label %19 unwind label %21

; <label>:19:                                     ; preds = %4
  %20 = bitcast i8* %18 to %"struct.std::__future_base::_Result_base"*
  invoke void @_ZNSt13__future_base12_Result_baseC2Ev(%"struct.std::__future_base::_Result_base"* nonnull %20)
          to label %34 unwind label %23, !noalias !281

; <label>:21:                                     ; preds = %4
  %22 = landingpad { i8*, i32 }
          cleanup
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [6 x i8*] }, { [6 x i8*] }* @_ZTVNSt13__future_base13_State_baseV2E, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %9, align 8, !tbaa !133
  br label %51

; <label>:23:                                     ; preds = %19
  %24 = landingpad { i8*, i32 }
          cleanup
  tail call void @_ZdlPv(i8* nonnull %18) #25, !noalias !281
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
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [5 x i8*] }, { [5 x i8*] }* @_ZTVNSt13__future_base7_ResultIvEE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %35, align 8, !tbaa !133, !noalias !281
  %36 = bitcast i8* %17 to i8**
  store i8* %18, i8** %36, align 8, !tbaa !27, !alias.scope !281
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [9 x i8*] }, { [9 x i8*] }* @_ZTVNSt13__future_base11_Task_stateISt5_BindIFPFvPvS2_ES2_S2_EESaIiEFvvEEE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %9, align 8, !tbaa !133
  %37 = getelementptr inbounds %"class.std::_Sp_counted_ptr_inplace", %"class.std::_Sp_counted_ptr_inplace"* %0, i64 0, i32 1, i32 0, i32 0, i32 0, i64 40
  %38 = bitcast %"class.std::_Bind"* %2 to i64*
  %39 = load i64, i64* %38, align 8, !tbaa !27
  %40 = bitcast i8* %37 to i64*
  store i64 %39, i64* %40, align 8, !tbaa !193
  %41 = getelementptr inbounds %"class.std::_Sp_counted_ptr_inplace", %"class.std::_Sp_counted_ptr_inplace"* %0, i64 0, i32 1, i32 0, i32 0, i32 0, i64 48
  %42 = getelementptr inbounds %"class.std::_Bind", %"class.std::_Bind"* %2, i64 0, i32 1
  %43 = bitcast %"class.std::tuple.102"* %42 to i64*
  %44 = load i64, i64* %43, align 8, !tbaa !27
  %45 = bitcast i8* %41 to i64*
  store i64 %44, i64* %45, align 8, !tbaa !196
  %46 = getelementptr inbounds %"class.std::_Sp_counted_ptr_inplace", %"class.std::_Sp_counted_ptr_inplace"* %0, i64 0, i32 1, i32 0, i32 0, i32 0, i64 56
  %47 = getelementptr inbounds %"class.std::_Bind", %"class.std::_Bind"* %2, i64 0, i32 1, i32 0, i32 1, i32 0
  %48 = bitcast i8** %47 to i64*
  %49 = load i64, i64* %48, align 8, !tbaa !27
  %50 = bitcast i8* %46 to i64*
  store i64 %49, i64* %50, align 8, !tbaa !198
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
  store i64 %7, i64* %8, align 8, !tbaa !181
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
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %9, i8* nonnull %10, i64 16, i32 8, i1 false), !tbaa.struct !284
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
  store i8 0, i8* %5, align 1, !tbaa !269
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
  %31 = load i8, i8* %5, align 1, !tbaa !269, !range !43
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
  %45 = load i64, i64* %44, align 8, !tbaa !285
  %46 = bitcast i8* %12 to i64*
  store i64 %45, i64* %46, align 8, !tbaa !285
  %47 = getelementptr inbounds %"class.std::shared_ptr", %"class.std::shared_ptr"* %2, i64 0, i32 0, i32 1
  %48 = getelementptr inbounds i8, i8* %10, i64 24
  %49 = bitcast i8* %48 to %"class.std::_Sp_counted_base"**
  %50 = load %"class.std::_Sp_counted_base"*, %"class.std::_Sp_counted_base"** %49, align 8, !tbaa !273
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
  %69 = load i64, i64* %68, align 8, !tbaa !273
  %70 = bitcast i8* %48 to i64*
  store i64 %69, i64* %70, align 8, !tbaa !273
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
  %78 = load %"class.std::_Sp_counted_base"*, %"class.std::_Sp_counted_base"** %77, align 8, !tbaa !273
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
  %7 = load i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)*, i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)** %6, align 8, !tbaa !123, !noalias !287
  %8 = icmp eq i1 (%"union.std::_Any_data"*, %"union.std::_Any_data"*, i32)* %7, null
  br i1 %8, label %9, label %10

; <label>:9:                                      ; preds = %3
  tail call void @_ZSt25__throw_bad_function_callv() #23
  unreachable

; <label>:10:                                     ; preds = %3
  %11 = getelementptr inbounds %"class.std::function.133", %"class.std::function.133"* %1, i64 0, i32 1
  %12 = load void (%"class.std::unique_ptr.58"*, %"union.std::_Any_data"*)*, void (%"class.std::unique_ptr.58"*, %"union.std::_Any_data"*)** %11, align 8, !tbaa !267, !noalias !287
  %13 = getelementptr inbounds %"class.std::function.133", %"class.std::function.133"* %1, i64 0, i32 0, i32 0
  call void %12(%"class.std::unique_ptr.58"* nonnull sret %4, %"union.std::_Any_data"* dereferenceable(16) %13)
  store i8 1, i8* %2, align 1, !tbaa !269
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
  %6 = load %"struct.std::_Bind_simple.127"*, %"struct.std::_Bind_simple.127"** %5, align 8, !tbaa !290
  %7 = getelementptr inbounds %"struct.std::_Bind_simple.127", %"struct.std::_Bind_simple.127"* %6, i64 0, i32 0, i32 0, i32 0, i32 0, i32 0
  %8 = load %"class.std::_Bind"*, %"class.std::_Bind"** %7, align 8, !tbaa !292
  %9 = getelementptr inbounds %"class.std::_Bind", %"class.std::_Bind"* %8, i64 0, i32 0
  %10 = load void (i8*, i8*)*, void (i8*, i8*)** %9, align 8, !tbaa !193
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
  %28 = load %"class.std::unique_ptr.109"*, %"class.std::unique_ptr.109"** %27, align 8, !tbaa !294
  %29 = getelementptr inbounds %"class.std::unique_ptr.109", %"class.std::unique_ptr.109"* %28, i64 0, i32 0, i32 0, i32 0, i32 0
  %30 = load %"struct.std::__future_base::_Result"*, %"struct.std::__future_base::_Result"** %29, align 8, !tbaa !27
  %31 = getelementptr inbounds %"struct.std::__future_base::_Result", %"struct.std::__future_base::_Result"* %30, i64 0, i32 0, i32 1
  %32 = bitcast %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* %3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %32) #8
  %33 = getelementptr inbounds %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider", %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* %4, i64 0, i32 0
  %34 = bitcast %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* %4 to i64*
  %35 = load i64, i64* %34, align 8, !tbaa !253
  %36 = bitcast %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider"* %3 to i64*
  store i64 %35, i64* %36, align 8, !tbaa !253
  store i8* null, i8** %33, align 8, !tbaa !253
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
  %42 = load %"class.std::unique_ptr.109"*, %"class.std::unique_ptr.109"** %41, align 8, !tbaa !294
  %43 = getelementptr inbounds %"class.std::unique_ptr.109", %"class.std::unique_ptr.109"* %42, i64 0, i32 0, i32 0, i32 0, i32 0
  %44 = bitcast %"class.std::unique_ptr.109"* %42 to i64*
  %45 = load i64, i64* %44, align 8, !tbaa !27
  store %"struct.std::__future_base::_Result"* null, %"struct.std::__future_base::_Result"** %43, align 8, !tbaa !27
  %46 = bitcast %"class.std::unique_ptr.109"* %0 to i64*
  store i64 %45, i64* %46, align 8, !tbaa !295
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
  %4 = load i8*, i8** %3, align 8, !tbaa !297
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
  br i1 %27, label %21, label %53

; <label>:28:                                     ; preds = %21
  %29 = landingpad { i8*, i32 }
          catch i8* null
  %30 = extractvalue { i8*, i32 } %29, 0
  %31 = tail call i8* @__cxa_begin_catch(i8* %30) #8
  %32 = icmp ugt i64** %22, %19
  br i1 %32, label %.preheader, label %.loopexit

.preheader:                                       ; preds = %28
  br label %33

; <label>:33:                                     ; preds = %.preheader, %33
  %34 = phi i64** [ %37, %33 ], [ %19, %.preheader ]
  %35 = bitcast i64** %34 to i8**
  %36 = load i8*, i8** %35, align 8, !tbaa !27
  tail call void @_ZdlPv(i8* %36) #8
  %37 = getelementptr inbounds i64*, i64** %34, i64 1
  %38 = icmp ult i64** %37, %22
  br i1 %38, label %33, label %.loopexit

.loopexit:                                        ; preds = %33, %28
  invoke void @__cxa_rethrow() #23
          to label %44 unwind label %39

; <label>:39:                                     ; preds = %.loopexit
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

; <label>:44:                                     ; preds = %.loopexit
  unreachable

; <label>:45:                                     ; preds = %39
  %46 = extractvalue { i8*, i32 } %40, 0
  %47 = tail call i8* @__cxa_begin_catch(i8* %46) #8
  %48 = load i8*, i8** %15, align 8, !tbaa !92
  tail call void @_ZdlPv(i8* %48) #8
  %49 = bitcast %"class.std::_Deque_base.22"* %0 to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %49, i8 0, i64 16, i32 8, i1 false)
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
  %54 = getelementptr inbounds %"class.std::_Deque_base.22", %"class.std::_Deque_base.22"* %0, i64 0, i32 0, i32 2
  %55 = getelementptr inbounds %"class.std::_Deque_base.22", %"class.std::_Deque_base.22"* %0, i64 0, i32 0, i32 2, i32 3
  store i64** %19, i64*** %55, align 8, !tbaa !93
  %56 = load i64*, i64** %19, align 8, !tbaa !27
  %57 = getelementptr inbounds %"class.std::_Deque_base.22", %"class.std::_Deque_base.22"* %0, i64 0, i32 0, i32 2, i32 1
  store i64* %56, i64** %57, align 8, !tbaa !94
  %58 = getelementptr inbounds i64, i64* %56, i64 64
  %59 = getelementptr inbounds %"class.std::_Deque_base.22", %"class.std::_Deque_base.22"* %0, i64 0, i32 0, i32 2, i32 2
  store i64* %58, i64** %59, align 8, !tbaa !95
  %60 = getelementptr inbounds i64*, i64** %20, i64 -1
  %61 = getelementptr inbounds %"class.std::_Deque_base.22", %"class.std::_Deque_base.22"* %0, i64 0, i32 0, i32 3, i32 3
  store i64** %60, i64*** %61, align 8, !tbaa !93
  %62 = load i64*, i64** %60, align 8, !tbaa !27
  %63 = getelementptr inbounds %"class.std::_Deque_base.22", %"class.std::_Deque_base.22"* %0, i64 0, i32 0, i32 3, i32 1
  store i64* %62, i64** %63, align 8, !tbaa !94
  %64 = getelementptr inbounds i64, i64* %62, i64 64
  %65 = getelementptr inbounds %"class.std::_Deque_base.22", %"class.std::_Deque_base.22"* %0, i64 0, i32 0, i32 3, i32 2
  store i64* %64, i64** %65, align 8, !tbaa !95
  %66 = ptrtoint i64* %56 to i64
  %67 = bitcast %"struct.std::_Deque_iterator.26"* %54 to i64*
  store i64 %66, i64* %67, align 8, !tbaa !102
  %68 = and i64 %1, 63
  %69 = getelementptr inbounds i64, i64* %62, i64 %68
  %70 = getelementptr inbounds %"class.std::_Deque_base.22", %"class.std::_Deque_base.22"* %0, i64 0, i32 0, i32 3, i32 0
  store i64* %69, i64** %70, align 8, !tbaa !85
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
  br i1 %27, label %21, label %53

; <label>:28:                                     ; preds = %21
  %29 = landingpad { i8*, i32 }
          catch i8* null
  %30 = extractvalue { i8*, i32 } %29, 0
  %31 = tail call i8* @__cxa_begin_catch(i8* %30) #8
  %32 = icmp ugt i32** %22, %19
  br i1 %32, label %.preheader, label %.loopexit

.preheader:                                       ; preds = %28
  br label %33

; <label>:33:                                     ; preds = %.preheader, %33
  %34 = phi i32** [ %37, %33 ], [ %19, %.preheader ]
  %35 = bitcast i32** %34 to i8**
  %36 = load i8*, i8** %35, align 8, !tbaa !27
  tail call void @_ZdlPv(i8* %36) #8
  %37 = getelementptr inbounds i32*, i32** %34, i64 1
  %38 = icmp ult i32** %37, %22
  br i1 %38, label %33, label %.loopexit

.loopexit:                                        ; preds = %33, %28
  invoke void @__cxa_rethrow() #23
          to label %44 unwind label %39

; <label>:39:                                     ; preds = %.loopexit
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

; <label>:44:                                     ; preds = %.loopexit
  unreachable

; <label>:45:                                     ; preds = %39
  %46 = extractvalue { i8*, i32 } %40, 0
  %47 = tail call i8* @__cxa_begin_catch(i8* %46) #8
  %48 = load i8*, i8** %15, align 8, !tbaa !72
  tail call void @_ZdlPv(i8* %48) #8
  %49 = bitcast %"class.std::_Deque_base.14"* %0 to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %49, i8 0, i64 16, i32 8, i1 false)
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
  %54 = getelementptr inbounds %"class.std::_Deque_base.14", %"class.std::_Deque_base.14"* %0, i64 0, i32 0, i32 2
  %55 = getelementptr inbounds %"class.std::_Deque_base.14", %"class.std::_Deque_base.14"* %0, i64 0, i32 0, i32 2, i32 3
  store i32** %19, i32*** %55, align 8, !tbaa !73
  %56 = load i32*, i32** %19, align 8, !tbaa !27
  %57 = getelementptr inbounds %"class.std::_Deque_base.14", %"class.std::_Deque_base.14"* %0, i64 0, i32 0, i32 2, i32 1
  store i32* %56, i32** %57, align 8, !tbaa !74
  %58 = getelementptr inbounds i32, i32* %56, i64 128
  %59 = getelementptr inbounds %"class.std::_Deque_base.14", %"class.std::_Deque_base.14"* %0, i64 0, i32 0, i32 2, i32 2
  store i32* %58, i32** %59, align 8, !tbaa !75
  %60 = getelementptr inbounds i32*, i32** %20, i64 -1
  %61 = getelementptr inbounds %"class.std::_Deque_base.14", %"class.std::_Deque_base.14"* %0, i64 0, i32 0, i32 3, i32 3
  store i32** %60, i32*** %61, align 8, !tbaa !73
  %62 = load i32*, i32** %60, align 8, !tbaa !27
  %63 = getelementptr inbounds %"class.std::_Deque_base.14", %"class.std::_Deque_base.14"* %0, i64 0, i32 0, i32 3, i32 1
  store i32* %62, i32** %63, align 8, !tbaa !74
  %64 = getelementptr inbounds i32, i32* %62, i64 128
  %65 = getelementptr inbounds %"class.std::_Deque_base.14", %"class.std::_Deque_base.14"* %0, i64 0, i32 0, i32 3, i32 2
  store i32* %64, i32** %65, align 8, !tbaa !75
  %66 = ptrtoint i32* %56 to i64
  %67 = bitcast %"struct.std::_Deque_iterator.18"* %54 to i64*
  store i64 %66, i64* %67, align 8, !tbaa !82
  %68 = and i64 %1, 127
  %69 = getelementptr inbounds i32, i32* %62, i64 %68
  %70 = getelementptr inbounds %"class.std::_Deque_base.14", %"class.std::_Deque_base.14"* %0, i64 0, i32 0, i32 3, i32 0
  store i32* %69, i32** %70, align 8, !tbaa !65
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
  br i1 %27, label %21, label %53

; <label>:28:                                     ; preds = %21
  %29 = landingpad { i8*, i32 }
          catch i8* null
  %30 = extractvalue { i8*, i32 } %29, 0
  %31 = tail call i8* @__cxa_begin_catch(i8* %30) #8
  %32 = icmp ugt i16** %22, %19
  br i1 %32, label %.preheader, label %.loopexit

.preheader:                                       ; preds = %28
  br label %33

; <label>:33:                                     ; preds = %.preheader, %33
  %34 = phi i16** [ %37, %33 ], [ %19, %.preheader ]
  %35 = bitcast i16** %34 to i8**
  %36 = load i8*, i8** %35, align 8, !tbaa !27
  tail call void @_ZdlPv(i8* %36) #8
  %37 = getelementptr inbounds i16*, i16** %34, i64 1
  %38 = icmp ult i16** %37, %22
  br i1 %38, label %33, label %.loopexit

.loopexit:                                        ; preds = %33, %28
  invoke void @__cxa_rethrow() #23
          to label %44 unwind label %39

; <label>:39:                                     ; preds = %.loopexit
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

; <label>:44:                                     ; preds = %.loopexit
  unreachable

; <label>:45:                                     ; preds = %39
  %46 = extractvalue { i8*, i32 } %40, 0
  %47 = tail call i8* @__cxa_begin_catch(i8* %46) #8
  %48 = load i8*, i8** %15, align 8, !tbaa !53
  tail call void @_ZdlPv(i8* %48) #8
  %49 = bitcast %"class.std::_Deque_base.6"* %0 to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %49, i8 0, i64 16, i32 8, i1 false)
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
  %54 = getelementptr inbounds %"class.std::_Deque_base.6", %"class.std::_Deque_base.6"* %0, i64 0, i32 0, i32 2
  %55 = getelementptr inbounds %"class.std::_Deque_base.6", %"class.std::_Deque_base.6"* %0, i64 0, i32 0, i32 2, i32 3
  store i16** %19, i16*** %55, align 8, !tbaa !54
  %56 = load i16*, i16** %19, align 8, !tbaa !27
  %57 = getelementptr inbounds %"class.std::_Deque_base.6", %"class.std::_Deque_base.6"* %0, i64 0, i32 0, i32 2, i32 1
  store i16* %56, i16** %57, align 8, !tbaa !55
  %58 = getelementptr inbounds i16, i16* %56, i64 256
  %59 = getelementptr inbounds %"class.std::_Deque_base.6", %"class.std::_Deque_base.6"* %0, i64 0, i32 0, i32 2, i32 2
  store i16* %58, i16** %59, align 8, !tbaa !56
  %60 = getelementptr inbounds i16*, i16** %20, i64 -1
  %61 = getelementptr inbounds %"class.std::_Deque_base.6", %"class.std::_Deque_base.6"* %0, i64 0, i32 0, i32 3, i32 3
  store i16** %60, i16*** %61, align 8, !tbaa !54
  %62 = load i16*, i16** %60, align 8, !tbaa !27
  %63 = getelementptr inbounds %"class.std::_Deque_base.6", %"class.std::_Deque_base.6"* %0, i64 0, i32 0, i32 3, i32 1
  store i16* %62, i16** %63, align 8, !tbaa !55
  %64 = getelementptr inbounds i16, i16* %62, i64 256
  %65 = getelementptr inbounds %"class.std::_Deque_base.6", %"class.std::_Deque_base.6"* %0, i64 0, i32 0, i32 3, i32 2
  store i16* %64, i16** %65, align 8, !tbaa !56
  %66 = ptrtoint i16* %56 to i64
  %67 = bitcast %"struct.std::_Deque_iterator.10"* %54 to i64*
  store i64 %66, i64* %67, align 8, !tbaa !63
  %68 = and i64 %1, 255
  %69 = getelementptr inbounds i16, i16* %62, i64 %68
  %70 = getelementptr inbounds %"class.std::_Deque_base.6", %"class.std::_Deque_base.6"* %0, i64 0, i32 0, i32 3, i32 0
  store i16* %69, i16** %70, align 8, !tbaa !46
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
  br i1 %26, label %21, label %51

; <label>:27:                                     ; preds = %21
  %28 = landingpad { i8*, i32 }
          catch i8* null
  %29 = extractvalue { i8*, i32 } %28, 0
  %30 = tail call i8* @__cxa_begin_catch(i8* %29) #8
  %31 = icmp ugt i8** %22, %19
  br i1 %31, label %.preheader, label %.loopexit

.preheader:                                       ; preds = %27
  br label %32

; <label>:32:                                     ; preds = %.preheader, %32
  %33 = phi i8** [ %35, %32 ], [ %19, %.preheader ]
  %34 = load i8*, i8** %33, align 8, !tbaa !27
  tail call void @_ZdlPv(i8* %34) #8
  %35 = getelementptr inbounds i8*, i8** %33, i64 1
  %36 = icmp ult i8** %35, %22
  br i1 %36, label %32, label %.loopexit

.loopexit:                                        ; preds = %32, %27
  invoke void @__cxa_rethrow() #23
          to label %42 unwind label %37

; <label>:37:                                     ; preds = %.loopexit
  %38 = landingpad { i8*, i32 }
          catch i8* null
  invoke void @__cxa_end_catch()
          to label %43 unwind label %39

; <label>:39:                                     ; preds = %37
  %40 = landingpad { i8*, i32 }
          catch i8* null
  %41 = extractvalue { i8*, i32 } %40, 0
  tail call void @__clang_call_terminate(i8* %41) #22
  unreachable

; <label>:42:                                     ; preds = %.loopexit
  unreachable

; <label>:43:                                     ; preds = %37
  %44 = extractvalue { i8*, i32 } %38, 0
  %45 = tail call i8* @__cxa_begin_catch(i8* %44) #8
  %46 = load i8*, i8** %15, align 8, !tbaa !26
  tail call void @_ZdlPv(i8* %46) #8
  %47 = bitcast %"class.std::_Deque_base"* %0 to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %47, i8 0, i64 16, i32 8, i1 false)
  invoke void @__cxa_rethrow() #23
          to label %72 unwind label %48

; <label>:48:                                     ; preds = %43
  %49 = landingpad { i8*, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %50 unwind label %69

; <label>:50:                                     ; preds = %48
  resume { i8*, i32 } %49

; <label>:51:                                     ; preds = %24
  %52 = getelementptr inbounds %"class.std::_Deque_base", %"class.std::_Deque_base"* %0, i64 0, i32 0, i32 2
  %53 = getelementptr inbounds %"class.std::_Deque_base", %"class.std::_Deque_base"* %0, i64 0, i32 0, i32 2, i32 3
  store i8** %19, i8*** %53, align 8, !tbaa !28
  %54 = load i8*, i8** %19, align 8, !tbaa !27
  %55 = getelementptr inbounds %"class.std::_Deque_base", %"class.std::_Deque_base"* %0, i64 0, i32 0, i32 2, i32 1
  store i8* %54, i8** %55, align 8, !tbaa !29
  %56 = getelementptr inbounds i8, i8* %54, i64 512
  %57 = getelementptr inbounds %"class.std::_Deque_base", %"class.std::_Deque_base"* %0, i64 0, i32 0, i32 2, i32 2
  store i8* %56, i8** %57, align 8, !tbaa !30
  %58 = getelementptr inbounds i8*, i8** %20, i64 -1
  %59 = getelementptr inbounds %"class.std::_Deque_base", %"class.std::_Deque_base"* %0, i64 0, i32 0, i32 3, i32 3
  store i8** %58, i8*** %59, align 8, !tbaa !28
  %60 = load i8*, i8** %58, align 8, !tbaa !27
  %61 = getelementptr inbounds %"class.std::_Deque_base", %"class.std::_Deque_base"* %0, i64 0, i32 0, i32 3, i32 1
  store i8* %60, i8** %61, align 8, !tbaa !29
  %62 = getelementptr inbounds i8, i8* %60, i64 512
  %63 = getelementptr inbounds %"class.std::_Deque_base", %"class.std::_Deque_base"* %0, i64 0, i32 0, i32 3, i32 2
  store i8* %62, i8** %63, align 8, !tbaa !30
  %64 = ptrtoint i8* %54 to i64
  %65 = bitcast %"struct.std::_Deque_iterator"* %52 to i64*
  store i64 %64, i64* %65, align 8, !tbaa !40
  %66 = and i64 %1, 511
  %67 = getelementptr inbounds i8, i8* %60, i64 %66
  %68 = getelementptr inbounds %"class.std::_Deque_base", %"class.std::_Deque_base"* %0, i64 0, i32 0, i32 3, i32 0
  store i8* %67, i8** %68, align 8, !tbaa !19
  ret void

; <label>:69:                                     ; preds = %48
  %70 = landingpad { i8*, i32 }
          catch i8* null
  %71 = extractvalue { i8*, i32 } %70, 0
  tail call void @__clang_call_terminate(i8* %71) #22
  unreachable

; <label>:72:                                     ; preds = %43
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
  %13 = load %"class.std::unique_ptr"**, %"class.std::unique_ptr"*** %12, align 16, !tbaa !214
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
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %24, i8* nonnull %25, i64 32, i32 8, i1 false) #8, !tbaa.struct !168
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
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %36, i8* nonnull %37, i64 32, i32 8, i1 false) #8, !tbaa.struct !168
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
  %13 = load %"class.std::function"**, %"class.std::function"*** %12, align 16, !tbaa !241
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
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %24, i8* nonnull %25, i64 32, i32 8, i1 false) #8, !tbaa.struct !168
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
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %36, i8* nonnull %37, i64 32, i32 8, i1 false) #8, !tbaa.struct !168
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
  br i1 %7, label %8, label %.loopexit

; <label>:8:                                      ; preds = %2
  %9 = getelementptr inbounds %"struct.std::atomic", %"struct.std::atomic"* %1, i64 0, i32 0, i32 0
  %10 = bitcast %"class.std::unique_ptr"* %3 to i8*
  %11 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %3, i64 0, i32 0, i32 0, i32 0, i32 0
  %12 = getelementptr inbounds %"class.MARC::ThreadPool", %"class.MARC::ThreadPool"* %0, i64 0, i32 1
  br label %13

; <label>:13:                                     ; preds = %8, %39
  store atomic i8 1, i8* %9 seq_cst, align 1
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %10) #8
  store %"class.MARC::IThreadTask"* null, %"class.MARC::IThreadTask"** %11, align 8, !tbaa !210
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
  br i1 %42, label %13, label %.loopexit

.loopexit:                                        ; preds = %39, %2
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
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %21, i8* nonnull %22, i64 16, i32 8, i1 false) #8, !tbaa.struct !249
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
  %42 = load i64, i64* %41, align 8, !tbaa !245
  %43 = bitcast %"class.std::deque.47"* %37 to i64*
  %44 = load i64, i64* %43, align 8, !tbaa !241
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
  %54 = load %"class.std::function"**, %"class.std::function"*** %40, align 8, !tbaa !245
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
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull %61, i8* nonnull %62, i64 16, i32 8, i1 false) #8, !tbaa.struct !249
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
  %74 = load %"class.std::function"**, %"class.std::function"*** %40, align 8, !tbaa !245
  %75 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %74, i64 1
  store %"class.std::function"** %75, %"class.std::function"*** %40, align 8, !tbaa !220
  %76 = load %"class.std::function"*, %"class.std::function"** %75, align 8, !tbaa !27
  %77 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.45", %"class.MARC::ThreadSafeQueue.45"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 1
  store %"class.std::function"* %76, %"class.std::function"** %77, align 8, !tbaa !223
  %78 = getelementptr inbounds %"class.std::function", %"class.std::function"* %76, i64 16
  store %"class.std::function"* %78, %"class.std::function"** %14, align 8, !tbaa !224
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
  %5 = load %"class.std::function"**, %"class.std::function"*** %4, align 8, !tbaa !245
  %6 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %0, i64 0, i32 0, i32 0, i32 2, i32 3
  %7 = load %"class.std::function"**, %"class.std::function"*** %6, align 8, !tbaa !244
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
  %20 = load %"class.std::function"**, %"class.std::function"*** %19, align 8, !tbaa !241
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
  %60 = load %"class.std::function"**, %"class.std::function"*** %6, align 8, !tbaa !244
  %61 = load %"class.std::function"**, %"class.std::function"*** %4, align 8, !tbaa !245
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
  %72 = load i8*, i8** %71, align 8, !tbaa !241
  tail call void @_ZdlPv(i8* %72) #8
  store i8* %53, i8** %71, align 8, !tbaa !241
  store i64 %48, i64* %14, align 8, !tbaa !301
  br label %73

; <label>:73:                                     ; preds = %38, %35, %32, %30, %70
  %74 = phi %"class.std::function"** [ %59, %70 ], [ %25, %30 ], [ %25, %32 ], [ %25, %35 ], [ %25, %38 ]
  store %"class.std::function"** %74, %"class.std::function"*** %6, align 8, !tbaa !220
  %75 = load %"class.std::function"*, %"class.std::function"** %74, align 8, !tbaa !27
  %76 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %0, i64 0, i32 0, i32 0, i32 2, i32 1
  store %"class.std::function"* %75, %"class.std::function"** %76, align 8, !tbaa !223
  %77 = getelementptr inbounds %"class.std::function", %"class.std::function"* %75, i64 16
  %78 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %0, i64 0, i32 0, i32 0, i32 2, i32 2
  store %"class.std::function"* %77, %"class.std::function"** %78, align 8, !tbaa !224
  %79 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %74, i64 %11
  store %"class.std::function"** %79, %"class.std::function"*** %4, align 8, !tbaa !220
  %80 = load %"class.std::function"*, %"class.std::function"** %79, align 8, !tbaa !27
  %81 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %0, i64 0, i32 0, i32 0, i32 3, i32 1
  store %"class.std::function"* %80, %"class.std::function"** %81, align 8, !tbaa !223
  %82 = getelementptr inbounds %"class.std::function", %"class.std::function"* %80, i64 16
  %83 = getelementptr inbounds %"class.std::deque.47", %"class.std::deque.47"* %0, i64 0, i32 0, i32 0, i32 3, i32 2
  store %"class.std::function"* %82, %"class.std::function"** %83, align 8, !tbaa !224
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
  %8 = load i64, i64* %7, align 8, !tbaa !151
  %9 = bitcast %"class.std::vector"* %0 to i64*
  %10 = load i64, i64* %9, align 8, !tbaa !167
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
  %28 = load i64, i64* %7, align 8, !tbaa !151
  %29 = load i64, i64* %9, align 8, !tbaa !167
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
  store i64 0, i64* %39, align 8, !tbaa !155
  %40 = bitcast { i64, i64 }* %1 to <2 x i64>*
  %41 = load <2 x i64>, <2 x i64>* %40, align 8, !tbaa !18, !noalias !302
  %42 = bitcast %"struct.std::atomic"** %3 to i64*
  %43 = load i64, i64* %42, align 8, !tbaa !27, !noalias !302
  %44 = bitcast %"class.MARC::ThreadPool"** %2 to i64*
  %45 = load i64, i64* %44, align 8, !tbaa !27, !noalias !302
  %46 = invoke i8* @_Znwm(i64 40) #24
          to label %47 unwind label %209

; <label>:47:                                     ; preds = %30
  %48 = bitcast i8* %46 to i32 (...)***
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [5 x i8*] }, { [5 x i8*] }* @_ZTVNSt6thread11_State_implISt12_Bind_simpleIFSt7_Mem_fnIMN4MARC10ThreadPoolEFvPSt6atomicIbEEEPS4_S7_EEEE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %48, align 8, !tbaa !133, !noalias !305
  %49 = getelementptr inbounds i8, i8* %46, i64 8
  %50 = bitcast i8* %49 to i64*
  store i64 %43, i64* %50, align 8, !tbaa !163, !noalias !305
  %51 = getelementptr inbounds i8, i8* %46, i64 16
  %52 = bitcast i8* %51 to i64*
  store i64 %45, i64* %52, align 8, !tbaa !165, !noalias !305
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
  br i1 %69, label %211, label %70

; <label>:70:                                     ; preds = %65
  %71 = bitcast %"class.MARC::IThreadTask"* %68 to void (%"class.MARC::IThreadTask"*)***
  %72 = load void (%"class.MARC::IThreadTask"*)**, void (%"class.MARC::IThreadTask"*)*** %71, align 8, !tbaa !133
  %73 = getelementptr inbounds void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %72, i64 1
  %74 = load void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %73, align 8
  call void %74(%"class.MARC::IThreadTask"* nonnull %68) #8
  br label %211

; <label>:75:                                     ; preds = %60, %56
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %38)
  %76 = getelementptr inbounds %"class.std::vector", %"class.std::vector"* %0, i64 0, i32 0, i32 0, i32 0
  %77 = load %"class.std::thread"*, %"class.std::thread"** %76, align 8, !tbaa !167
  %78 = bitcast %"class.std::thread"* %77 to i8*
  %79 = load %"class.std::thread"*, %"class.std::thread"** %6, align 8, !tbaa !151
  %80 = icmp eq %"class.std::thread"* %77, %79
  br i1 %80, label %.loopexit, label %81

; <label>:81:                                     ; preds = %75
  %82 = ptrtoint %"class.std::thread"* %77 to i64
  %83 = getelementptr %"class.std::thread", %"class.std::thread"* %79, i64 -1, i32 0, i32 0
  %84 = ptrtoint i64* %83 to i64
  %85 = sub i64 %84, %82
  %86 = lshr i64 %85, 3
  %87 = add nuw nsw i64 %86, 1
  %88 = icmp ult i64 %87, 4
  br i1 %88, label %149, label %89

; <label>:89:                                     ; preds = %81
  %90 = getelementptr %"class.std::thread", %"class.std::thread"* %34, i64 %87
  %91 = getelementptr %"class.std::thread", %"class.std::thread"* %77, i64 %87
  %92 = bitcast %"class.std::thread"* %91 to i8*
  %93 = icmp ult i8* %33, %92
  %94 = icmp ult %"class.std::thread"* %77, %90
  %95 = and i1 %94, %93
  br i1 %95, label %149, label %96

; <label>:96:                                     ; preds = %89
  %97 = and i64 %87, 4611686018427387900
  %98 = getelementptr %"class.std::thread", %"class.std::thread"* %34, i64 %97
  %99 = getelementptr %"class.std::thread", %"class.std::thread"* %77, i64 %97
  %100 = add nsw i64 %97, -4
  %101 = lshr exact i64 %100, 2
  %102 = add nuw nsw i64 %101, 1
  %103 = and i64 %102, 1
  %104 = icmp eq i64 %101, 0
  br i1 %104, label %.loopexit3, label %105

; <label>:105:                                    ; preds = %96
  %106 = sub nsw i64 %102, %103
  br label %107

; <label>:107:                                    ; preds = %107, %105
  %108 = phi i64 [ 0, %105 ], [ %131, %107 ]
  %109 = phi i64 [ %106, %105 ], [ %132, %107 ]
  %110 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %34, i64 %108, i32 0, i32 0
  %111 = bitcast i64* %110 to <2 x i64>*
  %112 = getelementptr i64, i64* %110, i64 2
  %113 = bitcast i64* %112 to <2 x i64>*
  %114 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %77, i64 %108, i32 0, i32 0
  %115 = bitcast i64* %114 to <2 x i64>*
  %116 = load <2 x i64>, <2 x i64>* %115, align 8, !tbaa !84, !alias.scope !308
  %117 = getelementptr i64, i64* %114, i64 2
  %118 = bitcast i64* %117 to <2 x i64>*
  %119 = load <2 x i64>, <2 x i64>* %118, align 8, !tbaa !84, !alias.scope !308
  store <2 x i64> %116, <2 x i64>* %111, align 8, !tbaa !84, !alias.scope !311, !noalias !308
  store <2 x i64> %119, <2 x i64>* %113, align 8, !tbaa !84, !alias.scope !311, !noalias !308
  store <2 x i64> zeroinitializer, <2 x i64>* %115, align 8, !tbaa !84, !alias.scope !308
  store <2 x i64> zeroinitializer, <2 x i64>* %118, align 8, !tbaa !84, !alias.scope !308
  %120 = or i64 %108, 4
  %121 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %34, i64 %120, i32 0, i32 0
  %122 = bitcast i64* %121 to <2 x i64>*
  %123 = getelementptr i64, i64* %121, i64 2
  %124 = bitcast i64* %123 to <2 x i64>*
  %125 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %77, i64 %120, i32 0, i32 0
  %126 = bitcast i64* %125 to <2 x i64>*
  %127 = load <2 x i64>, <2 x i64>* %126, align 8, !tbaa !84, !alias.scope !308
  %128 = getelementptr i64, i64* %125, i64 2
  %129 = bitcast i64* %128 to <2 x i64>*
  %130 = load <2 x i64>, <2 x i64>* %129, align 8, !tbaa !84, !alias.scope !308
  store <2 x i64> %127, <2 x i64>* %122, align 8, !tbaa !84, !alias.scope !311, !noalias !308
  store <2 x i64> %130, <2 x i64>* %124, align 8, !tbaa !84, !alias.scope !311, !noalias !308
  store <2 x i64> zeroinitializer, <2 x i64>* %126, align 8, !tbaa !84, !alias.scope !308
  store <2 x i64> zeroinitializer, <2 x i64>* %129, align 8, !tbaa !84, !alias.scope !308
  %131 = add i64 %108, 8
  %132 = add i64 %109, -2
  %133 = icmp eq i64 %132, 0
  br i1 %133, label %.loopexit3, label %107, !llvm.loop !313

.loopexit3:                                       ; preds = %107, %96
  %134 = phi i64 [ 0, %96 ], [ %131, %107 ]
  %135 = icmp eq i64 %103, 0
  br i1 %135, label %147, label %136

; <label>:136:                                    ; preds = %.loopexit3
  %137 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %34, i64 %134, i32 0, i32 0
  %138 = bitcast i64* %137 to <2 x i64>*
  %139 = getelementptr i64, i64* %137, i64 2
  %140 = bitcast i64* %139 to <2 x i64>*
  %141 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %77, i64 %134, i32 0, i32 0
  %142 = bitcast i64* %141 to <2 x i64>*
  %143 = load <2 x i64>, <2 x i64>* %142, align 8, !tbaa !84, !alias.scope !308
  %144 = getelementptr i64, i64* %141, i64 2
  %145 = bitcast i64* %144 to <2 x i64>*
  %146 = load <2 x i64>, <2 x i64>* %145, align 8, !tbaa !84, !alias.scope !308
  store <2 x i64> %143, <2 x i64>* %138, align 8, !tbaa !84, !alias.scope !311, !noalias !308
  store <2 x i64> %146, <2 x i64>* %140, align 8, !tbaa !84, !alias.scope !311, !noalias !308
  store <2 x i64> zeroinitializer, <2 x i64>* %142, align 8, !tbaa !84, !alias.scope !308
  store <2 x i64> zeroinitializer, <2 x i64>* %145, align 8, !tbaa !84, !alias.scope !308
  br label %147

; <label>:147:                                    ; preds = %.loopexit3, %136
  %148 = icmp eq i64 %87, %97
  br i1 %148, label %.loopexit1, label %149

; <label>:149:                                    ; preds = %147, %89, %81
  %150 = phi %"class.std::thread"* [ %34, %89 ], [ %34, %81 ], [ %98, %147 ]
  %151 = phi %"class.std::thread"* [ %77, %89 ], [ %77, %81 ], [ %99, %147 ]
  %152 = ptrtoint %"class.std::thread"* %151 to i64
  %153 = sub i64 %84, %152
  %154 = lshr i64 %153, 3
  %155 = add nuw nsw i64 %154, 1
  %156 = and i64 %155, 3
  %157 = icmp eq i64 %156, 0
  br i1 %157, label %.loopexit2, label %158

; <label>:158:                                    ; preds = %149
  %scevgep = getelementptr %"class.std::thread", %"class.std::thread"* %151, i64 %156
  br label %159

; <label>:159:                                    ; preds = %159, %158
  %160 = phi %"class.std::thread"* [ %167, %159 ], [ %150, %158 ]
  %161 = phi %"class.std::thread"* [ %166, %159 ], [ %151, %158 ]
  %162 = phi i64 [ %168, %159 ], [ %156, %158 ]
  %163 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %160, i64 0, i32 0, i32 0
  store i64 0, i64* %163, align 8, !tbaa !155
  %164 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %161, i64 0, i32 0, i32 0
  %165 = load i64, i64* %164, align 8, !tbaa !84
  store i64 %165, i64* %163, align 8, !tbaa !84
  store i64 0, i64* %164, align 8, !tbaa !84
  %166 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %161, i64 1
  %167 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %160, i64 1
  %168 = add nsw i64 %162, -1
  %169 = icmp eq i64 %168, 0
  br i1 %169, label %.loopexit2.loopexit, label %159, !llvm.loop !316

.loopexit2.loopexit:                              ; preds = %159
  %scevgep5 = getelementptr %"class.std::thread", %"class.std::thread"* %150, i64 %156
  br label %.loopexit2

.loopexit2:                                       ; preds = %.loopexit2.loopexit, %149
  %170 = phi %"class.std::thread"* [ %150, %149 ], [ %scevgep5, %.loopexit2.loopexit ]
  %171 = phi %"class.std::thread"* [ %151, %149 ], [ %scevgep, %.loopexit2.loopexit ]
  %172 = icmp ult i64 %153, 24
  br i1 %172, label %.loopexit1, label %.preheader

.preheader:                                       ; preds = %.loopexit2
  br label %173

; <label>:173:                                    ; preds = %.preheader, %173
  %174 = phi %"class.std::thread"* [ %189, %173 ], [ %170, %.preheader ]
  %175 = phi %"class.std::thread"* [ %188, %173 ], [ %171, %.preheader ]
  %176 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %174, i64 0, i32 0, i32 0
  store i64 0, i64* %176, align 8, !tbaa !155
  %177 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %175, i64 0, i32 0, i32 0
  %178 = load i64, i64* %177, align 8, !tbaa !84
  store i64 %178, i64* %176, align 8, !tbaa !84
  store i64 0, i64* %177, align 8, !tbaa !84
  %179 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %174, i64 1, i32 0, i32 0
  store i64 0, i64* %179, align 8, !tbaa !155
  %180 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %175, i64 1, i32 0, i32 0
  %181 = load i64, i64* %180, align 8, !tbaa !84
  store i64 %181, i64* %179, align 8, !tbaa !84
  store i64 0, i64* %180, align 8, !tbaa !84
  %182 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %174, i64 2, i32 0, i32 0
  store i64 0, i64* %182, align 8, !tbaa !155
  %183 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %175, i64 2, i32 0, i32 0
  %184 = load i64, i64* %183, align 8, !tbaa !84
  store i64 %184, i64* %182, align 8, !tbaa !84
  store i64 0, i64* %183, align 8, !tbaa !84
  %185 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %174, i64 3, i32 0, i32 0
  store i64 0, i64* %185, align 8, !tbaa !155
  %186 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %175, i64 3, i32 0, i32 0
  %187 = load i64, i64* %186, align 8, !tbaa !84
  store i64 %187, i64* %185, align 8, !tbaa !84
  store i64 0, i64* %186, align 8, !tbaa !84
  %188 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %175, i64 4
  %189 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %174, i64 4
  %190 = icmp eq %"class.std::thread"* %188, %79
  br i1 %190, label %.loopexit1, label %173, !llvm.loop !317

.loopexit1:                                       ; preds = %173, %.loopexit2, %147
  %191 = getelementptr %"class.std::thread", %"class.std::thread"* %34, i64 %87
  br label %194

; <label>:192:                                    ; preds = %194
  %193 = icmp eq %"class.std::thread"* %199, %79
  br i1 %193, label %.loopexit, label %194

; <label>:194:                                    ; preds = %192, %.loopexit1
  %195 = phi %"class.std::thread"* [ %77, %.loopexit1 ], [ %199, %192 ]
  %196 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %195, i64 0, i32 0, i32 0
  %197 = load i64, i64* %196, align 8
  %198 = icmp eq i64 %197, 0
  %199 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %195, i64 1
  br i1 %198, label %192, label %200

; <label>:200:                                    ; preds = %194
  call void @_ZSt9terminatev() #22
  unreachable

.loopexit:                                        ; preds = %192, %75
  %201 = phi %"class.std::thread"* [ %34, %75 ], [ %191, %192 ]
  %202 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %201, i64 1
  %203 = getelementptr inbounds %"class.std::vector", %"class.std::vector"* %0, i64 0, i32 0, i32 0, i32 2
  %204 = icmp eq %"class.std::thread"* %77, null
  br i1 %204, label %206, label %205

; <label>:205:                                    ; preds = %.loopexit
  call void @_ZdlPv(i8* %78) #8
  br label %206

; <label>:206:                                    ; preds = %.loopexit, %205
  %207 = bitcast %"class.std::vector"* %0 to i8**
  store i8* %33, i8** %207, align 8, !tbaa !167
  store %"class.std::thread"* %202, %"class.std::thread"** %6, align 8, !tbaa !151
  %208 = getelementptr inbounds %"class.std::thread", %"class.std::thread"* %34, i64 %19
  store %"class.std::thread"* %208, %"class.std::thread"** %203, align 8, !tbaa !154
  ret void

; <label>:209:                                    ; preds = %30
  %210 = landingpad { i8*, i32 }
          catch i8* null
  br label %211

; <label>:211:                                    ; preds = %65, %70, %209
  %212 = phi { i8*, i32 } [ %210, %209 ], [ %66, %70 ], [ %66, %65 ]
  %213 = extractvalue { i8*, i32 } %212, 0
  %214 = call i8* @__cxa_begin_catch(i8* %213) #8
  %215 = icmp eq %"class.std::thread"* %34, null
  br i1 %215, label %216, label %219

; <label>:216:                                    ; preds = %211
  call void @llvm.trap()
  unreachable

; <label>:217:                                    ; preds = %219
  %218 = landingpad { i8*, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %220 unwind label %221

; <label>:219:                                    ; preds = %211
  call void @_ZdlPv(i8* %33) #8
  invoke void @__cxa_rethrow() #23
          to label %224 unwind label %217

; <label>:220:                                    ; preds = %217
  resume { i8*, i32 } %218

; <label>:221:                                    ; preds = %217
  %222 = landingpad { i8*, i32 }
          catch i8* null
  %223 = extractvalue { i8*, i32 } %222, 0
  call void @__clang_call_terminate(i8* %223) #22
  unreachable

; <label>:224:                                    ; preds = %219
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
  br i1 %17, label %92, label %18

; <label>:18:                                     ; preds = %13
  %19 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 3, i32 0
  %20 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %19, align 8, !tbaa !227
  %21 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 0
  %22 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %21, align 8, !tbaa !227
  %23 = icmp eq %"class.std::unique_ptr"* %20, %22
  br i1 %23, label %24, label %.loopexit

; <label>:24:                                     ; preds = %18
  %25 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 3
  br label %26

; <label>:26:                                     ; preds = %30, %24
  %27 = load atomic i8, i8* %14 seq_cst, align 1
  %28 = and i8 %27, 1
  %29 = icmp eq i8 %28, 0
  br i1 %29, label %.loopexit, label %30

; <label>:30:                                     ; preds = %26
  call void @_ZNSt18condition_variable4waitERSt11unique_lockISt5mutexE(%"class.std::condition_variable"* nonnull %25, %"class.std::unique_lock"* nonnull dereferenceable(16) %3) #8
  %31 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %19, align 8, !tbaa !227
  %32 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %21, align 8, !tbaa !227
  %33 = icmp eq %"class.std::unique_ptr"* %31, %32
  br i1 %33, label %26, label %.loopexit

.loopexit:                                        ; preds = %30, %26, %18
  %34 = load atomic i8, i8* %14 seq_cst, align 1
  %35 = and i8 %34, 1
  %36 = icmp eq i8 %35, 0
  br i1 %36, label %92, label %37

; <label>:37:                                     ; preds = %.loopexit
  %38 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %21, align 8, !tbaa !227, !noalias !318
  %39 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %38, i64 0, i32 0, i32 0, i32 0, i32 0
  %40 = bitcast %"class.std::unique_ptr"* %38 to i64*
  %41 = load i64, i64* %40, align 8, !tbaa !27
  store %"class.MARC::IThreadTask"* null, %"class.MARC::IThreadTask"** %39, align 8, !tbaa !27
  %42 = bitcast %"class.std::unique_ptr"* %1 to i64*
  %43 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %1, i64 0, i32 0, i32 0, i32 0, i32 0
  %44 = load %"class.MARC::IThreadTask"*, %"class.MARC::IThreadTask"** %43, align 8, !tbaa !27
  store i64 %41, i64* %42, align 8, !tbaa !27
  %45 = icmp eq %"class.MARC::IThreadTask"* %44, null
  br i1 %45, label %51, label %46

; <label>:46:                                     ; preds = %37
  %47 = bitcast %"class.MARC::IThreadTask"* %44 to void (%"class.MARC::IThreadTask"*)***
  %48 = load void (%"class.MARC::IThreadTask"*)**, void (%"class.MARC::IThreadTask"*)*** %47, align 8, !tbaa !133
  %49 = getelementptr inbounds void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %48, i64 2
  %50 = load void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %49, align 8
  call void %50(%"class.MARC::IThreadTask"* nonnull %44) #8
  br label %51

; <label>:51:                                     ; preds = %46, %37
  %52 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %21, align 8, !tbaa !321
  %53 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 2
  %54 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %53, align 8, !tbaa !322
  %55 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %54, i64 -1
  %56 = icmp eq %"class.std::unique_ptr"* %52, %55
  br i1 %56, label %69, label %57

; <label>:57:                                     ; preds = %51
  %58 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %52, i64 0, i32 0, i32 0, i32 0, i32 0
  %59 = load %"class.MARC::IThreadTask"*, %"class.MARC::IThreadTask"** %58, align 8, !tbaa !27
  %60 = icmp eq %"class.MARC::IThreadTask"* %59, null
  br i1 %60, label %66, label %61

; <label>:61:                                     ; preds = %57
  %62 = bitcast %"class.MARC::IThreadTask"* %59 to void (%"class.MARC::IThreadTask"*)***
  %63 = load void (%"class.MARC::IThreadTask"*)**, void (%"class.MARC::IThreadTask"*)*** %62, align 8, !tbaa !133
  %64 = getelementptr inbounds void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %63, i64 2
  %65 = load void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %64, align 8
  call void %65(%"class.MARC::IThreadTask"* nonnull %59) #8
  br label %66

; <label>:66:                                     ; preds = %61, %57
  store %"class.MARC::IThreadTask"* null, %"class.MARC::IThreadTask"** %58, align 8, !tbaa !27
  %67 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %21, align 8, !tbaa !321
  %68 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %67, i64 1
  store %"class.std::unique_ptr"* %68, %"class.std::unique_ptr"** %21, align 8, !tbaa !321
  br label %90

; <label>:69:                                     ; preds = %51
  %70 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2
  %71 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %52, i64 0, i32 0, i32 0, i32 0, i32 0
  %72 = load %"class.MARC::IThreadTask"*, %"class.MARC::IThreadTask"** %71, align 8, !tbaa !27
  %73 = icmp eq %"class.MARC::IThreadTask"* %72, null
  br i1 %73, label %79, label %74

; <label>:74:                                     ; preds = %69
  %75 = bitcast %"class.MARC::IThreadTask"* %72 to void (%"class.MARC::IThreadTask"*)***
  %76 = load void (%"class.MARC::IThreadTask"*)**, void (%"class.MARC::IThreadTask"*)*** %75, align 8, !tbaa !133
  %77 = getelementptr inbounds void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %76, i64 2
  %78 = load void (%"class.MARC::IThreadTask"*)*, void (%"class.MARC::IThreadTask"*)** %77, align 8
  call void %78(%"class.MARC::IThreadTask"* nonnull %72) #8
  br label %79

; <label>:79:                                     ; preds = %74, %69
  store %"class.MARC::IThreadTask"* null, %"class.MARC::IThreadTask"** %71, align 8, !tbaa !27
  %80 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 1
  %81 = bitcast %"class.std::unique_ptr"** %80 to i8**
  %82 = load i8*, i8** %81, align 8, !tbaa !323
  call void @_ZdlPv(i8* %82) #8
  %83 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 2, i32 0, i32 0, i32 0, i32 2, i32 3
  %84 = load %"class.std::unique_ptr"**, %"class.std::unique_ptr"*** %83, align 8, !tbaa !234
  %85 = getelementptr inbounds %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %84, i64 1
  store %"class.std::unique_ptr"** %85, %"class.std::unique_ptr"*** %83, align 8, !tbaa !215
  %86 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %85, align 8, !tbaa !27
  store %"class.std::unique_ptr"* %86, %"class.std::unique_ptr"** %80, align 8, !tbaa !216
  %87 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %86, i64 64
  store %"class.std::unique_ptr"* %87, %"class.std::unique_ptr"** %53, align 8, !tbaa !217
  %88 = ptrtoint %"class.std::unique_ptr"* %86 to i64
  %89 = bitcast %"struct.std::_Deque_iterator.41"* %70 to i64*
  store i64 %88, i64* %89, align 8, !tbaa !321
  br label %90

; <label>:90:                                     ; preds = %66, %79
  %91 = getelementptr inbounds %"class.MARC::ThreadSafeQueue.32", %"class.MARC::ThreadSafeQueue.32"* %0, i64 0, i32 4
  call void @_ZNSt18condition_variable10notify_oneEv(%"class.std::condition_variable"* %91) #8
  br label %92

; <label>:92:                                     ; preds = %.loopexit, %13, %90
  %93 = phi i1 [ false, %13 ], [ false, %.loopexit ], [ true, %90 ]
  %94 = load i8, i8* %7, align 8, !tbaa !35, !range !43
  %95 = icmp eq i8 %94, 0
  br i1 %95, label %104, label %96

; <label>:96:                                     ; preds = %92
  %97 = load %"class.std::mutex"*, %"class.std::mutex"** %6, align 8, !tbaa !32
  %98 = icmp eq %"class.std::mutex"* %97, null
  br i1 %98, label %104, label %99

; <label>:99:                                     ; preds = %96
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %100, label %103

; <label>:100:                                    ; preds = %99
  %101 = getelementptr inbounds %"class.std::mutex", %"class.std::mutex"* %97, i64 0, i32 0, i32 0
  %102 = call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* %101) #8
  br label %103

; <label>:103:                                    ; preds = %100, %99
  store i8 0, i8* %7, align 8, !tbaa !35
  br label %104

; <label>:104:                                    ; preds = %92, %96, %103
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %4) #8
  ret i1 %93
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
  store i8* %13, i8** %15, align 8, !tbaa !241
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
  br i1 %27, label %21, label %53

; <label>:28:                                     ; preds = %21
  %29 = landingpad { i8*, i32 }
          catch i8* null
  %30 = extractvalue { i8*, i32 } %29, 0
  %31 = tail call i8* @__cxa_begin_catch(i8* %30) #8
  %32 = icmp ugt %"class.std::function"** %22, %19
  br i1 %32, label %.preheader, label %.loopexit

.preheader:                                       ; preds = %28
  br label %33

; <label>:33:                                     ; preds = %.preheader, %33
  %34 = phi %"class.std::function"** [ %37, %33 ], [ %19, %.preheader ]
  %35 = bitcast %"class.std::function"** %34 to i8**
  %36 = load i8*, i8** %35, align 8, !tbaa !27
  tail call void @_ZdlPv(i8* %36) #8
  %37 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %34, i64 1
  %38 = icmp ult %"class.std::function"** %37, %22
  br i1 %38, label %33, label %.loopexit

.loopexit:                                        ; preds = %33, %28
  invoke void @__cxa_rethrow() #23
          to label %44 unwind label %39

; <label>:39:                                     ; preds = %.loopexit
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

; <label>:44:                                     ; preds = %.loopexit
  unreachable

; <label>:45:                                     ; preds = %39
  %46 = extractvalue { i8*, i32 } %40, 0
  %47 = tail call i8* @__cxa_begin_catch(i8* %46) #8
  %48 = load i8*, i8** %15, align 8, !tbaa !241
  tail call void @_ZdlPv(i8* %48) #8
  %49 = bitcast %"class.std::_Deque_base.48"* %0 to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %49, i8 0, i64 16, i32 8, i1 false)
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
  %54 = getelementptr inbounds %"class.std::_Deque_base.48", %"class.std::_Deque_base.48"* %0, i64 0, i32 0, i32 2
  %55 = getelementptr inbounds %"class.std::_Deque_base.48", %"class.std::_Deque_base.48"* %0, i64 0, i32 0, i32 2, i32 3
  store %"class.std::function"** %19, %"class.std::function"*** %55, align 8, !tbaa !220
  %56 = load %"class.std::function"*, %"class.std::function"** %19, align 8, !tbaa !27
  %57 = getelementptr inbounds %"class.std::_Deque_base.48", %"class.std::_Deque_base.48"* %0, i64 0, i32 0, i32 2, i32 1
  store %"class.std::function"* %56, %"class.std::function"** %57, align 8, !tbaa !223
  %58 = getelementptr inbounds %"class.std::function", %"class.std::function"* %56, i64 16
  %59 = getelementptr inbounds %"class.std::_Deque_base.48", %"class.std::_Deque_base.48"* %0, i64 0, i32 0, i32 2, i32 2
  store %"class.std::function"* %58, %"class.std::function"** %59, align 8, !tbaa !224
  %60 = getelementptr inbounds %"class.std::function"*, %"class.std::function"** %20, i64 -1
  %61 = getelementptr inbounds %"class.std::_Deque_base.48", %"class.std::_Deque_base.48"* %0, i64 0, i32 0, i32 3, i32 3
  store %"class.std::function"** %60, %"class.std::function"*** %61, align 8, !tbaa !220
  %62 = load %"class.std::function"*, %"class.std::function"** %60, align 8, !tbaa !27
  %63 = getelementptr inbounds %"class.std::_Deque_base.48", %"class.std::_Deque_base.48"* %0, i64 0, i32 0, i32 3, i32 1
  store %"class.std::function"* %62, %"class.std::function"** %63, align 8, !tbaa !223
  %64 = getelementptr inbounds %"class.std::function", %"class.std::function"* %62, i64 16
  %65 = getelementptr inbounds %"class.std::_Deque_base.48", %"class.std::_Deque_base.48"* %0, i64 0, i32 0, i32 3, i32 2
  store %"class.std::function"* %64, %"class.std::function"** %65, align 8, !tbaa !224
  %66 = ptrtoint %"class.std::function"* %56 to i64
  %67 = bitcast %"struct.std::_Deque_iterator.52"* %54 to i64*
  store i64 %66, i64* %67, align 8, !tbaa !250
  %68 = and i64 %1, 15
  %69 = getelementptr inbounds %"class.std::function", %"class.std::function"* %62, i64 %68
  %70 = getelementptr inbounds %"class.std::_Deque_base.48", %"class.std::_Deque_base.48"* %0, i64 0, i32 0, i32 3, i32 0
  store %"class.std::function"* %69, %"class.std::function"** %70, align 8, !tbaa !299
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
define linkonce_odr void @_ZNSt11_Deque_baseISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EESaIS5_EE17_M_initialize_mapEm(%"class.std::_Deque_base.35"*, i64) local_unnamed_addr #5 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %3 = lshr i64 %1, 6
  %4 = add nuw nsw i64 %3, 1
  %5 = add nuw nsw i64 %3, 3
  %6 = icmp ugt i64 %5, 8
  %7 = select i1 %6, i64 %5, i64 8
  %8 = getelementptr inbounds %"class.std::_Deque_base.35", %"class.std::_Deque_base.35"* %0, i64 0, i32 0, i32 1
  store i64 %7, i64* %8, align 8, !tbaa !212
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
  store i8* %13, i8** %15, align 8, !tbaa !214
  %16 = load i64, i64* %8, align 8, !tbaa !212
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
  br i1 %27, label %21, label %53

; <label>:28:                                     ; preds = %21
  %29 = landingpad { i8*, i32 }
          catch i8* null
  %30 = extractvalue { i8*, i32 } %29, 0
  %31 = tail call i8* @__cxa_begin_catch(i8* %30) #8
  %32 = icmp ugt %"class.std::unique_ptr"** %22, %19
  br i1 %32, label %.preheader, label %.loopexit

.preheader:                                       ; preds = %28
  br label %33

; <label>:33:                                     ; preds = %.preheader, %33
  %34 = phi %"class.std::unique_ptr"** [ %37, %33 ], [ %19, %.preheader ]
  %35 = bitcast %"class.std::unique_ptr"** %34 to i8**
  %36 = load i8*, i8** %35, align 8, !tbaa !27
  tail call void @_ZdlPv(i8* %36) #8
  %37 = getelementptr inbounds %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %34, i64 1
  %38 = icmp ult %"class.std::unique_ptr"** %37, %22
  br i1 %38, label %33, label %.loopexit

.loopexit:                                        ; preds = %33, %28
  invoke void @__cxa_rethrow() #23
          to label %44 unwind label %39

; <label>:39:                                     ; preds = %.loopexit
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

; <label>:44:                                     ; preds = %.loopexit
  unreachable

; <label>:45:                                     ; preds = %39
  %46 = extractvalue { i8*, i32 } %40, 0
  %47 = tail call i8* @__cxa_begin_catch(i8* %46) #8
  %48 = load i8*, i8** %15, align 8, !tbaa !214
  tail call void @_ZdlPv(i8* %48) #8
  %49 = bitcast %"class.std::_Deque_base.35"* %0 to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %49, i8 0, i64 16, i32 8, i1 false)
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
  %54 = getelementptr inbounds %"class.std::_Deque_base.35", %"class.std::_Deque_base.35"* %0, i64 0, i32 0, i32 2
  %55 = getelementptr inbounds %"class.std::_Deque_base.35", %"class.std::_Deque_base.35"* %0, i64 0, i32 0, i32 2, i32 3
  store %"class.std::unique_ptr"** %19, %"class.std::unique_ptr"*** %55, align 8, !tbaa !215
  %56 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %19, align 8, !tbaa !27
  %57 = getelementptr inbounds %"class.std::_Deque_base.35", %"class.std::_Deque_base.35"* %0, i64 0, i32 0, i32 2, i32 1
  store %"class.std::unique_ptr"* %56, %"class.std::unique_ptr"** %57, align 8, !tbaa !216
  %58 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %56, i64 64
  %59 = getelementptr inbounds %"class.std::_Deque_base.35", %"class.std::_Deque_base.35"* %0, i64 0, i32 0, i32 2, i32 2
  store %"class.std::unique_ptr"* %58, %"class.std::unique_ptr"** %59, align 8, !tbaa !217
  %60 = getelementptr inbounds %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %20, i64 -1
  %61 = getelementptr inbounds %"class.std::_Deque_base.35", %"class.std::_Deque_base.35"* %0, i64 0, i32 0, i32 3, i32 3
  store %"class.std::unique_ptr"** %60, %"class.std::unique_ptr"*** %61, align 8, !tbaa !215
  %62 = load %"class.std::unique_ptr"*, %"class.std::unique_ptr"** %60, align 8, !tbaa !27
  %63 = getelementptr inbounds %"class.std::_Deque_base.35", %"class.std::_Deque_base.35"* %0, i64 0, i32 0, i32 3, i32 1
  store %"class.std::unique_ptr"* %62, %"class.std::unique_ptr"** %63, align 8, !tbaa !216
  %64 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %62, i64 64
  %65 = getelementptr inbounds %"class.std::_Deque_base.35", %"class.std::_Deque_base.35"* %0, i64 0, i32 0, i32 3, i32 2
  store %"class.std::unique_ptr"* %64, %"class.std::unique_ptr"** %65, align 8, !tbaa !217
  %66 = ptrtoint %"class.std::unique_ptr"* %56 to i64
  %67 = bitcast %"struct.std::_Deque_iterator.41"* %54 to i64*
  store i64 %66, i64* %67, align 8, !tbaa !321
  %68 = and i64 %1, 63
  %69 = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %62, i64 %68
  %70 = getelementptr inbounds %"class.std::_Deque_base.35", %"class.std::_Deque_base.35"* %0, i64 0, i32 0, i32 3, i32 0
  store %"class.std::unique_ptr"* %69, %"class.std::unique_ptr"** %70, align 8, !tbaa !205
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

define internal void @1(i8* nocapture readonly, i8* nocapture readonly) {
  %3 = getelementptr inbounds i8, i8* %1, i64 32
  %4 = bitcast i8* %3 to %"class.MARC::ThreadSafeQueue.11"**
  %5 = load %"class.MARC::ThreadSafeQueue.11"*, %"class.MARC::ThreadSafeQueue.11"** %4, align 8
  %6 = getelementptr inbounds i8, i8* %1, i64 40
  %7 = bitcast i8* %6 to %"class.MARC::ThreadSafeQueue.11"**
  %8 = load %"class.MARC::ThreadSafeQueue.11"*, %"class.MARC::ThreadSafeQueue.11"** %7, align 8
  %9 = getelementptr inbounds i8, i8* %1, i64 48
  %10 = bitcast i8* %9 to %"class.MARC::ThreadSafeQueue.11"**
  %11 = load %"class.MARC::ThreadSafeQueue.11"*, %"class.MARC::ThreadSafeQueue.11"** %10, align 8
  %12 = getelementptr inbounds i8, i8* %1, i64 56
  %13 = bitcast i8* %12 to %"class.MARC::ThreadSafeQueue"**
  %14 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %13, align 8
  %.sroa.018 = alloca i8, align 1
  %15 = getelementptr inbounds i8, i8* %1, i64 64
  %16 = bitcast i8* %15 to %"class.MARC::ThreadSafeQueue"**
  %17 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %16, align 8
  %.sroa.016 = alloca i8, align 1
  %18 = getelementptr inbounds i8, i8* %1, i64 72
  %19 = bitcast i8* %18 to %"class.MARC::ThreadSafeQueue"**
  %20 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %19, align 8
  %.sroa.014 = alloca i8, align 1
  %21 = getelementptr inbounds i8, i8* %1, i64 80
  %22 = bitcast i8* %21 to %"class.MARC::ThreadSafeQueue"**
  %23 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %22, align 8
  %.sroa.012 = alloca i8, align 1
  %24 = getelementptr inbounds i8, i8* %1, i64 88
  %25 = bitcast i8* %24 to %"class.MARC::ThreadSafeQueue"**
  %26 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %25, align 8
  %.sroa.010 = alloca i8, align 1
  %27 = getelementptr inbounds i8, i8* %1, i64 96
  %28 = bitcast i8* %27 to %"class.MARC::ThreadSafeQueue"**
  %29 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %28, align 8
  %.sroa.08 = alloca i8, align 1
  %30 = getelementptr inbounds i8, i8* %1, i64 104
  %31 = bitcast i8* %30 to %"class.MARC::ThreadSafeQueue"**
  %32 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %31, align 8
  %.sroa.06 = alloca i8, align 1
  %33 = getelementptr inbounds i8, i8* %1, i64 112
  %34 = bitcast i8* %33 to %"class.MARC::ThreadSafeQueue"**
  %35 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %34, align 8
  %.sroa.04 = alloca i8, align 1
  %36 = getelementptr inbounds i8, i8* %1, i64 120
  %37 = bitcast i8* %36 to %"class.MARC::ThreadSafeQueue"**
  %38 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %37, align 8
  %.sroa.02 = alloca i8, align 1
  %39 = getelementptr inbounds i8, i8* %1, i64 128
  %40 = bitcast i8* %39 to %"class.MARC::ThreadSafeQueue"**
  %41 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %40, align 8
  %.sroa.0 = alloca i8, align 1
  %42 = getelementptr inbounds i8, i8* %1, i64 136
  %43 = bitcast i8* %42 to %"class.MARC::ThreadSafeQueue"**
  %44 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %43, align 8
  %45 = alloca i1, align 1
  %46 = bitcast i1* %45 to i8*
  %47 = getelementptr inbounds i8, i8* %1, i64 208
  %48 = bitcast i8* %47 to %"class.MARC::ThreadSafeQueue"**
  %49 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %48, align 8
  %50 = alloca i1, align 1
  %51 = bitcast i1* %50 to i8*
  %52 = getelementptr inbounds i8, i8* %1, i64 288
  %53 = bitcast i8* %52 to %"class.MARC::ThreadSafeQueue"**
  %54 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %53, align 8
  %55 = alloca i1, align 1
  %56 = bitcast i1* %55 to i8*
  %57 = getelementptr inbounds i8, i8* %1, i64 368
  %58 = bitcast i8* %57 to %"class.MARC::ThreadSafeQueue"**
  %59 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %58, align 8
  %60 = alloca i1, align 1
  %61 = bitcast i1* %60 to i8*
  %62 = getelementptr inbounds i8, i8* %1, i64 440
  %63 = bitcast i8* %62 to %"class.MARC::ThreadSafeQueue"**
  %64 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %63, align 8
  %65 = alloca i1, align 1
  %66 = bitcast i1* %65 to i8*
  %.sroa.0.0.sroa_cast1 = bitcast i8* %.sroa.0 to i1*
  %.sroa.02.0.sroa_cast3 = bitcast i8* %.sroa.02 to i1*
  %.sroa.04.0.sroa_cast5 = bitcast i8* %.sroa.04 to i1*
  %.sroa.06.0.sroa_cast7 = bitcast i8* %.sroa.06 to i1*
  %.sroa.08.0.sroa_cast9 = bitcast i8* %.sroa.08 to i1*
  %.sroa.010.0.sroa_cast11 = bitcast i8* %.sroa.010 to i1*
  %.sroa.012.0.sroa_cast13 = bitcast i8* %.sroa.012 to i1*
  %.sroa.014.0.sroa_cast15 = bitcast i8* %.sroa.014 to i1*
  %.sroa.016.0.sroa_cast17 = bitcast i8* %.sroa.016 to i1*
  %.sroa.018.0.sroa_cast19 = bitcast i8* %.sroa.018 to i1*
  br label %71

; <label>:67:                                     ; preds = %71
  %68 = getelementptr inbounds i8, i8* %0, i64 24
  %69 = bitcast i8* %68 to i32**
  %70 = load i32*, i32** %69, align 8
  store i32 0, i32* %70, align 4
  ret void

; <label>:71:                                     ; preds = %2, %.loopexit
  %72 = phi i32 [ 0, %2 ], [ %90, %.loopexit ]
  call void @_ZN4MARC15ThreadSafeQueueIiE4pushEi(%"class.MARC::ThreadSafeQueue.11"* %11, i32 %72)
  call void @_ZN4MARC15ThreadSafeQueueIiE4pushEi(%"class.MARC::ThreadSafeQueue.11"* %8, i32 %72)
  call void @_ZN4MARC15ThreadSafeQueueIiE4pushEi(%"class.MARC::ThreadSafeQueue.11"* %5, i32 %72)
  %73 = icmp eq i32 %72, 10
  store i1 %73, i1* %.sroa.0.0.sroa_cast1, align 1
  %.sroa.0.0..sroa.0.0. = load i8, i8* %.sroa.0, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %41, i8 signext %.sroa.0.0..sroa.0.0.)
  store i1 %73, i1* %.sroa.02.0.sroa_cast3, align 1
  %.sroa.02.0..sroa.02.0. = load i8, i8* %.sroa.02, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %38, i8 signext %.sroa.02.0..sroa.02.0.)
  store i1 %73, i1* %.sroa.04.0.sroa_cast5, align 1
  %.sroa.04.0..sroa.04.0. = load i8, i8* %.sroa.04, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %35, i8 signext %.sroa.04.0..sroa.04.0.)
  store i1 %73, i1* %.sroa.06.0.sroa_cast7, align 1
  %.sroa.06.0..sroa.06.0. = load i8, i8* %.sroa.06, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %32, i8 signext %.sroa.06.0..sroa.06.0.)
  store i1 %73, i1* %.sroa.08.0.sroa_cast9, align 1
  %.sroa.08.0..sroa.08.0. = load i8, i8* %.sroa.08, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %29, i8 signext %.sroa.08.0..sroa.08.0.)
  store i1 %73, i1* %.sroa.010.0.sroa_cast11, align 1
  %.sroa.010.0..sroa.010.0. = load i8, i8* %.sroa.010, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %26, i8 signext %.sroa.010.0..sroa.010.0.)
  store i1 %73, i1* %.sroa.012.0.sroa_cast13, align 1
  %.sroa.012.0..sroa.012.0. = load i8, i8* %.sroa.012, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %23, i8 signext %.sroa.012.0..sroa.012.0.)
  store i1 %73, i1* %.sroa.014.0.sroa_cast15, align 1
  %.sroa.014.0..sroa.014.0. = load i8, i8* %.sroa.014, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %20, i8 signext %.sroa.014.0..sroa.014.0.)
  store i1 %73, i1* %.sroa.016.0.sroa_cast17, align 1
  %.sroa.016.0..sroa.016.0. = load i8, i8* %.sroa.016, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %17, i8 signext %.sroa.016.0..sroa.016.0.)
  store i1 %73, i1* %.sroa.018.0.sroa_cast19, align 1
  %.sroa.018.0..sroa.018.0. = load i8, i8* %.sroa.018, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %14, i8 signext %.sroa.018.0..sroa.018.0.)
  br i1 %73, label %67, label %74

; <label>:74:                                     ; preds = %71
  %75 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %44, i8* nonnull dereferenceable(1) %46)
  %76 = load i1, i1* %45, align 1
  br i1 %76, label %77, label %.loopexit22

; <label>:77:                                     ; preds = %74
  %78 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %49, i8* nonnull dereferenceable(1) %51)
  %79 = load i1, i1* %50, align 1
  br i1 %79, label %.loopexit22, label %.lr.ph.preheader

.lr.ph.preheader:                                 ; preds = %77
  br label %.lr.ph

.lr.ph:                                           ; preds = %.lr.ph.backedge, %.lr.ph.preheader
  %80 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %54, i8* nonnull dereferenceable(1) %56)
  %81 = load i1, i1* %55, align 1
  br i1 %81, label %82, label %.lr.ph.backedge

.lr.ph.backedge:                                  ; preds = %.lr.ph, %82
  br label %.lr.ph

; <label>:82:                                     ; preds = %.lr.ph
  %83 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %49, i8* nonnull dereferenceable(1) %51)
  %84 = load i1, i1* %50, align 1
  br i1 %84, label %.loopexit22, label %.lr.ph.backedge

.loopexit22:                                      ; preds = %82, %77, %74
  %85 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %59, i8* nonnull dereferenceable(1) %61)
  %86 = load i1, i1* %60, align 1
  br i1 %86, label %.preheader, label %.loopexit

.preheader:                                       ; preds = %.loopexit22
  br label %87

; <label>:87:                                     ; preds = %.preheader, %87
  %88 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %64, i8* nonnull dereferenceable(1) %66)
  %89 = load i1, i1* %65, align 1
  br i1 %89, label %.loopexit, label %87

.loopexit:                                        ; preds = %87, %.loopexit22
  %90 = add nuw nsw i32 %72, 1
  br label %71
}

define internal void @2(i8* nocapture readonly, i8* nocapture readonly) {
  %3 = getelementptr inbounds i8, i8* %1, i64 8
  %4 = bitcast i8* %3 to %"class.MARC::ThreadSafeQueue"**
  %5 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %4, align 8
  %.sroa.018 = alloca i8, align 1
  %6 = getelementptr inbounds i8, i8* %1, i64 136
  %7 = bitcast i8* %6 to %"class.MARC::ThreadSafeQueue"**
  %8 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %7, align 8
  %.sroa.016 = alloca i8, align 1
  %9 = getelementptr inbounds i8, i8* %1, i64 144
  %10 = bitcast i8* %9 to %"class.MARC::ThreadSafeQueue"**
  %11 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %10, align 8
  %.sroa.014 = alloca i8, align 1
  %12 = getelementptr inbounds i8, i8* %1, i64 152
  %13 = bitcast i8* %12 to %"class.MARC::ThreadSafeQueue"**
  %14 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %13, align 8
  %.sroa.012 = alloca i8, align 1
  %15 = getelementptr inbounds i8, i8* %1, i64 160
  %16 = bitcast i8* %15 to %"class.MARC::ThreadSafeQueue"**
  %17 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %16, align 8
  %.sroa.010 = alloca i8, align 1
  %18 = getelementptr inbounds i8, i8* %1, i64 168
  %19 = bitcast i8* %18 to %"class.MARC::ThreadSafeQueue"**
  %20 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %19, align 8
  %.sroa.08 = alloca i8, align 1
  %21 = getelementptr inbounds i8, i8* %1, i64 176
  %22 = bitcast i8* %21 to %"class.MARC::ThreadSafeQueue"**
  %23 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %22, align 8
  %.sroa.06 = alloca i8, align 1
  %24 = getelementptr inbounds i8, i8* %1, i64 184
  %25 = bitcast i8* %24 to %"class.MARC::ThreadSafeQueue"**
  %26 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %25, align 8
  %.sroa.04 = alloca i8, align 1
  %27 = getelementptr inbounds i8, i8* %1, i64 192
  %28 = bitcast i8* %27 to %"class.MARC::ThreadSafeQueue"**
  %29 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %28, align 8
  %.sroa.02 = alloca i8, align 1
  %30 = getelementptr inbounds i8, i8* %1, i64 200
  %31 = bitcast i8* %30 to %"class.MARC::ThreadSafeQueue"**
  %32 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %31, align 8
  %.sroa.0 = alloca i8, align 1
  %33 = getelementptr inbounds i8, i8* %1, i64 40
  %34 = bitcast i8* %33 to %"class.MARC::ThreadSafeQueue.11"**
  %35 = load %"class.MARC::ThreadSafeQueue.11"*, %"class.MARC::ThreadSafeQueue.11"** %34, align 8
  %36 = alloca i32, align 4
  %37 = getelementptr inbounds i8, i8* %1, i64 56
  %38 = bitcast i8* %37 to %"class.MARC::ThreadSafeQueue"**
  %39 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %38, align 8
  %40 = alloca i1, align 1
  %41 = bitcast i1* %40 to i8*
  %42 = getelementptr inbounds i8, i8* %1, i64 216
  %43 = bitcast i8* %42 to %"class.MARC::ThreadSafeQueue"**
  %44 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %43, align 8
  %45 = alloca i1, align 1
  %46 = bitcast i1* %45 to i8*
  %47 = getelementptr inbounds i8, i8* %1, i64 296
  %48 = bitcast i8* %47 to %"class.MARC::ThreadSafeQueue"**
  %49 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %48, align 8
  %50 = alloca i1, align 1
  %51 = bitcast i1* %50 to i8*
  %52 = getelementptr inbounds i8, i8* %1, i64 376
  %53 = bitcast i8* %52 to %"class.MARC::ThreadSafeQueue"**
  %54 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %53, align 8
  %55 = alloca i1, align 1
  %56 = bitcast i1* %55 to i8*
  %57 = getelementptr inbounds i8, i8* %1, i64 448
  %58 = bitcast i8* %57 to %"class.MARC::ThreadSafeQueue"**
  %59 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %58, align 8
  %60 = alloca i1, align 1
  %61 = bitcast i1* %60 to i8*
  %62 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIiE7waitPopERi(%"class.MARC::ThreadSafeQueue.11"* %35, i32* nonnull dereferenceable(4) %36)
  %63 = load i32, i32* %36, align 4
  %64 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %39, i8* nonnull dereferenceable(1) %41)
  %65 = load i1, i1* %40, align 1
  br i1 %65, label %._crit_edge, label %.lr.ph21

.lr.ph21:                                         ; preds = %2
  %.sroa.0.0.sroa_cast1 = bitcast i8* %.sroa.0 to i1*
  %.sroa.02.0.sroa_cast3 = bitcast i8* %.sroa.02 to i1*
  %.sroa.04.0.sroa_cast5 = bitcast i8* %.sroa.04 to i1*
  %.sroa.06.0.sroa_cast7 = bitcast i8* %.sroa.06 to i1*
  %.sroa.08.0.sroa_cast9 = bitcast i8* %.sroa.08 to i1*
  %.sroa.010.0.sroa_cast11 = bitcast i8* %.sroa.010 to i1*
  %.sroa.012.0.sroa_cast13 = bitcast i8* %.sroa.012 to i1*
  %.sroa.014.0.sroa_cast15 = bitcast i8* %.sroa.014 to i1*
  %.sroa.016.0.sroa_cast17 = bitcast i8* %.sroa.016 to i1*
  %.sroa.018.0.sroa_cast19 = bitcast i8* %.sroa.018 to i1*
  br label %69

._crit_edge:                                      ; preds = %.loopexit, %2
  %66 = getelementptr inbounds i8, i8* %0, i64 24
  %67 = bitcast i8* %66 to i32**
  %68 = load i32*, i32** %67, align 8
  store i32 0, i32* %68, align 4
  ret void

; <label>:69:                                     ; preds = %.lr.ph21, %.loopexit
  %70 = phi i32 [ %63, %.lr.ph21 ], [ %86, %.loopexit ]
  %71 = icmp ugt i32 %70, 3
  store i1 %71, i1* %.sroa.0.0.sroa_cast1, align 1
  %.sroa.0.0..sroa.0.0. = load i8, i8* %.sroa.0, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %32, i8 signext %.sroa.0.0..sroa.0.0.)
  store i1 %71, i1* %.sroa.02.0.sroa_cast3, align 1
  %.sroa.02.0..sroa.02.0. = load i8, i8* %.sroa.02, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %29, i8 signext %.sroa.02.0..sroa.02.0.)
  store i1 %71, i1* %.sroa.04.0.sroa_cast5, align 1
  %.sroa.04.0..sroa.04.0. = load i8, i8* %.sroa.04, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %26, i8 signext %.sroa.04.0..sroa.04.0.)
  store i1 %71, i1* %.sroa.06.0.sroa_cast7, align 1
  %.sroa.06.0..sroa.06.0. = load i8, i8* %.sroa.06, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %23, i8 signext %.sroa.06.0..sroa.06.0.)
  store i1 %71, i1* %.sroa.08.0.sroa_cast9, align 1
  %.sroa.08.0..sroa.08.0. = load i8, i8* %.sroa.08, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %20, i8 signext %.sroa.08.0..sroa.08.0.)
  store i1 %71, i1* %.sroa.010.0.sroa_cast11, align 1
  %.sroa.010.0..sroa.010.0. = load i8, i8* %.sroa.010, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %17, i8 signext %.sroa.010.0..sroa.010.0.)
  store i1 %71, i1* %.sroa.012.0.sroa_cast13, align 1
  %.sroa.012.0..sroa.012.0. = load i8, i8* %.sroa.012, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %14, i8 signext %.sroa.012.0..sroa.012.0.)
  store i1 %71, i1* %.sroa.014.0.sroa_cast15, align 1
  %.sroa.014.0..sroa.014.0. = load i8, i8* %.sroa.014, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %11, i8 signext %.sroa.014.0..sroa.014.0.)
  store i1 %71, i1* %.sroa.016.0.sroa_cast17, align 1
  %.sroa.016.0..sroa.016.0. = load i8, i8* %.sroa.016, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %8, i8 signext %.sroa.016.0..sroa.016.0.)
  store i1 %71, i1* %.sroa.018.0.sroa_cast19, align 1
  %.sroa.018.0..sroa.018.0. = load i8, i8* %.sroa.018, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %5, i8 signext %.sroa.018.0..sroa.018.0.)
  br i1 %71, label %72, label %.loopexit20

; <label>:72:                                     ; preds = %69
  %73 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %44, i8* nonnull dereferenceable(1) %46)
  %74 = load i1, i1* %45, align 1
  br i1 %74, label %.loopexit20, label %.lr.ph.preheader

.lr.ph.preheader:                                 ; preds = %72
  br label %.lr.ph

.lr.ph:                                           ; preds = %.lr.ph.backedge, %.lr.ph.preheader
  %75 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %49, i8* nonnull dereferenceable(1) %51)
  %76 = load i1, i1* %50, align 1
  br i1 %76, label %77, label %.lr.ph.backedge

.lr.ph.backedge:                                  ; preds = %.lr.ph, %77
  br label %.lr.ph

; <label>:77:                                     ; preds = %.lr.ph
  %78 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %44, i8* nonnull dereferenceable(1) %46)
  %79 = load i1, i1* %45, align 1
  br i1 %79, label %.loopexit20, label %.lr.ph.backedge

.loopexit20:                                      ; preds = %77, %72, %69
  %80 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %54, i8* nonnull dereferenceable(1) %56)
  %81 = load i1, i1* %55, align 1
  br i1 %81, label %.preheader, label %.loopexit

.preheader:                                       ; preds = %.loopexit20
  br label %82

; <label>:82:                                     ; preds = %.preheader, %82
  %83 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %59, i8* nonnull dereferenceable(1) %61)
  %84 = load i1, i1* %60, align 1
  br i1 %84, label %.loopexit, label %82

.loopexit:                                        ; preds = %82, %.loopexit20
  %85 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIiE7waitPopERi(%"class.MARC::ThreadSafeQueue.11"* %35, i32* nonnull dereferenceable(4) %36)
  %86 = load i32, i32* %36, align 4
  %87 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %39, i8* nonnull dereferenceable(1) %41)
  %88 = load i1, i1* %40, align 1
  br i1 %88, label %._crit_edge, label %69
}

define internal void @3(i8* nocapture readonly, i8* nocapture readonly) {
  %3 = getelementptr inbounds i8, i8* %1, i64 16
  %4 = bitcast i8* %3 to %"class.MARC::ThreadSafeQueue"**
  %5 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %4, align 8
  %.sroa.0 = alloca i8, align 1
  %6 = getelementptr inbounds i8, i8* %1, i64 32
  %7 = bitcast i8* %6 to %"class.MARC::ThreadSafeQueue.11"**
  %8 = load %"class.MARC::ThreadSafeQueue.11"*, %"class.MARC::ThreadSafeQueue.11"** %7, align 8
  %9 = alloca i32, align 4
  %10 = getelementptr inbounds i8, i8* %1, i64 64
  %11 = bitcast i8* %10 to %"class.MARC::ThreadSafeQueue"**
  %12 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %11, align 8
  %13 = alloca i1, align 1
  %14 = bitcast i1* %13 to i8*
  %15 = getelementptr inbounds i8, i8* %1, i64 144
  %16 = bitcast i8* %15 to %"class.MARC::ThreadSafeQueue"**
  %17 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %16, align 8
  %18 = alloca i1, align 1
  %19 = bitcast i1* %18 to i8*
  %20 = getelementptr inbounds i8, i8* %1, i64 224
  %21 = bitcast i8* %20 to %"class.MARC::ThreadSafeQueue"**
  %22 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %21, align 8
  %23 = alloca i1, align 1
  %24 = bitcast i1* %23 to i8*
  %25 = getelementptr inbounds i8, i8* %1, i64 304
  %26 = bitcast i8* %25 to %"class.MARC::ThreadSafeQueue"**
  %27 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %26, align 8
  %28 = alloca i1, align 1
  %29 = bitcast i1* %28 to i8*
  %30 = getelementptr inbounds i8, i8* %1, i64 384
  %31 = bitcast i8* %30 to %"class.MARC::ThreadSafeQueue"**
  %32 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %31, align 8
  %33 = alloca i1, align 1
  %34 = bitcast i1* %33 to i8*
  %35 = getelementptr inbounds i8, i8* %1, i64 456
  %36 = bitcast i8* %35 to %"class.MARC::ThreadSafeQueue"**
  %37 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %36, align 8
  %38 = alloca i1, align 1
  %39 = bitcast i1* %38 to i8*
  %40 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIiE7waitPopERi(%"class.MARC::ThreadSafeQueue.11"* %8, i32* nonnull dereferenceable(4) %9)
  %41 = load i32, i32* %9, align 4
  %42 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %12, i8* nonnull dereferenceable(1) %14)
  %43 = load i1, i1* %13, align 1
  br i1 %43, label %._crit_edge, label %.lr.ph3

.lr.ph3:                                          ; preds = %2
  %.sroa.0.0.sroa_cast1 = bitcast i8* %.sroa.0 to i1*
  br label %47

._crit_edge:                                      ; preds = %.loopexit, %2
  %44 = getelementptr inbounds i8, i8* %0, i64 24
  %45 = bitcast i8* %44 to i32**
  %46 = load i32*, i32** %45, align 8
  store i32 0, i32* %46, align 4
  ret void

; <label>:47:                                     ; preds = %.lr.ph3, %.loopexit
  %48 = phi i32 [ %41, %.lr.ph3 ], [ %66, %.loopexit ]
  %49 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %17, i8* nonnull dereferenceable(1) %19)
  %50 = load i1, i1* %18, align 1
  br i1 %50, label %51, label %.loopexit2

; <label>:51:                                     ; preds = %47
  %52 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %22, i8* nonnull dereferenceable(1) %24)
  %53 = load i1, i1* %23, align 1
  br i1 %53, label %.loopexit2, label %.lr.ph.preheader

.lr.ph.preheader:                                 ; preds = %51
  br label %.lr.ph

.lr.ph:                                           ; preds = %.lr.ph.backedge, %.lr.ph.preheader
  %54 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %27, i8* nonnull dereferenceable(1) %29)
  %55 = load i1, i1* %28, align 1
  br i1 %55, label %56, label %.lr.ph.backedge

.lr.ph.backedge:                                  ; preds = %.lr.ph, %56
  br label %.lr.ph

; <label>:56:                                     ; preds = %.lr.ph
  %57 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %22, i8* nonnull dereferenceable(1) %24)
  %58 = load i1, i1* %23, align 1
  br i1 %58, label %.loopexit2, label %.lr.ph.backedge

.loopexit2:                                       ; preds = %56, %51, %47
  %59 = icmp eq i32 %48, 9
  store i1 %59, i1* %.sroa.0.0.sroa_cast1, align 1
  %.sroa.0.0..sroa.0.0. = load i8, i8* %.sroa.0, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %5, i8 signext %.sroa.0.0..sroa.0.0.)
  %60 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %32, i8* nonnull dereferenceable(1) %34)
  %61 = load i1, i1* %33, align 1
  br i1 %61, label %.preheader, label %.loopexit

.preheader:                                       ; preds = %.loopexit2
  br label %62

; <label>:62:                                     ; preds = %.preheader, %62
  %63 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %37, i8* nonnull dereferenceable(1) %39)
  %64 = load i1, i1* %38, align 1
  br i1 %64, label %.loopexit, label %62

.loopexit:                                        ; preds = %62, %.loopexit2
  %65 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIiE7waitPopERi(%"class.MARC::ThreadSafeQueue.11"* %8, i32* nonnull dereferenceable(4) %9)
  %66 = load i32, i32* %9, align 4
  %67 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %12, i8* nonnull dereferenceable(1) %14)
  %68 = load i1, i1* %13, align 1
  br i1 %68, label %._crit_edge, label %47
}

define internal void @4(i8* nocapture readonly, i8* nocapture readonly) {
  %3 = getelementptr inbounds i8, i8* %1, i64 24
  %4 = bitcast i8* %3 to %"class.MARC::ThreadSafeQueue"**
  %5 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %4, align 8
  %.sroa.018 = alloca i8, align 1
  %6 = getelementptr inbounds i8, i8* %1, i64 368
  %7 = bitcast i8* %6 to %"class.MARC::ThreadSafeQueue"**
  %8 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %7, align 8
  %.sroa.016 = alloca i8, align 1
  %9 = getelementptr inbounds i8, i8* %1, i64 376
  %10 = bitcast i8* %9 to %"class.MARC::ThreadSafeQueue"**
  %11 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %10, align 8
  %.sroa.014 = alloca i8, align 1
  %12 = getelementptr inbounds i8, i8* %1, i64 384
  %13 = bitcast i8* %12 to %"class.MARC::ThreadSafeQueue"**
  %14 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %13, align 8
  %.sroa.012 = alloca i8, align 1
  %15 = getelementptr inbounds i8, i8* %1, i64 392
  %16 = bitcast i8* %15 to %"class.MARC::ThreadSafeQueue"**
  %17 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %16, align 8
  %.sroa.010 = alloca i8, align 1
  %18 = getelementptr inbounds i8, i8* %1, i64 400
  %19 = bitcast i8* %18 to %"class.MARC::ThreadSafeQueue"**
  %20 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %19, align 8
  %.sroa.08 = alloca i8, align 1
  %21 = getelementptr inbounds i8, i8* %1, i64 408
  %22 = bitcast i8* %21 to %"class.MARC::ThreadSafeQueue"**
  %23 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %22, align 8
  %.sroa.06 = alloca i8, align 1
  %24 = getelementptr inbounds i8, i8* %1, i64 416
  %25 = bitcast i8* %24 to %"class.MARC::ThreadSafeQueue"**
  %26 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %25, align 8
  %.sroa.04 = alloca i8, align 1
  %27 = getelementptr inbounds i8, i8* %1, i64 424
  %28 = bitcast i8* %27 to %"class.MARC::ThreadSafeQueue"**
  %29 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %28, align 8
  %.sroa.02 = alloca i8, align 1
  %30 = getelementptr inbounds i8, i8* %1, i64 432
  %31 = bitcast i8* %30 to %"class.MARC::ThreadSafeQueue"**
  %32 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %31, align 8
  %.sroa.0 = alloca i8, align 1
  %33 = getelementptr inbounds i8, i8* %1, i64 48
  %34 = bitcast i8* %33 to %"class.MARC::ThreadSafeQueue.11"**
  %35 = load %"class.MARC::ThreadSafeQueue.11"*, %"class.MARC::ThreadSafeQueue.11"** %34, align 8
  %36 = alloca i32, align 4
  %37 = getelementptr inbounds i8, i8* %1, i64 72
  %38 = bitcast i8* %37 to %"class.MARC::ThreadSafeQueue"**
  %39 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %38, align 8
  %40 = alloca i1, align 1
  %41 = bitcast i1* %40 to i8*
  %42 = getelementptr inbounds i8, i8* %1, i64 152
  %43 = bitcast i8* %42 to %"class.MARC::ThreadSafeQueue"**
  %44 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %43, align 8
  %45 = alloca i1, align 1
  %46 = bitcast i1* %45 to i8*
  %47 = getelementptr inbounds i8, i8* %1, i64 232
  %48 = bitcast i8* %47 to %"class.MARC::ThreadSafeQueue"**
  %49 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %48, align 8
  %50 = alloca i1, align 1
  %51 = bitcast i1* %50 to i8*
  %52 = getelementptr inbounds i8, i8* %1, i64 312
  %53 = bitcast i8* %52 to %"class.MARC::ThreadSafeQueue"**
  %54 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %53, align 8
  %55 = alloca i1, align 1
  %56 = bitcast i1* %55 to i8*
  %57 = getelementptr inbounds i8, i8* %1, i64 464
  %58 = bitcast i8* %57 to %"class.MARC::ThreadSafeQueue"**
  %59 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %58, align 8
  %60 = alloca i1, align 1
  %61 = bitcast i1* %60 to i8*
  %62 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIiE7waitPopERi(%"class.MARC::ThreadSafeQueue.11"* %35, i32* nonnull dereferenceable(4) %36)
  %63 = load i32, i32* %36, align 4
  %64 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %39, i8* nonnull dereferenceable(1) %41)
  %65 = load i1, i1* %40, align 1
  br i1 %65, label %._crit_edge, label %.lr.ph21

.lr.ph21:                                         ; preds = %2
  %.sroa.0.0.sroa_cast1 = bitcast i8* %.sroa.0 to i1*
  %.sroa.02.0.sroa_cast3 = bitcast i8* %.sroa.02 to i1*
  %.sroa.04.0.sroa_cast5 = bitcast i8* %.sroa.04 to i1*
  %.sroa.06.0.sroa_cast7 = bitcast i8* %.sroa.06 to i1*
  %.sroa.08.0.sroa_cast9 = bitcast i8* %.sroa.08 to i1*
  %.sroa.010.0.sroa_cast11 = bitcast i8* %.sroa.010 to i1*
  %.sroa.012.0.sroa_cast13 = bitcast i8* %.sroa.012 to i1*
  %.sroa.014.0.sroa_cast15 = bitcast i8* %.sroa.014 to i1*
  %.sroa.016.0.sroa_cast17 = bitcast i8* %.sroa.016 to i1*
  %.sroa.018.0.sroa_cast19 = bitcast i8* %.sroa.018 to i1*
  br label %69

._crit_edge:                                      ; preds = %.loopexit, %2
  %66 = getelementptr inbounds i8, i8* %0, i64 24
  %67 = bitcast i8* %66 to i32**
  %68 = load i32*, i32** %67, align 8
  store i32 0, i32* %68, align 4
  ret void

; <label>:69:                                     ; preds = %.lr.ph21, %.loopexit
  %70 = phi i32 [ %63, %.lr.ph21 ], [ %86, %.loopexit ]
  %71 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %44, i8* nonnull dereferenceable(1) %46)
  %72 = load i1, i1* %45, align 1
  br i1 %72, label %73, label %.loopexit20

; <label>:73:                                     ; preds = %69
  %74 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %49, i8* nonnull dereferenceable(1) %51)
  %75 = load i1, i1* %50, align 1
  br i1 %75, label %.loopexit20, label %.lr.ph.preheader

.lr.ph.preheader:                                 ; preds = %73
  br label %.lr.ph

.lr.ph:                                           ; preds = %.lr.ph.backedge, %.lr.ph.preheader
  %76 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %54, i8* nonnull dereferenceable(1) %56)
  %77 = load i1, i1* %55, align 1
  br i1 %77, label %78, label %.lr.ph.backedge

.lr.ph.backedge:                                  ; preds = %.lr.ph, %78
  br label %.lr.ph

; <label>:78:                                     ; preds = %.lr.ph
  %79 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %49, i8* nonnull dereferenceable(1) %51)
  %80 = load i1, i1* %50, align 1
  br i1 %80, label %.loopexit20, label %.lr.ph.backedge

.loopexit20:                                      ; preds = %78, %73, %69
  %81 = icmp ult i32 %70, 7
  store i1 %81, i1* %.sroa.0.0.sroa_cast1, align 1
  %.sroa.0.0..sroa.0.0. = load i8, i8* %.sroa.0, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %32, i8 signext %.sroa.0.0..sroa.0.0.)
  store i1 %81, i1* %.sroa.02.0.sroa_cast3, align 1
  %.sroa.02.0..sroa.02.0. = load i8, i8* %.sroa.02, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %29, i8 signext %.sroa.02.0..sroa.02.0.)
  store i1 %81, i1* %.sroa.04.0.sroa_cast5, align 1
  %.sroa.04.0..sroa.04.0. = load i8, i8* %.sroa.04, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %26, i8 signext %.sroa.04.0..sroa.04.0.)
  store i1 %81, i1* %.sroa.06.0.sroa_cast7, align 1
  %.sroa.06.0..sroa.06.0. = load i8, i8* %.sroa.06, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %23, i8 signext %.sroa.06.0..sroa.06.0.)
  store i1 %81, i1* %.sroa.08.0.sroa_cast9, align 1
  %.sroa.08.0..sroa.08.0. = load i8, i8* %.sroa.08, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %20, i8 signext %.sroa.08.0..sroa.08.0.)
  store i1 %81, i1* %.sroa.010.0.sroa_cast11, align 1
  %.sroa.010.0..sroa.010.0. = load i8, i8* %.sroa.010, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %17, i8 signext %.sroa.010.0..sroa.010.0.)
  store i1 %81, i1* %.sroa.012.0.sroa_cast13, align 1
  %.sroa.012.0..sroa.012.0. = load i8, i8* %.sroa.012, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %14, i8 signext %.sroa.012.0..sroa.012.0.)
  store i1 %81, i1* %.sroa.014.0.sroa_cast15, align 1
  %.sroa.014.0..sroa.014.0. = load i8, i8* %.sroa.014, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %11, i8 signext %.sroa.014.0..sroa.014.0.)
  store i1 %81, i1* %.sroa.016.0.sroa_cast17, align 1
  %.sroa.016.0..sroa.016.0. = load i8, i8* %.sroa.016, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %8, i8 signext %.sroa.016.0..sroa.016.0.)
  store i1 %81, i1* %.sroa.018.0.sroa_cast19, align 1
  %.sroa.018.0..sroa.018.0. = load i8, i8* %.sroa.018, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %5, i8 signext %.sroa.018.0..sroa.018.0.)
  br i1 %81, label %.preheader, label %.loopexit

.preheader:                                       ; preds = %.loopexit20
  br label %82

; <label>:82:                                     ; preds = %.preheader, %82
  %83 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %59, i8* nonnull dereferenceable(1) %61)
  %84 = load i1, i1* %60, align 1
  br i1 %84, label %.loopexit, label %82

.loopexit:                                        ; preds = %82, %.loopexit20
  %85 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIiE7waitPopERi(%"class.MARC::ThreadSafeQueue.11"* %35, i32* nonnull dereferenceable(4) %36)
  %86 = load i32, i32* %36, align 4
  %87 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %39, i8* nonnull dereferenceable(1) %41)
  %88 = load i1, i1* %40, align 1
  br i1 %88, label %._crit_edge, label %69
}

define internal void @5(i8* nocapture readonly, i8* nocapture readonly) {
  %3 = getelementptr inbounds i8, i8* %1, i64 8
  %4 = bitcast i8* %3 to %"class.MARC::ThreadSafeQueue"**
  %5 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %4, align 8
  %6 = alloca i1, align 1
  %7 = bitcast i1* %6 to i8*
  %8 = getelementptr inbounds i8, i8* %1, i64 80
  %9 = bitcast i8* %8 to %"class.MARC::ThreadSafeQueue"**
  %10 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %9, align 8
  %11 = alloca i1, align 1
  %12 = bitcast i1* %11 to i8*
  %13 = getelementptr inbounds i8, i8* %1, i64 240
  %14 = bitcast i8* %13 to %"class.MARC::ThreadSafeQueue"**
  %15 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %14, align 8
  %16 = alloca i1, align 1
  %17 = bitcast i1* %16 to i8*
  %18 = getelementptr inbounds i8, i8* %1, i64 320
  %19 = bitcast i8* %18 to %"class.MARC::ThreadSafeQueue"**
  %20 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %19, align 8
  %21 = alloca i1, align 1
  %22 = bitcast i1* %21 to i8*
  %23 = getelementptr inbounds i8, i8* %1, i64 392
  %24 = bitcast i8* %23 to %"class.MARC::ThreadSafeQueue"**
  %25 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %24, align 8
  %26 = alloca i1, align 1
  %27 = bitcast i1* %26 to i8*
  %28 = getelementptr inbounds i8, i8* %1, i64 472
  %29 = bitcast i8* %28 to %"class.MARC::ThreadSafeQueue"**
  %30 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %29, align 8
  %31 = alloca i1, align 1
  %32 = bitcast i1* %31 to i8*
  %33 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %10, i8* nonnull dereferenceable(1) %12)
  %34 = load i1, i1* %11, align 1
  br i1 %34, label %._crit_edge, label %.lr.ph2.preheader

.lr.ph2.preheader:                                ; preds = %2
  br label %.lr.ph2

._crit_edge:                                      ; preds = %.loopexit, %2
  %35 = getelementptr inbounds i8, i8* %0, i64 24
  %36 = bitcast i8* %35 to i32**
  %37 = load i32*, i32** %36, align 8
  store i32 0, i32* %37, align 4
  ret void

.lr.ph2:                                          ; preds = %.lr.ph2.preheader, %.loopexit
  %38 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %5, i8* nonnull dereferenceable(1) %7)
  %39 = load i1, i1* %6, align 1
  br i1 %39, label %40, label %.loopexit1

; <label>:40:                                     ; preds = %.lr.ph2
  %41 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %15, i8* nonnull dereferenceable(1) %17)
  %42 = load i1, i1* %16, align 1
  br i1 %42, label %.loopexit1, label %.lr.ph.preheader

.lr.ph.preheader:                                 ; preds = %40
  br label %.lr.ph

.lr.ph:                                           ; preds = %.lr.ph.backedge, %.lr.ph.preheader
  %43 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %20, i8* nonnull dereferenceable(1) %22)
  %44 = load i1, i1* %21, align 1
  br i1 %44, label %45, label %.lr.ph.backedge

.lr.ph.backedge:                                  ; preds = %.lr.ph, %45
  br label %.lr.ph

; <label>:45:                                     ; preds = %.lr.ph
  %46 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %15, i8* nonnull dereferenceable(1) %17)
  %47 = load i1, i1* %16, align 1
  br i1 %47, label %.loopexit1, label %.lr.ph.backedge

.loopexit1:                                       ; preds = %45, %40, %.lr.ph2
  %48 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %25, i8* nonnull dereferenceable(1) %27)
  %49 = load i1, i1* %26, align 1
  br i1 %49, label %.preheader, label %.loopexit

.preheader:                                       ; preds = %.loopexit1
  br label %50

; <label>:50:                                     ; preds = %.preheader, %50
  %51 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %30, i8* nonnull dereferenceable(1) %32)
  %52 = load i1, i1* %31, align 1
  br i1 %52, label %.loopexit, label %50

.loopexit:                                        ; preds = %50, %.loopexit1
  %53 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %10, i8* nonnull dereferenceable(1) %12)
  %54 = load i1, i1* %11, align 1
  br i1 %54, label %._crit_edge, label %.lr.ph2
}

define internal void @6(i8* nocapture readonly, i8* nocapture readonly) {
  %3 = bitcast i8* %1 to %"class.MARC::ThreadSafeQueue.11"**
  %4 = load %"class.MARC::ThreadSafeQueue.11"*, %"class.MARC::ThreadSafeQueue.11"** %3, align 8
  %5 = alloca float, align 4
  %6 = bitcast float* %5 to i32*
  %7 = getelementptr inbounds i8, i8* %1, i64 16
  %8 = bitcast i8* %7 to %"class.MARC::ThreadSafeQueue"**
  %9 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %8, align 8
  %10 = alloca i1, align 1
  %11 = bitcast i1* %10 to i8*
  %12 = getelementptr inbounds i8, i8* %1, i64 88
  %13 = bitcast i8* %12 to %"class.MARC::ThreadSafeQueue"**
  %14 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %13, align 8
  %15 = alloca i1, align 1
  %16 = bitcast i1* %15 to i8*
  %17 = getelementptr inbounds i8, i8* %1, i64 160
  %18 = bitcast i8* %17 to %"class.MARC::ThreadSafeQueue"**
  %19 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %18, align 8
  %20 = alloca i1, align 1
  %21 = bitcast i1* %20 to i8*
  %22 = getelementptr inbounds i8, i8* %1, i64 248
  %23 = bitcast i8* %22 to %"class.MARC::ThreadSafeQueue"**
  %24 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %23, align 8
  %25 = alloca i1, align 1
  %26 = bitcast i1* %25 to i8*
  %27 = getelementptr inbounds i8, i8* %1, i64 328
  %28 = bitcast i8* %27 to %"class.MARC::ThreadSafeQueue"**
  %29 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %28, align 8
  %30 = alloca i1, align 1
  %31 = bitcast i1* %30 to i8*
  %32 = getelementptr inbounds i8, i8* %1, i64 400
  %33 = bitcast i8* %32 to %"class.MARC::ThreadSafeQueue"**
  %34 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %33, align 8
  %35 = alloca i1, align 1
  %36 = bitcast i1* %35 to i8*
  %37 = getelementptr inbounds i8, i8* %1, i64 480
  %38 = bitcast i8* %37 to %"class.MARC::ThreadSafeQueue"**
  %39 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %38, align 8
  %40 = alloca i1, align 1
  %41 = bitcast i1* %40 to i8*
  %42 = getelementptr inbounds i8, i8* %0, i64 8
  %43 = bitcast i8* %42 to float**
  %44 = load float*, float** %43, align 8
  %45 = load float, float* %44, align 4
  %46 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %14, i8* nonnull dereferenceable(1) %16)
  %47 = load i1, i1* %15, align 1
  br i1 %47, label %._crit_edge, label %.lr.ph3.preheader

.lr.ph3.preheader:                                ; preds = %2
  br label %.lr.ph3

._crit_edge:                                      ; preds = %.loopexit, %2
  %48 = getelementptr inbounds i8, i8* %0, i64 24
  %49 = bitcast i8* %48 to i32**
  %50 = load i32*, i32** %49, align 8
  store i32 0, i32* %50, align 4
  ret void

.lr.ph3:                                          ; preds = %.lr.ph3.preheader, %.loopexit
  %51 = phi float [ %80, %.loopexit ], [ %45, %.lr.ph3.preheader ]
  %52 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %19, i8* nonnull dereferenceable(1) %21)
  %53 = load i1, i1* %20, align 1
  br i1 %53, label %54, label %.loopexit1

; <label>:54:                                     ; preds = %.lr.ph3
  %55 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %24, i8* nonnull dereferenceable(1) %26)
  %56 = load i1, i1* %25, align 1
  br i1 %56, label %.loopexit1, label %.lr.ph.preheader

.lr.ph.preheader:                                 ; preds = %54
  br label %.lr.ph

.lr.ph:                                           ; preds = %.lr.ph.backedge, %.lr.ph.preheader
  %57 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %29, i8* nonnull dereferenceable(1) %31)
  %58 = load i1, i1* %30, align 1
  br i1 %58, label %59, label %.lr.ph.backedge

.lr.ph.backedge:                                  ; preds = %.lr.ph, %59
  br label %.lr.ph

; <label>:59:                                     ; preds = %.lr.ph
  %60 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %24, i8* nonnull dereferenceable(1) %26)
  %61 = load i1, i1* %25, align 1
  br i1 %61, label %.loopexit1, label %.lr.ph.backedge

.loopexit1:                                       ; preds = %59, %54, %.lr.ph3
  %62 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIiE7waitPopERi(%"class.MARC::ThreadSafeQueue.11"* %4, i32* nonnull dereferenceable(4) %6)
  %63 = load float, float* %5, align 4
  %64 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %9, i8* nonnull dereferenceable(1) %11)
  %65 = load i1, i1* %10, align 1
  %66 = fadd float %51, %63
  %67 = select i1 %65, float %66, float %51
  %68 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %34, i8* nonnull dereferenceable(1) %36)
  %69 = load i1, i1* %35, align 1
  br i1 %69, label %70, label %.loopexit

; <label>:70:                                     ; preds = %.loopexit1
  %71 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %39, i8* nonnull dereferenceable(1) %41)
  %72 = load i1, i1* %40, align 1
  br i1 %72, label %.loopexit, label %.lr.ph2.preheader

.lr.ph2.preheader:                                ; preds = %70
  br label %.lr.ph2

.lr.ph2:                                          ; preds = %.lr.ph2.preheader, %.lr.ph2
  %73 = phi float [ %77, %.lr.ph2 ], [ %67, %.lr.ph2.preheader ]
  %74 = fpext float %73 to double
  %75 = fadd double %74, -1.980000e-01
  %76 = fptrunc double %75 to float
  %77 = tail call float @_ZSt4sqrtf(float %76)
  %78 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %39, i8* nonnull dereferenceable(1) %41)
  %79 = load i1, i1* %40, align 1
  br i1 %79, label %.loopexit, label %.lr.ph2

.loopexit:                                        ; preds = %.lr.ph2, %70, %.loopexit1
  %80 = phi float [ %67, %.loopexit1 ], [ %67, %70 ], [ %77, %.lr.ph2 ]
  %81 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %14, i8* nonnull dereferenceable(1) %16)
  %82 = load i1, i1* %15, align 1
  br i1 %82, label %._crit_edge, label %.lr.ph3
}

define internal void @7(i8* nocapture readonly, i8* nocapture readonly) {
  %3 = getelementptr inbounds i8, i8* %1, i64 24
  %4 = bitcast i8* %3 to %"class.MARC::ThreadSafeQueue"**
  %5 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %4, align 8
  %6 = alloca i1, align 1
  %7 = bitcast i1* %6 to i8*
  %8 = getelementptr inbounds i8, i8* %1, i64 96
  %9 = bitcast i8* %8 to %"class.MARC::ThreadSafeQueue"**
  %10 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %9, align 8
  %11 = alloca i1, align 1
  %12 = bitcast i1* %11 to i8*
  %13 = getelementptr inbounds i8, i8* %1, i64 168
  %14 = bitcast i8* %13 to %"class.MARC::ThreadSafeQueue"**
  %15 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %14, align 8
  %16 = alloca i1, align 1
  %17 = bitcast i1* %16 to i8*
  %18 = getelementptr inbounds i8, i8* %1, i64 256
  %19 = bitcast i8* %18 to %"class.MARC::ThreadSafeQueue"**
  %20 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %19, align 8
  %21 = alloca i1, align 1
  %22 = bitcast i1* %21 to i8*
  %23 = getelementptr inbounds i8, i8* %1, i64 336
  %24 = bitcast i8* %23 to %"class.MARC::ThreadSafeQueue"**
  %25 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %24, align 8
  %26 = alloca i1, align 1
  %27 = bitcast i1* %26 to i8*
  %28 = getelementptr inbounds i8, i8* %1, i64 488
  %29 = bitcast i8* %28 to %"class.MARC::ThreadSafeQueue"**
  %30 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %29, align 8
  %31 = alloca i1, align 1
  %32 = bitcast i1* %31 to i8*
  %33 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %10, i8* nonnull dereferenceable(1) %12)
  %34 = load i1, i1* %11, align 1
  br i1 %34, label %._crit_edge, label %.lr.ph2.preheader

.lr.ph2.preheader:                                ; preds = %2
  br label %.lr.ph2

._crit_edge:                                      ; preds = %.loopexit, %2
  %35 = getelementptr inbounds i8, i8* %0, i64 24
  %36 = bitcast i8* %35 to i32**
  %37 = load i32*, i32** %36, align 8
  store i32 0, i32* %37, align 4
  ret void

.lr.ph2:                                          ; preds = %.lr.ph2.preheader, %.loopexit
  %38 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %15, i8* nonnull dereferenceable(1) %17)
  %39 = load i1, i1* %16, align 1
  br i1 %39, label %40, label %.loopexit1

; <label>:40:                                     ; preds = %.lr.ph2
  %41 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %20, i8* nonnull dereferenceable(1) %22)
  %42 = load i1, i1* %21, align 1
  br i1 %42, label %.loopexit1, label %.lr.ph.preheader

.lr.ph.preheader:                                 ; preds = %40
  br label %.lr.ph

.lr.ph:                                           ; preds = %.lr.ph.backedge, %.lr.ph.preheader
  %43 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %25, i8* nonnull dereferenceable(1) %27)
  %44 = load i1, i1* %26, align 1
  br i1 %44, label %45, label %.lr.ph.backedge

.lr.ph.backedge:                                  ; preds = %.lr.ph, %45
  br label %.lr.ph

; <label>:45:                                     ; preds = %.lr.ph
  %46 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %20, i8* nonnull dereferenceable(1) %22)
  %47 = load i1, i1* %21, align 1
  br i1 %47, label %.loopexit1, label %.lr.ph.backedge

.loopexit1:                                       ; preds = %45, %40, %.lr.ph2
  %48 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %5, i8* nonnull dereferenceable(1) %7)
  %49 = load i1, i1* %6, align 1
  br i1 %49, label %.preheader, label %.loopexit

.preheader:                                       ; preds = %.loopexit1
  br label %50

; <label>:50:                                     ; preds = %.preheader, %50
  %51 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %30, i8* nonnull dereferenceable(1) %32)
  %52 = load i1, i1* %31, align 1
  br i1 %52, label %.loopexit, label %50

.loopexit:                                        ; preds = %50, %.loopexit1
  %53 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %10, i8* nonnull dereferenceable(1) %12)
  %54 = load i1, i1* %11, align 1
  br i1 %54, label %._crit_edge, label %.lr.ph2
}

define internal void @8(i8* nocapture readonly, i8* nocapture readonly) {
  %3 = getelementptr inbounds i8, i8* %1, i64 208
  %4 = bitcast i8* %3 to %"class.MARC::ThreadSafeQueue"**
  %5 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %4, align 8
  %.sroa.018 = alloca i8, align 1
  %6 = getelementptr inbounds i8, i8* %1, i64 216
  %7 = bitcast i8* %6 to %"class.MARC::ThreadSafeQueue"**
  %8 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %7, align 8
  %.sroa.016 = alloca i8, align 1
  %9 = getelementptr inbounds i8, i8* %1, i64 224
  %10 = bitcast i8* %9 to %"class.MARC::ThreadSafeQueue"**
  %11 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %10, align 8
  %.sroa.014 = alloca i8, align 1
  %12 = getelementptr inbounds i8, i8* %1, i64 232
  %13 = bitcast i8* %12 to %"class.MARC::ThreadSafeQueue"**
  %14 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %13, align 8
  %.sroa.012 = alloca i8, align 1
  %15 = getelementptr inbounds i8, i8* %1, i64 240
  %16 = bitcast i8* %15 to %"class.MARC::ThreadSafeQueue"**
  %17 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %16, align 8
  %.sroa.010 = alloca i8, align 1
  %18 = getelementptr inbounds i8, i8* %1, i64 248
  %19 = bitcast i8* %18 to %"class.MARC::ThreadSafeQueue"**
  %20 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %19, align 8
  %.sroa.08 = alloca i8, align 1
  %21 = getelementptr inbounds i8, i8* %1, i64 256
  %22 = bitcast i8* %21 to %"class.MARC::ThreadSafeQueue"**
  %23 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %22, align 8
  %.sroa.06 = alloca i8, align 1
  %24 = getelementptr inbounds i8, i8* %1, i64 264
  %25 = bitcast i8* %24 to %"class.MARC::ThreadSafeQueue"**
  %26 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %25, align 8
  %.sroa.04 = alloca i8, align 1
  %27 = getelementptr inbounds i8, i8* %1, i64 272
  %28 = bitcast i8* %27 to %"class.MARC::ThreadSafeQueue"**
  %29 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %28, align 8
  %.sroa.02 = alloca i8, align 1
  %30 = getelementptr inbounds i8, i8* %1, i64 280
  %31 = bitcast i8* %30 to %"class.MARC::ThreadSafeQueue"**
  %32 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %31, align 8
  %.sroa.0 = alloca i8, align 1
  %33 = getelementptr inbounds i8, i8* %1, i64 104
  %34 = bitcast i8* %33 to %"class.MARC::ThreadSafeQueue"**
  %35 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %34, align 8
  %36 = alloca i1, align 1
  %37 = bitcast i1* %36 to i8*
  %38 = getelementptr inbounds i8, i8* %1, i64 176
  %39 = bitcast i8* %38 to %"class.MARC::ThreadSafeQueue"**
  %40 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %39, align 8
  %41 = alloca i1, align 1
  %42 = bitcast i1* %41 to i8*
  %43 = getelementptr inbounds i8, i8* %1, i64 344
  %44 = bitcast i8* %43 to %"class.MARC::ThreadSafeQueue"**
  %45 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %44, align 8
  %46 = alloca i1, align 1
  %47 = bitcast i1* %46 to i8*
  %48 = getelementptr inbounds i8, i8* %1, i64 408
  %49 = bitcast i8* %48 to %"class.MARC::ThreadSafeQueue"**
  %50 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %49, align 8
  %51 = alloca i1, align 1
  %52 = bitcast i1* %51 to i8*
  %53 = getelementptr inbounds i8, i8* %1, i64 496
  %54 = bitcast i8* %53 to %"class.MARC::ThreadSafeQueue"**
  %55 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %54, align 8
  %56 = alloca i1, align 1
  %57 = bitcast i1* %56 to i8*
  %58 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %35, i8* nonnull dereferenceable(1) %37)
  %59 = load i1, i1* %36, align 1
  br i1 %59, label %._crit_edge, label %.lr.ph

.lr.ph:                                           ; preds = %2
  %.sroa.0.0.sroa_cast1 = bitcast i8* %.sroa.0 to i1*
  %.sroa.02.0.sroa_cast3 = bitcast i8* %.sroa.02 to i1*
  %.sroa.04.0.sroa_cast5 = bitcast i8* %.sroa.04 to i1*
  %.sroa.06.0.sroa_cast7 = bitcast i8* %.sroa.06 to i1*
  %.sroa.08.0.sroa_cast9 = bitcast i8* %.sroa.08 to i1*
  %.sroa.010.0.sroa_cast11 = bitcast i8* %.sroa.010 to i1*
  %.sroa.012.0.sroa_cast13 = bitcast i8* %.sroa.012 to i1*
  %.sroa.014.0.sroa_cast15 = bitcast i8* %.sroa.014 to i1*
  %.sroa.016.0.sroa_cast17 = bitcast i8* %.sroa.016 to i1*
  %.sroa.018.0.sroa_cast19 = bitcast i8* %.sroa.018 to i1*
  br label %63

._crit_edge:                                      ; preds = %.loopexit, %2
  %60 = getelementptr inbounds i8, i8* %0, i64 24
  %61 = bitcast i8* %60 to i32**
  %62 = load i32*, i32** %61, align 8
  store i32 0, i32* %62, align 4
  ret void

; <label>:63:                                     ; preds = %.lr.ph, %.loopexit
  %64 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %40, i8* nonnull dereferenceable(1) %42)
  %65 = load i1, i1* %41, align 1
  br i1 %65, label %.preheader22, label %.loopexit20

.preheader22:                                     ; preds = %63
  br label %66

; <label>:66:                                     ; preds = %.preheader22, %72
  %67 = phi i32 [ %73, %72 ], [ 0, %.preheader22 ]
  %68 = icmp eq i32 %67, 10
  store i1 %68, i1* %.sroa.0.0.sroa_cast1, align 1
  %.sroa.0.0..sroa.0.0. = load i8, i8* %.sroa.0, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %32, i8 signext %.sroa.0.0..sroa.0.0.)
  store i1 %68, i1* %.sroa.02.0.sroa_cast3, align 1
  %.sroa.02.0..sroa.02.0. = load i8, i8* %.sroa.02, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %29, i8 signext %.sroa.02.0..sroa.02.0.)
  store i1 %68, i1* %.sroa.04.0.sroa_cast5, align 1
  %.sroa.04.0..sroa.04.0. = load i8, i8* %.sroa.04, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %26, i8 signext %.sroa.04.0..sroa.04.0.)
  store i1 %68, i1* %.sroa.06.0.sroa_cast7, align 1
  %.sroa.06.0..sroa.06.0. = load i8, i8* %.sroa.06, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %23, i8 signext %.sroa.06.0..sroa.06.0.)
  store i1 %68, i1* %.sroa.08.0.sroa_cast9, align 1
  %.sroa.08.0..sroa.08.0. = load i8, i8* %.sroa.08, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %20, i8 signext %.sroa.08.0..sroa.08.0.)
  store i1 %68, i1* %.sroa.010.0.sroa_cast11, align 1
  %.sroa.010.0..sroa.010.0. = load i8, i8* %.sroa.010, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %17, i8 signext %.sroa.010.0..sroa.010.0.)
  store i1 %68, i1* %.sroa.012.0.sroa_cast13, align 1
  %.sroa.012.0..sroa.012.0. = load i8, i8* %.sroa.012, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %14, i8 signext %.sroa.012.0..sroa.012.0.)
  store i1 %68, i1* %.sroa.014.0.sroa_cast15, align 1
  %.sroa.014.0..sroa.014.0. = load i8, i8* %.sroa.014, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %11, i8 signext %.sroa.014.0..sroa.014.0.)
  store i1 %68, i1* %.sroa.016.0.sroa_cast17, align 1
  %.sroa.016.0..sroa.016.0. = load i8, i8* %.sroa.016, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %8, i8 signext %.sroa.016.0..sroa.016.0.)
  store i1 %68, i1* %.sroa.018.0.sroa_cast19, align 1
  %.sroa.018.0..sroa.018.0. = load i8, i8* %.sroa.018, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %5, i8 signext %.sroa.018.0..sroa.018.0.)
  br i1 %68, label %.loopexit20, label %.preheader

.preheader:                                       ; preds = %66
  br label %69

; <label>:69:                                     ; preds = %.preheader, %69
  %70 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %45, i8* nonnull dereferenceable(1) %47)
  %71 = load i1, i1* %46, align 1
  br i1 %71, label %72, label %69

; <label>:72:                                     ; preds = %69
  %73 = add nuw nsw i32 %67, 1
  br label %66

.loopexit20:                                      ; preds = %66, %63
  %74 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %50, i8* nonnull dereferenceable(1) %52)
  %75 = load i1, i1* %51, align 1
  br i1 %75, label %.preheader21, label %.loopexit

.preheader21:                                     ; preds = %.loopexit20
  br label %76

; <label>:76:                                     ; preds = %.preheader21, %76
  %77 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %55, i8* nonnull dereferenceable(1) %57)
  %78 = load i1, i1* %56, align 1
  br i1 %78, label %.loopexit, label %76

.loopexit:                                        ; preds = %76, %.loopexit20
  %79 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %35, i8* nonnull dereferenceable(1) %37)
  %80 = load i1, i1* %36, align 1
  br i1 %80, label %._crit_edge, label %63
}

define internal void @9(i8* nocapture readonly, i8* nocapture readonly) {
  %3 = getelementptr inbounds i8, i8* %1, i64 440
  %4 = bitcast i8* %3 to %"class.MARC::ThreadSafeQueue"**
  %5 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %4, align 8
  %.sroa.018 = alloca i8, align 1
  %6 = getelementptr inbounds i8, i8* %1, i64 448
  %7 = bitcast i8* %6 to %"class.MARC::ThreadSafeQueue"**
  %8 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %7, align 8
  %.sroa.016 = alloca i8, align 1
  %9 = getelementptr inbounds i8, i8* %1, i64 456
  %10 = bitcast i8* %9 to %"class.MARC::ThreadSafeQueue"**
  %11 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %10, align 8
  %.sroa.014 = alloca i8, align 1
  %12 = getelementptr inbounds i8, i8* %1, i64 464
  %13 = bitcast i8* %12 to %"class.MARC::ThreadSafeQueue"**
  %14 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %13, align 8
  %.sroa.012 = alloca i8, align 1
  %15 = getelementptr inbounds i8, i8* %1, i64 472
  %16 = bitcast i8* %15 to %"class.MARC::ThreadSafeQueue"**
  %17 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %16, align 8
  %.sroa.010 = alloca i8, align 1
  %18 = getelementptr inbounds i8, i8* %1, i64 480
  %19 = bitcast i8* %18 to %"class.MARC::ThreadSafeQueue"**
  %20 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %19, align 8
  %.sroa.08 = alloca i8, align 1
  %21 = getelementptr inbounds i8, i8* %1, i64 488
  %22 = bitcast i8* %21 to %"class.MARC::ThreadSafeQueue"**
  %23 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %22, align 8
  %.sroa.06 = alloca i8, align 1
  %24 = getelementptr inbounds i8, i8* %1, i64 496
  %25 = bitcast i8* %24 to %"class.MARC::ThreadSafeQueue"**
  %26 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %25, align 8
  %.sroa.04 = alloca i8, align 1
  %27 = getelementptr inbounds i8, i8* %1, i64 504
  %28 = bitcast i8* %27 to %"class.MARC::ThreadSafeQueue"**
  %29 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %28, align 8
  %.sroa.02 = alloca i8, align 1
  %30 = getelementptr inbounds i8, i8* %1, i64 512
  %31 = bitcast i8* %30 to %"class.MARC::ThreadSafeQueue"**
  %32 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %31, align 8
  %.sroa.0 = alloca i8, align 1
  %33 = getelementptr inbounds i8, i8* %1, i64 112
  %34 = bitcast i8* %33 to %"class.MARC::ThreadSafeQueue"**
  %35 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %34, align 8
  %36 = alloca i1, align 1
  %37 = bitcast i1* %36 to i8*
  %38 = getelementptr inbounds i8, i8* %1, i64 184
  %39 = bitcast i8* %38 to %"class.MARC::ThreadSafeQueue"**
  %40 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %39, align 8
  %41 = alloca i1, align 1
  %42 = bitcast i1* %41 to i8*
  %43 = getelementptr inbounds i8, i8* %1, i64 264
  %44 = bitcast i8* %43 to %"class.MARC::ThreadSafeQueue"**
  %45 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %44, align 8
  %46 = alloca i1, align 1
  %47 = bitcast i1* %46 to i8*
  %48 = getelementptr inbounds i8, i8* %1, i64 352
  %49 = bitcast i8* %48 to %"class.MARC::ThreadSafeQueue"**
  %50 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %49, align 8
  %51 = alloca i1, align 1
  %52 = bitcast i1* %51 to i8*
  %53 = getelementptr inbounds i8, i8* %1, i64 416
  %54 = bitcast i8* %53 to %"class.MARC::ThreadSafeQueue"**
  %55 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %54, align 8
  %56 = alloca i1, align 1
  %57 = bitcast i1* %56 to i8*
  %58 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %35, i8* nonnull dereferenceable(1) %37)
  %59 = load i1, i1* %36, align 1
  br i1 %59, label %._crit_edge, label %.lr.ph21

.lr.ph21:                                         ; preds = %2
  %.sroa.0.0.sroa_cast1 = bitcast i8* %.sroa.0 to i1*
  %.sroa.02.0.sroa_cast3 = bitcast i8* %.sroa.02 to i1*
  %.sroa.04.0.sroa_cast5 = bitcast i8* %.sroa.04 to i1*
  %.sroa.06.0.sroa_cast7 = bitcast i8* %.sroa.06 to i1*
  %.sroa.08.0.sroa_cast9 = bitcast i8* %.sroa.08 to i1*
  %.sroa.010.0.sroa_cast11 = bitcast i8* %.sroa.010 to i1*
  %.sroa.012.0.sroa_cast13 = bitcast i8* %.sroa.012 to i1*
  %.sroa.014.0.sroa_cast15 = bitcast i8* %.sroa.014 to i1*
  %.sroa.016.0.sroa_cast17 = bitcast i8* %.sroa.016 to i1*
  %.sroa.018.0.sroa_cast19 = bitcast i8* %.sroa.018 to i1*
  br label %63

._crit_edge:                                      ; preds = %.loopexit, %2
  %60 = getelementptr inbounds i8, i8* %0, i64 24
  %61 = bitcast i8* %60 to i32**
  %62 = load i32*, i32** %61, align 8
  store i32 0, i32* %62, align 4
  ret void

; <label>:63:                                     ; preds = %.lr.ph21, %.loopexit
  %64 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %40, i8* nonnull dereferenceable(1) %42)
  %65 = load i1, i1* %41, align 1
  br i1 %65, label %66, label %.loopexit20

; <label>:66:                                     ; preds = %63
  %67 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %45, i8* nonnull dereferenceable(1) %47)
  %68 = load i1, i1* %46, align 1
  br i1 %68, label %.loopexit20, label %.lr.ph.preheader

.lr.ph.preheader:                                 ; preds = %66
  br label %.lr.ph

.lr.ph:                                           ; preds = %.lr.ph.backedge, %.lr.ph.preheader
  %69 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %50, i8* nonnull dereferenceable(1) %52)
  %70 = load i1, i1* %51, align 1
  br i1 %70, label %71, label %.lr.ph.backedge

.lr.ph.backedge:                                  ; preds = %.lr.ph, %71
  br label %.lr.ph

; <label>:71:                                     ; preds = %.lr.ph
  %72 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %45, i8* nonnull dereferenceable(1) %47)
  %73 = load i1, i1* %46, align 1
  br i1 %73, label %.loopexit20, label %.lr.ph.backedge

.loopexit20:                                      ; preds = %71, %66, %63
  %74 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %55, i8* nonnull dereferenceable(1) %57)
  %75 = load i1, i1* %56, align 1
  br i1 %75, label %.preheader, label %.loopexit

.preheader:                                       ; preds = %.loopexit20
  br label %76

; <label>:76:                                     ; preds = %.preheader, %76
  %77 = phi i32 [ %79, %76 ], [ 0, %.preheader ]
  %78 = icmp eq i32 %77, 5
  store i1 %78, i1* %.sroa.0.0.sroa_cast1, align 1
  %.sroa.0.0..sroa.0.0. = load i8, i8* %.sroa.0, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %32, i8 signext %.sroa.0.0..sroa.0.0.)
  store i1 %78, i1* %.sroa.02.0.sroa_cast3, align 1
  %.sroa.02.0..sroa.02.0. = load i8, i8* %.sroa.02, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %29, i8 signext %.sroa.02.0..sroa.02.0.)
  store i1 %78, i1* %.sroa.04.0.sroa_cast5, align 1
  %.sroa.04.0..sroa.04.0. = load i8, i8* %.sroa.04, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %26, i8 signext %.sroa.04.0..sroa.04.0.)
  store i1 %78, i1* %.sroa.06.0.sroa_cast7, align 1
  %.sroa.06.0..sroa.06.0. = load i8, i8* %.sroa.06, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %23, i8 signext %.sroa.06.0..sroa.06.0.)
  store i1 %78, i1* %.sroa.08.0.sroa_cast9, align 1
  %.sroa.08.0..sroa.08.0. = load i8, i8* %.sroa.08, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %20, i8 signext %.sroa.08.0..sroa.08.0.)
  store i1 %78, i1* %.sroa.010.0.sroa_cast11, align 1
  %.sroa.010.0..sroa.010.0. = load i8, i8* %.sroa.010, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %17, i8 signext %.sroa.010.0..sroa.010.0.)
  store i1 %78, i1* %.sroa.012.0.sroa_cast13, align 1
  %.sroa.012.0..sroa.012.0. = load i8, i8* %.sroa.012, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %14, i8 signext %.sroa.012.0..sroa.012.0.)
  store i1 %78, i1* %.sroa.014.0.sroa_cast15, align 1
  %.sroa.014.0..sroa.014.0. = load i8, i8* %.sroa.014, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %11, i8 signext %.sroa.014.0..sroa.014.0.)
  store i1 %78, i1* %.sroa.016.0.sroa_cast17, align 1
  %.sroa.016.0..sroa.016.0. = load i8, i8* %.sroa.016, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %8, i8 signext %.sroa.016.0..sroa.016.0.)
  store i1 %78, i1* %.sroa.018.0.sroa_cast19, align 1
  %.sroa.018.0..sroa.018.0. = load i8, i8* %.sroa.018, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %5, i8 signext %.sroa.018.0..sroa.018.0.)
  %79 = add nuw nsw i32 %77, 1
  br i1 %78, label %.loopexit, label %76

.loopexit:                                        ; preds = %76, %.loopexit20
  %80 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %35, i8* nonnull dereferenceable(1) %37)
  %81 = load i1, i1* %36, align 1
  br i1 %81, label %._crit_edge, label %63
}

define internal void @10(i8* nocapture readonly, i8* nocapture readonly) {
  %3 = getelementptr inbounds i8, i8* %1, i64 288
  %4 = bitcast i8* %3 to %"class.MARC::ThreadSafeQueue"**
  %5 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %4, align 8
  %.sroa.018 = alloca i8, align 1
  %6 = getelementptr inbounds i8, i8* %1, i64 296
  %7 = bitcast i8* %6 to %"class.MARC::ThreadSafeQueue"**
  %8 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %7, align 8
  %.sroa.016 = alloca i8, align 1
  %9 = getelementptr inbounds i8, i8* %1, i64 304
  %10 = bitcast i8* %9 to %"class.MARC::ThreadSafeQueue"**
  %11 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %10, align 8
  %.sroa.014 = alloca i8, align 1
  %12 = getelementptr inbounds i8, i8* %1, i64 312
  %13 = bitcast i8* %12 to %"class.MARC::ThreadSafeQueue"**
  %14 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %13, align 8
  %.sroa.012 = alloca i8, align 1
  %15 = getelementptr inbounds i8, i8* %1, i64 320
  %16 = bitcast i8* %15 to %"class.MARC::ThreadSafeQueue"**
  %17 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %16, align 8
  %.sroa.010 = alloca i8, align 1
  %18 = getelementptr inbounds i8, i8* %1, i64 328
  %19 = bitcast i8* %18 to %"class.MARC::ThreadSafeQueue"**
  %20 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %19, align 8
  %.sroa.08 = alloca i8, align 1
  %21 = getelementptr inbounds i8, i8* %1, i64 336
  %22 = bitcast i8* %21 to %"class.MARC::ThreadSafeQueue"**
  %23 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %22, align 8
  %.sroa.06 = alloca i8, align 1
  %24 = getelementptr inbounds i8, i8* %1, i64 344
  %25 = bitcast i8* %24 to %"class.MARC::ThreadSafeQueue"**
  %26 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %25, align 8
  %.sroa.04 = alloca i8, align 1
  %27 = getelementptr inbounds i8, i8* %1, i64 352
  %28 = bitcast i8* %27 to %"class.MARC::ThreadSafeQueue"**
  %29 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %28, align 8
  %.sroa.02 = alloca i8, align 1
  %30 = getelementptr inbounds i8, i8* %1, i64 360
  %31 = bitcast i8* %30 to %"class.MARC::ThreadSafeQueue"**
  %32 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %31, align 8
  %.sroa.0 = alloca i8, align 1
  %33 = getelementptr inbounds i8, i8* %1, i64 120
  %34 = bitcast i8* %33 to %"class.MARC::ThreadSafeQueue"**
  %35 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %34, align 8
  %36 = alloca i1, align 1
  %37 = bitcast i1* %36 to i8*
  %38 = getelementptr inbounds i8, i8* %1, i64 192
  %39 = bitcast i8* %38 to %"class.MARC::ThreadSafeQueue"**
  %40 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %39, align 8
  %41 = alloca i1, align 1
  %42 = bitcast i1* %41 to i8*
  %43 = getelementptr inbounds i8, i8* %1, i64 272
  %44 = bitcast i8* %43 to %"class.MARC::ThreadSafeQueue"**
  %45 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %44, align 8
  %46 = alloca i1, align 1
  %47 = bitcast i1* %46 to i8*
  %48 = getelementptr inbounds i8, i8* %1, i64 424
  %49 = bitcast i8* %48 to %"class.MARC::ThreadSafeQueue"**
  %50 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %49, align 8
  %51 = alloca i1, align 1
  %52 = bitcast i1* %51 to i8*
  %53 = getelementptr inbounds i8, i8* %1, i64 504
  %54 = bitcast i8* %53 to %"class.MARC::ThreadSafeQueue"**
  %55 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %54, align 8
  %56 = alloca i1, align 1
  %57 = bitcast i1* %56 to i8*
  %58 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %35, i8* nonnull dereferenceable(1) %37)
  %59 = load i1, i1* %36, align 1
  br i1 %59, label %._crit_edge, label %.lr.ph21

.lr.ph21:                                         ; preds = %2
  %.sroa.0.0.sroa_cast1 = bitcast i8* %.sroa.0 to i1*
  %.sroa.02.0.sroa_cast3 = bitcast i8* %.sroa.02 to i1*
  %.sroa.04.0.sroa_cast5 = bitcast i8* %.sroa.04 to i1*
  %.sroa.06.0.sroa_cast7 = bitcast i8* %.sroa.06 to i1*
  %.sroa.08.0.sroa_cast9 = bitcast i8* %.sroa.08 to i1*
  %.sroa.010.0.sroa_cast11 = bitcast i8* %.sroa.010 to i1*
  %.sroa.012.0.sroa_cast13 = bitcast i8* %.sroa.012 to i1*
  %.sroa.014.0.sroa_cast15 = bitcast i8* %.sroa.014 to i1*
  %.sroa.016.0.sroa_cast17 = bitcast i8* %.sroa.016 to i1*
  %.sroa.018.0.sroa_cast19 = bitcast i8* %.sroa.018 to i1*
  br label %63

._crit_edge:                                      ; preds = %.loopexit, %2
  %60 = getelementptr inbounds i8, i8* %0, i64 24
  %61 = bitcast i8* %60 to i32**
  %62 = load i32*, i32** %61, align 8
  store i32 0, i32* %62, align 4
  ret void

; <label>:63:                                     ; preds = %.lr.ph21, %.loopexit
  %64 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %40, i8* nonnull dereferenceable(1) %42)
  %65 = load i1, i1* %41, align 1
  br i1 %65, label %66, label %.loopexit20

; <label>:66:                                     ; preds = %63
  %67 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %45, i8* nonnull dereferenceable(1) %47)
  %68 = load i1, i1* %46, align 1
  br i1 %68, label %.loopexit20, label %.lr.ph.preheader

.lr.ph.preheader:                                 ; preds = %66
  br label %.lr.ph

.lr.ph:                                           ; preds = %.lr.ph.preheader, %.lr.ph
  store i1 false, i1* %.sroa.0.0.sroa_cast1, align 1
  %.sroa.0.0..sroa.0.0. = load i8, i8* %.sroa.0, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %32, i8 signext %.sroa.0.0..sroa.0.0.)
  store i1 false, i1* %.sroa.02.0.sroa_cast3, align 1
  %.sroa.02.0..sroa.02.0. = load i8, i8* %.sroa.02, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %29, i8 signext %.sroa.02.0..sroa.02.0.)
  store i1 false, i1* %.sroa.04.0.sroa_cast5, align 1
  %.sroa.04.0..sroa.04.0. = load i8, i8* %.sroa.04, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %26, i8 signext %.sroa.04.0..sroa.04.0.)
  store i1 false, i1* %.sroa.06.0.sroa_cast7, align 1
  %.sroa.06.0..sroa.06.0. = load i8, i8* %.sroa.06, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %23, i8 signext %.sroa.06.0..sroa.06.0.)
  store i1 false, i1* %.sroa.08.0.sroa_cast9, align 1
  %.sroa.08.0..sroa.08.0. = load i8, i8* %.sroa.08, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %20, i8 signext %.sroa.08.0..sroa.08.0.)
  store i1 false, i1* %.sroa.010.0.sroa_cast11, align 1
  %.sroa.010.0..sroa.010.0. = load i8, i8* %.sroa.010, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %17, i8 signext %.sroa.010.0..sroa.010.0.)
  store i1 false, i1* %.sroa.012.0.sroa_cast13, align 1
  %.sroa.012.0..sroa.012.0. = load i8, i8* %.sroa.012, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %14, i8 signext %.sroa.012.0..sroa.012.0.)
  store i1 false, i1* %.sroa.014.0.sroa_cast15, align 1
  %.sroa.014.0..sroa.014.0. = load i8, i8* %.sroa.014, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %11, i8 signext %.sroa.014.0..sroa.014.0.)
  store i1 false, i1* %.sroa.016.0.sroa_cast17, align 1
  %.sroa.016.0..sroa.016.0. = load i8, i8* %.sroa.016, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %8, i8 signext %.sroa.016.0..sroa.016.0.)
  store i1 false, i1* %.sroa.018.0.sroa_cast19, align 1
  %.sroa.018.0..sroa.018.0. = load i8, i8* %.sroa.018, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %5, i8 signext %.sroa.018.0..sroa.018.0.)
  store i1 false, i1* %.sroa.0.0.sroa_cast1, align 1
  %.sroa.0.0..sroa.0.0..1 = load i8, i8* %.sroa.0, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %32, i8 signext %.sroa.0.0..sroa.0.0..1)
  store i1 false, i1* %.sroa.02.0.sroa_cast3, align 1
  %.sroa.02.0..sroa.02.0..1 = load i8, i8* %.sroa.02, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %29, i8 signext %.sroa.02.0..sroa.02.0..1)
  store i1 false, i1* %.sroa.04.0.sroa_cast5, align 1
  %.sroa.04.0..sroa.04.0..1 = load i8, i8* %.sroa.04, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %26, i8 signext %.sroa.04.0..sroa.04.0..1)
  store i1 false, i1* %.sroa.06.0.sroa_cast7, align 1
  %.sroa.06.0..sroa.06.0..1 = load i8, i8* %.sroa.06, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %23, i8 signext %.sroa.06.0..sroa.06.0..1)
  store i1 false, i1* %.sroa.08.0.sroa_cast9, align 1
  %.sroa.08.0..sroa.08.0..1 = load i8, i8* %.sroa.08, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %20, i8 signext %.sroa.08.0..sroa.08.0..1)
  store i1 false, i1* %.sroa.010.0.sroa_cast11, align 1
  %.sroa.010.0..sroa.010.0..1 = load i8, i8* %.sroa.010, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %17, i8 signext %.sroa.010.0..sroa.010.0..1)
  store i1 false, i1* %.sroa.012.0.sroa_cast13, align 1
  %.sroa.012.0..sroa.012.0..1 = load i8, i8* %.sroa.012, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %14, i8 signext %.sroa.012.0..sroa.012.0..1)
  store i1 false, i1* %.sroa.014.0.sroa_cast15, align 1
  %.sroa.014.0..sroa.014.0..1 = load i8, i8* %.sroa.014, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %11, i8 signext %.sroa.014.0..sroa.014.0..1)
  store i1 false, i1* %.sroa.016.0.sroa_cast17, align 1
  %.sroa.016.0..sroa.016.0..1 = load i8, i8* %.sroa.016, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %8, i8 signext %.sroa.016.0..sroa.016.0..1)
  store i1 false, i1* %.sroa.018.0.sroa_cast19, align 1
  %.sroa.018.0..sroa.018.0..1 = load i8, i8* %.sroa.018, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %5, i8 signext %.sroa.018.0..sroa.018.0..1)
  store i1 false, i1* %.sroa.0.0.sroa_cast1, align 1
  %.sroa.0.0..sroa.0.0..2 = load i8, i8* %.sroa.0, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %32, i8 signext %.sroa.0.0..sroa.0.0..2)
  store i1 false, i1* %.sroa.02.0.sroa_cast3, align 1
  %.sroa.02.0..sroa.02.0..2 = load i8, i8* %.sroa.02, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %29, i8 signext %.sroa.02.0..sroa.02.0..2)
  store i1 false, i1* %.sroa.04.0.sroa_cast5, align 1
  %.sroa.04.0..sroa.04.0..2 = load i8, i8* %.sroa.04, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %26, i8 signext %.sroa.04.0..sroa.04.0..2)
  store i1 false, i1* %.sroa.06.0.sroa_cast7, align 1
  %.sroa.06.0..sroa.06.0..2 = load i8, i8* %.sroa.06, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %23, i8 signext %.sroa.06.0..sroa.06.0..2)
  store i1 false, i1* %.sroa.08.0.sroa_cast9, align 1
  %.sroa.08.0..sroa.08.0..2 = load i8, i8* %.sroa.08, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %20, i8 signext %.sroa.08.0..sroa.08.0..2)
  store i1 false, i1* %.sroa.010.0.sroa_cast11, align 1
  %.sroa.010.0..sroa.010.0..2 = load i8, i8* %.sroa.010, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %17, i8 signext %.sroa.010.0..sroa.010.0..2)
  store i1 false, i1* %.sroa.012.0.sroa_cast13, align 1
  %.sroa.012.0..sroa.012.0..2 = load i8, i8* %.sroa.012, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %14, i8 signext %.sroa.012.0..sroa.012.0..2)
  store i1 false, i1* %.sroa.014.0.sroa_cast15, align 1
  %.sroa.014.0..sroa.014.0..2 = load i8, i8* %.sroa.014, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %11, i8 signext %.sroa.014.0..sroa.014.0..2)
  store i1 false, i1* %.sroa.016.0.sroa_cast17, align 1
  %.sroa.016.0..sroa.016.0..2 = load i8, i8* %.sroa.016, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %8, i8 signext %.sroa.016.0..sroa.016.0..2)
  store i1 false, i1* %.sroa.018.0.sroa_cast19, align 1
  %.sroa.018.0..sroa.018.0..2 = load i8, i8* %.sroa.018, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %5, i8 signext %.sroa.018.0..sroa.018.0..2)
  store i1 true, i1* %.sroa.0.0.sroa_cast1, align 1
  %.sroa.0.0..sroa.0.0..3 = load i8, i8* %.sroa.0, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %32, i8 signext %.sroa.0.0..sroa.0.0..3)
  store i1 true, i1* %.sroa.02.0.sroa_cast3, align 1
  %.sroa.02.0..sroa.02.0..3 = load i8, i8* %.sroa.02, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %29, i8 signext %.sroa.02.0..sroa.02.0..3)
  store i1 true, i1* %.sroa.04.0.sroa_cast5, align 1
  %.sroa.04.0..sroa.04.0..3 = load i8, i8* %.sroa.04, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %26, i8 signext %.sroa.04.0..sroa.04.0..3)
  store i1 true, i1* %.sroa.06.0.sroa_cast7, align 1
  %.sroa.06.0..sroa.06.0..3 = load i8, i8* %.sroa.06, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %23, i8 signext %.sroa.06.0..sroa.06.0..3)
  store i1 true, i1* %.sroa.08.0.sroa_cast9, align 1
  %.sroa.08.0..sroa.08.0..3 = load i8, i8* %.sroa.08, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %20, i8 signext %.sroa.08.0..sroa.08.0..3)
  store i1 true, i1* %.sroa.010.0.sroa_cast11, align 1
  %.sroa.010.0..sroa.010.0..3 = load i8, i8* %.sroa.010, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %17, i8 signext %.sroa.010.0..sroa.010.0..3)
  store i1 true, i1* %.sroa.012.0.sroa_cast13, align 1
  %.sroa.012.0..sroa.012.0..3 = load i8, i8* %.sroa.012, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %14, i8 signext %.sroa.012.0..sroa.012.0..3)
  store i1 true, i1* %.sroa.014.0.sroa_cast15, align 1
  %.sroa.014.0..sroa.014.0..3 = load i8, i8* %.sroa.014, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %11, i8 signext %.sroa.014.0..sroa.014.0..3)
  store i1 true, i1* %.sroa.016.0.sroa_cast17, align 1
  %.sroa.016.0..sroa.016.0..3 = load i8, i8* %.sroa.016, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %8, i8 signext %.sroa.016.0..sroa.016.0..3)
  store i1 true, i1* %.sroa.018.0.sroa_cast19, align 1
  %.sroa.018.0..sroa.018.0..3 = load i8, i8* %.sroa.018, align 1
  call void @_ZN4MARC15ThreadSafeQueueIaE4pushEa(%"class.MARC::ThreadSafeQueue"* %5, i8 signext %.sroa.018.0..sroa.018.0..3)
  %69 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %45, i8* nonnull dereferenceable(1) %47)
  %70 = load i1, i1* %46, align 1
  br i1 %70, label %.loopexit20, label %.lr.ph

.loopexit20:                                      ; preds = %.lr.ph, %66, %63
  %71 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %50, i8* nonnull dereferenceable(1) %52)
  %72 = load i1, i1* %51, align 1
  br i1 %72, label %.preheader, label %.loopexit

.preheader:                                       ; preds = %.loopexit20
  br label %73

; <label>:73:                                     ; preds = %.preheader, %73
  %74 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %55, i8* nonnull dereferenceable(1) %57)
  %75 = load i1, i1* %56, align 1
  br i1 %75, label %.loopexit, label %73

.loopexit:                                        ; preds = %73, %.loopexit20
  %76 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %35, i8* nonnull dereferenceable(1) %37)
  %77 = load i1, i1* %36, align 1
  br i1 %77, label %._crit_edge, label %63
}

define internal void @11(i8* nocapture readonly, i8* nocapture readonly) {
  %3 = bitcast i8* %1 to %"class.MARC::ThreadSafeQueue.11"**
  %4 = load %"class.MARC::ThreadSafeQueue.11"*, %"class.MARC::ThreadSafeQueue.11"** %3, align 8
  %5 = getelementptr inbounds i8, i8* %1, i64 128
  %6 = bitcast i8* %5 to %"class.MARC::ThreadSafeQueue"**
  %7 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %6, align 8
  %8 = alloca i1, align 1
  %9 = bitcast i1* %8 to i8*
  %10 = getelementptr inbounds i8, i8* %1, i64 200
  %11 = bitcast i8* %10 to %"class.MARC::ThreadSafeQueue"**
  %12 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %11, align 8
  %13 = alloca i1, align 1
  %14 = bitcast i1* %13 to i8*
  %15 = getelementptr inbounds i8, i8* %1, i64 280
  %16 = bitcast i8* %15 to %"class.MARC::ThreadSafeQueue"**
  %17 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %16, align 8
  %18 = alloca i1, align 1
  %19 = bitcast i1* %18 to i8*
  %20 = getelementptr inbounds i8, i8* %1, i64 360
  %21 = bitcast i8* %20 to %"class.MARC::ThreadSafeQueue"**
  %22 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %21, align 8
  %23 = alloca i1, align 1
  %24 = bitcast i1* %23 to i8*
  %25 = getelementptr inbounds i8, i8* %1, i64 432
  %26 = bitcast i8* %25 to %"class.MARC::ThreadSafeQueue"**
  %27 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %26, align 8
  %28 = alloca i1, align 1
  %29 = bitcast i1* %28 to i8*
  %30 = getelementptr inbounds i8, i8* %1, i64 512
  %31 = bitcast i8* %30 to %"class.MARC::ThreadSafeQueue"**
  %32 = load %"class.MARC::ThreadSafeQueue"*, %"class.MARC::ThreadSafeQueue"** %31, align 8
  %33 = alloca i1, align 1
  %34 = bitcast i1* %33 to i8*
  %35 = getelementptr inbounds i8, i8* %0, i64 16
  %36 = bitcast i8* %35 to float**
  %37 = load float*, float** %36, align 8
  %38 = load float, float* %37, align 4
  %39 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %7, i8* nonnull dereferenceable(1) %9)
  %40 = load i1, i1* %8, align 1
  %41 = bitcast i8* %0 to float**
  %42 = load float*, float** %41, align 8
  store float %38, float* %42, align 4
  br i1 %40, label %._crit_edge8, label %.lr.ph7.preheader

.lr.ph7.preheader:                                ; preds = %2
  br label %.lr.ph7

._crit_edge8:                                     ; preds = %.loopexit, %2
  %43 = getelementptr inbounds i8, i8* %0, i64 24
  %44 = bitcast i8* %43 to i32**
  %45 = load i32*, i32** %44, align 8
  store i32 0, i32* %45, align 4
  ret void

.lr.ph7:                                          ; preds = %.lr.ph7.preheader, %.loopexit
  %46 = phi float [ %64, %.loopexit ], [ %38, %.lr.ph7.preheader ]
  %47 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %12, i8* nonnull dereferenceable(1) %14)
  %48 = load i1, i1* %13, align 1
  br i1 %48, label %49, label %.loopexit1

; <label>:49:                                     ; preds = %.lr.ph7
  %50 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %17, i8* nonnull dereferenceable(1) %19)
  %51 = load i1, i1* %18, align 1
  br i1 %51, label %.loopexit1, label %.lr.ph4.preheader

.lr.ph4.preheader:                                ; preds = %49
  br label %.lr.ph4

.lr.ph4:                                          ; preds = %.lr.ph4.preheader, %._crit_edge
  %52 = phi float [ %.lcssa, %._crit_edge ], [ %46, %.lr.ph4.preheader ]
  %53 = fpext float %52 to double
  %54 = fadd double %53, 1.430000e-01
  %55 = fptrunc double %54 to float
  %56 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %22, i8* nonnull dereferenceable(1) %24)
  %57 = load i1, i1* %23, align 1
  br i1 %57, label %._crit_edge, label %.lr.ph.preheader

.lr.ph.preheader:                                 ; preds = %.lr.ph4
  br label %.lr.ph

.lr.ph:                                           ; preds = %.lr.ph.preheader, %.lr.ph
  %58 = phi float [ %59, %.lr.ph ], [ %55, %.lr.ph.preheader ]
  %59 = tail call float @_ZSt4sqrtf(float %58)
  %60 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %22, i8* nonnull dereferenceable(1) %24)
  %61 = load i1, i1* %23, align 1
  br i1 %61, label %._crit_edge, label %.lr.ph

._crit_edge:                                      ; preds = %.lr.ph, %.lr.ph4
  %.lcssa = phi float [ %55, %.lr.ph4 ], [ %59, %.lr.ph ]
  %62 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %17, i8* nonnull dereferenceable(1) %19)
  %63 = load i1, i1* %18, align 1
  br i1 %63, label %.loopexit1, label %.lr.ph4

.loopexit1:                                       ; preds = %._crit_edge, %49, %.lr.ph7
  %64 = phi float [ %46, %.lr.ph7 ], [ %46, %49 ], [ %.lcssa, %._crit_edge ]
  %65 = bitcast float %64 to i32
  call void @_ZN4MARC15ThreadSafeQueueIiE4pushEi(%"class.MARC::ThreadSafeQueue.11"* %4, i32 %65)
  %66 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %27, i8* nonnull dereferenceable(1) %29)
  %67 = load i1, i1* %28, align 1
  br i1 %67, label %.preheader, label %.loopexit

.preheader:                                       ; preds = %.loopexit1
  br label %68

; <label>:68:                                     ; preds = %.preheader, %68
  %69 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %32, i8* nonnull dereferenceable(1) %34)
  %70 = load i1, i1* %33, align 1
  br i1 %70, label %.loopexit, label %68

.loopexit:                                        ; preds = %68, %.loopexit1
  %71 = call zeroext i1 @_ZN4MARC15ThreadSafeQueueIaE7waitPopERa(%"class.MARC::ThreadSafeQueue"* %7, i8* nonnull dereferenceable(1) %9)
  %72 = load i1, i1* %8, align 1
  %73 = load float*, float** %41, align 8
  store float %64, float* %73, align 4
  br i1 %72, label %._crit_edge8, label %.lr.ph7
}

attributes #0 = { noinline uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noinline norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
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
!151 = !{!152, !4, i64 8}
!152 = !{!"_ZTSSt12_Vector_baseISt6threadSaIS0_EE", !153, i64 0}
!153 = !{!"_ZTSNSt12_Vector_baseISt6threadSaIS0_EE12_Vector_implE", !4, i64 0, !4, i64 8, !4, i64 16}
!154 = !{!152, !4, i64 16}
!155 = !{!156, !14, i64 0}
!156 = !{!"_ZTSNSt6thread2idE", !14, i64 0}
!157 = !{!158}
!158 = distinct !{!158, !159, !"_ZSt13__bind_simpleIMN4MARC10ThreadPoolEFvPSt6atomicIbEEJPS1_S4_EENSt19_Bind_simple_helperIT_JDpT0_EE6__typeEOS9_DpOSA_: argument 0"}
!159 = distinct !{!159, !"_ZSt13__bind_simpleIMN4MARC10ThreadPoolEFvPSt6atomicIbEEJPS1_S4_EENSt19_Bind_simple_helperIT_JDpT0_EE6__typeEOS9_DpOSA_"}
!160 = !{!161}
!161 = distinct !{!161, !162, !"_ZNSt6thread13_S_make_stateISt12_Bind_simpleIFSt7_Mem_fnIMN4MARC10ThreadPoolEFvPSt6atomicIbEEEPS4_S7_EEEESt10unique_ptrINS_6_StateESt14default_deleteISF_EEOT_: argument 0"}
!162 = distinct !{!162, !"_ZNSt6thread13_S_make_stateISt12_Bind_simpleIFSt7_Mem_fnIMN4MARC10ThreadPoolEFvPSt6atomicIbEEEPS4_S7_EEEESt10unique_ptrINS_6_StateESt14default_deleteISF_EEOT_"}
!163 = !{!164, !4, i64 0}
!164 = !{!"_ZTSSt10_Head_baseILm2EPSt6atomicIbELb0EE", !4, i64 0}
!165 = !{!166, !4, i64 0}
!166 = !{!"_ZTSSt10_Head_baseILm1EPN4MARC10ThreadPoolELb0EE", !4, i64 0}
!167 = !{!152, !4, i64 0}
!168 = !{i64 0, i64 8, !27, i64 8, i64 8, !27, i64 16, i64 8, !27, i64 24, i64 8, !27}
!169 = !{!170}
!170 = distinct !{!170, !171, !"_ZSt4bindIRPFvPvS0_EJRS0_S0_EENSt12_Bind_helperIXsr15__is_socketlikeIT_EE5valueES6_JDpT0_EE4typeEOS6_DpOS7_: argument 0"}
!171 = distinct !{!171, !"_ZSt4bindIRPFvPvS0_EJRS0_S0_EENSt12_Bind_helperIXsr15__is_socketlikeIT_EE5valueES6_JDpT0_EE4typeEOS6_DpOS7_"}
!172 = !{!173}
!173 = distinct !{!173, !174, !"_ZStL19__create_task_stateIFvvESt5_BindIFPFvPvS2_ES2_S2_EESaIiEESt10shared_ptrINSt13__future_base16_Task_state_baseIT_EEEOT0_RKT1_: argument 0"}
!174 = distinct !{!174, !"_ZStL19__create_task_stateIFvvESt5_BindIFPFvPvS2_ES2_S2_EESaIiEESt10shared_ptrINSt13__future_base16_Task_state_baseIT_EEEOT0_RKT1_"}
!175 = !{!176}
!176 = distinct !{!176, !177, !"_ZSt15allocate_sharedINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_JS7_RKS8_EESt10shared_ptrIT_ERKT0_DpOT1_: argument 0"}
!177 = distinct !{!177, !"_ZSt15allocate_sharedINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_JS7_RKS8_EESt10shared_ptrIT_ERKT0_DpOT1_"}
!178 = !{!179, !15, i64 8}
!179 = !{!"_ZTSSt16_Sp_counted_baseILN9__gnu_cxx12_Lock_policyE2EE", !15, i64 8, !15, i64 12}
!180 = !{!179, !15, i64 12}
!181 = !{!182, !4, i64 0}
!182 = !{!"_ZTSSt10_Head_baseILm0EPNSt13__future_base12_Result_baseELb0EE", !4, i64 0}
!183 = !{!184, !15, i64 0}
!184 = !{!"_ZTSSt13__atomic_baseIjE", !15, i64 0}
!185 = !{!186, !34, i64 0}
!186 = !{!"_ZTSSt18__atomic_flag_base", !34, i64 0}
!187 = !{!188, !15, i64 0}
!188 = !{!"_ZTSSt9once_flag", !15, i64 0}
!189 = !{!190, !176}
!190 = distinct !{!190, !191, !"_ZNSt13__future_base18_S_allocate_resultIviEESt10unique_ptrINS_7_ResultIT_EENS_12_Result_base8_DeleterEERKSaIT0_E: argument 0"}
!191 = distinct !{!191, !"_ZNSt13__future_base18_S_allocate_resultIviEESt10unique_ptrINS_7_ResultIT_EENS_12_Result_base8_DeleterEERKSaIT0_E"}
!192 = !{!190}
!193 = !{!194, !4, i64 0}
!194 = !{!"_ZTSSt5_BindIFPFvPvS0_ES0_S0_EE", !4, i64 0, !195, i64 8}
!195 = !{!"_ZTSSt5tupleIJPvS0_EE"}
!196 = !{!197, !4, i64 0}
!197 = !{!"_ZTSSt10_Head_baseILm1EPvLb0EE", !4, i64 0}
!198 = !{!199, !4, i64 0}
!199 = !{!"_ZTSSt10_Head_baseILm0EPvLb0EE", !4, i64 0}
!200 = !{!201, !4, i64 0}
!201 = !{!"_ZTSSt12__shared_ptrINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEELN9__gnu_cxx12_Lock_policyE2EE", !4, i64 0, !130, i64 8}
!202 = !{!203}
!203 = distinct !{!203, !204, !"_ZSt11make_uniqueIN4MARC10ThreadTaskISt13packaged_taskIFvvEEEEJS4_EENSt9_MakeUniqIT_E15__single_objectEDpOT0_: argument 0"}
!204 = distinct !{!204, !"_ZSt11make_uniqueIN4MARC10ThreadTaskISt13packaged_taskIFvvEEEEJS4_EENSt9_MakeUniqIT_E15__single_objectEDpOT0_"}
!205 = !{!206, !4, i64 48}
!206 = !{!"_ZTSSt11_Deque_baseISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EESaIS5_EE", !207, i64 0}
!207 = !{!"_ZTSNSt11_Deque_baseISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EESaIS5_EE11_Deque_implE", !4, i64 0, !14, i64 8, !208, i64 16, !208, i64 48}
!208 = !{!"_ZTSSt15_Deque_iteratorISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EERS5_PS5_E", !4, i64 0, !4, i64 8, !4, i64 16, !4, i64 24}
!209 = !{!206, !4, i64 64}
!210 = !{!211, !4, i64 0}
!211 = !{!"_ZTSSt10_Head_baseILm0EPN4MARC11IThreadTaskELb0EE", !4, i64 0}
!212 = !{!206, !14, i64 8}
!213 = !{!206, !4, i64 72}
!214 = !{!206, !4, i64 0}
!215 = !{!208, !4, i64 24}
!216 = !{!208, !4, i64 8}
!217 = !{!208, !4, i64 16}
!218 = !{!126, !4, i64 0}
!219 = distinct !{!219, !150}
!220 = !{!221, !4, i64 24}
!221 = !{!"_ZTSSt15_Deque_iteratorISt8functionIFvvEERS2_PS2_E", !4, i64 0, !4, i64 8, !4, i64 16, !4, i64 24}
!222 = !{!221, !4, i64 0}
!223 = !{!221, !4, i64 8}
!224 = !{!221, !4, i64 16}
!225 = !{!226, !4, i64 24}
!226 = !{!"_ZTSSt8functionIFvvEE", !4, i64 24}
!227 = !{!208, !4, i64 0}
!228 = !{!229}
!229 = distinct !{!229, !230, !"_ZNSt5dequeISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EESaIS5_EE5beginEv: argument 0"}
!230 = distinct !{!230, !"_ZNSt5dequeISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EESaIS5_EE5beginEv"}
!231 = !{!232}
!232 = distinct !{!232, !233, !"_ZNSt5dequeISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EESaIS5_EE3endEv: argument 0"}
!233 = distinct !{!233, !"_ZNSt5dequeISt10unique_ptrIN4MARC11IThreadTaskESt14default_deleteIS2_EESaIS5_EE3endEv"}
!234 = !{!206, !4, i64 40}
!235 = !{!236}
!236 = distinct !{!236, !237, !"_ZNSt5dequeISt8functionIFvvEESaIS2_EE5beginEv: argument 0"}
!237 = distinct !{!237, !"_ZNSt5dequeISt8functionIFvvEESaIS2_EE5beginEv"}
!238 = !{!239}
!239 = distinct !{!239, !240, !"_ZNSt5dequeISt8functionIFvvEESaIS2_EE3endEv: argument 0"}
!240 = distinct !{!240, !"_ZNSt5dequeISt8functionIFvvEESaIS2_EE3endEv"}
!241 = !{!242, !4, i64 0}
!242 = !{!"_ZTSSt11_Deque_baseISt8functionIFvvEESaIS2_EE", !243, i64 0}
!243 = !{!"_ZTSNSt11_Deque_baseISt8functionIFvvEESaIS2_EE11_Deque_implE", !4, i64 0, !14, i64 8, !221, i64 16, !221, i64 48}
!244 = !{!242, !4, i64 40}
!245 = !{!242, !4, i64 72}
!246 = !{!247}
!247 = distinct !{!247, !248, !"_ZNSt5dequeISt8functionIFvvEESaIS2_EE5beginEv: argument 0"}
!248 = distinct !{!248, !"_ZNSt5dequeISt8functionIFvvEESaIS2_EE5beginEv"}
!249 = !{i64 0, i64 8, !18, i64 0, i64 8, !18, i64 0, i64 8, !18, i64 0, i64 16, !18, i64 0, i64 16, !18}
!250 = !{!242, !4, i64 16}
!251 = !{!242, !4, i64 32}
!252 = !{!242, !4, i64 24}
!253 = !{!254, !4, i64 0}
!254 = !{!"_ZTSNSt15__exception_ptr13exception_ptrE", !4, i64 0}
!255 = !{!256, !4, i64 0}
!256 = !{!"_ZTSSt12__shared_ptrINSt13__future_base16_Task_state_baseIFvvEEELN9__gnu_cxx12_Lock_policyE2EE", !4, i64 0, !130, i64 8}
!257 = !{!258}
!258 = distinct !{!258, !259, !"_ZNKSt10error_code7messageEv: argument 0"}
!259 = distinct !{!259, !"_ZNKSt10error_code7messageEv"}
!260 = !{!261}
!261 = distinct !{!261, !262, !"_ZStplIcSt11char_traitsIcESaIcEESbIT_T0_T1_EPKS3_OS6_: argument 0"}
!262 = distinct !{!262, !"_ZStplIcSt11char_traitsIcESaIcEESbIT_T0_T1_EPKS3_OS6_"}
!263 = !{i64 0, i64 4, !17, i64 8, i64 8, !27}
!264 = !{!265}
!265 = distinct !{!265, !266, !"_ZSt13__bind_simpleISt17reference_wrapperISt5_BindIFPFvPvS2_ES2_S2_EEEJEENSt19_Bind_simple_helperIT_JDpT0_EE6__typeEOS9_DpOSA_: argument 0"}
!266 = distinct !{!266, !"_ZSt13__bind_simpleISt17reference_wrapperISt5_BindIFPFvPvS2_ES2_S2_EEEJEENSt19_Bind_simple_helperIT_JDpT0_EE6__typeEOS9_DpOSA_"}
!267 = !{!268, !4, i64 24}
!268 = !{!"_ZTSSt8functionIFSt10unique_ptrINSt13__future_base12_Result_baseENS2_8_DeleterEEvEE", !4, i64 24}
!269 = !{!34, !34, i64 0}
!270 = !{!271}
!271 = distinct !{!271, !272, !"_ZSt13__bind_simpleISt17reference_wrapperISt5_BindIFPFvPvS2_ES2_S2_EEEJEENSt19_Bind_simple_helperIT_JDpT0_EE6__typeEOS9_DpOSA_: argument 0"}
!272 = distinct !{!272, !"_ZSt13__bind_simpleISt17reference_wrapperISt5_BindIFPFvPvS2_ES2_S2_EEEJEENSt19_Bind_simple_helperIT_JDpT0_EE6__typeEOS9_DpOSA_"}
!273 = !{!274, !4, i64 0}
!274 = !{!"_ZTSSt12__weak_countILN9__gnu_cxx12_Lock_policyE2EE", !4, i64 0}
!275 = !{!276}
!276 = distinct !{!276, !277, !"_ZStL19__create_task_stateIFvvESt5_BindIFPFvPvS2_ES2_S2_EESaIiEESt10shared_ptrINSt13__future_base16_Task_state_baseIT_EEEOT0_RKT1_: argument 0"}
!277 = distinct !{!277, !"_ZStL19__create_task_stateIFvvESt5_BindIFPFvPvS2_ES2_S2_EESaIiEESt10shared_ptrINSt13__future_base16_Task_state_baseIT_EEEOT0_RKT1_"}
!278 = !{!279}
!279 = distinct !{!279, !280, !"_ZSt15allocate_sharedINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_JS7_RKS8_EESt10shared_ptrIT_ERKT0_DpOT1_: argument 0"}
!280 = distinct !{!280, !"_ZSt15allocate_sharedINSt13__future_base11_Task_stateISt5_BindIFPFvPvS3_ES3_S3_EESaIiEFvvEEES8_JS7_RKS8_EESt10shared_ptrIT_ERKT0_DpOT1_"}
!281 = !{!282}
!282 = distinct !{!282, !283, !"_ZNSt13__future_base18_S_allocate_resultIviEESt10unique_ptrINS_7_ResultIT_EENS_12_Result_base8_DeleterEERKSaIT0_E: argument 0"}
!283 = distinct !{!283, !"_ZNSt13__future_base18_S_allocate_resultIviEESt10unique_ptrINS_7_ResultIT_EENS_12_Result_base8_DeleterEERKSaIT0_E"}
!284 = !{i64 0, i64 8, !27, i64 8, i64 8, !27}
!285 = !{!286, !4, i64 0}
!286 = !{!"_ZTSSt10__weak_ptrINSt13__future_base13_State_baseV2ELN9__gnu_cxx12_Lock_policyE2EE", !4, i64 0, !274, i64 8}
!287 = !{!288}
!288 = distinct !{!288, !289, !"_ZNKSt8functionIFSt10unique_ptrINSt13__future_base12_Result_baseENS2_8_DeleterEEvEEclEv: argument 0"}
!289 = distinct !{!289, !"_ZNKSt8functionIFSt10unique_ptrINSt13__future_base12_Result_baseENS2_8_DeleterEEvEEclEv"}
!290 = !{!291, !4, i64 8}
!291 = !{!"_ZTSNSt13__future_base12_Task_setterISt10unique_ptrINS_7_ResultIvEENS_12_Result_base8_DeleterEESt12_Bind_simpleIFSt17reference_wrapperISt5_BindIFPFvPvSA_ESA_SA_EEEvEEvEE", !4, i64 0, !4, i64 8}
!292 = !{!293, !4, i64 0}
!293 = !{!"_ZTSSt17reference_wrapperISt5_BindIFPFvPvS1_ES1_S1_EEE", !4, i64 0}
!294 = !{!291, !4, i64 0}
!295 = !{!296, !4, i64 0}
!296 = !{!"_ZTSSt10_Head_baseILm0EPNSt13__future_base7_ResultIvEELb0EE", !4, i64 0}
!297 = !{!298, !4, i64 8}
!298 = !{!"_ZTSSt9type_info", !4, i64 8}
!299 = !{!242, !4, i64 48}
!300 = !{!242, !4, i64 64}
!301 = !{!242, !14, i64 8}
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
!321 = !{!206, !4, i64 16}
!322 = !{!206, !4, i64 32}
!323 = !{!206, !4, i64 24}
