#!/bin/bash

# Create a temporary file
tmpOut="`mktemp`" ;
tmpOut2="`mktemp`" ;

# Dump the set of tests that failed
make condor_check &> $tmpOut ;

# Fetch the list of tests that failed
grep regression_ $tmpOut &> $tmpOut2 ;
sed 's/\tr/r/g' $tmpOut2 &> $tmpOut ;

# Add the tests to the fail
cat $tmpOut >> regression/failing_tests ;

# Remove duplicates
sort -u regression/failing_tests > $tmpOut ;
sort $tmpOut > regression/failing_tests ;

# Clean
rm $tmpOut $tmpOut2;
