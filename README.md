# <u>N</u>OELLE <u>O</u>ffers <u>E</u>mpowering <u>LL</u>VM <u>E</u>xtensions


## Table of Contents
- [Description](#description)
- [Version](#version)
- [Prerequisites](#prerequisites)
- [Building and Installing](#building-and-installing)
- [Testing](#testing)
- [Repository structure](#repository-structure)
- [Examples](#examples)
- [Contributing](#contributing)
- [License](#license)


## Description
NOELLE provides abstractions to help build advanced code analyses and transformations for LLVM IR code.
It is built upon [SVF](https://svf-tools.github.io/SVF/), [SCAF](https://github.com/PrincetonUniversity/SCAF.git), and [LLVM](http://llvm.org).

NOELLE is in active development so more tools, tests, and abstractions will be added.


This repository has been released to the general public in the hope of strengthening 
[We](https://users.cs.northwestern.edu/~simonec/Team.html) released NOELLE's source code in the hope of benefiting 
You are kindly asked to acknowledge any use of the tool by citing the following paper:
```
@inproceedings{NOELLE,
    title={{NOELLE} {O}ffers {E}mpowering {LL}VM {E}xtensions},
    author={Angelo Matni and Enrico Armenio Deiana and Yian Su and Lukas Gross and Souradip Ghosh and Sotiris Apostolakis and Ziyang Xu and Zujun Tan and Ishita Chaturvedi and David I. August and Simone Campanoni},
    booktitle={International Symposium on Code Generation and Optimization, 2022. CGO 2022.},
    year={2022}
}
```

The following material compose the documentation currently available:
- An introductory [video](https://www.youtube.com/watch?v=whORNUUWIjI)
- The [CGO 2022 Paper](http://www.cs.northwestern.edu/~simonec/files/Research/papers/HELIX_CGO_2022.pdf)
- The slides used during [Advanced Topics in Compilers](http://www.cs.northwestern.edu/~simonec/ATC.html) at Northwestern
- The [Github Wiki](https://github.com/arcana-lab/noelle/wiki) of the project
- Comments in the source code

## Version
The latest stable version is 9.14.1 (tag = `v9.14.1`).

#### Version Numbering Scheme
The version number is in the form of \[v _Major.Minor.Revision_ \]
- **Major**: Each major version matches a specific LLVM version (e.g., version 9 matches LLVM 9, version 11 matches LLVM 11)
- **Minor**: Starts from 0, each minor version represents either one or more API replacements/removals that might impact the users OR a forced update every six months (the minimum minor update frequency)
- **Revision**: Starts from 0; each revision version may include bug fixes or incremental improvements

#### Update Frequency
- **Major**: Matches the LLVM releases on a best-effort basis
- **Minor**: At least once per six months, at most once per month (1/month ~ 2/year)
- **Revision**: At least once per month, at most twice per week (2/week ~ 1/month)

## Prerequisites
LLVM 9.0.0

### Northwestern users
Those who have access to the Zythos cluster at Northwestern can source LLVM 9.0.0 from any node of the cluster with:
```
source /project/extra/llvm/9.0.0/enable
```
Check out the Zythos cluster guide [here](http://www.cs.northwestern.edu/~simonec/files/Research/manuals/Zythos_guide.pdf) for more.


## Building and Installing

To build and install NOELLE you need to configure it first, unless the [default configuration](config.default.cmake) is satisfactory.
From the root directory:
```
make menuconfig         # to customize the installation
make
```
The number of jobs can be changed with `make JOBS=4` (16 by default).

To build with any other generator, e.g. Ninja, use `make GENERATOR=Ninja`.

Run `make uninstall` to uninstall without cleaning the build files.

Run `make clean` to reset the repository to its initial state.
For generality, the install directory is not removed.

## Testing
To run all tests, invoke the following commands:
```
cd tests
make clean      # optional but recommended
make
```

## Repository structure
All the C++ source of the framework are in `src`.
More specifically, `src/core` contains all the abstractions, while `src/tools` contains a set of tools build on top of the core.
All tools are independent from each other.

- `bin` contains the scripts through which the user will run all analyses and transformations
- `doc` contains the documentation
- `examples` contains examples of how to build an LLVM pass that uses NOELLE
- `tests` contains unit tests

## NOELLE as an external project

NOELLE can be easily integrated your project with
[ExternalProject](https://cmake.org/cmake/help/latest/module/ExternalProject.html)
and [FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html).

By using **ExternalProject**, cmake will download, compile and install the repository at build time.
By the time you compile your project, NOELLE will be already installed.

```cmake
include(ExternalProject)
ExternalProject_Add(
    noelle
    GIT_REPOSITORY  "https://github.com/arcana-lab/noelle.git"
    GIT_TAG         v9.14.1
    BUILD_COMMAND   ${CMAKE_COMMAND} --build . -j16
    INSTALL_COMMAND ${CMAKE_COMMAND} --install .
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/noelle
        -DNOELLE_SVF=OFF
        -DNOELLE_SCAF=OFF
)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/noelle/include)
```

By using **FetchContent**, the repository will be made available as soon as cmake is run.
your project and noelle will then be compiled as a single project.

```cmake
include(FetchContent)
FetchContent_Declare(
    noelle
    GIT_REPOSITORY  "https://github.com/arcana-lab/noelle.git"
    GIT_TAG         v9.14.1
)
set(NOELLE_SVF OFF)
set(NOELLE_SCAF OFF)
FetchContent_MakeAvailable(noelle)
FetchContent_GetProperties(noelle)

# at this point noelle is available but NOT installed
include_directories(${noelle_SOURCE_DIR}/src/core/alloc_aa/include) # for example
```

## Examples
LLVM passes in the directory `examples/passes` shows use cases of NOELLE.

If you have any trouble using this framework feel free to reach out to us for help (contact simone.campanoni@northwestern.edu).

## Contributing
We welcome contributions from the community to improve this framework and evolve it to cater for more users.

NOELLE uses `clang-format` to ensure uniform styling across the project's source code.
To format all `.cpp` and `.hpp` files in the repository run `make format` from the root.
`clang-format` is run automatically as a pre-commit git hook, meaning that when you commit a file `clang-format` is automatically run on the file in-place.

Since git doesn't allow for git hooks to be installed when you clone the repository,
cmake will install the pre-commit git hook upon installation.

## License
NOELLE is licensed under the [MIT License](./LICENSE.md).
