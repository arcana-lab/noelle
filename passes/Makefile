all: printer analysis parallelization_utilities parallelizer heuristics dswp doall

printer:
	cd $@ ; ../scripts/run_me.sh

parallelization_utilities:
	cd $@ ; ../scripts/run_me.sh

analysis:
	cd $@ ; ../scripts/run_me.sh

heuristics:
	cd $@ ; ../scripts/run_me.sh

parallelizer:
	cd $@ ; ../scripts/run_me.sh

dswp:
	cd $@ ; ../scripts/run_me.sh

doall:
	cd $@ ; ../scripts/run_me.sh

uninstall:
	rm -rf ~/CAT ;

clean:
	rm -rf */build */*.json ; find ./ -name .ycm_extra_conf.py -delete

.PHONY: clean analysis printer parallelizer download parallelization_utilities heuristics dswp doall
