#!/bin/bash

function runningTests {
  echo $1 ;

  local checked_tests=0 ;
  local passed_tests=0 ;
  local dirs_of_failed_tests="" ;
  
  for i in `ls`; do
    if ! test -d $i ; then
      continue ;
    fi
    checked_tests=`echo "$checked_tests + 1" | bc` ;

    # Go to the test directory
    cd $i ;
    echo -n "   Testing `basename $i` " ;

    # Clean
    make clean > /dev/null ; 

    # Compile
    make PARALLELIZATION_OPTIONS="$2" >> compiler_output.txt 2>&1 ;

    # Baseline
    ./baseline 2 2 2 &> output_baseline.txt ;

    # Transformation
    ./parallelized 2 2 2 &> output_parallelized.txt ;

    # Check the output ;
    cmp output_baseline.txt output_parallelized.txt &> /dev/null ;
    if test $? -ne 0 ; then
      echo "Failed" ;
      dirs_of_failed_tests="${i} ${dirs_of_failed_tests}" ;
    else
      passed_tests=`echo "$passed_tests + 1" | bc` ;
      echo "Passed" ;
    fi

    cd ../ ;
  done

  # Print the results
  echo "    Tests passed: ${passed_tests} / ${checked_tests}" ;
  if test "${dirs_of_failed_tests}" != "" ; then
    echo "    Tests failed: ${dirs_of_failed_tests}" ;
  fi
  echo "" ;
}

cd regression ;
runningTests "Testing the default configuration" "-noelle-verbose=3" ;

runningTests "Testing with \"-dswp-force -noelle-disable-helix\"" "-noelle-verbose=3 -dswp-force -noelle-disable-helix" ;
runningTests "Testing with \"-dswp-force -dswp-no-scc-merge -noelle-disable-helix\"" "-noelle-verbose=3 -dswp-force -dswp-no-scc-merge -noelle-disable-helix" ;

runningTests "Testing with \"-dswp-force -noelle-disable-dswp\"" "-noelle-verbose=3 -dswp-force -noelle-disable-dswp" ;
runningTests "Testing with \"-dswp-force -dswp-no-scc-merge -noelle-disable-dswp\"" "-noelle-verbose=3 -dswp-force -dswp-no-scc-merge -noelle-disable-dswp" ;
cd ../ ;

exit 0;
