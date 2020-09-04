ALL=enablers deadfunctioneliminator loop_invariant_code_motion scev_simplification inliner

all: $(ALL)

scripts:
	..//scripts/installNOELLE.sh

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

clean:
	rm -rf */build */*.json ; 
	find ./ -name .ycm_extra_conf.py -delete ;
	find ./ -name compile_commands.json -delete ;
	find ./ -name .clangd -exec rm -rv {} +

.PHONY: $(ALL)

.PHONY: clean uninstall
