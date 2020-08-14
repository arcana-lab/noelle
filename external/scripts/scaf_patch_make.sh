#!/bin/bash

echo 'ifdef PDG_INSTALL_DIR
SCAF_INSTALL_DIR=$(PDG_INSTALL_DIR)
else
SCAF_INSTALL_DIR=$(shell pwd)/../../install
endif
SVF_AVAILABLE=1' > scaf/tmp_makefile

cat scaf/Makefile >> scaf/tmp_makefile
mv scaf/tmp_makefile scaf/Makefile
