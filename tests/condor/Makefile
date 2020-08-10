EMAIL=
CONDOR_FILE=regression.con

all: performance.con regression

regression: regression_enablers regression_default regression_1 regression_2 regression_3 regression_4 regression_5 regression_6 regression_7 regression_8 regression_9 regression_10 regression_11 regression_12 regression_13 regression_14 regression_15 regression_16 unit.con

regression_enablers:
	cd ../ ; cp -r regression $@ ;
	python2 scripts/generateCondorScript.py scripts/$(CONDOR_FILE) ./$@.con "$@" "-noelle-pdg-check -noelle-verbose=3 -noelle-disable-helix -noelle-disable-dswp -noelle-disable-doall" " " ${EMAIL};
	./scripts/appendTests.sh $@.con ;

regression_default:
	cd ../ ; cp -r regression $@ ;
	python2 scripts/generateCondorScript.py scripts/$(CONDOR_FILE) ./$@.con "$@" "-noelle-pdg-check -noelle-verbose=3" "-noelle-parallelizer-force" ${EMAIL};
	./scripts/appendTests.sh $@.con ;

regression_1:
	cd ../ ; cp -r regression $@ ;
	python2 scripts/generateCondorScript.py scripts/$(CONDOR_FILE) ./$@.con "$@" "-noelle-pdg-check -noelle-verbose=3 -noelle-disable-helix" "-noelle-parallelizer-force" ${EMAIL};
	./scripts/appendTests.sh $@.con ;

regression_2:
	cd ../ ; cp -r regression $@ ;
	python2 scripts/generateCondorScript.py scripts/$(CONDOR_FILE) ./$@.con "$@" "-noelle-pdg-check -noelle-verbose=3 -noelle-disable-helix" "-noelle-parallelizer-force -dswp-no-scc-merge" ${EMAIL};
	./scripts/appendTests.sh $@.con ;

regression_3:
	cd ../ ; cp -r regression $@ ;
	python2 scripts/generateCondorScript.py scripts/$(CONDOR_FILE) ./$@.con "$@" "-noelle-pdg-check -noelle-verbose=3 -noelle-disable-dswp" "-noelle-parallelizer-force" ${EMAIL};
	./scripts/appendTests.sh $@.con ;

regression_4:
	cd ../ ; cp -r regression $@ ;
	python2 scripts/generateCondorScript.py scripts/$(CONDOR_FILE) ./$@.con "$@" "-noelle-pdg-check -noelle-verbose=3 -noelle-disable-dswp" "-noelle-parallelizer-force -dswp-no-scc-merge " ${EMAIL};
	./scripts/appendTests.sh $@.con ;

regression_5:
	cd ../ ; cp -r regression $@ ;
	python2 scripts/generateCondorScript.py scripts/$(CONDOR_FILE) ./$@.con "$@" "-noelle-pdg-check -noelle-verbose=3 -noelle-disable-dswp -noelle-disable-inliner" "-noelle-parallelizer-force" ${EMAIL};
	./scripts/appendTests.sh $@.con ;

regression_6:
	cd ../ ; cp -r regression $@ ;
	python2 scripts/generateCondorScript.py scripts/$(CONDOR_FILE) ./$@.con "$@" "-noelle-pdg-check -noelle-verbose=3 -noelle-disable-helix -noelle-disable-inliner" " -noelle-parallelizer-force" ${EMAIL};
	./scripts/appendTests.sh $@.con ;

regression_7:
	cd ../ ; cp -r regression $@ ;
	python2 scripts/generateCondorScript.py scripts/$(CONDOR_FILE) ./$@.con "$@" "-noelle-pdg-check -noelle-verbose=3 -noelle-disable-helix -noelle-disable-dswp -noelle-max-cores=2" " -noelle-parallelizer-force" ${EMAIL};
	./scripts/appendTests.sh $@.con ;

regression_8:
	cd ../ ; cp -r regression $@ ;
	python2 scripts/generateCondorScript.py scripts/$(CONDOR_FILE) ./$@.con "$@" "-noelle-pdg-check -noelle-verbose=3 -noelle-disable-helix -noelle-disable-doall -noelle-max-cores=2" " -noelle-parallelizer-force" ${EMAIL};
	./scripts/appendTests.sh $@.con ;

regression_9:
	cd ../ ; cp -r regression $@ ;
	python2 scripts/generateCondorScript.py scripts/$(CONDOR_FILE) ./$@.con "$@" "-noelle-pdg-check -noelle-verbose=3 -noelle-disable-doall -noelle-disable-dswp -noelle-max-cores=2" " -noelle-parallelizer-force" ${EMAIL};
	./scripts/appendTests.sh $@.con ;

regression_10:
	cd ../ ; cp -r regression $@ ;
	python2 scripts/generateCondorScript.py scripts/$(CONDOR_FILE) ./$@.con "$@" "-noelle-pdg-check -noelle-verbose=3 -noelle-inliner-avoid-hoist-to-main" " -noelle-parallelizer-force" ${EMAIL};
	./scripts/appendTests.sh $@.con ;

regression_11:
	cd ../ ; cp -r regression $@ ;
	python2 scripts/generateCondorScript.py scripts/$(CONDOR_FILE) ./$@.con "$@" "-noelle-pdg-check -noelle-verbose=3 -noelle-inliner-avoid-hoist-to-main -noelle-disable-helix" " -noelle-parallelizer-force" ${EMAIL};
	./scripts/appendTests.sh $@.con ;

regression_12:
	cd ../ ; cp -r regression $@ ;
	python2 scripts/generateCondorScript.py scripts/$(CONDOR_FILE) ./$@.con "$@" "-noelle-pdg-check -noelle-verbose=3 -noelle-disable-loop-invariant-code-motion -noelle-disable-helix -noelle-disable-dswp -noelle-disable-doall" " -noelle-parallelizer-force" ${EMAIL};
	./scripts/appendTests.sh $@.con ;

regression_13:
	cd ../ ; cp -r regression $@ ;
	python2 scripts/generateCondorScript.py scripts/$(CONDOR_FILE) ./$@.con "$@" "-noelle-pdg-check -noelle-verbose=3 -noelle-disable-loop-invariant-code-motion -noelle-disable-dswp -noelle-disable-doall" " -noelle-parallelizer-force" ${EMAIL};
	./scripts/appendTests.sh $@.con ;

regression_14:
	cd ../ ; cp -r regression $@ ;
	python2 scripts/generateCondorScript.py scripts/$(CONDOR_FILE) ./$@.con "$@" "-noelle-pdg-check -noelle-verbose=3 -noelle-disable-loop-invariant-code-motion -noelle-disable-helix -noelle-disable-doall" " -noelle-parallelizer-force" ${EMAIL};
	./scripts/appendTests.sh $@.con ;

regression_15:
	cd ../ ; cp -r regression $@ ;
	python2 scripts/generateCondorScript.py scripts/$(CONDOR_FILE) ./$@.con "$@" "-noelle-pdg-check -noelle-verbose=3 -noelle-disable-loop-invariant-code-motion -noelle-disable-helix" " -noelle-parallelizer-force" ${EMAIL};
	./scripts/appendTests.sh $@.con ;

regression_16:
	cd ../ ; cp -r regression $@ ;
	python2 scripts/generateCondorScript.py scripts/$(CONDOR_FILE) ./$@.con "$@" "-noelle-pdg-check -noelle-verbose=3 -noelle-disable-loop-invariant-code-motion -noelle-disable-dswp" " -noelle-parallelizer-force" ${EMAIL};
	./scripts/appendTests.sh $@.con ;

unit.con:
	python2 scripts/generateCondorScript.py scripts/$@ ./$@ "$@" " " " -noelle-parallelizer-force" ${EMAIL};

performance.con:
	python2 scripts/generateCondorScript.py scripts/$@ ./$@ "$@" " " " -noelle-parallelizer-force" ${EMAIL};

submit:
	scripts/submit.sh 

clean:
	rm -rf log/*.out log/*.err log/*.log *~ ./*.con

.PHONY: submit regression clean
