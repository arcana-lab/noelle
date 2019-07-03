EMAIL=

all: regression

regression:
	python scripts/generateCondorScript.py scripts/$@.con ./$@.con ${EMAIL};
	./scripts/appendTests.sh $@.con ;

submit:
	condor_submit ./*.con

clean:
	rm -rf log/*.out log/*.err log/*.log *~ ./*.con

.PHONY: submit regression clean
