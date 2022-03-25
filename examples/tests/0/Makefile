LIBS=

all: test_opt

%.bc: %.cpp
	clang++ -O1 -Xclang -disable-llvm-passes -emit-llvm -c $< -o $@
	llvm-dis $@

%.bc: %.c
	clang -O1 -Xclang -disable-llvm-passes -emit-llvm -c $< -o $@
	llvm-dis $@

test_norm.bc: test.bc
	noelle-norm $^ -o $@
	llvm-dis $@

test_pre_prof: test_norm.bc
	noelle-prof-coverage $^ $@ $(LIBS)

default.profraw: test_pre_prof
	./$< 100

output.prof: default.profraw
	llvm-profdata merge $^ -output=$@

test_with_metadata.bc: output.prof test_norm.bc
	noelle-meta-prof-embed $^ -o $@
	noelle-meta-pdg-embed $@ -o $@
	llvm-dis $@

test_opt.bc: test_with_metadata.bc
	noelle-load -load ~/CAT/lib/CAT.so -CAT $< -o $@
	llvm-dis $@

test_opt: test_opt.bc
	clang $< -O3 -march=native -o $@

clean:
	rm -f *.bc *.ll test_opt test_pre_prof output.prof default.* ;
