all: install
	../scripts/build_unit.sh 

install:
	mkdir $@ 

clean:
	rm -f *.txt ;
	rm -rf install ;
	rm -rf */build ;
	find ./ -name compile_commands.json -delete

.PHONY: clean
