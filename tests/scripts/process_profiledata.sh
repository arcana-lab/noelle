#!/bin/bash -e

# Prepare the profile results
llvm-profdata merge default.profraw -output=output.prof
#opt -load ~/CAT/lib/HOTProfiler.so -block-freq -pgo-instr-use -pgo-test-profile-file=output.prof -HOTProfiler $srcBC
