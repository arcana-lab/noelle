#!/bin/bash

# Global variables
let idx=0 ;

# Functions
function generateCondor {
  local cf="$1" ;
  local no="$2" ;
  local po="$3" ;
  local feo="$4" ;

  # Create the directory
  pushd ./ ;
	cd ../ ; cp -r regression regression_${idx} ;
  popd ;

  baseBf="`basename ${cf}`" ;
  python2 scripts/generateCondorScript.py ${cf} ./${baseBf}_${idx}.con "regression_${idx}" "-noelle-pdg-check -noelle-verbose=3 ${no}" "${po}" "$feo" ${email};
	./scripts/appendTests.sh ./${baseBf}_${idx}.con ;
  let idx=idx+1 ;

  return 
}

function generateCondorJobs {
  local cores="$1" ;
  local coresStr="" ;
  if test "$cores" != "0" ; then
    coresStr="-noelle-max-cores=$cores" ;
  fi

  # Default parallelization
  noelleOptions="${coresStr}" ;
  generateCondor "$condorFile" "$noelleOptions" "$parOptions" "$feOptions"
  noelleOptions2="${noelleOptions} -noelle-inliner-avoid-hoist-to-main" ;
  generateCondor "$condorFile" "$noelleOptions2" "$parOptions" "$feOptions"
  noelleOptions2="${noelleOptions} -noelle-disable-loop-invariant-code-motion" ;
  generateCondor "$condorFile" "$noelleOptions2" "$parOptions" "$feOptions"

  # No HELIX
  noelleOptions="${coresStr}" ;
  noelleOptions="${noelleOptions} -noelle-disable-helix" ;
  generateCondor "$condorFile" "$noelleOptions" "$parOptions" "$feOptions"
  noelleOptions2="${noelleOptions} -noelle-disable-inliner" ;
  generateCondor "$condorFile" "$noelleOptions2" "$parOptions" "$feOptions"
  noelleOptions2="${noelleOptions} -noelle-disable-loop-invariant-code-motion" ;
  generateCondor "$condorFile" "$noelleOptions2" "$parOptions" "$feOptions"

  # No DSWP
  noelleOptions="${coresStr}" ;
  noelleOptions="${noelleOptions} -noelle-disable-dswp" ;
  generateCondor "$condorFile" "$noelleOptions" "$parOptions" "$feOptions"
  noelleOptions2="${noelleOptions} -noelle-disable-inliner" ;
  generateCondor "$condorFile" "$noelleOptions2" "$parOptions" "$feOptions"
  noelleOptions2="${noelleOptions} -noelle-disable-loop-invariant-code-motion" ;
  generateCondor "$condorFile" "$noelleOptions2" "$parOptions" "$feOptions"

  # No HELIX and DSWP
  noelleOptions="${coresStr}" ;
  noelleOptions="${noelleOptions} -noelle-disable-helix -noelle-disable-dswp" ;
  generateCondor "$condorFile" "$noelleOptions" "$parOptions" "$feOptions"
  noelleOptions2="${noelleOptions} -noelle-disable-inliner" ;
  generateCondor "$condorFile" "$noelleOptions2" "$parOptions" "$feOptions"
  noelleOptions2="${noelleOptions} -noelle-disable-loop-invariant-code-motion" ;
  generateCondor "$condorFile" "$noelleOptions2" "$parOptions" "$feOptions"

  # No HELIX, DSWP, and DOALL
  noelleOptions="${coresStr}" ;
  noelleOptions="${noelleOptions} -noelle-disable-helix -noelle-disable-dswp -noelle-disable-doall" ;
  generateCondor "$condorFile" "$noelleOptions" "$parOptions" "$feOptions"
  noelleOptions2="${noelleOptions} -noelle-disable-inliner" ;
  generateCondor "$condorFile" "$noelleOptions2" "$parOptions" "$feOptions"
  noelleOptions2="${noelleOptions} -noelle-disable-loop-invariant-code-motion" ;
  generateCondor "$condorFile" "$noelleOptions2" "$parOptions" "$feOptions"

  return ;
}

function generateAllCondorJobs {
  generateCondorJobs "0" ;
  generateCondorJobs "2" ;

  noelleOptions="-noelle-inliner-avoid-hoist-to-main -noelle-disable-helix" ;
  generateCondor "$condorFile" "$noelleOptions" "$parOptions" "$feOptions"

  noelleOptions="-noelle-disable-dswp -noelle-disable-doall -noelle-disable-helix -noelle-disable-inliner -noelle-disable-whilifier -noelle-disable-loop-distribution -noelle-disable-scev-simplification" ;
  generateCondor "$condorFile" "$noelleOptions" "$parOptions" "$feOptions"

  return 
}

# Fetch the input
if test $# -lt 1 ; then
  echo "USAGE: `basename $0` CONDOR_FILE [EMAIL]" ;
  exit 1;
fi
condorFile="$1" ;
email="" ;
if test $# -gt 1 ; then
  email="$2" ;
fi

# Generate the condor jobs
feOptions="-O0 -Xclang -disable-O0-optnone" ;
parOptions="-noelle-parallelizer-force";
generateAllCondorJobs ;

feOptions="-O1 -Xclang -disable-llvm-passes " ;
parOptions="-noelle-parallelizer-force";
generateAllCondorJobs ;
