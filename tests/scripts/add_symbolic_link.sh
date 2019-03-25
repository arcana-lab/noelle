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
      ln -s ../../scripts/Parallelizer_utils.cpp ;
    fi
    cd ../ ;
  done

  cd ../ ;
}


function linkMakefile {
  cd $1 ;

  for i in `ls`; do
    if ! test -d "$i" ; then
      continue ;
    fi
    cd $i
    if ! test -f Makefile ; then
      ln -s ../../scripts/Makefile Makefile ;
    fi
    cd ../ ;
  done

  cd ../
}

pwd

echo "Adding symbolic links for regression tests" ;
linkParUtils regression
linkMakefile regression
pwd
echo "Adding symbolic links for oracle_aa tests"  ;
linkParUtils oracle_aa
linkMakefile oracle_aa
pwd
echo "Adding symbolic links for performance tests" ;
linkParUtils performance
linkMakefile performance

echo "Adding symbolic links for commutativity tests"  ;
linkParUtils commutativity
linkMakefile commutativity
