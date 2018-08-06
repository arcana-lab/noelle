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
  if ! test -f Parallelizer_utils.cpp ; then
    ln -s ../scripts/Parallelizer_utils.cpp ;
  fi
  if ! test -f Makefile ; then
    ln -s ../scripts/Makefile ;
  fi
  cd ../ ;
done
