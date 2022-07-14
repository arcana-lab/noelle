rm -f benchmark* default.profraw
clang++ -c -emit-llvm -O0 -g3 -o benchmark.bc test.cpp
noelle-norm benchmark.bc -o benchmark.norm.bc
noelle-prof-coverage benchmark.norm.bc benchmark.exe -lstdc++
./benchmark.exe 1
noelle-meta-prof-embed default.profraw benchmark.norm.bc -o benchmark.embedded.bc
llvm-dis benchmark.embedded.bc
noelle-parallel-load -load ~/CAT/lib/CAT.so -CAT benchmark.embedded.bc --disable-output
