#!/bin/bash

rm -rf build/ ; 
mkdir build ; 
cd build ; 
cmake3 -DCMAKE_INSTALL_PREFIX="~/CAT" -DCMAKE_BUILD_TYPE=Debug ../ ; 
make ;
make install ;
cd ../ 
