# Execution
NUM_RUNS=30

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
NOELLE_OPTIONS=-noelle-pdg-verbose=1 -noelle-verbose=2 -noelle-min-hot=0 -noelle-inliner-avoid-hoist-to-main
TOOLS_OPTIONS=
OPT_LEVEL=-O3

# Front-end
INCLUDES=-I../../include/threadpool/include
FRONTEND_OPTIONS=-O1 -Xclang -disable-llvm-passes
PRE_MIDDLEEND_OPTIONS=-O0
FRONTEND_FLAGS=-emit-llvm $(FRONTEND_OPTIONS)

OPTIMIZED=parallelized
OPTIMIZED_AUTOTUNED=parallelized_autotuned
PRINT_OUTPUT_FILE=test.txt

# Autotuner
# Input to run the autotuner
INPUT=input.txt
# Time limit for the autotuner to run (in seconds)
TIME=100
# Max number of configurations the autotuner explores
NUM_CONF=500
# Seed configuration the autotuner will start with
SEED_CONF=
# Disable generation of search space (implies the user will provide autotuner_space.info)
NOSEARCHSPACE=
# Disable filtering of search space that forces DOALL on loops that we know are DOALL at compile time
NODOALLFILTERING=
# Disable filtering of search space that checks if parallelizing one DOALL loop at a time yields a better execution time
NODOALLTIMEFILTERING=

all: baseline $(OPTIMIZED)

%.bc: %.c
	$(CC) $(INCLUDES) $(FRONTEND_FLAGS) -c $< -o $@
	opt $(PRE_MIDDLEEND_OPTIONS) $@ -o $@

%.bc: %.cpp
	$(CPP) $(INCLUDES) $(FRONTEND_FLAGS) -c $< -o $@
	opt $(PRE_MIDDLEEND_OPTIONS) $@ -o $@

baseline: baseline.bc
	$(CPP) $(OPT_LEVEL) $^ -o $@

Parallelizer_utils.bc: Parallelizer_utils.cpp
	$(CPP) $(RUNTIME_CFLAGS) $(INCLUDES) -std=c++14 -emit-llvm $(OPT_LEVEL) -c $^ -o $@

$(OPTIMIZED): test_parallelized.bc
	$(CPP) -std=c++14 -pthreads $(OPT_LEVEL) $< $(LIBS) -o $@

$(OPTIMIZED_AUTOTUNED): test_parallelized_autotuned.bc
	$(CPP) -std=c++14 -pthreads $(OPT_LEVEL) $< $(LIBS) -o $@

test_parallelized.bc: baseline_with_metadata.bc Parallelizer_utils.bc
	noelle-parallelizer $< -o $@ $(NOELLE_OPTIONS) $(PARALLELIZATION_OPTIONS)

test_parallelized_autotuned.bc: baseline_with_metadata.bc input.txt Parallelizer_utils.bc
	noelle-parallelizer-autotuner --parallelizer-args="$< -o $@ $(NOELLE_OPTIONS) $(PARALLELIZATION_OPTIONS)" --libs="$(LIBS)" --input="$(INPUT)" --autotuner-time="$(TIME)" --autotuner-numconf="$(NUM_CONF)" --autotuner-seedconf="$(SEED_CONF)" $(NOSEARCHSPACE) $(NODOALLFILTERING) $(NODOALLTIMEFILTERING)

baseline.bc: test.bc
	$(CPP) $(OPT_LEVEL) -c -emit-llvm $^ -o $@

baseline_with_metadata.bc: default.profraw baseline_pre.bc
	noelle-meta-prof-embed $^ -o $@
	noelle-meta-pdg-embed $(NOELLE_OPTIONS) $@ -o $@

baseline_with_runtime_prof: test.bc
	noelle-prof-coverage $^ $@ $(LIBS)

pre_profiles.profraw: baseline_with_runtime_prof input.txt
	./$< `cat input.txt` 
	mv default.profraw $@

baseline_pre.bc: pre_profiles.profraw test.bc
	noelle-meta-prof-embed $^ -o $@
	noelle-pre $@ $(NOELLE_OPTIONS) $(TOOLS_OPTIONS)

default.profraw: baseline_pre_prof input.txt
	./$< `cat input.txt` 

baseline_pre_prof: baseline_pre.bc
	noelle-prof-coverage $^ $@ $(LIBS)

input.txt:
	@../../scripts/create_input.sh $@

test_correctness: input.txt baseline $(OPTIMIZED)
	@../../scripts/test_correctness.sh $(NUM_RUNS)

clean:
	rm -f *.bc *.dot *.jpg *.ll *.S *.s *.o baseline $(OPTIMIZED) $(OPTIMIZED_AUTOTUNED) *.prof *.profraw *prof .*.dot
	rm -f time_parallelized.txt compiler_output.txt input.txt ;
	rm -f output*.txt ;
	rm -f OUT ;
	rm -rf binaryToRun baselineTime.txt executionTime.txt exploredConfs.json opentuner.db opentuner.log ;

.PHONY: test_correctness clean
