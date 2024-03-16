BUILD_DIR ?= build
export GENERATOR ?= Unix Makefiles
export JOBS ?= 16
export MAKEFLAGS += --no-print-directory
export KCONFIG_CONFIG = .config
export MENUCONFIG_STYLE = aquatic

all: install

install: external compile
	cmake --install $(BUILD_DIR) 

compile: build
	cmake --build $(BUILD_DIR) -j$(JOBS) 

build:
	cmake -S . -B $(BUILD_DIR) -G "$(GENERATOR)" \
		-DCMAKE_INSTALL_MESSAGE=LAZY \
		-DCMAKE_INSTALL_PREFIX=install

tests: install
	$(MAKE) -C tests

format:
	find ./src -regex '.*\.[c|h]pp' | xargs clang-format -i

menuconfig:
	@python3 bin/menuconfig.py

clean: uninstall
	rm -rf $(BUILD_DIR)
	rm -rf .config
	$(MAKE) -C tests clean
	$(MAKE) -C examples clean
	rm -f compile_commands.json
	rm -f config.cmake

uninstall:
	-cat $(BUILD_DIR)/install_manifest.txt | xargs rm -f
	rm -f enable
	rm -f .git/hooks/pre-commit

.PHONY: all build install compile external menuconfig tests format clean uninstall
