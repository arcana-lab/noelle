#!/bin/bash

function cleanTests {
  cd $1 ;

  for i in `ls`; do
    if ! test -d $i ; then
      continue ;
    fi

    cd $i ;
    make clean ;
    rm -f *_utils.cpp Makefile ;
    cd ../ ;
  done

  cd ../ ;
}

./scripts/add_symbolic_link.sh ;

cleanTests regression
cleanTests performance

# Remove speedup info on performance tests
cd performance ;
rm -f *.txt ;
cd ../ ;

# Clean unit tests
cd unit 
find ./ -iname *.bc -delete ;
find ./ -iname *.ll -delete ;
rm -rf */build ;
cd ../ ;
