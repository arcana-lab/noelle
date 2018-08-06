#!/bin/bash

echo "Adding symbolic links" ;
for i in `ls`; do
  if ! test -d "$i" ; then
    continue ;
  fi
  if test "$i" == "scripts" ; then
    continue ;
  fi
  cd $i ;
  echo "  $i" ;
  ln -s ../scripts/Parallelizer_utils.cpp ;
  ln -s ../scripts/Makefile ;
  cd ../ ;
done
