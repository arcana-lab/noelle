#!/bin/bash

# Set the installation directory
installDir=$PDG_INSTALL_DIR ;
if test "$installDir" == "" ; then
  installDir="~/CAT"
fi

# Install
rm -rf build/ ; 
mkdir build ; 
cd build ; 
cmake3 -DCMAKE_INSTALL_PREFIX="${installDir}" -DCMAKE_BUILD_TYPE=Debug ../ ; 
make ;
make install ;
cd ../ 
