#!/bin/bash

function linkParUtils {
  cd $1 ;

  for i in `ls`; do
    if ! test -d "$i" ; then
      continue ;
    fi
    cd $i ;
    echo "  $i" ;
    if ! test -f Parallelizer_utils.cpp ; then
      ln -s ../../../src/runtime/Parallelizer_utils.cpp ;
    fi
    if ! test -f Makefile ; then
      ln -s ../../scripts/Makefile ;
    fi
    cd ../ ;
  done

  cd ../ ;
}

echo "Adding symbolic links for regression tests" ;
linkParUtils regression
echo "Adding symbolic links for performance tests" ;
linkParUtils performance
