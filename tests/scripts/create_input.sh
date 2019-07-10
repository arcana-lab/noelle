#!/bin/bash -e

if test $# -lt 1 ; then
  echo "USAGE: `basename $0` FILE_TO_GENERATE" ;
  exit 1;
fi
outFile="$1" ;

if test -f test_args.info ; then 
  cp test_args.info $outFile ; 
else 
  echo "20 20 20" > $outFile ;
fi
