#!/bin/bash -e

# Fetch the inputs
if test $# -lt 1 ; then
  echo "USAGE: `basename $0` NUM" ;
  exit 1;
fi
num=$1 ;
echo "Running the parallelized version $num times" ;

# Run the baseline
./baseline `cat input.txt` &> output_baseline.txt ;

# Run the parallelized version
for i in `seq 1 $num` ; do
  ./parallelized `cat input.txt` &> output_parallelized.txt ;
  cmp output_baseline.txt output_parallelized.txt ;
  if test $? -ne 0 ; then
    echo "Error $$?: the parallelized version has generated an incorrect output" ;
    exit 1;
  fi

  sleep 2; 
done

echo "Success"
