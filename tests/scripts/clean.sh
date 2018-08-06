#!/bin/bash

./scripts/add_symbolic_link.sh ;

for i in `ls`; do
  if ! test -d $i ; then
    continue ;
  fi
  if test $i == "scripts" ; then
    continue ;
  fi
  if test $i == "include" ; then
    continue ;
  fi

  cd $i ;
  make clean ;
  rm -f *_utils.cpp Makefile ;
  cd ../ ;
done
