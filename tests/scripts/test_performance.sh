#!/bin/bash -e

function measureTime {
  local binaryName=$1 ;
  local outputFileName=$2 ;

  # Read input for arguments to performance runs
  local ARGS=$(< perf_args.info) ;

  # Create a temporary file
  tempFile=`mktemp` ;
  tempFile2=`mktemp` ;
  tempFile3=`mktemp` ;

  # Measure the execution times
  for j in `seq 0 5` ; do

    # Measure the time
    { time ./$binaryName $ARGS ; } &> $tempFile ;

    # Append the time
    local MATCHER="/real\t(.*)m(.*)s/" ;
    local PRINTER="{ print a[1] * 60 + a[2] }" ;
    local GAWK_CMD=" match(\$0, ${MATCHER}, a) ${PRINTER} " ;
    local timeMeasured=$(gawk "$GAWK_CMD" $tempFile) ;
    echo $timeMeasured >> $tempFile2 ;

  done
  
  # Sort the times
  sort -g $tempFile2 > $tempFile3 ;

  # Fetch the number of lines of the file
  lines=`wc -l $tempFile3 | awk '{print $1}'` ;

   # Compute the median
  median=`echo "$lines / 2" | bc` ;

  # Fetch the median
  result=`awk -v median=$median '
    BEGIN {
      c = 0;
    }{
      if (c == median){
        print ;
      }
      c++;
    }' $tempFile3` ;

  # Print the median
  echo "$result" > $outputFileName ;

  # Clean
  rm $tempFile ;
  rm $tempFile2 ;
  rm $tempFile3 ;

  return ;
}

function runningTests {
  echo $1 ;
  > $4 ;

  # Export autotuner specifications for parallelization
  export INDEX_FILE="autotuner.info" ;

  for i in `ls`; do
    if ! test -d $i ; then
      continue ;
    fi

    # Go to the test directory
    cd $i ;
    echo -e " Testing `basename $i` " ;

    # Clean
    # echo "   Make clean " ;
    make clean > /dev/null ; 

    # Compile
    # echo "   Make " ;
    make NOELLE_OPTIONS="$2" PARALLELIZATION_OPTIONS="$3" >> compiler_output.txt 2>&1 ;

    # Read input for arguments to performance runs
    local ARGS=$(< perf_args.info) ;

    # Measure the baseline
    if ! test -f time_baseline.txt ; then 
      echo -e "  Running baseline " ;
      measureTime baseline time_baseline.txt 
    fi
    local BASE=`cat time_baseline.txt` ;

    # Measure the parallelized binary
    echo -e "  Running performance " ;
    measureTime parallelized time_parallelized.txt
    local PAR=`cat time_parallelized.txt` ;

    cd ../ ;

    echo -ne "$i\\t" >> $4 ;
    local SPEEDUP=$(bc <<< " scale=3; $BASE / $PAR ") ;
    echo -e "  Speedup: $SPEEDUP" ;
    echo $SPEEDUP >> $4 ;
  done

  echo "Done"
}

export PATH=`pwd`/../install/bin:$PATH ;

# Run
cd performance ;
runningTests "Measuring the default configuration" "-noelle-verbose=3" " " "speedups.txt" ;

cd ../ ;

exit 0;
