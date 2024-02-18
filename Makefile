INSTALL_DIR=install
BUILD_DIR=build
JOBS=8
GENERATOR="Unix Makefiles" # or Ninja

all: install

install: compile
	cmake --install $(BUILD_DIR)

compile: $(BUILD_DIR)
	cmake --build $(BUILD_DIR) -- -j$(JOBS)

$(BUILD_DIR):
	cmake -S . -B $(BUILD_DIR) -G$(GENERATOR) -DCMAKE_INSTALL_PREFIX=$(INSTALL_DIR)
	
external:
	cd external ; make DEBUG=$(DEBUG) JOBS=$(JOBS) $(EXTERNAL_OPTIONS);

tests: setup
	cd tests ; make

format:
	cd src ; ./scripts/format_source_code.sh

clean:
	rm -rf $(BUILD_DIR)
	# cd external ; make clean
	# cd tests ; make clean
	# cd examples ; make clean
	find ./ -name .clangd -exec rm -rv {} +
	find ./ -name .cache -exec rm -rv {} +

uninstall:
	cat $(BUILD_DIR)/install_manifest.txt | xargs rm -f
	# cd external ; make $@


.PHONY: all install compile external tests format clean uninstall
