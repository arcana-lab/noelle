UTILS=transformations basic_utilities dominators task loop_induction_variables loop_carried_dependences loop_scc_attributes loop_sccdag_attributes loop_content loop_nesting_graph architecture clean_metadata callgraph scheduler metadata_manager loop_transformer
ANALYSIS=dg pdg pdg_analysis talkdown alloc_aa dataflow loop_structure loop_environment loop_forest loop_invariants
ENABLERS=loop_distribution loop_unroll loop_whilifier outliner cfg_analysis cfg_transformer
ALL=$(UTILS) $(ANALYSIS) $(ENABLERS) hotprofiler unique_ir_marker noelle scripts

all: $(ALL)

scripts:
	./scripts/installNOELLE.sh

noelle:
	cd $@ ; ../../scripts/run_me.sh

metadata_manager:
	cd $@ ; ../../scripts/run_me.sh

dominators:
	cd $@ ; ../../scripts/run_me.sh

loop_transformer:
	cd $@ ; ../../scripts/run_me.sh

cfg_analysis:
	cd $@ ; ../../scripts/run_me.sh

cfg_transformer:
	cd $@ ; ../../scripts/run_me.sh

loop_invariants:
	cd $@ ; ../../scripts/run_me.sh

unique_ir_marker:
	cd $@ ; ../../scripts/run_me.sh

outliner:
	cd $@ ; ../../scripts/run_me.sh

dataflow:
	cd $@ ; ../../scripts/run_me.sh

hotprofiler:
	cd $@ ; ../../scripts/run_me.sh

basic_utilities:
	cd $@ ; ../../scripts/run_me.sh

transformations:
	cd $@ ; ../../scripts/run_me.sh

loop_structure:
	cd $@ ; ../../scripts/run_me.sh

loop_environment:
	cd $@ ; ../../scripts/run_me.sh

loop_forest:
	cd $@ ; ../../scripts/run_me.sh

task:
	cd $@ ; ../../scripts/run_me.sh

scheduler:
	cd $@ ; ../../scripts/run_me.sh

loop_induction_variables:
	cd $@ ; ../../scripts/run_me.sh

loop_carried_dependences:
	cd $@ ; ../../scripts/run_me.sh

loop_scc_attributes:
	cd $@ ; ../../scripts/run_me.sh

loop_sccdag_attributes:
	cd $@ ; ../../scripts/run_me.sh

loop_content:
	cd $@ ; ../../scripts/run_me.sh

architecture:
	cd $@ ; ../../scripts/run_me.sh

dg:
	cd $@ ; ../../scripts/run_me.sh

pdg:
	cd $@ ; ../../scripts/run_me.sh

pdg_analysis:
	cd $@ ; ../../scripts/run_me.sh

callgraph:
	cd $@ ; ../../scripts/run_me.sh

alloc_aa:
	cd $@ ; ../../scripts/run_me.sh

talkdown:
	cd $@ ; ../../scripts/run_me.sh

loop_whilifier:
	cd $@ ; ../../scripts/run_me.sh

loop_distribution:
	cd $@ ; ../../scripts/run_me.sh

loop_unroll:
	cd $@ ; ../../scripts/run_me.sh

loop_nesting_graph:
	cd $@ ; ../../scripts/run_me.sh

clean_metadata:
	cd $@ ; ../../scripts/run_me.sh

clean:
	rm -rf */build */*.json ; 
	rm -rf */build */*/*.json ; 

.PHONY: $(ALL)

.PHONY: clean
