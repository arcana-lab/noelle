all: compile

compile: svf patch.tar
	cd svf ; tar xf ../patch.tar ;
	cd svf ; ./build.sh ;

svf:
	git clone https://github.com/SVF-tools/SVF.git svf
	./scripts/patch.sh

patch.tar: patch svf
	cd $< ; tar cf $@ * ; mv $@ ../ ;

clean:
	./scripts/clean.sh
	rm -f patch.tar 

uninstall:
	rm -rf svf ;

.PHONY: compile clean uninstall
