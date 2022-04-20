# Commands
CC=clang
CPP=clang++
OPT=opt

# Libraries
LIBS=-lm -lstdc++ -lpthread

# Set the runtime flags
RUNTIME_CFLAGS="-DDEBUG"

# Parallelization options
# 	- To dump the PDG: -noelle-pdg-dump
PARALLELIZATION_OPTIONS=-alloc-aa-verbose=1 -noelle-parallelizer-force
NOELLE_OPTIONS=-noelle-pdg-verbose=1 -noelle-verbose=2 -noelle-min-hot=0
TOOLS_OPTIONS=
OPT_LEVEL=-O3

# Front-end
INCLUDES=-I../../include/threadpool/include
FRONTEND_OPTIONS=-O1 -Xclang -disable-llvm-passes
PRE_MIDDLEEND_OPTIONS=-O0
FRONTEND_FLAGS=-emit-llvm $(FRONTEND_OPTIONS)

THREADER=Parallelizer_utils
OPTIMIZED=parallelized
PRINT_OUTPUT_FILE=test.txt

# Fetch the list of files to compile
CPP_FILES := $(wildcard *.cpp)
BC_FILES := $(notdir $(CPP_FILES:.cpp=.bc))

all: baseline testseq $(OPTIMIZED)

%.bc: %.c
	$(CC) $(INCLUDES) $(FRONTEND_FLAGS) -c $< -o $@
	opt $(PRE_MIDDLEEND_OPTIONS) $@ -o $@

%.bc: %.cpp
	$(CPP) $(INCLUDES) $(FRONTEND_FLAGS) -c $< -o $@
	opt $(PRE_MIDDLEEND_OPTIONS) $@ -o $@

%.ll: %.bc
	llvm-dis $^

baseline: baseline.bc
	$(CPP) $(OPT_LEVEL) $^ -o $@

testseq: baseline_pre.bc
	$(CPP) -std=c++14 -pthreads $(OPT_LEVEL) $^ $(LIBS) -o $@
	
$(THREADER).bc: $(THREADER).cpp
	$(CPP) $(RUNTIME_CFLAGS) $(INCLUDES) -std=c++14 -emit-llvm $(OPT_LEVEL) -c $^ -o $@

$(OPTIMIZED): test_parallelized.bc
	$(CPP) -std=c++14 -pthreads $(OPT_LEVEL) $^ $(LIBS) -o $@

test_parallelized_unoptimized.bc: baseline_with_parallel_plan.bc
	noelle-parallelizer $^ -o $@ $(NOELLE_OPTIONS) $(PARALLELIZATION_OPTIONS)
	noelle-meta-clean $@ $@

test_parallelized.bc: test_parallelized_unoptimized.bc
	$(CPP) $(OPT_LEVEL) -c -emit-llvm $^ -o $@

baseline.bc: test.bc
	$(CPP) $(OPT_LEVEL) -c -emit-llvm $^ -o $@

baseline_with_parallel_plan.bc: baseline_with_metadata.bc
	noelle-parallelization-planner $^ -o $@ $(NOELLE_OPTIONS) $(PARALLELIZATION_OPTIONS)

baseline_with_metadata.bc: default.profraw baseline_pre.bc
	noelle-meta-prof-embed $^ -o $@
	noelle-meta-pdg-embed $@ -o $@

baseline_with_runtime.bc: test.bc $(THREADER).bc
	llvm-link $^ -o $@

baseline_with_runtime_prof: baseline_with_runtime.bc
	noelle-prof-coverage $^ $@ $(LIBS)

pre_profiles.profraw: baseline_with_runtime_prof input.txt
	./$< `cat input.txt` 
	mv default.profraw $@

baseline_pre.bc: pre_profiles.profraw baseline_with_runtime.bc
	noelle-meta-prof-embed $^ -o $@
	noelle-pre $@ $(NOELLE_OPTIONS) $(TOOLS_OPTIONS)
	noelle-meta-clean $@ $@

default.profraw: baseline_pre_prof input.txt
	./$< `cat input.txt` 

baseline_pre_prof: baseline_pre.bc
	noelle-prof-coverage $^ $@ $(LIBS)

input.txt:
	@../../scripts/create_input.sh $@

test_correctness: input.txt baseline $(OPTIMIZED)
	./baseline `cat input.txt` &> output_baseline.txt ;
	./parallelized `cat input.txt` &> output_parallelized.txt ;
	cmp output_baseline.txt output_parallelized.txt || (echo "Error $$?"; exit 1)
	./parallelized `cat input.txt` &> output_parallelized.txt ;
	cmp output_baseline.txt output_parallelized.txt || (echo "Error $$?"; exit 1)
	./parallelized `cat input.txt` &> output_parallelized.txt ;
	cmp output_baseline.txt output_parallelized.txt || (echo "Error $$?"; exit 1)
	./parallelized `cat input.txt` &> output_parallelized.txt ;
	cmp output_baseline.txt output_parallelized.txt || (echo "Error $$?"; exit 1)
	echo "Success"

clean:
	rm -f *.bc *.dot *.jpg *.ll *.S *.s *.o baseline testseq $(OPTIMIZED) *.prof *.profraw *prof .*.dot
	rm -f time_parallelized.txt compiler_output.txt input.txt ;
	rm -f output*.txt ;
	rm -f OUT ;

.PHONY: test_correctness clean
