UTILS=transformations basic_utilities task loops architecture loop_metadata clean_metadata pdg_stats
ANALYSIS=pdg talkdown alloc_aa dataflow loop_structure
PARALLELIZATION=parallelizer heuristics parallelization_technique dswp doall helix
ENABLERS=enablers loop_distribution loop_unroll inliner
ALL=$(UTILS) $(ANALYSIS) $(ENABLERS) $(PARALLELIZATION) hotprofiler unique_ir_marker noelle scripts

all: $(ALL)

scripts:
	./scripts/installNOELLE.sh

noelle:
	cd $@ ; ../scripts/run_me.sh

unique_ir_marker:
	cd $@ ; ../scripts/run_me.sh

dataflow:
	cd $@ ; ../scripts/run_me.sh

hotprofiler:
	cd $@ ; ../scripts/run_me.sh

basic_utilities:
	cd $@ ; ../scripts/run_me.sh

transformations:
	cd $@ ; ../scripts/run_me.sh

loop_structure:
	cd $@ ; ../scripts/run_me.sh

task:
	cd $@ ; ../scripts/run_me.sh

loops:
	cd $@ ; ../scripts/run_me.sh

architecture:
	cd $@ ; ../scripts/run_me.sh

pdg:
	cd $@ ; ../scripts/run_me.sh

alloc_aa:
	cd $@ ; ../scripts/run_me.sh

talkdown:
	cd $@ ; ../scripts/run_me.sh

enablers:
	cd $@ ; ../scripts/run_me.sh

inliner:
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

loop_unroll:
	cd $@ ; ../scripts/run_me.sh

loop_metadata:
	cd $@ ; ../scripts/run_me.sh

clean_metadata:
	cd $@ ; ../scripts/run_me.sh

pdg_stats:
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
