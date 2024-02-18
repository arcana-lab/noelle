INSTALL_DIR=install
BUILD_DIR=build
JOBS=8
GENERATOR="Unix Makefiles" # or Ninja

all: hooks build
	cmake --build $(BUILD_DIR) -- -j$(JOBS)

build:
	cmake -S . -B $(BUILD_DIR) -G$(GENERATOR) -DCMAKE_INSTALL_PREFIX=$(INSTALL_DIR)

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
	rm -f enable .git/hooks/pre-commit
	rm -rf install
	cd external ; make $@


.PHONY: all tests hooks format clean uninstall external
