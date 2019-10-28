all: compile

compile: svf
	cd svf ;

svf:
	git clone https://github.com/SVF-tools/SVF.git svf
	./scripts/patch.sh

clean:
	./scripts/clean.sh

uninstall:
	rm -rf svf ;

.PHONY: compile clean uninstall
