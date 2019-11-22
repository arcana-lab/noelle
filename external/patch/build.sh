#  !bash

# Set the installation directory
installDir=$PDG_INSTALL_DIR ;
if test "$installDir" == "" ; then
  installDir="`realpath ../../install`"  ;
fi
mkdir -p $installDir ;
export CC=clang
export CPP=clang++

alias cmake="cmake3"
# type './build.sh'  for release build
# type './build.sh debug'  for debug build

#########
# Please change the following home directories of your LLVM builds
########
LLVMRELEASE

if [[ $1 == 'debug' ]]
then
BuildTY='Debug'
else
BuildTY='Release'
fi
export LLVM_DIR=$LLVMRELEASE

export PATH=$LLVM_DIR/bin:$PATH
Build=$BuildTY'-build'

rm -rf $Build
mkdir $Build
cd $Build

if [[ $1 == 'debug' ]]
then
cmake -D CMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_INSTALL_PREFIX="${installDir}" ../
else
cmake -DCMAKE_INSTALL_PREFIX="${installDir}" ../
fi
cmake -DCMAKE_INSTALL_PREFIX="${installDir}" ../
make -j8

make install 
