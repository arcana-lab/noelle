all: src tests

src:
	cd src ; make ; 
	
tests: src
	cd tests ; make ;

clean:
	cd src ; make clean ; 
	cd tests ; make clean; 

uninstall:
	rm -rf install ;

.PHONY: src tests clean uninstall
