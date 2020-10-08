all: 
	cd core ; make  ;
	cd tools ; make  ;

uninstall:
	rm -rf ../install ;

clean:
	cd core ; make clean ;
	cd tools ; make clean ; 
	find ./ -name .ycm_extra_conf.py -delete ;
	find ./ -name compile_commands.json -delete ;
	find ./ -name .clangd -exec rm -rv {} +

.PHONY: clean uninstall
