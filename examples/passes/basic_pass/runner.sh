cd ../../../alex/basic;


for filename in *.c; do

    # compile program with malloc
    clang -o "$filename" "$filename" -pthread;


done

cd ../../examples/passes/basic_pass/run;

# compile all programs to bitcode
# for filename in *.c; do
#     clang -emit-llvm -o "$filename".bc -c ../../../../alex/basic/"$filename" -pthread;
# done

# noelle-norm malloc_test.bc -o norm-ir.bc;
# noelle-load -load /home/akg434/CAT/lib/CAT.so -CAT norm-ir.bc -o jemalloc.bc;
# llc -filetype=obj jemalloc.bc -o jemalloc.o;
# clang jemalloc.o -o jemalloc -I`jemalloc-config --includedir` \
# -L`jemalloc-config --libdir` -Wl,-rpath,`jemalloc-config --libdir` \
# -ljemalloc `jemalloc-config --libs`;

# perf stat ./jemalloc;