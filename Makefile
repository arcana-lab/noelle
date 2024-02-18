INSTALL_DIR=install
BUILD_DIR=build
JOBS=8

all: hooks build
	cmake --build $(BUILD_DIR) -- -j$(JOBS)

build:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_INSTALL_PREFIX=$(INSTALL_DIR)

install: build
	cmake --build $(BUILD_DIR) -- -j$(JOBS)
	cmake --install $(BUILD_DIR)

external:
	cd external ; make DEBUG=$(DEBUG) JOBS=$(JOBS) $(EXTERNAL_OPTIONS);

tests: build
	cd tests ; make

hooks:
	ln -sf .githooks/pre-commit .git/hooks/pre-commit

format:
	cd src ; ./scripts/format_source_code.sh

clean:
	rm -rf build
	# cd external ; make clean
	# cd tests ; make clean
	# cd examples ; make clean
	find ./ -name .clangd -exec rm -rv {} +
	find ./ -name .cache -exec rm -rv {} +

uninstall: clean
	rm -f enable
	rm -rf install
	cd external ; make $@
	if test -d .githooks ; then cd .githooks ; make clean ; fi

.PHONY: all tests hooks format clean uninstall external
