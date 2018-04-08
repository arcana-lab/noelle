all: download
	./scripts/test.sh

download:
	mkdir -p include ; cd include ; ../scripts/download.sh "/project/marc/repositories/threadpool"

clean:
	./scripts/clean.sh ; rm -rf include/
