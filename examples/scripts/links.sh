#!/bin/bash

for i in `ls` ; do
  if ! test -d "$i" ; then
    continue ;
  fi
  pushd ./ ;

  cd $i ;

  # Prepare the links
  if ! test -e scripts ; then
    ln -s ../../template/scripts 
  fi
  if ! test -e CMakeLists.txt ; then
    ln -s ../../template/CMakeLists.txt ;
  fi

  popd ;
done
