#!/bin/bash

if ! test -d svf ; then 
  exit 0;
fi
  
cd svf ; 
make clean ; 
