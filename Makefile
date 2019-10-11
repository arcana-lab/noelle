all: src tests

src:
	cd src ; make ; 
	
tests: src
	cd tests ; make ;

clean:
	cd src ; make clean ; cd ../tests ; make clean; 

.PHONY: src tests clean
