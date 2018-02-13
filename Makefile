all: printer analysis user

printer:
	cd $@ ; ./run_me.sh

analysis:
	cd $@ ; ./run_me.sh

user:
	cd $@ ; ./run_me.sh

uninstall:
	rm -rf ~/CAT ;

clean:
	./scripts/clean.sh ;

.PHONY: clean analysis printer user
