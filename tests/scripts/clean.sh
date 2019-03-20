#!/bin/bash

function cleanTests {
  cd $1 ;

  for i in `ls`; do
    if ! test -d $i ; then
      continue ;
    fi

    cd $i ;
    if [ -f artifacts ] ; then
      cat artifacts | xargs rm 
      rm artifacts
    fi 
    make clean ;
    rm -f *_utils.cpp Makefile ;
    cd ../ ;
  done

  cd ../ ;
}

./scripts/add_symbolic_link.sh ;

cleanTests regression
cleanTests performance

cleanTests oracle_aa
cleanTests commutativity

# Remove speedup info on performance tests
cd performance ;
rm -f *.txt ;
cd ../ ;
