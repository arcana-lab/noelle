#!/bin/bash

# Fetch the inputs
if test $# -lt 5 ; then
  echo "USAGE: `basename $0` REPO_DIR TEST_DIR NOELLE_OPTIONS PARALLELIZATION_OPTIONS MIDDLE_END_OPTIONS ERROR_FILE" ;
  exit 1;
fi
repoDir="$1" ;
testDir="$2" ;
noelleOptions="$3" ;
parallelizationOptions="$4" ;
frontendOptions="$5" ;
meOptions="$6" ;
errorFile="$7" ;

# Setup the environment
source ~/.bash_profile ;
cd $repoDir/ ;

# Go to the directory
cd $testDir ;

# Clean
make clean ;

echo "Machine = `hostname`" > node.txt ;

# Dump the script to run to re-produce the configuration
echo "#!/bin/bash" > run_me.sh ; 
echo "source ../../../enable ; " >> run_me.sh ;
echo "make clean ; " >> run_me.sh ;
echo "make FRONTEND_OPTIONS='$frontendOptions' PRE_MIDDLEEND_OPTIONS='$meOptions' NOELLE_OPTIONS='$noelleOptions' PARALLELIZATION_OPTIONS='$parallelizationOptions' ;" >> run_me.sh ;
chmod 744 run_me.sh ;

# Compile
timeout 3h make FRONTEND_OPTIONS="$frontendOptions" PRE_MIDDLEEND_OPTIONS="$meOptions" NOELLE_OPTIONS="$noelleOptions" PARALLELIZATION_OPTIONS="$parallelizationOptions" >> compiler_output.txt 2>&1 ;
if test $? -ne 0 ; then
  echo "ERROR: the following test did not pass because the compilation timed out" ;
  echo "  Test = `pwd`" ;
  echo "  Node = `hostname`" ;
  echo "$testDir $noelleOptions $parallelizationOptions $frontendOptions $meOptions" >> $errorFile ;
  exit 0 ;
fi

# Generate the input
make input.txt 

# Baseline
./baseline `cat input.txt` &> output_baseline.txt ;

# Test the parallelized binary
for i in `seq 0 5` ; do

  # Run the parallelized binary
  timeout 1h ./parallelized `cat input.txt` &> output_parallelized.txt ;
  if test $? -ne 0 ; then
    echo "ERROR: the following test did not pass because its parallel execution timed out" ;
    echo "  Test = `pwd`" ;
    echo "  Node = `hostname`" ;
    echo "$testDir $noelleOptions $parallelizationOptions $frontendOptions $meOptions" >> $errorFile ;
    exit 0 ;
  fi

  # Check the output generated
  cmp output_baseline.txt output_parallelized.txt ;
  if test $? -ne 0 ; then
    echo "ERROR: the test didn't pass" ;
    echo "$testDir $noelleOptions $parallelizationOptions $frontendOptions $meOptions" >> $errorFile ;
    exit 0;
  fi

done
