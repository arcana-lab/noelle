# NOELLE Offers Empowering LLVM Extensions


## Table of Contents
- [Description](#description)
- [Prerequisites](#prerequisites)
- [Build NOELLE](#build-noelle)
- [Testing](#testing)
- [Structure](#structure)
- [License](#license)


## Description
NOELLE provides abstractions to help build advanced code analyses and transformations.

NOELLE is in active development so more tools, tests, and abstractions will be added.


## Prerequisites
LLVM 9.0.0

For those that have access to the Zythos cluster at Northwestern: to enable the correct LLVM, run the following command from any node of the Zythos cluster:
```
source /project/extra/llvm/9.0.0/enable
```


## Build NOELLE
To build and install: run `make` from the repository root directory.

Run `make clean` from the root directory to clean the repository.

Run `make uninstall` from the root directory to uninstall the NOELLE installation.


## Testing
To run all tests in parallel using Condor:
```
make clean ; 
cd tests ;
make condor ;
```
To monitor how tests are doing: `cd tests ; make condor_watch`

To find out if all tests passed: `cd tests ; make condor_check`

To test NOELLE using condor to run all tests in parallel, go to "tests" and run "make condor".


## Structure
The directory `src` includes sources of the noelle framework.
Within this directory, `src/core` includes the abstractions provided by NOELLE.
Also, `src/tools` includes code transformations that rely on the NOELLE's abstractions to modify the code.

The directory `external` includes libraries that are external to noelle that are used by noelle.
Some of these libraries are patched and/or extended for noelle.

The directory `tests` includes unit tests, integration tests, and performance tests.
Furthermore, this directory includes the scripts to run all these tests in parallel via condor.

The directory `examples` includes examples of LLVM passes (and their tests) that rely on the noelle framework.

Finally, the directory `doc` includes the documentation of noelle.


### Examples of using NOELLE
LLVM passes in the directory `examples/passes` shows use cases of NOELLE.

If you have any trouble using this framework feel free to reach out to us for help (contact simone.campanoni@northwestern.edu).


### Contributions
We welcome contributions from the community to improve this framework and evolve it to cater for more users.


## License
NOELLE is licensed under the [MIT License](./LICENSE.md).
