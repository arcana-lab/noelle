all: passes tests

passes:
	cd passes ; make ; 
	
tests: passes
	cd tests ; make ;

clean:
	cd passes ; make clean ; cd ../tests ; make clean; 

.PHONY: passes tests
