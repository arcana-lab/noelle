LIBS=

all: test_opt

%.bc: %.cpp
	clang++ -O0 -Xclang -disable-O0-optnone -emit-llvm -c $< -o $@
	llvm-dis $@

%.bc: %.c
	clang -O0 -Xclang -disable-O0-optnone -emit-llvm -c $< -o $@
	llvm-dis $@

test_pre.bc: test.bc
	noelle-norm $^ -o $@
	llvm-dis $@

test_pre_prof: test_pre.bc
	noelle-prof-coverage $^ $@ $(LIBS)

default.profraw: test_pre_prof
	./$< 100

output.prof: default.profraw
	llvm-profdata merge $^ -output=$@

test_with_metadata.bc: output.prof test_pre.bc
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
