AUTOTUNER_INSTALL_RELEASE?=$(shell pwd)/../../install

all: install

install:
	cp -r ../autotuner $(AUTOTUNER_INSTALL_RELEASE)/

clean:
	rm -rf $(AUTOTUNER_INSTALL_RELEASE)/autotuner

.PHONY: install clean
