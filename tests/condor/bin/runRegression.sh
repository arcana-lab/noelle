#!/bin/bash

# Fetch the inputs
if test $# -lt 5 ; then
  echo "USAGE: `basename $0` REPO_DIR TEST_DIR NOELLE_OPTIONS PARALLELIZATION_OPTIONS ERROR_FILE" ;
  exit 1;
fi
repoDir="$1" ;
testDir="$2" ;
noelleOptions="$3" ;
parallelizationOptions="$4" ;
errorFile="$5" ;

# Setup the environment
source ~/.bash_profile ;
cd $repoDir/ ;

# Go to the directory
cd $testDir ;

# Clean
make clean ;

# Compile
make NOELLE_OPTIONS="$noelleOptions" PARALLELIZATION_OPTIONS="$parallelizationOptions" >> compiler_output.txt 2>&1 ;

# Generate the input
make input.txt 

# Baseline
./baseline `cat input.txt` &> output_baseline.txt ;

# Transformation
timeout 10m ./parallelized `cat input.txt` &> output_parallelized.txt ;
if test $? -ne 0 ; then
  echo "ERROR: the test didn't pass because it timed out" ;
  echo "$testDir $noelleOptions $parallelizationOptions" >> $errorFile ;
  exit 0 ;
fi

# Check the output ;
cmp output_baseline.txt output_parallelized.txt ;
if test $? -ne 0 ; then
  echo "ERROR: the test didn't pass" ;
  echo "$testDir $noelleOptions $parallelizationOptions" >> $errorFile ;
fi
