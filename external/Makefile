all: compile

compile: svf patch.tar scaf
	#cd svf ; tar xf ../patch.tar ;
	#./scripts/patch.sh
	#./scripts/scaf_patch_make.sh 
ifeq ($(DEBUG), "TRUE")
	echo "Making debug SVF and SCAF"
	cd svf ; ./build.sh debug;
	cd scaf ; make scaf-debug;
else
	echo "Making release SVF and SCAF"
	cd svf ; ./build.sh;
	cd scaf ; make scaf-release;
endif

svf:
	git clone -b SVF-1.8 https://github.com/SVF-tools/SVF.git svf

patch.tar: patch svf
	cd $< ; tar cf $@ * ; mv $@ ../ ;

scaf:
	git clone -b v9.0.0 https://github.com/PrincetonUniversity/SCAF scaf

clean:
	./scripts/clean.sh
	rm -f patch.tar 

uninstall:
	rm -rf svf ;
	rm -rf scaf ;

.PHONY: compile clean uninstall
