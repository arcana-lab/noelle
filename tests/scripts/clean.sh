#!/bin/bash

function cleanTests {
  cd $1 ;

  rm -f *.txt ;

  for i in `ls`; do
    if ! test -d $i ; then
      continue ;
    fi

    cd $i ;
    make clean ;
    rm -f NOELLE_APIs.* *_utils.cpp Makefile *.log *.dot ;
    cd ../ ;
  done

  cd ../ ;
}

cleanTests unit

# Clean unit tests
cd unit 
find ./ -iname *.bc -delete ;
find ./ -iname *.ll -delete ;
find ./ -iname *prof* -delete ;
rm -rf */build ;
cd ../ ;
