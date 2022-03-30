RUNTIME_DIRNAME="threadpool"
RUNTIME_GITREPO="https://github.com/scampanoni/virgil.git"
RUNTIME_VERSION="1.0.0"

all: regression performance unit

condor: download
	cd condor ; make ; make submit ;

condor_watch:
	watch condor_q `whoami` ;

condor_check: 
	./scripts/condor_check.sh ;

condor_regression_add:
	./scripts/condor_regression_set.sh ;

regression: download
	./scripts/test_regression.sh ;

performance: download
	./scripts/test_performance.sh ;

unit:
	cd unit ; make ;

download:
	mkdir -p include ; cd include ; ../scripts/download.sh "$(RUNTIME_GITREPO)" $(RUNTIME_VERSION) "$(RUNTIME_DIRNAME)" ;
	./scripts/add_symbolic_link.sh ;

clean:
	./scripts/clean.sh ; rm -rf include/ ; rm -f regression/*.txt ;
	rm -rf regression_* ;
	rm -rf tmp.* ;
	cd condor ; make clean ; 
	cd unit ; make clean ;
	rm -f compiler_output* ;
	find ./ -name output_parallelized.txt.xz -delete
	find ./ -name vgcore* -delete

.PHONY: condor condor_check regression performance unit download clean condor_regression_add
