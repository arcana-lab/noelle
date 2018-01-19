all: printer analysis

printer:
	cd $@ ; ./run_me.sh

analysis:
	cd $@ ; ./run_me.sh

uninstall:
	rm -rf ~/CAT ;

clean:
	rm -rf printer/build analysis/build ;

.PHONY: clean analysis printer
