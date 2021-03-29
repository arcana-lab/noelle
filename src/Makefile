DEBUG?=0
JOBS?=8

ifdef CLANG_BIN
	CC:=$(CLANG_BIN)/clang
	CXX:=$(CLANG_BIN)/clang++
else
	CC:=clang
	CXX:=clang++
endif

all: 
	cd core ; make ;
	cd tools ; make ;
	./scripts/generate_enable_file.sh

core-fast: export CC:=$(CC)
core-fast: export CXX:=$(CXX)
core-fast: 
ifeq ($(DEBUG), 1)
	echo $(DEBUG)
	echo "Making debug NOELLE: core"
	mkdir -p core-debug
	cd core-debug&&\
	cmake -DCMAKE_INSTALL_PREFIX="$(PDG_INSTALL_DIR)" -DCMAKE_BUILD_TYPE=Debug -DLLVM_ENABLE_UNWIND_TABLES=On ../core && \
	make -j${JOBS} &&\
	make install
else
	echo $(DEBUG)
	echo "Making debug NOELLE: core"
	echo "Making release NOELLE: core"
	mkdir -p core-release
	cd core-release&& \
	cmake -DCMAKE_INSTALL_PREFIX="$(PDG_INSTALL_DIR)" -DCMAKE_BUILD_TYPE=Release ../core && \
	make -j${JOBS} &&\
	make install
endif
	cd core; make scripts # install scripts

tools-fast: export CC:=$(CC)
tools-fast: export CXX:=$(CXX)
tools-fast: core-fast
ifeq ($(DEBUG), 1)
	echo "Making debug NOELLE: tools"
	mkdir -p tools-debug
	cd tools-debug&&\
	cmake -DCMAKE_INSTALL_PREFIX="$(PDG_INSTALL_DIR)" -DCMAKE_BUILD_TYPE=Debug -DLLVM_ENABLE_UNWIND_TABLES=On ../tools && \
	make -j${JOBS} &&\
	make install
else
	echo "Making release NOELLE: tools"
	mkdir -p tools-release
	cd tools-release && \
	cmake -DCMAKE_INSTALL_PREFIX="$(PDG_INSTALL_DIR)" -DCMAKE_BUILD_TYPE=Release ../tools && \
	make -j${JOBS} &&\
	make install
endif

uninstall:
	rm -rf ../install ;

clean:
	cd core ; make clean ;
	cd tools ; make clean ; 
	find ./ -name .ycm_extra_conf.py -delete ;
	find ./ -name compile_commands.json -delete ;
	find ./ -name .clangd -exec rm -rv {} +

.PHONY: clean uninstall
