all: printer analysis dswp

printer:
	cd $@ ; ./run_me.sh

analysis:
	cd $@ ; ./run_me.sh

dswp:
	cd $@ ; ./run_me.sh

download:
	../scripts/download.sh "/project/marc/repositories/threadpool"

uninstall:
	rm -rf ~/CAT ;

clean:
	rm -rf */build ;

.PHONY: clean analysis printer dswp download
