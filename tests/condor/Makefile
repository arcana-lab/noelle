EMAIL=
CONDOR_FILE=regression.con

all: regression

regression: regression_enablers regression_default regression_1 regression_2 regression_3 regression_4 unit.con performance.con

regression_enablers:
	cd ../ ; cp -r regression $@ ;
	python2 scripts/generateCondorScript.py scripts/$(CONDOR_FILE) ./$@.con "$@" "-noelle-verbose=3 -noelle-disable-helix -noelle-disable-dswp -noelle-disable-doall" " " ${EMAIL};
	./scripts/appendTests.sh $@.con ;

regression_default:
	cd ../ ; cp -r regression $@ ;
	python2 scripts/generateCondorScript.py scripts/$(CONDOR_FILE) ./$@.con "$@" "-noelle-verbose=3" " " ${EMAIL};
	./scripts/appendTests.sh $@.con ;

regression_1:
	cd ../ ; cp -r regression $@ ;
	python2 scripts/generateCondorScript.py scripts/$(CONDOR_FILE) ./$@.con "$@" "-noelle-verbose=3 -noelle-disable-helix" "-dswp-force" ${EMAIL};
	./scripts/appendTests.sh $@.con ;

regression_2:
	cd ../ ; cp -r regression $@ ;
	python2 scripts/generateCondorScript.py scripts/$(CONDOR_FILE) ./$@.con "$@" "-noelle-verbose=3 -noelle-disable-helix" "-dswp-force -dswp-no-scc-merge" ${EMAIL};
	./scripts/appendTests.sh $@.con ;

regression_3:
	cd ../ ; cp -r regression $@ ;
	python2 scripts/generateCondorScript.py scripts/$(CONDOR_FILE) ./$@.con "$@" "-noelle-verbose=3 -noelle-disable-dswp" "-dswp-force" ${EMAIL};
	./scripts/appendTests.sh $@.con ;

regression_4:
	cd ../ ; cp -r regression $@ ;
	python2 scripts/generateCondorScript.py scripts/$(CONDOR_FILE) ./$@.con "$@" "-noelle-verbose=3 -noelle-disable-dswp" "-dswp-force -dswp-no-scc-merge " ${EMAIL};
	./scripts/appendTests.sh $@.con ;

unit.con:
	python2 scripts/generateCondorScript.py scripts/$@ ./$@ "$@" " " " " ${EMAIL};

performance.con:
	python2 scripts/generateCondorScript.py scripts/$@ ./$@ "$@" " " " " ${EMAIL};

submit:
	scripts/submit.sh 

clean:
	rm -rf log/*.out log/*.err log/*.log *~ ./*.con

.PHONY: submit regression clean
