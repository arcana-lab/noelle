EMAIL=
CONDOR_FILE=regression.con

all: performance.con unit.con regression

regression:
	./scripts/generateRegressions.sh scripts/$(CONDOR_FILE) $(EMAIL)

unit.con:
	python2 scripts/generateCondorScript.py scripts/$@ ./$@ "$@" " " " -noelle-parallelizer-force" "-O0 -Xclang -disable-O0-optnone" "-O0" " " ${EMAIL};

performance.con:
	python2 scripts/generateCondorScript.py scripts/$@ ./$@ "$@" " " " -noelle-parallelizer-force" "-O0 -Xclang -disable-O0-optnone" "-O0" " " ${EMAIL};

submit:
	scripts/submit.sh 

clean:
	find log -name 'out_*.txt' -type f -delete
	find log -name 'err_*.txt' -type f -delete
	find log -name 'log_*.txt' -type f -delete
	find log -name 'condor*.err' -type f -delete
	find log -name 'condor*.log' -type f -delete
	find log -name 'condor*.out' -type f -delete
	rm -rf *~ ./*.con log/*.txt 

.PHONY: submit regression clean
