CC=clang++
PASSES=-load ~/CAT/lib/PDGAnalysis.so -load ~/CAT/lib/DSWP.so
AA6=-cfl-anders-aa
AA5=-scev-aa
AA4=-tbaa
AA3=-cfl-steens-aa
AA2=-globals-aa
AA1=-basicaa
AA=$(AA1) $(AA2) $(AA3) $(AA4) $(AA5) $(AA6)
ANALYSES=$(AA) -scalar-evolution -loops -loop-simplify -lcssa -domtree
THREADER=threadpooler
LIBS=-lm

all: $(THREADER).bc test baseline

baseline: test.cpp
	$(CC) -std=c++14 -O3 $^ -o $@

$(THREADER).bc: $(THREADER).cpp
	$(CC) -std=c++14 -emit-llvm -O3 -c $^ -o $@

test: test_dswp.bc
	$(CC) -std=c++14 -pthreads -O0 $^ -o $@

test_dswp.bc: test.bc
	opt $(PASSES) $(ANALYSES) -DSWP $^ -o $@
	llvm-dis $@

test.bc: test.cpp
	$(CC) -std=c++14 -emit-llvm -O0 -Xclang -disable-O0-optnone -c $^ -o $@ $(LIBS)
	opt -mem2reg $@ -o $@
	llvm-dis $@
	llvm-link $@ $(THREADER).bc -o $@

show_passes: test.bc
	opt $(PASSES) $(ANALYSES) -DSWP $^ --debug-pass=Structure -disable-output

show_time: test.bc
	opt $(PASSES) $(ANALYSES) -DSWP $^ -time-passes -disable-output

clean:
	rm -f *.bc *.dot *.jpg *.ll *.S *.s *.o test baseline

.PHONY: show_passes show_time
