

cd basic;

clang malloc_test.c -o malloc_test -I`jemalloc-config --includedir` \
-L`jemalloc-config --libdir` -Wl,-rpath,`jemalloc-config --libdir` \
-ljemalloc `jemalloc-config --libs`;

clang dlmalloc_test.c -L/home/akg434/noelle/alex/dlmalloc/lib/ -lmalloc -o dlmalloc_test;

clang jemalloc_test.c -o jemalloc_test -I`jemalloc-config --includedir` \
-L`jemalloc-config --libdir` -Wl,-rpath,`jemalloc-config --libdir` \
-ljemalloc `jemalloc-config --libs`;

perf stat ./malloc_test;
perf stat ./dlmalloc_test;
perf stat ./jemalloc_test;

cd ..;