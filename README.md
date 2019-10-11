# NOELLE: An Open Source Parallelizing Compilation Framework for Irregular Workloads


## Prerequisites
LLVM 5.0.0
GCC that supports C++ 14

To enable them on the Zythos cluster: 
  - source /project/llvm/5.0.0/enable
  - source /opt/rh/devtoolset-8/enable


## Build
To build, go to "src" and run "make -j".


## Test
To test NOELLE, go to "tests" and run "make".

To test NOELLE using condor to run all tests in parallel, go to "tests" and run "make condor".


## License
MIT License
