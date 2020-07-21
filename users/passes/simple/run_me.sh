#!/bin/bash -e

export CC=clang
export CXX=clang++

rm -rf build/ ; 
mkdir build ; 
cd build ; 
cmake -DCMAKE_INSTALL_PREFIX="~/CAT" -DCMAKE_BUILD_TYPE=Debug ../ ; 
make ;
make install ;
cd ../ 

cp build/compile_commands.json ./ ;
