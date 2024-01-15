TOOLS=pdg_stats loop_size removefunction
ALL=$(TOOLS) privatizer deadfunctioneliminator loop_invariant_code_motion scev_simplification loop_stats scripts

all: $(ALL)

scripts:
	./scripts/installNOELLETools.sh

pdg_stats:
	cd $@ ; ../../scripts/run_me.sh

loop_size:
	cd $@ ; ../../scripts/run_me.sh

privatizer:
	cd $@ ; ../../scripts/run_me.sh

deadfunctioneliminator:
	cd $@ ; ../../scripts/run_me.sh

loop_invariant_code_motion:
	cd $@ ; ../../scripts/run_me.sh

scev_simplification:
	cd $@ ; ../../scripts/run_me.sh

loop_stats:
	cd $@ ; ../../scripts/run_me.sh

removefunction:
	cd $@ ; ../../scripts/run_me.sh

repl:
	cd $@ ; ../../scripts/run_me.sh

clean:
	rm -rf */build */*.json ; 
	rm -rf */build */*/*.json ; 

.PHONY: $(ALL) clean uninstall
