DEBUG?=0
JOBS?=8
SVF_BRANCH?=SVF-1.8
SCAF_BRANCH?=v9.1.8

all: compile autotuner install_scaf

compile: svf patch.tar scaf
ifeq ($(DEBUG), 1)
	cd svf ; ./build.sh debug
else
	cd svf ; ./build.sh
endif

install_scaf: scaf
	./scripts/scaf_install.sh $(DEBUG)

compile_without_scaf: svf patch.tar
	cd svf ; tar xf ../patch.tar
	./scripts/patch.sh
	cd svf ; ./build.sh

svf:
	git clone -b $(SVF_BRANCH) https://github.com/SVF-tools/SVF.git svf
	cd svf ; tar xf ../patch.tar
	./scripts/svf_patch.sh

patch.tar: patch svf
	cd $< ; tar cf $@ * ; mv $@ ../

scaf:
	git clone -b $(SCAF_BRANCH) https://github.com/PrincetonUniversity/SCAF scaf

autotuner:
	cd autotuner ; $(MAKE)

clean:
	rm -rf scaf
	rm -rf svf
	rm -f patch.tar 

uninstall:
	-cat svf/Debug-build/install_manifest.txt | xargs rm -f
	-cat svf/Release-build/install_manifest.txt | xargs rm -f
	-cat scaf/scaf-build-release/install_manifest.txt | xargs rm -f
	-cat scaf/scaf-build-debug/install_manifest.txt | xargs rm -f

.PHONY: compile install_scaf compile_without_scaf clean uninstall autotuner
