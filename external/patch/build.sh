#  !bash

# Set the installation directory
installDir=$PDG_INSTALL_DIR ;
if test "$installDir" == "" ; then
  installDir="`realpath ../../install`"  ;
fi
mkdir -p $installDir ;
export CC=clang
export CPP=clang++

# Check the cmake binary
command -v cmake3
if test $? -eq 1 ; then
  CMAKE="cmake" ;
else
  CMAKE="cmake3" ;
fi

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
cmake -D CMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_INSTALL_PREFIX="${installDir}" -DCMAKE_CXX_FLAGS="-std=c++17" ../
else
cmake -DCMAKE_INSTALL_PREFIX="${installDir}" -DCMAKE_CXX_FLAGS="-std=c++17" ../
fi
cmake -DCMAKE_INSTALL_PREFIX="${installDir}" -DCMAKE_CXX_FLAGS="-std=c++17" ../
make -j8

make install 
