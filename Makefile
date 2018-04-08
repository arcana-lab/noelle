all:
	cd passes ; make ; cd ../tests ; make ;

clean:
	cd passes ; make clean ; cd ../tests ; make clean; 
