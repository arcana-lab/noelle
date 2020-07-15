# NOELLE: A Compilation Layer for Next Generation Code Analysis and Transformation

## Table of Contents
- [Prerequisites](#prerequisites)
- [Build NOELLE](#build_noelle)
- [Testing](#testing)
- [Structure](#structure)
- [License](#license)

## Prerequisites
LLVM 9.0.0

To enable it on the Zythos cluster: 
```
source /project/extra/llvm/9.0.0/enable
```


## Build NOELLE
To build, run from the repository root directory: `make`

Follow the next steps from the root directory to build NOELLE in parallel among the cores of your machine:
```
make external ;
cd src ; make -j ;
```

Run `make clean` from the root directory to clean the repository.

Run `make uninstall` from the root directory to uninstall the NOELLE installation


## Testing
To test NOELLE, go to "tests" and run "maker".

To test NOELLE using condor to run all tests in parallel, go to "tests" and run "make condor".


## Structure
The directory `src` includes sources of the noelle framework.

The directory `external` includes libraries that are external to noelle that are used by noelle.
Some of these libraries are patched and/or extended for noelle.

The directory `tests` includes unit tests, integration tests, and performance tests.
Furthermore, this directory includes the scripts to run all these tests in parallel via condor.

The directory `users` includes examples of LLVM passes (and their tests) that rely on the noelle framework.

Finally, the directory `doc` includes the documentation of noelle.

## License
MIT License
