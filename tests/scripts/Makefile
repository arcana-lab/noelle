CC=clang
CPP=clang++
AA_BASE=-basicaa
AA_ADVANCED=$(AA_BASE) -globals-aa -cfl-steens-aa -tbaa -scev-aa -cfl-anders-aa
ANALYSES=$(AA_ADVANCED) -scalar-evolution -loops -loop-simplify -lcssa -domtree -parallelization 
LIBS=-lm

# Set the runtime flags
RUNTIME_CFLAGS="-DDEBUG"
#RUNTIME_CFLAGS="$(RUNTIME_CFLAGS) -DDSWP_STATS"
#RUNTIME_CFLAGS="$(RUNTIME_CFLAGS) -DRUNTIME_PRINT"

# Other parallelization options:
PARALLELIZATION_OPTIONS=-alloc-aa-verbose=1 -pdg-verbose=1 -noelle-verbose=2 -dswp-force 
OPT_LEVEL=-O3

INCLUDES=-I../../include/threadpool/include
THREADER=Parallelizer_utils
OPTIMIZED=parallelized

TRANSFORMATIONS_BEFORE_PARALLELIZATION=$(AA_BASE) -mem2reg -simplifycfg -instcombine -tailcallelim -loop-simplify -lcssa -loop-unswitch -instcombine -loop-simplify -lcssa -indvars -loop-deletion -instcombine -indvars
PDGPASS=-load ~/CAT/lib/AllocAA.so -load ~/CAT/lib/TalkDown.so -load ~/CAT/lib/PDGAnalysis.so
PRINTPASSES=$(PDGPASS) -load ~/CAT/lib/PDGPrinter.so
PARALLELIZATION_TECHNIQUES= -load ~/CAT/lib/DSWP.so -load ~/CAT/lib/DOALL.so -load ~/CAT/lib/HELIX.so
PRE_OPTPASSES=$(PDGPASS) -load ~/CAT/lib/HotProfiler.so -load ~/CAT/lib/Parallelization.so -load ~/CAT/lib/DGSimplify.so
OPTPASSES=$(PDGPASS) -load ~/CAT/lib/DataFlow.so -load ~/CAT/lib/HotProfiler.so -load ~/CAT/lib/Parallelization.so -load ~/CAT/lib/Heuristics.so -load ~/CAT/lib/ParallelizationTechnique.so $(PARALLELIZATION_TECHNIQUES) -load ~/CAT/lib/Parallelizer.so 
PRINT_OUTPUT_FILE=test.txt

# Fetch the list of files to compile
CPP_FILES := $(wildcard *.cpp)
BC_FILES := $(notdir $(CPP_FILES:.cpp=.bc))


all: baseline testseq $(OPTIMIZED)

%.bc: %.c
	$(CC) $(INCLUDES) -emit-llvm -O0 -Xclang -disable-O0-optnone -c $< -o $@
	opt $(TRANSFORMATIONS_BEFORE_PARALLELIZATION) $@ -o $@
	opt -dot-cfg -disable-output $@
	llvm-dis $@

%.bc: %.cpp
	$(CPP) $(INCLUDES) -std=c++14 -emit-llvm -O0 -Xclang -disable-O0-optnone -c $< -o $@
	opt $(TRANSFORMATIONS_BEFORE_PARALLELIZATION) $@ -o $@
	opt -dot-cfg -disable-output $@
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
	opt $(PRINTPASSES) $(ANALYSES) -heuristics -PDGPrinter $^ -o $@

test_dswp_unoptimized.bc: test_seq.bc
	opt $(OPTPASSES) $(ANALYSES) -heuristics -parallelizer $(PARALLELIZATION_OPTIONS) $^ -o $@
	llvm-dis $@

test_dswp.bc: test_dswp_unoptimized.bc
	$(CPP) $(OPT_LEVEL) -c -emit-llvm $^ -o $@
	llvm-dis $@

baseline.bc: test.bc
	$(CPP) $(OPT_LEVEL) -c -emit-llvm $^ -o $@
	llvm-dis $@

test_seq.bc: test.bc $(THREADER).bc
	llvm-link $^ -o $@
	@../../scripts/iterate_dgsimplify.sh "-inliner-verbose=1" "$(PRE_OPTPASSES)" "$@"
	opt $(TRANSFORMATIONS_BEFORE_PARALLELIZATION) $@ -o $@
	llvm-dis $@

show_passes: test_seq.bc
	opt $(OPTPASSES) $(ANALYSES) -heuristics -Parallelizer $(PARALLELIZATION_OPTIONS) $^ --debug-pass=Structure -disable-output

show_time: test_seq.bc
	opt $(OPTPASSES) $(ANALYSES) -heuristics -Parallelizer $(PARALLELIZATION_OPTIONS) $^ -time-passes -disable-output

print_transformations_before_parallelization:
	../../scripts/print.sh "$(PRINT_OUTPUT_FILE)" "$(TRANSFORMATIONS_BEFORE_PARALLELIZATION)" 

print_alias_analyses:
	../../scripts/print.sh "$(PRINT_OUTPUT_FILE)" "$(AA_ADVANCED)" 

print_analyses:
	../../scripts/print.sh "$(PRINT_OUTPUT_FILE)" "$(ANALYSES)" 

clean:
	rm -f *.bc *.dot *.jpg *.ll *.S *.s *.o *.txt baseline testseq $(OPTIMIZED) *.prof *.profraw

.PHONY: show_passes show_time
