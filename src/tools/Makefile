PARALLELIZER=parallelizer heuristics parallelization_technique dswp doall helix parallelization_planner parallelizer_plan_info
TOOLS=pdg_stats loop_size removefunction time_saved autotuner_search_space autotuner_doall_filter input_output scc_printer
ALL=$(TOOLS) privatizer enablers deadfunctioneliminator loop_invariant_code_motion scev_simplification inliner $(PARALLELIZER) loop_stats scripts

all: $(ALL)

scripts:
	./scripts/installNOELLETools.sh

pdg_stats:
	cd $@ ; ../../scripts/run_me.sh

loop_size:
	cd $@ ; ../../scripts/run_me.sh

parallelizer:
	cd $@ ; ../../scripts/run_me.sh

privatizer:
	cd $@ ; ../../scripts/run_me.sh

enablers:
	cd $@ ; ../../scripts/run_me.sh

deadfunctioneliminator:
	cd $@ ; ../../scripts/run_me.sh

loop_invariant_code_motion:
	cd $@ ; ../../scripts/run_me.sh

scev_simplification:
	cd $@ ; ../../scripts/run_me.sh

inliner:
	cd $@ ; ../../scripts/run_me.sh

input_output:
	cd $@ ; ../../scripts/run_me.sh

heuristics:
	cd $@ ; ../../scripts/run_me.sh

parallelization_technique:
	cd $@ ; ../../scripts/run_me.sh

dswp:
	cd $@ ; ../../scripts/run_me.sh

doall:
	cd $@ ; ../../scripts/run_me.sh

helix:
	cd $@ ; ../../scripts/run_me.sh

loop_stats:
	cd $@ ; ../../scripts/run_me.sh

parallelization_planner:
	cd $@ ; ../../scripts/run_me.sh

autotuner_search_space:
	cd $@ ; ../../scripts/run_me.sh

autotuner_doall_filter:
	cd $@ ; ../../scripts/run_me.sh

removefunction:
	cd $@ ; ../../scripts/run_me.sh

repl:
	cd $@ ; ../../scripts/run_me.sh

time_saved:
	cd $@ ; ../../scripts/run_me.sh

parallelizer_plan_info:
	cd $@ ; ../../scripts/run_me.sh

scc_printer:
	cd $@ ; ../../scripts/run_me.sh

clean:
	rm -rf */build */*.json ; 
	rm -rf */build */*/*.json ; 

.PHONY: $(ALL)

.PHONY: clean uninstall
