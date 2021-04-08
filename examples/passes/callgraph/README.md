# LLVM_middleend_template

This is the template to use for assignments of the Code Analysis and Transformation class at Northwestern University.

Website of the class = https://users.cs.northwestern.edu/~simonec/CAT.html

To build: 
  Compile and install your code by invoking `./run_me.sh`
  The script run_me.sh compiles and installs an LLVM-based compiler that includes your CAT in the directory ~/CAT

To run:
  1) Add your compiler cat-c in your PATH (i.e., `export PATH=~/CAT/bin:$PATH`)

  2) Invoke your compiler to compile a C/C++ program. For example:
  ```sh
    $ cat-c program_to_analyse.c -o mybinary
    $ cat-c -O3 program_to_analyse.c -o mybinary
    $ cat-c -O0 program_to_analyse.bc -o mybinary
  ```
