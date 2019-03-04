#!/bin/bash -e

# Set the installation directory
installDir=$PDG_INSTALL_DIR ;
if test "$installDir" == "" ; then
  installDir="~/CAT"
fi

# Install
# rm -rf build/ ; 

if [ ! -d "build" ]
then 
  mkdir build ; 
fi
cd build ; 
cmake -DCMAKE_INSTALL_PREFIX="${installDir}" -DCMAKE_BUILD_TYPE=Debug ../ ; 
make ;
make install ;
cd ../

if [ ! -f "compile_commands.json" ]
then 
  ln -s build/compile_commands.json
fi 
