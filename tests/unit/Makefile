UTIL_UNITS=empty_template helpers control_flow_equivalence dominator_summary
ENABLER_UNITS=loop_invariant_code_motion
ANALYSIS_UNITS=dependence_graphs iv_attributes sccdag_attributes loop_domain_space
ALL_UNITS=$(UTIL_UNITS) $(ENABLER_UNITS) $(ANALYSIS_UNITS)

all: setup $(ALL_UNITS)

run:
	for i in `ls`; do ../scripts/run_unit.sh $$i ; done

setup:
	mkdir -p `realpath ../../install`/test

control_flow_equivalence:
	cd $@ ; PDG_INSTALL_DIR=`realpath ../../../install`/test ../../../src/scripts/run_me.sh

dependence_graphs:
	cd $@ ; PDG_INSTALL_DIR=`realpath ../../../install`/test ../../../src/scripts/run_me.sh

dominator_summary:
	cd $@ ; PDG_INSTALL_DIR=`realpath ../../../install`/test ../../../src/scripts/run_me.sh

empty_template:
	cd $@ ; PDG_INSTALL_DIR=`realpath ../../../install`/test ../../../src/scripts/run_me.sh

helpers:
	cd $@ ; PDG_INSTALL_DIR=`realpath ../../../install`/test ../../../src/scripts/run_me.sh

iv_attributes:
	cd $@ ; PDG_INSTALL_DIR=`realpath ../../../install`/test ../../../src/scripts/run_me.sh

loop_domain_space:
	cd $@ ; PDG_INSTALL_DIR=`realpath ../../../install`/test ../../../src/scripts/run_me.sh

loop_invariant_code_motion:
	cd $@ ; PDG_INSTALL_DIR=`realpath ../../../install`/test ../../../src/scripts/run_me.sh

sccdag_attributes:
	cd $@ ; PDG_INSTALL_DIR=`realpath ../../../install`/test ../../../src/scripts/run_me.sh

clean:
	rm -f *.txt ;
	rm -rf */build ;
	find ./ -name compile_commands.json -delete
	find ./ -name *.bc -delete
	find ./ -name *.ll -delete
	find ./ -name output.prof -delete
	find ./ -name default.profraw -delete

.PHONY: $(ALL_UNITS)

.PHONY: clean setup run
