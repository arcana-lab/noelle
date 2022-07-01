GIT_ROOT=$(shell git rev-parse --show-toplevel)

GIT_HOOKS_DIR=$(GIT_ROOT)/.git/hooks

HOOK_NAMES=pre-commit
GIT_HOOKS=$(addprefix $(GIT_HOOKS_DIR)/, $(HOOK_NAMES))

all: $(GIT_HOOKS)

$(GIT_HOOKS_DIR)/%: $(CURDIR)/hooks/%
	ln -s $< $@

clean:
	rm -f $(GIT_HOOKS)

.PHONY: all clean
