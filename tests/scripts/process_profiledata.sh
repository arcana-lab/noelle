#!/bin/bash -e

# Prepare the profile results
llvm-profdata merge default.profraw -output=output.prof
