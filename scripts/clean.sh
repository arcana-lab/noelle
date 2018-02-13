#!/bin/bash

rm -rf */build ;

pushd ./ ;
cd tests ;
for i in `ls` ; do
  if ! test -d $i ; then
    continue ;
  fi
  if test "$i" == "." ; then
    continue ;
  fi

  pushd ./ ;
  cd $i ;
  make clean ;
  popd ;
done
popd ; 
	
