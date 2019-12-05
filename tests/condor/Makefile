EMAIL=
CONDOR_FILE=regression.con

all: regression

regression: regression_enablers regression_default regression_1 regression_2 regression_3 regression_4

regression_enablers:
	cd ../ ; cp -r regression $@ ;
	python scripts/generateCondorScript.py scripts/$(CONDOR_FILE) ./$@.con "$@" "-noelle-verbose=3 -noelle-disable-helix -noelle-disable-dswp -noelle-disable-doall" ${EMAIL};
	./scripts/appendTests.sh $@.con ;

regression_default:
	cd ../ ; cp -r regression $@ ;
	python scripts/generateCondorScript.py scripts/$(CONDOR_FILE) ./$@.con "$@" "-noelle-verbose=3" ${EMAIL};
	./scripts/appendTests.sh $@.con ;

regression_1:
	cd ../ ; cp -r regression $@ ;
	python scripts/generateCondorScript.py scripts/$(CONDOR_FILE) ./$@.con "$@" "-noelle-verbose=3 -dswp-force -noelle-disable-helix" ${EMAIL};
	./scripts/appendTests.sh $@.con ;

regression_2:
	cd ../ ; cp -r regression $@ ;
	python scripts/generateCondorScript.py scripts/$(CONDOR_FILE) ./$@.con "$@" "-noelle-verbose=3 -dswp-force -noelle-disable-helix -dswp-no-scc-merge" ${EMAIL};
	./scripts/appendTests.sh $@.con ;

regression_3:
	cd ../ ; cp -r regression $@ ;
	python scripts/generateCondorScript.py scripts/$(CONDOR_FILE) ./$@.con "$@" "-noelle-verbose=3 -dswp-force -noelle-disable-dswp" ${EMAIL};
	./scripts/appendTests.sh $@.con ;

regression_4:
	cd ../ ; cp -r regression $@ ;
	python scripts/generateCondorScript.py scripts/$(CONDOR_FILE) ./$@.con "$@" "-noelle-verbose=3 -dswp-force -noelle-disable-dswp -dswp-no-scc-merge " ${EMAIL};
	./scripts/appendTests.sh $@.con ;

submit:
	export PATH=`realpath ../../install/bin`:${PATH} ; scripts/submit.sh 

clean:
	rm -rf log/*.out log/*.err log/*.log *~ ./*.con

.PHONY: submit regression clean
