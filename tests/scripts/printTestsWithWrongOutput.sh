#!/bin/bash

myName="`whoami`" ;

# Print tests that generated a non-empty wrong output
echo "Print tests that generated a non-empty wrong output that didn't timeout" ;
for i in `ls regression_*/errors.txt` ; do
  while IFS="" read -r p || [ -n "$p" ] ; do

    # Fetch the name of the test
    failedTest="`echo \"$p\" | awk '{print $1}'`"

    # Check if the test compiled
    pushd ./ &> /dev/null ;
    cd $failedTest ;
    if ! test -e parallelized ; then
      popd &> /dev/null ;
      continue ;
    fi

    # Check if the output is available
    if ! test -e output_baseline.txt ; then
      popd &> /dev/null ;
      continue ;
    fi
    if ! test -e output_parallelized.txt ; then
      popd &> /dev/null ;
      continue ;
    fi

    # Check if the output file is empty
    if ! test -s output_parallelized.txt ; then
      popd &> /dev/null ;
      continue ;
    fi

    # Check if a timeout happened
    grep timeout output_parallelized.txt &> /dev/null ;
    if test $? -eq 0 ; then
      popd &> /dev/null ;
      continue ;
    fi

    # Compare the output
    cmp output_baseline.txt output_parallelized.txt &> /dev/null ;
    if test $? -ne 0 ; then

      # The test failed
      echo "  Test $failedTest" ;
    fi

    popd &> /dev/null ;
  done < $i ;
done
