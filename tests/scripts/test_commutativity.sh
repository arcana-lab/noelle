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
    make baseline_comm >> compiler_output.txt 2>&1 ;

    # Check the output ;
    output=0-oracle-ddg.dep
    answer=test.aa
    cmp $output $answer &>> test_output.txt 2>&1  ;
    if test $? -ne 0 ; then
      echo -e "\e[31mFailed\e[0m" ;
      echo "-- Printing result on .dep and correct answer --"
      diff -y $output $answer
      echo "--------"
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

cd commutativity ;
runningTests 
cd ../ ;

exit 0;
