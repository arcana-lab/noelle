NOELLE_SCAF ?= ON
NOELLE_SVF ?= ON
NOELLE_AUTOTUNER ?= ON
SCAF_DEBUG ?= 0
SVF_DEBUG ?= 0
JOBS ?= 8
SVF_BRANCH ?= SVF-1.8
SCAF_BRANCH ?= v9.1.8
export SCAF_INSTALL_DEBUG ?= $(NOELLE_INSTALL_DIR)
export SCAF_INSTALL_RELEASE ?= $(NOELLE_INSTALL_DIR)
export SPECULATION_MODULES=0

all: install

install:
	@if [[ $(NOELLE_SCAF) =~ ^ON|On|on$$ ]]; then \
		$(MAKE) install_scaf ; \
	fi
	@if [[ $(NOELLE_AUTOTUNER) =~ ^ON|On|on$$ ]]; then \
		$(MAKE) install_autotuner	; \
	fi

install_scaf: scaf
ifeq ($(SCAF_DEBUG), 1)
	$(MAKE) -C scaf scaf-debug
else
	$(MAKE) -C scaf scaf-release
endif

install_autotuner: autotuner
	mkdir -p $(shell realpath $(NOELLE_INSTALL_DIR))
	cp -r $< $(shell realpath $(NOELLE_INSTALL_DIR))
	
scaf:
	git clone -b $(SCAF_BRANCH) https://github.com/PrincetonUniversity/SCAF scaf
	rsync -a scaf_patch/ scaf

clean:
	rm -rf scaf

uninstall:
	-cat scaf/scaf-build-release/install_manifest.txt | xargs rm -f
	-cat scaf/scaf-build-debug/install_manifest.txt | xargs rm -f

.PHONY: all clean install install_scaf install_autotuner uninstall
