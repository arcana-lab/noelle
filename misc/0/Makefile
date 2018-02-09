all: test

test: test.cpp
	clang++ -std=c++14 $^ -O0 -g -DDEBUG -o $@

leak: test
	valgrind --tool=memcheck --leak-check=full ./$^

clean:
	rm -f test *.o

.PHONY: leak clean
