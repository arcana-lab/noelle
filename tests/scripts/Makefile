# Commands
CC=clang
CPP=clang++
OPT=opt

# Libraries
LIBS=-lm

# Set the runtime flags
RUNTIME_CFLAGS="-DDEBUG"
#RUNTIME_CFLAGS="$(RUNTIME_CFLAGS) -DDSWP_STATS"
#RUNTIME_CFLAGS="$(RUNTIME_CFLAGS) -DRUNTIME_PRINT"

# Other parallelization options:
PARALLELIZATION_OPTIONS=-alloc-aa-verbose=1 -noelle-pdg-verbose=1 -noelle-verbose=2 -dswp-force -noelle-min-hot=0
OPT_LEVEL=-O3

INCLUDES=-I../../include/threadpool/include
THREADER=Parallelizer_utils
OPTIMIZED=parallelized
PRINT_OUTPUT_FILE=test.txt

# Fetch the list of files to compile
CPP_FILES := $(wildcard *.cpp)
BC_FILES := $(notdir $(CPP_FILES:.cpp=.bc))

all: baseline testseq $(OPTIMIZED)

%.bc: %.c
	$(CC) $(INCLUDES) -emit-llvm -O0 -Xclang -disable-O0-optnone -c $< -o $@
	noelle-norm $@ -o $@
	$(OPT) -dot-cfg -disable-output $@
	llvm-dis $@

%.bc: %.cpp
	$(CPP) $(INCLUDES) -std=c++14 -emit-llvm -O0 -Xclang -disable-O0-optnone -c $< -o $@
	noelle-norm $@ -o $@
	$(OPT) -dot-cfg -disable-output $@
	llvm-dis $@

baseline: baseline.bc
	$(CPP) $(OPT_LEVEL) $^ -o $@

testseq: test_seq.bc
	$(CPP) -std=c++14 -pthreads $(OPT_LEVEL) $^ $(LIBS) -o $@
	
$(THREADER).bc: $(THREADER).cpp
	$(CPP) $(RUNTIME_CFLAGS) $(INCLUDES) -std=c++14 -emit-llvm $(OPT_LEVEL) -c $^ -o $@

$(OPTIMIZED): test_dswp.bc
	$(CPP) -std=c++14 -pthreads $(OPT_LEVEL) $^ $(LIBS) -o $@

test_pdgdot.bc: test_seq.bc
	$(OPT) $(PRINTPASSES) $(ANALYSES) -heuristics -PDGPrinter $^ -o $@

test_dswp_unoptimized.bc: test_seq.bc output.prof
	noelle-pgo output.prof $(PARALLELIZATION_OPTIONS) $< -o $@
	llvm-dis $@

test_dswp.bc: test_dswp_unoptimized.bc
	$(CPP) $(OPT_LEVEL) -c -emit-llvm $^ -o $@
	llvm-dis $@

baseline.bc: test.bc
	$(CPP) $(OPT_LEVEL) -c -emit-llvm $^ -o $@
	llvm-dis $@

test_seq.bc: test.bc $(THREADER).bc
	llvm-link $^ -o $@
	noelle-pre "$@"
	llvm-dis $@

output.prof: default.profraw
	llvm-profdata merge $^ -output=$@

default.profraw: test_seq_prof input.txt
	./$< `cat input.txt` 

test_seq_prof: test_seq_prof.bc
	$(CPP) -pthreads -fprofile-instr-generate -lstdc++ $^ $(LIBS) -o $@

test_seq_prof.bc: test_seq.bc
	@../../scripts/inject_hotprofiler.sh $^ $@

input.txt:
	@../../scripts/create_input.sh $@

test_correctness: input.txt baseline $(OPTIMIZED)
	./baseline `cat input.txt` &> output_baseline.txt ;
	./parallelized `cat input.txt` &> output_parallelized.txt ;
	cmp output_baseline.txt output_parallelized.txt || (echo "Error $$?"; exit 1)
	echo "Success"

show_passes: test_seq.bc output.prof
	noelle-pgo output.prof $(PARALLELIZATION_OPTIONS) $^ --debug-pass=Structure -disable-output

show_time: test_seq.bc output.prof
	noelle-pgo output.prof $(PARALLELIZATION_OPTIONS) $^ -time-passes -disable-output

clean:
	rm -f *.bc *.dot *.jpg *.ll *.S *.s *.o *.txt baseline testseq $(OPTIMIZED) *.prof *.profraw *prof .*.dot

.PHONY: show_passes show_time
