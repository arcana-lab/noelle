#!/bin/bash -e

# Set the installation directory
installDir=$PDG_INSTALL_DIR ;
if test "$installDir" == "" ; then
  installDir="~/CAT"
fi

# Check the cmake binary
command -v cmake3
if test $? -eq 1 ; then
  CMAKE="cmake" ;
else
  CMAKE="cmake3" ;
fi

# Install
pushd ./ ;
rm -rf build/ ; 
mkdir build ; 
cd build ; 
${CMAKE} -DCMAKE_INSTALL_PREFIX="${installDir}" -DCMAKE_BUILD_TYPE=Debug ../ ; 
make ;
make install ;
popd ;

pushd ./ ;
cd src ;
if ! test -f compile_commands.json ; then
  ln -s ../build/compile_commands.json
fi
popd ;

if test -d include ; then
  pushd ./ ;
  cd include ;
  if ! test -f compile_commands.json ; then
    ln -s ../build/compile_commands.json
  fi
  popd ;
fi
