INSTALL_DIR=install
BUILD_DIR=build
GENERATOR=Unix Makefiles # or Ninja
JOBS=8

all: hooks build
	cmake --build $(BUILD_DIR) -- -j$(JOBS)

build:
	cmake -S . -B $(BUILD_DIR) -G$(GENERATOR) -DCMAKE_INSTALL_PREFIX=$(INSTALL_DIR)

install: build
	cmake --build $(BUILD_DIR) -- -j$(JOBS)
	cmake --install $(BUILD_DIR)

external:
	@make -C external DEBUG=$(DEBUG) JOBS=$(JOBS)

tests: build
	@make -C tests

hooks:
	ln -sf .githooks/pre-commit .git/hooks/pre-commit

format:
	cd src ; ./scripts/format_source_code.sh

clean:
	rm -rf build
	# @make -C external clean
	@make -C tests clean
	@make -C examples clean
	find ./ -name .clangd -exec rm -rv {} +
	find ./ -name .cache -exec rm -rv {} +

uninstall: clean
	rm -f enable .git/hooks/pre-commit
	rm -rf install
	@make -C external uninstall


.PHONY: all tests hooks format clean uninstall external
