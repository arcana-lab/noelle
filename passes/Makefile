all: printer pdg parallelization_utilities dgsimplify parallelizer heuristics parallelization_technique dswp doall helix oracle_aa unique_ir_marker

oracle_aa:
	cd $@ ; ../scripts/run_me.sh

unique_ir_marker:
	cd $@ ; ../scripts/run_me.sh

printer:
	cd $@ ; ../scripts/run_me.sh

parallelization_utilities:
	cd $@ ; ../scripts/run_me.sh

pdg:
	cd $@ ; ../scripts/run_me.sh

dgsimplify:
	cd $@ ; ../scripts/run_me.sh

heuristics:
	cd $@ ; ../scripts/run_me.sh

parallelizer:
	cd $@ ; ../scripts/run_me.sh

parallelization_technique:
	cd $@ ; ../scripts/run_me.sh

dswp:
	cd $@ ; ../scripts/run_me.sh

doall:
	cd $@ ; ../scripts/run_me.sh

helix:
	cd $@ ; ../scripts/run_me.sh

uninstall:
	rm -rf ~/CAT ;

clean:
	rm -rf */build */*.json ; find ./ -name .ycm_extra_conf.py -delete

.PHONY: clean pdg printer dgsimplify parallelizer download parallelization_utilities heuristics parallelization_technique dswp doall helix oracle_aa unique_ir_marker
