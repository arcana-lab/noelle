RUNTIME_DIRNAME="threadpool"
RUNTIME_GITREPO="https://github.com/scampanoni"

all: regression performance

oracle_aa: download
	./scripts/test_oracle_aa.sh ;

regression: download
	./scripts/test_regression.sh ;

performance: download
	./scripts/test_performance.sh ;

download:
	mkdir -p include ; cd include ; ../scripts/download.sh "$(RUNTIME_GITREPO)/$(RUNTIME_DIRNAME)" "$(RUNTIME_DIRNAME)" ;
	./scripts/add_symbolic_link.sh ;

clean:
	./scripts/clean.sh ; rm -rf include/
