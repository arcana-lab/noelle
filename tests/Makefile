all: regression performance

regression: download
	./scripts/test_regression.sh ;

performance: download
	./scripts/test_performance.sh ;

download:
	mkdir -p include ; cd include ; ../scripts/download.sh "/project/parallelizing_compiler/repositories/threadpool/" ;
	./scripts/add_symbolic_link.sh ;

clean:
	./scripts/clean.sh ; rm -rf include/
