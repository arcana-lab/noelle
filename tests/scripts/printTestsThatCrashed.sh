#!/bin/bash

myName="`whoami`" ;

for i in `ls regression_*/errors.txt` ; do
  while IFS="" read -r p || [ -n "$p" ] ; do

    # Fetch the name of the test
    failedTest="`echo \"$p\" | awk '{print $1}'`"

    # Check if the test failed
    pushd ./ &> /dev/null ;
    cd $failedTest ;
    if test -e parallelized ; then
      popd &> /dev/null ;
      continue ;
    fi

    # Check if the test timeout
    grep Terminated compiler_output.txt &> /dev/null ;
    if test $? -eq 0 ; then
      popd &> /dev/null ;
      continue ;
    fi
    grep Killed compiler_output.txt &> /dev/null ;
    if test $? -eq 0 ; then
      popd &> /dev/null ;
      continue ;
    fi

    # The test failed
    echo "Test $failedTest" ;

    popd &> /dev/null ;
  done < $i ;
done
