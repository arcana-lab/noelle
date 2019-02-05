#!/bin/bash -e

# Fetch the inputs
if test $# -lt 2 ; then
  echo "USAGE: `basename $0` SRC_BC DST_BC BINARY" ;
  exit 0;
fi
srcBC="$1" ;
profBC="$2" ;
profExec="" ;
if test $# -ge 3 ; then
  profExec="$3" ;
fi

# Clean
rm -f $profBC $profExec *.profraw ;

# Inject code needed by the profiler
opt -pgo-instr-gen -instrprof $srcBC -o $profBC ;

# Generate the binary
if test "$profExec" != "" ; then
  clang $profBC -fprofile-instr-generate -o $profExec ;
fi
