RUNTIME_DIRNAME="threadpool"
RUNTIME_GITREPO="https://github.com/scampanoni/virgil.git"

all: regression performance buildunit rununit

condor: download
	cd condor ; make ; make submit ;

regression: download
	./scripts/test_regression.sh ;

performance: download
	./scripts/test_performance.sh ;

buildunit:
	cd unit ; for i in `ls`; do ../scripts/build_unit.sh $$i ; done ; cd ../ ;

rununit:
	cd unit ; for i in `ls`; do ../scripts/run_unit.sh $$i ; done ; cd ../ ;

download:
	mkdir -p include ; cd include ; ../scripts/download.sh "$(RUNTIME_GITREPO)" "$(RUNTIME_DIRNAME)" ;
	./scripts/add_symbolic_link.sh ;

clean:
	./scripts/clean.sh ; rm -rf include/ ; rm -f regression/*.txt ;
	rm -rf regression_* ;
	cd condor ; make clean ; 

.PHONY: condor regression performance buildunit rununit download clean 
