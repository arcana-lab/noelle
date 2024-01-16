#!/bin/bash

function patchInstallDir {
  local fileToPatch="$1" ;

  awk -v installDirectory="${installDir}" '{
    if ($1 == "installDir"){
      printf("%s=\"%s\"\n", $1, installDirectory);
    } else {
      print ;
    }
  }' scripts/$fileToPatch > ${installDir}/bin/$fileToPatch ;

  chmod 744 ${installDir}/bin/$fileToPatch ;

  return 
}

# Set the installation directory
installDir=$NOELLE_INSTALL_DIR ;
if test "$installDir" == "" ; then
  installDir="`git rev-parse --show-toplevel`/install"  ;
fi

mkdir -p ${installDir}/bin ;

patchInstallDir "noelle-norm" ;
patchInstallDir "noelle-load" ;
patchInstallDir "noelle-pdg" ;
patchInstallDir "noelle-meta-clean" ;
patchInstallDir "noelle-meta-loop-embed" ;
patchInstallDir "noelle-meta-pdg-clean " ;
patchInstallDir "noelle-meta-pdg-embed" ;
patchInstallDir "noelle-meta-scc-embed" ;
patchInstallDir "noelle-meta-prof-clean" ;
patchInstallDir "noelle-meta-prof-embed" ;
patchInstallDir "noelle-prof-coverage" ;
patchInstallDir "noelle-simplification" ;
patchInstallDir "noelle-codesize" ;
patchInstallDir "loopaa" ;
