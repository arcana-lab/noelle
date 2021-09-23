#!/bin/bash

# Fetch the inputs
if test $# -lt 3 ; then
  echo "USAGE: `basename $0` HOST_AND_DIR_TO_USE VERSION DEST_DIR" ;
  exit 1;
fi
hostAndDir="$1" ;
softwareVersion="$2" ;
destDir="$3" ;

# Check if the destination directory already exist
if test -d "$destDir" ; then
  exit 0;
fi

# Clone the repository
echo "Cloning ${hostAndDir}" ;
git clone -b ${softwareVersion} "${hostAndDir}" "$destDir" ;
