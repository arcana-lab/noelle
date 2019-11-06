all: tests

external:
	cd external ; make ;

src: external
	cd src ; make -j ; 
	
tests: src
	cd tests ; make ;

clean:
	cd external ; make clean ; 
	cd src ; make clean ; 
	cd tests ; make clean; 

uninstall:
	rm -rf install ;
	cd external ; make $@

.PHONY: src tests clean uninstall external
