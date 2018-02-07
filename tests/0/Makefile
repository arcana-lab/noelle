all: pdg.dot

pdg.dot: test.bc
	opt -load ~/CAT/lib/PDGAnalysis.so -load ~/CAT/lib/PDGPrinter.so -basicaa -disable-output -PDGPrinter test.bc

test.bc: test.c
	clang -emit-llvm -O0 -c $^ -o $@
	llvm-dis $@

pdg.jpg: pdg.dot
	dot2jpg $^

clean:
	rm -f *.bc *.dot *.jpg *.ll *.S *.s *.o
