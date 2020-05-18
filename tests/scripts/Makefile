# Commands
CC=clang
CPP=clang++
OPT=opt

# Libraries
LIBS=-lm -lstdc++ -lpthread

# Set the runtime flags
RUNTIME_CFLAGS="-DDEBUG"
#RUNTIME_CFLAGS="$(RUNTIME_CFLAGS) -DDSWP_STATS"
#RUNTIME_CFLAGS="$(RUNTIME_CFLAGS) -DRUNTIME_PRINT"

# Other parallelization options:
PARALLELIZATION_OPTIONS=-alloc-aa-verbose=1 -noelle-pdg-verbose=1 -noelle-verbose=2 -dswp-force -noelle-min-hot=0
OPT_LEVEL=-O3

# Front-end flags
FRONTEND_FLAGS=-emit-llvm -O0 -Xclang -disable-O0-optnone
INCLUDES=-I../../include/threadpool/include

THREADER=Parallelizer_utils
OPTIMIZED=parallelized
PRINT_OUTPUT_FILE=test.txt

# Fetch the list of files to compile
CPP_FILES := $(wildcard *.cpp)
BC_FILES := $(notdir $(CPP_FILES:.cpp=.bc))

all: baseline testseq $(OPTIMIZED)

%.bc: %.c
	$(CC) $(INCLUDES) $(FRONTEND_FLAGS) -c $< -o $@
	noelle-norm $@ -o $@
	llvm-dis $@

%.bc: %.cpp
	$(CPP) $(INCLUDES) $(FRONTEND_FLAGS) -std=c++14 -c $< -o $@
	noelle-norm $@ -o $@
	llvm-dis $@

baseline: baseline.bc
	$(CPP) $(OPT_LEVEL) $^ -o $@

testseq: test_baseline.bc
	$(CPP) -std=c++14 -pthreads $(OPT_LEVEL) $^ $(LIBS) -o $@
	
$(THREADER).bc: $(THREADER).cpp
	$(CPP) $(RUNTIME_CFLAGS) $(INCLUDES) -std=c++14 -emit-llvm $(OPT_LEVEL) -c $^ -o $@

$(OPTIMIZED): test_parallelized.bc
	$(CPP) -std=c++14 -pthreads $(OPT_LEVEL) $^ $(LIBS) -o $@

test_parallelized_unoptimized.bc: baseline_with_metadata.bc
	noelle-parallelizer $^ -o $@ $(PARALLELIZATION_OPTIONS)
	llvm-dis $@

test_parallelized.bc: test_parallelized_unoptimized.bc
	$(CPP) $(OPT_LEVEL) -c -emit-llvm $^ -o $@
	llvm-dis $@

baseline.bc: test.bc
	$(CPP) $(OPT_LEVEL) -c -emit-llvm $^ -o $@
	llvm-dis $@

baseline_with_metadata.bc: output.prof test_baseline.bc
	noelle-meta-prof-embed $^ -o $@
	noelle-meta-pdg-embed $@ -o $@
	llvm-dis $@

test_baseline.bc: test.bc $(THREADER).bc
	llvm-link $^ -o $@
	noelle-pre "$@"
	llvm-dis $@

test_baseline_enable.bc: test_baseline.bc output.prof
	noelle-enable output.prof $(PARALLELIZATION_OPTIONS) $< -o $@

output.prof: default.profraw
	llvm-profdata merge $^ -output=$@

default.profraw: test_baseline_prof input.txt
	./$< `cat input.txt` 

test_baseline_prof: test_baseline_prof.bc
	$(CPP) -fprofile-instr-generate $^ $(LIBS) -o $@

test_baseline_prof.bc: test_baseline.bc
	@../../scripts/inject_hotprofiler.sh $^ $@

input.txt:
	@../../scripts/create_input.sh $@

test_correctness: input.txt baseline $(OPTIMIZED)
	./baseline `cat input.txt` &> output_baseline.txt ;
	./parallelized `cat input.txt` &> output_parallelized.txt ;
	cmp output_baseline.txt output_parallelized.txt || (echo "Error $$?"; exit 1)
	echo "Success"

show_passes: test_baseline.bc output.prof
	noelle-pgo output.prof $(PARALLELIZATION_OPTIONS) $^ --debug-pass=Structure -disable-output

show_time: test_baseline.bc output.prof
	noelle-pgo output.prof $(PARALLELIZATION_OPTIONS) $^ -time-passes -disable-output

pdg: test_baseline.bc
	noelle-pdg $^

meta: test_baseline.bc
	noelle-meta-embed $^ -o $^

meta_clean: test_baseline.bc
	noelle-meta-clean $^ -o $^
	llvm-dis $^

clean:
	rm -f *.bc *.dot *.jpg *.ll *.S *.s *.o *.txt baseline testseq $(OPTIMIZED) *.prof *.profraw *prof .*.dot

.PHONY: test_correctness show_passes show_time pdg meta_clean meta clean
