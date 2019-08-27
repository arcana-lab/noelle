#!/bin/bash

# Set the installation directory
installDir=$PDG_INSTALL_DIR ;
if test "$installDir" == "" ; then
  eval installDir="~/CAT"
fi

mkdir -p ${installDir}/bin ;

awk -v installDirectory="${installDir}" '{
    if ($1 == "installDir"){
      printf("%s=\"%s\"\n", $1, installDirectory);
    } else {
      print ;
    }
  }' scripts/noelle > ${installDir}/bin/noelle ;

awk -v installDirectory="${installDir}" '{
    if ($1 == "installDir"){
      printf("%s=\"%s\"\n", $1, installDirectory);
    } else {
      print ;
    }
  }' scripts/noelle-pre > ${installDir}/bin/noelle-pre ;

awk -v installDirectory="${installDir}" '{
    if ($1 == "installDir"){
      printf("%s=\"%s\"\n", $1, installDirectory);
    } else {
      print ;
    }
  }' scripts/noelle-pgo > ${installDir}/bin/noelle-pgo ;


chmod 744 ${installDir}/bin/noelle ;
chmod 744 ${installDir}/bin/noelle-pre ;
chmod 744 ${installDir}/bin/noelle-pgo ;
