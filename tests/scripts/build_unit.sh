#!/bin/bash -e

# Set the installation directory
installDir="`realpath ../../install`/test"  ;
mkdir -p $installDir ;
export PDG_INSTALL_DIR="$installDir" ;

for i in `ls` ; do

  # Check if this is a directory
  if ! test -d "$i"; then
    continue ;
  fi

  # Skip install dir
  if test "$i" == "install" ; then
    continue ;
  fi
  echo $i

  # Compile 
  pushd ./ ;
  cd "$i" ;
  ../../../src/scripts/run_me.sh ;
  popd ;
done
