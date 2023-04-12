all: links
	../scripts/compileExamples.sh

links:
	../scripts/links.sh

clean: links
	../scripts/clean.sh

uninstall: clean
	rm */scripts */*.txt ;

.PHONY: clean links uninstall
