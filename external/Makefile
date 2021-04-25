DEBUG?=0
JOBS?=8
SVF_BRANCH?=SVF-1.8
SCAF_BRANCH?=v9.1.1

all: compile

compile: svf patch.tar scaf
	cd svf ; tar xf ../patch.tar ;
	./scripts/patch.sh
	./scripts/scaf_patch_make.sh 
ifeq ($(DEBUG), 1)
	echo "Making debug SVF and SCAF"
	cd svf ; ./build.sh debug;
	cd scaf ; make scaf-debug;
else
	echo "Making release SVF and SCAF"
	cd svf ; ./build.sh;
	cd scaf ; make scaf-release;
endif

compile_without_scaf: svf patch.tar
	cd svf ; tar xf ../patch.tar ;
	./scripts/patch.sh
	cd svf ; ./build.sh ;

svf:
	git clone -b $(SVF_BRANCH) https://github.com/SVF-tools/SVF.git svf

patch.tar: patch svf
	cd $< ; tar cf $@ * ; mv $@ ../ ;

scaf:
	git clone -b $(SCAF_BRANCH) https://github.com/PrincetonUniversity/SCAF scaf

clean:
	./scripts/clean.sh
	rm -f patch.tar 

uninstall:
	rm -rf svf ;
	rm -rf scaf ;

.PHONY: compile compile_without_scaf clean uninstall
