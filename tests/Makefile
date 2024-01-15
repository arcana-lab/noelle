all: unit

unit:
	cd unit ; make ;

clean:
	./scripts/clean.sh ; 
	rm -rf tmp* ;
	cd unit ; make clean ;
	rm -f compiler_output* ;
	find ./ -name output_parallelized.txt.xz -delete
	find ./ -name vgcore* -delete
	rm -f TestDir_not_exists*

.PHONY: unit clean 
