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

function printTestsThatDoNotFailAnymore {

  tmpFileTests="`mktemp`" ;
  tmpFileTests2="`mktemp`" ;
  ./scripts/printUniqueTests.sh regression/failing_tests > $tmpFileTests ;
  ./scripts/printUniqueTestsThatFailed.sh > $tmpFileTests2 ;
  cmp $tmpFileTests $tmpFileTests2 &> /dev/null ;
  if test $? -ne 0 ; then
    echo "    There are new tests that now pass for all configurations. They are the next ones:" ;
    identifyElementsOutsideSet $tmpFileTests2 $tmpFileTests ;
    echo -e "$outsideElements" ;
  fi
  rm $tmpFileTests ;
  rm $tmpFileTests2 ;

  return 0;
}

echo "################################### REGRESSION TESTS:" ;
echo "  Checking the regression test results" ;

# Check the tests that are still running
regressionFinished="0" ;
stillRunning="`mktemp`" ;
condor_q `whoami` -l | grep ^Arguments | grep "`pwd`" | grep regression > $stillRunning ;
stillRunningRegressionTests="0";
if test -s $stillRunning ; then
  stillRunningJobs=`wc -l $stillRunning | awk '{print $1}'` ;
  echo "    There are $stillRunningJobs jobs that are still running" ;
  stillRunningRegressionTests=`echo "$stillRunningJobs < 20 | bc"` ;
  if test "$stillRunningRegressionTests" == "1" ; then
    echo "    The running jobs are the following ones:" ;
    while IFS= read -r line; do
      testRunning=`echo $line | awk '{print $4}'` ;
      echo "        $testRunning" ;
    done < "$stillRunning"
  fi

else
  echo "    All tests finished" ;
  regressionFinished="1" ;
fi

# Local variables
origDir="`pwd`" ;
origDirLength=`echo "${#origDir} + 2" | bc` ;

# Fetch the local results
currentResultsToTrim="`mktemp`" ; 
cat regression*/*.txt 2>>/dev/null | sort | cut -c ${origDirLength}- > $currentResultsToTrim ;
currentResults="`mktemp`" ; 
while IFS= read -r line; do
  line=$(trim "$line") ;
  echo "$line" >> $currentResults ;
done < "$currentResultsToTrim"
rm $currentResultsToTrim ;

# Compare with the known one
newTestsFailed="" ;
newTestsFailedCounter="0" ;
while IFS= read -r line; do
  line=$(trim "$line") ;
  grep -Fxq "$line" regression/failing_tests ;
  if test $? -ne 0 ; then
    newTestsFailed="${newTestsFailed}\n\t$line" ;
    newTestsFailedCounter=`echo "$newTestsFailedCounter + 1" | bc` ;
  fi
done < "$currentResults"

# Check the results
if test "$newTestsFailed" != "" ; then
  echo -e "    $newTestsFailedCounter new tests ${RED}failed${NC}: $newTestsFailed" ;
  echo -e "    The regression tests ${RED}failed${NC}" ;

elif test "$stillRunningRegressionTests" == "0" ; then

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

    # Print tests that completely pass for all configurations now
    printTestsThatDoNotFailAnymore ;
  fi

  # Check if there are still running tests
  if test $regressionFinished == "1" ; then
    echo "" ;
    echo -e "  The regression tests passed ${GREEN}succesfully${NC}" ;
  fi

else
  echo "    No new tests failed so far" ;
  
  # Print tests that completely pass for all configurations now
  printTestsThatDoNotFailAnymore ;
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
fi

# Check if they are still running
if test -f performance/speedups.txt ; then
  linesRunning=`wc -l performance/speedups.txt | awk '{print $1}'` ;
  linesOracle=`wc -l performance/oracle_speedups | awk '{print $1}'` ;
  if test "$linesOracle" != "$linesRunning" ; then
    echo "  They are still running" ;

  else
    tempSpeedups=`mktemp` ;
    tempOracle=`mktemp` ;
    tempCompare=`mktemp` ;
    tempOutput=`mktemp` ;
    sort performance/speedups.txt > $tempSpeedups ;
    sort performance/oracle_speedups > $tempOracle ;
    paste $tempSpeedups $tempOracle > $tempCompare ;
    awk '
      {
        if (  ($2 < ($4 * 0.9)) && (($4 - $2) > 0.3)   ){
          printf("    Performance degradation for %s (from %.1fx to %.1fx)\n", $1, $4, $2);
        }
      }' $tempCompare > $tempOutput ;
    grep -i "Performance degradation" $tempOutput &> /dev/null ;
    if test $? -eq 0 ; then
      echo -e "  Next are the performance tests that run ${RED}slower${NC}:" ;
      grep -i "Performance degradation" $tempOutput ;

    else 
      echo -e "  All performance tests ${GREEN}succeded!${NC}" ;
      awk '{
        if (     (($2 > ($4 * 1.1)) && (($2 - $4) > 0.2)) || (($2 - $4) >= 1) ){
              printf("    Performance increase for %s (from %.1fx to %.1fx)\n", $1, $4, $2);
            }
        }' $tempCompare > $tempOutput ;
      cat $tempOutput ;
    fi

    # Remove the files
    rm $tempOracle ;
    rm $tempSpeedups ;
    rm $tempCompare ;
    rm $tempOutput ;
  fi

else 
  echo "  They are still running" ;
fi

# Clean 
rm $currentResults ;
