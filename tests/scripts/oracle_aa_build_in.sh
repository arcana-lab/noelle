#!/bin/bash

function buildFromCpp {
  cpp_file=test.cpp
  cd $1
  echo "Compiling $1/$cpp_file to test.in" ;

  make ir_mark.bc
  mv ir_mark.bc test.in
  make clean

}

cd oracle_aa 
buildFromCpp $1
cd ../

exit 0
