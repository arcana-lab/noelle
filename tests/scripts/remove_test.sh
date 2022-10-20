#!/bin/bash

# Fetch the inputs
if test $# -lt 1 ; then
  echo "USAGE: `basename $0` TEST_TO_REMOVE" ;
  exit 1;
fi
testToRemove="$1" ;

# Temporary file
tmpFile="`mktemp`" ;

while read p; do
  testName=`echo "$p" | awk '{print $1}' | sed s/regression_[0-9]*//g` ;
  if test "$testName" == "/$testToRemove" ; then
    continue ;
  fi
  echo "$p" >> $tmpFile ;

done < regression/failing_tests ;

# Swap the files
cp $tmpFile regression/failing_tests ;

# Clean
rm $tmpFile ;
