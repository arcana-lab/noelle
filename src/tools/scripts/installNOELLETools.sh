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

patchInstallDir "noelle-privatizer" ;
patchInstallDir "noelle-deadcode" ;
patchInstallDir "noelle-rm-function" ;
patchInstallDir "noelle-loopsize" ;
patchInstallDir "noelle-fixedpoint" ;
patchInstallDir "noelle-pdg-stats" ;
patchInstallDir "noelle-loop-stats" ;
