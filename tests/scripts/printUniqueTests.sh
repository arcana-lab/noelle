#!/bin/bash

if test $# -lt 1 ; then
  echo "USAGE: `basename $0` FILE" ;
  exit 1;
fi
fileName="$1" ;

awk '{print $1}' $fileName | sed s/regression_[0-9]*[/]//g | sort -u 
