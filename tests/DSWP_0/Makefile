PASSES=-load ~/CAT/lib/PDGAnalysis.so -load ~/CAT/lib/DSWP.so
AA6=-cfl-anders-aa
AA5=-scev-aa
AA4=-tbaa
AA3=-cfl-steens-aa
AA2=-globals-aa
AA1=-basicaa
AA=$(AA1) $(AA2) $(AA3) $(AA4) $(AA5) $(AA6)
ANALYSES=$(AA) -scalar-evolution -loops -loop-simplify -lcssa -domtree

all: test

test: test_dswp.bc
	clang -O0 $^ -o $@

test_dswp.bc: test.bc
	opt $(PASSES) $(ANALYSES) -DSWP $^ -o $@
	llvm-dis $@

test.bc: test.c
	clang -emit-llvm -O0 -c $^ -o $@
	opt -mem2reg $@ -o $@
	llvm-dis $@

show_passes: test.bc
	opt $(PASSES) $(ANALYSES) -DSWP $^ --debug-pass=Structure -disable-output

show_time: test.bc
	opt $(PASSES) $(ANALYSES) -DSWP $^ -time-passes -disable-output

clean:
	rm -f *.bc *.dot *.jpg *.ll *.S *.s *.o test

.PHONY: show_passes show_time
