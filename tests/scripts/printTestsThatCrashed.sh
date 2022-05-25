#!/bin/bash -e

myName="`whoami`" ;

for i in `ls | grep regression_` ; do
  if ! test -d $i ; then
    continue ;
  fi
  pushd ./ &> /dev/null ;
  cd $i ;

  for j in `ls` ; do
    if ! test -d $j ; then
      continue ;
    fi
    pushd ./ &> /dev/null ;
    cd $j ;
    
    # Check if the parallel binary exists
    if test -e parallelized ; then
      popd &> /dev/null ;
      continue ;
    fi

    # Check if the test is still running
    testName=`condor_q ${myName} -l | grep regression_ | grep "${i}/${j}" | grep Argum | awk '{print $4}'` ;
    if test "$testName" != "" ; then
      popd &> /dev/null ;
      continue ;
    fi

    # Check if the test aborted or seg faulted
    grep -i Aborted compiler_output.txt &> /dev/null ;
    if test $? -eq 0 ; then
      echo "Aborted: `pwd`" ;
      popd &> /dev/null ;
      continue ;
    fi
 
    # Check if the test seg faulted
    grep -i fault compiler_output.txt &> /dev/null ;
    if test $? -eq 0 ; then
      echo "Seg faulted: `pwd`" ;
      popd &> /dev/null ;
      continue ;
    fi

    popd &> /dev/null ;
  done

  popd &> /dev/null ;
done
