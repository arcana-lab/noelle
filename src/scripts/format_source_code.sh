#!/bin/bash

for i in `find ./ -name *.cpp` ; do
  clang-format $i -i;
done
