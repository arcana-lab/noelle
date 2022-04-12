all: links
	../scripts/compileExamples.sh

links:
	../scripts/links.sh

clean: links
	cd simple ; ./scripts/clean.sh
	cd pdg ; ./scripts/clean.sh
	cd callgraph ; ./scripts/clean.sh
	cd loops ; ./scripts/clean.sh
	cd profile ; ./scripts/clean.sh
	cd dfa ; ./scripts/clean.sh
	cd dfa2 ; ./scripts/clean.sh
	cd dfa3 ; ./scripts/clean.sh
	cd induction_variables ; ./scripts/clean.sh
	cd environment ; ./scripts/clean.sh

uninstall: clean
	rm */scripts */*.txt ;

.PHONY: clean links uninstall
