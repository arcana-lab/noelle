all: printer analysis dswp parallelization

printer:
	cd $@ ; ../scripts/run_me.sh

parallelization:
	cd $@ ; ../scripts/run_me.sh

analysis:
	cd $@ ; ../scripts/run_me.sh

dswp:
	cd $@ ; ../scripts/run_me.sh

uninstall:
	rm -rf ~/CAT ;

clean:
	rm -rf */build */*.json ; find ./ -name .ycm_extra_conf.py -delete

.PHONY: clean analysis printer dswp download parallelization
