#!/bin/bash

# Fetch the inputs
if test $# -lt 1 ; then
  echo "USAGE: `basename $0` DEBUG" ;
  exit 1;
fi
debugCompilation=$1 ;

# Fetch the installation directory
if test -z ${NOELLE_INSTALL_DIR} ; then
  installDir="`pwd`/../install" ;
else
  installDir="${NOELLE_INSTALL_DIR}" ;
fi

# Set the environment
export SPECULATION_MODULES=0;
export SCAF_INSTALL_DEBUG=${installDir} ;
export SCAF_INSTALL_RELEASE=${installDir} ;

# Compile SCAF
cd scaf ; 
if test "${debugCompilation}" == "1" ; then
	echo "NOELLE: Compiling SCAF in debug mode" ;
  make scaf-debug;

else
	echo "NOELLE: Compiling SCAF in release mode" ;
	make scaf-release;
fi
