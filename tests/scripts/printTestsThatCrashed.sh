#!/bin/bash -e

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

    # The test failed
    echo "Test $failedTest" ;

    popd &> /dev/null ;
  done < $i ;
done
