#!/bin/bash -e

for i in `ls` ; do
  if ! test -d "$i" ; then
    continue ;
  fi
  pushd ./ ;

  cd $i ;
  ./scripts/run_me.sh ;

  popd ;
done
