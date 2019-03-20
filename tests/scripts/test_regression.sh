#!/bin/bash

function runningTestsWrapper {
  local optionsToUse="$@" ;

  if test "${optionsToUse}" == "" ; then
    runningTests "Testing the default configuration" "-noelle-verbose=3" ;
  else
    runningTests "Testing with \"${optionsToUse}\"" "-noelle-verbose=3 ${optionsToUse}" ;
  fi

  return ;
}

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
      echo -e "\e[31mFailed\e[0m" ;
      dirs_of_failed_tests="${i} ${dirs_of_failed_tests}" ;
    else
      passed_tests=`echo "$passed_tests + 1" | bc` ;
      echo -e "\e[32mPassed\e[0m" ;
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

runningTestsWrapper 

runningTestsWrapper -dswp-force -noelle-disable-helix ;
runningTestsWrapper -dswp-force -noelle-disable-helix -dswp-no-scc-merge ;

runningTestsWrapper -dswp-force -noelle-disable-dswp ;
runningTestsWrapper -dswp-force -noelle-disable-dswp -dswp-no-scc-merge ;

runningTestsWrapper -dswp-force -noelle-disable-doall ;
runningTestsWrapper -dswp-force -noelle-disable-doall -dswp-no-scc-merge ;

runningTestsWrapper -dswp-force -noelle-disable-doall -noelle-disable-helix ;
runningTestsWrapper -dswp-force -noelle-disable-doall -noelle-disable-helix -dswp-no-scc-merge ;

runningTestsWrapper -dswp-force -noelle-disable-doall -noelle-disable-dswp ;
runningTestsWrapper -dswp-force -noelle-disable-doall -noelle-disable-dswp -dswp-no-scc-merge ;

cd ../ ;

exit 0;
