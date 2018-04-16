PARALLELIZATION=DSWP
CC=clang++
AA6=-cfl-anders-aa
AA5=-scev-aa
AA4=-tbaa
AA3=-cfl-steens-aa
AA2=-globals-aa
AA1=-basicaa
AA=$(AA1) $(AA2) $(AA3) $(AA4) $(AA5) $(AA6)
ANALYSES=$(AA) -scalar-evolution -loops -loop-simplify -lcssa -domtree
THREADER=DSWP_utils
LIBS=-lm
INCLUDES=-I../include/threadpool/include
OPTIMIZED=parallelized

PASSES=-load ~/CAT/lib/PDGAnalysis.so -load ~/CAT/lib/$(PARALLELIZATION).so

all: $(OPTIMIZED) baseline

baseline: test.cpp
	$(CC) -std=c++14 -O3 $^ -o $@

$(THREADER).bc: $(THREADER).cpp
	$(CC) $(INCLUDES) -std=c++14 -emit-llvm -O3 -c $^ -o $@

$(OPTIMIZED): test_dswp.bc
	$(CC) -std=c++14 -pthreads -O0 $^ $(LIBS) -o $@

test_dswp.bc: test.bc
	opt $(PASSES) $(ANALYSES) -$(PARALLELIZATION) $^ -o $@
	llvm-dis $@

test.bc: test.cpp $(THREADER).bc
	$(CC) $(INCLUDES) -std=c++14 -emit-llvm -O0 -Xclang -disable-O0-optnone -c $< -o $@
	opt -mem2reg $@ -o $@
	llvm-dis $@
	llvm-link $@ $(THREADER).bc -o $@

show_passes: test.bc
	opt $(PASSES) $(ANALYSES) -$(PARALLELIZATION) $^ --debug-pass=Structure -disable-output

show_time: test.bc
	opt $(PASSES) $(ANALYSES) -$(PARALLELIZATION) $^ -time-passes -disable-output

clean:
	rm -f *.bc *.dot *.jpg *.ll *.S *.s *.o *.txt baseline $(OPTIMIZED)

.PHONY: show_passes show_time
