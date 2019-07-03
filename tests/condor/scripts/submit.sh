#!/bin/bash

for i in `ls | grep .con` ; do
  echo $i ;
  condor_submit $i ;
done
