all: download
	./scripts/test.sh

download:
	mkdir -p include ; cd include ; ../scripts/download.sh "/project/marc/repositories/threadpool" ;
	./scripts/add_symbolic_link.sh ;

clean:
	./scripts/clean.sh ; rm -rf include/
