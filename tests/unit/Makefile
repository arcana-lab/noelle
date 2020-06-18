all: install
	../scripts/build_unit.sh 

install:
	mkdir $@ 

clean:
	rm -f *.txt ;
	rm -rf install ;
	rm -rf */build ;
	find ./ -name compile_commands.json -delete
	find ./ -name *.bc -delete
	find ./ -name *.ll -delete
	find ./ -name output.prof -delete
	find ./ -name default.profraw -delete

.PHONY: clean
