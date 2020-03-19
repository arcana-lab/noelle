#!/bin/bash

echo "Checking the regression test results" ;

# Local variables
origDir="`pwd`" ;
origDirLength=`echo "${#origDir} + 2" | bc` ;

# Fetch the local results
currentResults="`mktemp`" ; 
cat regression*/*.txt  | sort | cut -c ${origDirLength}- > $currentResults ;

# Compare with the known one
newTestsFailed="" ;
while IFS= read -r line; do
  grep -Fxq "$line" failing_tests.txt ;
  if test $? -ne 0 ; then
    newTestsFailed="${newTestsFailed} $line" ;
  fi
done < "$currentResults"

# Check the results
if test "$newTestsFailed" == "" ; then
  echo "The regression tests passed" ;
  oldTestsNumber=`wc -l failing_tests.txt | awk '{print $1}'` ;
  newTestsNumber=`wc -l $currentResults | awk '{print $1}'` ;
  if test ${newTestsNumber} == ${oldTestsNumber} ; then
    echo "  All tests that failed before still fail" ;
  else
    lessTests=`echo "${oldTestsNumber} - ${newTestsNumber}" | bc` ;
    echo "  There are $lessTests less tests that fail now!" ;
  fi
  exit 0;
fi

echo "  New tests failed: $newTestsFailed" ;
