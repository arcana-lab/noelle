#!/bin/bash -e

# Fetch the inputs
if test $# -lt 1 ; then
  echo "USAGE: `basename $0` UNITTEST_DIR" ;
  exit 1;
fi
testDir="$1" ;

# Setup the environment
source ~/.bash_profile ;

# Go to the directory
cd $testDir ;

# Clean
make clean ;

# Compile ;
make -j ;

# Run
make run >> compiler_output.txt 2>&1 ;
