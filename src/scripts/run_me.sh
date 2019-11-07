#!/bin/bash -e

# Set the installation directory
installDir=$PDG_INSTALL_DIR ;
if test "$installDir" == "" ; then
  installDir="`realpath ../../install`"  ;
fi
mkdir -p $installDir ;

# Check the cmake binary
command -v cmake3
if test $? -eq 1 ; then
  CMAKE="cmake" ;
else
  CMAKE="cmake3" ;
fi

# Set the compiler
export CC=clang
export CXX=clang++

# Install
pushd ./ ;
rm -rf build/ ; 
mkdir build ; 
cd build ; 
${CMAKE} -DCMAKE_INSTALL_PREFIX="${installDir}" -DCMAKE_BUILD_TYPE=Debug ../ ; 
make -j;
make install ;
popd ;

if test -d src ; then
  pushd ./ ;
  cd src ;
  if ! test -e compile_commands.json ; then
    ln -s ../build/compile_commands.json
  fi
  popd ;
fi

if test -d include ; then
  pushd ./ ;
  cd include ;
  if ! test -e compile_commands.json ; then
    ln -s ../build/compile_commands.json
  fi
  popd ;
fi
