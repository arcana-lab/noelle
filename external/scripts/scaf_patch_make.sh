#!/bin/bash

echo 'ifdef PDG_INSTALL_DIR
SCAF_INSTALL_DEBUG=$(PDG_INSTALL_DIR)
SCAF_INSTALL_RELEASE=$(PDG_INSTALL_DIR)
else
SCAF_INSTALL_DEBUG=$(shell pwd)/../../install
SCAF_INSTALL_RELEASE=$(shell pwd)/../../install
endif
SVF_AVAILABLE=1' > scaf/tmp_makefile

cat scaf/Makefile >> scaf/tmp_makefile

if test "$CLANG_BIN" != "" ; then
  sed -i "s#CC=clang#CC=$CLANG_BIN/clang#g" scaf/tmp_makefile
  sed -i "s#CXX=clang++#CXX=$CLANG_BIN/clang++#g" scaf/tmp_makefile
fi

mv scaf/tmp_makefile scaf/Makefile
