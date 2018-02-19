all: printer analysis user dswp

printer:
	cd $@ ; ./run_me.sh

analysis:
	cd $@ ; ./run_me.sh

user:
	cd $@ ; ./run_me.sh

dswp:
	cd $@ ; ./run_me.sh

download:
	./scripts/download.sh "/project/marc/repositories/threadpool"

uninstall:
	rm -rf ~/CAT ;

clean:
	./scripts/clean.sh ;

.PHONY: clean analysis printer user dswp download
