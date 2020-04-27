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

sed -i 's/add_llvm_library(Svf ${SOURCES})/add_llvm_library(Svf SHARED ${SOURCES})/g' svf/lib/CMakeLists.txt
sed -i 's/add_llvm_library(LLVMSvf ${SOURCES})/add_llvm_library(LLVMSvf SHARED ${SOURCES})/g' svf/lib/CMakeLists.txt
sed -i 's/target_link_libraries(Svf LLVMCudd )/# target_link_libraries(Svf LLVMCudd )/g' svf/lib/CMakeLists.txt
sed -i 's/target_link_libraries(Svf )/# target_link_libraries(Svf )/g' svf/lib/CMakeLists.txt

sed -i 's/add_llvm_library(Cudd ${SOURCES})/add_llvm_library(Cudd SHARED ${SOURCES})/g' svf/lib/CUDD/CMakeLists.txt
sed -i 's/add_llvm_library(LLVMCudd ${SOURCES})/add_llvm_library(LLVMCudd SHARED ${SOURCES})/g' svf/lib/CUDD/CMakeLists.txt
sed -i 's/set_target_properties(Cudd PROPERTIES COMPILE_FLAGS "-Wno-format -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -O3 -w -DHAVE_IEEE_754 -DSIZEOF_VOID_P=8 -DSIZEOF_LONG=8")/set_target_properties(Cudd PROPERTIES COMPILE_FLAGS "-Wno-format -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -O3 -w -DHAVE_IEEE_754 -DSIZEOF_VOID_P=8 -DSIZEOF_LONG=8 -fPIC")/g' svf/lib/CUDD/CMakeLists.txt
sed -i 's/set_target_properties(LLVMCudd PROPERTIES COMPILE_FLAGS "-Wno-format -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -O3 -w -DHAVE_IEEE_754 -DSIZEOF_VOID_P=8 -DSIZEOF_LONG=8")/set_target_properties(LLVMCudd PROPERTIES COMPILE_FLAGS "-Wno-format -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -O3 -w -DHAVE_IEEE_754 -DSIZEOF_VOID_P=8 -DSIZEOF_LONG=8 -fPIC")/g' svf/lib/CUDD/CMakeLists.txt