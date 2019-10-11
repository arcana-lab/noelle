#!/bin/bash

function runningTests {
  echo $1 ;
  > $3 ;

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
    make PARALLELIZATION_OPTIONS="$2" >> compiler_output.txt 2>&1 ;

    # Read input for arguments to performance runs
    local ARGS=$(< perf_args.info) ;

    # Capture times
    echo -e "  Running baseline " ;
    { time ./baseline $ARGS ; } &> time_baseline.txt ;
    echo -e "  Running performance " ;
    { time ./parallelized $ARGS ; } &> time_parallelized.txt ;

    local MATCHER="/real\t(.*)m(.*)s/" ;
    local PRINTER="{ print a[1] * 60 + a[2] }" ;
    local GAWK_CMD=" match(\$0, ${MATCHER}, a) ${PRINTER} " ;

    local BASE=$(gawk "$GAWK_CMD" time_baseline.txt) ;
    local PAR=$(gawk "$GAWK_CMD" time_parallelized.txt) ;

    cd ../ ;

    echo -ne "$i\\t" >> $3 ;
    local SPEEDUP=$(bc <<< " scale=3; $BASE / $PAR ") ;
    echo -e "  Speedup: $SPEEDUP" ;
    echo $SPEEDUP >> $3 ;
  done

  echo "Done"
}

export PATH=`pwd`/../install/bin:$PATH ;

cd performance ;
runningTests "Measuring the default configuration" "-noelle-verbose=0" "speedups.txt" ;
cd ../ ;

exit 0;

