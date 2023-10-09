UTILS=transformations basic_utilities types_manager globals_manager functions_manager constants_manager compilation_options_manager linker dominators task loop_induction_variables loop_carried_dependences may_points_to_analysis memory_cloning_analysis loop_scc_attributes loop_sccdag_attributes loop_sccdag_normalizer ldg_analysis loop_content loop_nesting_graph architecture metadata_cleaner callgraph scheduler metadata_manager loop_transformer alias_analysis_engine codesize
ANALYSIS=dg pdg sccdag pdg_printer pdg_analysis talkdown alloc_aa dataflow loop_structure loop_environment loop_forest loop_invariants data_dependence_analysis
ENABLERS=loop_distribution loop_unroll loop_whilifier outliner cfg_analysis cfg_transformer
ALL=$(UTILS) $(ANALYSIS) $(ENABLERS) loop_metadata hotprofiler unique_ir_marker noelle scripts

all: $(ALL)

scripts:
	./scripts/installNOELLE.sh

noelle:
	cd $@ ; ../../scripts/run_me.sh

metadata_manager:
	cd $@ ; ../../scripts/run_me.sh

alias_analysis_engine:
	cd $@ ; ../../scripts/run_me.sh

dominators:
	cd $@ ; ../../scripts/run_me.sh

compilation_options_manager:
	cd $@ ; ../../scripts/run_me.sh

loop_transformer:
	cd $@ ; ../../scripts/run_me.sh

cfg_analysis:
	cd $@ ; ../../scripts/run_me.sh

cfg_transformer:
	cd $@ ; ../../scripts/run_me.sh

data_dependence_analysis:
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

globals_manager:
	cd $@ ; ../../scripts/run_me.sh

functions_manager:
	cd $@ ; ../../scripts/run_me.sh

types_manager:
	cd $@ ; ../../scripts/run_me.sh

linker:
	cd $@ ; ../../scripts/run_me.sh

constants_manager:
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

may_points_to_analysis:
	cd $@ ; ../../scripts/run_me.sh

memory_cloning_analysis:
	cd $@ ; ../../scripts/run_me.sh

loop_scc_attributes:
	cd $@ ; ../../scripts/run_me.sh

loop_sccdag_normalizer:
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

sccdag:
	cd $@ ; ../../scripts/run_me.sh

pdg_printer:
	cd $@ ; ../../scripts/run_me.sh

pdg_analysis:
	cd $@ ; ../../scripts/run_me.sh

ldg_analysis:
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

loop_metadata:
	cd $@ ; ../../scripts/run_me.sh

metadata_cleaner:
	cd $@ ; ../../scripts/run_me.sh

codesize:
	cd $@ ; ../../scripts/run_me.sh

clean:
	rm -rf */build */*.json ; 
	rm -rf */build */*/*.json ; 

.PHONY: $(ALL)

.PHONY: clean
