EMAIL=
CONDOR_FILE=regression.con

all: performance.con regression unit.con

regression:
	./scripts/generateRegressions.sh scripts/$(CONDOR_FILE) $(EMAIL)

unit.con:
	python2 scripts/generateCondorScript.py scripts/$@ ./$@ "$@" " " " -noelle-parallelizer-force" "-O0 -Xclang -disable-O0-optnone" "-O0" ${EMAIL};

performance.con:
	python2 scripts/generateCondorScript.py scripts/$@ ./$@ "$@" " " " -noelle-parallelizer-force" "-O0 -Xclang -disable-O0-optnone" "-O0" ${EMAIL};

submit:
	scripts/submit.sh 

clean:
	rm -rf log/*.out log/*.err log/*.log *~ ./*.con log/*.txt 

.PHONY: submit regression clean
