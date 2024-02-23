#!/bin/bash -e

# Set the installation directory
installDir=$NOELLE_INSTALL_DIR ;
if test "$installDir" == "" ; then
  installDir="`git rev-parse --show-toplevel`/install"  ;
fi
mkdir -p $installDir ;

# Check the cmake binary
CMAKE="cmake" ;

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

# Check if anything has been built
if ! test -e build/compile_commands.json ; then
  exit 0;
fi

# Add the link to the compiler commands file
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

if ! test -e compile_commands.json ; then
  ln -s build/compile_commands.json
fi
