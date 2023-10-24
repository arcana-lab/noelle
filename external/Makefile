DEBUG?=0
JOBS?=8
SVF_BRANCH?=SVF-1.8
SCAF_BRANCH?=v9.1.6
SCAF_ENABLE?=0
SVF_ENABLE?=0

all: compile autotuner

compile: svf patch.tar scaf
	cd svf ; tar xf ../patch.tar ;
	./scripts/patch.sh
ifeq ($(SVF_ENABLE), 1)
ifeq ($(DEBUG), 1)
	cd svf ; ./build.sh debug;
else
	cd svf ; ./build.sh;
endif
endif

svf:
	git clone -b $(SVF_BRANCH) https://github.com/SVF-tools/SVF.git svf

patch.tar: patch svf
	cd $< ; tar cf $@ * ; mv $@ ../ ;

scaf:
ifeq ($(SCAF_ENABLE), 1)
	git clone -b $(SCAF_BRANCH) https://github.com/PrincetonUniversity/SCAF scaf
	./scripts/scaf_install.sh $(DEBUG) ;
endif

autotuner:
	cd autotuner ; make

clean:
	./scripts/clean.sh
	rm -f patch.tar 
	cd autotuner ; make clean

uninstall:
	rm -rf svf ;
	rm -rf scaf ;

.PHONY: compile compile_without_scaf clean uninstall autotuner
