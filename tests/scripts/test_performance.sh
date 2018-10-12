#!/bin/bash

function runningTests {
  echo $1 ;
  > $3 ;

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

    # Capture output only
    # echo "   Small run " ;
    ./baseline 10 &> output_baseline.txt ;
    ./parallelized 10 &> output_parallelized.txt ;

    # Check the output to confirm correctness before measuring performance
    cmp output_baseline.txt output_parallelized.txt &> /dev/null ;
    if test $? -ne 0 ; then
      echo "Performance test failed" ;
      return 1 ;
    fi

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

cd performance ;
runningTests "Measuring with \"-dswp-force -dswp-no-scc-merge\"" "-dswp-verbose=0 -dswp-force -dswp-no-scc-merge" "speedups-no-merge.txt" ;
runningTests "Measuring with \"-dswp-force\"" "-dswp-verbose=0 -dswp-force" "speedups-force.txt" ;
runningTests "Measuring the default configuration" "-dswp-verbose=0" "speedups.txt" ;
cd ../ ;

exit 0;

