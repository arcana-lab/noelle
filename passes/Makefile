all: hotprofiler printer pdg parallelization_utilities talkdown alloc_aa dataflow dgsimplify parallelizer heuristics parallelization_technique dswp doall helix

dataflow:
	cd $@ ; ../scripts/run_me.sh

hotprofiler:
	cd $@ ; ../scripts/run_me.sh

printer:
	cd $@ ; ../scripts/run_me.sh

parallelization_utilities:
	cd $@ ; ../scripts/run_me.sh

pdg:
	cd $@ ; ../scripts/run_me.sh

alloc_aa:
	cd $@ ; ../scripts/run_me.sh

talkdown:
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

.PHONY: clean pdg printer talkdown alloc_aa dgsimplify parallelizer download parallelization_utilities heuristics parallelization_technique dswp doall helix hotprofiler dataflow
