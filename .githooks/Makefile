GIT_ROOT=$(shell git rev-parse --git-common-dir)

GIT_HOOKS_DIR=$(GIT_ROOT)/hooks

HOOK_NAMES=pre-commit
GIT_HOOKS=$(addprefix $(GIT_HOOKS_DIR)/, $(HOOK_NAMES))

all: $(GIT_HOOKS)

$(GIT_HOOKS_DIR)/%: $(CURDIR)/hooks/%
	ln -s $< $@

clean:
	rm -f $(GIT_HOOKS)

.PHONY: all clean
