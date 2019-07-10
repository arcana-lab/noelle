#!/bin/bash

# Fetch the inputs
if test $# -lt 4 ; then
  echo "USAGE: `basename $0` REPO_DIR TEST_DIR PARALLELIZATION_OPTIONS ERROR_FILE" ;
  exit 1;
fi
repoDir="$1" ;
testDir="$2" ;
parallelizationOptions="$3" ;
errorFile="$4" ;

# Setup the environment
source ~/.bash_profile ;
cd $repoDir/ ;

# Go to the directory
cd $testDir ;

# Clean
make clean ;

# Compile
make PARALLELIZATION_OPTIONS="$parallelizationOptions" >> compiler_output.txt 2>&1 ;

# Generate the input
make input.txt 

# Baseline
./baseline `cat input.txt` &> output_baseline.txt ;

# Transformation
./parallelized `cat input.txt` &> output_parallelized.txt ;

# Check the output ;
cmp output_baseline.txt output_parallelized.txt ;
if test $? -ne 0 ; then
  echo "ERROR: the test didn't pass" ;
  echo "$testDir $parallelizationOptions" >> $errorFile ;
fi
