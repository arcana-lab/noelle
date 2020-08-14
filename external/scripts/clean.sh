#!/bin/bash

if test -d scaf ; then 
  cd scaf ;
	make clean ;
fi

if ! test -d svf ; then 
  exit 0;
fi
  
cd svf ; 
make clean ; 
