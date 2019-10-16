#!/bin/bash -e

CC="clang++"
TRANSFORMATIONS_BEFORE_PARALLELIZATION="-basicaa -mem2reg"

UNIT_TEST_PASS=" \
  -load ../../../helpers/build/UnitTestHelpers.so \
  -load ../../build/UnitTester.so -UnitTester \
"

function runTest {
  if ! test -d $2 ; then
    return ;
  fi
  if ! test -f $2/test.cpp ; then
    return ;
  fi

  cd $2 ;

  echo -e "$1: Testing $2 " ;

  rm -f *.bc *.ll *.s *.o ;

  ${CC} -std=c++14 -emit-llvm -O0 -Xclang -disable-O0-optnone -c test.cpp -o test_pre.bc
  opt ${TRANSFORMATIONS_BEFORE_PARALLELIZATION} test_pre.bc -o test.bc
  llvm-dis test.bc -o test.ll
  noelle-load ${UNIT_TEST_PASS} test.bc -o tested.bc

  cd ../ ;
}

function checkRunSuite {
  if ! test -d $1 ; then
    return ;
  fi
  if ! test -d $1/suite ; then
    return ;
  fi

  cd $1/suite/ ;

  if ! test -z "$2" ; then
    runTest $1 $2
  else
    for i in `ls`; do
      runTest $1 $i ;
    done
  fi

  cd ../../ ;
}

checkRunSuite $1 $2 ;
