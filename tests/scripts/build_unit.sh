#!/bin/bash -e

function buildSuite {
  if ! test -d $1 ; then
    return ;
  fi

  if [ ! -f $1/CMakeLists.txt ]; then
    return ;
  fi

  cd $1 ;

  echo -e " Building unit test suite: $i" ;
  rm -rf build/ ; 
  mkdir build ; 
  cd build ; 
  cmake3 -DCMAKE_BUILD_TYPE=Debug ../ ; 
  make ;
  cd ../

  cd ../
}

buildSuite $1 ;
