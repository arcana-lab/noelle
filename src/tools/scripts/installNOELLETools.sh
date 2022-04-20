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
installDir=$PDG_INSTALL_DIR ;
if test "$installDir" == "" ; then
  installDir="`git rev-parse --show-toplevel`/install"  ;
fi

mkdir -p ${installDir}/bin ;

patchInstallDir "noelle-parallel-load" ;
patchInstallDir "noelle-parallelizer" ;
patchInstallDir "noelle-inline" ;
patchInstallDir "noelle-enable" ;
patchInstallDir "noelle-pre" ;
patchInstallDir "noelle-deadcode" ;
patchInstallDir "noelle-codesize" ;
patchInstallDir "noelle-loopsize" ;
patchInstallDir "noelle-fixedpoint" ;
patchInstallDir "noelle-pdg-stats" ;
patchInstallDir "noelle-loop-stats" ;
patchInstallDir "noelle-parallelization-planner" ;
