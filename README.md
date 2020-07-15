# NOELLE: A Compilation Layer for Next Generation Code Analysis and Transformation


## Prerequisites
LLVM 9.0.0

To enable it on the Zythos cluster: 
```
source /project/extra/llvm/9.0.0/enable
```


## Build
To build, run from the repository root directory: `make`

Follow the next steps from the root directory to build NOELLE in parallel among the cores of your machine:
```
make external ;
cd src ; make -j ;
```

Run `make clean` from the root directory to clean the repository.

Run `make uninstall` from the root directory to uninstall the NOELLE installation


## Test
To test NOELLE, go to "tests" and run "maker".

To test NOELLE using condor to run all tests in parallel, go to "tests" and run "make condor".


## License
MIT License
