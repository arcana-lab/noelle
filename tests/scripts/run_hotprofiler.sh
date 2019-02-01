#!/bin/bash

# Fetch the inputs
if test $# -lt 1 ; then
  echo "USAGE: `basename $0` SRC_BC [INPUTS]" ;
  exit 0;
fi

srcBC=$1 ;
if test $# -gt 1 ; then
  inputs="$2" ;
  echo "Using inputs \"${inputs}\"" ;
fi
profBC="$1_prof.bc" ;
profExec="$1_bin" ;

# Clean
rm -f $profBC $profExec *.profraw ;

# Inject code needed by the profiler
opt -pgo-instr-gen -instrprof $srcBC -o $profBC ;

# Generate the binary
clang $profBC -fprofile-instr-generate -o $profExec ;

# Run
./$profExec "$inputs";

# Prepare the profile results
llvm-profdata merge default.profraw -output=output.prof
#opt -load ~/CAT/lib/HOTProfiler.so -block-freq -pgo-instr-use -pgo-test-profile-file=output.prof -HOTProfiler $srcBC
