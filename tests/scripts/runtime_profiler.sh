#!/bin/bash

# Fetch the inputs
if test $# -lt 2 ; then
  echo "USAGE: `basename $0` BASELINE_OUTPUT RUN2_OUTPUT" ;
  exit 1;
fi

setup1=`grep overhead $1 | grep Setup | awk '{c += $5} END {print c}'` ;
setup2=`grep overhead $2 | grep Setup | awk '{c += $5} END {print c}'` ;
delta=`echo "scale=3; (($setup2 - $setup1) / $setup1) * 100" | bc`;
echo "Setup overhead = $delta %" ;

forking1=`grep overhead $1 | grep Task | awk '{c += $6} END {print c}'` ;
forking2=`grep overhead $2 | grep Task | awk '{c += $6} END {print c}'` ;
delta=`echo "scale=3; (($forking2 - $forking1) / $forking1) * 100" | bc`;
echo "Forking overhead = $delta %" ;

joining1=`grep overhead $1 | grep Joining | awk '{c += $5} END {print c}'`;
joining2=`grep overhead $2 | grep Joining | awk '{c += $5} END {print c}'`;
delta=`echo "scale=3; (($joining2 - $joining1) / $joining1) * 100" | bc`;
echo "Joining overhead = $delta %" ;

tot1=`echo "$setup1 + $forking1 + $joining1" | bc`;
tot2=`echo "$setup2 + $forking2 + $joining2" | bc`;
delta=`echo "scale=3; (($tot2 - $tot1) / $tot1) * 100" | bc`;
echo "Total overhead = $delta %" ;

setup1Rel=`echo "scale=3; ($setup1 / $tot1) * 100" | bc` ;
forking1Rel=`echo "scale=3; ($forking1 / $tot1) * 100" | bc` ;
joining1Rel=`echo "scale=3; ($joining1 / $tot1) * 100" | bc` ;
echo "$1 breakdown" ;
echo "  $setup1Rel % (Setup)";
echo "  $forking1Rel % (Forking)";
echo "  $joining1Rel % (Joining)";

setup2Rel=`echo "scale=3; ($setup2 / $tot2) * 100" | bc` ;
forking2Rel=`echo "scale=3; ($forking2 / $tot2) * 100" | bc` ;
joining2Rel=`echo "scale=3; ($joining2 / $tot2) * 100" | bc` ;
invocs=`grep overhead $2 | grep Setup | wc -l | awk '{print $1}'` ;
tot2Avg=`echo "$tot2 / $invocs" | bc`;
echo "$2 breakdown of $tot2Avg clock cycles per invocation (among $invocs invocations)" ;
echo "  $setup2Rel % (Setup)";
echo "  $forking2Rel % (Forking)";
echo "  $joining2Rel % (Joining)";
