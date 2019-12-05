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

  return 
}

# Set the installation directory
installDir=$PDG_INSTALL_DIR ;
if test "$installDir" == "" ; then
  eval installDir="`realpath ../install`"  ;
fi

mkdir -p ${installDir}/bin ;

patchInstallDir "noelle-load" ;
patchInstallDir "noelle" ;
patchInstallDir "noelle-norm" ;
patchInstallDir "noelle-pre" ;
patchInstallDir "noelle-pgo" ;
patchInstallDir "noelle-seq" ;
patchInstallDir "noelle-inline" ;
patchInstallDir "noelle-loopmeta" ;

chmod 744 ${installDir}/bin/noelle-load ;
chmod 744 ${installDir}/bin/noelle ;
chmod 744 ${installDir}/bin/noelle-norm ;
chmod 744 ${installDir}/bin/noelle-pre ;
chmod 744 ${installDir}/bin/noelle-pgo ;
chmod 744 ${installDir}/bin/noelle-seq ;
chmod 744 ${installDir}/bin/noelle-inline ;
chmod 744 ${installDir}/bin/noelle-loopmeta ;
