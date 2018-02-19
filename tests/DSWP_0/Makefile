PASSES=-load ~/CAT/lib/PDGAnalysis.so -load ~/CAT/lib/DSWP.so
ANALYSES=-basicaa

all: test

test: test_dswp.bc
	clang -O0 $^ -o $@

test_dswp.bc: test.bc
	opt $(PASSES) $(ANALYSIS) -DSWP $^ -o $@
	llvm-dis $@

test.bc: test.c
	clang -emit-llvm -O0 -c $^ -o $@
	opt -mem2reg $@ -o $@
	llvm-dis $@

clean:
	rm -f *.bc *.dot *.jpg *.ll *.S *.s *.o
