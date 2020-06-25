#!/bin/bash

RED='\033[1;31m' ;
GREEN='\033[0;32m' ;
NC='\033[0m' ;

trim() {
  local s2 s="$*"
  until s2="${s#[[:space:]]}"; [ "$s2" = "$s" ]; do s="$s2"; done
  until s2="${s%[[:space:]]}"; [ "$s2" = "$s" ]; do s="$s2"; done
  echo "$s"
}

function identifyElementsOutsideSet {
  local setFile="$1" ;
  local elementsToCheckFile="$2" ;

  outsideElements="" ;
  while IFS= read -r line; do
    line=$(trim "$line") ;
    grep -Fxq "$line" $setFile ;
    if test $? -ne 0 ; then
      outsideElements="${outsideElements}\n\t$line" ;
    fi
  done < "$elementsToCheckFile" 

  return 0;
}

echo "################################### REGRESSION TESTS:" ;
echo "  Checking the regression test results" ;

# Check the tests that are still running
regressionFinished="0" ;
stillRunning="`mktemp`" ;
condor_q `whoami` -l | grep ^Arguments | grep "`pwd`" | grep regression > $stillRunning ;
if test -s $stillRunning ; then
  echo "    The following tests are still running" ;
  while IFS= read -r line; do
    testRunning=`echo $line | awk '{print $4}'` ;
    echo "      $testRunning" ;
  done < "$stillRunning"

else
  echo "    All tests finished" ;
  regressionFinished="1" ;
fi

# Local variables
origDir="`pwd`" ;
origDirLength=`echo "${#origDir} + 2" | bc` ;

# Fetch the local results
currentResultsToTrim="`mktemp`" ; 
cat regression*/*.txt  | sort | cut -c ${origDirLength}- > $currentResultsToTrim ;
currentResults="`mktemp`" ; 
while IFS= read -r line; do
  line=$(trim "$line") ;
  echo "$line" >> $currentResults ;
done < "$currentResultsToTrim"
rm $currentResultsToTrim ;

# Compare with the known one
newTestsFailed="" ;
while IFS= read -r line; do
  line=$(trim "$line") ;
  grep -Fxq "$line" regression/failing_tests ;
  if test $? -ne 0 ; then
    newTestsFailed="${newTestsFailed}\n\t$line" ;
  fi
done < "$currentResults"

# Check the results
if test "$newTestsFailed" != "" ; then
  echo -e "    New tests ${RED}failed${NC}: $newTestsFailed" ;
  echo -e "    The regression tests ${RED}failed${NC}" ;

else

  # The regression passed
  # 
  # Check if there are tests that use to fail that now pass
  oldTestsNumber=`wc -l regression/failing_tests | awk '{print $1}'` ;
  newTestsNumber=`wc -l $currentResults | awk '{print $1}'` ;
  if test ${newTestsNumber} == ${oldTestsNumber} ; then
    echo "    All tests that failed before still fail" ;
  else
    lessTests=`echo "${oldTestsNumber} - ${newTestsNumber}" | bc` ;
    echo "    There are $lessTests less tests that fail now!" ;

    # Print the tests that now pass
    echo "    These tests are the following ones:" ;
    identifyElementsOutsideSet $currentResults regression/failing_tests ;
    echo -e "$outsideElements" ;
  fi

  # Check if there are still running tests
  if test $regressionFinished == "1" ; then
    echo "" ;
    echo -e "  The regression tests passed ${GREEN}succesfully${NC}" ;
  fi
fi
echo "" ;
echo "" ;

# Check the unit tests
echo "################################### UNIT TESTS:" ;
if ! test -f unit/compiler_output.txt ;then
  echo "  They are still running" ;

else
  fails=`grep Failures unit/compiler_output.txt | awk '
    BEGIN {
      f = 0;
    } {
      f += $8 ;
    } END {
      printf("%d\n", f);
    }' `;
  if ! test $fails == "0" ; then
    echo -e "  $fails tests ${RED}failed${NC}" ;
  else
    echo -e "  All unit tests ${GREEN}succeded${NC}" ;
  fi
fi
echo "" ;
echo "" ;

# Check the performance tests
echo "################################### PERFORMANCE TESTS:" ;
grep -i error compiler_output_performance.txt &> /dev/null ;
if test $? -eq 0 ; then
  echo -e "  At least one performance test ${RED}failed${NC} to compile" ;
  
else 

  # Check if they are still running
  if test -f performance/speedups.txt ; then
    linesRunning=`wc -l performance/speedups.txt | awk '{print $1}'` ;
    linesOracle=`wc -l performance/oracle_speedups | awk '{print $1}'` ;
    if test "$linesOracle" != "$linesRunning" ; then
      echo "  They are still running" ;

    else
      echo "  All performance tests compiled correctly" ;
      grep -i "Performance degradation" compiler_output_performance.txt &> /dev/null ;
      if test $? -eq 0 ; then
        echo -e "  Next are the performance tests that run ${RED}slower${NC}:" ;
        grep -i "Performance degradation" compiler_output_performance.txt ;
      else 
        echo -e "  All performance tests ${GREEN}succeded!${NC}" ;
      fi
    fi

  else 
    echo "  They are still running" ;
  fi
fi

# Clean 
rm $currentResults ;
