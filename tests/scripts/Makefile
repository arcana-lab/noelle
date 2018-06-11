PARALLELIZATION=DSWP
PARALLELIZATION_OPTIONS=-dswp-force
PDGPRINTER=PDGPrinter
CC=clang++
AA6=-cfl-anders-aa
AA5=-scev-aa
AA4=-tbaa
AA3=-cfl-steens-aa
AA2=-globals-aa
AA1=-basicaa
AA=$(AA1) $(AA2) $(AA3) $(AA4) $(AA5) $(AA6)
ANALYSES=$(AA) -scalar-evolution -loops -loop-simplify -lcssa -domtree -parallelization
THREADER=$(PARALLELIZATION)_utils
LIBS=-lm
INCLUDES=-I../include/threadpool/include
OPTIMIZED=parallelized
TRANSFORMATIONS_BEFORE_PARALLELIZATION=-basicaa -mem2reg -simplifycfg -instcombine -tailcallelim -loop-simplify -lcssa -loop-unswitch -instcombine -loop-simplify -lcssa -indvars -loop-deletion -instcombine -indvars
OPT_LEVEL=-O3
RUNTIME_CFLAGS="-DDSWP_STATS"
RUNTIME_CFLAGS=""

PRINTPASSES=-load ~/CAT/lib/PDGAnalysis.so -load ~/CAT/lib/$(PDGPRINTER).so
OPTPASSES=-load ~/CAT/lib/PDGAnalysis.so -load ~/CAT/lib/Parallelization.so -load ~/CAT/lib/$(PARALLELIZATION).so

all: baseline $(OPTIMIZED)

baseline: baseline.bc
	$(CC) $(OPT_LEVEL) $^ -o $@

$(THREADER).bc: $(THREADER).cpp
	$(CC) $(RUNTIME_CFLAGS) $(INCLUDES) -std=c++14 -emit-llvm $(OPT_LEVEL) -c $^ -o $@

$(OPTIMIZED): test_dswp.bc
	$(CC) -std=c++14 -pthreads $(OPT_LEVEL) $^ $(LIBS) -o $@

test_pdgdot.bc: test_seq.bc
	opt $(PRINTPASSES) $(ANALYSES) -$(PDGPRINTER) $^ -o $@

test_dswp.bc: test_seq.bc
	opt $(OPTPASSES) $(ANALYSES) -$(PARALLELIZATION) $(PARALLELIZATION_OPTIONS) $^ -o $@
	$(CC) $(OPT_LEVEL) -c -emit-llvm $@ -o $@
	llvm-dis $@

baseline.bc: test.bc
	$(CC) $(OPT_LEVEL) -c -emit-llvm $^ -o $@
	llvm-dis $@

test.bc: test.cpp
	$(CC) $(INCLUDES) -std=c++14 -emit-llvm -O0 -Xclang -disable-O0-optnone -c $< -o $@
	opt $(TRANSFORMATIONS_BEFORE_PARALLELIZATION) $@ -o $@
	opt -dot-cfg -disable-output $@
	llvm-dis $@

test_seq.bc: test.bc $(THREADER).bc
	llvm-link $^ -o $@
	llvm-dis $@

show_passes: test_seq.bc
	opt $(OPTPASSES) $(ANALYSES) -$(PARALLELIZATION) $(PARALLELIZATION_OPTIONS) $^ --debug-pass=Structure -disable-output

show_time: test_seq.bc
	opt $(OPTPASSES) $(ANALYSES) -$(PARALLELIZATION) $(PARALLELIZATION_OPTIONS) $^ -time-passes -disable-output

clean:
	rm -f *.bc *.dot *.jpg *.ll *.S *.s *.o *.txt baseline $(OPTIMIZED)

.PHONY: show_passes show_time
