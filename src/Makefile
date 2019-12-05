UTILS=basic_utilities parallelization_utilities architecture loop_metadata
ANALYSIS=pdg talkdown alloc_aa dataflow loops
PARALLELIZATION=parallelizer heuristics parallelization_technique dswp doall helix
ALL=$(UTILS) $(ANALYSIS) loop_distribution $(PARALLELIZATION) hotprofiler dgsimplify unique_ir_marker noelle

all: $(ALL)

noelle:
	./scripts/installNOELLE.sh

unique_ir_marker:
	cd $@ ; ../scripts/run_me.sh

dataflow:
	cd $@ ; ../scripts/run_me.sh

hotprofiler:
	cd $@ ; ../scripts/run_me.sh

basic_utilities:
	cd $@ ; ../scripts/run_me.sh

loops:
	cd $@ ; ../scripts/run_me.sh

parallelization_utilities:
	cd $@ ; ../scripts/run_me.sh

architecture:
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

loop_distribution:
	cd $@ ; ../scripts/run_me.sh

loop_metadata:
	cd $@ ; ../scripts/run_me.sh

uninstall:
	rm -rf ../install ;

clean:
	rm -rf */build */*.json ; 
	find ./ -name .ycm_extra_conf.py -delete ;
	find ./ -name compile_commands.json -delete ;
	rm -rf */*/.clangd ;

.PHONY: $(ALL)

.PHONY: clean download uninstall
