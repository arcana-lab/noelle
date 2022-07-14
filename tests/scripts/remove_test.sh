#!/bin/bash

testToRemove="$1" ;

while read p; do
  testName=`echo "$p" | awk '{print $1}' | sed s/regression_[0-9]*//g` ;
  if test "$testName" == "/$testToRemove" ; then
    continue ;
  fi
  echo "$p" ;

done < regression/failing_tests ;
