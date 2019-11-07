#!/bin/bash

llvmDir="`llvm-config --obj-root`";

awk -v llvmDir=${llvmDir} '
  {
    if ($2 == "LLVM_OBJ_ROOT=/home/ysui/llvm-9.0.0/llvm-9.0.0.obj"){
      printf("export LLVM_OBJ_ROOT=%s\n", llvmDir);
    } else {
      print ;
    }
  }' svf/setup.sh > setup.sh
mv setup.sh svf/ ;

awk -v llvmDir=${llvmDir} '
  {
    if ($1 == "LLVMRELEASE"){
      printf("LLVMRELEASE=%s\n", llvmDir);
    } else {
      print ;
    }
  }' svf/build.sh > build.sh ;

mv build.sh svf/build.sh ;

chmod 744 svf/*.sh ;
