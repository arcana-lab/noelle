all: core-fast tools-fast

external:
	cd external ; make ;

core-fast: export CC=/scratch/install/llvm-install-release/bin/clang
core-fast: export CXX=/scratch/install/llvm-install-release/bin/clang++
core-fast: external
ifeq ($(DEBUG), "TRUE")
	echo "Making debug NOELLE: core"
	mkdir -p core-debug
	cd core-debug&&\
	cmake -DCMAKE_INSTALL_PREFIX="$(PDG_INSTALL_DIR)" -DCMAKE_BUILD_TYPE=Debug -DLLVM_ENABLE_UNWIND_TABLES=On ../src/core && \
	make -j${JOBS} &&\
	make install
else
	echo "Making release NOELLE: core"
	mkdir -p core-release
	cd core-release&& \
	cmake -DCMAKE_INSTALL_PREFIX="$(PDG_INSTALL_DIR)" -DCMAKE_BUILD_TYPE=Release ../src/core && \
	make -j${JOBS} &&\
	make install
endif

tools-fast: export CC=/scratch/install/llvm-install-release/bin/clang
tools-fast: export CXX=/scratch/install/llvm-install-release/bin/clang++
tools-fast: core-fast
ifeq ($(DEBUG), "TRUE")
	echo "Making debug NOELLE: tools"
	mkdir -p tools-debug
	cd tools-debug&&\
	cmake -DCMAKE_INSTALL_PREFIX="$(PDG_INSTALL_DIR)" -DCMAKE_BUILD_TYPE=Debug -DLLVM_ENABLE_UNWIND_TABLES=On ../src/tools && \
	make -j${JOBS} &&\
	make install
else
	echo "Making release NOELLE: tools"
	mkdir -p tools-release
	cd tools-release && \
	cmake -DCMAKE_INSTALL_PREFIX="$(PDG_INSTALL_DIR)" -DCMAKE_BUILD_TYPE=Release ../src/tools && \
	make -j${JOBS} &&\
	make install
endif

src: 
	cd src ; make ; 
	
tests: src
	cd tests ; make ;

clean:
	cd external ; make clean ; 
	cd src ; make clean ; 
	cd tests ; make clean; 
	cd examples ; make clean ; 
	find ./ -name .clangd -exec rm -rv {} +

uninstall:
	rm -rf install ;
	cd external ; make $@

.PHONY: src tests clean uninstall external
