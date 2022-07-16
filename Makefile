EXTERNAL_OPTIONS=
DEBUG?=0
JOBS?=8

BUILD_DIR=build
INSTALL_DIR=install

all: hooks build

external:
	cd external ; make DEBUG=$(DEBUG) JOBS=$(JOBS) $(EXTERNAL_OPTIONS);

build: external
	mkdir -p $(BUILD_DIR)
	mkdir -p $(INSTALL_DIR)
	cmake -DCMAKE_C_COMPILER=`which clang` -DCMAKE_CXX_COMPILER=`which clang++` -S . -B $(BUILD_DIR)
	make -C $(BUILD_DIR) all DEBUG=$(DEBUG) -j$(JOBS)

# src-fast: external
# 	cd src ; make core-fast DEBUG=$(DEBUG) JOBS=$(JOBS);
# 	cd src ; make tools-fast DEBUG=$(DEBUG) JOBS=$(JOBS);

# tests: src
# 	cd tests ; make ;

hooks:
	make -C .githooks

format:
	cd src ; ./scripts/format_source_code.sh

clean:
	cd external ; make clean ; 
	cd src ; make clean ; 
	cd tests ; make clean; 
	cd examples ; make clean ; 
	find ./ -name .clangd -exec rm -rv {} +
	find ./ -name .cache -exec rm -rv {} +

uninstall: clean
	rm -f enable ;
	rm -rf install ;
	cd external ; make $@

.PHONY: src src-fast tests hooks format clean uninstall external
