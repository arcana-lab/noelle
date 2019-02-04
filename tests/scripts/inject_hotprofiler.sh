#!/bin/bash -e

# Fetch the inputs
if test $# -lt 3 ; then
  echo "USAGE: `basename $0` SRC_BC DST_BC BINARY" ;
  exit 0;
fi
srcBC="$1" ;
profBC="$2" ;
profExec="$3" ;

# Clean
rm -f $profBC $profExec *.profraw ;

# Inject code needed by the profiler
opt -pgo-instr-gen -instrprof $srcBC -o $profBC ;

# Generate the binary
clang $profBC -fprofile-instr-generate -o $profExec ;
