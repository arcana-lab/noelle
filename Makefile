BUILD_DIR?=build
export GENERATOR?="Unix Makefiles" # or Ninja
export JOBS?=8
export NOELLE_INSTALL_DIR?=$(shell realpath ./install)
export NOELLE_SCAF=ON
export NOELLE_SVF=ON
export NOELLE_AUTOTUNER=ON

all: install

install: external compile
	cmake --install $(BUILD_DIR)
	$(MAKE) -C external install

compile: $(BUILD_DIR)
	cmake --build $(BUILD_DIR) -- -j$(JOBS) 

$(BUILD_DIR):
	cmake -S . -B $(BUILD_DIR) -G$(GENERATOR) \
		-DCMAKE_INSTALL_PREFIX=$(NOELLE_INSTALL_DIR) \
		-DNOELLE_SCAF=$(NOELLE_SCAF) \
		-DNOELLE_SVF=$(NOELLE_SVF)
	
external:
	$(MAKE) -C external

tests: install
	$(MAKE) -C tests

format:
	find ./src -regex '.*\.[c|h]pp' | xargs clang-format -i

clean:
	rm -rf $(BUILD_DIR)
	$(MAKE) -C tests clean
	$(MAKE) -C examples clean
	$(MAKE) -C external clean
	rm -f compile_commands.json
	find ./ -name .clangd -exec rm -rv {} +
	find ./ -name .cache -exec rm -rv {} +

uninstall:
	-cat $(BUILD_DIR)/install_manifest.txt | xargs rm -f
	$(MAKE) -C external uninstall
	rm -rf $(NOELLE_INSTALL_DIR)/autotuner
	rm -rf $(NOELLE_INSTALL_DIR)/include/svf
	rm -rf $(NOELLE_INSTALL_DIR)/include/scaf
	rm -rf $(NOELLE_INSTALL_DIR)/test
	rm -f enable
	rm -f .git/hooks/pre-commit

.PHONY: all install compile external tests format clean uninstall
