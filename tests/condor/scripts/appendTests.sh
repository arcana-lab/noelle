#!/bin/bash

condorFile=`realpath $1` ;

cd ../regression ; 
for i in `ls` ; do
  if ! test -d $i ; then
    continue ;
  fi

  testName=`basename $i` ;
  echo "Test = $testName" >> "$condorFile" ;
  echo "Queue" >> "$condorFile" ;
done
