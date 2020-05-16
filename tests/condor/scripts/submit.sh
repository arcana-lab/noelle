#!/bin/bash

export PATH=`realpath ../../install/bin`:${PATH} ; 

for i in `ls | grep .con` ; do
  echo $i ;
  condor_submit $i ;
done
