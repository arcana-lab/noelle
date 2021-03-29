#!/bin/bash

# Fetch the inputs
if test $# -lt 2 ; then
  echo "USAGE: `basename $0` HOST_AND_DIR_TO_USE DEST_DIR" ;
  exit 1;
fi
hostAndDir="$1" ;
destDir="$2" ;

# Check if the destination directory already exist
if test -d "$destDir" ; then
  echo "Updating $destDir" ;
  pushd ./ ;
  cd "$destDir" ;
  git pull ;
  popd ;
  exit 0;
fi

# Clone the repository
echo "Cloning ${hostAndDir}" ;
git clone "${hostAndDir}" "$destDir" ;
