INSTALL_DIR?=install
BUILD_DIR?=build
JOBS?=8
GENERATOR?="Unix Makefiles" # or Ninja

all: install

install: compile
	cmake --install $(BUILD_DIR)

compile: external $(BUILD_DIR)
	cmake --build $(BUILD_DIR) -- -j$(JOBS)

$(BUILD_DIR):
	cmake -S . -B $(BUILD_DIR) -G$(GENERATOR) -DCMAKE_INSTALL_PREFIX=$(INSTALL_DIR)
	
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
	find ./ -name .clangd -exec rm -rv {} +
	find ./ -name .cache -exec rm -rv {} +

uninstall:
	-cat $(BUILD_DIR)/install_manifest.txt | xargs rm -f
	$(MAKE) -C external uninstall
	rm -rf $(INSTALL_DIR)/autotuner
	rm -rf $(INSTALL_DIR)/include/svf
	rm -rf $(INSTALL_DIR)/include/scaf
	rm -rf $(INSTALL_DIR)/test
	rm -f enable
	rm -f .git/hooks/pre-commit

.PHONY: all install compile external tests format clean uninstall
