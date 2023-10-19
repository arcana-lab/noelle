#!/bin/bash

myName=`whoami` ;
origDir=`pwd` ;

testList="`condor_q $myName -l | grep $origDir | grep regression_ | grep Argum | awk '{print $4}' | sed 's/.*tests\/regression/regression/g'`" ;

for t in $testList ; do
  if test -f ${t}/parallelized ; then
    continue ;
  fi

  echo $t
done
