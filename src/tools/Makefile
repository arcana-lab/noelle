PARALLELIZER=parallelizer heuristics parallelization_technique dswp doall helix parallelization_planner
TOOLS=pdg_stats codesize loop_size
ALL=$(TOOLS) enablers deadfunctioneliminator loop_invariant_code_motion scev_simplification inliner $(PARALLELIZER) loop_stats loop_metadata scripts

all: $(ALL)

scripts:
	./scripts/installNOELLETools.sh

pdg_stats:
	cd $@ ; ../../scripts/run_me.sh

loop_metadata:
	cd $@ ; ../../scripts/run_me.sh

codesize:
	cd $@ ; ../../scripts/run_me.sh

loop_size:
	cd $@ ; ../../scripts/run_me.sh

parallelizer:
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

clean:
	rm -rf */build */*.json ; 
	rm -rf */build */*/*.json ; 

.PHONY: $(ALL)

.PHONY: clean uninstall
