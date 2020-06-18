all: run

run:
	for i in `ls`; do ../scripts/run_unit.sh $$i ; done

build:
	../scripts/build_unit.sh 

clean:
	rm -f *.txt ;
	rm -rf */build ;
	find ./ -name compile_commands.json -delete
	find ./ -name *.bc -delete
	find ./ -name *.ll -delete
	find ./ -name output.prof -delete
	find ./ -name default.profraw -delete

.PHONY: clean build run
