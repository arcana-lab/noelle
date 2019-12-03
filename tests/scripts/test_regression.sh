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
 
  # Compute the number of tests
  numOfTests="0" ;
  for i in `ls`; do
    if ! test -d $i ; then
      continue ;
    fi
    numOfTests=`echo "$numOfTests + 1" | bc` ;
  done

  currentTest="0" ; 
  for i in `ls`; do
    if ! test -d $i ; then
      continue ;
    fi
    checked_tests=`echo "$checked_tests + 1" | bc` ;

    # Go to the test directory
    cd $i ;
    echo -n -e "\r   Successes $passed_tests : Testing $currentTest / $numOfTests : `basename $i`                                                 " ;

    # Clean
    make clean > /dev/null ; 

    # Compile
    make PARALLELIZATION_OPTIONS="$2" >> compiler_output.txt 2>&1 ;
    
    # Generate the input
    make input.txt &> /dev/null ;

    # Baseline
    ./baseline `cat input.txt` &> output_baseline.txt ;

    # Transformation
    ./parallelized `cat input.txt` &> output_parallelized.txt ;

    # Check the output ;
    cmp output_baseline.txt output_parallelized.txt &> /dev/null ;
    if test $? -ne 0 ; then
      dirs_of_failed_tests="${i} ${dirs_of_failed_tests}" ;
    else
      passed_tests=`echo "$passed_tests + 1" | bc` ;
    fi

    currentTest=`echo "$currentTest + 1" | bc` ;
    cd ../ ;
  done

  # Print the results
  echo -n -e "\r   Tests passed: ${passed_tests} / ${checked_tests}                                                                   " ;
  echo "" ;
  if test "${dirs_of_failed_tests}" != "" ; then
    echo "    Tests failed: ${dirs_of_failed_tests}" ;
  fi
  echo "" ;
}

export PATH=`pwd`/../install/bin:$PATH

cd regression ;

# Test enablers
runningTestsWrapper -noelle-disable-helix -noelle-disable-dswp -noelle-disable-doall ;

# Test parallelization techniques
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
