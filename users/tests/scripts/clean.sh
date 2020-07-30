#!/bin/bash

for i in `ls` ; do
  if ! test -d $i ; then
    continue ;
  fi
  if test $i == "scripts" ; then
    continue ;
  fi

  pushd ./ ;
  cd $i ;
  make clean ;
  popd ;
done
