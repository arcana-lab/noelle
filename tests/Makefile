RUNTIME_DIRNAME="threadpool"
RUNTIME_GITREPO="/project/parallelizing_compiler/repositories"

all: regression performance

condor: download
	cd condor ; make ; make submit ;
	 
regression: download
	./scripts/test_regression.sh ;

performance: download
	./scripts/test_performance.sh ;

download:
	mkdir -p include ; cd include ; ../scripts/download.sh "$(RUNTIME_GITREPO)/$(RUNTIME_DIRNAME)" "$(RUNTIME_DIRNAME)" ;
	./scripts/add_symbolic_link.sh ;

clean:
	./scripts/clean.sh ; rm -rf include/ ; rm -f regression/*.txt ;
	rm -rf regression_* ;
	cd condor ; make clean ; 
